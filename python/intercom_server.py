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

from messages import *

HOST = ''
HOST_PORT = 50007

id_counter = 0

intercom_name_to_id_table = {}

intercom_id_to_intercom_table = {}

class Message_Handler:
    def __init__(self, msg_table):
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._sock.bind((HOST, HOST_PORT))
        self._msg_table = msg_table

    def send(self, payload_data, id, address):

        print "Tx Msg: %d"%(id)
        msg = []
        
        msg.append(chr(id&0xff))
        msg.append(chr((id>>8)&0xff))
        msg.append(chr((id>>16)&0xff))
        msg.append(chr((id>>24)&0xff))

        msg.extend(payload_data)

        self._sock.sendto(''.join(msg), address)

    def receive(self):
        data, address = self._sock.recvfrom(1024)
        if len(data) < 4:
            print "Msg. too short."
            return False

        msg_id = ord(data[0])+(ord(data[1])<<8)+(ord(data[2])<<16)+(ord(data[3])<<24)
        if not self._msg_table.has_key(msg_id):
            print "Unknown Message %d"%(msg_id)
            return

        #Invoke the handler
        print "Rx Msg: %d"%(msg_id)
        fun = self._msg_table[msg_id]
        fun(data[4:], address, self)

class Intercom:
    def __init__(self, name, id, address, msg_handler):
        self.name = name
        self.id = id
        self.address = address
        self.msg_handler = msg_handler
        self.buddy_list = []

    def sendTo(self, sender_id, voice_data_msg):
        if sender_id in self.buddy_list:
            self.msg_handler.send(voice_data_msg, voice_data_t.voice_data_t.MSG_ID, self.address)
        else:
            print "sender %d not in buddy list"%(sender_id)

    def addBuddy(self, id):
        if id not in self.buddy_list:
            self.buddy_list.append(id)

    def delBuddy(self, id):
        if id in self.buddy_list:
            self.buddy_list.remove(id)
        
def msg_voice_data_handler(msg_data, address, msg_handler):
    voice_data = voice_data_t.voice_data_t.decode(msg_data)
    if intercom_id_to_intercom_table.has_key(voice_data.destination_id):
        intercom = intercom_id_to_intercom_table[voice_data.destination_id]
        intercom.sendTo(voice_data.source_id, msg_data)
    else:
        print "Unknown destination %d"%(voice_data.destination_id)

def msg_i_am_handler(msg_data, address, msg_handler):
    global id_counter

    i_am = i_am_t.i_am_t.decode(msg_data)
    i_am.name = ''.join([chr(x) for x in i_am.name])
    #Known name or new name?
    if not intercom_name_to_id_table.has_key(i_am.name):
        print "New name %s"%i_am.name
        id = id_counter
        id_counter += 1
        intercom_name_to_id_table[i_am.name] = id
        intercom_id_to_intercom_table[id] = Intercom(i_am.name, id, address, msg_handler)

    #Send response
    if intercom_name_to_id_table.has_key(i_am.name):
        i_am_reply = i_am_reply_t.i_am_reply_t()
        i_am_reply.id = intercom_name_to_id_table[i_am.name]
        i_am_reply.name = [ord(x) for x in i_am.name]
        data = i_am_reply.encode()
        msg_handler.send(data, i_am_reply_t.i_am_reply_t.MSG_ID, address)

def msg_who_is_handler(msg_data, address, msg_handler):
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
        msg_handler.send(data, who_is_reply_t.who_is_reply_t.MSG_ID, address)

def add_buddy_handler(msg_data, address, msg_handler):
    add_buddy = add_buddy_t.add_buddy_t.decode(msg_data)
    if intercom_id_to_intercom_table.has_key(add_buddy.my_id):
        intercom = intercom_id_to_intercom_table[add_buddy.my_id]
        intercom.addBuddy(add_buddy.buddy_id)
    else:
        print "add_buddy.my_id unknown %d"%(add_buddy.buddy_id)

def del_buddy_handler(msg_data, address, msg_handler):
    del_buddy = del_buddy_t.del_buddy_t.decode(msg_data)
    if intercom_id_to_intercom_table.has_key(del_buddy.my_id):
        intercom = intercom_id_to_intercom_table[del_buddy.my_id]
        intercom.delBuddy(del_buddy.buddy_id)
    else:
        print "del_buddy.my_id unknown %d"%(del_buddy.buddy_id)

MSG_TABLE = {
    voice_data_t.voice_data_t.MSG_ID : msg_voice_data_handler,
    i_am_t.i_am_t.MSG_ID : msg_i_am_handler,
    who_is_t.who_is_t.MSG_ID : msg_who_is_handler,
    add_buddy_t.add_buddy_t.MSG_ID : add_buddy_handler,
    del_buddy_t.del_buddy_t.MSG_ID : del_buddy_handler
}

try:
    this_file = __file__
except NameError:
    this_file = "intercom_server.py"

def main(argv):

    msg_handler = Message_Handler(MSG_TABLE)

    print "Entering receive loop."
    while True:
        msg_handler.receive()

if __name__ == "__main__":
    main(sys.argv[1:])
