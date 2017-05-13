#!/usr/bin/perl

require("fixCsvFile.pl");

sub processInvProd
{
    my ($input, $output) = @_;
    my @result;

    &fixCsvFile($input, $output);

    open(FIXEDFILE, "<", $output) or die "Cannot open $output.\n", $!;

    my @lines = (<FIXEDFILE>);
    my $numLines = @lines;

    $numLines == 0 and die "$output is either empty or non-formatted.\n", $!;

    foreach my $line (@lines)
    {
        my @lineList = split(/,/,$line);
        s/^\s+// foreach @lineList;
        s/\s+$// foreach @lineList;

        if ($#lineList < 5)
        {
            # Cert file, make it look like prod file
            splice(@lineList, 1, 0, "", "");
            splice(@lineList, 4, 0, "", "", "", "pod", "", "", "", "", "", "", "", "");
        }

        push(@result, [@lineList]);
    }

    return @result;
}

1
