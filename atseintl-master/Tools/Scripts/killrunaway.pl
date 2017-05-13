#!/usr/bin/perl

#  This script looks for programs known to run-away, and kill 
#     them if they meet the criteria to be run-aways.
#     (no longer tty associated, cpu > 80%)


# What we are looking for are sessions that have no associated terminal 
# anymore. We can find those by doing a ps auxww, searching for vims, and 
# then checking to see if the tty column is a "?". Remember that the 
# ps auxww format is:
#
# USER       PID %CPU %MEM   VSZ  RSS TTY      STAT START   TIME COMMAND
# root         1  0.0  0.0  1320  408 ?        S    Feb26   0:58 init [5]   --init

@output = `/bin/ps auxww | /bin/egrep "vim|emacs|dcopserver_shut"`;
%proclist = ();

# first we build the list of processes to watch.
foreach $process (@output)
{
  chomp ($process);
  ($user,$pid,$cpu,$mem,$vsz,$rss,$tty,$stat,$start,$time,$command)
    = split( ' ',$process );

  if( $tty eq "?" )
  {
    if( $cpu > 80 )
    {
      print ("Process $pid is at $cpu\%\n");
      $proclist{$pid} = $cpu;
    }
  }
}


$proccnt = keys(%proclist);

# if we found anything on the last look,
if ($proccnt > 0)
{
  # then we wait 30 seconds, and see if they are still giving us problems.  
  sleep (30);
  
  # process through the list, and see if they still meet the criteria as 
  # run-aways.  
  
  # Get all suspect processes again
  @output = `/bin/ps auxww | /bin/egrep "vim|emacs|dcopserver_shut"`;
  
  # loop through the suspect processes, compare them to the last time.
  foreach $process (@output)
  {
    chomp ($process);
    ($user,$pid,$cpu,$mem,$vsz,$rss,$tty,$stat,$start,$time,$command)
      = split( ' ',$process );
  
    if( $tty eq "?" )
    {
      if( $cpu > 80 )
      {
        # if the processes existed 30 seconds ago, kill them.
        if (exists ($proclist{$pid}))
        {
          `/usr/bin/kill -9 $pid `;
           print ("Killed process: $pid, which belongs to $user \n");
           print ("Command: $command\n");
        }
      }
    }
  }
}
