#! /usr/bin/perl
use strict vars;
use IO::Socket;

my ($hostlist, $port, $defaultlist, @tables);

while ($_ = shift @ARGV)
{
    if (uc $_ eq '--PRICING')
    {
        ($defaultlist, $port) = &pricing_hosts;
    }
    elsif (uc $_ eq '--FAREDISPLAY')
    {
        ($defaultlist, $port) = &faredisplay_hosts;
    }
    elsif (uc $_ eq '--TAX')
    {
        ($defaultlist, $port) = &tax_hosts;
    }
    elsif (uc $_ eq '--SHOPPING')
    {
        ($defaultlist, $port) = &shopping_hosts;
    }
    elsif ($_ eq '-h' || $_ eq '--host')
    {
        my $host = shift @ARGV;
        print "host = $host \n";
        $hostlist = ();
        push @$hostlist, $host;
    }
    elsif ($_ eq '-p' || $_ eq '--port')
    {
        $port = shift @ARGV;
        print "port = $port \n";
    }
    elsif (/-.*/)
    {
        print STDERR "unknown option ", $_, "\n";
    }
    else
    {
        push @tables, $_;
    }
}

if (!$hostlist) { $hostlist = $defaultlist; }

if (!$hostlist || !$port) { die "usage flush.pl {-h host -p port} | {--pricing | --faredisplay | --tax | --shopping} [table...]" };

if (!@tables)
{
    print "flushing all tables\n" ;
    @tables = ( 'ADDONCOMBFARECLASSHIST', 'ADDONHIST',
                'ADDONMARKETCARRIERSHIST', 'ADDONZONEHIST', 'ADDONZONEINFOHIST', 'ADDONZONESITAHIST',
                'AFTERTKTFAREHIST', 'BANKERSELLRATEHIST', 'BEFORETKTFAREHIST', 'BOOKINGCODECONVHIST',
                'COMBINABILITYRULEHIST', 'FAREBYRULEAPPHIST', 'FAREBYRULEAPPRULETARIFFHIST',
                'FAREBYRULECTRLHIST', 'FARECLASSAPPHIST', 'FAREHIST', FOOTNOTECTRLHIST',
                'GENERALFARERULEHIST', 'MARKETCARRIERSHIST', 'MARKUPBYSECURITYITEMNOHIST',
                'MARKUPBYSECONDSELLERIDHIST', 'MARKUPCONTROLHIST', 'MINFAREAPPLHIST',
                'MINFAREDEFAULTLOGICHIST', 'MINFARERULELEVELEXCHIST', 'NUCHIST', 'ROUTINGFORMARKETHIST',
                'ROUTINGHIST', 'ZONEHIST'
            ); }

for (@$hostlist)
{
    &send_flush($_, $port, @tables);
}

exit;

sub send_flush
{
    my $host = shift;
    my $port = shift;
    my @tables = @_;
    
    my $headersize = pack('N',16);

    foreach (@tables) 
    {
        my $payload = uc $_;
        my $payloadsize = pack('N', length($payload));
        my $command = $headersize . $payloadsize . 'FLSH' . '0001' . '0000' . $payload;

        my $sock = new IO::Socket::INET(PeerAddr => $host, PeerPort => $port, Proto => 'tcp' )
                   or (print STDERR "Could not connect to $host:$port\n" and return);
        print "flushing $payload on $host:$port\n";
        print $sock $command;
        close $sock;
        sleep 1;
    }
}

sub faredisplay_hosts
{
    print "processing fare display hosts\n";
    my @hosts = (
        'piclp002',
        'piclp003',
        'piclp006',
        'piclp007',
        'piclp008',
        'piclp143',
        'piclp144',
        'piclp168',
        'piclp219',
        'piclp220',
        'piclp221',
        'piclp222',
        'piclp223',
        'piclp224',
        'piclp225',
        'piclp226',
        'piclp260',
		'pimlp038',
		'pimlp039',
		'pimlp040',
		'pimlp041',
		'pimlp042'
    );
    return \@hosts, 5002;
}
sub pricing_hosts
{
    print "processing pricing hosts\n";
    my @hosts = (
        'piclp004',
        'piclp005',
        'piclp009',
        'piclp010',
        'piclp011',
        'piclp012',
        'piclp013',
        'piclp145',
        'piclp152',
        'piclp161',
        'piclp162',
        'piclp163',
        'piclp164',
        'piclp165',
        'piclp166',
        'piclp167',
        'piclp215',
        'piclp216',
        'piclp217',
        'piclp254',
        'piclp255',
        'piclp256',
        'piclp257',
        'piclp258',
        'piclp259',
        'pimlp001',
        'pimlp002',
        'pimlp003',
        'pimlp004',
        'pimlp005',
        'pimlp006',
        'pimlp007',
	'pimlp043',
	'pimlp044',
	'pimlp045',
	'pimlp046',
	'pimlp047',
	'pimlp048',
	'pimlp049'
    );
    return \@hosts, 5000;
}
sub shopping_hosts
{
    print "processing shopping hosts\n";
    my @hosts = (
        'piclp014',
        'piclp015',
        'piclp016',
        'piclp017',
        'piclp018',
        'piclp146',
        'piclp147',
        'piclp148',
        'piclp149',
        'piclp150',
        'piclp151',
        'piclp153',
        'piclp154',
        'piclp155',
        'piclp156',
        'piclp157',
        'piclp158',
        'piclp159',
        'piclp160',
        'piclp169',
        'piclp170',
        'piclp171',
        'piclp172',
        'piclp173',
        'piclp174',
        'piclp175',
        'piclp176',
        'piclp177',
        'piclp178',
        'piclp179',
        'piclp180',
        'piclp181',
        'piclp182',
        'piclp183',
        'piclp184',
        'piclp185',
        'piclp186',
        'piclp187',
        'piclp188',
        'piclp189',
        'piclp190',
        'piclp191',
        'piclp192',
        'piclp193',
        'piclp194',
        'piclp195',
        'piclp196',
        'piclp197',
        'piclp198',
        'piclp199',
        'piclp200',
        'piclp201',
        'piclp202',
        'piclp203',
        'piclp204',
        'piclp205',
        'piclp206',
        'piclp207',
        'piclp208',
        'piclp209',
        'piclp210',
        'piclp211',
        'piclp212',
        'piclp213',
        'piclp214',
        'piclp218',
        'piclp227',
        'piclp228',
        'piclp229',
        'piclp230',
        'piclp231',
        'piclp236',
        'piclp237',
        'piclp238',
        'piclp239',
        'piclp240',
        'piclp241',
        'piclp242',
        'piclp243',
        'piclp244',
        'piclp245',
        'piclp246',
        'piclp247',
        'piclp248',
        'piclp249',
        'piclp250',
        'piclp251',
        'piclp252',
        'piclp253',
        'piclp261',
        'piclp262',
        'piclp263',
        'piclp264',
        'piclp265',
        'piclp266',
        'piclp267',
        'piclp268',
        'piclp269',
        'piclp270',
        'piclp271',
        'piclp272',
        'piclp273',
        'piclp274',
        'piclp275',
        'piclp276',
        'piclp277',
        'piclp278',
        'piclp279',
        'piclp280',
        'piclp281',
        'piclp282',
        'piclp283',
        'piclp284',
        'piclp285',
        'piclp286',
        'piclp287',
        'piclp288',
        'piclp289',
        'piclp290',
        'piclp291',
        'piclp292',
        'piclp293',
        'piclp294',
        'piclp295',
        'piclp296',
        'piclp297',
        'piclp298',
        'piclp299',
        'piclp303',
        'piclp322',
        'piclp326',
        'piclp327',
        'piclp328',
        'piclp329',
        'piclp330',
        'piclp331',
        'piclp405',
        'piclp406',
        'piclp407',
        'piclp408',
        'piclp409',
        'piclp410',
        'piclp411',
        'piclp412',
        'piclp413',
        'piclp414',
        'piclp415',
        'pimlp008',
        'pimlp009',
        'pimlp010',
        'pimlp011',
        'pimlp012',
        'pimlp013',
        'pimlp014',
        'pimlp015',
        'pimlp016',
        'pimlp017',
        'pimlp018',
        'pimlp019',
        'pimlp020',
        'pimlp021',
        'pimlp022',
        'pimlp023',
        'pimlp024',
        'pimlp025',
        'pimlp026',
        'pimlp027',
        'pimlp028',
        'pimlp029',
        'pimlp030',
        'pimlp031',
        'pimlp032',
        'pimlp033',
        'pimlp034',
        'pimlp035',
        'pimlp036',
        'pimlp037',
        'pimlp052',
        'pimlp053',
        'pimlp054',
        'pimlp055',
        'pimlp056',
        'pimlp057',
        'pimlp058',
        'pimlp059',
        'pimlp060',
        'pimlp061',
        'pimlp062'
    );
    return \@hosts, 5001;
}
sub tax_hosts
{
    print "processing tax hosts\n";
    my @hosts = (
        'piclp002',
        'piclp003',
        'piclp006',
        'piclp007',
        'piclp008',
        'piclp143',
        'piclp144',
        'piclp168',
        'piclp219',
        'piclp220',
        'piclp221',
        'piclp222',
        'piclp223',
        'piclp224',
        'piclp225',
        'piclp226',
        'piclp260',
	'pimlp038',
	'pimlp039',
	'pimlp040',
	'pimlp041',
	'pimlp042'
    );
    return \@hosts, 5003;
}
