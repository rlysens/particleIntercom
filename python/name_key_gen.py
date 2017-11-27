
import json
import random
from namegen.namegen import NameGen
import os
import string
import shutil

NAME_KEYS_FILENAME = 'name_key.json'
MAX_NAME_LEN = 19

class NameKeyGenerator:
	def __init__(self):
		nameKeyJsonFile = open(NAME_KEYS_FILENAME,'r')
		self.nameKeys = json.load(nameKeyJsonFile)
		nameKeyJsonFile.close()
		#get all available language files
		self.languageFiles = os.listdir('./namegen/Languages')
		self.rnd = random.SystemRandom()

	def commit(self):
		"Commit NameKey dictionary to JSON file."
		shutil.copyfile(NAME_KEYS_FILENAME,NAME_KEYS_FILENAME+".bak")
		nameKeyJsonFile = open(NAME_KEYS_FILENAME,'w')
		json.dump(self.nameKeys, nameKeyJsonFile, indent=4, separators=(',', ': '))
		nameKeyJsonFile.close()

	def gen_name_and_key(self):
		name = None
		while not name:
			languageFilename = self.rnd.choice(self.languageFiles)
			#load generator data from language file
			generator = NameGen('./namegen/Languages/' + languageFilename)
			candidate = generator.gen_word()
			if (len(candidate)<=MAX_NAME_LEN) and (not self.nameKeys.has_key(candidate)):
				name = candidate

		keyString = ''.join([self.rnd.choice(string.ascii_letters+string.digits) for _ in range(16)])
		
		self.nameKeys[name] = keyString

		return (name, keyString)