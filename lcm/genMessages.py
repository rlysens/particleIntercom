import os
import sys
import re

def main(argv):
	os.system("lcm-gen -c --c-no-pubsub messages.lcm")



	for filename in os.listdir("."):
		if filename.endswith(".h"):
			inFile = filename
			outFile = os.path.join("../", filename)

			outFileFd = open(outFile, 'w')

			with open(inFile) as f:
				for line in f:
					repLine = re.sub('#include <lcm/lcm_coretypes.h>', '#include "lcm_coretypes.h"', line)
					outFileFd.write(repLine)

	os.system("copy *.c ..\*.cpp")

if __name__ == "__main__":
    main(sys.argv[1:])