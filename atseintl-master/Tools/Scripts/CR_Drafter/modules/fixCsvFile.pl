#!/usr/bin/perl

sub fixCsvFile
{
    my($csv, $fixedCsv) = @_;
    open(CSV, "<", $csv) or die "Cannot open $csv.\n", $!;
    open(FIXEDCSV, ">", $fixedCsv) or die "Cannot create $fixedCsv.\n", $!;

    my $count = 0;

    $inQuotes = 0;
    while (<CSV>)
    {
        next if ($count++ == 0);  # skip header

        if ($inQuotes == 1)
        {
            $inQuotes = 0;
        }
        elsif (m/\".*\n/)
        {
            s/\n/ /;
            $inQuotes = 1;
        }

        print FIXEDCSV $_;
    }

    close FIXEDCSV;
    close CSV;
}

1
