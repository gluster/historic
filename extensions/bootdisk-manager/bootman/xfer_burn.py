#!/usr/bin/python

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
