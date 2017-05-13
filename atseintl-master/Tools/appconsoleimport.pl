#!/usr/bin/perl
use warnings;
use strict;
use LWP::UserAgent;
use HTTP::Cookies;

my $quiet = 0;
if ( $#ARGV >= 0 ) {
  if ( $ARGV[0] eq "--quiet") {
    $quiet = 1;
  }
}

my %hostEnvs;
# This function de-HTMLizes a given URL. It's very rudimentary, but it's enough for this script.
sub sanitize
{
  my $url = $1;
  $url =~ s/&amp;/&/;
  return $url;
}

sub myprint
{
  if ($quiet == 0) {
     my $text = shift;
     print $text;
  }
}

sub printEnv
{
  my $env = uc shift;
  my $base = shift;

  if ($env eq "PROD") {
  } elsif ($env eq "CERT") {
  } elsif ($env eq "INT") {
  } else {
    die "Environment $env is unknown! ";
  }

  myprint ("\nDownloading $env from $base\n");

  my $ua = LWP::UserAgent->new();
  $ua->agent("Test.pl/0.1 ");
  $ua->cookie_jar (HTTP::Cookies->new());

  # Get the main page. Should be successful. It should, however, contain just the login form.
  # However, we have to request this first as it starts a session and sets a cookie.
  myprint ("Requesting the index...");
  
  my $req = HTTP::Request->new(GET => $base);
  my $res = $ua->request($req);
  if (! $res->is_success ) {
    myprint ("failed: ". $res->status_line. "\n");
    return 1;
  }
  myprint (" done.\n");

  # Go on and login:
  myprint "Trying to log in...";
  $req = HTTP::Request->new(POST => "$base/j_security_check");
  $req->content_type('application/x-www-form-urlencoded');
  $req->content('j_username=guest&j_password=guest');

  # Pass request to the user agent and get a response back
  $res = $ua->request($req);

  # Check the login outcome. We should get redirected to a 
  if ($res->is_success) {
    myprint " Unexpected server response: " .  $res->status_line . "\n";
  #    print $res->content;
    return 1;
  }
  elsif ($res->is_redirect) {
    myprint " ok, redirected to " . $res->header("Location") . "!\n";
    $req = HTTP::Request->new(GET => $res->header("Location"));
    $res = $ua->request($req);
  }
  else {
    myprint $res->status_line, "\n";
    return 1;
  }

  # Get the main page again - it should contain the group listing.
  myprint "Getting the host groups...";
  if (!$res->is_success) {
    myprint " failed: " . $res->status_line . "\n";
    return 1;
  }

  myprint " done.\n";
  #print $res->content;

  # We have to search for a pattern like this:
  # <a href="ATSEv2/Scoreboard.jsp?TYPE=ATSEv2&amp;GROUP=FareDisplay">FareDisplay<span class="groupInstances">[2]</span></a>
  # or
  # <a href="ATSEv2-200/Scoreboard.jsp?TYPE=ATSEv2-200&amp;GROUP=C-ShoppingIS200">C-ShoppingIS200</a>

  # The @groups list has a corresponding @urls list containg URLs for the corresponding group from @groups.
  my %urls;
  my $remains = $res->content;
  while ( $remains =~ /<a href=\"ATSEv2((-200)?)\/Scoreboard.jsp\?TYPE=ATSEv2\1\&(amp;)?GROUP=(.*)\">\4(<span class=\"groupInstances\"( title=\"Number of Instances\")?>\[[0-9]+\]<\/span>)?<\/a>/ )
  {
    my $link = $&;
    my $name = $4;
    $remains = $';
    
    if ($name =~ /.*[sS]witch$/ ) {
      next;
    }

    $link =~ /\"([^\"]*)\"/;
    $urls{$name} = "/" . sanitize ($1);
#    print "found group ", $name, "\n";
  }

  # Now that we have downloaded the list of groups, we can try to create a function variant:
  print ACDATA "if (\$env eq '", lc $env, "') {\n    ";
  for (keys %urls) {
    my $group = $_;
    my $acgroup = lc $group;
    if ($env eq "PROD" ) {
      if ($group =~ /^Shopping/i) {
        $acgroup = lc "$group-a";
      } elsif ($group =~ /^(.)-Shopping(.*)200$/i) {
        $acgroup = lc "shopping$2-$1";
      }
    }
    if ($acgroup =~ /service/i ) {
      $acgroup = $` . $';
    }
    
    my @hosts;
    myprint "$group: URL " . $urls{$group} . ", ";
    print ACDATA "if (lc \$group eq '$acgroup') {\n      return ( \@empty";
    
    $req =  HTTP::Request->new(GET => $base . $urls{$group});
    $res = $ua->request($req);
    
    if (!$res->is_success) {
      myprint " failed: " . $res->status_line . "\n";
      return 1;
    }
    else {
      # Hosts which are active are printed like:
      # <a href="AppDetail.jsp?AID=AppConsole:type=ATSEv2,group=TaxService-IntA,host=piili001,port=5003">piili001</a>
      # <a href="AppDetail.jsp?AID=AppConsole:type=ATSEv2,group=Pricing,host=piclp164,port=5000">piclp164</a>
      $remains = $res->content;
#      print $remains;
      while ( $remains =~ /<a href=\"AppDetail.jsp\?AID=AppConsole:type=ATSEv2(-200)?,group=$group,host=(\w+),port=\d+\">\2<\/a>/ )
      {
        my $host = lc $2;
        $remains = $';

        if ( !defined $hostEnvs{$host} ) {
           $hostEnvs{$host} = $env;
           push @hosts, $host;
        } elsif ( $hostEnvs{$host} eq $env ) {
           push @hosts, $host;
        } else {
           myprint "Host $host found in $env and in $hostEnvs{$host}!\n";
        }
      }

      # Hosts which are not active are printed like:
      # <td nowrap="nowrap" align="left">picli405</td>
      $remains = $res->content;
      while ( $remains =~ /<td nowrap="nowrap" align="left">(\w+)<\/td>/ )
      {
        my $host = lc $1;
        $remains = $';

        if ( !defined $hostEnvs{$host} ) {
           $hostEnvs{$host} = $env;
           push @hosts, $host;
        } elsif ( $hostEnvs{$host} eq $env ) {
           push @hosts, $host;
        } else {
           myprint "Host $host found in $env and in $hostEnvs{$host}!\n";
        }
      }
    }
    
    for (@hosts) {
      print ACDATA ", '$_'";
    }
    
    if (scalar @hosts == 0) { print "WARNING: "; }

    myprint ( scalar @hosts . " hosts found.\n");
#    if (scalar @hosts == 0) { print $res->content; }

    print ACDATA " );\n    } els";
  }
  if ($env eq "CERT") {
    print ACDATA "if (lc \$group eq 'shopping') {\n      return ( hostsInGroup ( 'CERT', \"shoppingis\" ), hostsInGroup ( 'CERT', \"shoppingmip\" ));\n    } els";
  } elsif ($env eq "PROD") {
    print ACDATA "if (lc \$group eq 'shopping') {\n      return ( hostsInGroup ( 'PROD', \"shoppingis\" ), hostsInGroup ( 'PROD', \"shoppingmip\" ));\n    } els";
    print ACDATA "if (lc \$group eq 'shopping-a') {\n      return ( hostsInGroup ( 'PROD', \"shoppingis-a\" ), hostsInGroup ( 'PROD', \"shoppingmip-a\" ));\n    } els";
    print ACDATA "if (lc \$group eq 'shopping-c') {\n      return ( hostsInGroup ( 'PROD', \"shoppingis-c\" ), hostsInGroup ( 'PROD', \"shoppingmip-c\" ));\n    } els";
    print ACDATA "if (lc \$group eq 'shopping-d') {\n      return ( hostsInGroup ( 'PROD', \"shoppingis-d\" ), hostsInGroup ( 'PROD', \"shoppingmip-d\" ));\n    } els";
    print ACDATA "if (lc \$group eq 'shopping-e') {\n      return ( hostsInGroup ( 'PROD', \"shoppingis-e\" ), hostsInGroup ( 'PROD', \"shoppingmip-e\" ));\n    } els";
    print ACDATA "if (lc \$group eq 'shopping-g') {\n      return ( hostsInGroup ( 'PROD', \"shoppingis-g\" ), hostsInGroup ( 'PROD', \"shoppingmip-g\" ));\n    } els";
    print ACDATA "if (lc \$group eq 'shopping-h') {\n      return ( hostsInGroup ( 'PROD', \"shoppingis-h\" ), hostsInGroup ( 'PROD', \"shoppingmip-h\" ));\n    } els";
    print ACDATA "if (lc \$group eq 'shoppingis') {\n      return ( hostsInGroup ( 'PROD', \"shoppingis-a\" ), hostsInGroup ( 'PROD', \"shoppingis-c\" ), hostsInGroup ( 'PROD', \"shoppingis-d\" ), hostsInGroup ( 'PROD', \"shoppingis-e\" ), hostsInGroup ( 'PROD', \"shoppingis-g\" ), hostsInGroup ( 'PROD', \"shoppingis-h\" ));\n    } els";
    print ACDATA "if (lc \$group eq 'shoppingmip') {\n      return ( hostsInGroup ( 'PROD', \"shoppingmip-a\" ), hostsInGroup ( 'PROD', \"shoppingmip-c\" ), hostsInGroup ( 'PROD', \"shoppingmip-d\" ), hostsInGroup ( 'PROD', \"shoppingmip-e\" ), hostsInGroup ( 'PROD', \"shoppingmip-g\" ), hostsInGroup ( 'PROD', \"shoppingmip-h\" ));\n    } els";
    print ACDATA "if (lc \$group eq 'shoppingesv') {\n      return ( hostsInGroup ( 'PROD', \"shoppingesv-a\" ));\n    } els";
    print ACDATA "if (lc \$group eq 'shoppinghist') {\n      return ( hostsInGroup ( 'PROD', \"shoppinghist-a\" ));\n    } els";
  } elsif ($env eq "INT") {
    print ACDATA "if (lc \$group eq 'pricing') {\n      return ( hostsInGroup ( 'INT', \"pricing-daily\" ), hostsInGroup ( 'INT', \"pricing-sandbox\" ), hostsInGroup ( 'INT', \"pricing-inta\" ), hostsInGroup ( 'INT', \"pricing-intb\" ));\n    } els";
    print ACDATA "if (lc \$group eq 'faredisplay') {\n      return ( hostsInGroup ( 'INT', \"faredisplay-daily\" ), hostsInGroup ( 'INT', \"faredisplay-sandbox\" ), hostsInGroup ( 'INT', \"faredisplay-inta\" ), hostsInGroup ( 'INT', \"faredisplay-intb\" ));\n    } els";
    print ACDATA "if (lc \$group eq 'historical') {\n      return ( hostsInGroup ( 'INT', \"historical-daily\" ), hostsInGroup ( 'INT', \"historical-sandbox\" ), hostsInGroup ( 'INT', \"historical-inta\" ), hostsInGroup ( 'INT', \"historical-intb\" ));\n    } els";
    print ACDATA "if (lc \$group eq 'shopping') {\n      return ( hostsInGroup ( 'INT', \"shopping-daily\" ), hostsInGroup ( 'INT', \"shopping-sandbox\" ), hostsInGroup ( 'INT', \"shopping-inta\" ), hostsInGroup ( 'INT', \"shopping-intb\" ), hostsInGroup ( 'INT', \"shoppinghist-daily\" ), hostsInGroup ( 'INT', \"shoppingesv\" ), hostsInGroup ( 'INT', \"shopping-cpt\" ));\n    } els";
    print ACDATA "if (lc \$group eq 'shopping-daily') {\n      return ( hostsInGroup ( 'INT', \"shoppingis-daily\" ), hostsInGroup ( 'INT', \"shoppingmip-daily\" ));\n    } els";
    print ACDATA "if (lc \$group eq 'shopping-inta') {\n      return ( hostsInGroup ( 'INT', \"shoppingis-inta\" ), hostsInGroup ( 'INT', \"shoppingmip-inta\" ));\n    } els";
    print ACDATA "if (lc \$group eq 'shopping-intb') {\n      return ( hostsInGroup ( 'INT', \"shoppingis-intb\" ), hostsInGroup ( 'INT', \"shoppingmip-intb\" ));\n    } els";
    print ACDATA "if (lc \$group eq 'shoppingis') {\n      return ( hostsInGroup ( 'INT', \"shoppingis-daily\" ), hostsInGroup ( 'INT', \"shoppingis-inta\" ), hostsInGroup ( 'INT', \"shoppingis-intb\" ));\n    } els";
    print ACDATA "if (lc \$group eq 'shoppingmip') {\n      return ( hostsInGroup ( 'INT', \"shoppingmip-daily\" ), hostsInGroup ( 'INT', \"shoppingmip-inta\" ), hostsInGroup ( 'INT', \"shoppingmip-intb\" ));\n    } els";
    print ACDATA "if (lc \$group eq 'shoppinghist') {\n      return ( hostsInGroup ( 'INT', \"shoppinghist-daily\" ), hostsInGroup ( 'INT', \"shoppinghist-inta\" ), hostsInGroup ( 'INT', \"shoppinghist-intb\" ));\n    } els";
    print ACDATA "if (lc \$group eq 'shoppingesv') {\n      return ( hostsInGroup ( 'INT', \"shoppingesv-daily\" ), hostsInGroup ( 'INT', \"shoppingesv-inta\" ), hostsInGroup ( 'INT', \"shoppingesv-intb\" ));\n    } els";
    print ACDATA "if (lc \$group eq 'tax') {\n      return ( hostsInGroup ( 'INT', \"tax-daily\" ), hostsInGroup ( 'INT', \"tax-sandbox\" ), hostsInGroup ( 'INT', \"tax-inta\" ), hostsInGroup ( 'INT', \"tax-intb\" ));\n    } els";
    print ACDATA "if (lc \$group eq 'inta') {\n      return ( hostsInGroup ( 'INT', \"pricing-inta\" ), hostsInGroup ( 'INT', \"historical-inta\" ), hostsInGroup ( 'INT', \"shoppingis-inta\" ), hostsInGroup ( 'INT', \"shoppingmip-inta\" ));\n    } els";
    print ACDATA "if (lc \$group eq 'intb') {\n      return ( hostsInGroup ( 'INT', \"pricing-intb\" ), hostsInGroup ( 'INT', \"historical-intb\" ), hostsInGroup ( 'INT', \"shoppingis-intb\" ), hostsInGroup ( 'INT', \"shoppingmip-intb\" ));\n    } els";
    print ACDATA "if (lc \$group eq 'daily') {\n      return ( hostsInGroup ( 'INT', \"pricing-daily\" ), hostsInGroup ( 'INT', \"historical-daily\" ), hostsInGroup ( 'INT', \"shoppingis-daily\" ), hostsInGroup ( 'INT', \"shoppingmip-daily\" ), hostsInGroup ( 'INT', \"shoppinghist-daily\" ), hostsInGroup ( 'INT', \"shoppingesv-daily\" ));\n    } els";
  } else {
    die "Shouldn't happen - we should have faulted earlier.";
  }
  print ACDATA "e {\n      return ();\n    }\n  }\n\n  ";
  return 0;
}

sub printHostMap
{
  myprint "\nSaving the {host, env} map...";
  my $num = 0;
  print ACDATA "\nsub hostEnv\n{\n  my \$host=lc shift;\n  my %hostEnvs= (";
  for (keys %hostEnvs) {
    my $host = lc $_;
    if ($num) { print ACDATA ","; }
    if ($num % 5 == 0) { print ACDATA "\n   "; }
    print ACDATA " \"$host\" => \"$hostEnvs{$host}\"";
    $num++;
  }

  print ACDATA "  );\n";
  print ACDATA "  if (lc \$host eq 'all') {\n    my \$env=uc shift;\n    my \@envHosts;\n    for (keys %hostEnvs) {\n";
  print ACDATA "      if (\$hostEnvs{\$_} eq \$env) {\n";
  print ACDATA "        push \@envHosts, \$_;\n";
  print ACDATA "      }\n";
  print ACDATA "    }\n";
  print ACDATA "    return \@envHosts;\n";
  print ACDATA "  }\n";
  print ACDATA "  if (defined \$hostEnvs{\$host}) { return \$hostEnvs{\$host}; }\n";
  print ACDATA "  return \"UNKNOWN\";\n}\n";

  myprint " done.\n";
  return 0;
}

open ACDATA, "> " . $ENV{"HOME"} . "/bin/groupmembers.pl";
print ACDATA "#!/usr/bin/perl\n\n";
print ACDATA "use strict;\n";
print ACDATA "use warnings;\n";
print ACDATA "use Exporter;\n";
print ACDATA "our \$EXPORT_OK = 'hostsInGroup';\n\n";
print ACDATA "sub hostsInGroup\n{\n  my \$env=lc shift;\n  my \$group=lc shift;\n  my \@empty=();\n  ";

if (printEnv ('pRod', 'http://appcon.prod.ha.sabre.com/AppConsole') > 0) { die "Failed!\n"; }
if (printEnv ('INT',  'http://appcon.int.sabre.com/AppConsole') > 0) { die "Failed!\n"; }
if (printEnv ('cert', 'http://appcon.cert.ha.sabre.com/AppConsole') > 0) { die "Failed!\n"; }
print ACDATA "die \"Unknown environment: \$env\";\n}\n";

if (printHostMap() > 0) { die "Failed!\n"; }

close ACDATA;
