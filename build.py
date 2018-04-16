import getopt, sys, subprocess, glob, pdb, os
import json

def usage():
	print "build.py --device device_id/all [--flash]"

def build():
    os.chdir('firmware')
    cpp_files = ' '.join(glob.glob("*.cpp"))
    h_files = ' '.join(glob.glob("*.h"))
    subprocess.call("particle compile photon %s %s"%(cpp_files, h_files), shell=True)
    os.chdir('..')

def flash_dev(device):
    os.chdir('firmware')
    cpp_files = ' '.join(glob.glob("*.cpp"))
    h_files = ' '.join(glob.glob("*.h"))
    subprocess.call("particle flash %s %s %s"%(device, cpp_files, h_files), shell=True)
    os.chdir('..')

def genMessages():
	subprocess.call("cd lcm && python genMessages.py", shell=True)

def main():
    try:
        opts, args = getopt.getopt(sys.argv[1:], "d:fh", ["device=", "flash", "help"])
    except getopt.GetoptError as err:
        # print help information and exit:
        print str(err)  # will print something like "option -a not recognized"
        usage()
        sys.exit(2)

    device = "Intercom1"
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
           device_list.append("Intercom3")
        else:
    	   device_list.append(device)

        for dd in device_list:
    	   flash_dev(dd)
    else:
        build()

if __name__ == "__main__":
    main()
