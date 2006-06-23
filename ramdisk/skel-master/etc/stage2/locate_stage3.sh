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
}

list_cdrom_devices ()
{
    local dev;

    for dev in $( find /proc/ide -name media 2>/dev/null )
    do
      [ "$(cat ${dev})" = "cdrom" ] && echo $(echo ${dev} | cut -f5 -d/);
    done

    for dev in $( ls /dev/scd* 2>/dev/null )
    do
      echo $dev | cut -f3 -d/;
    done
}


method_nfs ()
{
    local ip;
    local share;
    local ip_exportpoint;
    local exportpoint_subdir;

    if dialog --yesno 'Do you want to configure your network interfaces now?' 0 0 ; then
	/etc/stage2/config_network.sh
    fi
    while :
    do
      ip_exportpoint=$(dialog --title "[ NFS Export ]" \
	  --inputbox "Specify NFS export e.g: 192.168.13.22:/tmp " 0 0 \
	  "${ip_exportpoint}" 2>&1 >&3 ) || return 1;
      [ -z "${ip_exportpoint}" ] && { 
	  dialog --msgbox "Given NFS share path is invalid." 0 0;
	  continue;
      }
      ip=$(echo $ip_exportpoint | cut -f1 -d':');
      [ -z "${ip}" -o "${ip}" = "${ip_exportpoint}" ] && {
	  dialog --msgbox "Specified hostname is invalid." 0 0;
	  continue;
      }
      share=$(echo $ip_exportpoint | cut -f2 -d':');
      [ -z "${share}" -o "${share}" = "${ip_exportpoint}" ] && {
	  dialog --msgbox "Specified export point is invalid." 0 0;
	  continue;
      }
      exportpoint_subdir=$(dialog --title "[ Extensions Path ]" \
	  --inputbox "Specify path relative to the export pointing to the \'extensions\' directory"\
	  0 0 "${exportpoint_subdir}" 2>&1 >&3 ) || return 1;

#      while :
#	do
#	/sbin/ip route get "${ip}";
#	[ $? -eq 0 ] && break;
#	dialog --yesno " ${ip} is not reachable. configure network? " 0 0 || break;
#	/etc/stage2/config_network.sh;
#      done
#      /sbin/ip route get "${ip}";
#      [ $? -eq 0 ] || continue;

      /usr/sbin/rpc.portmap
      mkdir -p /mnt/nfs;
      err=$(mount -t nfs $ip_exportpoint /mnt/nfs 2>&1);
      [ $? -eq 0 ] || {
	  dialog --msgbox "Could not mount NFS volume from $ip_exportpoint ($err)" 0 0;
	  continue;
      }
      [ -f /mnt/nfs/${exportpoint_subdir}/init.gex ] || {
	  dialog --msgbox "$ip_exportpoint is not a valid Gluster extensions directory " 0 0;
	  umount /mnt/nfs;
	  continue;
      }
      
      rm -rf /stage3 && ln -sf /mnt/nfs/${exportpoint_subdir} /stage3;
      [ $? -eq 0 ] || {
	  dialog --msgbox " Could not create symlink " 0 0;
	  umount /mnt/nfs;
	  continue;
      }
      return 0;
    done
}

method_samba()
{
    local ip;
    local share;
    local ip_sambapath;
    local samba_subdir;
    while :
      do 
      ip_sambapath=$( dialog --inputbox "Specify SAMBA share path, e.g: //Hostname/share " 0 0 "${ip_sambapath}" 2>&1 >&3) || return 1;
      nothing=$(echo $ip_sambapath | sed -re 's,^//[^/]+/[^/]+$,,')
      ip=$(echo $ip_sambapath | sed -e 's/^.*\/\///' | cut -f1 -d'/');
      share=$(echo $ip_sambapath | sed -e 's/^.*\/\///' | cut -f2 -d'/');

      [ -z "${nothing}" ] || {
	  dialog --msgbox "Invalid samba path specification. Should be like //hostname/share" 0 0
	  continue;
      }
      [ -z "${ip_sambapath}" ] && {
	  dialog --msgbox " Given SMB share path is invalid " 0 0;
	  continue;
      }
      [ -z "${ip}" ] && {
	  dialog --msgbox " Given Hostname is invalid... " 0 0;
	  continue;
      }
      [ -z "${share}" ] && {
	  dialog --msgbox " Given Sharepath is invalid... " 0 0;
	  continue;
      }
      samba_subdir=$( dialog --inputbox "Enter subdirectory inside the share pointing to 'extensions' directory. E.g If remote directory path is /YOURSHARE/Gluster/extensions/ then enter  Gluster/extensions/" 0 0 "${samba_subdir}" 2>&1 >&3) || return 1;
      
#      while :
#	do
#	/sbin/ip route get "${ip}";
#	[ $? -eq 0 ] && break;
#	dialog --yesno " $ip is not reachable. configure network? " || break;
#	/etc/stage2/config_network.sh
#      done
#      /sbin/ip route get "${ip}";
#      [ $? -eq 0 ] || continue;
      
      mkdir -p /mnt/samba;
      mount -f smbfs $ip_sambapath /mnt/samba;
      [ $? -eq 0 ] || {
	  dialog --msgbox "Could not mount SAMBA volume from $ip_sambapath" 0 0;
	  continue;
      }
      [ -f /mnt/samba/${samba_subdir}/init.gex ] || {
	  dialog --msgbox "$ip_sambapath/${samba_subdir} is not a VALID Gluster extensions directory " 0 0;
	  umount /mnt/samba;
	  continue;
      }

      rm -rf /stage3 && ln -sf /mnt/samba/${samba_subdir} /stage3;
      [ $? -eq 0 ] || {
	  dialog --msgbox " Could not create symlink " 0 0;
	  umount /mnt/samba;
	  continue;
      }
      return 0
    done
}

method_http()
{
    local url;
    local ip;
    while :
      do
      http_path=$( dialog --inputbox "Specify HTTP server path for example: http://192.168.0.1/share/ \n HTTP server path " 0 0 "${http_path}" 2>&1 >&3 ) || return 1;
      [ -z "${http_path}" ] && {
	  dialog --msgbox " Specified HTTP path is invalid... " 0 0;
	  continue;
      }
      ip=$(echo $http_path | sed 's/^.*http:\/\///' | cut -f1 -d'/');
      subdir_path=$( dialog --inputbox "Specify the subdir path for example: http://192.168.0.1/share/Gluster/extensions/ " 0 0 "${subdir_path}" 2>&1 >&3 ) || return 1;
      [ -z "${subdir_path}" ] && {
	  dialog --msgbox " Specified SUBDIR path is invalid ..." 0 0;
	  continue;
      }
      while :
      do
      /sbin/ip route get "${ip}";
      [ $? -eq 0 ] && break;
      dialog --yesno " $ip is not reachable. configure network? " || break;
      /etc/stage2/config_network.sh
      done
      /sbin/ip route get "${ip}";
      [ $? -eq 0 ] || continue;
    done
}

method_cdrom ()
{
    local dev;
    local dev_str;
    local err;

    while :
    do
      dev_str="";
      for dev in $( list_cdrom_devices )
      do
	minor_num=$(ls -l /dev/$dev | awk '{print $6}');
	major_num=$(ls -l /dev/$dev | awk '{print $5}' | cut -f1 -d,);
	this_dev_str="''"
	[ -d /proc/ide/${dev} ] && \
	    this_dev_str="$(cat /proc/ide/${dev}/model)";
	
	grep_out="$(grep $major_num:$minor_num /sys/block/*/dev)"
	[ -z "${grep_out}" ] || {
	    [ -f $(dirname $grep_out)/device/model ] && \
	    this_dev_str="$(cat $(dirname $grep_out)/device/model)";
        }
	dev_str="${dev_str} '${dev}' '${this_dev_str}'";
      done

      dev_str="${dev_str} 'manual' 'Manually specify cdrom device'";
      dev=$( sh -c "dialog --title '[ CDROM Source ]' --menu 'Choose CD/DVD-ROM device' 0 0 0 ${dev_str}" 2>&1 >&3 ) || return 1;
      [ "${dev}" = "manual" ] && {
	  dev=$( dialog --inputbox 'Specify cdrom device' 0 0 /dev/hda 2>&1 >&3 ) || continue;
	  dev=$(basename "${dev}");
      }
      [ -b /dev/"${dev}" ] || {
	  dialog --msgbox "Cannot access block device /dev/${dev}" 0 0;
	  continue;
      }
      [ ! -z "$(grep /dev/${dev}\  /proc/mounts)" ] && {
	  dialog --msgbox "/dev/${dev} already mounted on $(grep /dev/${dev}\  /proc/mounts | awk '{print $2}')" 0 0;
	  continue;
      }

      err=$( mkdir -p /mnt/cdrom 2>&1) ||  {
	  dialog --msgbox "Could not create mount point directory /mnt/cdrom; (${err})" 0 0;
	  continue;
      }
      err=$( mount -t iso9660 /dev/${dev} /mnt/cdrom 2>&1) || {
	  dialog --msgbox "Could not mount /dev/${dev} on /mnt/cdrom; (${err})" 0 0;
	  continue;
      }
      [ -f /mnt/cdrom/extensions/init.gex ] || {
	  umount /mnt/cdrom;
	  dialog --msgbox "CD/DVD-ROM is not a valid Gluster CDROM; unmounted" 0 0;
	  continue;
      }
      err=$( rm -rf /stage3 && ln -s /mnt/cdrom/extensions /stage3 2>&1 ) || {
	  dialog --msgbox "Could not create /stage3 symlink; (${err})" 0 0;
	  continue;
      }

      break;
    done    
    return 0;
}

method_path ()
{
    local path;
    local err;

    while :
    do
      path=$( dialog --title '[ Manual PATH ]' --inputbox "Path for Gluster" 0 0 "${path}" 2>&1 >&3 ) || return 1;

      [ -f "${path}/init.gex" ] || {
	  dialog --msgbox "${path} does not contain init.gex extension" 0 0;
	  continue;
      }

      err=$( rm -rf /stage3 && ln -s "${path}" /stage3 2>&1 ) || {
	  dialog --msgbox "Could not create /stage3 symlink; (${err})" 0 0;
	  continue;
      }
      break;
    done
    return 0;
}

check_auto_mode ()
{
    # example stage3 specifiers
    #
    # gluster_stage3=disk:sda1:gluster-beta/iso_fs/extensions
    # gluster_stage3=cdrom:sdb

    local gl_s3;
    local s3_scheme;
    local teh_disk;
    local subdir;

    gl_s3=$(sed -n -e 's/.*gluster_stage3=\([^ ]*\).*/\1/p' /proc/cmdline);
    [ -z "$gl_s3" ] && return 0;

    s3_scheme=$(echo $gl_s3 | cut -f1 -d:)

    case $s3_scheme in
	disk)
	    teh_disk=$(echo $gl_s3 | cut -f2 -d:)
	    [ -b "/dev/$teh_disk" ] || {
		dialog --msgbox "Invalid disk '$teh_disk'" 0 0
		return 1;
	    }
	    mkdir -p /var/stage3-mount;
	    umount /var/stage3-mount 2>/dev/null;
	    err=$(mount /dev/$teh_disk /var/stage3-mount 2>&1) || {
		dialog --title "Mount error" \
		    --msgbox "mount $teh_disk: $error" 0 0;
		return 1;
	    }
	    subdir=$(echo $gl_s3 | cut -f3 -d:)
	    [ -f /var/stage3-mount/$subdir/init.tgz -a ! -z "$(tar tzf /var/stage3-mount/$subdir/init.tgz ./runme)" ] || {
		dialog --title "Invalid location" \
		    --msgbox "$teh_disk:$subdir does not have a valid Gluster init extension" 0 0;
		umount /var/stage3-mount;
		return 1;
	    }
	    rm -f /stage3;
	    ln -s /var/stage3-mount/$subdir /stage3;
	    dialog --infobox "Loading Gluster from $teh_disk:$subdir ..." 0 0
	    exec /etc/stage2/load_stage3.sh /stage3/init.tgz;
	    ;;
    esac
}
    
main ()
{
    local method;

    [ -z "$(which GDesktop)" ] || exec GDesktop app
    check_auto_mode;


    while [ ! -f /stage3/init.tgz ] || [ -z "$(tar tzf /stage3/init.tgz ./runme)" ]
    do
      method=$( dialog --title "[ Gluster Source ]" --no-cancel --menu \
        "Select Gluster extensions source\n" 0 0 0\
	"cdrom" "Select Gluster CD/DVD-ROM device"\
        "nfs" "NFS mount Gluster extensions" \
	"path" "Manually specify Gluster extensions path" 2>&1 >&3) || break;
#	"samba" "SAMBA mount Gluster extensions" \
      [ "${method}" = "cdrom" ] && method_cdrom && break;
      [ "${method}" = "path" ] && method_path && break;
      [ "${method}" = "nfs" ] && method_nfs && break;
      [ "${method}" = "samba" ] && method_samba && break;
    done

    exec /etc/stage2/load_stage3.sh /stage3/init.tgz
}

_init "$@" && main "$@"
