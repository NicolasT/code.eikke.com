#!/bin/bash

ARTICLELIST=articlelist

OLDIFS=${IFS}
IFS=$'\n'
TRS=""
TRBP="<tr>\n
	<td class=\"arrow\">\&raquo;</td>\n
	<td class=\"title\">@TITLE@</td>\n
	<td class=\"center\">@HTML@</td>\n
	<td class=\"center\">@DOCBOOK@</td>\n
	<td class=\"center\">@PDF@</td>\n</tr>\n"

FILEEXISTS="[<a href=\"@FILENAME@\" title=\"@TITLE@ - @TYPE@ Version\">X</a>]"
FILEDOESNOTEXIST='\\\&nbsp;'

GetFileExistsLine() {
#call using BOILERPLATE, FILENAME, EXTENSION, TITLE and TYPE
	RET=`echo "$1" | sed "s/@FILENAME@/$2.$3/"`
	RET=`echo "${RET}" | sed "s/@TITLE@/$4/"`
	RET=`echo "${RET}" | sed "s/@TYPE@/$5/"`
	echo -e "${RET}"
}

for LINE in `cat ${ARTICLELIST}`;
do
	TITLE=`echo "${LINE}" | cut -d_ -f1`
	FILE=`echo "${LINE}" | cut -d_ -f2`

	CTR=`echo ${TRBP} | sed "s/@TITLE@/${TITLE}/g"`

	if [ -e "${FILE}.html" ];
	then
		HTML=`GetFileExistsLine ${FILEEXISTS} ${FILE} html ${TITLE} HTML`
	else
		HTML=${FILEDOESNOTEXIST}
	fi

	if [ -e "${FILE}.dbk" ];
        then
                DOCBOOK=`GetFileExistsLine ${FILEEXISTS} ${FILE} dbk ${TITLE} DocBook`
        else
                DOCBOOK=${FILEDOESNOTEXIST}
        fi

	if [ -e "${FILE}.pdf" ];
        then
                PDF=`GetFileExistsLine ${FILEEXISTS} ${FILE} pdf ${TITLE} PDF`
        else
                PDF=${FILEDOESNOTEXIST}
        fi

	

	CTR=`echo "${CTR}" | sed "s:@HTML@:${HTML}:"`
	CTR=`echo "${CTR}" | sed "s:@DOCBOOK@:${DOCBOOK}:"`
	CTR=`echo "${CTR}" | sed "s:@PDF@:${PDF}:"`

	TRS="${TRS}${CTR}"

done

sed -e "s:@TRS@:${TRS}:" index.html.in

export IFS=${OLDIFS}
