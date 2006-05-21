#!/usr/bin/python

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
