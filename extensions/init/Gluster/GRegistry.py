from Gluster.GFrontEnd import dialog

state_dir = '/var/gluster'

def default_get (var):
    try:
        fname = open (state_dir + '/' + var, "r")
    except:
        return GetDefault (var)
    ret_var = fname.readline ().strip ()
    fname.close ()
    return ret_var

def default_set (var, value):
    fname = open (state_dir + '/' + var, "w")
    fname.write (value)
    fname.close ()

def default_default (var):
    return var

def hostname_default (var):
    return 'cluster'

def domain_default (var):
    return 'gluster.org'

def interface_default (var):
    return 'eth0'

def ipconfig_default (var):
    return "0.0.0.0/0"

def macs_get (var):
    try:
        macfile = open (state_dir + "/" + 'mac.list', "r")
    except:
        return 'Not Collected'
    count = len (macfile.readlines ())
    macfile.close ()
    return repr (count) + " entries"

def macs_set (var, val):
    return

def macs_default (var):
    return 'Not Collected'

_registry = {
    'interface' : { 'get' : default_get,
                    'set' : default_set,
                    'default': interface_default },
    'hostname' : { 'get': default_get,
                   'set': default_set,
                   'default': hostname_default },
    'domain' : { 'get' : default_get,
                 'set' : default_set,
                 'default' : domain_default },
    'ipconfig' : { 'get' : default_get,
                   'set' : default_set,
                   'default' : ipconfig_default },
    'macs' : { 'get' : macs_get,
               'set' : macs_set,
               'default' : macs_default }
    }


def GetValue (var):
    try:
        return _registry [var]['get'] (var)
    except KeyError:
        default_get (var)


def SetValue (var, val):
    try:
        _registry [var]['set'] (var, val)
    except KeyError:
        default_set (var, val)

def GetDefault (var):
    try:
        fn = _registry [var]['default']
    except:
        fn = default_default
    return fn (var)
