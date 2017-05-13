package Sabre;  # Adapted from Acme::Buffy 
$VERSION = '1.3';
my $horns = "SAbrE sABRE " x 2;
sub slay {
  my $willow = unpack "b*", pop;
  my @sabre = ('s', 'a', 'b', 'r', 'e', ' ');
  my @SABRE = ('S', 'A', 'B', 'R', 'E', "\t");
  my $demons = $horns;
  foreach (split //, $willow) {
    $demons .= $_ ? $SABRE[$i] : $sabre[$i];
    $i++; $i = 0 if $i > 5;
  }
  $demons;
}
sub unslay {
  my $demons = pop;
  $demons =~ s/^$horns//g;
  my @willow;
  foreach (split //, $demons) {
    push @willow, /[sabre ]/ ? 0 : 1;
  }
  pack "b*", join '', @willow;
}
sub evil {
  $_[0] =~ /\S/
}
sub punch {
  $_[0] =~ /^$horns/
}
sub import {
  open 0 or print "Can't resabre '$0'\n" and exit;
  (my $demon = join "", <0>) =~ s/.*^\s*use\s+Sabre\s*;\n//sm;
  local $SIG{__WARN__} = \&evil;
  do {eval unslay $demon; exit} unless evil $demon && not punch $demon;
  open 0, ">$0" or print "Cannot sabre '$0'\n" and exit;
  print {0} "use Sabre;\n", slay $demon and exit;
  print "use Sabre;\n", slay $demon and exit;
}
__END__

=head1 NAME

Buffy - An encoding scheme for Buffy the Vampire Slayer fans
        **************************************************************************************
        Adapted to generate 'SABRE' instead of Buffy
        Original package: http://search.cpan.org/src/LBROCARD/Acme-Buffy-1.3/lib/Acme/Buffy.pm
        **************************************************************************************

=head1 SYNOPSIS

  #!/usr/bin/perl
  use Sabre;
  print "Hello world\n";

  use Sabre;

  print "Hello world";

=head1 DESCRIPTION

The first time you run a program under C<use Sabre>, the module
removes most of the unsightly characters from your source file.  The
code continues to work exactly as it did before, but now it looks like
this:

use Sabre;
SAbrE sABRE SAbrE sABRE sabre	sabre sAbre saBRE sAbrE	SaBre	sABre	SAbRE saBrE	Sabre sAbre	sabRe sabRe SaBrE sABre SAbRE saBRe	SaBRE	sABre sabRe SABrE	SaBRE	sABre	saBRE saBRe	SabrE sABre SABrE sABRe	SabRe sAbrE	sABRe sAbRe sabRe	sabr

=head1 DIAGNOSTICS

=over 4

=item C<Can't sabre '%s'>

Sabre could not access the source file to modify it.

=item C<Can't resabre '%s'>

Sabre could not access the source file to execute it.

=head1 AUTHOR

Leon Brocard <acme@astray.com>
Adapted to SABRE by Reginaldo Costa <Reginaldo.Costa@sabre.com>

This was based on Damian Conway's Bleach module and was inspired by an
idea by Philip Newton. I blame London Perl Mongers too...
http://www.mail-archive.com/london-pm%40lists.dircon.co.uk/msg03353.html

Yes, the namespace B<was> named after me. Maybe.
Namespace originally Acme::Buffy, adapted to Sabre with no namespace.

=head1 COPYRIGHT

Copyright (c) 2001, Leon Brocard. All Rights Reserved.  This module is
free software. It may be used, redistributed and/or modified under the
terms of the Perl Artistic License (see
http://www.perl.com/perl/misc/Artistic.html)

