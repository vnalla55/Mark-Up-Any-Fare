package InstrDBStats;

use DBI;

my $INSTR_TABLE_PREFIX = "BASEINSTR";
my $FSR_INSTR_TABLE_PREFIX = "FSRINSTR";
my $debug = 0;

sub new
{
    my $package = shift;
    return bless({}, $package);
}


sub initConnection
{
    my $self = shift;
    my($host, $username, $password) = @_;

    my $dbh = DBI->connect($host, $username, $password)
       || die "Cannot connect to db: '$DBI::errstr'";
    print "Connected to database.\n" if($debug);
    return $dbh;
}

sub closeConnection
{
    my $self = shift;
    my $dbh = shift;
    $dbh->disconnect();
}

sub calculate_days
{
    my $self = shift;
    my($start, $end) = @_;
    my @days;

    # get day only from the input start and end
    my $_start = `date +%F -d \"$start\"`;
    my $_end = `date +%F -d \"$end\"`;

    my $date_counter=$_start;
    my $i=0;

    while($_end ge $date_counter)
    {
        $day = `date +\"%d\" -d \"$date_counter\"`;
        chomp($day);
        @days[$i++] = $day;
        $date_counter = `date +\"%F\" -d \"$_start +  $i day\"`;
    }

    return @days;
}

sub formatNamesToDB
{
    my $self = shift;
    my ($names) = @_;

    my @namesList = split(",", $names);
    my @namesMap = map{"'$_'"} @namesList;
    my $namesDB = join(",",@namesMap);
    return $namesDB;
}

sub formatNamesToDBWithDelimiter
{
    my $self = shift;
    my ($names, $delimiter) = @_;

    my @namesList = split($delimiter, $names);
    my @namesMap = map{"'$_'"} @namesList;
    my $namesDB = join(",",@namesMap);
    return $namesDB;
}

sub buildSql
{
    my $self = shift;
    my ($selectList, $day, $whereClause, $groupBy) = @_;
    my $tableName = $INSTR_TABLE_PREFIX . $day;
    my $sql = "SELECT " . $selectList;
    $sql .= " FROM " . $tableName;
    $sql .= " WHERE " . $whereClause;
    $sql .= " GROUP BY " . $groupBy if($groupBy ne "");
    return $sql;
}

# deprecated -- use getAvgStatsByMin 
sub getMinuteAvgStats
{
    my $self = shift;
    my ($dbh, $serverNames, $clientServiceNames, $startdate, $enddate) = @_;

    my $serverNames = $self->formatNamesToDB($serverNames);
    my $clientServiceNames = $self->formatNamesToDB($clientServiceNames);
    my @days = $self->calculate_days($startdate, $enddate);
    my $groupBy = "TIMESTAMP(DATE_FORMAT(TRANSACTIONDATETIME, '%Y-%m-%d %H:%i'))";
    my $selectList = " TIMESTAMP(DATE_FORMAT(TRANSACTIONDATETIME, '%Y-%m-%d %H:%i')) AS datetime";
    $selectList .= ", COUNT(*) AS cnt";
    $selectList .= ", AVG(elapsedtime) AS elapsed, AVG(cpuused) AS cpu";
    my $whereClause = "nodeid in (" . $serverNames . ")";
    $whereClause .= " and clientservicename in (" . $clientServiceNames . ")";
    $whereClause .= " and transactiondatetime > '" . $startdate . "'";
    $whereClause .= " and transactiondatetime < '" . $enddate . "'";
    $whereClause .= " and cpuused <> 0 ";

    $outputBuf = "DateTime,Count,Total Trx Count,Elapsed,CPU,Avg Elapsed,Avg CPU\n";
    $totalTrx = 0;
    $totalCpu = 0;
    $totalElapsed = 0;
    foreach my $day (@days)
    {
        print "processing for day : $day \n" if($debug);
        my $sql = $self->buildSql($selectList, $day, $whereClause, $groupBy);
        print "SQL : $sql\n" if($debug);
        my $sth=$dbh->prepare($sql);
        if (!$sth)
        {
            $message .= "Error in sql: $DBI::errstr";
            print "message : $message \n";
        }
        else
        {
            $sth->execute;
            while (@data = $sth->fetchrow_array())
            {
                $totalTrx += $data[1];
                $totalElapsed += ($data[1] * $data[2]);
                $totalCpu += ($data[1] * $data[3]);
                $avgElapsed = $totalElapsed / $totalTrx;
                $avgCpu = $totalCpu / $totalTrx;
                $outputBuf .= sprintf("%s,%d,%d,%.3f,%.3f,%.3f,%.3f\n",
                    $data[0],$data[1],$totalTrx,
                    $data[2]/1000000, $data[3]/1000000,
                    $avgElapsed/1000000, $avgCpu/1000000);
                    
            }
        }
        $sth->finish;
    }
    return $outputBuf;
}

sub getAvgStatsByMinute
{
    my $self = shift;
    my ($dbh, $serverNames, $clientServiceNames, $startdate, $enddate) = @_;

    return $self->getAvgStatsByTime($dbh, $serverNames, $clientServiceNames,
        $startdate, $enddate, 
        "TIMESTAMP(DATE_FORMAT(TRANSACTIONDATETIME, '%Y-%m-%d %H:%i'))");
}


sub getAvgStatsByHour
{
    my $self = shift;
    my ($dbh, $serverNames, $clientServiceNames, $startdate, $enddate) = @_;

    return $self->getAvgStatsByTime($dbh, $serverNames, $clientServiceNames,
        $startdate, $enddate, 
        "TIMESTAMP(DATE_FORMAT(TRANSACTIONDATETIME, '%Y-%m-%d %H'))");
}

sub getAvgStatsByTime
{
    my $self = shift;
    my ($dbh, $serverNames, $clientServiceNames, $startdate,
        $enddate, $grouping) = @_;

    my $serverNames = $self->formatNamesToDB($serverNames);
    my $clientServiceNames = $self->formatNamesToDB($clientServiceNames);
    my @days = $self->calculate_days($startdate, $enddate);
    my $groupBy = $grouping;
    my $selectList = " " . $grouping . " AS datetime";
    $selectList .= ", COUNT(*) AS cnt";
    $selectList .= ", AVG(elapsedtime) AS elapsed, AVG(cpuused) AS cpu";
    my $whereClause = "nodeid in (" . $serverNames . ")";
    if($clientServiceName ne "") {
      $whereClause .= " and clientservicename in (" . $clientServiceNames . ")";
    }
    $whereClause .= " and transactiondatetime > '" . $startdate . "'";
    $whereClause .= " and transactiondatetime < '" . $enddate . "'";
    $whereClause .= " and cpuused <> 0 ";

    $outputBuf = "DateTime,Count,Total Trx Count,Elapsed,CPU,Avg Elapsed,Avg CPU\n";
    $totalTrx = 0;
    $totalCpu = 0;
    $totalElapsed = 0;
    foreach my $day (@days)
    {
        print "processing for day : $day \n" if($debug);
        my $sql = $self->buildSql($selectList, $day, $whereClause, $groupBy);
        print "SQL : $sql\n" if($debug);
        my $sth=$dbh->prepare($sql);
        if (!$sth)
        {
            $message .= "Error in sql: $DBI::errstr";
            print "message : $message \n";
        }
        else
        {
            $sth->execute;
            while (@data = $sth->fetchrow_array())
            {
                $totalTrx += $data[1];
                $totalElapsed += ($data[1] * $data[2]);
                $totalCpu += ($data[1] * $data[3]);
                $avgElapsed = $totalElapsed / $totalTrx;
                $avgCpu = $totalCpu / $totalTrx;
                $outputBuf .= sprintf("%s,%d,%d,%.3f,%.3f,%.3f,%.3f\n",
                    $data[0],$data[1],$totalTrx,
                    $data[2]/1000000, $data[3]/1000000,
                    $avgElapsed/1000000, $avgCpu/1000000);
                    
            }
        }
        $sth->finish;
    }
    return $outputBuf;
}

sub getAvgStatsByBusinessFunction
{
    my $self = shift;
    my ($dbh, $serverNames, $startdate, $enddate) = @_;

    my $serverNames = $self->formatNamesToDB($serverNames);
    my @days = $self->calculate_days($startdate, $enddate);

    my $selectList = "clientservicename, count(*), ";
    $selectList .= "sum(case when numsolutionsreturned = 0 then 1 else 0 end)";
    $selectList .= ", AVG(elapsedtime), AVG(cpuused)";
    $selectList .= ", AVG(numsolutionsreturned)";
    my $whereClause = "nodeid in (" . $serverNames . ")";
    $whereClause .= " and transactiondatetime > '" . $startdate . "'";
    $whereClause .= " and transactiondatetime < '" . $enddate . "'";
    my $groupBy = "clientservicename";

    # all these maps are keyed by clientservicename
    my %totalTrxMap, %totalTrxErrMap, %avgElapsedMap, %avgCpuMap, %avgSolutionsMap;
    my $i = 0;

    foreach my $day (@days)
    {
        print "processing for day : $day \n" if($debug);
        my $sql = $self->buildSql($selectList, $day, $whereClause, $groupBy);
        print "SQL : $sql\n" if($debug);
        my $sth=$dbh->prepare($sql);
        if (!$sth)
        {
            $message .= "Error in sql: $DBI::errstr";
            print "message : $message \n";
        }
        else
        {
            $sth->execute;
            if($i eq 0) {  
                while (@data = $sth->fetchrow_array())
                {
                    print "$data[0],$data[1],$data[2],$data[3],$data[4],$data[5] \n" if($debug);
                    $totalTrxMap{$data[0]} = $data[1];
                    $totalTrxErrMap{$data[0]} = $data[2];
                    $avgElapsedMap{$data[0]} = $data[3];
                    $avgCpuMap{$data[0]} = $data[4];
                    $avgSolutionsMap{$data[0]} = $data[5];
                }
            }
            else {
                while (@data = $sth->fetchrow_array())
                {
                    print "$data[0],$data[1],$data[2],$data[3],$data[4],$data[5] \n" if($debug);
                    $pElapsed =
                        $totalTrxMap{$data[0]} * $avgElapsedMap{$data[0]};
                    $pCpu = $totalTrxMap{$data[0]} * $avgCpuMap{$data[0]};
                    $pSolutions = $totalTrxMap{$data[0]} * $avgSolutions{$data[0]};
                    $totalTrxMap{$data[0]} += $data[1];
                    $totalTrxErrMap{$data[0]} += $data[2];
                    $avgElapsedMap{$data[0]} = 
                     ($pElapsed + ($data[1] * $data[3]))/$totalTrxMap{$data[0]};
                    $avgCpuMap{$data[0]} = 
                     ($pCpu + ($data[1] * $data[4]))/$totalTrxMap{$data[0]};
                    $avgSolutionsMap{$data[0]} = 
                     ($pSolutions + ($data[1] * $data[5]))/$totalTrxMap{$data[0]};
                }
            }
            $i++;
        }
    } #end-foreach @days
    
    #now, write the output
    my $gTotalTrx, $gTotalTrxErr, $gAvgElapsed, $gAvgCpu, $gAvgSolutions;
    $gTotalTrx = $gTotalTrxErr = $gAvgElapsed = $gAvgCpu = $gAvgSolutions = 0;
    my %outputMap;
    my @clientServiceNames = sort { lc($a) cmp lc($b) } keys(%totalTrxMap);
    foreach my $clientServiceName (@clientServiceNames)
    {
        my $errPercentage = $totalTrxErrMap{$clientServiceName}/$totalTrxMap{$clientServiceName} * 100;
        my $outputBuf = sprintf("%s,%d,%d,%.2f\%,%.3f,%.3f,%.3f",
            $clientServiceName, $totalTrxMap{$clientServiceName},
            $totalTrxErrMap{$clientServiceName}, $errPercentage,
            $avgElapsedMap{$clientServiceName}/1000000,
            $avgCpuMap{$clientServiceName}/1000000,
            $avgSolutionsMap{$clientServiceName});
        $outputMap{$clientServiceName} = $outputBuf;

        $gTotalTrx += $totalTrxMap{$clientServiceName};
        $gTotalTrxErr += $totalTrxErrMap{$clientServiceName};
        $gAvgElapsed += ($avgElapsedMap{$clientServiceName} * $totalTrxMap{$clientServiceName});
        $gAvgCpu += ($avgCpuMap{$clientServiceName} * $totalTrxMap{$clientServiceName});
        $gAvgSolutions += ($avgSolutionsMap{$clientServiceName} * $totalTrxMap{$clientServiceName});
    }
    $gTotalTrx = 1 if ($gTotalTrx == 0);

    # Add grand totals to the outputMap
    my $errPercentage = $gTotalTrxErr/$gTotalTrx * 100;
    my $outputBuf = sprintf("TOTAL,%d,%d,%.2f\%,%.3f,%.3f,%.3f",
        $gTotalTrx, $gTotalTrxErr, $errPercentage, 
        $gAvgElapsed/($gTotalTrx * 1000000),
        $gAvgCpu/($gTotalTrx * 1000000),
        $gAvgSolutions/$gTotalTrx);
    $outputMap{"TOTAL"} = $outputBuf;

    return %outputMap;
}

sub getAvgStatsForPCCGroup
{
    my $self = shift;
    my ($dbh, $serverNames, $pccList, $startdate, $enddate) = @_;

    my $serverNames = $self->formatNamesToDB($serverNames);
    my $pccs = $self->formatNamesToDBWithDelimiter($pccList, " ");
    my @days = $self->calculate_days($startdate, $enddate);
    my $selectList .= "COUNT(*) AS cnt";
    $selectList .= ", sum(elapsedtime), sum(cpuused)";
    my $whereClause = "nodeid in (" . $serverNames . ")";
    $whereClause .= " and pseudocity in (" . $pccs . ")";
    $whereClause .= " and transactiondatetime > '" . $startdate . "'";
    $whereClause .= " and transactiondatetime < '" . $enddate . "'";
    $whereClause .= " and cpuused <> 0 ";

    $outputBuf = "Count,Avg Elapsed,Avg CPU\n";
    $totalTrx = 0;
    $totalCpu = 0;
    $totalElapsed = 0;
    foreach my $day (@days)
    {
        print "processing for day : $day \n" if($debug);
        my $sql = $self->buildSql($selectList, $day, $whereClause, "");
        print "SQL : $sql\n" if($debug);
        my $sth=$dbh->prepare($sql);
        if (!$sth)
        {
            $message .= "Error in sql: $DBI::errstr";
            print "message : $message \n";
        }
        else
        {
            $sth->execute;
            while (@data = $sth->fetchrow_array())
            {
                $totalTrx += $data[0];
                $totalElapsed += $data[1];
                $totalCpu += $data[2];
            }
        }
    }
    my $avgElapsed = $totalElapsed / $totalTrx;
    my $avgCpu = $totalCpu / $totalTrx;
    my $buf = sprintf("%d,%.3f,%.3f,", $totalTrx, $avgElapsed/1000000,
                      $avgCpu/1000000);
    $outputBuf .= $buf;
    $outputBuf .= "\n";
    return $outputBuf;
}

sub getAvgFareCounters
{
    my $self = shift;
    my ($dbh, $serverNames, $pccList, $clientServiceNames, $serviceNames,
        $startdate, $enddate) = @_;

    my $serverNames = $self->formatNamesToDB($serverNames);
    my $pccs = $self->formatNamesToDBWithDelimiter($pccList, " ");
    my $clientServiceNames = $self->formatNamesToDB($clientServiceNames);
    my $serviceNames = $self->formatNamesToDB($serviceNames);
    my @days = $self->calculate_days($startdate, $enddate);

    my $whereClause = "B.nodeid in (" . $serverNames . ")";
    $whereClause .= " and B.clientservicename in (" . $clientServiceNames . ")";
    $whereClause .= " and B.servicename in (" . $serviceNames . ")";
    $whereClause .= " and B.pseudocity in (" . $pccs . ")";
    $whereClause .= " and B.transactiondatetime > '" . $startdate . "'";
    $whereClause .= " and B.transactiondatetime < '" . $enddate . "'";
    $whereClause .= " and F.servicename = B.servicename";
    $whereClause .= " and F.nodeid = B.nodeid";
    $whereClause .= " and F.transactionid = B.transactionid";
    $whereClause .= " and F.clienttransactionid = B.clienttransactionid";
    my $groupBy = " B.clientservicename";

    my $selectList  = "B.clientservicename, count(*), ";
    $selectList .= "sum(case when F.PUBLISHEDFARESONLY = 'T' then 1 else 0 end) AS WEBREQCOUNT,";
    $selectList .= "sum(F.NUMPUBFARESRETRIEVED) AS Total_Number_Of_Fares,";
    $selectList .= "sum(F.NUMFARESUSEDINFAREPATHS) AS Number_of_Cat25_Fares,";
    $selectList .= "sum(F.NUMFARESPASS1STFILTER) AS Number_of_Addon_Fares,";
    $selectList .= "sum(F.NUMLBTROWSOUTOFDATE) AS Number_of_Validated_Fares,";
    $selectList .= "sum(F.NUMALTSOLUTIONS) AS Number_of_Fare_Markets,";
    $selectList .= "sum(F.NUMMIDPOINTSTRIED) AS Number_of_Pax_Types";

    # all these maps are keyed by clientservicename
    my %totalTrxMap, %totalWebReqMap, %totalFaresMap, %totalCat25FaresMap;
    my %totalAddonFaresMap, %totalValidatedFaresMap, %totalFareMarketsMap, %totalPaxTypesMap;

    foreach my $day (@days)
    {
        print "processing for day : $day \n" if($debug);
        my $sql = "SELECT " . $selectList;
        $sql .= " FROM " .  $INSTR_TABLE_PREFIX . $day . " B, " . $FSR_INSTR_TABLE_PREFIX . $day . " F";
        $sql .= " WHERE " . $whereClause;
        $sql .= " GROUP BY " . $groupBy;
        print "SQL : $sql\n" if($debug);
        my $sth=$dbh->prepare($sql);
        if (!$sth)
        {
            $message .= "Error in sql: $DBI::errstr";
            print "message : $message \n";
        }
        else
        {
            $sth->execute;
            while (@data = $sth->fetchrow_array())
            {
                $totalTrxMap{$data[0]} += $data[1];
                $totalWebReqMap{$data[0]} += $data[2];
                $totalFaresMap{$data[0]} += $data[3];
                $totalCat25FaresMap{$data[0]} += $data[4];
                $totalAddonFaresMap{$data[0]} += $data[5];
                $totalValidatedFaresMap{$data[0]} += $data[6];
                $totalFareMarketsMap{$data[0]} += $data[7];
                $totalPaxTypesMap{$data[0]} += $data[8];

                #$totalTrxMap{"TOTAL"} += $data[1];
                #$totalWebReqMap{"TOTAL"} += $data[2];
                #$totalFaresMap{"TOTAL"} += $data[3];
                #$totalCat25FaresMap{"TOTAL"} += $data[4];
                #$totalAddonFaresMap{"TOTAL"} += $data[5];
                #$totalValidatedFaresMap{"TOTAL"} += $data[6];
                #$totalFareMarketsMap{"TOTAL"} += $data[7];
                #$totalPaxTypesMap{"TOTAL"} += $data[8];
            }
        }
    }

    #now, write the output
    my %outputMap;
    my @clientServiceNames = sort { lc($a) cmp lc($b) } keys(%totalTrxMap);
    foreach my $clientServiceName (@clientServiceNames)
    {
        $totaltrx = $totalTrxMap{$clientServiceName};
        $totalwebreq = $totalWebReqMap{$clientServiceName};
        $avgwebreq = abs($totalwebreq)/$totaltrx;
        $totalfares = $totalFaresMap{$clientServiceName};
        $avgfares = abs($totalfares)/$totaltrx;
        $totalcat25fares = $totalCat25FaresMap{$clientServiceName};
        $avgcat25fares = abs($totalcat25fares)/$totaltrx;
        $totaladdonfares = $totalAddonFaresMap{$clientServiceName};
        $avgaddonfares = abs($totaladdonfares)/$totaltrx;
        $totalvalidatedfares = $totalValidatedFaresMap{$clientServiceName};
        $avgvalidatedfares = abs($totalvalidatedfares)/$totaltrx;
        $totalfaremarkets = $totalFareMarketsMap{$clientServiceName};
        $avgfaremarkets = abs($totalfaremarkets)/$totaltrx;
        $totalpaxtypes = $totalPaxTypesMap{$clientServiceName};
        $avgpaxtypes = abs($totalpaxtypes)/$totaltrx;

        my $outputBuf = sprintf("%s,%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f",
            $clientServiceName, $totaltrx,$avgwebreq,$avgfares,$avgcat25fares,
            $avgaddonfares,$avgvalidatedfares,$avgfaremarkets,$avgpaxtypes
            );
        $outputMap{$clientServiceName} = $outputBuf;
    }
    return %outputMap;
}

sub getMinAvg95thPercentile
{
    my $self = shift;
    my ($dbh, $serverNames, $serviceNames, $clientServiceNames, $startdate,
        $enddate) = @_;

    my $serverNames = $self->formatNamesToDB($serverNames);
    my $clientServiceNames = $self->formatNamesToDB($clientServiceNames);
    my @days = $self->calculate_days($startdate, $enddate);
}


1;
__END__
