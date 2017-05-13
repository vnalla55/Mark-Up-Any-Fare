#!/usr/bin/perl

push (@INC, "./modules", "./data");

require("dashify.pl");
require("getPoolNodes.pl");
require("processInvProd.pl");
require("getPoolGroups.pl");
require("AppList.pl");

$INVPROD = "data/invprod.csv";
$FIXED_INVPROD = "/tmp/$$.invprod_fixed.csv";
%podCounts;

sub numLimited
{
    my ($poolCount) = @_;
    
    return 1 if ($poolCount <= 9);
    return 2 if ($poolCount <= 14);
    return int (($poolCount/10) + .5);
}

## Main ##

@invprod = &processInvProd($INVPROD, $FIXED_INVPROD);
@atseV2Apps = @prodApps;

foreach my $poolName (@atseV2Apps)
{
    print "\n\n**** $poolName pool Info ****:\n";

    my @poolList = &getPoolNodes($poolName, "all", \@invprod);
    my @limitedNodes = &getPoolNodes($poolName, "limited", \@invprod);
    my @comparisonNodes =  &getPoolNodes($poolName, "comparison", \@invprod);
    my @remainingNodes = &getPoolNodes($poolName, "remaining", \@invprod);

    my $poolCount = @poolList;
    my $limitedCount = @limitedNodes;
    my $comparisonCount = @comparisonNodes;
    my $correctCount = &numLimited($poolCount);

    print "\n$poolCount nodes, $limitedCount limited nodes, $comparisonCount comparison nodes.\n";

    print "!!! ERROR: Limited not equal to comp !!!\n" if $limitedCount != $comparisonCount;
    print "!!! ERROR: Count should have been $correctCount !!!\n" if  $limitedCount != $correctCount;

    print "\nNode List:\n";

    print "All       : ", &dashifyList(sort @poolList), "\n";
    print "Limited   : ", &dashifyList(sort @limitedNodes), "\n";
    print "Comparison: ", &dashifyList(sort @comparisonNodes), "\n";
    print "Remaining : ", &dashifyList(sort @remainingNodes), "\n";

    my %hwList;
    my %lmtHwList;
    my %cmpHwList;
    my $maxPetalCount = 0;
    my $remainingNodeCount = 0;
    @poolList = &getPoolNodesList($poolName, \@invprod);

    foreach my $line (@poolList)
    {
        my @list = @{$line};

        my $node = $list[0];
        my $pod = $list[7];

        my $hw = $list[25];
        $hwList{$hw}++;
        $lmtHwList{$hw}++ if ($list[6] eq "X");
        $cmpHwList{$hw}++ if ($list[6] eq "C");
         
        print "!!! ERROR! Column Q (trx count) is missing for $node !!!" if (length($list[16]) == 0);

        $podCounts{$pod}++;
    }
    
    print "\nHardware types:\n";
    while ( my($hw, $count) = each(%hwList) ) 
    {
        print "$hw : $count nodes (%", int($count*100/$poolCount), ")\n";
    }

    print "\nLimited node hardware types:\n";
    while ( my($hw, $count) = each(%lmtHwList) )
    {
        print "$hw : $count nodes (%", int($count*100/$limitedCount), ")\n";
    }

    print "\nComparison node hardware types:\n";
    while ( my($hw, $count) = each(%cmpHwList) )
    {
        print "$hw : $count nodes (%", int($count*100/$comparisonCount), ")\n";
    }

    while ( my($hw, $count) = each(%lmtHwList) )
    {
        if ($cmpHwList{$hw} != $count)
        {
            print "!!! ERROR: Limited/Comparison hardware mismatch !!!\n";
            last;
        }
    }

}

print "\n\nPods and connected petal counts:\n";
foreach my $pod (sort {$podCounts{$b} <=> $podCounts{$a}} keys %podCounts)
{
    print "$pod: $podCounts{$pod}\n";
}

unlink $FIXED_INVPROD
