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

HOST = ''
HOST_PORT = 50007

try:
    this_file = __file__
except NameError:
    this_file = "udp_test_server.py"

class Message_Handler:
    def send(self, message):

    def receive(self):
        data, address = self.sock.recvfrom(1024)
        if len(data) < 4:
            print "Msg. too short."
            return False

        msg_id = ord(data[0])+(ord(data[1])<<8)+(ord(data[2])<<16)+ord(data[3]<<24)
        
    def register_handler(self, handler, msg_id):

    def __init__(self):
        self._sock socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._sock.bind((HOST, HOST_PORT))

def main(argv):

    
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    try:
        opts, args = getopt.getopt(argv, "hp:", ["help", "port="])
    except getopt.GetoptError:
        # print help information and exit:
        usage()
        sys.exit(2)

    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
            sys.exit()
        if o in ("-p", "--port"):
            print "port: %s => %s"%(PORT, a)
            PORT = string.atoi(a)

    try:
        s.bind((HOST, HOST_PORT))
    except socket.error, msg:
        sys.exit('Error binding UDP server to port %d : %s' % (PORT, msg))
            
    count = 0

    while 1:
        data, address = s.recvfrom(1024)
        #print address
        if data:
            print "%d [R%d]"%(count,len(data))
            count += 1
            s.sendto(data, address)
            #print "[S%d]"%(len(data))

    s.close()


def usage():
    print "Report Server Usage:"
    print
    print "Options:"
    print "  -n or --no_echo    Optional.  When given, no data will be echoed."
    print "                     Data-echoing is enabled on default."
    print "  -h or --help       Display this help Intercom_Message and exit."
    print "  -p or --port       Change the port that the report server binds to."
    print

if __name__ == "__main__":
    main(sys.argv[1:])
