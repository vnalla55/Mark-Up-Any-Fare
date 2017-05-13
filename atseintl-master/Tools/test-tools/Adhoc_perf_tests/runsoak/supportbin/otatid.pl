#!/usr/bin/perl
$numArgs = $#ARGV + 1;

if( $numArgs ne 2 )
{
    print( "Invalid number of arguments\n" );
    exit(1);
}
$input = $ARGV[0];
$output = $ARGV[1];
prepareRequests( $input, $output );

sub prepareRequests
{
    my ( $inputFile, $outputFile ) = @_;
    my $outputData = "";
    my $reqbuf = "";
    my $inreq = 0;
    my $reqno = 88880001;

    #remove the output file if it exists
    unlink( $outputFile );

    open INPUT, "<$inputFile"
        or die "could not open '$inputFile' for reading";

    while( my $line = <INPUT> )
    {
        $line =~ s/TranID Value="[0-9]+"/TranID Value="${reqno}"/;
        $outputData .= $line;
        $reqno++;
    }
    close INPUT;

    open OUTPUT, ">$outputFile"
        or die "could not open '$outputFile' for writing";
    print OUTPUT $outputData;
    close OUTPUT;
}
