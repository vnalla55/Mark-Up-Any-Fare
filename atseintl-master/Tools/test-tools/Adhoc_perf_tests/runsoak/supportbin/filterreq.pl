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
    my $count = 0;
    my $reqbuf = "";
    my $inreq = 0;
    my $validreq = 0;

    #remove the output file if it exists
    unlink( $outputFile );

    open INPUT, "<$inputFile"
        or die "could not open '$inputFile' for reading";

    while( my $line = <INPUT> )
    {
        # presently tserequest.log for shopping requests do not log first
        # 8 characters due to a bug in the application. To overcome that bug
        # following substitution is added. After that bug is fixed, this
        # substitution can be removed.
        $line =~ s/ gRequest / <ShoppingRequest /;
        $line =~ s/<\/ShoppingRequest>}}}/<\/ShoppingRequest>/;

        #$outputData .= $line;
        if( $line =~ /^<ShoppingRequest / )
        {
            $inreq = 1;
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
