import string;
import os;

def items ():
    path_var = '/usr/share/gluster/extensions';
    
    ext_dirs = string.split (path_var, ':');
    ret_list = [];
    for dir in ext_dirs:
        dir_stuff = os.listdir (dir);
	for stuff in dir_stuff:
            if (os.access (dir + "/" + stuff + "/" + stuff, os.X_OK ) == True):
                ret_list.append(stuff);
    return ret_list;

def run (ext, args=""):
    path_var = '/usr/share/gluster/extensions';

    ext_dirs = string.split (path_var, ':');
    for dir in ext_dirs:
        os.system ("chmod +x " + dir + "/" + ext + "/" + ext)
        if (os.access (dir + "/" + ext + "/" + ext, os.X_OK) == True):
            ret = os.system ("exec " + dir + "/" + ext + "/" + ext + " " + args) / 256;
            return ret
