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
import xfer_burn;

_Reg = {
  'isodev' : [ ['--iso-img'] ,
                  'string',
                  'ask',
                  'Where to create iso image eg. ATA:1,1,0'
               ]
};

GArgs._G.AddRawOptConf (_Reg);
dlg = dialog.Dialog ()

def iso_build (src, dst):
    xfer_burn.cdrecord_iso (src, dst);
    return;


def build (tmpl):

    tmp = Dir.Get (). Name ();
    img = GArgs._G.GetValue ('isodev');

    os.system ("mkisofs -R -J -o " + tmp + "/glboot.iso -b isolinux.bin -c boot.cat -no-emul-boot -boot-load-size 4 -boot-info-table " + tmpl);

    if (img == 'ask'):
        while True:
            (ret, choice) = dlg.menu ("Choose operation",
                               choices=[('burn',"Create bootable CD")])
            if ret != 0:
                break
            (ret, choice) = dlg.inputbox ('Img location e.g. ATA:1,1,0');
            if ret != 0:
                break
            iso_build (tmpl + "/glboot.iso", choice);
    else:
        iso_build (tmpl + "/glboot.iso", choice);

    os.system ("rm -rf " + tmp + "/glboot.iso");

    return;
