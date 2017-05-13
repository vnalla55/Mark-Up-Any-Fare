#!/bin/ksh

NAME=$0

function create_pricing_request
{
	echo "<PricingRequest><AGI A10=\"TYO\" A20=\"A0NC\" A21=\"FC83\" AB0=\"9999999\" AB1=\"1639119\" A90=\"HOR\" N0G=\"*\" A80=\"A0NC\" B00=\"1S\" C40=\"JPY\" Q01=\"19\" PAV=\"T\" AE0=\"1J\" /><BIL S0R=\"PSS\" A20=\"FC83\" AE0=\"AA\" AD0=\"547EE0\" A22=\"A0NC\" AA0=\"HOR\" C20=\"INTLWPI1\" C01=\"363495939624454402\" A70=\"WPQ/*\" /><PRO C45=\"JPY\" D07=\"$REQUESTDATE\" D54=\"0364\" PBZ=\"F\" N08=\"B\" Q0P=\"1\" C10=\"854\" P0J=\"T\" SEZ=\"T\" S14=\"WPQ/*854/FCCONFIG+\" S15=\"CRSAGT\" /><DIG Q0A=\"30\" S01=\"FCCONFIG\" /><PXI B70=\"ADT\" Q0U=\"01\" /><SGI Q0C=\"01\" ><FLI Q0C=\"01\" N03=\"A\" B00=\"AA\" B01=\"AA\" A01=\"AUS\" A02=\"DFW\" B30=\"Y\" D01=\"$TRAVELDATE\" Q0B=\"389\" D02=\"$TRAVELDATE\" D00=\"$REQUESTDATE\" D31=\"0780\" D32=\"0845\" D30=\"0364\" BB2=\"SS\" BB0=\"OK\" /></SGI><RES ><IDS B00=\"AA \" Q0B=\"389\" B30=\"Y \" D01=\"$TRAVELDATE\" D31=\"1300\" A01=\"AUS\" D02=\"$TRAVELDATE\" D32=\"1405\" A02=\"DFW\" A70=\"SS\" Q0U=\"01\" N0V=\"J\" P2X=\"ET\" /></RES></PricingRequest>" > $REQUESTFILE
}

function create_faredisplay_request
{
	echo "<FAREDISPLAYREQUEST A01=\"AUS\" A02=\"DFW\" D01=\"$TRAVELDATE\" D06=\"$REQUESTDATE\" D54=\"0275\" S58=\"FQ\" ><BIL S0R=\"PSS\" A20=\"HDQ\" A22=\"A0NC\" A70=\"FQAUS\" AA0=\"HOR\" AD0=\"02BD9D\" AE0=\"AA\" C20=\"FAREDSP1\" C01=\"365243614893433346\" /><PRO P80=\"T\" PBO=\"T\" PCF=\"T\" Q40=\"854\" S14=\"FQAUSDFW15MAY-AA$854/FCCONFIG\" /><PCL B01=\"AA\" /><AGI A10=\"TYO\" A20=\"A0NC\" A21=\"FC83\" A80=\"A0NC\" A90=\"HOR\" AB0=\"9999999\" AB1=\"1639119\" B00=\"1J\" C40=\"JPY\" N0G=\"*\" Q01=\"19\" /><DIG Q0A=\"30\" S01=\"FCCONFIG\" /></FAREDISPLAYREQUEST >"  > $REQUESTFILE
}

function create_shopping_request
{
	echo "<ShoppingRequest N06=\"C\" D70=\"150\" TID=\"1257954728734229\"><DIA Q0A=\"854\"><ARG NAM=\"DD\" VAL=\"ALL\"/></DIA><AGI A10=\"DFW\" A20=\"7BOB\" A21=\"7BOB\" AB0=\"2325292\" AB1=\"2325292\" A90=\"HX9\" N0G=\"*\" B00=\"1S\" A40=\"DE\" Q01=\"9\" A80=\"7BOB\"/><BIL A20=\"7BOB\" A22=\"7BOB\" A70=\"JR\" Q03=\"925\" Q02=\"3470\" AE0=\"AA\" AD0=\"F26102\" AA0=\"DH3\" L00=\"1257954728734229\" C01=\"1257954728734229\" C02=\"1257954728734229\" C20=\"ATSEIMIP\" C21=\"SISXX100\" C22=\"ISELL\"/><PXI B70=\"NEG\" Q0U=\"2\"></PXI><PRO Q0S=\"1\" D07=\"$REQUESTDATE\" D54=\"1012\" P0J=\"T\" PPC=\"T\" SEZ=\"T\" CS0=\"T\"></PRO><PDT D01=\"$TRAVELDATE\"/><AAF Q1K=\"1\" B00=\"XQ\" B01=\"XQ\" Q0B=\"838\" S05=\"738\" P0Z=\"T\"><BRD A01=\"SAW\" D01=\"$TRAVELDATE\" D31=\"0005\"/><OFF A02=\"SXF\" D02=\"$TRAVELDATE\" D32=\"0200\"/></AAF><AVL Q16=\"1\" A01=\"SAW\" A02=\"SXF\"><FBK Q1K=\"1\" Q17=\"Y4|B4|H4|M4|K4|N4|L4|G4|T4|U4|X4|E4|V4|W4|Q4\"></FBK></AVL><LEG Q14=\"0\" B31=\"Y\"><SOP Q15=\"1\"><FID Q1K=\"1\"/><AID Q16=\"1\"/></SOP></LEG><ITN Q5Q=\"0\" C65=\"0\"><SID Q15=\"1\" Q14=\"0\"><BKK B30=\"Q\" B31=\"Y\"/></SID></ITN><OND A01=\"IST\" A02=\"BER\" D01=\"$TRAVELDATE\"/></ShoppingRequest>" > $REQUESTFILE
}

function create_tax_request
{
	echo "<AncillaryPricingRequest Version=\"1.0.0\"> <AGI A10=\"LON\" A21=\"LON\" A90=\"-MF\" AB2=\"9120120\" B00=\"1S\" C40=\"USD\" /> <BIL A20=\"HDQ\" A22=\"LON\" AA0=\"-MF\" AD0=\"D63AA7\" AE0=\"AA\" Q02=\"6034\" Q03=\"901\" S0R=\"PPSS\" /> <PRO AF0=\"DFW\" C10=\"854\"> <RFG Q0A=\"1\" S01=\"BG\" /> </PRO> <DIG Q0A=\"854\" S01=\"FCCONFIG\" /> <ITN Q00=\"1\"> <IRO SHC=\"12345\"> <PXI B70=\"ADT\" Q0U=\"1\" /> </IRO> <SGI Q00=\"01\"> <FLI A01=\"DFW\" A02=\"FRA\" B00=\"AA\" B01=\"AA\" B30=\"Y\" D01=\"$TRAVELDATE\" D02=\"$TRAVELDATE\" D31=\"2025\" D32=\"2140\" Q0B=\"2485\" /> </SGI> <SGI Q00=\"02\"> <FLI A01=\"FRA\" A02=\"LAX\" B00=\"AA\" B01=\"AA\" B30=\"Y\" D01=\"$TRAVELDATE\" D02=\"$TRAVELDATE\" D31=\"2320\" D32=\"0855\" Q0B=\"7356\" /> </SGI> </ITN> </AncillaryPricingRequest>" > $REQUESTFILE
}

function usage
{
   echo -e >&2 "\n** Error: Missing/Incorrect parameters **\n"
   echo >&2 "Usage: $NAME application hostname:port"
   echo -e >&2 "\nApplication must be one of pricing, faredisplay, tax or shopping"
   echo >&2 "Use 'pricing' for pricing & historical pricing servers"
   echo >&2 "Use 'faredisplay' for faredisplay servers"
   echo -e >&2 "Use 'shopping' for shopping is, mip, esv and historical shopping\n"
   echo -e >&2 "Examples:"
   echo >&2 "$NAME pricing piili001:53701"
   echo >&2 "$NAME shopping pimli006:53601"
   echo >&2 "$NAME faredisplay piili001:53901"
   echo >&2 "$NAME tax piili001:53501"
   echo -e >&2 "\n**** IMPORTANT: DO NOT USE MISMATCHED APPLICATION NAME. IT MAY KILL THE SERVER!!! ****\n"

   exit 1
}

function get_travel_date
{
	MOUNT=`date "+%m" | sed 's/^0*//'`
	YEAR=`date "+%Y"`

	(( MOUNT++ ))

	if (( $MOUNT > 12 )); then
    	MOUNT=1
	    (( YEAR++ ))
	fi

	if (( $MOUNT < 10 )); then
		TRAVELDATE=$YEAR-0$MOUNT-15
	else
    	TRAVELDATE=$YEAR-$MOUNT-15
	fi
}

# *** MAIN ***

if [[ $# -ne 2 ]]; then
   usage
fi

APPLICATION=$1
HOST_PORT=$2

if [[ "$APPLICATION" != "shopping" && "$APPLICATION" != "tax" && "$APPLICATION" != "pricing" && "$APPLICATION" != "faredisplay" ]] ; then
   usage
fi

CLIENT=`dirname $0`/socket_client.pl
if [ ! -s $CLIENT ]; then
    echo "$CLIENT not found. Set a view to continue."
    exit 1
fi

TMPDIR=/tmp/gc$$
IOR_FILE=$TMPDIR/ior
OUT1=$TMPDIR/out1
OUT=$TMPDIR/out
mkdir $TMPDIR

REQUESTDATE=`date "+%Y-%m-%d"`
REQUESTFILE=$TMPDIR/req.xml
get_travel_date

create_${APPLICATION}_request $DATE $TRAVELDATE $REQUEST
echo $HOST_PORT > $IOR_FILE

$CLIENT $IOR_FILE $REQUESTFILE -verbose > $OUT

ENDLINE=`grep -n "CONFIGURATION DIAGNOSTIC END" $OUT | cut -d: -f1`

if [ -z $ENDLINE ]; then
	echo >&2 "Incorrect response. Is hostname:port ($HOST_PORT) correct? Check host name and port number."
    rm -rf $TMPDIR
    exit 1
fi

(( ENDLINE-- ))
sed -i ${ENDLINE}q $OUT

if [[ "$APPLICATION" = "shopping" ]]; then
    sed 's/^1//' $OUT > $OUT1

    grep "HOSTNAME :" $OUT1 | grep PORT | cut -d[ -f3 > $OUT
    grep "DATABASE:" $OUT1 >> $OUT
    grep "BASELINE:" $OUT1 >> $OUT

    sed -i 's/:/=/g' $OUT
    sed -i 's/ //g' $OUT
    sed -i 's/PORT/ PORT/' $OUT

    START=`grep -n "CONFIGURATION DIAGNOSTIC START" $OUT1 | cut -d: -f1`
    sed -i 1,${START}d $OUT1

    sed -i '/./!d' $OUT1
    sed -i 's/ /_/g' $OUT1

    sed -i 's/\*\*\*\*\*_/\n[/' $OUT1
    sed 's/_\*\*\*\*\*/]/' $OUT1 >> $OUT

    sed -i 's/:/=/' $OUT
else
    grep "^<MSG" $OUT | cut -d\" -f6 > $OUT1

	head -3 $OUT1 > $OUT
	sed -i 's/:/=/' $OUT
	sed -i 's/ //g' $OUT

	sed -i '1,4d' $OUT1

	sed -i 's/ /_/g' $OUT1
	sed -i 's/\//:/' $OUT1
	sed -i 's/\//=/' $OUT1

	sed -i 's/[^:]*/[&]/' $OUT1
	sed -i 's/]:/]\n/' $OUT1

	sed -n 'G; s/\n/&&/; /^\(\[[ -~]*\n\).*\n\1/d; s/\n//; h; P' $OUT1 >> $OUT
    sed -i 's/\[/\n[/' $OUT
fi

if [ ! -s $OUT ]; then
	echo >&2 "Empty response. Is hostname:port ($HOST_PORT) correct? Check host name and port number."
	rm -rf $TMPDIR
    exit 1
else
	cat $OUT
fi

rm -rf $TMPDIR
