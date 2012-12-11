#!/bin/sh
# Autoupdater for modsec rulesets.
#
# This script will attempt to update your rulefiles, and restart apache.
# If it apache does not start after changing rules, it will roll back to
# the old ruleset and restart apache again.
#
# URL: http://cs.evilnetwork.org/cycro
# Copyright 2005, All Rights Reserved
#
# Version 2 by Ikke - http://www.eikke.com
# Copyright 2005

#This is the default config for a Gentoo Apache2 installation, change to suit your needs
APACHESTART="/etc/init.d/apache2 restart"
MODSECPATH="/etc/modsecurity"
APACHEPID="/var/run/apache2.pid"

#Modules
#If you want the "exclude" rules, they should be the first entry in the list
#Spaces in rule names are not allowed
MODULES="exclude blacklist rules useragents blacklist2 apache2-rules rootkits badips"
BASEURI="http://www.gotroot.com/downloads/ftp/mod_security"

##########################################################################
######### you probably don't need to change anything below here ##########
##########################################################################

# internal
PID=`cat ${APACHEPID}`
UPDATED=0

echo -n "Changing PWD: "
/bin/mkdir -p "${MODSECPATH}"
cd "${MODSECPATH}"
echo `pwd`

/usr/bin/rm -f all.conf
/usr/bin/touch all.conf

for MODULE in ${MODULES}; do
        echo -n "Updating ${MODULE}.conf: "
        /usr/bin/wget -t 30 -O "${MODULE}.conf.1" -q ${BASEURI}/${MODULE}.conf
        /usr/bin/touch "${MODULE}.conf"
        if [ `md5sum "${MODULE}.conf" | cut -d " " -f1` != `md5sum "${MODULE}.conf.1" | cut -d " " -f1` ] ; then
                /bin/mv "${MODULE}.conf" "${MODULE}.conf.bak"
                /bin/mv "${MODULE}.conf.1" "${MODULE}.conf"
                UPDATED=`expr $UPDATED + 1`
                echo "ok."
        else
                echo "allready up to date."
                /bin/rm -f "${MODULE}.conf.1"
        fi

        #Make sure the file exists, so Apache won't hang on an non-existing Include file
        /usr/bin/touch "${MODSECPATH}/${MODULE}.conf"
        echo "Include ${MODSECPATH}/${MODULE}.conf" >> all.conf
done

echo "Make sure you got \"Include ${MODSECPATH}/all.conf\" somewhere in your Apache config"

# try restart
if [ "$UPDATED" -gt "0" ]; then
        echo -n "Restarting apache: "
        /bin/kill -HUP ${PID} 2>/dev/null
        # did it work?
        if `/bin/kill -CHLD ${PID} >/dev/null 2>&1`; then
                echo "ok."
                exit 0
        fi
        echo "error. Apache not running."

        #Rollback
        for MODULE in ${MODULES}; do
                echo -n "Rolling back ${MODULE}.conf: "
                /bin/mv ${MODULE}.conf ${MODULE}.conf.new
                /bin/mv ${MODULE}.conf.bak ${MODULE}.conf
                echo "ok."
        done

        # try starting httpd again
        `${APACHESTART}`
        PID=`cat ${APACHEPID}`

        # did that fix the problem?
        if `/bin/kill -CHLD ${PID} >/dev/null 2>&1`; then
                echo "That did the trick."
                exit 0
        fi

        echo "Fatal: Apache still not running! Run apachectl -t to find the error."
        exit 999
fi
