#!/bin/sh
# Copyright (C) 2006 Z RESEARCH Inc. <http://www.zresearch.com>
#  
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#  
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#  
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#  

_init ()
{
    exec 3>&1
    return 0;
}

get_parts ()
{
    all_devs=$(cat /proc/partitions | tail -n +3 | awk '{print $2,$4}'|grep -v '^0' | cut -f2 -d' ');
    mkdir -p /tmp/.mounttest;
    for dev in $all_devs; do
	mount /dev/$dev /tmp/.mounttest 2>/dev/null|| continue;
	size=$(df -h /dev/$dev | tail -n -1 | awk '{print $4}')
	total=$(df -h /dev/$dev | tail -n -1 | awk '{print $2}')
	type=$(mount | grep "/dev/$dev on /tmp/.mounttest" | awk '{print $5}')
	found="n"
	[ -d /tmp/.mounttest/gluster-state ] && found="yes"
	umount /tmp/.mounttest 2>/dev/null;
	echo "$dev:$size:$total:$type:$found";
    done
    return 0;
}

init_fix ()
{
    local parts="$@";
    count=$(echo "${parts}" | wc -l | awk '{print $0}');
    msg="dialog --menu 'The following partitions were found to have Gluster state preserved from previous sessions. Select one of them to use its configuration and state, or select cancel to start freshely' 0 0 0"
    for part in $parts; do
	dev=$(echo $part | cut -f1 -d:)
	free=$(echo $part | cut -f2 -d:)
	total=$(echo $part | cut -f3 -d:)
	type=$(echo $part | cut -f4 -d:)
	msg=$(printf "${msg} $dev '%6s free | %6s total | type %s'" $free $total $type);
    done
    choice=$(eval "${msg}" 2>&1 >&3) || return 1;
    rm -rf /var/state-mount
    mkdir -p /var/state-mount;
    mount /dev/$choice /var/state-mount;
    rm -rf /var/gluster;
    ln -s /var/state-mount/gluster-state /var/gluster;
    return 0;
}

main ()
{
    mkdir -p /var/gluster-state;
    rm -rf /var/gluster;
    ln -s gluster-state /var/gluster;

    parts="$(get_parts)";
    found=$(echo "${parts}" | grep ':yes' | wc -l | awk '{print $0}');
    [ $found = 0 ] || {
	init_fix $(echo "${parts}" | grep ':yes') && return;
    }
    choice="manual";
    while :; do
      lines=$(echo "${parts}" | wc -l);
      params="";
      for part in $parts; do
	  dev=$(echo $part | cut -f1 -d:);
	  size=$(echo $part | cut -f2 -d:);
	  total=$(echo $part | cut -f3 -d:);
	  type=$(echo $part | cut -f4 -d:);
	  params=$(printf "${params} $dev '%6s free | %6s total | FS type $type'" $size $total);
      done
      params="${params} manual 'Specify directory manually'";
      params="${params} skip 'Do not use persistant storage'";
      choice=$(eval "dialog --ok-label 'select' --cancel-label 'probe again' --default-item $choice --menu 'Select a source for persistant storage of Gluster state. This is necessary to maintain configuration and state across reboots. If you chose to skip having a persistant storage, all your configuration and state will be lost with this session of Gluster.\nSelecting a partition will create a top level directory named gluster-state in it, inside which all of Gluster configuration and state will be preserved. If the selected partition already has a previous Gluster save, it will be continued to be used' 0 0 0 $params" 2>&1 >&3) || {
	  parts="$(get_parts)";
	  choice="skip"
	  continue;
      }

      case $choice in
	  skip)
	      break
	      ;;
	  manual)
	      path=$(dialog --inputbox "Enter directory to store Gluster state" 0 0 2>&1 >&3) || continue
	      [ -d "${path}" ] || {
		  dialog --title Error --msgbox "${path} is not a directory" 0 0;
		  continue;
	      }
	      err="$(mkdir -p ${path}/gluster-state 2>&1)" || {
		  dialog --title Error --msgbox "Cannot create directory inside ${path} ($err)" 0 0;
		  continue;
	      }
	      rm -rf /var/gluster
	      ln -s "${path}/gluster-state" /var/gluster;
	      break
	      ;;
	  *)
	      mkdir -p /var/state-mount;
	      err="$(mount /dev/$choice /var/state-mount 2>&1)" || {
		  dialog --title Error --msgbox "${err}" 0 0;
		  continue;
	      }
	      err="$(mkdir -p /var/state-mount/gluster-state 2>&1)" || {
		  dialog --title Error --msgbox "Cannot create directory in $choice partition ($err)" 0 0;
		  umount /var/state-mount;
		  continue;
	      }
	      rm -rf /var/gluster;
	      ln -s /var/state-mount/gluster-state /var/gluster;
	      break
	      ;;
      esac
    done
    return 0;
}

_init "$@" && main "$@"
