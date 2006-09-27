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
import xfer_burn;

_Reg = {
  'usbdev' : [ ['--usb-img'] ,
                  'string',
                  'ask',
                  'Where to create usb image eg. /dev/sda'
               ]
};

GArgs._G.AddRawOptConf (_Reg);
dlg = dialog.Dialog ()

def build (tmpl):

    img = GArgs._G.GetValue ('usbdev');

    if (img == 'ask'):
        while True:
            (ret, choice) = dlg.menu ("Choose operation",
                                      choices=[('IMG',"Create bootable usb")])
            if ret != 0:
                return

            (ret, choice) = dlg.inputbox ('Img location e.g. /dev/sda');
            xfer_burn.xfer_makebootfat (tmpl, choice);
    else:
        xfer_burn.xfer_makebootfat (tmpl, img);

    return;
