import particle

import json
import time
import pdb
import getopt, sys
import name_key_gen

def manuf_config(skipClaim, skipFlash, imageFilename):
	credentialsJsonFile = open('credentials.json', 'r')
	credentials = json.load(credentialsJsonFile)
	credentialsJsonFile.close()
	particleLocal = particle.Local(credentials['ssid'], 
		credentials['wifi_password'], credentials['wifi_encryption'])

	print "Retrieving connected ports..."
	serialPorts = particleLocal.get_connected_ports()
	if len(serialPorts) > 2:
		serialPorts = serialPorts[0:2]

	print "Generating names and keys..."
	nameKeyGen = name_key_gen.NameKeyGenerator()
	nameKeys = {}

	for serialPort in serialPorts:
		nameKeys[serialPort] = nameKeyGen.gen_name_and_key()
		print "%s"%(nameKeys[serialPort][0])

	nameKeyGen.commit()
	

	print "Logging into cloud..."
	particleCloud = particle.Particle(
			credentials['particle_username'],
			credentials['particle_password'])
		
	for ii in range(len(serialPorts)):
		port = serialPorts[ii]
		if (len(serialPorts)==2):
			otherPort = serialPorts[(ii+1)%2]
		else:
			otherPort = None

		print "Identifying device..."
		particleId = particleLocal.identify_device(port)
		print "Setting WiFi..."
		particleLocal.set_wifi(port)
		time.sleep(20)
		if not skipClaim:
			print "Claiming device..."
			particleCloud.claim_device(particleId)
		
		if not skipFlash:
			print "Flashing device..."
			res = particleCloud.flash(particleId, imageFilename)
			print res
			time.sleep(20)

		name, keyString = nameKeys[port]
		
		print "Erasing..."

		particleCloud.call_function(particleId, "erase", "")
		time.sleep(5)

		print "Setting name (%s) and key (%s)..."%(name, keyString)
		callFunctionSuccess = False
		while not callFunctionSuccess:
			callFunctionSuccess = particleCloud.call_function(particleId, "my_name", name)
			print ".",
		callFunctionSuccess = False
		while not callFunctionSuccess:	
			callFunctionSuccess = particleCloud.call_function(particleId, "set_key", keyString)
			print ".",

		print ""

		if otherPort:
			buddyName, buddyKeyString = nameKeys[otherPort]
			print "Setting buddy name: %s"%(buddyName)
			particleCloud.call_function(particleId, "buddy_0_name", buddyName)

		if not skipClaim:
			print "Unclaiming device..."
			particleCloud.unclaim_device(particleId)

def usage():
	print """
manuf_config.py [-h,--help] [-c,--skip_claim] [-f,--skip_flash] [-i,--image_filename <image_filename>]
	Finds and claims connected devices, sets up Wifi, flashes reference fw image and configures name, buddy name and secret key.
	"""	

if __name__ == "__main__":
	try:
		opts, args = getopt.getopt(sys.argv[1:], "hcfi:", ["help", "skip_claim", "skip_flash", "image_filename="])
   	except getopt.GetoptError as err:
    	# print help information and exit:
		print str(err)  # will print something like "option -a not recognized"
		usage()
		sys.exit(2)
    
	imageFilename = 'photon_firmware_ref.bin'
	skipClaim = False
	skipFlash = False

	for o, a in opts:
		if o in ("-c", "--skip_claim"):
			skipClaim = True
		elif o in ("-f", "--skip_flash"):
			skipFlash = True
		elif o in ("-h", "--help"):
			usage()
			sys.exit()
		elif o in ("-i", "--image_filename"):
			imageFilename = a 
    
	manuf_config(skipClaim, skipFlash, imageFilename)
	print "Done."
