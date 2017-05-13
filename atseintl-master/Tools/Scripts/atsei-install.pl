#!/usr/bin/env perl
use Getopt::Long;

#-------------------------------------------------------------------------------
# Configuration settings:
#-------------------------------------------------------------------------------
$tarFileTemplate = "atseintl.%s.tar.gz";
$tarDirTemplate = "atseintl.%s";
$tarCommand = "/bin/tar xvzf ../%s";
$currLinkName = "atsei_current";

#-------------------------------------------------------------------------------
#
# subroutines
#
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# kill running servers
#-------------------------------------------------------------------------------
sub kill_servers
{
  my ($test) = @_;
  print "\n\nKilling all running tseservers.\n";
  if (! ($test) )
  {
    `/bin/touch /opt/atseintl/noTseTaxServerRestart`;
#    `/bin/touch /opt/atseintl/noTsePricingServerRestart`;
#    `/bin/touch /opt/atseintl/noTseFDServerRestart`;
#    `/bin/touch /opt/atseintl/noTseShoppingServerRestart`;
#    `/bin/touch /opt/atseintl/noTsePricingServerTcpipRestart`;
  }
  `/usr/bin/skill -9 tseserver`;
}


#-------------------------------------------------------------------------------
# restart the servers
#-------------------------------------------------------------------------------
sub restart_servers
{
  my ($test) = @_;
  print "\n\nRestarting tseservers.\n";
  if (! ($test) )
  {
    `/bin/rm /opt/atseintl/noTseTaxServerRestart`;
#    `/bin/rm /opt/atseintl/noTsePricingServerRestart`;
#    `/bin/rm /opt/atseintl/noTseFDServerRestart`;
#    `/bin/rm /opt/atseintl/noTseShoppingServerRestart`;
#    `/bin/rm /opt/atseintl/noTsePricingServerTcpipRestart`;
  }
}


#-------------------------------------------------------------------------------
# install a new release
#-------------------------------------------------------------------------------
sub install
{
  my ($currLinkName, $tarDirTemplate, $releaseLevel, $tarFileTemplate, $tarCommand) = @_;
  my $retval = 0;

  # next, create the new directory name from template and releaseLevel
  my $tarDirName = sprintf ($tarDirTemplate, $releaseLevel);

  # next, make the directory with the release level as part of the name
  my $mkdircmd = sprintf ("/bin/mkdir %s", $tarDirName);
  print ("Running Command: \"$mkdircmd\"\n");
  $retval = system ($mkdircmd);
  if ($retval)
  {
    die "mkdir command returned an error: $retval";
  }
  

  #
  # Extract the tar file.
  #

  # first, create the file name from the template and releaseLevel
  my $tarFileName = sprintf ($tarFileTemplate, $releaseLevel);
  if ( ! ( -f $tarFileName))
  {
    print (STDERR "Tar file, \"${tarFileName}\", does not exist\n");
    exit (1);
  }

  # next, run the tar command to untar the files:
  my $tarcmd = sprintf ("cd $tarDirName; ${tarCommand}", $tarFileName);
  print ("Running Command: \"$tarcmd\"\n");
  $retval = system ($tarcmd);
  if ($retval)
  {
    die "tar command returned an error: $retval";
  }

  # stop the running servers
  kill_servers();

  # symlink the file to atsei_current
  if (-e $currLinkName)
  {
    `rm -f $currLinkName`;
  }
  my $symlinkcmd = sprintf ("/bin/ln -s %s $currLinkName", $tarDirName);
  print ("Running Command: \"$symlinkcmd\"\n");
  $retval = system ($symlinkcmd);
  if ($retval) 
  {
    die "symlink command returned an error: $retval";
  }

  # restart the servers
  restart_servers();
}

#-------------------------------------------------------------------------------
# fallback the version to another previous version
#-------------------------------------------------------------------------------
sub fallback
{
  my ($currLinkName, $tarDirTemplate, $releaseLevel) = @_;
  my $tarDirName = sprintf ($tarDirTemplate, $releaseLevel);
  # symlink the file to atsei_current
  if (-e $currLinkName)
  {
    `rm -f $currLinkName`;
  }
  # stop the running servers
  kill_servers();

  # symlink the fallback directory to "atsei_current"
  my $symlinkcmd = sprintf ("/bin/ln -s %s $currLinkName", $tarDirName);
  print ("Running Command: \"$symlinkcmd\"\n");
  $retval = system ($symlinkcmd);
  if ($retval) 
  {
    die "symlink command returned an error: $retval";
  }

  # restart the servers
  restart_servers();
}

#-------------------------------------------------------------------------------
# print the usage message to STDERR
#-------------------------------------------------------------------------------
sub usage()
{
  my $usage = q/
  USAGE: 
    atsei-install.pl -h
            or
    atsei-install.pl ("stop"|"restart")
            or
    atsei-install.pl <release level> ("install"|"fallback")

/;
  print (STDERR $usage);
}

#-------------------------------------------------------------------------------
# MAINLINE
#-------------------------------------------------------------------------------

# handle command line params
Getopt::Long::Configure ('bundling');
GetOptions ('c|conf=s' => \$configfile,
    'b|basepath' => \$BASEPATH,
    'h|help|?' => \$helpflag);
          
if ($helpflag == 1)
{
  usage();
  exit (0);
}

$argcnt = @ARGV;
if ($argcnt == 1)
{
  $direction = shift;
  if (!($direction eq "stop" or $direction eq "restart"))
  {
    print (STDERR "A single param must be either \"stop\" or \"restart\"\n");
    usage();
    exit(1);
  }
}
elsif ($argcnt == 2)
{
  $releaseLevel = shift;
  $direction = shift;
  if (!($direction eq "install" or $direction eq "fallback"))
  {
    print (STDERR "Second param must be either \"install\" or \"fallback\"\n");
    usage();
    exit(1);
  }
}
else
{
  print (STDERR "Wrong number of parameters\n");
  usage();
  exit(1);
}

# we're finished with the parameters, we know what to do, so let's do it!
if ($direction eq "install") 
{
  install ($currLinkName, $tarDirTemplate, $releaseLevel, $tarFileTemplate, $tarCommand);
}
elsif ($direction eq "fallback")
{
  fallback ($currLinkName, $tarDirTemplate, $releaseLevel);
}
elsif ($direction eq "stop")
{
  kill_servers();
}
elsif ($direction eq "restart")
{
  restart_servers();
}
else
{
  print STDERR "param invalid\n";
  usage();
  exit(1);
}

print "\n\nSuccessful $direction\n";
