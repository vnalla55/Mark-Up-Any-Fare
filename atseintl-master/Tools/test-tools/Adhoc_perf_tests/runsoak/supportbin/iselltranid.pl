#!/usr/bin/perl

$numArgs = $#ARGV + 1;

if( $numArgs ne 3 )
{
    print( "Invalid number of arguments\n" );
    printUsage();
    exit(1);
}
my $input = $ARGV[0];
my $output = $ARGV[1];
my $startNumber = $ARGV[2];

modifyTranId($input, $output, $startNumber);

sub printUsage
{
  print "Usage: iselltranid.pl <input-file> <output-file> <start-num>\n";
}


sub modifyTranId($input, $output, $startNumber)
{
  my ($input, $output, $startNumber) = @_;

  my $tranid = $startNumber;

  #remove the output file if it exists
  unlink( $output );

  open INPUT, "<$input" or die "could not open '$input' for reading";
  open OUTPUT, ">$output" or die "could not open '$output' for writing";

  while( my $line = <INPUT> )
  {
    if( $line =~ /<TranID/ ) {
      $line =~ s/<TranID Value="[0-9]+"/<TranID Value="${tranid}"/;
      $tranid++;
    }
    print OUTPUT $line;
  }
  close INPUT;
  close OUTPUT;
}
