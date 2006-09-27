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

from Gluster import GArgs;
from Gluster.GFrontEnd import dialog

    
_Reg = {
  'bootmode' : [ ['--bootmode'] ,
                  'string',
                  'ask',
                  'usb|floppy|iso'
               ]
};

GArgs._G.AddRawOptConf (_Reg);
dlg = dialog.Dialog ()

def GetMode ():
    mode = GArgs._G.GetValue ('bootmode');

    if (mode == 'usb' or mode == 'floppy' or mode == 'iso'):
        return mode;
    else:
        (ret, reply) = dlg.menu ('Choose BootMan boot mode',
                                 choices=[ ('usb',
                                            'Create bootable USB disk'),
                                           ('floppy',
                                            'Create bootable Floppy disk'),
                                           ('iso',
                                            'Create bootable ISO image')]);
        if ret != 0:
            return None
    return reply;
