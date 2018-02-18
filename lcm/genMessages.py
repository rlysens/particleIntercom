import os
import sys
import re
import genMessageNameTables

def main(argv):
	os.system("lcm-gen -c --c-no-pubsub messages.lcm")

	genMessageNameTables.gen()
	
	for filename in os.listdir("."):
		if filename.endswith(".h"):
			inFile = filename
			outFile = os.path.join("../firmware/", filename)

			outFileFd = open(outFile, 'w')

			with open(inFile) as f:
				for line in f:
					repLine = re.sub('#include <lcm/lcm_coretypes.h>', '#include "lcm_coretypes.h"', line)
					outFileFd.write(repLine)

	os.system("copy *.c ..\\firmware\\*.cpp")
	os.system("lcm-gen -p messages.lcm")
	os.system("copy *.py ..\\python\\messages")

if __name__ == "__main__":
    main(sys.argv[1:])