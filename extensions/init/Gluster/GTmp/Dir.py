from Gluster import GRegistry;
import os;
import tempfile;

tmp_dir = {};

class TmpDir:
    my_tmp = None;
    sticky = False
    tmp_base = '/tmp'
    def __init__ (self):
	self.my_tmp = tempfile.mkdtemp (suffix = '.gtmp',
                                        prefix= 'gluster-' + repr (os.getpid()) + '-',
                                        dir = self.tmp_base);

    def __del__ (self):
        if not self.sticky:
            rm_rf = 'rm -rf ' + repr (self.my_tmp);
            os.system (rm_rf);
	return True

    def SetSticky (self, bool):
        self.sticky = bool

    def Name (self):
        return self.my_tmp;

tmp_dir[0] = TmpDir ();
	    
def Get ():
    return tmp_dir[0];

def Create ():
    """
    Create temporary directory
    """



