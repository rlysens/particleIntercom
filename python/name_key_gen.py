
import random
from namegen.namegen import NameGen
import os
import string
import shutil
import boto3
from boto3.dynamodb.conditions import Key, Attr
import uuid

MAX_NAME_LEN = 19

class NameKeyGenerator:
	def __init__(self):
		self.dynamodb = boto3.resource('dynamodb', region_name='us-west-2')
		self.table = self.dynamodb.Table('Intercom_Table')
		#get all available language files
		self.languageFiles = os.listdir('./namegen/Languages')
		self.rnd = random.SystemRandom()

	def commit(self, intercom_id, intercom_name, intercom_secret_key):
		"Commit id, name and key to database. Returns True on success."
		try:
			self.table.put_item(
				Item={
				'intercom_id': intercom_id,
				'intercom_name': intercom_name,
				'intercom_secret_key': intercom_secret_key,
				'intercom_buddies' : []
				}
			)
		except:
			return False

		return True

	def lookup_item_by_id(self, candidate_id):
		"Returns dictionary, or None."
		try:
			response = self.table.get_item(
				Key={
					'intercom_id': candidate_id
				}
			)
		except:
			return None

		if response.has_key('Item'):
			return response['Item']
		else:
			return None

	def lookup_item_by_name(self, name):
		"Returns dictionary, or None."
		try:
			response = self.table.query(
				IndexName='intercom-name-index',
				KeyConditionExpression=Key('intercom_name').eq(name)
			)
		except:
			return None
		
		if response.has_key('Items') and (len(response['Items']) > 0):
			return response['Items'][0]
		else:
			return None
	
	def gen_name_key_id(self):
		name = None
		while not name:
			languageFilename = self.rnd.choice(self.languageFiles)
			#load generator data from language file
			generator = NameGen('./namegen/Languages/' + languageFilename)
			candidate = generator.gen_word()
			if (len(candidate)<=MAX_NAME_LEN) and (not self.lookup_item_by_name(candidate)):
				name = candidate

		key_string = ''.join([self.rnd.choice(string.ascii_letters+string.digits) for _ in range(16)])

		intercom_id = None
		while not intercom_id:
			candidate = uuid.uuid4().int & 0xffffffff
			if self.lookup_item_by_id(candidate) == None:
				intercom_id = candidate

		return (intercom_id, name, key_string)