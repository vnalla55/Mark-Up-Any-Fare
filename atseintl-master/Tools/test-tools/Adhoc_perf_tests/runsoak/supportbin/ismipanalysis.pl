#!/usr/bin/perl
use XML::Parser;

package Fare;
sub new
{
  my $class = shift;
  my $self = {
    _a01 => shift, # fare origin
    _a02 => shift, # fare destination
    _b00 => shift, # carrier
    _b50 => shift, # fare basis code
  };

  bless $self, $class;
  return $self;
}
sub getFareOrigin {
  my ( $self ) = @_;
  return $self->{_a01};
}
sub getFareDestination {
  my ( $self ) = @_;
  return $self->{_a02};
}
sub getCarrier {
  my ( $self ) = @_;
  return $self->{_b00};
}
sub getFareBasisCode {
  my ( $self ) = @_;
  return $self->{_b50};
}
sub print {
  my ( $self, $indent ) = @_;
  print $indent . "FARE {\n";
  print $indent . "  ORG = [" . $self->getFareOrigin() . "]\n";
  print $indent . "  DST = [" . $self->getFareDestination() . "]\n";
  print $indent . "  CXR = [" . $self->getCarrier() . "]\n";
  print $indent . "  FBC = [" . $self->getFareBasisCode() . "]\n";
  print $indent . "}\n";
}

package SOP;
sub new
{
  my $class = shift;
  my $self = {
    _q14 => shift, # leg id
    _q15 => shift, # sop identifier
    _a01 => shift, # fare origin
    _a02 => shift, # fare destination
    _b50 => shift, # fare basis code
  };

  bless $self, $class;
  return $self;
}
sub getLegId {
  my ( $self ) = @_;
  return $self->{_q14};
}
sub getSopId {
  my ( $self ) = @_;
  return $self->{_q15};
}
sub getFareOrigin {
  my ( $self ) = @_;
  return $self->{_a01};
}
sub getFareDestination {
  my ( $self ) = @_;
  return $self->{_a02};
}
sub getFareBasisCode {
  my ( $self ) = @_;
  return $self->{_b50};
}
sub hasFare {
  my ( $self ) = @_;
  return 1 if(defined($self->{_b50}));
  return 0;
}
sub print {
  my ( $self, $indent ) = @_;
  print $indent . "SOP {\n";
  print $indent . "  Leg = [" . $self->getLegId() . "]\n";
  print $indent . "  SID = [" . $self->getSopId() . "]\n";
  print $indent . "  ORG = [" . $self->getFareOrigin() . "]\n";
  print $indent . "  DST = [" . $self->getFareDestination() . "]\n";
  print $indent . "  FBC = [" . $self->getFareBasisCode() . "]\n";
  print $indent . "}\n";
}

package Itin;

sub new
{
  my $class = shift;
  my $self = {
    _q4q    => shift, # family group id
    _q5q    => shift, # family id
    _sids   => undef, # SOP array reference
    _mfares => undef, # MIP Fare Infos
  };

  bless $self, $class;
  return $self;
}
sub getFamilyGroupId {
  my( $self ) = @_;
  return $self->{_q4q};
}
sub getFamilyId {
  my( $self ) = @_;
  return $self->{_q5q};
}
sub addSOP {
  my( $self, $sop ) = @_;

  if( ! defined($self->{_sids}) ) {
    my @sids = ();
    $self->{_sids} = \@sids;
  }
  my $sidsref = $self->{_sids};
  push(@$sidsref, $sop);
}
sub addFare {
  my( $self, $fare ) = @_;

  if( ! defined($self->{_mfares}) ) {
    my @fares = ();
    $self->{_mfares} = \@fares;
  }
  my $faresref = $self->{_mfares};
  push(@$faresref, $fare);
}
sub getFares {
  my( $self ) = @_;
  return $self->{_mfares};
}
sub setFares {
  my( $self, $fares ) = @_;
  $self->{_mfares} = $fares;
}
sub getItinIdentifier {
  my( $self ) = @_;
  my $id = "";

  my $sidsref = $self->{_sids};
  foreach $sop (@$sidsref) {
    $id .= "L" . $$sop->getLegId() . "_" . "S" . $$sop->getSopId() . "_";
  }
  #$len = @$sidsref;
  #print "sid length : $len\n";
  return $id;
}
sub getISFarePath {
  my( $self ) = @_;
  my $fp = "";
  my $sidsref = $self->{_sids};
  foreach $sop (@$sidsref) {
    if($$sop->hasFare()) {
      $fp .= $$sop->getFareBasisCode() . "|";
    }
  }
  return $fp;
}
sub getMIPFarePath {
  my( $self ) = @_;
  my $fp = "";
  my $faresref = $self->{_mfares};
  foreach $fare (@$faresref) {
    $fp .= $$fare->getCarrier()
           . $$fare->getFareOrigin() . $$fare->getFareDestination() 
           . "_" . $$fare->getFareBasisCode() . "|";
  }
  return $fp;
}
sub hasFare {
  my( $self ) = @_;
  my $res = 1;
  my $sidsref = $self->{_sids};
  foreach $sop (@$sidsref) {
    if($$sop->hasFare() == 0) {
      $res = 0;
    }
  }
  return $res;
}
sub print {
  my( $self, $indent ) = @_;
  print $indent . "Itin {\n";
  print $indent . "  Group       = [" . $self->getFamilyGroupId() . "]\n";
  print $indent . "  Family      = [" . $self->getFamilyId() . "]\n";
  print $indent . "  Identifier  = [" . $self->getItinIdentifier() . "]\n";
  print $indent . "  ISFarePath  = [" . $self->getISFarePath() . "]\n";
  print $indent . "  MIPFarePath = [" . $self->getMIPFarePath() . "]\n";
  my $sidsref = $self->{_sids};
  foreach $sop (@$sidsref) {
    $$sop->print($indent . "  ");
  }
  my $faresref = $self->{_mfares};
  foreach $fare (@$faresref) {
    $$fare->print($indent . "  ");
  }
  print $indent . "}\n";
}

package Request;
sub new
{
  my $class = shift;
  my $self = {
    _tid      => shift, # transaction id 
    _isitins  => undef, # IS itins list
    _mipitins => undef, # MIP itins list
    _itins    => undef, # itin references keyed by itin identifier
  };

  bless $self, $class;
  return $self;
}
sub getTid {
  my( $self ) = @_;
  return self->{_tid};
}
sub addItin {
  my( $self, $itin ) = @_;
  if( ! defined($self->{_itins}) ) {
    my %itins = ();
    $self->{_itins} = \%itins;
  }
  $itinsref = $self->{_itins};
  $$itinsref{$$itin->getItinIdentifier()} = $itin;
}
sub getItin {
  my( $self, $itinIdentifier ) = @_;
  my $itin = undef;
  if(defined($self->{_itins}) ) {
    $itinsref = $self->{_itins};
    $itin = $$itinsref{$itinIdentifier};
  }
  return $itin;
}
sub addISItin {
  my( $self, $itin ) = @_;
  if( ! defined($self->{_isitins}) ) {
    my @itins = ();
    $self->{_isitins} = \@itins;
  }
  $itinsref = $self->{_isitins};
  push(@$itinsref, $itin);
}
sub addMIPItin {
  my( $self, $itin ) = @_;

  if( ! defined($self->{_mipitins}) ) {
    my @itins = ();
    $self->{_mipitins} = \@itins;
  }
  $itinsref = $self->{_mipitins};
  push(@$itinsref, $itin);
}
sub isvalid {
  my( $self ) = @_;
  return 1 if( defined($self->{_itins}) );
  return 0;
}
sub print {
  my( $self ) = @_;
  print "Request {\n";
  print "  TID=" . $self->{_tid} . "\n";

  #if(defined($self->{_isitins})) {
  #  $itinsref = $self->{_isitins};
  #  foreach $itin (@$itinsref) {
  #    $$itin->print("    ") if($$itin->hasFare());
  #  }
  #}
  if(defined($self->{_itins})) {
    my $itinsref = $self->{_itins};
    my @itinkeys = keys(%$itinsref);
    foreach $ikey (@itinkeys) {
      $itin = $$itinsref{$ikey};
      $$itin->print("    ") ; #if($$itin->hasFare());
    }
  }

  print "}\n";
}
sub printMIPFarePath {
  my( $self ) = @_;
  my $itinsref = $self->{_itins};
  my @itinkeys = keys(%$itinsref);
  foreach $ikey (@itinkeys) {
    $itin = $$itinsref{$ikey};
    my $id = $$itin->getFamilyId();
    my $mipfp = $$itin->getMIPFarePath();
    if(length($mipfp) > 0) {
      print $self->{_tid} . "," . $mipfp . ", " . $id . "\n";
    }
  }
}

sub printMIPFarePath1 {
  my( $self ) = @_;
  my %farepath = ();
  my $itinsref = $self->{_itins};
  my @itinkeys = keys(%$itinsref);
  foreach $ikey (@itinkeys) {
    $itin = $$itinsref{$ikey};
    my $id = $$itin->getFamilyId();
    my $mipfp = $$itin->getMIPFarePath();
    if(not exists $farepath{$mipfp}) {
      %idarr = ();
      $farepath{$mipfp} = \%idarr;
    }
    my $idarrref = $farepath{$mipfp};
    $idarrref->{$id} = 1;
  }
  foreach $fpkkey (keys %farepath) {
    my $famids = "";
    my $idarrref = $farepath{$fpkey};
    foreach $id (sort keys %$idarrref) {
      $famids .= $id . ":";
    }
    print $self->{_tid} . "," . $fpkey . ", " . $famids . "\n";
  }
}


package main;

my $numArgs = @ARGV;

my $islog  = $ARGV[0];
my $miplog = $ARGV[1];

my $ptid = undef;
my %requests = ();

my $curreq = undef;
my $curitn = undef;
my $cursop = undef;
my $parsingIS = 1;

parseLogFile($islog);
$parsingIS = 0;
parseLogFile($miplog);
#testfunc();

@tids = keys( %requests );
print "size of requests:  " . @tids . ".\n";
foreach $tid (@tids) {
  $req = $requests{$tid};
  $req->print() if defined $req;
}
foreach $tid (@tids) {
  $req = $requests{$tid};
  $req->printMIPFarePath() if defined $req;
}

# The Handlers for IS Response parsing
sub hdl_start{
  my ($p, $elt, %atts) = @_;
  if($elt eq "ShoppingResponse") {
    #print "is_hdl_start : $elt \n";
    #@attkeys = keys(%atts);
    #foreach $attkey (%atts) {
    #  print "key[" . $attkey . "] = [" . $atts{$attkey} . "]\n";
    #}
    my $tid = $atts{"C01"};
    #print "tid : $tid\n";
    $curreq = $requests{$tid};
  }
  elsif($elt eq "ITN") {
    my $group = $atts{"Q4Q"};
    my $fm = $atts{"Q5Q"};
    my $itn = new Itin($group, $fm);
    #$itn->print("MM");
    #$curreq->addISItin(\$itn);
    $curitn = \$itn;
  }
  elsif($elt eq "SID") {
    my $leg = $atts{"Q14"};
    my $sid = $atts{"Q15"};
    my $org = $atts{"A01"};
    my $dst = $atts{"A02"};
    my $fbc = $atts{"B50"};
    my $sop = new SOP($leg, $sid, $org, $dst, $fbc);
    $$curitn->addSOP(\$sop);
  }
  elsif($elt eq "FDC") {
    my $org = $atts{"A01"};
    my $dst = $atts{"A02"};
    my $fbc = $atts{"B50"};
    my $cxr = $atts{"B00"};
    my $fare = new Fare($org, $dst, $cxr, $fbc);
    $$curitn->addFare(\$fare);
  }
}

sub hdl_end{
  my ($p, $elt) = @_;
  if($elt eq "ShoppingResponse") {
    $curreq = undef;
  }
  elsif($elt eq "ITN") {
    if(defined($curreq) ) {
      if($parsingIS) {
        #$curreq->addISItin($curitn);
        $curreq->addItin($curitn);
      }
      else { # parsing MIP
        my $itin = $curreq->getItin($$curitn->getItinIdentifier());
        if(defined($itin)) {
          $$itin->setFares($$curitn->getFares());
        } else {
          printf(STDERR "Itin identifier not found %s\n", $$curitn->getItinIdentifier());
        }
      }
      $curitn = undef;
    }
  }
}

sub hdl_char {
   my ($p, $str) = @_;
}

sub hdl_def { }  # We just throw everything else


# parse IS log file
sub parseLogFile
{
  my ($logfile) = @_;
  my $psgcount = 0;
  my $tid = undef;
  my $resp = "";
  
  my $parser = new XML::Parser ( Handlers => { # Creates our parser object
                              Start   => \&hdl_start,
                              End     => \&hdl_end,
                              Char    => \&hdl_char,
                              Default => \&hdl_def,
                            });


  open INPUT, "<$logfile" or die "couldn't open '$logfile' for reading";
  while(my $line = <INPUT>) {
    if($parsingIS) {
      if($line =~ m/<ShoppingRequest (.*) TID="(.*)" /) {
        $tid = $2;
        $psgcount = 0;
      }
      if($line =~ m/<PXI/) {
        $psgcount++;
      }
      if($line =~ m/<\/ShoppingRequest/ && $psgcount == 1) {
        if($tid > 17000 && $tid < 17501) {
            if( $tid == $ptid ) {
              $requests{$tid} = undef;
            } else {
              my $request = new Request($tid);
              $requests{$tid} = $request;
              $ptid = $tid;
            }
        }
      }
    }
    if($line =~ m/<ShoppingResponse/) {
      $inresp = 1;
      $resp = "";
    }
    if($inresp == 1) {
      $resp .= $line;
    }
    if($line =~ m/<\/ShoppingResponse/) {
      $parser->parse($resp);
    }
  }
  close INPUT;
}

sub testfunc {
print "testing $islog\n";
$s1 = new SOP("0", "1", "DFW", "NYC", "ABCD");
$s2 = new SOP("1", "5", "NYC", "DFW", "XYZD");

$i = new Itin("0", "0");
$i->addSOP(\$s1);
$i->addSOP(\$s2);

$id = $i->getItinIdentifier();
print "Itin identifier = $id \n";
$fp = $i->getISFarePath();
print "ISFare Path = $fp \n";
print "end\n";
}
