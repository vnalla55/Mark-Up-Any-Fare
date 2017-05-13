#!/bin/ksh

if [[ $# -ne 1 ]]; then
   echo -e >&2 "\n** Error: Missing/Incorrect parameter **\n"
   echo >&2 "Usage: $0 release_label"
   echo -e >&2 "\nExample:"
   echo -e >&2 "$0 atsev2.2010.07.00\n"

   exit 1
fi

TMP1=/tmp/f1$$
TMP2=/tmp/f2$$
TMP3=/tmp/f3$$

for i in historical faredisplay pricingv2 tax shopping shoppingis shoppingesv shoppinghist
do
    echo -e "**** $i Differences ****\n" 
    echo Processing $i ...
    ./GetACMSConfig.sh $1 $i prod > $TMP1
    ./GetACMSConfig.sh $1 $i plab > $TMP2
    diff -wBb $TMP1 $TMP2 | egrep -v "DSSV2_HOST|ASV2_HOST|ASV2_PORT|RTG_SERVER|USE_ASAP|2010|2009|2008|FAM=|BRAND_URL|ASAP_INI|MAX_TRANSACTION|MAX_VIRTUAL" | grep "^>\|<" > $TMP3

    if [ -s $TMP3  ]; then
        cat $TMP3
    else
        echo "No differences detected."
    fi

    echo -e "\n\n"; 

done

rm -f $TMP1 $TMP2 $TMP3
