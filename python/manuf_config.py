import particle

import json
import time
import pdb
import getopt, sys
import name_key_gen

def loadCredentials():
	credentialsJsonFile = open('credentials.json', 'r')
	credentials = json.load(credentialsJsonFile)
	credentialsJsonFile.close()
	return credentials

def createParticleCloud(credentials):
	return particle.Particle(
		credentials['particle_username'],
		credentials['particle_password'])

def enterListeningMode():
	print "Please re-enter listening mode. Press ENTER to continue."
	raw_input()

def manufConfig(skipClaim, skipFlash, imageFilename, deviceName):
	credentials = loadCredentials()
	particleLocal = particle.Local(credentials['ssid'], 
		credentials['wifi_password'], credentials['wifi_encryption'])

	print "Retrieving connected ports..."
	serialPorts = particleLocal.get_connected_ports()
	if len(serialPorts)==0:
		print "No connected ports found. Aborting..."
		return

	serialPort = serialPorts[0]
	print serialPort

	nameKeyGen = name_key_gen.NameKeyGenerator()
	#Only generate name if no name is given
	if deviceName is None:
		print "Generating name and key..."
		name, keyString = nameKeyGen.gen_name_and_key()
		print name
		nameKeyGen.commit()
	else:
		name = deviceName
		keyString = nameKeyGen.lookup_key(name)
		if not keyString:
			print "Device name not found. Aborting..."
			return

	print "Logging into cloud..."
	particleCloud = createParticleCloud(credentials)

	print "Identifying device..."
	particleId = particleLocal.identify_device(serialPort)
	print "particleId: %s"%(particleId)

	if not skipFlash:
		print "Flashing device..."
		res = particleLocal.flash(imageFilename)
		time.sleep(20)
		enterListeningMode()

	print "Setting WiFi..."
	particleLocal.set_wifi(port=serialPort)
	time.sleep(20)

	if not skipClaim:
		print "Claiming device..."
		particleCloud.claim_device(particleId)

	print "Erasing..."
	callFunctionSuccess = False
	attemptCount = 0
	while not callFunctionSuccess:
		callFunctionSuccess = particleCloud.call_function(particleId, "erase", "")
		attemptCount += 1
		if attemptCount >= 5:
			print "Erase failed. Aborting..."
			return
		time.sleep(5)
		print ".",

	print "Setting name (%s) and key (%s)..."%(name, keyString)
	callFunctionSuccess = False
	attemptCount = 0
	while not callFunctionSuccess:
		callFunctionSuccess = particleCloud.call_function(particleId, "my_name", name)
		attemptCount += 1
		if attemptCount >= 5:
			print "Setting name failed. Aborting..."
			return
		time.sleep(5)
		print ".",

	callFunctionSuccess = False
	attemptCount = 0
	while not callFunctionSuccess:	
		callFunctionSuccess = particleCloud.call_function(particleId, "set_key", keyString)
		attemptCount += 1
		if attemptCount >= 5:
			print "Setting key failed. Aborting..."
			return
		time.sleep(5)
		print ".",

	print ""

	print "Setting server name (%s)..."%(credentials['server_name'])
	callFunctionSuccess = False
	attemptCount = 0
	while not callFunctionSuccess:
		callFunctionSuccess = particleCloud.call_function(particleId, "srvr_name", credentials['server_name'])
		attemptCount += 1
		if attemptCount >= 5:
			print "Setting server failed. Aborting..."
			return
		time.sleep(5)
		print ".",

	if not skipClaim:
		print "Unclaiming device..."
		particleCloud.unclaim_device(particleId)

def usage():
	print """
manuf_config.py [-h,--help] [-c,--skip_claim] [-f,--skip_flash] [-i,--image_filename <image_filename>] [-n,--device_name <name>]
	Finds and claims connected device, sets up Wifi, flashes reference fw image and configures name, secret key and server.
	"""	

if __name__ == "__main__":
	try:
		opts, args = getopt.getopt(sys.argv[1:], "hcfi:n:", ["help", "skip_claim", "skip_flash", "image_filename=", "device_name="])
   	except getopt.GetoptError as err:
    	# print help information and exit:
		print str(err)  # will print something like "option -a not recognized"
		usage()
		sys.exit(2)
    
	imageFilename = 'photon_firmware_ref.bin'
	skipClaim = False
	skipFlash = False
	deviceName = None

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
		elif o in ("-n", "--device_name"):
			deviceName = a
    
	manufConfig(skipClaim, skipFlash, imageFilename, deviceName)
	print "Done."
