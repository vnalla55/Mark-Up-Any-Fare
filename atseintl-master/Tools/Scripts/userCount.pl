#!/usr/bin/perl -w

# get the unsorted list of system counts, and sort them.
@rsystems = `/bin/su  -c "/opt/atseintl/bin/userCountUnSorted.pl | /bin/sort" maker`;

# print the system counts.
foreach $sys (@rsystems)
{
  print $sys;
}


# print user count for atsela01 last
@userlist = `/opt/atseintl/bin/userlist.sh`;
$usercnt = @userlist;
print ("${usercnt}: atsela01.dev.sabre.com\n");
