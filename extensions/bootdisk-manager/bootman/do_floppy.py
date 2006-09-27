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
import os;
import sys;
import xfer_burn;

_Reg = {
  'floppydev' : [ ['--floppy-img'] ,
                  'string',
                  'ask',
                  'Where to create floppy image eg. /dev/fd0 or /tmp/boot.img'
               ]
};

GArgs._G.AddRawOptConf (_Reg);
dlg = dialog.Dialog ()

def build (tmpl):
    skel = os.path.dirname (sys.argv[0]) + "/template/boot-template.img.gz";
    tmp = Dir.Get ().Name ()
    os.system ("cat " + skel + "| gzip -d ->" + tmp + "/boot.img");
    skel = tmp + "/boot.img";
    os.mkdir (tmp + "/fmount");
    mpoint = tmp + "/fmount";
    os.system ("mount -o loop " + skel + " " + mpoint);
    os.system ("cp -a " + tmpl + "/* " + mpoint);
    os.system ("umount " + mpoint);
    os.system ("rm -rf " + mpoint);

    img = GArgs._G.GetValue ('floppydev');

    if (img == 'ask'):
        while True:
            (ret, choice) = dlg.menu ("Choose operation",
                                      choices=[('IMG',
                                                "Create bootable floppy")])

            if ret != 0:
                break

            (ret, choice) = dlg.inputbox ('Img location e.g. /dev/fd0');
            if ret != 0:
                break
            xfer_burn.xfer_img (skel, choice);
    else:
        xfer_burn.xfer_img (skel, img);

    os.system ("rm -rf " + skel);
    return;
