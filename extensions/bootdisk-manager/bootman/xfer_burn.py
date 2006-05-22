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
import os;

dlg = dialog.Dialog ()
def xfer_img (src, dest = ''):

    while True:
        if (not dest == ''):
            break;
        (ret, dest) = dlg.inputbox ("Enter path to transfer "  + src,
                                    init=dest);
        if ret != 0:
            return

        if (dest == ''):
            dlg.msgbox ("Please enter path");
        else:
            break

    dlg.infobox ("Transferring " + src + " to " + 
            dest + "\n\n\tPlease Wait...\n");

    ret = os.system ("dd if=" + src + " of=" + dest) / 256;

    if (ret == 0):
        dlg.msgbox ("Transferred successfully");
    else:
        dlg.msgbox ("Transfer failed!!\n");

    return

def cdrecord_iso (iso, spec = 'ATA:1,1,0'):
    
    while True:
	if (not spec == ''):
            break
        (ret,spec) = dlg.inputbox ("Enter device specification." +
                "\nEg. ATA:1,1,0 (refer cdrecord -scanbus)",
                 init=spec);
        if ret != 0:
            return
        if (spec == ''):
            dlg.msgbox ("Please enter cdrecord device spec\n(refer cdrecord -scanbus)");
        else:
            break

    dlg.infobox ("Burning ISO to CD ...");

    ret = os.system ("cdrecord -dev=" + spec + " -s multi -data " + iso) / 256;
 
    if (ret == 0):
        dlg.msgbox ("Transferred successfully");
    else:
        dlg.msgbox ("Transfer failed!!\n");
    return;

def xfer_makebootfat (src, dst):
    ret = os.system ("makebootfat -o " + dst + " -Y -Z -m " + src + "/mbr.bin -b " + src + "/ldlinux.bss -c " + src + "/ldlinux.sys " + src) / 265;
    if (ret == 0):
        dlg.msgbox ("Transferred successfully");
    else:
        dlg.msgbox ("Transfer failed!!\n");
    return
