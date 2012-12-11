#!/bin/bash

getrandomfile() {
        LS=(`ls`)
        NUMFILES=${#LS[*]}
        RND=$[${RANDOM}%${NUMFILES}]

        #get the first part
        NUM=`echo ${RND} | cut -d"." -f1`

        echo ${LS[${NUM}]}
}

for i in `seq 1 10`;
do
        echo "${i}th run: `getrandomfile`"
done
        
