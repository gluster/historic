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

import os, string, sys
from Gluster.GTmp import Dir

def uniq (orig_list):
    uniq_list = []
    for i in orig_list:
        if not i in uniq_list:
            uniq_list.append (i)
    return uniq_list

def get_mpoint (part):
    hnd = os.popen ("mount | grep '^ *%s ' | awk '{print $3}'" % part)
    line = hnd.readline ().strip ()
    hnd.close ()
    if line:
        return (False, line)
    else:
        tmpdir = Dir.TmpDir ()
        tmpdir.SetSticky (True)
        ret = os.system ("mount %s %s 2>/dev/null" % (part, tmpdir.Name ()))
        if ret:
            os.system ('rm -rf %s' % tmpdir.Name())
            return (False, None)
        return (True, tmpdir.Name ())

def unget_mpoint (need_umount, mpoint):
    if not need_umount or not mpoint:
        return
    if need_umount:
        os.system ('umount %s && rm -rf %s' % (mpoint, mpoint))

def get_disk_part (partname):
    diskname = partname.strip ('0123456789')
    if diskname == partname:
        return (diskname, 0)
    partnum = string.atoi (partname[len (diskname):])
    return (diskname, partnum)

def write_dump (fd, disks):
    for disk_name in disks.keys ():
        disk = disks[disk_name]
        for part_no in disk.keys ():
            if part_no == 0:
                part_name = disk_name
            else:
                part_name = '%s%d' % (disk_name, part_no)
            fmt = '%s:%s:%s:%s:%s:%s:%s:%s:%s:%s\n'
            fd.write (fmt % (part_name,
                             disk[part_no]['TYPE'],
                             disk[part_no]['FS'],
                             disk[part_no]['LABEL'],
                             disk[part_no]['START'],
                             disk[part_no]['END'],
                             disk[part_no]['SIZE'],
                             disk[part_no]['OS'],
                             disk[part_no]['DESC'],
                             disk[part_no]['XFER']))
            for fstab_part in disk[part_no]['FSTAB'].keys ():
                fd.write ('fstab:%s:%s:%s\n' %
                          (part_name,
                           fstab_part,
                           disk[part_no]['FSTAB'][fstab_part]))
            for bootloader in disk[part_no]['BLOADER'].keys ():
                fd.write ('bootloader:%s:%s:%s\n' %
                          (part_name,
                           bootloader,
                           disk[part_no]['BLOADER'][bootloader]))


def read_gdump (fd):
    _disks = {}
    all_lines = fd.readlines ()
    for line in all_lines:
        pieces = line.strip (). split (':')
        if pieces[0] == 'fstab':
            (d, p) = get_disk_part (pieces[1])
            _disks[d][p]['FSTAB'][pieces[2]] = pieces[3]
            continue
        if pieces[0] == 'bootloader':
            (d, p) = get_disk_part (pieces[1])
            _disks[d][p]['BLOADER'][pieces[2]] = pieces[3]
            continue
        (d, p) = get_disk_part (pieces[0])
        if not _disks.has_key (d):
            _disks[d] = {}
        if not _disks[d].has_key (p):
            _disks[d][p] = {}
        _disks[d][p]['FSTAB'] = {}
        _disks[d][p]['BLOADER'] = {}
        _disks[d][p]['TYPE'] = pieces[1]
        _disks[d][p]['FS'] = pieces[2]
        _disks[d][p]['LABEL'] = pieces[3]
        _disks[d][p]['START'] = pieces[4]
        _disks[d][p]['END'] = pieces[5]
        _disks[d][p]['SIZE'] = pieces[6]
        _disks[d][p]['OS'] = pieces[7]
        _disks[d][p]['DESC'] = pieces[8]
        _disks[d][p]['XFER'] = pieces[9]

    return _disks

def gdump_partlist (dump):
    _parts = {}
    for disk in dump.keys ():
        for part in dump[disk].keys ():
            part_name = '%s%d' % (disk, part)
            _parts[part_name] = dump[disk][part].copy ()
    return _parts

def partlist_gdump (partlist):
    _dump = {}
    for part_name in partlist.keys ():
        (d, p) = get_disk_part (part_name)
        if not _dump.has_key (d):
            _dump[d] = {}
        if not _dump[d].has_key (p):
            _dump[d][p] = {}
        _dump[d][p] = partlist[part_name]
    return _dump

