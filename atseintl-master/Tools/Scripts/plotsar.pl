#!/usr/bin/perl 
#
#  This script is run by maker's cron to get the system stats, graph them, and 
#  place them on a website.  It resides in /login/maker/bin
#
use Cwd;

# run sar on each of the machines, parsing the output from sar, and build
#  an input file for gnuplot.
#
$webbasedir = "/opt/atseintl/doc/sar.reports";

#@machines = ("atsela01"); 
@machines = ("atsela01", 
			 "atsela02",
			 "atsela03",
			 "atsela04",
			 "atsela05",
			 "atselb01",
			 "atselb02",
			 "atselb03",
			 "atselb04",
			 "atselb05" );

@months = ("Jan.","Feb.","March","April","May" ,"June",
           "July","Aug.","Sept.","Oct." ,"Nov.","Dec.");
@dow = ("Sun.","Mon.","Tue.","Wed.","Thur.","Fri." ,"Sat.");

$SECONDS_IN_DAY=60*60*24;
$BYTES_IN_GIGABIT=(1024**3)/8;
$days_ago = 1; # default processing to yesterday

%HOUR_FRACTS = ("00", ".00",
              "10", ".17",
              "20", ".33",
              "30", ".50",
              "40", ".66",
              "50", ".83");

#-------------------------------------------------------------------------------
# subroutines
#-------------------------------------------------------------------------------
sub get_date
{
  my ($days_past) = @_;
  my $sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst,$date;

  ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = 
     localtime(time() - ($SECONDS_IN_DAY * $days_past));
  $year+=1900;
  $mon++;
  $date = sprintf ("%04d%02d%02d", ${year},${mon},${mday});
#  $date = "${year}${mon}${mday}";
  return (${date},$dow[$wday]);
}


#-------------------------------------------------------------------------------
# Mainline
#-------------------------------------------------------------------------------
# check commandline for an override date
$argcnt = @ARGV;
if ( $argcnt > 0 )
{
  if ($ARGV[0] !~ /^\d+$/)
  {
    print (STDERR "Invalid number, valid arguments:\n ");
    print (STDERR "<n> Where n = number of days in past to process sar data.\n");
    print (STDERR "    Default = 1. \n");
	exit 0;
  }
  $days_ago = $ARGV[0];
}

$stime = time - ($SECONDS_IN_DAY * $days_ago);
($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime($stime);

# create the directories to hold the information.
# use the date for yesterday, we want to run sar for yesterday to get the 
# whole day's stats (sar as setup on RedHat Linux systems archive 9 days 
# worth of stats.
$date = sprintf ("%04d%02d%02d",${year}+1900, ${mon}+1, ${mday});
$pdate = sprintf ("%s %02d, %02d", $months[${mon}],${mday},${year}+1900);
$datedir = "${webbasedir}/${date}";
$safile = sprintf ("/var/log/sa/sa%02d", ${mday});
$downame = $dow[${wday}];

  if ( ! -f "${safile}" )
  {
    print (STDERR "no SA file for that date ${pdate} available.\n");
	exit 0;
  }

# Setup the directory to store the files in.
#-------------------------------------------------------------------------------
if ( ! -d  ${datedir} )
{
	`mkdir ${datedir}`;
}
else
{
	`cd ${datedir}; rm -rf *`;
}

# gather and process the SAR data, create input files for gnuplot, call gnuplot
# and create the machine specific web pages.
#-------------------------------------------------------------------------------
if ( -d ${datedir} )
{
	foreach $machine (@machines)
	{
	   # read the output for network traffic, filter it, and put it in 
	   # terms of percent of bandwidth.  Create a time based hash. 
	   # ------------------------------------------------------------- 
	   open (SARCMD, "rsh $machine /usr/bin/sar -f ${safile} -n DEV |") 
	      or die ("Could not run 'sar -n DEV' command on ${machine}."); 
	   @tmpdata = <SARCMD>;
	   close SARCMD;
	   foreach $line (@tmpdata) 
	   {
	      ($time,$iface,$rxpk,$txpk,$rxbyte,$txbyte) = split( /\s+/,$line );
		  if ($iface eq "eth0" and $time ne "Average:")
		  {
			($thh, $tmm,) = split ( /:/,$time);
			$time = "${thh}:${tmm}";
			$totbytes = ($rxbyte + $txbyte);
			$percent = $totbytes / $BYTES_IN_GIGABIT * 100;
			$netdata{$time} = sprintf ("%20.2f       %3.5f", $totbytes, $percent);
		  }
	   }

       # read the output of sar for run queue (not relevent, leave for future)
	   #`rsh $machine /usr/bin/sar -f ${safile} -q`;

	   # read the output of sar for memory put it in a time based hash
	   # rsh $machine /usr/bin/sar -f ${safile} -r
	   # -------------------------------------------------------------
	   open (SARCMD, "rsh $machine /usr/bin/sar -f ${safile} -r |") 
	      or die ("Could not run 'sar -r' command on ${machine}."); 
	   @tmpdata = <SARCMD>;
	   close SARCMD;
	   foreach $line (@tmpdata) 
	   {
		  if ( $line !~ /kbmemfree/ and $line !~ /Linux/ and $line !~ /^\s*$/)
		  {
	         ($time,$ampm,$memfr,$mempct,$memsh,$buffs,$cach,$swpfr,$spwus,$swppct) 
			     = split( /\s+/,$line );
			($thh, $tmm,) = split ( /:/,$time);
			$time = "${thh}:${tmm}";
			$memdata{$time} = sprintf ("%3.2f", $swppct);
		  }
	   }

	   # read the output of sar for cpu utilization. put it in a time based hash
	   # rsh $machine /usr/bin/sar -f ${safile} -u;
	   # -----------------------------------------------------------------------
	   open (SARCMD, "rsh $machine /usr/bin/sar -f ${safile} -u|") 
	      or die ("Could not run 'sar -u' command on ${machine}."); 
	   @tmpdata = <SARCMD>;
	   close SARCMD;
	   foreach $line (@tmpdata) 
	   {
		  if ( $line !~ /CPU/ and $line !~ /Linux/ and $line !~ /^\s*$/)
		  {
	        ($time,$cpu,$usrl,$nice,$sys,$idle) = split( /\s+/,$line );
			($thh, $tmm,) = split ( /:/,$time);
			$time = "${thh}:${tmm}";
			$cpudata{$time} = sprintf ("%3.2f", (100 - $idle));
		  }
	   }
	   
	   # Print the data file from the processed data in the hash tables
	   # --------------------------------------------------------------
	   if ( open (TXTFILE, ">${datedir}/${machine}.txt") )
	   {
	     print (TXTFILE "#System Statistics  for ${machine} for date $downame}, {$pdate} \n");
	     print (TXTFILE "#The time must be represented in hours.fraction of hour \n");
	     print (TXTFILE "#for correct representation in the graphing program \n");
	     print (TXTFILE "#Time:   Network Traffic (B/s)    bndwdth %    swap %   CPU% \n");
	     print (TXTFILE "#-----   ---------------------    ----------  -------   ------\n");

	     foreach $rpttm (sort(keys(%netdata)))
	     {
		   ($rhh, $rmm,) = split ( /:/,$rpttm);
		   $mrpttm = "${rhh}$HOUR_FRACTS{$rmm}";
	       print (TXTFILE "$mrpttm     $netdata{$rpttm}     $memdata{$rpttm}    $cpudata{$rpttm}\n");
	     }
		 close (TXTFILE);
	   }
       else
	   {
	     print (STDERR "Could not open output text file ${machine}.txt\n");
	   }
	   %netdata = ();
	   %memdata = ();
	   %cpudata = ();

       # call gnuplot to generate the graphics from the data for this machine
       # --------------------------------------------------------------------

	   $gnuplotCmds=" set ylabel \"Percentage of Capacity (Avg. over 10 min. interval)\"
set xlabel \"Time of day (Military Time Notation)\"
set title \"${machine} Resource Utilization for ${downame}, ${pdate} \"
set yrange [0:120]
set ytics (10,20,30,40,50,60,70,80,90,100)
set xrange [0:24]
set xtics (3,6,9,12,15,18,21,24)
set grid
set term png small color
plot \"${machine}.txt\" u 1:3 t \"Net Bandwidth %\" w lines,\\
     \"${machine}.txt\" u 1:4 t \"Swap Space %\" w lines,\\
     \"${machine}.txt\" u 1:5 t \"CPU %\" w lines
";
	   chdir ("${datedir}");
       open (GNUPLOT, "| /usr/bin/gnuplot > ${machine}.png") or die "cannot run gnuplot";
       print (GNUPLOT $gnuplotCmds);
       close GNUPLOT;
	   chdir ("$dir");
	}


    # Create the index web page for this date dir.
    # --------------------------------------------
	open (INDEX, ">${datedir}/index.html") 
       or die ("cannot open ${datedir}/index.html for writing");
	print (INDEX "<HTML>\n");
	print (INDEX "<HEAD> <TITLE> ATSE International server stats for ${downame}, ${pdate} </TITLE>\n");
	print (INDEX "</HEAD>\n<BODY>\n");
	print (INDEX "<H1> System Access reports for ${downame}, ${pdate} </H1>\n");
    foreach $machine (@machines)
	{
	  if ( -f "$datedir/$machine.png" or  -f "$datedir/$machine.txt")
	  {
	    print (INDEX "<H2>${machine}\n</H2>\n");

	    if ( -f "$datedir/$machine.png" )
	    {
	      print (INDEX "<IMG SRC=\"${machine}.png\"\n<BR>\n");
	    }
	    if ( -f "$datedir/$machine.txt" )
	    {
	      print (INDEX "<A HREF=\"${machine}.txt\"> Source Data </A> \n");
	    }
	    print (INDEX "<HR>\n");
	  }
	}
	print (INDEX "</BODY>\n</HTML>\n");
	close (INDEX);

    # Create the index web page for the base dir
    # --------------------------------------------
    opendir (BASEDIR, "$webbasedir") or die "cannot open $webbasedir for reading";
	@dirlist = readdir (BASEDIR);
    closedir (BASEDIR);

	#cleanse the list.  It should only contain directories which are numeric.
	for $entry (@dirlist)
	{
	  if ( $entry =~ /^\d+$/ )
	  {
	    push (@newlist, $entry);
	  }
	}
	$dircnt = @dirlist;
	@dirlist=@newlist;

	if ($dircnt > 0)
	{
	  #
	  # find day_of_week name for each of the dates represented by directories.
	  #

	  # first sort the list of date-named directories.
      @sortlist = sort(@dirlist);
	  $leastdir = $sortlist[0];

	  # preload the hastable with key values from sorted list.
	  foreach $entry (@sortlist)
	  {
	    $dirhash{$entry} = "x";
	  }

	  # load our hash table with the dow names that match the date in the key.
      # preload currdir
	  ($currdir,$currdow) = get_date (0);
	  for ($i = 1; $leastdir le $currdir; $i++)
	  {
		if (exists ($dirhash{$currdir} ))
		{
		  $dirhash{$currdir} = $currdow;
		}
	    ($currdir,$currdow) = get_date ($i);
	  }

	  # now, print the index file.
	  open (INDEX, ">${webbasedir}/index.html") or 
	      die ("cannot open ${webbasedir}/index.html for writing");
	  print (INDEX "<HTML>\n");
	  print (INDEX "<HEAD> <TITLE> ATSE International server stats</TITLE>\n");
	  print (INDEX "</HEAD>\n<BODY>\n");
	  print (INDEX "<H1> ATSE International server stats </H1>\n");
	  print (INDEX "<HR>\n");
      print (INDEX "<UL>\n");
	  foreach $file (reverse(sort(@dirlist)))
	  {
	    $file =~ /^(\d\d\d\d)/;
	    $lyear = $1;
		$file =~ /^\d\d\d\d(\d\d)/;
		$lmon = $1;
		$file =~ /(\d\d)$/;
		$lday = $1;
		$pdate = "$months[$lmon - 1 ] $lday, $lyear";
		if ($dirhash{$file} eq "Sat.")
		{
		  print (INDEX "</UL>\n<HR>\n<UL>\n");
		}
	    print (INDEX "<LI> <A href=\"${file}\"> $dirhash{$file}, ${pdate} </A></LI>\n");
	  }
      print (INDEX "</UL>\n");
	  print (INDEX "</BODY>\n</HTML>\n");
	  close (INDEX);
    }
} 
else 
{
	print ("Could not create run directory ${rundir}, exiting \n");
}

