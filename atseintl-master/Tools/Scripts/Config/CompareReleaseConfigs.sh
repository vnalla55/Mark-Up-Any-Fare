#!/bin/ksh

if [[ $# -ne 3 ]]
then
	echo
   	echo Usage: $0 OldReleaseLabel NewReleaseLabel xmlFileName
	echo
	echo Example:
	echo $0 atsev2.2009.00.01 atsev2.2009.01.01 change_template.xml
	echo
	exit 1
fi

OLD_RELEASE=$1
NEW_RELEASE=$2
XML_FILE=$3
TMPDIR=/tmp/gc$$
OLDCFG=$TMPDIR/f1
NEWCFG=$TMPDIR/f2
DIFF=$TMPDIR/diff.txt
ACMS_SCRIPT=`dirname $0`/GetACMSConfig.sh

if [ "$OLD_RELEASE" = "$NEW_RELEASE" ]
then
	echo Please enter different release labels for comparison
	exit 1
fi

mkdir $TMPDIR

FILES="/vobs/atseintl/deploy/prod/config/faredisplay/dbaccess.ini
/vobs/atseintl/deploy/prod/config/pricing/dbaccess.ini
/vobs/atseintl/deploy/prod/config/tax/dbaccess.ini
/vobs/atseintl/deploy/prod/config/shopping/dbaccess.ini
/vobs/atseintl/deploy/prod/config/shoppingis/dbaccess.ini
/vobs/atseintl/deploy/prod/config/historical/dbaccess.ini
/vobs/atseintl/Server/cacheNotify.xml"


set -A CONFIG_FILE_APPS faredisplay pricingv2 tax shopping shoppingis historical cachenotify

j=0

for i in $FILES
do

	echo
	echo Processing $i
	OLD=$(cleartool describe -short $i@@\/$OLD_RELEASE)
	NEW=$(cleartool describe -short $i@@\/$NEW_RELEASE)


	if [ "$OLD" = "$NEW" ]
	then
		echo Old version=$OLD
		echo New version=$NEW
		echo 
		echo There are NO updates to this file.
	else
		echo
		echo There are updates to this file:
		echo

		echo $i CHANGE > $TMPDIR/${CONFIG_FILE_APPS[$j]}.diffs

		/usr/atria/bin/cleartool diff -ser -opt "-b" $OLD $NEW >> $TMPDIR/${CONFIG_FILE_APPS[$j]}.diffs
		
		cat $TMPDIR/${CONFIG_FILE_APPS[$j]}.diffs
	
		echo >> $TMPDIR/${CONFIG_FILE_APPS[$j]}.diffs
		echo "*******************************************" >> $TMPDIR/${CONFIG_FILE_APPS[$j]}.diffs
		echo >> $TMPDIR/${CONFIG_FILE_APPS[$j]}.diffs
	fi

	echo
	echo "*******************************************"
	echo


	((j=$j+1))

done

APPS="shopping shoppingis shoppinghist shoppingesv pricingv2 faredisplay tax historical shopping_d shoppingis_d fallback"

for i in $APPS
do
    echo -e "\n\nProcessing $i configuration\n\n";

    if [ "$i" = "fallback" ]
    then
        $ACMS_SCRIPT $OLD_RELEASE pricingv2 prod | grep FALLBACK_SECTION > $OLDCFG
        $ACMS_SCRIPT $NEW_RELEASE pricingv2 prod | grep FALLBACK_SECTION > $NEWCFG
    else
        $ACMS_SCRIPT $OLD_RELEASE $i prod | grep -v FALLBACK_SECTION > $OLDCFG
        $ACMS_SCRIPT $NEW_RELEASE $i prod | grep -v FALLBACK_SECTION > $NEWCFG
    fi
        
    /usr/bin/diff -wbB $OLDCFG $NEWCFG > $DIFF
    if [ -s $DIFF ]
    then
        cat $DIFF >> $TMPDIR/$i.diffs
	cat $DIFF
    else
        echo There are NO updates to this configuration
    fi

    echo
    echo "*******************************************"
    echo
done

# Do not miss D pool changes:
for i in shopping shoppingis 
do
    rm -f $DIFF
    diff $TMPDIR/$i.diffs $TMPDIR/${i}_d.diffs 2> /dev/null | egrep "^<|>" | sed 's/^[<>] //' > $DIFF 
    if [ -s $DIFF ]
    then
        echo -e "\n\nD-Pool $i differences:\n" >> $TMPDIR/$i.diffs
        cat $DIFF >> $TMPDIR/$i.diffs
    fi
done

SHEET_APPS="shopping shoppingis shoppinghist shoppingesv pricingv2 faredisplay tax historical fallback cachenotify"

cat ./doc_parts/xml_header.part >$XML_FILE

sed -e "s/__BASELINE__/${NEW_RELEASE}/g" ./doc_parts/xml_worksheet_summary.part >> $XML_FILE 

for i in $SHEET_APPS
do

	if [ ! -s $TMPDIR/$i.diffs ]; then 
		echo "No Changes" >$TMPDIR/$i.diffs
	fi

 	WCOUNT=$( wc -l $TMPDIR/${i}.diffs | awk '{ print $1 }' )

	if [ $i = "fallback"  -o $i = "cachenotify" ]; then
		sed -e "s/__NAME__/${i}/g" -e "s/__BASELINE__/${NEW_RELEASE}/g" \
		-e "s/__ROW_COUNT__/${WCOUNT}/g" \
		./doc_parts/xml_worksheet_header_short.part >> $XML_FILE
	else
		((WCOUNT=$WCOUNT+18))	
		sed -e "s/__NAME__/${i}/g" -e "s/__BASELINE__/${NEW_RELEASE}/g" \
		-e "s/__ROW_COUNT__/${WCOUNT}/g" \
		./doc_parts/xml_worksheet_header.part >> $XML_FILE
	fi

	sed -f sed.cmds < $TMPDIR/$i.diffs >> $XML_FILE
	cat ./doc_parts/xml_worksheet_footer.part >> $XML_FILE	

done
cat ./doc_parts/xml_worksheet_rtg.part >> $XML_FILE
cat ./doc_parts/xml_footer.part >> $XML_FILE


rm -rf $TMPDIR
