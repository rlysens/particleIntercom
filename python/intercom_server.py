#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

#!/usr/bin/python

"""
A simple "udp echo server" for demonstrating UDP usage.
The server listens for UDP frames and echoes any received
frames back to the originating host.

"""

import socket
import string
import sys
import time
import getopt
import select
import pdb
import xtea
import random
import string
import json
from threading import Thread
from threading import Event

from messages import *

HOST = ''
HOST_PORT = 50007
NAME_KEYS = None
MAX_NUM_BUDDIES = 3
#ENCRYPTION_MODE = xtea.MODE_CBC
ENCRYPTION_MODE = xtea.MODE_ECB
#ENCRYPTION_MODE = xtea.MODE_NONE

PRINT_MSG_RX_TX = True

keyPressThreadToMainEvent = Event()
mainToKeyPressThreadEvent = Event()
quitEvent = Event()

id_counter = 1

intercom_name_to_id_table = {}

intercom_id_to_intercom_table = {}

NO_FILTER = 0
FILTER_ALL = -1

inputFilter = NO_FILTER
outputFilter = NO_FILTER

def keyPressThread(dummy):
    while not quitEvent.isSet():
        raw_input()
        keyPressThreadToMainEvent.set()
        mainToKeyPressThreadEvent.wait()
        mainToKeyPressThreadEvent.clear()

def handleKeyboardInput():
    global inputFilter
    global outputFilter

    quitStringRaw = raw_input('Quit? (y/n) --> ')
    if quitStringRaw!= 'y':
        debugStringRaw = raw_input('Debug? (y/n) --> ')
        if debugStringRaw=='y':
            pdb.set_trace()

        inputFilterStringRaw = raw_input('Display Rx Messages from: all/none/<name> --> ')
        outputFilterStringRaw = raw_input('Display Tx Messages to: all/none/<name> --> ')
        
        if inputFilterStringRaw == 'all':
            inputFilter = NO_FILTER
        elif inputFilterStringRaw == 'none':
            inputFilter = FILTER_ALL
        else:
            inputFilterStringRaw += '\x00'*min(32,32-len(inputFilterStringRaw))
            inputFilter = intercom_name_to_id_table.get(inputFilterStringRaw)
            if not inputFilter:
                print "Invalid Rx filter."

        if outputFilterStringRaw == 'all':
            outputFilter = NO_FILTER
        elif outputFilterStringRaw == 'none':
            outputFilter = FILTER_ALL
        else:
            outputFilterStringRaw += '\x00'*min(32,32-len(outputFilterStringRaw))
            outputFilter = intercom_name_to_id_table.get(outputFilterStringRaw)
            if not outputFilter:
                print "Invalid Tx filter."
    else:
        quitEvent.set()

def trimString(s):
    zeroPos = s.find('\0')
    if zeroPos == -1:
        return s
    else:
        return s[:zeroPos]

class Intercom_MessageHandler:
    def __init__(self, msg_table):
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._sock.bind((HOST, HOST_PORT))
        self._msg_table = msg_table
        self.source_id = 0 #Server is 0

    def send(self, payload_data, msgId, address, destination_id, source_id=None, cryptoCodec=None):
        global outputFilter

        if ((outputFilter==NO_FILTER) or (outputFilter==destination_id)):
            name = 'X'
            try:
                name = intercom_id_to_intercom_table[destination_id].name
            except:
                pass

            if len(message_name_table.messageNameTable) > msgId:
                print "Tx Msg: %s(%d) Dst:%s(%d)"%(message_name_table.messageNameTable[msgId], msgId, name, destination_id)
            else:
                print "Tx Msg: XX(%d) Dst: %s(%d)"%(msgId, name, destination_id)

        if cryptoCodec:
            payload_data = cryptoCodec.encrypt(payload_data)
            if ENCRYPTION_MODE == xtea.MODE_CBC:
                cryptoCodec.IV = payload_data[-8:]
                
        msg = []
        
        msg.append(chr(msgId&0xff))
        msg.append(chr((msgId>>8)&0xff))
        msg.append(chr((msgId>>16)&0xff))
        msg.append(chr((msgId>>24)&0xff))

        if source_id == None:
            source_id = self.source_id
        msg.append(chr(source_id&0xff))
        msg.append(chr((source_id>>8)&0xff))
        msg.append(chr((source_id>>16)&0xff))
        msg.append(chr((source_id>>24)&0xff))

        msg.extend(payload_data)

        self._sock.sendto(''.join(msg), address)

    def receive(self):
        global inputFilter

        data, address = self._sock.recvfrom(1024)
        if len(data) < 4:
            print "Msg. too short."
            return False

        msgId = ord(data[0])+(ord(data[1])<<8)+(ord(data[2])<<16)+(ord(data[3])<<24)
        source_id = ord(data[4])+(ord(data[5])<<8)+(ord(data[6])<<16)+(ord(data[7])<<24)
        if not self._msg_table.has_key(msgId):
            print "Unknown Message %d"%(msgId)
            return

        intercom = intercom_id_to_intercom_table.get(source_id)

        #Invoke the handler
        if ((inputFilter==NO_FILTER) or (inputFilter==source_id)):
            if intercom:
                name = intercom.name
            else:
                name = 'X'
            
            if len(message_name_table.messageNameTable) > msgId:
                print "Rx Msg: %s(%d) Src: %s(%d)"%(message_name_table.messageNameTable[msgId], msgId, name, source_id)
            else:
                print "Rx Msg: XX(%d) Src: %s(%d)"%(msgId, name, source_id)
            
        fun, decrypt = self._msg_table[msgId]

        msg_payload = None        
        if decrypt:
            if intercom:
                cryptoCodec = intercom.getDecoderCryptoCodec()
                msg_payload = cryptoCodec.decrypt(data[8:])
                if ENCRYPTION_MODE == xtea.MODE_CBC:
                    cryptoCodec.IV = data[-8:]
            else:
                print "Can't decrypt msgId %d from %s"%(msgId, name)
        else:
            msg_payload = data[8:]

        if msg_payload:
            try: #A decode failure will trigger a ValueError exception
                fun(msg_payload, address, self, intercom)
            except ValueError as v:
                print "Can't decode msgId %d from %s"%(msgId, name)
                print v

class Intercom:
    def __init__(self, name, id, address, msg_handler):
        global NAME_KEYS
        self.name = name
        self.id = id
        self.address = address
        self.msg_handler = msg_handler
        self.buddy_list = [0,0,0]
        self.next_buddy_idx = 0
        #This could raise an exception
        self.keyString = NAME_KEYS[trimString(self.name)]
        self.encCrypto = xtea.new(self.keyString, mode=ENCRYPTION_MODE, IV="\0"*8)
        self.decCrypto = xtea.new(self.keyString, mode=ENCRYPTION_MODE, IV="\0"*8)

    def getEncoderCryptoCodec(self):
        return self.encCrypto

    def getDecoderCryptoCodec(self):
        return self.decCrypto

    def sendTo(self, sender_id, msg, msgId, encrypt):
        if sender_id in self.buddy_list:
            if encrypt:
                self.msg_handler.send(msg, msgId, self.address, self.id, sender_id, self.encCrypto)
            else:
                self.msg_handler.send(msg, msgId, self.address, self.id, sender_id, None)
        else:
            print "sendTo %s: sender %d not in buddy list"%(self.name, sender_id)
            print self.buddy_list

    def setBuddy(self, id):
        if id not in self.buddy_list:
            self.buddy_list[self.next_buddy_idx] = id
            self.next_buddy_idx += 1
            if self.next_buddy_idx >= 3:
                self.next_buddy_idx =0 
        
def msg_keep_alive_handler(msg_data, address, msg_handler, intercom):
    keep_alive = keep_alive_t.keep_alive_t.decode(msg_data)
    buddy_intercom = intercom_id_to_intercom_table.get(keep_alive.destination_id)
    if buddy_intercom:
        buddy_intercom.sendTo(keep_alive.source_id, msg_data, 
            keep_alive_t.keep_alive_t.MSG_ID, True)
    else:
        print "Unknown destination %d"%(keep_alive.destination_id)

def msg_comm_start_handler(msg_data, address, msg_handler, intercom):
    comm_start = comm_start_t.comm_start_t.decode(msg_data)
    buddy_intercom = intercom_id_to_intercom_table.get(comm_start.destination_id)
    if buddy_intercom:
        buddy_intercom.sendTo(comm_start.source_id, msg_data, 
            comm_start_t.comm_start_t.MSG_ID, True)
    else:
        print "Unknown destination %d"%(comm_start.destination_id)

def msg_comm_start_ack_handler(msg_data, address, msg_handler, intercom):
    comm_start_ack = comm_start_ack_t.comm_start_ack_t.decode(msg_data)
    buddy_intercom = intercom_id_to_intercom_table.get(comm_start_ack.destination_id)
    if buddy_intercom:
        buddy_intercom.sendTo(comm_start_ack.source_id, msg_data, 
            comm_start_ack_t.comm_start_ack_t.MSG_ID, True)
    else:
        print "Unknown destination %d"%(comm_start_ack.destination_id)

def msg_comm_stop_handler(msg_data, address, msg_handler, intercom):
    comm_stop = comm_stop_t.comm_stop_t.decode(msg_data)
    buddy_intercom = intercom_id_to_intercom_table.get(comm_stop.destination_id)
    if buddy_intercom:
        buddy_intercom.sendTo(comm_stop.source_id, msg_data, 
            comm_stop_t.comm_stop_t.MSG_ID, True)
    else:
        print "Unknown destination %d"%(comm_stop.destination_id)

def msg_comm_stop_ack_handler(msg_data, address, msg_handler, intercom):
    comm_stop_ack = comm_stop_ack_t.comm_stop_ack_t.decode(msg_data)
    buddy_intercom = intercom_id_to_intercom_table.get(comm_stop_ack.destination_id)
    if buddy_intercom:
        buddy_intercom.sendTo(comm_stop_ack.source_id, msg_data, 
            comm_stop_ack_t.comm_stop_ack_t.MSG_ID, True)
    else:
        print "Unknown destination %d"%(comm_stop_ack.destination_id)

def msg_voice_data_handler(msg_data, address, msg_handler, intercom):
    voice_data = voice_data_t.voice_data_t.decode(msg_data)
    buddy_intercom = intercom_id_to_intercom_table.get(voice_data.destination_id)
    if buddy_intercom:
        buddy_intercom.sendTo(voice_data.source_id, msg_data, 
            voice_data_t.voice_data_t.MSG_ID, True)

def msg_i_am_handler(msg_data, address, msg_handler, intercom):
    global id_counter

    i_am = i_am_t.i_am_t.decode(msg_data)

    name = ''
    for x in i_am.name:
        if x==0:
            break
        name += chr(x)

    createNewIntercom = False
    id = intercom_name_to_id_table.get(name)

    if not id:
        #Not known yet. Add it to name->id table
        print "New name %s"%name
        id = id_counter
        id_counter += 1
        intercom_name_to_id_table[name] = id
        createNewIntercom = True

    assert id is not None

    if i_am.restarted != 0:
        #If the intercom indicates it's just been restarted, also create a fresh object
        createNewIntercom = True
        
    if createNewIntercom:
        try:
            intercom_id_to_intercom_table[id] = Intercom(name, id, address, msg_handler)
        except:
            print "Could not create Intercom %s. Name not recognized."%(name)
            pass

    #Send response
    intercom = intercom_id_to_intercom_table.get(id)
    if intercom:
        i_am_reply = i_am_reply_t.i_am_reply_t()
        i_am_reply.id = id
        i_am_reply.name = i_am.name
        data = i_am_reply.encode()
        cryptoCodec = intercom.getEncoderCryptoCodec()
        msg_handler.send(data, i_am_reply_t.i_am_reply_t.MSG_ID, address, id, source_id=None, cryptoCodec=cryptoCodec)

def msg_who_is_handler(msg_data, address, msg_handler, intercom):
    who_is = who_is_t.who_is_t.decode(msg_data)
    
    name = ''
    for x in who_is.name:
        if x==0:
            break
        name += chr(x)

    #Known name?
    id = intercom_name_to_id_table.get(name)
    if id:
        #Send who-is-reply-back
        who_is_reply = who_is_reply_t.who_is_reply_t()
        who_is_reply.name = who_is.name
        who_is_reply.id = id
        data = who_is_reply.encode()
        assert intercom
        cryptoCodec = intercom.getEncoderCryptoCodec()
        msg_handler.send(data, who_is_reply_t.who_is_reply_t.MSG_ID, address, id,
            source_id=None, cryptoCodec=cryptoCodec)

def set_buddy_handler(msg_data, address, msg_handler, intercom):
    set_buddy = set_buddy_t.set_buddy_t.decode(msg_data)
    if intercom_id_to_intercom_table.has_key(set_buddy.my_id):
        assert intercom
        intercom.setBuddy(set_buddy.buddy_id)
    else:
        print "add_buddy.my_id unknown %d"%(add_buddy.buddy_id)

MSG_TABLE = {
    voice_data_t.voice_data_t.MSG_ID : (msg_voice_data_handler, True),
    i_am_t.i_am_t.MSG_ID : (msg_i_am_handler, False),
    who_is_t.who_is_t.MSG_ID : (msg_who_is_handler, True),
    set_buddy_t.set_buddy_t.MSG_ID : (set_buddy_handler, True),
    keep_alive_t.keep_alive_t.MSG_ID : (msg_keep_alive_handler, True),
    comm_start_t.comm_start_t.MSG_ID : (msg_comm_start_handler, True),
    comm_start_ack_t.comm_start_ack_t.MSG_ID : (msg_comm_start_ack_handler, True),
    comm_stop_t.comm_stop_t.MSG_ID : (msg_comm_stop_handler, True),
    comm_stop_ack_t.comm_stop_ack_t.MSG_ID : (msg_comm_stop_ack_handler, True),
}

try:
    this_file = __file__
except NameError:
    this_file = "intercom_server.py"

def main(argv):
    global NAME_KEYS
    nameKeyJsonFile = open('name_key.json','r')
    NAME_KEYS = json.load(nameKeyJsonFile)
    nameKeyJsonFile.close()

    msg_handler = Intercom_MessageHandler(MSG_TABLE)

    t = Thread(target=keyPressThread, args=(0,))
    t.start()

    print "Entering receive loop."
    while not quitEvent.isSet():
        msg_handler.receive()
        if keyPressThreadToMainEvent.isSet():
            keyPressThreadToMainEvent.clear()
            handleKeyboardInput()
            mainToKeyPressThreadEvent.set()

    t.join()

if __name__ == "__main__":
    main(sys.argv[1:])
