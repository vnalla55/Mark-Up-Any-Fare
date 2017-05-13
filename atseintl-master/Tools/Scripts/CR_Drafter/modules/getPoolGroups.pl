#!/usr/bin/perl

push @INC,"../modules";

sub getPoolCapacity
{
    my($poolName, $invprod) = @_;

    my $poolCapacity = 0;
    my $nodeCount = 0;
    my @poolList = &getPoolNodesList($poolName, $invprod);

    foreach my $line (@poolList)
    {
        my @list = @{$line};
        my $trxCount = $list[16];
        $trxCount = 16 if (length($trxCount) == 0);
        $poolCapacity += $trxCount;
        $nodeCount++;
    }

    return ($poolCapacity, $nodeCount);
}

sub getPoolGroups
{
    my($poolName, $includeAllNodes, $invprod) = @_;
    my @result;
   
    my %podGroups;
    my %nodeTrxCount;
    my $maxPetalCount = 0;
    my $remainingNodeCount = 0;    
    my @poolList = &getPoolNodesList($poolName, $invprod);

    foreach my $line (@poolList)
    {
        my @list = @{$line};
        if ($list[6] ne "X") 
        {
            my $node = $list[0];
            my $pod = $list[7];
            my $trxCount = $list[16];
 
            $trxCount = 16 if (length($trxCount) == 0);

            $nodeTrxCount{$node} = $trxCount;
            
            my @podNodes = @{$podGroups{$pod}};
            push(@podNodes, $node);

            $remainingNodeCount++;
           
            my $count = @podNodes;
            $maxPetalCount = $count if $count > $maxPetalCount;
           
            $podGroups{$pod} = [@podNodes];
        }
    }
  
    my ($poolCapacity, $nodeCount) = &getPoolCapacity($poolName, \@invprod);
 
    my $tenPercentCapacity = int (($poolCapacity/10) + .5);
    
    print "\nTotal & Remaining node count in $poolName is $nodeCount & $remainingNodeCount\n" if $includeAllNodes == 0;
    print "\nTotal node count in $poolName is $nodeCount\n" if $includeAllNodes == 1;
    print "Maximum petal count per pod in $poolName is ", $maxPetalCount, "\n";

    my $avgCapacity = int (($poolCapacity/$nodeCount) + .5); 
    print "Total & Average pool capacity for $poolName is $poolCapacity & $avgCapacity concurrent transactions\n";

    my $groupCount = int (($tenPercentCapacity/$avgCapacity) + .5);
    $groupCount = 1 if $groupCount == 0;

    print "10% of capacity is $tenPercentCapacity concurrent transactions, about $groupCount nodes \n";

    my @podSortedPool;
    my @tempList;
    for (my $i = 0; $i < $maxPetalCount; $i++)
    {
	    while ((my $pod, my $nodes) = each(%podGroups))
	    {
            my @podNodes = @{$nodes};
            my $count = @podNodes;
            push(@tempList, $podNodes[$i]) if ($i < $count);
        }

        push(@podSortedPool, sort @tempList);
        undef @tempList;
    }

    my $count = @podSortedPool;
    my @groupNodes;
    my $groupCapacity = 0;
  
    for (my $i = 0; $i < $count; $i++) 
    {
        my $node = $podSortedPool[$i];
        $groupCapacity += $nodeTrxCount{$node};

        push(@groupNodes, $node);

        if ( ($groupCapacity >= $tenPercentCapacity) || ($i == $count-1) )
        {
            push(@result, [ @groupNodes ]);
            undef @groupNodes;
            $groupCapacity = 0;
        }
    }

    # Prevent non-homogenous group count in the last one
    my $size = @result;
    if ($size > 1)
    {
        my $last1 = @{$result[$size-1]};
        my $last2 = @{$result[$size-2]};

        my $diff = int (($last2-$last1)/2);
        if ($diff >= 3)
        {
            push(@{$result[$size-2]}, @{$result[$size-1]});
            pop @result;
        }
        else 
        {
            for (my $i = 0; $i < $diff; $i++)
            {
                push(@{$result[$size-1]}, pop @{$result[$size-2]});
            }
        }
    }
   
    my $size = @result;
    print "There are $size groups in pool $poolName\n";

    return @result;
}

1
