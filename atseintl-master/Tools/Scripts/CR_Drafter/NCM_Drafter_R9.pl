#!/usr/bin/perl

push (@INC, "./modules", "./data");

require("processInvProd.pl");
require("ncmUtils_R9.pl");
require("AppList.pl");

$baseDir = "/tmp/crcr";
$tmpDir = $baseDir . "/" . getlogin() . ".Generated_NCM";
$fixedDir = $baseDir . "/" . getlogin() . ".input";
$INVPROD = "data/invprod.csv";
$FIXED_INVPROD = "$fixedDir/invprod_fixed.csv";
$NCM_OVERRIDES = "$tmpDir/NCM.TXT";

## Main ##

print "Creating work area ...\n";

mkdir $baseDir;
chmod 0777, $baseDir;
mkdir $tmpDir;
mkdir $fixedDir;
unlink $FIXED_INVPROD, $NCM_OVERRIDES;

print "Processing input files ...\n";
@invprod = &processInvProd($INVPROD, $FIXED_INVPROD);
@atseV2Apps = @prodApps;

print "Generating ncm overrides ...\n";
open(NCMFILE, "> $NCM_OVERRIDES") or die "Cannot open $NCM_OVERRIDES.\n", $!;

print NCMFILE "### ********************************* ###\n";
print NCMFILE "### *** BEGIN AUTO-GENERATED PART *** ###\n";
print NCMFILE "### ********************************* ###\n\n";

print NCMFILE "### *** BEGIN AUTO-GENERATED APP LIST *** ###\n\n";
generateAppList(\@invprod);
print NCMFILE "\n### *** END AUTO-GENERATED APP LIST *** ###\n";

print NCMFILE "\n### *** BEGIN AUTO-GENERATED LIMITED NODES INFO *** ###\n\n";
generateLimitedNodes(\@invprod);
print NCMFILE "\n### *** END AUTO-GENERATED LIMITED NODES INFO *** ###\n";

print NCMFILE "\n### *** BEGIN AUTO-GENERATED SHOPPING GROUPS INFO *** ###\n\n";
generateShoppingGroups(\@invprod);
print NCMFILE "\n### *** END AUTO-GENERATED SHOPPING GROUPS INFO *** ###\n";

print NCMFILE "\n### ******************************* ###\n";
print NCMFILE "### *** END AUTO-GENERATED PART *** ###\n";
print NCMFILE "### ******************************* ###\n\n";

print "\n*** NCM Overrides are in $NCM_OVERRIDES\n\n";

