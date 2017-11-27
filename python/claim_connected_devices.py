import particle
import os
import random
from namegen.namegen import NameGen
import json
import time
import pdb

def gen_name_and_key():
	nameKeyJsonFile = open('name_key.json','r')
	nameKeys = json.load(nameKeyJsonFile)
	nameKeyJsonFile.close()

	#get all available language files
	languageFiles = os.listdir('./namegen/Languages')

	rnd = random.SystemRandom()
	name = None

	while not name:
		languageFilename = rnd.choice(languageFiles)
		#load generator data from language file
		generator = NameGen('./namegen/Languages/' + languageFilename)
		candidate = generator.gen_word()
		if not nameKeys.has_key(candidate):
			name = candidate

	key = [rnd.randint(0,127) for _ in range(16)]
	keyString = ''.join([chr(x) for x in key])
	nameKeys[name] = keyString

	nameKeyJsonFile = open('name_key.json','w')
	json.dump(nameKeys, nameKeyJsonFile)
	nameKeyJsonFile.close()

	return (name, keyString)

def claimConnectedDevices():
	credentialsJsonFile = open('credentials.json', 'r')
	credentials = json.load(credentialsJsonFile)
	credentialsJsonFile.close()
	particleLocal = particle.Local(credentials['ssid'], 
		credentials['wifi_password'], credentials['wifi_encryption'])

	print "Retrieving connected ports..."
	serialPorts = particleLocal.get_connected_ports()
	assert len(serialPorts) >= 2
	if len(serialPorts) > 2:
		serialPorts = serialPorts[0:2]

	print "Logging into cloud..."
	particleCloud = particle.Particle(
			credentials['particle_username'],
			credentials['particle_password'])
		
	for ii in range(len(serialPorts)):
		port = serialPorts[ii]
		otherPort = serialPorts[(ii+1)%2]
		print "Identifying device..."
		particleId = particleLocal.identify_device(port)
		print "Setting WiFi..."
		particleLocal.set_wifi(port)
		time.sleep(20)
		print "Claiming device..."
		particleCloud.claim_device(particleId)
		
if __name__ == "__main__":
    claimConnectedDevices()
    print "Done."
