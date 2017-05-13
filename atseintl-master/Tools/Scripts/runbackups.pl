#!/usr/bin/perl

# Global config
$BASEDIR = "/root/backups";
$BKUPDIR = "/backups";


################################################################################
# log_info - just open this file, and write the log message into it, close it
#            behind me.
################################################################################
sub log_info 
{
  my ($log_fn, $str) = @_ ;
  open (LOG_FILE, ">> $log_fn") or die ("Can't open log file, $log_fn");
  print (LOG_FILE "$str\n");
  close (LOG_FILE);
} 

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
# example - prints an example file ready to be redirected into a config file
#           and edited.
################################################################################
sub example
{
  print ("# Example Config file, just redirect it to a file and edit it\n");
  print ("global: \n");
  print ("   emaillst = user\@company.com\n");
  print ("   cleanup  = 28\n");
  print ("root: \n");
  print ("   hostname  = <hostname>\n");
  print ("   topdir    = /\n");
  print ("   levSun    = F\n");
  print ("   levMon    = I\n");
  print ("   levTue    = I\n");
  print ("   levWed    = I\n");
  print ("   levThu    = I\n");
  print ("   levFri    = I\n");
  print ("   levSat    = I\n");
  print ("   fsrestrct = Y\n");
} 

################################################################################
# howto - print a howto message
################################################################################
sub howto
{
  print ("\n# Command: runbackups.pl [-h] [-e]\n");
  print ("#\n");
  print ("#         -h - prints this help message\n");
  print ("#         -e - prints an example config file, ready to be redirected \n");
  print ("#              into a config file, then edited. \n");
  print ("#\n");
  print ("# The application, \"runbackups.pl\" runs the backups configured in the \n");
  print ("# backups.conf file.  Runbackups needs a directory heirarchy of a basedir  \n");
  print ("# with 3 directories, bin, etc, and logs.  This script is in the bin \n");
  print ("# directory, the config file belongs in the etc directory and the log \n");
  print ("# files from the backup will be placed in the logs directory.  Runbackups.pl \n");
  print ("# works through ssh, so you have to setup ssh to work from root on the backup\n");
  print ("# server to root on the remote server\n");
  print ("#\n");
  print ("# The config file must obey the following rules:\n");
  print ("#\n");
  print ("#\n");
  print ("# Config File Rules:\n");
  print ("# ------------------\n");
  print ("# 1. The backup-name lines must end with a ':'.\n");
  print ("# 2. The parameter lines must start with at least one white space character.\n");
  print ("# 3. Any line beginning with '#' is considered a comment.\n");
  print ("# 4. Comments are not allowed at the end of a line containing a param or app\n");
  print ("#     name, a comment there is taken to be part of the param.\n");
  print ("#\n");
  print ("# The Config file follows the following format\n");
  print ("# global: <There must be a \"global\" section in the config file.>\n");
  print ("#    emaillst = <REQUIRED, Space delimited list of who to email\n");
  print ("#                          when a backups fails>\n");
  print ("#    cleanup   = <REQUIRED, the number of days to retain old backups and logs.>\n");
  print ("# bkupname: <you pick it, it doesn't matter what it says, must be unique>\n");
  print ("#    hostname  = <REQUIRED, the name of the host to be backed up>\n");
  print ("#    topdir    = <REQUIRED, the top directory for the backup on the remote host>\n");
  print ("#    levSun    = <REQUIRED, the backup level(F/I) for Sunday>\n");
  print ("#    levMon    = <REQUIRED, the backup level(F/I) for Monday>\n");
  print ("#    levTue    = <REQUIRED, the backup level(F/I) for Tuesday>\n");
  print ("#    levWed    = <REQUIRED, the backup level(F/I) for Wednesday>\n");
  print ("#    levThu    = <REQUIRED, the backup level(F/I) for Thursday>\n");
  print ("#    levFri    = <REQUIRED, the backup level(F/I) for Friday>\n");
  print ("#    levSat    = <REQUIRED, the backup level(F/I) for Saturday>\n");
  print ("#    fsrestrct = <If this param is \"Y\" or \"y\", this backup will be restricted>\n");
  print ("#                          to a single file system (usually used for root partitions>\n");
}

################################################################################
# sanitycheck - checks the bkups process table for possible mistakes,
#               removes entries which don't pass.
################################################################################
sub sanitycheck
{
  ($bkups) = @_;
  my $name;
  my $key;
  my $prtbuf = "";

  # Check first for the global section.
  if ( ! (exists $$bkups{"global"})){
    $prtbuff="$prtbuff\nERROR: no global section in config file"; }
  if ($$bkups{"global"}{"emaillst"} eq ""){
    $prtbuff="$prtbuff\nERROR: no mailing list defined in config file global section"; }
  if ($$bkups{"global"}{"cleanup"} eq ""){
    "$prtbuff\nERROR: no cleanup value defined in config file global section"; }

  # next check the backup sections
  for $name ( keys %$bkups )
  { 
    # we've already checked the global section, don't check it again, it has different rules.
    if ($name ne "global" ) 
    {
      if ($$bkups{$name}{"hostname"} eq ""){
        $prtbuff="$prtbuff\nERROR: $name is missing hostname param."; }
      if ($$bkups{$name}{"topdir"} eq ""){ 
        $prtbuff="$prtbuff\nERROR:  $name is missing topdir param."; }
      if ($$bkups{$name}{"levSun"} eq ""){ 
        $prtbuff="$prtbuff\nERROR:  $name is missing levSun param."; }
      if ($$bkups{$name}{"levMon"} eq ""){ 
        $prtbuff="$prtbuff\nERROR:  $name is missing levMon param."; }
      if ($$bkups{$name}{"levTue"} eq ""){ 
        $prtbuff="$prtbuff\nERROR:  $name is missing levTue param."; }
      if ($$bkups{$name}{"levWed"} eq ""){ 
        $prtbuff="$prtbuff\nERROR:  $name is missing levWed param."; }
      if ($$bkups{$name}{"levThu"} eq ""){ 
        $prtbuff="$prtbuff\nERROR:  $name is missing levThu param."; }
      if ($$bkups{$name}{"levFri"} eq ""){ 
        $prtbuff="$prtbuff\nERROR:  $name is missing levFri param."; }
      if ($$bkups{$name}{"levSat"} eq ""){ 
        $prtbuff="$prtbuff\nERROR:  $name is missing levSat param."; }
    }
  }
  if ($prtbuff ne "")
  {
    die ("$prtbuff\n");
  }
}


################################################################################
# loadConfig - loads the application data from the config file into a 
#              hash of hashes.
################################################################################
sub loadConfig 
{
  my ($filename) = @_;
  my %bkups;
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
      $bkups{$who}{$key} = $value;
    }
  }
  close (CONFFILE);
  return %bkups;
}


################################################################################
# debugreport - print a list of backups which are configured
################################################################################
sub debugreport
{
  ($bkups) = @_;
  my $name;
  my $key;

  for $name ( keys %$bkups )
  {
    print ("Backup Name: \"$name\"\n");
    for $key ( keys %{ $$bkups{$name} } )
    {
      print ("    \"$key\" = \"$$bkups{$name}{$key}\"\n");
    }
  }
}


################################################################################
# create the file name of the incremental backup marker.
################################################################################
sub create_incremental_marker_fn 
{
  my ($name,$host) = @_;
  return "$configdir/${host}_${name}_last_incremental.marker";
}

################################################################################
# create the file name of the full backup marker.
################################################################################
sub create_full_marker_fn 
{
  my ($name,$host) = @_;
  return "$configdir/${host}_${name}_last_full.marker";
}

################################################################################
# get the file name of the last full backup marker.
################################################################################
sub get_last_full_marker_fn 
{
  my ($name,$host) = @_;
  my $filename;

  # TODO - look and see if a full backup marker file exists.
  $filename = create_full_marker_fn($name,$host);
  if ( -f $filename ) 
  {
    return $filename;
  } else {
    return "";
  }
}


################################################################################
# create a log file name
################################################################################
sub create_log_fn
{
  my ($name, $host, $topdir, $level, $now) = @_;
  my $cnt = 0;

  do
  {
    if ($cnt)
    {
      $log_fn = "$logdir/${host}_${name}_${level}_${now}_${cnt}.log";
    } else {
      $log_fn = "$logdir/${host}_${name}_${level}_${now}.log";
    }
  } while ( -f $log_fn );
  return ($log_fn);
}


################################################################################
# create a backup file name
################################################################################
sub create_backup_fn
{
  my ($name, $host, $topdir, $level, $now) = @_;
  my $cnt = 0;

  do
  {
    if ($cnt)
    {
      $backup_fn = "$BKUPDIR/${host}_${name}_${level}_${now}_${cnt}.tar.gz";
    } else {
      $backup_fn = "$BKUPDIR/${host}_${name}_${level}_${now}.tar.gz";
    }
  } while ( -f $backup_fn );
  return ($backup_fn);
}


################################################################################
# run a backup.
################################################################################
sub backup 
{
  my ($name, $host,$topdir, $level,$fs_restrict) = @_;
  my $startfile = "$configdir/${host}_${name}_${level}_${now}_startfile";
  my $cmd;
  my $retval;
  my $fs_restrict_str;
  my $newer_str;
  my $last_full_fn;
  my $backup_fn;
  my $log_fn;
  my $now;
  my $last_full_time;
  my $bnow = time(); # binary time for start of backups
  my $snow = localtime($bnow); # human readable time string
  my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime($bnow);
  $year += 1900;
  $mon++;
  $now = sprintf("%04d%02d%02d%02d%02d%02d",$year,$mon,$mday,$hour,$min,$sec);

  $last_full_marker_fn = get_last_full_marker_fn($name, $host);
  $full_marker_fn = create_full_marker_fn($name, $host);
  $incremental_marker_fn = create_incremental_marker_fn($name, $host);

  # create a backup "start" file, which simply captures the start time of the backup.
  `date > $startfile`;
  
  # setup the variables for fs restrict, and date.
  if ($level eq "I" or $level eq "i") # scheduled to be incremental backup
  {
    if ($last_full_marker_fn eq "") # if no previous full backup
    {
      $level = "F"; #force a full backup
      $newer_str = ""; # no previous full backup to key off of for incremental build.
    } else {
      $last_full_time = `cat $last_full_marker_fn`;
      chomp $last_full_time;
      $newer_str = "--newer \'$last_full_time\'"; # backup all files newer than the last full marker file
    }
  } else {
    # cleanup old logs and backup files if the backup is a full
    cleanup($cleanup, $logdir, $BKUPDIR, $name);
  }

  if ($fs_restrict eq "Y" or $fs_restrict eq "y") # restrict this backup to one file system only
  {
    $fs_restrict_str = "--one-file-system"; 
  }

  $backup_fn = create_backup_fn ($name, $host, $topdir, $level, $now);
  $log_fn = create_log_fn ($name, $host, $topdir, $level, $now);
  
  # create the backup command
  $cmd = "ssh $host \"cd $topdir \;tar cvzf - $fs_restrict_str $newer_str .\"  > $backup_fn 2>> $log_fn";

  # first make sure the log file is there, otherwise we can't write into it with an "append"
  touch ($log_fn);

  # log the backup information.
  log_info ($log_fn, "Backup: $name \nHost: $host\nLevel: $level");

  # log the begin time of the backup
  log_info ($log_fn, "Time Started: $snow");
  
  # log the command to run the backup
  log_info ($log_fn, "Running backup command: \"$cmd\"");
  log_info ($log_fn, "-----------------------------------------------------------------------\n");

  # run the backup.
  $retval = system("$cmd");

  # if the backup was successful
  if (! ($retval))
  {
    # if the backup was a full - move the "start" file to "<hostname>_last_full
    if ($level eq "F")
    {
      $cmd = "mv $startfile $full_marker_fn";
    }
    # if the backup was a incremental - move the "start" file to $incremental_marker_fn
    else
    {
      $cmd = "mv $startfile $incremental_marker_fn";
    }
    #print "Running move command: \"$cmd\"\n";
    system ($cmd);
  } else {
    unlink $startfile;
    print (STDERR "Error in backup $name, check log file \"$log_fn\"\n");
  }
  # log the end time of the backup
  $snow = localtime();
  log_info ($log_fn, "\n-----------------------------------------------------------------------");
  log_info ($log_fn, "Time completed: $snow");

}


################################################################################
#cleanup - cleanup the logfiles and backup tar files.
################################################################################
sub cleanup 
{
  my ($days, $logdir, $tardir,$bkup) = @_;

  # first delete the logs
  $cmd = "cd ${logdir};find . -mtime +${days} -name \"\*${bkup}\*\" -exec /bin/rm \"\{\}\" \\\;";
  `$cmd`;
  # then delete the tar files.
  $cmd = "cd ${tardir};find . -mtime +${days} -name \"\*${bkup}\*\" -exec /bin/rm \"\{\}\" \\\;";
  `$cmd`;


}

################################################################################
#mainline
################################################################################
# directories, where to find things.
$bindir = "$BASEDIR/bin";
$configdir = "$BASEDIR/etc";
$logdir = "$BASEDIR/logs";
$filename = "$configdir/backups.conf";
$mailCmd = getEmailCmd();
($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime();
$dow = (Sun,Mon,Tue,Wed,Thu,Fri,Sat)[$wday];
$lev_ind = "lev$dow";

# touch a file to tell when it ran last
#touch ("$BASEDIR/last_run_pl");

# check command line args 
$argcnt = @ARGV;
if ($argcnt >= 1 and $ARGV[0] eq "-h" ) {
  howto();
  exit(0);
}
if ($argcnt >= 1 and $ARGV[0] eq "-e" ) {
  example();
  exit(0);
}

# load the config file
%bkups = loadConfig ($filename);

# sanitycheck the config file
sanitycheck(\%bkups);

# strip off "global" section, set global variables, leaving only real backups in the %bkups table.
$email_list = $bkups{"global"}{"emaillst"};
$cleanup = $bkups{"global"}{"cleanup"};
delete $bkups{"global"};

#debugreport(\%bkups);

#for each backup defined in the %bkups table, 
foreach $name ( keys %bkups) 
{
  # run the backup.
  backup ($name, $bkups{$name}{"hostname"}, $bkups{$name}{"topdir"}, 
          $bkups{$name}{$lev_ind}, $bkups{$name}{"fsrestrct"});
}


