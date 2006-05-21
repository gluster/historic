#!/usr/bin/python

from Gluster.GFrontEnd import dialog
from Gluster import GArgs;
from Gluster.GTmp import Dir;

import sys;
import os;

import mode;
import do_floppy;
import do_iso;
import do_usb;

_build_reg = {
    'usb' : do_usb.build,
    'iso' : do_iso.build,
    'floppy' : do_floppy.build
};

def plugme():
    tar = os.path.dirname (sys.argv[0]) + "/template/pxe-image.tar.bz2"

    bmode = mode.GetMode ();
    if bmode is None:
        return
    tmp_dir = Dir.Get ();

    os.system ("tar -C %s -xjf %s" % (tmp_dir.Name (), tar))
    
    _build_reg[bmode](tmp_dir.Name () + "/pxe-image");
    
    os.system ("rm -rf " + tmp_dir.Name () + "/pxe-image");
    return;
