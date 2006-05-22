#!/usr/bin/python
##
## Copyright (C) 2006 Z RESEARCH Inc. <http://www.zresearch.com>
##  
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##  
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##  
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
##  
##

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
