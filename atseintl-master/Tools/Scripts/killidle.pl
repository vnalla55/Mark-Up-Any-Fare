#!/usr/bin/perl

# This script looks for sessions which have been idle for too
#     long, and kills them and all their children.


#looks up all processes for a given login session, runs kill on them.
sub log_them_out 
{
  my ($user,$tty,$from,$idle,$what) = @_;
  my @myoutput;

  print ("\n  User: $user has been idle for $idle, on $tty.  I'm logging them out.\n");

  @myoutput = `/bin/ps -lHt $tty`;

  foreach $process (@myoutput)
  {
    chomp ($process);
    ($f,$s,$uid,$pid,$ppid,$c,$pri,$ni,$addr,$sz,$sz,$wchan,$tty,$time,$cmd) = split( /\s+/,$process ); 
    if ( $uid ne 'UID' )
    {
      print ("killing process: $pid\n");
      kill (9,$pid);
    }
  }
}


# -----------------------
# Idle processes Mainline
# -----------------------

# find all login sessions.
@output = `/usr/bin/w -sh | sort`;

foreach $process (@output)
{
  chomp ($process);
  ($user,$tty,$from,$idle,$what) = split( /\s+/,$process );
  # idle time can be listed in <sec>.<fract>, hour:min, or days notation.
  # if it is hour:min or days notation, and > 8 hours kill the process.
  if ( $idle =~ /\d+days/ )
  {
    log_them_out ($user,$tty,$from,$idle,$what);
  }
 # if ( $idle =~ /(\d+):\d+m/ )
 # {
 #   if ( $1 >= 8 )
 #   {
 #     log_them_out ($user,$tty,$from,$idle,$what);
 #   }
 # }
}


