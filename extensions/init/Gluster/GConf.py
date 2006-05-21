from Gluster.GFrontEnd import UI;
import ConfigParser, os, sys;

def GetValue (var):
    """
    Get Value of a variable
    """
    try:
        val = Load (var);
    except KeyError:
        val = None;
    return val;


def Load (Key):
    """
    Load from config file
    """

    AssertSanity();

    config = ConfigParser.ConfigParser ();
    config.read ('/etc/gluster/gluster.conf')
    try:
        return config.get ('GConf', Key);
    except ConfigParser.Error:
        return None;


def AssertSanity ():

    if (not os.access ('/etc/gluster/gluster.conf', os.W_OK)):
        try:
            file = open ('/etc/gluster/gluster.conf', "w+");
            file.write ("[GConf]\n");
            file.close ();
        except IOError:
            return -1;

    config = ConfigParser.ConfigParser ();
    try:
        config.read ('/etc/gluster/gluster.conf')
    except ConfigParser.Error:
        print "GConf not in sane state; exiting";
        os.abort ();
        
    if (not config.has_section ('GConf')):
        print "GConf is not in sane state. Exiting";
        os.abort ();
        
