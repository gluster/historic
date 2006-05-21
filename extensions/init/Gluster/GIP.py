import os, string

def get_ifaces_list ():
    hnd = os.popen ("ip link list " +
              "| " +
              "sed -n -e 's/^[0-9]*: \\([^:]*\\):.*$/\\1/p'", "r")
    ret = map(string.strip, hnd.readlines ())
    hnd.close ()
    return ret

def get_iface_status (iface):
    ret = os.system ("ip link list dev " +
                     iface +
                     " | grep -q UP") / 256;

    if ret == 0:
        return 'UP'
    else:
        return 'DOWN'

def subnet_to_prefix (dotted):
    return

def prefix_to_subnet (number):
    return

def get_subnet_size (dotted):
    return

def get_prefix_size (number):
    return
