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
$tmpDir = $baseDir . "/" . getlogin() . ".Drafted_CRs";
$fixedDir = $baseDir . "/" . getlogin() . ".input";
$INVPROD = "data/invprod.csv";
$FIXED_INVPROD = "$fixedDir/invprod_fixed.csv";
$BIGIP_CSV = "data/big_ips.csv";
$FIXED_BIGIP_CSV = "$fixedDir/big_ips_fixed.csv";
$CERT_CSV = "data/cert_nodes.csv";
$FIXED_CERT_CSV = "$fixedDir/cert_nodes_fixed.csv";

$WARNINGS="* Make sure CheckInvprod.pl was executed before running this script!\n";

$PREDEPLOY_INSTRUCTIONS = "$tmpDir/PREDEPLOY_INSTRUCTIONS.TXT";
$PREDEPLOY_ASSETS = "$tmpDir/PREDEPLOY_ASSETS.TXT";
$PREDEPLOY_FALLBACK = "$tmpDir/PREDEPLOY_FALLBACK.TXT";

$REMAINING_NODES_INSTRUCTIONS = "$tmpDir/REMAINING_NODES_INSTRUCTIONS.TXT";
$REMAINING_NODES_FALLBACK = "$tmpDir/REMAINING_NODES_FALLBACK.TXT";
$REMAINING_NODES_ASSETS = "$tmpDir/REMAINING_NODES_ASSETS.TXT";

$LIMITED_NODES_INSTRUCTIONS = "$tmpDir/LIMITED_NODES_INSTRUCTIONS.TXT";
$LIMITED_NODES_FALLBACK = "$tmpDir/LIMITED_NODES_FALLBACK.TXT";
$LIMITED_NODES_ASSETS = "$tmpDir/LIMITED_NODES_ASSETS.TXT";

$ALL_NODES_INSTRUCTIONS = "$tmpDir/ALL_NODES_INSTRUCTIONS.TXT";
$ALL_NODES_FALLBACK = "$tmpDir/ALL_NODES_FALLBACK.TXT";
$ALL_NODES_ASSETS = "$tmpDir/ALL_NODES_ASSETS.TXT";

$CERT_INSTRUCTIONS = "$tmpDir/CERT_INSTRUCTIONS.TXT";
$CERT_FALLBACK = "$tmpDir/CERT_FALLBACK.TXT";
$CERT_ASSETS = "$tmpDir/CERT_ASSETS.TXT";

$NCM_OVERRIDES = "$tmpDir/NCM.TXT";

@cr_assets;
@cr_bigIpAssets;

sub cleanup
{
    unlink $PREDEPLOY_INSTRUCTIONS, $PREDEPLOY_ASSETS, $PREDEPLOY_FALLBACK;
    unlink $REMAINING_NODES_INSTRUCTIONS, $REMAINING_NODES_FALLBACK, $REMAINING_NODES_ASSETS;
    unlink $LIMITED_NODES_INSTRUCTIONS, $LIMITED_NODES_FALLBACK, $LIMITED_NODES_ASSETS;
    unlink $ALL_NODES_INSTRUCTIONS, $ALL_NODES_FALLBACK, $ALL_NODES_ASSETS;
    unlink $CERT_INSTRUCTIONS, $CERT_FALLBACK, $CERT_ASSETS, $NCM_OVERRIDES;
    unlink $FIXED_INVPROD, $FIXED_BIGIP_CSV, $FIXED_CERT_CSV;
}

sub DIE_handler 
{
    &cleanup;
    die "\n!!! ", @_;
}

$SIG{__DIE__}  = 'DIE_handler';

sub usage
{
    print "\nUsage: $0 <new release label> <old release label> CR_Number PROD|CERT \"<space separated delete stuff>\"\n\n";
    print "Example: $0 atsev2.2010.00.01 atsev2.2009.12.01 123456 PROD \"*.atsev2.2009.0* *.atsev2.2009.10.*\"\n\n";
    exit 1;
}
 
sub createAssetList
{
    my ($file) = @_;
    open(ASSETS, "> $file") or die "Cannot open $file.\n", $!;

    my %hash = map {$_, 1} @cr_assets;
    my @assets = sort(keys %hash);
    print ASSETS $_, "\n" foreach (@assets);

    $bigIpCnt = 0;

    %hash = map {$_, 1} @cr_bigIpAssets;
    my @bigIpAssets = sort(keys %hash);
	print ASSETS $_, "\n" foreach (@bigIpAssets);

    close ASSETS;

    my $size = @assets;
    my $bigIpCnt = @bigIpAssets;
    print "\nTotal Assets in the CR is ", $size + $bigIpCnt, " ($size nodes and $bigIpCnt BigIP boxes.)\n";
    
    undef(@cr_assets);
    undef(@cr_bigIpAssets);
}

sub processBigIpAssets
{
    my ($bigIp) = @_;
    my @bigIps = split(/\s+/, $bigIp);

    foreach (@bigIps)
    {
        my @t = split(/:/, $_);
        my $node = $t[0] if $#t > 0;
        push(@cr_bigIpAssets, $node);
        if ($node eq "d3pindislb01")
        {
            push(@cr_bigIpAssets, "D3PINDISLB01P - 6800 F5");
            push(@cr_bigIpAssets, "D3PINDISLB01S - 6800 F5");
        }
        else
        {
            push(@cr_bigIpAssets, $node . "P");
            push(@cr_bigIpAssets, $node . "S");
        }
    }
}

sub preDeploy
{
    print "\n*** Creating predeploy $action CR for release $newRelease ***\n\n";

    my %appPoolList = &getAppPoolList;

    while ((my $app, my $n) = each(%appPoolList))
    {
        my @nodes = @{$n};
        push(@cr_assets, @nodes);
    }
    
    $appPoolList{"tseshared"} = [@cr_assets];

    open(OUTFILE, "> $PREDEPLOY_INSTRUCTIONS") or die "Cannot open $PREDEPLOY_INSTRUCTIONS.\n", $!;
    print OUTFILE "\nOn $cc_node, using C&C, do the following:\n";

    $total = 0;
    my $tsesharedList;
    print "\n";
    $WARNINGS .= "* No old baselines will be deleted since command line parameter was not specified!\n" if length($deleteStuff) == 0;

    while ((my $app, my $nodes) = each(%appPoolList))
    {
        print OUTFILE "\n** Deploy $app **\n";

        my %hash = map {$_, 1} @{$nodes};
        my @poolList = sort(keys %hash);

        my $count = @poolList;
        $total += $count;
        print "$app has total $count nodes\n";

        print OUTFILE "Command> clear\n";

        my $nodeList = &dashifyList(@poolList);
        print OUTFILE "Command> set node $nodeList\n";
        print OUTFILE "Command> show\n";
        print OUTFILE "(confirm correct nodes are set)\n";

        if ($app eq "tseshared")
        {
            $tsesharedList = $nodeList;
            $deleteStuff =~ s/\/opt\/atseintl\///g;
            if (length($deleteStuff) > 0)
            {
                print OUTFILE "\nDelete old V2 baselines:\n";
                my @deleteList = split(/\s+/, $deleteStuff);
                print OUTFILE "Command> remote rm -rf /opt/atseintl/$_\n" foreach (@deleteList);
                print OUTFILE "\n";
            }
        }

        print OUTFILE "Command> deploy $app.$newRelease\n";
        print OUTFILE "select the number that matches $app.$newRelease\n\n";
   
        print OUTFILE "Verify that baseline exists and finished deploying\n";
        print OUTFILE "Command> remote ls -la /opt/atseintl/$app.$newRelease/.setup\n\n";
   
        print OUTFILE "An \"OK\" return code indicates success. Errors are reported with a longer message like this:\n\n";
  
        print OUTFILE "node0999 - remote [1] - Remote command [ls -la /opt/atseintl/$app.$newRelease/.setup] returned non-zero exit code [1].\n";
        print OUTFILE "ls: /opt/atseintl/$app.$newRelease/.setup: No such file or directory\n";
        print OUTFILE "node0999 - remote [1] - Non-zero exit code from remote command.\n\n";

        print OUTFILE "If an error is encountered, check for sufficient disk space and try the deploy again to just the failed node.\n";
    }

    close OUTFILE; 

    open(OUTFILE, "> $PREDEPLOY_FALLBACK") or die "Cannot open $PREDEPLOY_FALLBACK.\n", $!;
    print OUTFILE "\n\n***Deploy Fallback ***\n\n";
    print OUTFILE "On $cc_node, as hybfunc, using C&C, do the following:\n\n";
    print OUTFILE "Command> clear\n";
    print OUTFILE "Command> set node $tsesharedList\n";
    print OUTFILE "Command> show\n";
    print OUTFILE "(confirm correct nodes are set)\n";
    print OUTFILE "Command> remote rm -rf /opt/atseintl/*.$newRelease\n\n";

    close OUTFILE;


    &createAssetList("$PREDEPLOY_ASSETS");
}

sub acmsInstructions
{
    print "\n*** Creating $action acms instructions ***\n";

    my $acms1 = "piflp001";
    my $acms2 = "piclp483";

    if ($action eq "CERT")
    {
        $acms1 = "pchlc001";
        $acms2 = "pchlc002";
    }

    print OUTFILE "\n*** Promote ACMS configuration ***\n";

    print OUTFILE "\nOn $acms1, as hybfunc, issue the following command and enter EDS User's password when prompted:\n";
    print OUTFILE "# /opt/atse/acmsconsole/scripts/promote_prod.sh $newRelease\n";

    print OUTFILE "\nLog into PROD ACMS as guest (password guest) using any from the following links:\n";
    print OUTFILE "http://$acms1.sabre.com:8085/acmsgui\n";
    print OUTFILE "http://$acms2.sabre.com:8085/acmsgui\n";
    print OUTFILE "\nGo to the Baselines panel and verify if the promoted baseline is visible in the Baselines list.\n";
}

sub activateInstructions
{
    print OUTFILE "Command> set app <Group Application>\n";
    print OUTFILE "Command> set node <Group Nodes>\n";
    print OUTFILE "Command> set baseline @_ \n";
    print OUTFILE "Command> show\n";
    print OUTFILE "Review filters to ensure that only these nodes and applications are affected.\n";
    print OUTFILE "Command> activate baseline\n";
    print OUTFILE "Command> ncmpush\n";
    print OUTFILE "Command> start\n";
    print OUTFILE "Verification Plan:\n"; 
    print OUTFILE " Verify the nodes are enabled in the <Group BigIP> pools.\n";
    print OUTFILE " Verify that there are no errors.\n";
    print OUTFILE " Ensure that each application comes up in AppConsole.\n";
    print OUTFILE " Ensure that each application starts processing traffic.\n";
    print OUTFILE " If there are any issues, please escalate to System Owner.\n";
    print OUTFILE "\nPerform a ConfigSync on the active F5 device(s) if any changes were performed.\n";
}

sub remainingNodes
{
    my($includeAllNodes) = @_;

    my $wording = "Remaining";
    my $INSTRUCTIONS = $REMAINING_NODES_INSTRUCTIONS;
    my $FALLBACK = $REMAINING_NODES_FALLBACK;
    my $ASSETS = $REMAINING_NODES_ASSETS;

    if ($includeAllNodes == 1)
    {
        $wording = "All";
        $INSTRUCTIONS = $ALL_NODES_INSTRUCTIONS;
        $FALLBACK = $ALL_NODES_FALLBACK;
        $ASSETS = $ALL_NODES_ASSETS;

        &acmsInstructions();
        &ncmInstructions(0);
    }

    print "\n*** Creating $wording nodes $action CR for release $newRelease ***\n";

    open(OUTFILE, "> $INSTRUCTIONS") or die "Cannot open $INSTRUCTIONS.\n", $!;

    if ($includeAllNodes == 1)
    {
        &acmsInstructions();
        &ncmInstructions(0);
    }

    my @appPoolGroups;
    my $maxPoolGroup = 0;
    my %poolGroupCounts;
    
    foreach my $poolName (@atseV2Apps)
    {
        my @groupList = &getPoolGroups($poolName, $includeAllNodes, \@invprod);
        
        if ($includeAllNodes == 1)
        {
            my @limitedNodes = &getPoolNodes($poolName, "limited", \@invprod);
            unshift(@groupList, [@limitedNodes]) if $#limitedNodes >= 0;
        }

        my $count = @groupList;

        $maxPoolGroup = $count if $count > $maxPoolGroup;
        $appPoolGroups{$poolName} = [@groupList];
        $poolGroupCounts{$poolName} = $count;
    }

    print OUTFILE "\n*** $wording Nodes Groups ***\n\n";

    # Sort pools by group size
    my @sortedPools;
    foreach my $key (sort {$poolGroupCounts{$a} <=> $poolGroupCounts{$b}} keys %poolGroupCounts)
    {
        push(@sortedPools, $key);
    }

    my $groupNum = 1;
    my $iteration = 0;
    my %groupProbability;

    # Make sure every single node is included (in case the algorithm to distribute groups evenly fails)
    my $finished = 0;
    while ($finished == 0)
    {
        $finished = 1;

        foreach my $poolName (@sortedPools)
        {
            my @poolGroups = @{$appPoolGroups{$poolName}};
            my $count = @poolGroups;
            $finished = 0 if $count > 0;
            my $doTheGroup = 0;

            # Include all groups in first and last iterations
            if (($count > 0) && (($iteration == 0) || ($iteration >= $maxPoolGroup-1)))
            {
                $doTheGroup = 1;
            }
            elsif ($count > 1)
            {
                $groupProbability{$poolName} += ($poolGroupCounts{$poolName}-1)/($maxPoolGroup-1);
                if ($groupProbability{$poolName} >= 1)
                {
                    $doTheGroup = 1;
                    $groupProbability{$poolName} -= 1;
                } 
            }

            if ($doTheGroup == 1)
            {
                my $app = $deployList{$poolName};
            
                my @groupNodes = @{shift @poolGroups};
                $appPoolGroups{$poolName} = [@poolGroups];
                my $bigIps = &getNodeBigIp($groupNodes[0], $poolName);
                &processBigIpAssets($bigIps);

                push(@cr_assets, @groupNodes);

                print OUTFILE "Group ", $groupNum++, ": $friendlyPoolName{$poolName} Pool, Application: $app\n";
                print OUTFILE "  BigIP: $bigIps\n";
                print OUTFILE "  Nodes: ", &dashifyList(sort @groupNodes), "\n\n";
            }
        }

        $iteration++;
    }

    print OUTFILE "\n*** Instructions ***\n\n";
    print OUTFILE "On $cc_node, using C&C, for each group, do the following:\n\n";

    &activateInstructions($newRelease);

    close OUTFILE;

    open(OUTFILE, "> $FALLBACK") or die "Cannot open $FALLBACK.\n", $!;
    &ncmInstructions(1) if ($includeAllNodes == 1);

    print OUTFILE "*** Baseline Fallback ***\n\n";
    print OUTFILE "For all groups above, on $cc_node, as hybfunc, using C&C, do the following:\n\n";
    &activateInstructions($oldRelease);
    close OUTFILE;
   
    &createAssetList("$ASSETS");
}

sub limitedNodeHelper
{
    my ($nodeType, $group) = @_;

    my $totalCount = 0;

    foreach my $poolName (@atseV2Apps)
    {
        my $deployApp = $deployList{$poolName};
        my @poolList = &getPoolNodes($poolName, $nodeType, \@invprod);
        my $count = @poolList;

        if ($count == 0)
        {
            $WARNINGS .= "* $poolName pool does not have any $nodeType nodes!\n";
            next;
        }

        $totalCount += $count;

        push(@cr_assets, @poolList);

        my $bigIps = &getNodeBigIp($poolList[0], $poolName);
        &processBigIpAssets($bigIps);

        print "Adding $count $poolName $nodeType nodes\n";
        print OUTFILE "Group ", $group++, " : $friendlyPoolName{$poolName} Pool, Application: $deployApp\n";
        print OUTFILE "  BigIP: $bigIps\n";
        print OUTFILE "  Nodes: ", &dashifyList(@poolList), "\n\n";

        undef(@poolList);
    }

    print "Total $nodeType node count is ", $totalCount, "\n";

    return $group;
}

sub ncmInstructions
{
    my ($fallback) = @_;
    print "\n*** Creating $action ncm instructions ***\n";

    print OUTFILE "\n*** NCM Changes ***\n\n" if $fallback == 0;
    print OUTFILE "\n*** NCM Fallback ***\n\n" if $fallback == 1;
    print OUTFILE "On $cc_node, as hybfunc:\n";
    print OUTFILE "# cd /tmp/sabre/ncm/$cr_number\n";
    print OUTFILE "# cp /opt/atse/ncm.csv .\n";
    print OUTFILE "# patch -b -z .bak ncm.csv patch.txt\n\n" if $fallback == 0;
    print OUTFILE "# patch -R -b -z .bak ncm.csv patch.txt\n\n" if $fallback == 1;

    print OUTFILE "Verify proposed NCM changes are as expected:\n";
    print OUTFILE "# diff /opt/atse/ncm.csv ./ncm.csv\n" if $fallback == 0;
    print OUTFILE "# diff ./ncm.csv /opt/atse/ncm.csv\n" if $fallback == 1;
    print OUTFILE "The differences should match \"diff.txt\" (line numbers may differ).\n\n";

    print OUTFILE "Validate NCM returns no errors:\n";
    print OUTFILE "# /opt/atse/nodeagent/validator.sh file=\$PWD/ncm.csv\n";
    print OUTFILE "<...>\nvalidator.sh: Encountered no errors. The file is valid.\n\n";

    print OUTFILE "Import the NCM changes using C&C on $cc_node:\n";
    print OUTFILE "Command> clear\n";
    print OUTFILE "Command> import ncm /tmp/sabre/ncm/$cr_number/ncm.csv\n";
    print OUTFILE "Verify that there are no errors.\n\n";

    push(@cr_assets, $cc_node);
}

sub limitedNodes
{
    print "\n*** Creating limited nodes $action CR for release $newRelease ***\n\n";

    open(OUTFILE, "> $LIMITED_NODES_INSTRUCTIONS") or die "Cannot open $LIMITED_NODES_INSTRUCTIONS.\n", $!;
   
    &acmsInstructions; 
    &ncmInstructions(0);

    print OUTFILE "\n*** Limited Node Groups ***\n\n";
    
    my $group = 1;
    $group = &limitedNodeHelper("limited", $group);
    my $lastLimitedGroup = $group-1;

    print OUTFILE "\n*** Limited Node Instructions ***\n\n";
    print OUTFILE "On $cc_node, using C&C, for each group, do the following:\n\n";

    &activateInstructions($newRelease);

    print "\n";

    print OUTFILE "\n\n*** Comparison Node Groups ***\n\n";
    &limitedNodeHelper("comparison", $group);

    print OUTFILE "\n*** Comparison Node Instructions ***\n\n";

    print OUTFILE "On $cc_node, using C&C, for each group, do the following:\n\n";
    print OUTFILE "Command> set app <Group Application>\n";
    print OUTFILE "Command> set node <Group Nodes>\n";
    print OUTFILE "Command> show\n";
    print OUTFILE "Review filters to ensure that only these nodes and applications are affected.\n";
    print OUTFILE "Command> bounce\n";
    print OUTFILE "Verification Plan:\n"; 
    print OUTFILE " Verify the nodes are enabled in the <Group BigIP> pools.\n";
    print OUTFILE " Verify that there are no errors.\n";
    print OUTFILE " Ensure that each application comes up in AppConsole.\n";
    print OUTFILE " Ensure that each application starts processing traffic.\n";
    print OUTFILE " If there are any issues, please escalate to System Owner.\n";




   
    close OUTFILE;

    open(OUTFILE, "> $LIMITED_NODES_FALLBACK") or die "Cannot open $LIMITED_NODES_FALLBACK.\n", $!;
    &ncmInstructions(1);

    print OUTFILE "*** Baseline Fallback ***\n";
    print OUTFILE "\nFor groups 1-$lastLimitedGroup, on $cc_node, as hybfunc, using C&C, do the following:\n\n";
    &activateInstructions($oldRelease);
    
    close OUTFILE;
    
    &createAssetList("$LIMITED_NODES_ASSETS");
}

sub checkArguments
{
    return 0 if (length($newRelease) == 0) || (length($oldRelease) == 0);
    return 0 if $cr_number =~ m/\D/;
    return 0 if $action ne "CERT" && $action ne "PROD"; 

    return 1;
}

sub generateNCMInfo
{
    open(NCMFILE, "> $NCM_OVERRIDES") or die "Cannot open $NCM_OVERRIDES.\n", $!;
    print NCMFILE "\n### *** BEGIN AUTO-GENERATED LIMITED NODES INFO *** ###\n\n";
 
    generateBigIPInfo($NCM_OVERRIDES, \%bigIpList, \@invprod);
    generateCoreNodes($NCM_OVERRIDES, \%bigIpList, \@invprod);

    print NCMFILE "\n### *** END AUTO-GENERATED LIMITED NODES INFO *** ###\n\n";
    close NCMFILE;
}

### Main ###

$newRelease = $ARGV[0];
$oldRelease = $ARGV[1];
$cr_number = $ARGV[2];
$action =  $ARGV[3];
$deleteStuff = $ARGV[4];

&usage if &checkArguments == 0;

&cleanup;
mkdir $baseDir;
chmod 0777, $baseDir;
mkdir $tmpDir;
mkdir $fixedDir;

my ($prodBigIPs, $certBigIPs) = &getBigIpList;

if ($action eq "CERT")
{
    @invprod = &processInvProd($CERT_CSV, $FIXED_CERT_CSV);
    @atseV2Apps = @certApps;
    %bigIpList = %$certBigIPs;
    $cc_node = "pinlc101";
    $action = "CERT";

    &preDeploy;
    &remainingNodes(1);

    system("mv $PREDEPLOY_INSTRUCTIONS $CERT_INSTRUCTIONS");
    system("cat $ALL_NODES_INSTRUCTIONS >> $CERT_INSTRUCTIONS");

    system("mv $ALL_NODES_FALLBACK $CERT_FALLBACK");
    system("cat $PREDEPLOY_FALLBACK >> $CERT_FALLBACK");

    system("mv $ALL_NODES_ASSETS $CERT_ASSETS");

    unlink $ALL_NODES_INSTRUCTIONS, $PREDEPLOY_ASSETS, $PREDEPLOY_FALLBACK;
    
    print "\n*** CERT CR instructions are in $CERT_INSTRUCTIONS\n";
    print "*** CERT fallback is in $CERT_FALLBACK\n";
    print "*** CERT asset list is in $CERT_ASSETS\n\n";

    exit(0);
}

# Prod CRs
@invprod = &processInvProd($INVPROD, $FIXED_INVPROD);
@atseV2Apps = @prodApps;
%bigIpList = %$prodBigIPs;
$cc_node = "piflp001";

&preDeploy;
&limitedNodes;
&remainingNodes(0);
&remainingNodes(1);
&generateNCMInfo();

print "\n*** Predeploy instructions are in $PREDEPLOY_INSTRUCTIONS\n";
print "*** Predeploy fallback is in $PREDEPLOY_FALLBACK\n";
print "*** Predeploy asset list is in $PREDEPLOY_ASSETS\n\n";

print "*** Limited nodes instructions are in $LIMITED_NODES_INSTRUCTIONS\n";
print "*** Limited nodes fallback is in $LIMITED_NODES_FALLBACK\n";
print "*** Limited nodes asset list is in $LIMITED_NODES_ASSETS\n\n";

print "*** Remaining nodes CR instructions are in $REMAINING_NODES_INSTRUCTIONS\n";
print "*** Remaining nodes fallback is in $REMAINING_NODES_FALLBACK\n";
print "*** Remaining nodes asset list is in $REMAINING_NODES_ASSETS\n\n";

print "*** All nodes CR instructions are in $ALL_NODES_INSTRUCTIONS\n";
print "*** All nodes fallback is in $ALL_NODES_FALLBACK\n";
print "*** All nodes asset list is in $ALL_NODES_ASSETS\n\n";

print "*** NCM Overrides are in $NCM_OVERRIDES\n\n";

print "\n!* WARNINGS *!\n\n$WARNINGS\n";
