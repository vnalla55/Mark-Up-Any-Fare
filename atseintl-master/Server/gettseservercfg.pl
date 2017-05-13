#!/usr/bin/perl -w

use strict;
use Sys::Hostname;
use Getopt::Long qw(:config pass_through);

#CONSTANTS
my $ACMS_CONN_FILE = "/opt/atse/acmsconn.ini";
my $SVCHOST = `cat $ACMS_CONN_FILE | grep '^\\s*acms\\.host\\s*=' | awk -F'=' '{print \$2}'`;
chomp($SVCHOST);
$SVCHOST =~ s/^\s+//;
$SVCHOST =~ s/\s+$//;
my $SVCPORT = `cat $ACMS_CONN_FILE | grep '^\\s*acms\\.port\\s*=' | awk -F'=' '{print \$2}'`;
chomp($SVCPORT);
$SVCPORT =~ s/^\s+//;
$SVCPORT =~ s/\s+$//;

my $TSESERVER_CFG_FILE = "tseserver.acms.cfg";
my $DATA_TYPE = "tseserver";
my $DATA_FORMAT = "INI";
my $FAMILY = "atsev2";
my $DEFAULT_ENVIRONMENT = "integ";
my $ACMS_QUERY_PATH = "/opt/atse/common/acmsquery/acmsquery.sh";
my $INTEGRATION_STREAM = "atsev2_Integration@/vobs/atse-intl-proj";
my $appname;
my $baseline;
my $user;
my $environment;
my $actlist;

sub get_host_name
{
  my $ourhost = hostname;
  my @hostpieces = split( /\./, $ourhost);
  my $ournode = $hostpieces[0];
}

sub get_baseline
{
  my $baseline = '';
  $baseline = `git ls-remote --tags origin 'atsev2.*[a-z]' | tail -1 | sed 's/^.*refs\\/tags\\///' | awk -F'.' '{print \$1"."\$2"."\$3".00"}'`;
  chomp($baseline);
  $baseline;
}

sub read_params
{
  GetOptions("app=s" => \$appname, "bsl=s" => \$baseline, "user=s" => \$user, "env=s" => \$environment, "act=s" => \$actlist);
  if(!defined($appname)) {

    die "ERROR: Application name not specified!";
  }
}

sub main
{
  read_params();
  my $node = get_host_name();
  
  if(!defined($user)) {

    $user = getlogin();
    if (!defined($user)) {
      $user = $ENV{USER};
    }
  }
  if(!defined($baseline)) {

    $baseline = get_baseline();
  }
  if(!defined($environment)) {

    $environment = $DEFAULT_ENVIRONMENT;
  }

  if ( !defined($user) || ($user eq "") ) {
    die "ERROR: Cannot figure out a user name";
  }

  my $acmsQueryCmd = "$ACMS_QUERY_PATH ";
  $acmsQueryCmd = $acmsQueryCmd . "-svchost $SVCHOST -svcport $SVCPORT ";
  $acmsQueryCmd = $acmsQueryCmd . "-baseline $baseline -user $user ";
  $acmsQueryCmd = $acmsQueryCmd . "-FAM $FAMILY -APP $appname ";
  $acmsQueryCmd = $acmsQueryCmd . "-ENV $environment -NOD $node ";
  $acmsQueryCmd = $acmsQueryCmd . "-datatype $DATA_TYPE ";
  $acmsQueryCmd = $acmsQueryCmd . "-dataformat $DATA_FORMAT ";
  $acmsQueryCmd = $acmsQueryCmd . "-outputfile $TSESERVER_CFG_FILE";

  if(defined($actlist)) {

    $acmsQueryCmd = $acmsQueryCmd . " -act \"$actlist\"";
  }

  print "$acmsQueryCmd\n";
  system($acmsQueryCmd);
}

main();
exit 0;



