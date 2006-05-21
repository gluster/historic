#!/usr/bin/python

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
