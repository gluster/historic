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
    UP_IFACES_LIST="";
    DOWN_IFACES_LIST="";
    PATH=$PATH:/sbin:/usr/sbin
    refresh_ifaces_list;

    exec 3>&1 
}

subnet_to_prefix ()
{
    local arr;
    local subnet;
    local prefix=0;
    local idx=0;

    arr[0]="0";
    arr[128]="1";
    arr[192]="2";
    arr[224]="3";
    arr[240]="4";
    arr[248]="5";
    arr[252]="6";
    arr[254]="7";
    arr[255]="8";

    subnet[0]=$(echo $1 | cut -f1 -d'.');
    subnet[1]=$(echo $1 | cut -f2 -d'.');
    subnet[2]=$(echo $1 | cut -f3 -d'.');
    subnet[3]=$(echo $1 | cut -f4 -d'.');

    prefix=$(( ${arr[${subnet[0]}]} + ${arr[${subnet[1]}]} + ${arr[${subnet[2]}]} + ${arr[${subnet[3]}]} ));
    echo ${prefix};
}

prefix_to_subnet ()
{
    local arr;
    local subnet;
    local prefix=$1;
    local idx=0;

    arr[0]="0";
    arr[1]="128";
    arr[2]="192";
    arr[3]="224";
    arr[4]="240";
    arr[5]="248";
    arr[6]="252";
    arr[7]="254";
    arr[8]="255";

    subnet[0]="0";
    subnet[1]="0";
    subnet[2]="0";
    subnet[3]="0";

    while [ ${prefix} -ge 8 ]
    do
      subnet[${idx}]="255";
      idx=$(( ${idx} + 1 ));
      prefix=$(( ${prefix} - 8 ));
    done
    if [ ${prefix} -gt 0 ]
    then
	subnet[${idx}]=${arr[${prefix}]};
    fi
    echo "${subnet[0]}.${subnet[1]}.${subnet[2]}.${subnet[3]}";
}    

refresh_ifaces_list ()
{
    UP_IFACES_LIST=$( ip link list | grep UP | sed -n -e 's/^[0-9]*:\ \([^:]*\):.*$/\1/p' );
    DOWN_IFACES_LIST=$( ip link list | grep -v UP | sed -n -e 's/^[0-9]*:\ \([^:]*\):.*$/\1/p' );
}

get_iface_type ()
{
    local IFACE=$1;
    TYPE=$( ip link list dev ${IFACE} | sed -n -e 's/^.*link\/\([^ ]*\).*$/\1/p' );

    echo "${TYPE}";
    [ ! -z "${TYPE}" ];
}

get_iface_mac ()
{
    local iface=$1;
    mac=$( ip link list dev ${iface} | sed -n -e 's/^.*link\/[^ ]*\ *\([^ ]*\).*$/\1/p' );

    echo "${mac}";
    [ ! -z "${mac}" ];
}

get_iface_addrs ()
{
    local iface=$1;
    ip=$( ip address list dev ${iface} | sed -n -e 's/^.*inet \([^ ]*\).*$/\1/p' );

    echo ${ip};
    [ ! -z "${ip}" ];
}

get_add_via ()
{
    local gw=$1;

    [ "${gw}" != "0.0.0.0" ] && [ "${gw}" != "0.0.0.0/0" ] && [ "${gw}" != "default" ] && echo "via ${gw}";
}

edit_route ()
{
    local orig_prefix=$1;
    local orig_iface=$( ip route list | grep "^${orig_prefix} " | sed -n -e 's/.*dev \([^ ]*\).*/\1/p' );
    local orig_gw=$( ip route list | grep "^${orig_prefix} " | sed -n -e 's/.*via \([^ ]*\).*/\1/p' );
    [ -z "${orig_gw}" ] && orig_gw="0.0.0.0";
    local prefix="${orig_prefix}";
    local gw="${orig_gw}";
    local iface="${orig_iface}";
    local oput;
    local err;

    while :
    do
      oput=$( dialog --ok-label "apply" --cancel-label "delete" --extra-label "modify" \
	  --inputmenu "modify route specifics" 0 0 10 \
	  "prefix:" "${prefix}" \
	  "gateway:" "${gw}" \
	  "interface:" "${iface}" 2>&1 >&3);
      err=$?;
      [ ${err} -eq 3 ] && {
	  local field;
	  local value;
	  
	  field=$(echo ${oput} | cut -f1 -d':' | cut -f2- -d' ' );
	  value=$(echo ${oput} | cut -f2 -d':' | cut -f2- -d' ' );

	  [ "${field}" = "prefix" ] && prefix="${value}";
	  [ "${field}" = "gateway" ] && gw="${value}";
	  [ "${field}" = "interface" ] && iface="${value}";
	  continue;

      }
      [ ${err} -eq 0 ] && {
	  oput=$(ip route del ${orig_prefix} via ${orig_gw} dev ${orig_iface} 2>&1) || {
	      dialog --msgbox "could not delete old route ${orig_prefix} via ${orig_gw} dev ${orig_iface}; (${oput})" 0 0;
	      break;
	  }

	  oput=$(ip route add ${prefix} $( get_add_via ${gw}) dev ${iface} 2>&1) || {
	      ip route add ${orig_preifx} $( get_add_via ${orig_gw}) dev ${orig_iface};
	      dialog --msgbox "could not add new route ${prefix} via ${gw} dev ${iface}; (${oput}) [RESTORING ORIGINAL ROUTE]" 0 0;
	      break;
	  }
	  dialog --msgbox "added ${prefix} on ${iface} via ${gw}" 0 0;
	  break;
      }
      [ ${err} -eq 1 ] && {
	  oput=$(ip route del ${prefix} via ${gw} dev ${iface} 2>&1) || {
	      dialog --msgbox "could not delete route ${prefix} via ${gw} dev ${iface}; (${oput})" 0 0;
	      break;
	  }
	  break;
      }
    done
}

accept_new_route ()
{
    local prefix="a.b.c.d/e";
    local iface="eth0";
    local gw="0.0.0.0";
    local oput;
    local err;

    while :
    do
      oput=$( dialog --ok-label "apply" --cancel-label "back" --extra-label "modify" \
	  --inputmenu "enter details of new route" 0 0 10 \
	  "prefix:" "${prefix}" \
	  "gateway:" "${gw}" \
	  "interface:" "${iface}" 2>&1 >&3);
      err=$?;
      [ ${err} -eq 3 ] && {
	  local field;
	  local value;
	  
	  field=$(echo ${oput} | cut -f1 -d':' | cut -f2- -d' ' );
	  value=$(echo ${oput} | cut -f2 -d':' | cut -f2- -d' ' );

	  [ "${field}" = "prefix" ] && prefix="${value}";
	  [ "${field}" = "gateway" ] && gw="${value}";
	  [ "${field}" = "interface" ] && iface="${value}";

      }
      [ ${err} -eq 0 ] && {
	  oput=$( ip route add ${prefix} $( get_add_via ${gw}) dev ${iface} 2>&1) || {
	      dialog --msgbox "could not add route ${prefix} via ${gw} dev ${iface}; (${oput})" 0 0;
	      continue;
	  }
	  dialog --msgbox "added ${prefix} on ${iface} via ${gw}" 0 0;
	  break;
      }
      [ ${err} -eq 1 ] && break;
    done
}

config_iface ()
{
    local iface=$1;
    local ip_list;
    local ip;
    local mode="down";
    local action;
    local no_flag;

    for if in ${UP_IFACES_LIST}
    do
      if [ "${if}" = "${iface}" ]
      then
	  mode="up";
	  break;
      fi
    done

    while :
    do
      ip_list=$(get_iface_addrs ${iface});

      action=$( dialog  --ok-label "modify" --cancel-label \
	  "back" --menu "choose action" 0 0 0 \
          "status" "${mode}" "address(es)" "${ip_list}" 2>&1 >&3 ) || break;
      [ "${action}" = "status" ] && {
	  no_flag="";
          [ "${mode}" = "down" ] && no_flag="--defaultno";
	  dialog ${no_flag} --yes-label "up" --no-label "down" --yesno "set ${iface} state to" 0 0;
	  if [ $? -eq 0 ]
	  then
	      err=$(ip link set up dev ${iface} 2>&1) && mode="up" && continue;
	  else
	      err=$(ip link set down dev ${iface} 2>&1) && mode="down" && continue;
	  fi
	  dialog --msgbox "could not change ${iface} state! (${err})" 0 0 ;
      }
      [ "${action}" = "address(es)" ] && {
	  t_ip_list=$( dialog --inputbox "enter ip address(es) in the form a.b.c.d/e, \nenter multiple entries seperated by space"  0 0 "${ip_list}" 2>&1 >&3 ) || continue;
	  old_ip_list=${ip_list};
	  ip_list=${t_ip_list};
	  ip address flush dev ${iface};
	  for ip in ${ip_list};
	  do
	    err=$(ip address add ${ip} brd + dev ${iface} 2>&1) || {
		ip address flush dev ${iface};
		for old_ip in ${old_ip_list}
		do
		  ip address add ${old_ip} brd + dev ${iface};
		done
		dialog --msgbox "could not add ip ${ip} !! (${err})" 0 0;
		break;
	    }
	  done
      }
    done
}

manage_ifaces ()
{
    local iface;
    local if_list;
    local type;

    while :
    do
      refresh_ifaces_list;
      if_list="";
      for iface in ${UP_IFACES_LIST}
      do
	type="[ up ] $(get_iface_type ${iface})";
	[ ! "${type}" = "[ up ] loopback" ] && type="${type} $(get_iface_mac ${iface})";
	if_list="${if_list} ${iface} '${type}'";
      done

      for iface in ${DOWN_IFACES_LIST}
      do
	type="[down] $(get_iface_type ${iface})";
	[ ! "${type}" = "[down] loopback" ] && type="${type} $(get_iface_mac ${iface})";
	if_list="${if_list} ${iface} '${type}'";
      done

      if=$( sh -c "dialog --ok-label Select --cancel-label Back --menu 'choose interface' 0 0 0 ${if_list}" 2>&1 >&3 ) || return $?;

      config_iface ${if};
    done
}

manage_routes ()
{
    local prefix;
    local r_list;
    local op;

    while :
    do
      r_list="add 'Add new route entry'";
      for prefix in $(ip route list | cut -f1 -d' ')
      do
	local dev_info;
	local via_info;
	local r_info;

	dev_info=$(ip route list | grep "^${prefix} " | sed -n -e 's/^.*\(dev [^ ]*\).*$/\1/p');
	via_info=$(ip route list | grep "^${prefix} " | sed -n -e 's/^.*\(via [^ ]*\).*$/\1/p');
	r_info="${dev_info} ${via_info}";
	r_list="${r_list} ${prefix} '${r_info}'";
      done

      op=$( sh -c "dialog --cancel-label Back --ok-label Select --menu 'Choose to add or edit route' 0 0 0 ${r_list}" 2>&1 >&3 ) || break;
      [ "${op}" = "add" ] && accept_new_route;
      [ "${op}" != "add" ] && edit_route "${op}";
    done

}

manage_names ()
{
    local host;
    local domain;
    local outp;
    local err;

    host=$(hostname 2>/dev/null);
    domain=$(hostname --fqdn | cut -f2- -d. 2>/dev/null);

    while :
    do
      outp=$( dialog --inputmenu "edit entries as needed \n(hostname is without domain suffix)" \
	  15 40 6 "hostname:" "${host}" \
	"domain:" "${domain}" 2>&1 >&3);
      err=$?;

      [ ${err} -eq 3 ] && {
	  local field;
	  field=$(echo ${outp} | cut -f 1 -d: | cut -f 2- -d' ');
	  newval=$(echo ${outp} | cut -f 2 -d: | cut -f 2- -d' ');
	  [ "${field}" = "hostname" ] && host=${newval};
	  [ "${field}" = "domain" ] && domain=${newval};
      }
      [ ${err} -eq 0 ] && {
	  hostname ${host};
	  break;
      }
      [ ${err} -eq 1 ] && {
	  dialog --msgbox  "changes were not applied" 5 30;
	  break;
      }
    done
}

do_advanced ()
{
    local op="";

    while :
    do
      op=$( dialog --menu "choose what to configure" \
	  0 0 \
	  0 \
	  "names" "configure hostname/domainname" \
	  "interfaces" "configure interfaces and ip addresses" \
	  "routes" "configure routes (gateway)" 2>&1 >&3 ) || return $?;

      [ "${op}" = "interfaces" ] && manage_ifaces;
      [ "${op}" = "names" ] && manage_names;
      [ "${op}" = "routes" ] && manage_routes;
    done
}

do_dhcp ()
{
    local eth_ifaces;

    while :
    do
      local iface;

      eth_ifaces="";
      for iface in ${UP_IFACES_LIST} ${DOWN_IFACES_LIST}
      do
	[ "$( get_iface_type ${iface} )" = "ether" ] &&
	eth_ifaces="${eth_ifaces} ${iface} '$(get_iface_mac ${iface})'";
      done
      eth_ifaces="${eth_ifaces} manual 'specify interface name manually'";

      iface=$( sh -c "dialog --ok-label Select --cancel-label Back --menu 'Choose interface to configure via DHCP' 0 0 0 ${eth_ifaces}" \
	  2>&1 >&3 ) || break;
      
      [ "${iface}" = "manual" ] && {
	  iface=$( dialog --ok-label Select --cancel-label Back \
	      --inputbox "Enter name of interface to run DHCP on:" \
	      0 0 eth0 2>&1 >&3 ) || continue;
      }
      
      echo -e "\nAttempting DHCP probe on ${iface}, Hit Ctrl-C to interrupt\n"
      dhclient -q -sf /etc/dhclient-script "${iface}"

      ERR=$?;
      [ ${ERR} -eq 0 ] && {
	  echo -e "\nSuccessfully acquired lease\n\nHit [Enter] to continue";
	  read;
	  exit 0;
      }
      [ ${ERR} -ne 0 ] && {
	  echo -e "\nFailed to acquire lease!\n\nHit [Enter] to continue";
	  read;
      }
    done
}

do_manual_iface_config ()
{
    local iface=$1;
    local op;
    
    local pfx=$(get_iface_addrs ${iface});
    [ -z "${pfx}" ] && pfx="0.0.0.0/0";
    local ip=$(echo ${pfx} | cut -f1 -d/);
    local nm=$(prefix_to_subnet $(echo ${pfx} | cut -f2 -d/));
    local gw=$(ip route list dev ${iface} | sed -n -e 's/default via \([^ ]*\).*/\1/p' | head -1 );
    local hn=$(hostname 2>/dev/null);

    local orig_ip="${ip}";
    local orig_nm="${nm}";
    local orig_gw="${gw}";

    while :
    do
      local ERR;
      op=$( dialog --ok-label Apply --extra-label Modify --cancel-label Back \
	  --inputmenu "manual settings of ${iface}" 0 40 10 \
	  "IP Address:" "${ip}" \
	  "Subnet Mask:" "${nm}" \
	  "Gateway:" "${gw}" 2>&1 >&3 );
      ERR=$?;
      [ ${ERR} -eq 3 ] && {
	  local field;
	  local value;
	  
	  field=$(echo ${op} | cut -f1 -d':' | cut -f2- -d' ');
	  value=$(echo ${op} | cut -f2 -d':' | cut -f2- -d' ');

	  [ "${field}" = "IP Address" ] && ip="${value}";
	  [ "${field}" = "Subnet Mask" ] && nm="${value}";
	  [ "${field}" = "Gateway" ] && gw="${value}";
	  [ "${field}" = "Hostname" ] && hn="${value}";
      }
      [ ${ERR} -eq 1 ] && {
	  local oput;
	  local cmd;
	  [ "${ip}" = "0.0.0.0" ] || [ "${nm}" = "0.0.0.0" ] && {
	      dialog --msgbox "Invalid IP/MASK : ${ip}/${nm}" 0 0;
	      continue;
	  }
	    cmd="ip address flush dev ${iface}" && oput=$( ${cmd} 2>&1 ) && \
	    cmd="ip address add ${ip}/$(subnet_to_prefix ${nm}) brd + dev ${iface}" && oput=$(${cmd} 2>&1 ) && \
	    cmd="ip route add default via ${gw}" && if [ ! -z "${gw}" ] ; then oput=$(${cmd} 2>&1); fi  && \
	    cmd="hostname ${hn}" && oput=$(${cmd} 2>&1) || {
	      dialog --msgbox "ERROR: ${cmd}\n${oput}" 0 0;
	      continue;
	   }
	  break;
      }
      [ ${ERR} -eq 1 ] && {
	  break;
      }
    done
}

do_manual ()
{
   local eth_ifaces;

    while :
    do
      local iface;

      eth_ifaces="";
      for iface in ${UP_IFACES_LIST} ${DOWN_IFACES_LIST}
      do
	[ "$( get_iface_type ${iface} )" = "ether" ] &&
	eth_ifaces="${eth_ifaces} ${iface} '$(get_iface_mac ${iface})'";
      done
      eth_ifaces="${eth_ifaces} manual 'specify interface name manually'";

      iface=$( sh -c "dialog --ok-label Select --cancel-label Back --menu 'Choose interface to configure manually' 0 0 0 ${eth_ifaces}" \
	  2>&1 >&3 ) || break;
      
      [ "${iface}" = "manual" ] && {
	  iface=$( dialog --ok-label Select --cancel-label Back \
	      --inputbox "Enter name of interface to configure manually:" \
	      0 0 eth0 2>&1 >&3 ) || continue;
      }
      do_manual_iface_config "${iface}";
    done

}
main ()
{
    local op;
    
    while :
    do
      op=$( dialog --title "Configure Network" --ok-label Select \
	  --cancel-label Done --menu "Select mode of network configuration" \
	  0 0 \
	  0 \
	  "dhcp" "configure interfaces/routes/hostname via DHCP" \
	  "manual" "manually specify details" \
	  "advanced" "more finegrained configuration" 2>&1 >&3 ) || break;
      [ "${op}" = "dhcp" ] && do_dhcp;
      [ "${op}" = "manual" ] && do_manual;
      [ "${op}" = "advanced" ] && do_advanced;
    done
}

_init "@$" && main "$@";