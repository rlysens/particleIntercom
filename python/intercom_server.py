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

from messages import *

HOST = ''
HOST_PORT = 50007
NAME_KEYS = None
MAX_NUM_BUDDIES = 3

id_counter = 1

intercom_name_to_id_table = {}

intercom_id_to_intercom_table = {}

def trimString(s):
    zeroPos = s.find('\0')
    if zeroPos == -1:
        return s
    else:
        return s[:zeroPos]

class Message_Handler:
    def __init__(self, msg_table):
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._sock.bind((HOST, HOST_PORT))
        self._msg_table = msg_table
        self.source_id = 0 #Server is 0

    def send(self, payload_data, msg_id, address, cryptoCodec=None):

        print "Tx Msg: %d"%(msg_id)

        if cryptoCodec:
            payload_data = cryptoCodec.encrypt(payload_data)
            cryptoCodec.IV = payload_data[-8:]
                
        msg = []
        
        msg.append(chr(msg_id&0xff))
        msg.append(chr((msg_id>>8)&0xff))
        msg.append(chr((msg_id>>16)&0xff))
        msg.append(chr((msg_id>>24)&0xff))

        msg.append(chr(self.source_id&0xff))
        msg.append(chr((self.source_id>>8)&0xff))
        msg.append(chr((self.source_id>>16)&0xff))
        msg.append(chr((self.source_id>>24)&0xff))

        msg.extend(payload_data)

        self._sock.sendto(''.join(msg), address)

    def receive(self):
        data, address = self._sock.recvfrom(1024)
        if len(data) < 4:
            print "Msg. too short."
            return False

        msg_id = ord(data[0])+(ord(data[1])<<8)+(ord(data[2])<<16)+(ord(data[3])<<24)
        source_id = ord(data[4])+(ord(data[5])<<8)+(ord(data[6])<<16)+(ord(data[7])<<24)
        if not self._msg_table.has_key(msg_id):
            print "Unknown Message %d"%(msg_id)
            return

        intercom = None
        if intercom_id_to_intercom_table.has_key(source_id):
            intercom = intercom_id_to_intercom_table[source_id]

        #Invoke the handler
        print "Rx Msg: %d Src: %d"%(msg_id, source_id)
        fun, decrypt = self._msg_table[msg_id]

        msg_payload = None        
        if decrypt:
            if intercom:
                cryptoCodec = intercom.getDecoderCryptoCodec()
                msg_payload = cryptoCodec.decrypt(data[8:])
                cryptoCodec.IV = data[-8:]
            else:
                print "Can't decrypt msg_id %d source_id %x"%(msg_id, source_id)
        else:
            msg_payload = data[8:]

        if msg_payload:
            try: #A decode failure will trigger a ValueError exception
                fun(msg_payload, address, self, intercom)
            except ValueError:
                print "Can't decode msg_id %d source_id %x"%(msg_id, source_id)

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
        self.encCrypto = xtea.new(self.keyString, mode=xtea.MODE_CBC, IV="\0"*8)
        self.decCrypto = xtea.new(self.keyString, mode=xtea.MODE_CBC, IV="\0"*8)

    def getEncoderCryptoCodec(self):
        return self.encCrypto

    def getDecoderCryptoCodec(self):
        return self.decCrypto

    def sendTo(self, sender_id, msg, msg_id, encrypt):
        if sender_id in self.buddy_list:
            if encrypt:
                self.msg_handler.send(msg, msg_id, self.address, self.encCrypto)
            else:
                self.msg_handler.send(msg, msg_id, self.address)
        else:
            print "sender %d not in buddy list"%(sender_id)

    def setBuddy(self, id):
        if (id not in self.buddy_list):
            self.buddy_list[self.next_buddy_idx] = id
            self.next_buddy_idx += 1
            if self.next_buddy_idx >= 3:
                self.next_buddy_idx =0 
        
def msg_echo_request_handler(msg_data, address, msg_handler, intercom):
    echo_request = echo_request_t.echo_request_t.decode(msg_data)
    if intercom_id_to_intercom_table.has_key(echo_request.destination_id):
        buddy_intercom = intercom_id_to_intercom_table[echo_request.destination_id]
        buddy_intercom.sendTo(echo_request.source_id, msg_data, 
            echo_request_t.echo_request_t.MSG_ID, True)
    else:
        print "Unknown destination %d"%(echo_request.destination_id)

def msg_echo_reply_handler(msg_data, address, msg_handler, intercom):
    echo_reply = echo_reply_t.echo_reply_t.decode(msg_data)
    if intercom_id_to_intercom_table.has_key(echo_reply.destination_id):
        buddy_intercom = intercom_id_to_intercom_table[echo_reply.destination_id]
        buddy_intercom.sendTo(echo_reply.source_id, msg_data, 
            echo_reply_t.echo_reply_t.MSG_ID, True)
    else:
        print "Unknown destination %d"%(echo_reply.destination_id)

def msg_voice_data_handler(msg_data, address, msg_handler, intercom):
    voice_data = voice_data_t.voice_data_t.decode(msg_data)
    if intercom_id_to_intercom_table.has_key(voice_data.destination_id):
        buddy_intercom = intercom_id_to_intercom_table[voice_data.destination_id]
        buddy_intercom.sendTo(voice_data.source_id, msg_data, 
            voice_data_t.voice_data_t.MSG_ID, True)
    else:
        print "Unknown destination %d"%(voice_data.destination_id)

def msg_i_am_handler(msg_data, address, msg_handler, intercom):
    global id_counter

    i_am = i_am_t.i_am_t.decode(msg_data)
    i_am.name = ''.join([chr(x) for x in i_am.name])

    createNewIntercom = False
    id = None

    if not intercom_name_to_id_table.has_key(i_am.name):
        #Not known yet. Add it to name->id table
        print "New name %s"%i_am.name
        id = id_counter
        id_counter += 1
        intercom_name_to_id_table[i_am.name] = id
        createNewIntercom = True
    else:
        id = intercom_name_to_id_table[i_am.name]

    assert id is not None

    if i_am.restarted != 0:
        #If the intercom indicates it's just been restarted, also create a fresh object
        createNewIntercom = True
        
    if createNewIntercom:
        try:
            intercom_id_to_intercom_table[id] = Intercom(i_am.name, id, address, msg_handler)
        except:
            print "Could not create Intercom %s. Name not recognized."%(i_am.name)
            pass

    #Send response
    intercom = intercom_id_to_intercom_table.get(id)
    if intercom:
        i_am_reply = i_am_reply_t.i_am_reply_t()
        i_am_reply.id = id
        i_am_reply.name = [ord(x) for x in i_am.name]
        data = i_am_reply.encode()
        cryptoCodec = intercom.getEncoderCryptoCodec()
        msg_handler.send(data, i_am_reply_t.i_am_reply_t.MSG_ID, address, cryptoCodec)

def msg_who_is_handler(msg_data, address, msg_handler, intercom):
    who_is = who_is_t.who_is_t.decode(msg_data)
    who_is.name = ''.join([chr(x) for x in who_is.name])
    #Known name?
    if intercom_name_to_id_table.has_key(who_is.name):
        id = intercom_name_to_id_table[who_is.name]
        #Send who-is-reply-back
        who_is_reply = who_is_reply_t.who_is_reply_t()
        who_is_reply.name = [ord(x) for x in who_is.name]
        who_is_reply.id = id
        data = who_is_reply.encode()
        assert intercom
        cryptoCodec = intercom.getEncoderCryptoCodec()
        msg_handler.send(data, who_is_reply_t.who_is_reply_t.MSG_ID, address,
            cryptoCodec)

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
    echo_request_t.echo_request_t.MSG_ID : (msg_echo_request_handler, True),
    echo_reply_t.echo_reply_t.MSG_ID : (msg_echo_reply_handler, True)
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

    msg_handler = Message_Handler(MSG_TABLE)

    print "Entering receive loop."
    while True:
        msg_handler.receive()

if __name__ == "__main__":
    main(sys.argv[1:])
