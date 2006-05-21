#!/usr/bin/python

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
