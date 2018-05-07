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
import boto3
from boto3.dynamodb.conditions import Key, Attr
from threading import Thread
from threading import Event
from messages import *
import decimal
import struct

HOST = ''
HOST_PORT = 50007

MAX_NUM_BUDDIES = 3
#ENCRYPTION_MODE = xtea.MODE_CBC
ENCRYPTION_MODE = xtea.MODE_ECB
#ENCRYPTION_MODE = xtea.MODE_NONE

PRINT_MSG_RX_TX = True

keyPressThreadToMainEvent = Event()
mainToKeyPressThreadEvent = Event()
quitEvent = Event()
intercomTable = None

NO_FILTER = 0
FILTER_ALL = -1

inputFilter = NO_FILTER
outputFilter = NO_FILTER

class IntercomDB:
	def __init__(self):
		self.dynamodb = boto3.resource('dynamodb', region_name='us-west-2')
		self.table = self.dynamodb.Table('Intercom_Table')
	
	def lookupIdByName(self, name):
		try:
			response = self.table.query(
				IndexName='intercom-name-index',
				KeyConditionExpression=Key('intercom_name').eq(name))
		except:
			return None
		
		if response.has_key('Items') and (len(response['Items']) > 0):
			return int(response['Items'][0]['intercom_id'])
		else:
			return None

	def lookupAttributes(self, intercom_id):
		"Returns following tuple: (name, secret_key, server, buddies), or raises IndexError."
		
		response = self.table.get_item(
			Key={
				'intercom_id': intercom_id
			}
		)
		
		if not response.has_key('Item'):
			raise IndexError()

		item = response['Item']
		#Create Intercom object from Item attributes
		name = item['intercom_name']
		secret_key = item['intercom_secret_key']
		server = item.get('server')
		if server is not None:
			server = int(server)
		buddies = [int(b) for b in item.get('intercom_buddies')]
		
		return (name, secret_key, server, buddies)

	def setServer(self, intercom_id, server):
		self.table.update_item(
    		Key={'intercom_id': intercom_id},
   			UpdateExpression="set server = :s",
    		ExpressionAttributeValues={':s': decimal.Decimal(server)},
    		ReturnValues="UPDATED_NEW")

intercomDB = IntercomDB()

class Intercom_LocalTables:
	def __init__(self):
		self.intercom_name_to_id_table = {}
		self.intercom_id_to_intercom_table = {}

	def getIdByName(self, name, lookupInDBifNotLocal=True):
		intercomId = self.intercom_name_to_id_table.get(name)
		if intercomId:
			return intercomId

		#ID not found locally. Check the database.
		if lookupInDBifNotLocal:
			return intercomDB.lookupIdByName(name)

		return None

	def getIntercomById(self, intercomId):
		return self.intercom_id_to_intercom_table.get(intercomId)

	def addIntercom(self, intercomId, intercom):
		self.intercom_id_to_intercom_table[intercomId] = intercom

intercomLocalTables = Intercom_LocalTables()

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
			inputFilter = intercomLocalTables.getIdByName(inputFilterStringRaw)
			if not inputFilter:
				print "Invalid Rx filter."

		if outputFilterStringRaw == 'all':
			outputFilter = NO_FILTER
		elif outputFilterStringRaw == 'none':
			outputFilter = FILTER_ALL
		else:
			outputFilterStringRaw += '\x00'*min(32,32-len(outputFilterStringRaw))
			outputFilter = intercomLocalTables.getIdByName(outputFilterStringRaw)
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
				name = intercomLocalTables.getIntercomById(destination_id).getName()
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

		intercom = intercomLocalTables.getIntercomById(source_id)
		if not intercom:
			#If the intercom object doesn't exist on this server yet, create a representation.
			#Note that this does not mean this intercom is considered 'local'. An intercom does not become
			#local until its address is set in the object. The i_am message does this.
			#This intercom is currently not known locally by this server.
			try:
				intercom = Intercom(source_id, self)
				intercomLocalTables.addIntercom(source_id, intercom)
			except IndexError:
				print "Intercom id %d unknown"%(source_id)
				return
			
		#Invoke the handler
		if ((inputFilter==NO_FILTER) or (inputFilter==source_id)):
			name = intercom.getName()
			
			if len(message_name_table.messageNameTable) > msgId:
				print "Rx Msg: %s(%d) Src: %s(%d)"%(message_name_table.messageNameTable[msgId], msgId, name, source_id)
			else:
				print "Rx Msg: XX(%d) Src: %s(%d)"%(msgId, name, source_id)
			
		fun, decrypt = self._msg_table[msgId]

		msg_payload = None        
		if decrypt:
			cryptoCodec = intercom.getDecoderCryptoCodec()
			msg_payload = cryptoCodec.decrypt(data[8:])
			if ENCRYPTION_MODE == xtea.MODE_CBC:
				cryptoCodec.IV = data[-8:]
		else:
			msg_payload = data[8:]

		if msg_payload:
			try: #A decode failure will trigger a ValueError exception
				fun(msg_payload, address, self, intercom)
			except ValueError as v:
				print "Can't decode msgId %d from %s"%(msgId, name)
				print v

class Intercom:
	def __init__(self, intercom_id, msg_handler):
		"May raise IndexError exception."
		self.msg_handler = msg_handler
		#This could raise an exception
		(self.name, self.keyString, self.serverAddress, buddies) = intercomDB.lookupAttributes(intercom_id)
		self.id = intercom_id
		self.address = None
		self.buddy_list = [0,0,0]
		self.next_buddy_idx = 0
		for buddy in buddies:
			self.setBuddy(buddy)

		self.next_buddy_idx = 0
		self.encCrypto = xtea.new(self.keyString, mode=ENCRYPTION_MODE, IV="\0"*8)
		self.decCrypto = xtea.new(self.keyString, mode=ENCRYPTION_MODE, IV="\0"*8)

	def getId(self):
		return self.id

	def getName(self):
		return self.name

	def getServerAddress(self):
		return self.serverAddress

	def setServerAddress(self, serverAddress):
		self.serverAddress = serverAddress

	def isLocal(self):
		return self.address != None

	def getAddress(self):
		return self.address

	def setAddress(self, address):
		self.address = address

	def getEncoderCryptoCodec(self):
		return self.encCrypto

	def getDecoderCryptoCodec(self):
		return self.decCrypto

	def sendTo(self, sender_id, msg, msgId, encrypt):
		if not self.address:
			print "sendTo %s: address unknown."%(self.name)
		elif sender_id in self.buddy_list:
			if encrypt:
				self.msg_handler.send(msg, msgId, self.address, self.id, sender_id, self.encCrypto)
			else:
				self.msg_handler.send(msg, msgId, self.address, self.id, sender_id, None)
		else:
			print "sendTo %s: sender %d not in buddy list."%(self.name, sender_id)
			print self.buddy_list

	def getBuddies(self):
		return self.buddy_list

	def setBuddy(self, buddy_id):
		if buddy_id not in self.buddy_list:
			self.buddy_list[self.next_buddy_idx] = buddy_id
			self.next_buddy_idx += 1
			if self.next_buddy_idx >= 3:
				self.next_buddy_idx =0 

def msg_i_am_handler(msg_data, address, msg_handler, intercom):
	i_am = i_am_t.i_am_t.decode(msg_data)
	intercom.setAddress(address)

	#Compare the stored server address with the message's server address
	#to find out if this intercom migrated here from another server,
	#in which case we should send a retire message to the previous server.
	intercomOldServerAddress = intercom.getServerAddress()
	if (intercomOldServerAddress != None) and (intercomOldServerAddress != i_am.srvr_addr):
		retire = retire_t.retire_t()
		retire.my_id = i_am.my_id
		data = retire.encode()
		cryptoCodec = intercom.getEncoderCryptoCodec()
		serverAddrString = socket.inet_ntoa(struct.pack('!I', i_am.srvr_addr))
		#Note that we're spoofing the source_id here.
		msg_handler.send(data, retire_t.retire_t.MSG_ID, (serverAddrString, HOST_PORT), 
			destination_id=i_am.my_id, source_id=i_am.my_id, cryptoCodec=cryptoCodec)
		
	intercom.setServerAddress(i_am.srvr_addr)
	intercomDB.setServer(i_am.my_id, i_am.srvr_addr)

def msg_retire_handler(msg_data, address, msg_handler, intercom):
	intercom.setAddress(None)
	intercom.setServerAddress(None)

def msg_keep_alive_handler(msg_data, address, msg_handler, intercom):
	keep_alive = keep_alive_t.keep_alive_t.decode(msg_data)
	buddy_intercom = intercomLocalTables.getIntercomById(keep_alive.destination_id)
	#If buddy intercom is local, we forward the request
	if buddy_intercom and buddy_intercom.isLocal():
		buddy_intercom.sendTo(keep_alive.source_id, msg_data, keep_alive_t.keep_alive_t.MSG_ID, True)
	else: #otherwise redirect
		keep_alive_resp = keep_alive_resp_t.keep_alive_resp_t()
		try:
			(name, secret_key, buddyServer, buddies) = intercomDB.lookupAttributes(keep_alive.destination_id)
		except IndexError:
			print "destination id %d unknown"%(keep_alive.destination_id)
			return

		if buddyServer:
			keep_alive_resp.redirect_addr = buddyServer
			data = keep_alive_resp.encode()
			cryptoCodec = intercom.getEncoderCryptoCodec()
			msg_handler.send(data, keep_alive_resp_t.keep_alive_resp_t.MSG_ID, 
				address, intercom.getId(), source_id=None, cryptoCodec=cryptoCodec)
		else:
			print "Buddy %d serverAddress unknown"%(keep_alive.destination_id)

def msg_comm_start_handler(msg_data, address, msg_handler, intercom):
	comm_start = comm_start_t.comm_start_t.decode(msg_data)
	buddy_intercom = intercomLocalTables.getIntercomById(comm_start.destination_id)
	if buddy_intercom:
		buddy_intercom.sendTo(comm_start.source_id, msg_data, 
			comm_start_t.comm_start_t.MSG_ID, True)
	else:
		print "Unknown destination %d"%(comm_start.destination_id)

def msg_comm_start_ack_handler(msg_data, address, msg_handler, intercom):
	comm_start_ack = comm_start_ack_t.comm_start_ack_t.decode(msg_data)
	buddy_intercom = intercomLocalTables.getIntercomById(comm_start_ack.destination_id)
	if buddy_intercom:
		buddy_intercom.sendTo(comm_start_ack.source_id, msg_data, 
			comm_start_ack_t.comm_start_ack_t.MSG_ID, True)
	else:
		print "Unknown destination %d"%(comm_start_ack.destination_id)

def msg_comm_stop_handler(msg_data, address, msg_handler, intercom):
	comm_stop = comm_stop_t.comm_stop_t.decode(msg_data)
	buddy_intercom = intercomLocalTables.getIntercomById(comm_stop.destination_id)
	if buddy_intercom:
		buddy_intercom.sendTo(comm_stop.source_id, msg_data, 
			comm_stop_t.comm_stop_t.MSG_ID, True)
	else:
		print "Unknown destination %d"%(comm_stop.destination_id)

def msg_comm_stop_ack_handler(msg_data, address, msg_handler, intercom):
	comm_stop_ack = comm_stop_ack_t.comm_stop_ack_t.decode(msg_data)
	buddy_intercom = intercomLocalTables.getIntercomById(comm_stop_ack.destination_id)
	if buddy_intercom:
		buddy_intercom.sendTo(comm_stop_ack.source_id, msg_data, 
			comm_stop_ack_t.comm_stop_ack_t.MSG_ID, True)
	else:
		print "Unknown destination %d"%(comm_stop_ack.destination_id)

def msg_voice_data_handler(msg_data, address, msg_handler, intercom):
	voice_data = voice_data_t.voice_data_t.decode(msg_data)
	buddy_intercom = intercomLocalTables.getIntercomById(voice_data.destination_id)
	if buddy_intercom:
		buddy_intercom.sendTo(voice_data.source_id, msg_data, 
			voice_data_t.voice_data_t.MSG_ID, True)

def set_buddy_handler(msg_data, address, msg_handler, intercom):
	set_buddy = set_buddy_t.set_buddy_t.decode(msg_data)

	if intercomLocalTables.getIntercomById(set_buddy.my_id):
		intercom.setBuddy(set_buddy.buddy_id)
		set_buddy_ack = set_buddy_ack_t.set_buddy_ack_t()
		set_buddy_ack.buddy_id = set_buddy.buddy_id
		intercom.sendTo(0, set_buddy_ack.encode(),
			set_buddy_ack_t.set_buddy_ack_t.MSG_ID, True)
	else:
		print "add_buddy.my_id unknown %d"%(add_buddy.buddy_id)

MSG_TABLE = {
	voice_data_t.voice_data_t.MSG_ID : (msg_voice_data_handler, True),
	i_am_t.i_am_t.MSG_ID : (msg_i_am_handler, True),
	retire_t.retire_t.MSG_ID : (msg_retire_handler, True),
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
	global intercomTable

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
