#!/usr/bin/perl

require("fixCsvFile.pl");

sub getBigIpList
{
    my %prodBigIPs;
    my %certBigIPs;

    &fixCsvFile($BIGIP_CSV, $FIXED_BIGIP_CSV);

    open(FIXEDBIGIP, "<", $FIXED_BIGIP_CSV) or die "Cannot open $FIXED_BIGIP_CSV.\n", $!;

    my @lines = (<FIXEDBIGIP>);

    foreach my $line (@lines)
    {
        my @lineList = split(/,/,$line);
        s/^\s+// foreach @lineList;
        s/\s+$// foreach @lineList;

        my $app = $lineList[0];
        $prodBigIPs{$app} = $lineList[1];

        my $limitedApp = $app . "-limited";
        $prodBigIPs{$limitedApp} = $lineList[1] . " " . $lineList[2];

        $certBigIPs{$app} = $lineList[3];
    }

    return (\%prodBigIPs, \%certBigIPs);
}

sub getNodeBigIp
{
    my ($nodeName, $poolName) = @_;

    my $limited = &isNodeLimited($nodeName, \@invprod);

    $key = $poolName;
    $key .= "-limited" if $limited == 1;
    $key .= "-" . $nodeName if $nodeName eq "pinlc116";

    my $result = $bigIpList{$key};
    die "Cannot determine big ip information for $nodeName in $poolName.\n" if length($result) == 0;

    return $result;
}

1
