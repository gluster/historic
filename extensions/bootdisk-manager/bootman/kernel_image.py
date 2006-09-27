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
from Gluster import GArgs
from Gluster.GTmp import Dir

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

dlg = dialog.Dialog ()
_Reg = {
'kernel-bzimage' : [ ['--kernel-bzimage'],
                      'string',
                      'ask',
                      'Kernel bzimage for the kernel-image bootdisk'],
'kernel-initrd' :  [ ['--kernel-initrd'],
                      'string',
                      'ask',
                      'Kernel initrd for the kernel-image bootdisk']
};

GArgs._G.AddRawOptConf (_Reg);

prev_initrd = ['/tftpboot/initrd.slave.gz']
prev_bzimage = ['/tftpboot/linux.slave']


def plugme():
    tar_path = (os.path.dirname (sys.argv[0]) + "/template/kernel-image.tar.bz2")

    bmode = mode.GetMode ()
    if bmode is None:
        return
    tmp_dir = Dir.Get ()
    os.system ("tar -C %s -xjf %s" % (tmp_dir.Name (), tar_path))

# sneek in a new kernel and initrd
    bzimage = GArgs._G.GetValue ('kernel-bzimage')
    if (bzimage == 'ask'):
        (ret, bzimage) = dlg.inputbox ("Enter path to bzImage",
                                       init=prev_bzimage[0])
        if ret != 0:
            return
    initrd = GArgs._G.GetValue ('kernel-initrd')
    if (initrd == 'ask'):
        (ret, initrd) = dlg.inputbox ("Enter path to initrd",
                                              init=prev_initrd[0])
        if ret != 0:
            return

    prev_bzimage[0] = bzimage
    prev_initrd[0] = initrd

    os.system ("cp -a " + bzimage + " " + tmp_dir.Name () + "/kernel-image/bzImage")
    os.system ("cp -a " + initrd + " " + tmp_dir.Name () + "/kernel-image/initrd.img")
# done

    _build_reg[bmode](tmp_dir.Name () + "/kernel-image")

    os.system ("rm -rf " + tmp_dir.Name () + "/kernel-image")

    return
