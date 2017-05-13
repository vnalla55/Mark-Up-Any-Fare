#!/usr/bin/perl

push (@INC, "./modules", "./data");

require("dashify.pl");
require("getPoolNodes.pl");
require("getBigIpList.pl");
require("processInvProd.pl");
require("getPoolGroups.pl");
require("ncmUtils.pl");
require("AppList.pl");

$baseDir = "/tmp/crcr";
$tmpDir = $baseDir . "/" . getlogin() . ".Generated_NCM";
$fixedDir = $baseDir . "/" . getlogin() . ".input";
$INVPROD = "data/invprod.csv";
$FIXED_INVPROD = "$fixedDir/invprod_fixed.csv";
$BIGIP_CSV = "data/big_ips.csv";
$FIXED_BIGIP_CSV = "$fixedDir/big_ips_fixed.csv";
$NCM_OVERRIDES = "$tmpDir/NCM.TXT";

%appConNames = (
               "ShopIS200", "C-ShoppingIS200",
               "ShopISLMC", "D-ShoppingIS200",
               "ShopISTVCY", "E-ShoppingIS200",
               "ShopMIP200", "C-ShoppingMIP200",
               "ShopMIPLMC", "D-ShoppingMIP200",
               "ShopMIPTVCY", "E-ShoppingMIP200"
              );

## Main ##

print "Creating work area ...\n";

mkdir $baseDir;
chmod 0777, $baseDir;
mkdir $tmpDir;
mkdir $fixedDir;
unlink $FIXED_INVPROD, $FIXED_BIGIP_CSV, $NCM_OVERRIDES;

print "Processing input files ...\n";
@invprod = &processInvProd($INVPROD, $FIXED_INVPROD);
@atseV2Apps = @prodApps;
my ($t1, $t2) = &getBigIpList;
%bigIpList = %$t1;

print "Generating ncm overrides ...\n";
open(NCMFILE, "> $NCM_OVERRIDES") or die "Cannot open $NCM_OVERRIDES.\n", $!;

print NCMFILE "### ********************************* ###\n";
print NCMFILE "### *** BEGIN AUTO-GENERATED PART *** ###\n";
print NCMFILE "### ********************************* ###\n\n";

print NCMFILE "### *** BEGIN AUTO-GENERATED LIMITED NODES INFO *** ###\n\n";

generateBigIPInfo($NCM_OVERRIDES, \%bigIpList, \@invprod);
generateCoreNodes($NCM_OVERRIDES, \%bigIpList, \@invprod);

print NCMFILE "\n### *** END AUTO-GENERATED LIMITED NODES INFO *** ###\n";

print NCMFILE "\n\n################################################\n";
print NCMFILE "# D-Pool overrides\n";
print NCMFILE "################################################\n";
foreach my $poolName (@atseV2Apps)
{
    next if ($poolName ne "ShopISLMC" && $poolName ne "ShopMIPLMC");

    my @poolList = &getPoolNodes($poolName, "all", \@invprod);
    $appName = $deployList{$poolName};

    $nodes = &dashifyList(sort @poolList);
    print NCMFILE "$nodes, config.vars, $appName, ADMIN_SHOPPING_OVERRIDE_CFG, D\n";
}

print NCMFILE "\n\n################################################\n";
print NCMFILE "# Application console settings\n";
print NCMFILE "################################################\n";
foreach my $poolName (@atseV2Apps)
{
   $appConGroup = $appConNames{$poolName};
   next if length($appConGroup) == 0;

   my @poolList = &getPoolNodes($poolName, "all", \@invprod);
   $appName = $deployList{$poolName};

   $nodes = &dashifyList(sort @poolList);
   $cfgName = "ADMIN_" . uc $appName . "_OVERRIDE_CFG";
   print NCMFILE "$nodes, config.vars, $appName, $cfgName, $appConGroup\n";
}

print NCMFILE "\n### ******************************* ###\n";
print NCMFILE "### *** END AUTO-GENERATED PART *** ###\n";
print NCMFILE "### ******************************* ###\n\n";

print "\n*** NCM Overrides are in $NCM_OVERRIDES\n\n";

