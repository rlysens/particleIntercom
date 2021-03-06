import os
import sys
import re

def gen():
	
	nameTable = {}
	for filename in os.listdir("."):
		if filename.endswith(".h"):
			inFile = filename

			with open(inFile) as f:
				for line in f:
					#define SET_BUDDY_T_MSG_ID 6
					m=re.search("(\w+)_T_MSG_ID\s+(\d+)", line)
					if m:
						name = m.group(1)
						id = int(m.group(2))
						nameTable[id] = name

	numElems=max(nameTable.keys())+1

	coutFilename = os.path.join("message_name_table.h")
	poutFilename = os.path.join("message_name_table.py")

	coutFileFd = open(coutFilename, 'w')
	poutFileFd = open(poutFilename, 'w')
	
	coutFileFd.write("#ifndef MESSAGE_NAME_TABLE_H\n")
	coutFileFd.write("#define MESSAGE_NAME_TABLE_H\n")

	coutFileFd.write("/*This file is autogenerated. Do not edit.*/\n")
	poutFileFd.write("#This file is autogenerated. Do not edit.\n")

	coutFileFd.write("static const char* messageNameTable[]={\n")
	
	poutFileFd.write("messageNameTable = [\n");
	
	for ii in range(numElems):
		if nameTable.has_key(ii):
			coutFileFd.write('"%s", /*%d*/\n'%(nameTable[ii],ii))
			poutFileFd.write('\t"%s", #%d\n'%(nameTable[ii],ii))
		else:
			coutFileFd.write('"",\n')
			poutFileFd.write('\t"",\n')

	coutFileFd.write("};\n")
	coutFileFd.write("#endif /*MESSAGE_NAME_TABLE_H*/")

	poutFileFd.write("]\n")

	coutFileFd.close()
	poutFileFd.close()

if __name__ == "__main__":
    gen()