#!/usr/bin/perl

push (@INC, "../data");

require("dashify.pl");
require("getPoolNodes.pl");

sub generateAppList
{
    my ($inv) = @_;
    my @invprod = @{$inv};

    my %appPoolList = &getAppPoolList;

    while ((my $app, my $n) = each(%appPoolList))
    {
        my @nodeList = @{$n};

        $nodes = &dashifyList(sort @nodeList);

        print NCMFILE "$nodes, app, tseshared $app, activate, Y\n";
    }
}

sub generateLimitedNodes
{
    my ($inv) = @_;
    my @invprod = @{$inv};

    foreach my $poolName (@atseV2Apps)
    {
        $appName = $deployList{$poolName};

        my @limitedNodes = &getPoolNodes($poolName, "limited", \@invprod);

        $nodes = &dashifyList(sort @limitedNodes);

        print NCMFILE "# $friendlyPoolName{$poolName} pool:\n" if ($appName eq "shopping" || $appName eq "shoppingis");
        $varName = "ADMIN_" . uc $appName . "_IS_LIMITED_NODE";
        $varName =~ s/PRICINGV2/PRICING/;

        print NCMFILE "$nodes, config.vars, $appName, $varName, \"Y\"\n";
    }
}

sub generateShoppingGroups
{
    my ($inv) = @_;
    my @invprod = @{$inv};

    foreach my $poolName (@atseV2Apps)
    {
        $appName = $deployList{$poolName};
        if ($appName eq "shopping" || $appName eq "shoppingis")
        {
            my @poolList = &getPoolNodes($poolName, "all", \@invprod);
            $nodes = &dashifyList(sort @poolList);
            $groupName = $shoppingPoolName{$poolName};
           
            $varName = "ADMIN_" . uc $appName . "_PROD_GROUP";

            print NCMFILE "$nodes, config.vars, $appName, $varName, \"$groupName\"\n";
        }
    }
}

1
