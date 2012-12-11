#!/bin/bash

getrandomfile() {
        LS=(`ls`)
        NUMFILES=${#LS[*]}
        RND=`echo "scale=5;(${RANDOM}/32767)*${NUMFILES};" | bc`

        #get the first part
        NUM=`echo ${RND} | cut -d"." -f1`

        echo ${LS[${NUM}]}
}

for i in `seq 1 10`;
do
        echo "${i}th run: `getrandomfile`"
done
        
