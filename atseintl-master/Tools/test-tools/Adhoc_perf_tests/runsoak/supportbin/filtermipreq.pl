#!/usr/bin/perl
$numArgs = $#ARGV + 1;

if( $numArgs ne 3 )
{
    print( "Invalid number of arguments\n" );
    exit(1);
}
$input = $ARGV[0];
$output = $ARGV[1];
my $startno = $ARGV[2];

prepareRequests( $input, $output, $startno );

sub prepareRequests
{
    my ( $inputFile, $outputFile ) = @_;
    my $outputData = "";
    my $reqbuf = "";
    my $inreq = 0;
    my $reqno = $startno;
    my $validreq = 0;

    #remove the output file if it exists
    unlink( $outputFile );

    open INPUT, "<$inputFile"
        or die "could not open '$inputFile' for reading";

    while( my $line = <INPUT> )
    {
        if( $line =~ /^<ShoppingRequest / )
        {
            $inreq = 1;
            $line =~ s/TID="[0-9]+"/TID="${reqno}"/;
        }
        if( $line =~ /<BIL / )
        {
            $line =~ s/C01="[0-9]+"/C01="${reqno}"/;
            $line =~ s/C02="[0-9]+"/C02="${reqno}"/;
        }

        if( $inreq eq 1 )
        {
            if( $line =~ /^<BIL / )
            {
                $validreq = 1 if( $line =~ / C21="SDSVI/ );
            }
            $reqbuf .= $line;
        }
        if( $line =~ /<\/ShoppingRequest/ )
        {
            if( ($inreq eq 1) && ($validreq eq 1) )
            {
                $outputData .= $reqbuf;
                $reqno++;
            }
            $inreq = 0;
            $reqbuf = "";
            $validreq = 0;
        }
    }
    close INPUT;

    open OUTPUT, ">$outputFile"
        or die "could not open '$outputFile' for writing";
    print OUTPUT $outputData;
    close OUTPUT;
}
