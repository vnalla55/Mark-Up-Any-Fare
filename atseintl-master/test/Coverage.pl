#!/usr/bin/perl

use Cwd;
use File::Basename;

#local $rootDir = "/vobs/atseintl";
#local $rootDir = ".";
local $rootDir = `pwd`;
chop( $rootDir );

# Globals

$pkgLineCount = 0;
$pkgNumFiles = 0;
$pkgCoveredLines = 0;

$projectLineCount = 0;
$projectNumFiles = 0;
$projectCoveredLines = 0;

$projectMaxLineCount = 0;
$projectMaxCoveredLines = 0;
$projectMaxUncoveredLines = 0;

$packageMaxLineCount = 0;
$packageMaxCoveredLines = 0;
$packageMaxUncoveredLines = 0;

# What we need to do is loop over every directory in /vobs/atseintl, and
# recurse the directory structure. The top-level name is what we want to 
# name the packages, so that's kind of special.

opendir( DIR, $rootDir ) || die "Could not open $rootDir\n";
local @entries = readdir( DIR );
closedir( DIR );

@entries = sort( @entries );

printHeader();

local $file;
foreach $file ( @entries )
{
#   if( $file eq "." ) { next; }
   if( $file eq ".." ) { next; }
   if( $file eq "lost+found" ) { next; }
   if( $file eq "obsolete" ) { next; }
   if( $file eq "test" ) { next; }
   if( $file eq "ItinGenerator" ) { next; }

   if( -d $file )
   {
      beginPackage( $file );
      recurseToSubdirectory( $rootDir, $file );
      endPackage();
   }

   # Ensure we are at the root directory level.
   chdir( $rootDir );
}

printFooter();

exit(0);

sub recurseToSubdirectory()
{
   local( $rootDir, $dir ) = (@_);
#   print "chdir to $rootDir/$dir\n";

   local $currentDir = getcwd();

   chdir( "$rootDir/$dir" ) || die "Could not chdir to $rootDir/$dir\n";

   opendir( DIR, "." ) || die "Could not open $rootDir/$dir\n";
   local @entries = readdir( DIR );
   closedir( DIR );

   @entries = sort( @entries );

   local $file;

   # Save directories to be processed after files.

   local @directories;

   @entries = sort( @entries );

   foreach $file ( @entries )
   {
      if( $file eq "." ) { next; }
      if( $file eq ".." ) { next; }
      if( $file eq "sfc" ) { next; }
      if( $file eq "test" ) { next; }

      if( -d $file )
      {
          @directories = (@directories, $file);
      }

      if( -f $file )
      {
          processFile( $file );
      }
   }

   removeUnnecessaryFiles();

   foreach $file ( @directories )
   {
      recurseToSubdirectory( "$rootDir/$dir", $file );
   }

   chdir( $currentDir ) || die "Could not chdir to $currentDir\n";
}

sub printHeader()
{
   print "<project name=\"atseintl\">\n";
}

sub printFooter()
{
   local $realProjectCov = 0;
   if( $projectLineCount > 0 )
   {
      $realProjectCov = $projectCoveredLines / $projectLineCount;
   }
   outputMetrics( $projectLineCount, $realProjectCov * 100 );
   outputAdditionalMetrics( $projectNumFiles );
   print "         <maxcoveredlines value=\"$projectMaxCoveredLines\"/>\n";
   print "         <maxuncoveredlines value=\"$projectMaxUncoveredLines\"/>\n";
   print "         <maxlines value=\"$projectMaxLineCount\"/>\n";
   print "         <maxpkglines value=\"$packageMaxLineCount\"/>\n";
   print "         <maxpkgcoveredlines value=\"$packageMaxCoveredLines\"/>\n";
   print "         <maxpkguncoveredlines value=\"$packageMaxUncoveredLines\"/>\n";

   local $when = gmtime();
   print "         <date value=\"$when\"/>\n";
   print "</project>\n";
}

sub beginPackage()
{
   local( $package ) = (@_);

   $pkgLineCount = 0;
   $pkgNumFiles = 0;
   $pkgCoveredLines = 0;

   print "   <package name=\"$package\">\n";
}

sub endPackage()
{
   local $realPkgCov =  0;

   if( $pkgLineCount > 0 )
   {
       $realPkgCov = $pkgCoveredLines / $pkgLineCount;
   }

   outputMetrics( $pkgLineCount, $realPkgCov * 100);
   outputAdditionalMetrics( $pkgNumFiles );

   $projectLineCount = $projectLineCount + $pkgLineCount;
   $projectNumFiles  = $projectNumFiles + $pkgNumFiles;
   $projectCoveredLines  = $projectCoveredLines + $pkgCoveredLines;

   local $pkgUncoveredLines = $pkgLineCount - $pkgCoveredLines;

   if( $pkgLineCount > $packageMaxLineCount )
   {
      $packageMaxLineCount = $pkgLineCount;
   }

   if( $pkgCoveredLines > $packageMaxCoveredLines)
   {
      $packageMaxCoveredLines = $pkgCoveredLines;
   }
   
   if( $pkgUncoveredLines > $packageMaxUncoveredLines)
   {
      $packageMaxUncoveredLines = $pkgUncoveredLines;
   }

   print "   </package>\n";
}

sub processFile()
{
    local( $file ) = (@_);

    local $output;

    local( $name, $path, $suffix ) = fileparse( $file, (".cpp", ".C") );
#    local( $name, $path, $suffix ) = fileparse( $file, (".cpp", ".C", ".h") );

    if( $suffix ne "" )
    {
#      if( $name !~ m/.*Test$/)
#      {
#        if( $name !~ m/^Mock.*/)
#        {
       
           beginMetrics( $file );

           if( -e "$name.da" )
           {
              # local $gcovFilter = "| grep -v Creating | grep -v /opt | grep -v \"\\.h\" | grep -v \"\\.bb\"| grep -v \"\\.da\" | grep -v \"Assuming that all\" | grep -v \"Permission denied\" | grep -v \"Unexpected EOF\" | grep -v \"Mock\" | grep -v \"Test\" | grep -v exhausted 2>&1";
              local $gcovFilter = "| grep -v Creating | grep \"$name$suffix\" 2>&1";
              $output = `gcov $file $gcovFilter`;
              parseOutput( $output );
           }
           else
           {
              emptyMetrics( $file );
           }

           endMetrics();
        }
#      }
#    }
}

sub beginMetrics()
{
    local ($file) = (@_);
    print "      <file name=\"$file\">\n";
}

sub endMetrics()
{
    local ($file) = (@_);
    print "      </file>\n";
}

sub emptyMetrics()
{
    local ($file) = (@_);

    local $linecount = `wc -l $file`;
    chop( $linecount );
    $linecount =~ s/\s+(.*?)\s+(.*)/$1/;

    outputMetrics( $linecount, 0 );

    $pkgLineCount = $pkgLineCount + $linecount;
    $pkgNumFiles = $pkgNumFiles + 1;
}

sub parseOutput()
{
    local ( $output ) = (@_);
    chop( $output );

    $coverage = $output;
    $coverage =~ s/\s*(.*?)% of .*/$1/;

    $linecount = $output;
    $linecount =~ s/.* of (.*?) source.*/$1/;

    outputMetrics( $linecount, $coverage );

    $pkgLineCount = $pkgLineCount + $linecount;
    $pkgNumFiles = $pkgNumFiles + 1;

    local $coveredLines = $coverage * $linecount / 100;

    $pkgCoveredLines = $pkgCoveredLines + $coveredLines;

    if( $coveredLines > $projectMaxCoveredLines )
    {
       $projectMaxCoveredLines = $coveredLines;
    }

    if( $linecount - $coveredLines > $projectMaxUncoveredLines )
    {
       $projectMaxUncoveredLines = $linecount - $coveredLines;
    }

    if( $linecount > $projectMaxLineCount )
    {
       $projectMaxLineCount = $linecount;
    }
}

sub outputMetrics()
{
    local ( $lineCount, $coverage ) = (@_);

    if( $lineCount eq "" ) { $lineCount = 0; }
    if( $coverage eq "" ) { $coverage = 0; }

    print "         <coverage value=\"$coverage\"/>\n";
    print "         <linecount value=\"$lineCount\"/>\n";
}

sub outputAdditionalMetrics()
{
    local ( $numFiles ) = (@_);
    print "         <filecount value=\"$numFiles\"/>\n";
}

# If any file ending with ".gcov" exists in the current directory, and it
# doesn't have a corresponding file without the .gcov extension, remove it.

sub removeUnnecessaryFiles()
{
   opendir( DIR, "." ) || die "Could not open $rootDir/$dir\n";
   local @entries = readdir( DIR );
   closedir( DIR );
   local $file;

   # Save directories to be processed after files.

   local @directories;

   foreach $file ( @entries )
   {
      if( $file eq "." ) { next; }
      if( $file eq ".." ) { next; }

      if( ! -d $file )
      {
         local( $name, $path, $suffix ) = fileparse( $file, (".gcov") );

         if( $suffix ne "" )
         {
            if( -e "$name.gcov" && ! -e "$name" )
            {
               unlink "$name.gcov";
            }
         }
      }
   }

}

