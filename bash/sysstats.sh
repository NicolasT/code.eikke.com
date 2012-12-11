#!/bin/bash

# Gather system information, can e.g. be sent to | mail in a cronjob
#
# Copyright (C) 2005 Nicolas "Ikke" Trangez (eikke eikke commercial)
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.

# ChangeLog
# ---------
# 05/17/05: Initial version

# TODO
# ----
# * Netstat output
# * Memory/swap information
# * ifconfig information
# * Make output dependent on argv
# * md5sum checks                               (done)
# * Keep previeous values and compare

# HOWTO
# -----
# * To get system info, just run the script: sh sysstat.sh
#
# * To enable md5 checksum checking, you need a file called "md5sums.lst" in
# *    the current directory.
# * This file should be formatted like this:
#       /path/to/file:md5sum
# * where "/path/to/file" must start with a "/"
# * An example:
#       /etc/passcd:r154j3054475f984d71edf67d5800718
#
# * You can run this script in a crontab.
# * You could call it like this (in a wrapper script):
#       cd /path/to/sysstat/script
#       sh /path/to/sysstat/script/sysstats.sh | mail -s "System information for `hostname`" ikke@somehost
# * We need to cd to make sure we can find md5sums.lst

####################### DO NOT EDIT UNDER THIS LINE ###########################

siheader() {
        echo "================================================================"
        echo -e "* System statistics for: \t`hostname`.`dnsdomainname`"
        echo -e "* Generated on: \t\t`date`"
        echo -e "* Running as: \t\t\t`whoami`"
        echo
}

fuptime() {
        upSeconds=$(cat /proc/uptime | tr "." " " | awk '{print $1}')
        secs=$((${upSeconds}%60))
        mins=$((${upSeconds}/60%60))
        hours=$((${upSeconds}/3600/24))
        days=$((${upSeconds}/86400))
        if [ "${days}" -ne "0" ]
        then
                echo -n "${days} days, "
        fi
        echo "`printf '%02d' ${hours}`:`printf '%02d' ${mins}`:`printf '%02d' ${secs}`"
}

siuptime() {
        echo "=================== Uptime ====================================="
        echo -e "* Uptime: \t\t\t`fuptime`"
        if [ -x /usr/bin/uprecords ];
        then
                echo -e "* High score: \t\t\t`uprecords | tail -n1 | cut -d'|' -f1 | cut -d'n' -f3 | sed 's/^[[:blank:]]*//'`"
        fi
        echo -e "* Load average: \t\t`cat /proc/loadavg | head -c14`"
        echo
}

fw () {
        OFS=${IFS}
        IFS=$'\n'
        CNT=$(echo `w | wc -l`-1 | bc)

        w | tail -n ${CNT} | while read -r a;
        do
                echo -e "\t${a}"
        done

        IFS=${OFS}
}

siusers() {
        echo "=================== Users ======================================"
        echo -e "* Active users: \t\t`who | wc -l`"
        echo "* User information"
        fw
        echo
}

fpstree() {
        OFS=${IFS}
        IFS=$'\n'
        pstree | while read -r a;
        do
                echo -e "\t${a}"
        done
        IFS=${OFS}
}

fcomms() {
        ps -eo comm | sort | uniq | grep -v ^COMMAND | column
        echo

        IFS=${OFS}
}

fprocs() {
        echo `TERM=Linux top -n1 -b | grep "Tasks" | awk '{print $4,$6,$8,$10}'`
}

sitasks() {
        echo "=================== Tasks ======================================"
        echo -e "* Number of running tasks: \t$(echo `ps ax | wc -l` - 1 | bc)"
        #This fucks up the email
        #echo "* PS Tree:"
        #fpstree
        echo "* Running programs:"
        fcomms
        echo -e "* CPU load: \t\t\t`TERM=linux top -n2 -b | grep 'Cpu' | tail -n1 | awk '{print $2+$4+$6}'`%"
        PROCS=`fprocs`
        echo "* Process state:"
        echo -e "\tRunning: \t\t`echo ${PROCS} | awk '{print $1}'`"
        echo -e "\tSleeping: \t\t`echo ${PROCS} | awk '{print $2}'`"
        echo -e "\tZombie: \t\t`echo ${PROCS} | awk '{print $3}'`"
        echo -e "\tStopped: \t\t`echo ${PROCS} | awk '{print $4}'`"
        echo
}

froute() {
        OFS=${IFS}
        IFS=$'\n'

        CNT=$(echo `/sbin/route | wc -l` - 1 | bc)
        /sbin/route | tail -n ${CNT} | while read -r a;
        do
                echo -e "\t${a}"
        done

        IFS=${OFS}
}

sinetwork() {
        echo "=================== Networking ================================"
        echo "* Routing table:"
        froute
        echo
}

simemory() {
        echo "=================== Memory ===================================="
        RAMTOTAL=`echo $(($(cat /proc/meminfo | grep MemTotal | awk '{print $2}')/1024))`
        echo "* Ram:\t\t\t free of ${RAMTOTAL}Mb"
}

sidiskstats() {
        echo "=================== Hard Disc ================================="
        echo "* Disk Information:"
        OFS=${IFS}
        IFS=$'\n'
        df -h | grep -v ^none | while read -r a;
        do
                echo -e "\t${a}"
        done
        IFS=${OFS}
        echo
        echo "* Mounts:"
        OFS=${IFS}
        IFS=$'\n'
        mount | while read -r a;
        do
                echo -e "\t${a}"
        done
        IFS=${OFS}
        echo
}

simd5() {
        if [ ! -e md5sums.lst ];
        then
                return
        fi

        echo "=================== MD5Sum Check =============================="
        
        OFS=${IFS}
        IFS=$'\n'

        echo "* Checking MD5 Sums"

        cat md5sums.lst | grep "^/" | while read -r a;
        do
                F=`echo ${a} | cut -d':' -f1`
                S=`echo ${a} | cut -d':' -f2`
                S2=`md5sum ${F} | cut -d' ' -f1`
                echo -n -e "\t${F}: \t\t\t\t"
                if [ "${S}" = "${S2}" ];
                then
                        echo "[OK]"
                else
                        echo "[INVALID]"
                fi                        
        done

        IFS=${OFS}
        echo
}

summary() {
        siheader
        siuptime
        siusers
        sitasks
        sinetwork

        #simemory
        sidiskstats

        simd5

        echo "==============================================================="
        echo "* Brought to you by Ikke - http://www.eikke.com"
}

summary
