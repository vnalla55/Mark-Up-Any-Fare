#!/usr/bin/perl

push (@INC, "../data");

require("dashify.pl");
require("getPoolNodes.pl");
require("getBigIpList.pl");
require("processInvProd.pl");
require("getPoolGroups.pl");
require("AppList.pl");

sub processBigIp
{
    my ($bigIp) = @_;
    my @bigIps = split(/\s+/, $bigIp);
    my $result;

    foreach (@bigIps)
    {
        my @t = split(/:/, $_);
        my $node = $t[0] if $#t >= 0;
        my $pool = $t[1] if $#t >= 1;
       
        $result = $result . " " . $pool;
    }

    $result =~ s/^\s//;
    return $result;
}

sub bigIpNcmLine
{
    my ($appName, $bigIp, $nodes) = @_;
    @nodeList = @{$nodes};
    
    $varName = "ADMIN_" . uc $appName . "_BIG_IP_POOL";
    $varName = "ADMIN_PRICING_BIG_IP_POOL" if $appName eq "pricingv2";

    $nodes = "*";
    $nodes = &dashifyList(sort @nodeList) if $#nodeList >= 0;
    
    print NCMFILE $nodes, ", config.vars, $appName, $varName, \"$bigIp\"\n";        
}

sub generateBigIPInfo
{
    my($NCM_OVERRIDES, $bigIps, $inv) = @_;
    my %bigIpList = %{$bigIps};
    my @invprod = @{$inv};
    
    print NCMFILE "################################################\n";
    print NCMFILE "# BIGIP INFO:\n";
    print NCMFILE "################################################\n";
    foreach my $poolName (@atseV2Apps)
    {
        $appName = $deployList{$poolName};

        my @poolList = &getPoolNodes($poolName, "all", \@invprod);
        my @limitedNodes = &getPoolNodes($poolName, "limited", \@invprod);
        my @comparisonNodes =  &getPoolNodes($poolName, "comparison", \@invprod);
        my @remainingNodes = &getPoolNodes($poolName, "remaining", \@invprod);

        $limitedBigIp = &processBigIp(&getNodeBigIp($limitedNodes[0], $poolName));
        $remainingBigIp = &processBigIp(&getNodeBigIp($comparisonNodes[0], $poolName));

        print NCMFILE "\n# $friendlyPoolName{$poolName} BigIP:\n";

        if ($limitedBigIp eq $remainingBigIp)
        {
            if ($appName ne "shopping" && $appName ne "shoppingis")
            {
                &bigIpNcmLine($appName, $limitedBigIp);
            }
            else
            {
                &bigIpNcmLine($appName, $limitedBigIp, \@poolList);
            }
        }
        else
        {
            if ($appName ne "shopping" && $appName ne "shoppingis")
            {
                &bigIpNcmLine($appName, $remainingBigIp);
            }
            else
            {
                &bigIpNcmLine($appName, $remainingBigIp, \@remainingNodes);
            }

            &bigIpNcmLine($appName, $limitedBigIp, \@limitedNodes);
        }
    }
}

sub generateCoreNodes
{
    my($NCM_OVERRIDES, $bigIps, $inv) = @_;
    my %bigIpList = %{$bigIps};
    my @invprod = @{$inv};

    print NCMFILE "\n\n################################################\n";
    print NCMFILE "# Enable Cores on Limited Nodes:\n";
    print NCMFILE "################################################\n";
    foreach my $poolName (@atseV2Apps)
    {
        $appName = $deployList{$poolName};

        my @limitedNodes = &getPoolNodes($poolName, "limited", \@invprod);

        $nodes = &dashifyList(sort @limitedNodes);

        print NCMFILE "# $friendlyPoolName{$poolName} pool:\n";
        print NCMFILE "$nodes, config.vars, $appName, ADMIN_ULIMIT_CORES, \"unlimited\"\n";
    }
}

# print "\n\n################################################\n";
# print "# D-Pool overrides\n";
# print "################################################\n";
# foreach my $poolName (@atseV2Apps)
# {
#     next if ($poolName ne "ShopISLMC" && $poolName ne "ShopMIPLMC");
#
#     my @poolList = &getPoolNodes($poolName, "all", \@invprod);
#     $appName = $deployList{$poolName};
#
#     $nodes = &dashifyList(sort @poolList);
#     print "$nodes, config.vars, $appName, ADMIN_SHOPPING_OVERRIDE_CFG, D\n";
# }
#
# %appConNames = (
#               "ShopIS200", "C-ShoppingIS200",
#               "ShopISLMC", "D-ShoppingIS200",
#               "ShopISTVCY", "E-ShoppingIS200",
#               "ShopMIP200", "C-ShoppingMIP200",
#               "ShopMIPLMC", "D-ShoppingMIP200",
#               "ShopMIPTVCY", "E-ShoppingMIP200"
#              );
#
# print "\n\n################################################\n";
# print "# Application console settings\n";
# print "################################################\n";
# foreach my $poolName (@atseV2Apps)
# {
#     $appConGroup = $appConNames{$poolName};
#    next if length($appConGroup) == 0;
#
#    my @poolList = &getPoolNodes($poolName, "all", \@invprod);
#    $appName = $deployList{$poolName};
#
#    $nodes = &dashifyList(sort @poolList);
#    $cfgName = "ADMIN_" . uc $appName . "_OVERRIDE_CFG";
#    print "$nodes, config.vars, $appName, $cfgName, $appConGroup\n";
# }

1
