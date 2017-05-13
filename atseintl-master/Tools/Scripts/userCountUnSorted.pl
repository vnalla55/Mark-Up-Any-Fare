#!/usr/bin/perl -w

# Setup the list of remote systems for the user count app
@rsystems= (
  "atsela02.dev.sabre.com",
  "atsela03.dev.sabre.com",
  "atsela04.dev.sabre.com",
  "atsela05.dev.sabre.com",
  "atselb01.dev.sabre.com",
  "atselb02.dev.sabre.com",
  "atselb03.dev.sabre.com",
  "atselb04.dev.sabre.com",
  "atselb05.dev.sabre.com"
);

# Get the unsorted list of user counts by using rsh to get the user list from
# each remote system..
foreach $sys (@rsystems)
{
  @userlist = `/usr/kerberos/bin/rsh $sys /opt/atseintl/bin/userlist.sh`;
  $usercnt = @userlist;
  print ("${usercnt}: ${sys}\n");
}
