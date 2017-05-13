#!/bin/env perl

use Thread; 

################################################################################
# getEmailCmd - finds what the email command is on this box
################################################################################
sub getEmailCmd
{
  my $mail;

  # decide which email command to use
  for $mail ("/bin/xmail","/usr/bin/xmail",
             "/bin/mail","/usr/bin/mail",
             "/bin/mailx","/usr/bin/mailx"
           )
  {
    if ( -f $mail )
    {
      return $mail;
    }
  }
  if ($mail eq "")
  {
    die "Could not find mail command, exiting";
  }
}

################################################################################
# sendEmail - the name says it all
################################################################################
sub sendEmail
{
  my ($subject, $content, $addr) = @_;
  open (MAIL,"| $mailCmd -s \"$subject\" $addr");
  print (MAIL $content);
  close (MAIL);
}

################################################################################
# touch - creates an empty file of the name passed in using the unix command
#         "touch".
################################################################################
sub touch
{
  my($filename) = @_;
  open (NORESTART, ">$filename");
  close (NORESTART);
}

################################################################################
# triggerReload - sets the reloadconfig flag which triggers a reload of the 
#                 config file.
################################################################################
sub triggerReload
{
   $reloadconfig = 1;
}

################################################################################
# trim - trims leading and trailing white space from a string. 
################################################################################
sub trim
{
  my ($string) = @_;
  chomp $string;
  $string =~ s/^\s*//;
  $string =~ s/\s*$//;
  return $string;
}

################################################################################
# sanitycheck - checks the passed-in process table for possible mistakes,
#               removes entries which don't pass.
################################################################################
sub sanitycheck
{
  my ($app_ptr) = @_;
  my $name;
  my $key;
  my $err;
  my $nameerr;

  for $name ( keys %$app_ptr )
  {
    $nameerr = 0;
    if ($$app_ptr{$name}{"emaillst"} eq "")
    {
      print (STDERR "Application $name failed config file sanity check.\n");
      print (STDERR "\"emaillst\" parameter missing\n");
      $nameerr = 1;
    }
    if ($$app_ptr{$name}{"nostart"} eq "" ) 
    {
      print ("Application $name failed config file sanity check.\n");
      print (STDERR "\"nostart\" parameter missing\n");
      $nameerr = 1;
    }
    if ( ! (-x $$app_ptr{$name}{"cmdfile"} ) ) 
    {
      print ("Application $name failed config file sanity check.\n");
      print (STDERR "File listed in \"cmdfile\" must exist, and be executable\n");
      $nameerr = 1;
    }
    if ($$app_ptr{$name}{"timeout"} < 0) 
    {
      print ("Application $name failed config file sanity check.\n");
      print (STDERR "\"timeout\" parameter missing\n");
      $nameerr = 1;
    }
    if ($$app_ptr{$name}{"upgrdfile"} eq "")
    {
      print (STDERR "Application $name failed config file sanity check.\n");
      print (STDERR "\"upgrdfile\" parameter missing\n");
      $nameerr = 1;
    }
    if ( ! (-x $$app_ptr{$name}{"upgrdscript"} ) ) 
    {
      print ("Application $name failed config file sanity check.\n");
      print (STDERR "File listed in \"upgrdscript\" must exist, and be executable\n");
      $nameerr = 1;
    }
    if ($$app_ptr{$name}{"falbkfile"} eq "")
    {
      print (STDERR "Application $name failed config file sanity check.\n");
      print (STDERR "\"falbkfile\" parameter missing\n");
      $nameerr = 1;
    }
    if ( ! (-x $$app_ptr{$name}{"falbkscript"} ) ) 
    {
      print ("Application $name failed config file sanity check.\n");
      print (STDERR "File listed in \"falbkscript\" must exist, and be executable\n");
      $nameerr = 1;
    }

    if ( $nameerr )
    {
      $err = 1;
      delete $$app_ptr{$name};
    }
  }

  if ($err)
  {
    howto();
    return($err);
  }
  else
  {
    return (0);
  }
}

################################################################################
# report - print a list of apps and configs
################################################################################
sub report
{
  my ($app_ptr) = @_;
  my $name;
  my $key;

  for $name ( keys %$app_ptr )
  {
    print ("App Name: \"$name\"\n");
    for $key ( keys %{ $$app_ptr{$name} } )
    {
      print ("    \"$key\" = \"$$app_ptr{$name}{$key}\"\n");
    }
  }
}

################################################################################
# howto - print a howto message
################################################################################
sub howto
{
  my $message = qq/
# Command: respawn.pl <config file>
#
# The application, \"respawn.pl\" runs programs that are given to it in a config
# file, and if the program dies, or is killed, respawn restarts it, following the
# rules supplied in the config file.
#
#
# Config File Rules:
# ------------------
# 1. The application-name lines must end with a ':'.
# 2. The parameter lines must start with at least one white space character.
# 3. Any line beginning with '#' is considered a comment.
# 4. Comments are not allowed at the end of a line containing a param or app
#     name, a comment there is taken to be part of the param.
# 5. You can add new apps to the config file, while respawn.pl is running,
#    the command \"kill -1 <pid>\" will cause respawn.pl to re-read the config
#    file.  You cannot remove an app from the config file, and have respawn.pl
#    kill it.  The Perl thread API doesn't allow the main thread to kill a child
#    thread.
# 6. If you kill respawn.pl, or it dies, all the child processes will also die
#
# The Config file follows the following format
# appname: (you pick it, it doesn't matter what it says, must be unique)
#    cmdfile     = <REQUIRED, executable file to run>
#    cmdparms    = <command  line params for executable>
#    upgrdfile   = <REQUIRED, if this file exists, run the upgrdscript>
#    upgrdscript = <REQUIRED, run this script if upgrdfile exists on file system>
#    falbkfile   = <REQUIRED, if this file exists, run the falbkscript>
#    falbkscript = <REQUIRED, run this script if falbkfile exists on file system>
#    emaillst    = <REQUIRED, Space delimited list of who to email
#                          when a command fails>
#    nostart     = <REQUIRED, don't restart if this file exists>
#    timeout     = <A number >= 0, if the program dies 10 times in a minute,
#                   respawn will wait \"\$timeout\" seconds before restarting,
#                   a value of 0 will cause it to not restart, but
#                   create the norestart file>
/;

print $message;
}

################################################################################
# loadConfig - loads the application data from the config file into a 
#              hash of hashes.
################################################################################
sub loadConfig 
{
  my ($filename) = @_;
  my %apps;
  my $key, $value, $who, $buff;
  
  #open conig file
  open (CONFFILE, $filename) or die ("couldn't open $filename");
  # parse config file contents
  while ($buff = <CONFFILE>)
  {
    #print ("$buff\n");
    chomp $buff;
    if ($buff =~ /^\s*#/ or $buff =~ /^\s*$/)
    {
      # do nothing, it's a comment or blank line
    }
    elsif ($buff =~ /^(.*?):\s*$/)
    {
      #process the application name field
      $who = $1;
    }
    elsif ( $buff =~ /^\s+(.*?)\s*=\s*(.*)/)
    {
      $key = trim($1);
      $value = trim($2);
      # process key/value pair
      $apps{$who}{$key} = $value;
    }
  }
  close (CONFFILE);
  return %apps;
}


################################################################################
# spawnthis - spawns a process within a thread
################################################################################
sub spawnthis 
{
  my ($name, $cmdfile,$cmdparms,$nostart,$timeout,$emaillst,$upgrdflag,$upgrdscript,$fallbkflag,$fallbkscript) = @_;
  my @times;
  my $count;
  my $now;

  print ("\n\nStarting thread with parameters:\n"); 
  print ("Name: \"$name\"\n");
  print ("Command: \"$cmdfile $cmdparms\"\n");
  print ("No-Restart file: \"$nostart\"\n");
  print ("Timeout: \"$timeout\"\n");
  print ("Email List: \"$emaillst\"\n");
  print ("Upgrade Flag File: \"$upgrdflag\"\n");
  print ("Upgrade Script: \"$upgrdscript\"\n");
  print ("Fallback Flag File: \"$fallbkflag\"\n");
  print ("Fallback Script: \"$fallbkscript\"\n\n\n");

  while (1)
  {
    if ( -f $nostart)
    {
      sleep (10);
    }
    else
    {
      if ( -f $upgrdflag )
      {
        print ("Running upgrade script: \"$upgrdflag\"\n");
        system ($upgrdscript);
        unlink ($upgrdflag);
      }

      if ( -f $fallbkflag )
      {
        print ("Running fallback script: \"$fallbkscript\"\n");
        system ($fallbkscript);
        unlink ($fallbkflag);
      }

      my $cmd = ("$cmdfile $cmdparms");
      #print ("RUNNING CMD: $cmd\n");
      system ($cmd);
      push @times, time();
      # send email when the app crashes
      $now = localtime();
      sendEmail ("Application $name on $host died", 
                 "The application will be restarted or not based upon\nthe rules in the configuration file and the number of times the app has died\n$now",
                 $emaillst);
    
      $count = @times;
      # delete first entry if count of times is > 10
      if ($count > 10)
      {
        shift @times;
        $count = @times;
      }
      # if the app has died 10 times in the last minute, sleep for 300 seconds
      if ($count == 10)
      {
        if ($times[9] - $times[0] < 60)
        {
          if ( $timeout == 0 )
          {
            touch ($nostart);
            # TODO - send email when the app doesn't restart
            $now = localtime();
            sendEmail ("Application $name on $host won't be restarted", 
                       "The application will not be restarted\nto restart manually, remove the file: $nostart\n$now",
                       $emaillst);
          }
          else 
          {
            sleep ($timeout);
            # TODO - send email when the app goes into timeout
            sendEmail ("Application $name on $host will be restarted after $timeout seconds", 
                       "The application will be restarted after $timeout seconds\n $now",
                       $emaillst);
          }
        }
      }
    }
  }
}

################################################################################
#mainline
################################################################################
$reloadconfig = 0;
$host = `uname -n`;

# Setup a catch of Signal 1 to reload params
$SIG{HUP} = \&triggerReload;

$mailCmd = getEmailCmd();

# get config file name from arg list.
$argcnt = @ARGV;
if ($argcnt < 1) {
  howto();
  die ("\nERROR: You must give a config file name as parameter\n\n");
}
else {
  $filename = shift @ARGV;
}

# load the config file
%apps = loadConfig ($filename);

if (sanitycheck(\%apps))
{
  exit (1);
}

# format the command paramters for each app and spin each app off in it's own thread
for $name ( keys %apps )
{
  @args = ( $name, 
            $apps{$name}{"cmdfile"}, 
            $apps{$name}{"cmdparms"}, 
            $apps{$name}{"nostart"}, 
            $apps{$name}{"timeout"}, 
            $apps{$name}{"emaillst"},
            $apps{$name}{"upgrdfile"},
            $apps{$name}{"upgrdscript"},
            $apps{$name}{"falbkfile"},
            $apps{$name}{"falbkscript"}
  );
  $apps{$name}{"thread"} =  new Thread \&spawnthis, @args; 
}

$now = localtime();
print ("Starting at $now\nConfigured Applications:\n");
report(\%apps);
# loop for ever, check for reload every 5 seconds
while (1)
{
  sleep (5);
  if ( $reloadconfig != 0 )
  {
    $now = localtime();
    print ("Reloading config file - $now\n");

    # re-read the config file.
    %newapps = loadConfig ($filename);
    sanitycheck(\%newapps);
    
    # format the command paramters for each app and spin each app off 
    # in it's own thread
    for $name ( keys %newapps )
    {
      if ( !(exists $apps{$name}))
      {
        @args = ( $name,
                  $newapps{$name}{"cmdfile"},
                  $newapps{$name}{"cmdparms"},
                  $newapps{$name}{"nostart"},
                  $newapps{$name}{"timeout"},
                  $newapps{$name}{"emaillst"},
                  $newapps{$name}{"upgrdfile"},
                  $newapps{$name}{"upgrdscript"},
                  $newapps{$name}{"falbkfile"},
                  $newapps{$name}{"falbkscript"}
        );
        $apps{$name} = $newapps{$name};
        $apps{$name}{"thread"} =  new Thread \&spawnthis, @args; 
      }
    }
    undef %newapps;
    $reloadconfig = 0;
    $now = localtime();
    print ("\nReconfiguring at $now\nConfigured Applications:\n");
    report(\%apps);
  }
}
