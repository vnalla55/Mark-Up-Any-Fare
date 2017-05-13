#!/usr/bin/perl

use Getopt::Long;

GetOptions( "b=s" => \$baseline,
	  );

@links = `find . -type l -lname '/project/atsesvn/idop_export*' -print`;

for $link (@links)
	{
		chomp ($link);
		$str = `ls -l $link`;
		($trash, $ptr) = split (/->/,$str);
		chomp ($ptr);

		$oldptr = $ptr;

		$ptr =~ s/(.*)\/airserv-idop\.\d\d\d\d\.\d\d\.\d\d\/(.*)/$1\/$baseline\/$2/g;

		unlink("$link");
		$retstr = `ln -s $ptr $link`;
	}
exit;
