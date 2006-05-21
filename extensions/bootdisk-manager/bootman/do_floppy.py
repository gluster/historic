#!/usr/bin/python

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
