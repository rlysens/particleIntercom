import getopt, sys, subprocess, glob, pdb

def usage():
	print "build.py --device Intercom1/Intercom2/all [--flash]"

def build():
	cpp_files = ' '.join(glob.glob("*.cpp"))
	h_files = ' '.join(glob.glob("*.h"))
	subprocess.call("particle compile photon intercom.ino %s %s"%(cpp_files, h_files), shell=True)

def flash_dev(device):
    cpp_files = ' '.join(glob.glob("*.cpp"))
    h_files = ' '.join(glob.glob("*.h"))
    subprocess.call("particle flash %s intercom.ino %s %s"%(device, cpp_files, h_files), shell=True)

def genMessages():
	subprocess.call("cd lcm && python genMessages.py && cd ..", shell=True)

def main():
    try:
        opts, args = getopt.getopt(sys.argv[1:], "d:fh", ["device=", "flash", "help"])
    except getopt.GetoptError as err:
        # print help information and exit:
        print str(err)  # will print something like "option -a not recognized"
        usage()
        sys.exit(2)

    device = None
    flash = False

    for o, a in opts:
        if o in ("-f", "--flash"):
            flash = True
        elif o in ("-h", "--help"):
            usage()
            sys.exit()
        elif o in ("-d", "--device"):
            device = a
        else:
            assert False, "unhandled option"
    
    if (device is None) and flash:
    	usage()
    	sys.exit()

    genMessages()

    if flash:
        device_list = []
        if device == "all":
    	   device_list.append("Intercom1")
    	   device_list.append("Intercom2")
        else:
    	   device_list.append(device)

        for dd in device_list:
    	   flash_dev(dd)
    else:
        build()

if __name__ == "__main__":
    main()
