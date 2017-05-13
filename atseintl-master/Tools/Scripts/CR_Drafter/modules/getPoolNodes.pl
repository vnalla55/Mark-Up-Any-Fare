#!/usr/bin/perl

sub getPoolNodesList
{
    my($poolName, $invprod) = @_;
    my @result;
    my @lines = @{$invprod};

    foreach my $line (@lines)
    {
        my @list = @{$line};
        my @apps = split(/\//, $list[3]);

        if (grep {$_ eq $poolName} @apps)
        {
            push(@result, [@list]);
        }
    }

    return @result;
}

sub isNodeLimited
{
    my($nodeName, $invprod) = @_;

    my @lines = @{$invprod};

    foreach my $line (@lines)
    {
        my @list = @{$line};
        return 1 if ($list[0] eq $nodeName) && ($list[6] eq "X");
    }

    return 0;
}

sub getPoolNodes
{
    my($poolName, $nodeType, $invprod) = @_;
    my @result;

    my @poolList = &getPoolNodesList($poolName, $invprod);

    foreach my $line (@poolList)
    {
        my @list = @{$line};

        if ( (($nodeType eq "limited") && ($list[6] eq "X")) ||
             (($nodeType eq "comparison") && ($list[6] eq "C")) ||
             (($nodeType eq "remaining") && ($list[6] ne "X")) ||
              ($nodeType eq "all") )
        {
            push(@result, $list[0]);
        }
    }
 
    return @result;
}

sub getAppPoolList
{
    my $total = 0;
    my %appPoolList;

    foreach my $pool (@atseV2Apps)
    {
        my $deployApp = $deployList{$pool};

        my @poolList = &getPoolNodes($pool, "all", \@invprod);
        my @nodes = @{$appPoolList{$deployApp}};

        push(@nodes, @poolList);

        $appPoolList{$deployApp} = [@nodes];

        undef(@poolList);
    }

    return %appPoolList;
}

1
