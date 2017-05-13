#------------------------------------------------------------------------------
#
#  File:    BIGIP.pm (PERL Class)
#  Author:  Reginaldo Costa
#  Created: 08/07/2006
#  Description:
#           This package is holds the BIGIP class which is used to interface
#           with BIGIP boxes by operational scripts.
#
#           Refer to atsecpp/install/perlpm/BIGIP.test.pl for
#           example of usage.
#
#  Copyright (c) SABRE 2006
#
#  The copyright to the computer program(s) herein is the property of SABRE.
#  The program(s) may be used and/or copied only with the written permission
#  of SABRE or in accordance with the terms and conditions stipulated in the
#  agreement/contract under which the program(s) have been supplied.
#
#------------------------------------------------------------------------------

#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
BEGIN { $SIG{'__WARN__'} = sub { warn $_[0] if substr($_[0],0,10) ne "use_prefix" } }
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

package BIGIP;
use     SOAP::Lite; # For iControl interface

use constant FALSE    => 0;
use constant TRUE     => 1;
use constant DISABLED => 0;
use constant ENABLED  => 1;
use constant DOWN     => 1;
use constant UP       => 2;

# Constants used for monitors
use constant ITYPE_UNSET    => 0;
use constant ITYPE_INTERVAL => 1;
use constant ITYPE_TIMEOUT  => 2;
use constant TTYPE_TCP      => 2;
use constant ATYPE_UNSET                          => 0;   #  The address type is unknown.
use constant ATYPE_STAR_ADDRESS_STAR_PORT         => 1;   #  For example, "*:*".
use constant ATYPE_STAR_ADDRESS_EXPLICIT_PORT     => 2;   #  For example, "*:80".
use constant ATYPE_EXPLICIT_ADDRESS_EXPLICIT_PORT => 3;   #  For example, "10.10.10.1:80".
use constant ATYPE_STAR_ADDRESS                   => 4;   #  For example, "*:*".
use constant ATYPE_EXPLICIT_ADDRESS               => 5;   #  For example, "10.10.10.1".

my %RGX = ( 'XIPADDR'   => '\b(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\b'
          , 'XIPADDR1'  =>  '^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$'
          , 'XIPPORT'   => '\b(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\:(\d{1,5})\b'
          , 'XIPPORT1'  =>  '^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\:(\d{1,5})$'
          , 'XSERVICE'  =>  '^(\d{1,5})$'
          );

my $NODEFMT = $RGX{XIPPORT1};
my $SERVFMT = $RGX{XSERVICE};

my %LBMethodMap =
    ( "default"                  => 0
    , "Default"                  => 0
    , "DEFAULT"                  => 0
    , "Round Robin"              => 0
    , "RoundRobin"               => 0
    , "ROUND ROBIN"              => 0
    , "ROUNDROBIN"               => 0
    , "0"                        => 0
    , "1"                        => 1
    , "2"                        => 2
    , "3"                        => 3
    , "4"                        => 4
    , "5"                        => 5
    , "Ratio"                    => 1
    , "Fastest"                  => 2
    , "Least Connections"        => 3
    , "LeastConnections"         => 3
    , "Ratio Member"             => 4
    , "RatioMember"              => 4
    , "Least Connections Member" => 5
    , "LeastConnections Member"  => 5
    , "LeastConnectionsMember"   => 5
    , "RATIO"                    => 1
    , "FASTEST"                  => 2
    , "LEAST CONNECTIONS"        => 3
    , "LEASTCONNECTIONS"         => 3
    , "RATIO MEMBER"             => 4
    , "RATIOMEMBER"              => 4
    , "LEAST CONNECTIONS MEMBER" => 5
    , "LEASTCONNECTIONS MEMBER"  => 5
    , "LEASTCONNECTIONSMEMBER"   => 5
    );

# Map for Virtual Server Status
my %StatusMap =
   (  '0' => 'UNCHECKED'         # Status of an enabled node that is not being monitored
   ,  '1' => 'UP'                # Status of an enabled node when its monitors succeed
   ,  '2' => 'DOWN'              # Status of an enabled node when its monitors fail.
   ,  '3' => 'FORCED_DOWN'       # Status of a node when it's forced down manually.
   ,  '4' => 'CHECKING'          # Initial status of a node until its monitors report.
   ,  '5' => 'MAINT'             # Status of an object when in maintenance mode.
   ,  '6' => 'ENABLED'           # Status of an object and all of its dependent objects when enabled.
   ,  '7' => 'DISABLED'          # Status of an object when disabled.
   ,  '8' => 'ADDRESS_DISABLED'  # Status of an object that is enabled, but whose address is disabled.
   ,  '9' => 'PORT_DISABLED'     # Status of an object that is enabled, but whose port is disabled.
   , '10' => 'ADDRESS_DOWN'      # Status of an object that is enabled, but its address is down.
   );

# Map for Node Availability Status
my %NodeAvailMap =
   (  '0' => 'AVAILABILITY_UNCHECKED'         # The node has not been checked for availability.
   ,  '1' => 'AVAILABILITY_DOWN'              # The node is not available.
   ,  '2' => 'AVAILABILITY_UP'                # The node is available.
   ,  '3' => 'AVAILABILITY_CHECKING'          # The node is currently being checked for availability.
   ,  '4' => 'AVAILABILITY_FORCED_DOWN'       # The node has been forced down manually.
   ,  '5' => 'AVAILABILITY_ADDR_DOWN'         # The node address for a node server is down.
   ,  '6' => 'AVAILABILITY_UNKNOWN'           # An unknown, undecipherable state has been received for the node.
   ,  '7' => 'AVAILABILITY_MAINT'             # Maintenance mode.
   ,  '8' => 'AVAILABILITY_ENABLED'           # The node is enabled.
   ,  '9' => 'AVAILABILITY_DISABLED'          # The node is disabled.
   , '10' => 'AVAILABILITY_ADDR_DISABLED'     # The node address is disabled.
   , '11' => 'AVAILABILITY_PORT_DISABLED'     # The node service is disabled.
   );


# Map for Monitor Types
my %TemplateType =
   (  '0' => 'UNSET'             # The template type is unknown
   ,  '1' => 'ICMP'              # The ICMP template type
   ,  '2' => 'TCP'               #
   ,  '3' => 'TCP_ECHO'          #
   ,  '4' => 'EXTERNAL'          #
   ,  '5' => 'HTTP'              #
   ,  '6' => 'HTTPS'             #
   ,  '7' => 'NNTP'              #
   ,  '8' => 'FTP'               #
   ,  '9' => 'POP3'              #
   , '10' => 'SMTP'              #
   , '11' => 'SQL'               #
   , '12' => 'GATEWAY'           #
   , '13' => 'IMAP'              #
   , '14' => 'RADIUS'            #
   , '15' => 'LDAP'              #
   , '16' => 'WMI'               #
   , '17' => 'SNMP_DCA'          #
   , '18' => 'SNMP_DCA_BASE'     #
   , '19' => 'REAL_SERVER'       #
   , '20' => 'UDP'               #
   );


#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#     S U B R O U T I N E S
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#===============================================================================
sub secondMath {

   (my $seconds, my $format) = @_;
   $seconds = 0 unless $seconds;
   $format = "%04d/%02d/%02d %02d:%02d:02d %s" unless $format;

   ( my $sec
   , my $min
   , my $hour
   , my $mday
   , my $mon
   , my $year
   , my $wday
   , my $yday
   , my $isdst
   ) = localtime(time + ($seconds));

   (my $yy, my $mm, my $dd, my $hh, my $mi, my $ss) = split /-/,
    sprintf( "%04d-%02d-%02d-%02d-%02d-%02d"
           , ($year + 1900)
           , ($mon + 1)
           , $mday
           , $hour
           , $min
           , $sec
           );

   my $ampm = '';
   if ($format =~ /\%s$/)
   {
       if ($hh > 11)
       {
           $ampm = "PM";
           $hh -= 12;
       }
       else
       {
           $ampm = "AM";
       };
   };
   return sprintf($format, $yy, $mm, $dd, $hh, $mi, $ss, $ampm);
};

sub dayMath {
   (my $days, my $format) = @_;
   $days = 0 unless $days;
   my $seconds = $days * 24*60*60;
   $format = "%04d/%02d/%02d" unless $format;
   return secondMath($seconds, $format);
};

sub now {
   (my $format) = @_;
   $format = "%04d-%02d-%02d %02d:%02d:%02d" unless $format;
   return dayMath(0,$format);
};

sub printm {
  (my $fmt, my @fields) = @_;
  printf STDERR "%s " . $fmt, now(), @fields;
  return 1;
};

sub prints {
  (my $fmt, my @fields) = @_;
  printf STDOUT "%s " . $fmt, now(), @fields;
  return 1;
};

sub printLong2IPAddr()
{
        my ($ipaddr) = @_;

        my $a = ($ipaddr >> 24) & 0x000000FF;
        my $b = ($ipaddr >> 16) & 0x000000FF;
        my $c = ($ipaddr >> 8 ) & 0x000000FF;
        my $d = ($ipaddr      ) & 0x000000FF;

        print "$a.$b.$c.$d";
};

sub getIPAddr($) {
    use Socket;
    my $name = shift;
    my $XIPADDR = $RGX{'XIPADDR'};

    if ($name =~ /$XIPADDR/)
    {
        return "$1.$2.$3.$4";
    }
    else
    {
        my $iaddr  = gethostbyname($name) or return "";
        return inet_ntoa($iaddr);
    };
};

sub fixString($)
{
        my ($old_string) = (@_);
        my $new_string = $old_string;

        $_ = $old_string;
        s/&quot;/'/g;
        s/&lt;/</g;
        s/&gt;/>/g;

        $new_string = $_;

        return $new_string;
}

sub fixName($)
{
        my ($old_string) = (@_);
        my $new_string = $old_string;

        $_ = $old_string;
        s/\./\_/g;

        $new_string = $_;

        return $new_string;
}

#===============================================================================
#     C O N S T R U C T O R
#===============================================================================

sub new {

# Example of BIGIP entry:
#    my $BigIP =  new BIGIP('redball.dev.sabre.com');
#    my $BigIP = new BIGIP('snpfunc:abc123@redball.dev.sabre.com');
#    my $BigIP = new BIGIP('snpfunc:abc123@redball.dev.sabre.com','SSH');
#    $BigIP->retries(5);   # set retries to 5
#    if ( $special_user ) { $BigIP->user($special_user); $BigIP->password('ABC123'); };
#

    my $type = shift;
    my $class = ref( $type ) || $type;


   (my $entry, my $interface) = @_;
   $interface = 'iControl' unless $interface;

   (my $left,my $right)      = split /@/, $entry;
   $right = $left   unless $right;
   $left = ''       if $left eq $right;
   (my $user, my $password) = split /:/, $left;
   (my $host, my $port)     = split /:/, $right;

   my $ipaddr = getIPAddr($host);
   if ($ipaddr eq '') {
      die "BIGIP.pm - Host not found: " . $host . "\n";
   };

   $port = 443 unless ($port);

   my $hostdir = $ENV{HOME} . "/.bigip/hosts" ;
   if( exists $ENV{BIGIP_USER_HOSTS} && $ENV{BIGIP_USER_HOSTS} ne "" ) {
     $hostdir = $ENV{BIGIP_USER_HOSTS} ;
   }

   my $hostfile = ${hostdir} . "/" . $host ;
   if (-f $hostfile) {
      $left = `perl $hostfile`;
      ($user, $password) = split /:/, $left unless $user;  # Let user@host take precedence
   };

   my $iURL = sprintf("https://%s:%s@%s:%s/iControl/iControlPortal.cgi", $user, $password, $host, $port);
   my  $URL = sprintf("%s", $host);

   my @errorcodes = qw();
   my @responses  = qw();

   my $this =  {  user          => $user        # BIG-IP user code
               ,  password      => $password    # BIG-IP user password
               ,  host          => $host        # BIG-IP Host name
               ,  port          => $port        # BIG-IP port
               ,  interface     => $interface   # Interface being used
               ,  retries       => 3            # Number of SSH retries
               ,  ipaddr        => $ipaddr      # BIG-IP ip address
               ,  SSH           => 'SSH'        # Valid Interface holds its name
               ,  iControl      => 'iControl'   # Interfaces will hold its name
               ,  errorcount    => 0            # Error counter
               ,  errorlimit    => 5            # No print STDERR after limit
               ,  errorcode     => 0            # Last error code
               ,  errorcodes    => \@errorcodes # Error code history
               ,  responses     => \@responses  # Response stack
               ,  preview       => TRUE         # Used by SSH to not submit cmds
               ,  debug         => FALSE        # Activate DEBUG code TRUE/FALSE
               ,  verbose       => FALSE        # Verbose messages if any
               ,  stoponerror   => TRUE         # Stop processing on error
               ,  stoponlimit   => TRUE         # Stop processing on errorlimit
               ,  URL           => $URL         # BIG-IP URL
               ,  iCTLGlobal    => SOAP::Lite
                                -> uri('urn:iControl:ITCMLocalLB/Global')
                                -> readable(1)
                                -> proxy("$iURL")
               ,  iCTLPool      => SOAP::Lite
                                -> uri('urn:iControl:ITCMLocalLB/Pool')
                                -> readable(1)
                                -> proxy("$iURL")
               ,  iCTLNode      => SOAP::Lite
                                -> uri('urn:iControl:ITCMLocalLB/Node')
                                -> readable(1)
                                -> proxy("$iURL")
               ,  iCTLMonitor   => SOAP::Lite
                                -> uri('urn:iControl:ITCMLocalLB/Monitor')
                                -> readable(1)
                                -> proxy("$iURL")
               ,  iCTLMonitor2  => SOAP::Lite
                                -> uri('urn:iControl:ITCMLocalLB/Monitor2')
                                -> readable(1)
                                -> proxy("$iURL")
               ,  iCTLVirtual   => SOAP::Lite
                                -> uri('urn:iControl:ITCMLocalLB/VirtualServer')
                                -> readable(1)
                                -> proxy("$iURL")
               ,  iCTLSysinfo   => SOAP::Lite
                                -> uri('urn:iControl:ITCMSystem/SystemInfo')
                                -> proxy("$iURL")
               ,  iCTLBigdb     => SOAP::Lite
                                -> uri('urn:iControl:ITCMManagement/Bigdb')
                                -> proxy("$iURL")
               ,  defaultDB     => '/config/user.db'

               };

   die "BIGIP.pm - Invalid Interface: " . $this->{"interface"} . "\n"
   unless (exists($this->{$interface}) && $this->{$interface} eq $interface);

   bless $this, $class;
   $this->peeripaddr($this->getPeerIPAddress()); # Peer BIG-IP ip address
   my $version = $this->getVersion();  # Get BIG IP Software version
   die "This package only supports BIG IP version 4: $version\n"
   unless substr($version,0,1) eq '4';


   return $this;
};

#-----------------Utilities-----------------------------------------------------

sub statusName {    # Returns Mnemonic Status from status code
    my $self = shift;
    my ($status_code) = @_;

    return $StatusMap{"$status_code"} ;
};


#-------------------GET/SET'ers-------------------------------------------------

sub defaultDB {
    my $self = shift;
    if (@_)  { $self->{"defaultDB"} = shift; } else { $self->{"defaultDB"}; };
}

sub peeripaddr {
    my $self = shift;
    if (@_)  { $self->{"peeripaddr"} = shift; } else { $self->{"peeripaddr"}; };
};

sub debug {
    my $self = shift;
    if (@_)  { $self->{"debug"} = shift; } else { $self->{"debug"}; };
};

sub stoponerror {
    my $self = shift;
    if (@_)  { $self->{"stoponerror"} = shift; } else { $self->{"stoponerror"}; };
};

sub stoponlimit {
    my $self = shift;
    if (@_)  { $self->{"stoponlimit"} = shift; } else { $self->{"stoponlimit"}; };
};

sub errorcodes {
    my $self = shift;
    if (@_)  { $self->{"errorcodes"} = shift; } else { $self->{"errorcodes"}; };
};

sub errorlimit {
    my $self = shift;
    if (@_)  { $self->{"errorlimit"} = shift; } else { $self->{"errorlimit"}; };
};

sub errorcount {
    my $self = shift;
    if (@_)  { $self->{"errorcount"} += shift; } else { $self->{"errorcount"}; };
};

sub verbose {
    my $self = shift;
    if (@_)  { $self->{"verbose"} = shift; } else { $self->{"verbose"}; };
};

sub errorcode {
    my $self = shift;
    if (@_)  { $self->{"errorcode"} = shift; } else { $self->{"errorcode"}; };
};

#===============================================================================
#===============================================================================
#    M E T H O D S   U S E D   F O R   S S H   I N T E R F A C E
#===============================================================================
#===============================================================================

sub user {
    my $self = shift;
    if ((@_) and ($self->{$interface} eq 'SSH'))
    {
        $self->{"user"} = shift;
    }
    else
    {
        print STDERR "BIGIP::user can only be set for SSH\n" if (@_);
        $self->{"user"};
    };
};

sub password {
    my $self = shift;
    if ((@_) and ($self->{$interface} eq 'SSH'))
    {
        $self->{"password"} = shift;
    }
    else
    {
        print STDERR "BIGIP::password can only be set for SSH\n" if (@_);
        $self->{"password"};
    };
};

sub host {
    my $self = shift;
    if ((@_) and ($self->{$interface} eq 'SSH'))
    {
        my $host = shift;
        my $ipaddr = getIPAddr($host);
        if ($ipaddr eq '') {
           die "BIGIP.pm - Host not found: " . $host . "\n";
        };
        $self->{"host"} = $host;
        $self->{"ipaddr"} = $ipaddr;
    }
    else
    {
        print STDERR "BIGIP::host can only be set for SSH\n" if (@_);
        $self->{"host"};
    };
};

sub ipaddr {
    my $self = shift;
    if ((@_) and ($self->{$interface} eq 'SSH'))
    {
        my $ipaddr = shift;
        my $XIPADDR = '\b(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\b';
        if ($ipaddr =~ /$XIPADDR/)
        {
            $ipaddr = "$1.$2.$3.$4";
        }
        else
        {
            die "BIGIP.pm - Invalid IP address: " . $ipaddr . "\n";
        };
        $self->{"host"} = $ipaddr;
        $self->{"ipaddr"} = $ipaddr;
    }
    else
    {
        print STDERR "BIGIP::ippadr can only be set for SSH\n" if (@_);
        $self->{"ipaddr"};
    };
};

sub port {
    my $self = shift;
    if ((@_) and ($self->{$interface} eq 'SSH'))
    {
        $self->{"port"} = shift;
    }
    else
    {
        print STDERR "BIGIP::port can only be set for SSH\n" if (@_);
        $self->{"port"};
    };
};

sub interface {
    my $self = shift;
    if (@_)
    {
        $interface = shift;
        die "BIGIP.pm - Invalid Interface: " . $interface . "\n"
        unless (exists($self->{$interface}) && $self->{$interface} eq $interface);
        $self->{"interface"} = $interface;
    }
    else
    {
        $self->{"interface"};
    };
};

sub retries {
    my $self = shift;
    if ((@_) and ($self->{$interface} eq 'SSH'))
    {
        my $count = shift;
        if ($count =~ /^\d{1,2}$/)
        {
           $self->{"retries"} = $count;
        }
        else
        {
           $self->{"retries"} = 3;  # Fix Invalid retries value
        };
    }
    else
    {
        print STDERR "BIGIP::port can only be set for SSH\n" if (@_);
        $self->{"retries"};
    };
};


sub SSH {
   my $self = shift;
   if (@_)
   {
       my @responses = qw();
       my $SSH = 'ssh ';
       $SSH .=       $self->{'user'}     if $self->{'user'};
       $SSH .= ":" . $self->{'password'} if $self->{'user'} && $self->{'password'};
       $SSH .= "@"                       if $self->{"user"};
       $SSH .=       $self->{'ipaddr'};
       my $errorcode = 0;
       while ((@_) && ($errorcode == 0))
       {
           # (my $cmd, my $regexpr)  = @_;
           my $cmd = shift;
           my $regexpr = '';
           if (@_)
           {
               $regexpr  = shift;
           };
           my $response = '';
           if ($regexpr)
           {
              my $count = $self->{"retries"};
              if (!$count =~ /^\d{1,2}$/) {
                 $count = 3;
                 $self->{"retries"} = $count;  # Fix Invalid retries value
              };

              do {
                    if ($self->debug())
                    {
                      $response = "$SSH $cmd 2>&1";
                      $errorcode = 0;
                    }
                    else
                    {
                      $response = "$SSH $cmd 2>&1 (Rx:$regexpr)";
                      $errorcode = $?;
                    };
                 } until ( ($response =~ /$regexpr/)
                        or ($count-- == 0)
                        or ($self->debug())
                        or ( ($errorcode > 0) and ($self->stoponerror()) )
                         );
           }
           else
           {
              if ($self->debug())
              {
                $response = "$SSH $cmd 2>&1" if     $self->debug();
                $errorcode = 0;
              }
              else
              {
                $response = `$SSH $cmd 2>&1` unless $self->debug();
                $errorcode = $?;
              };
           };
           unshift(@responses,$response);
           my $rref = $self->errorcodes();
           unshift(@$rref,$errorcode);
       };
       $self->{errorcode} = $errorcode;
       unshift(@{$self->{errorcodes}},$errorcode);
       unshift(@{$self->{responses}},\@responses);
   }
   return $self->{responses};                      # This statement too needs to be the last one
};

sub iControl {
   my $self = shift;
   die "BIGIP.pm *ERROR* interface iControl cannot use send method: iControl\n";
}

sub send {
   my $self = shift;
   if ($self->{interface} eq 'SSH')
   {
      $self->SSH(@_);
   }
   elsif ($self->{interface} eq 'iControl')
   {
      $self->iControl(@_);
   }
   else
   {
      die "BIGIP.pm - Invalid Interface: " . $self->{interface} . "\n";
   }
}

sub print {
    my $self = shift;
    printf STDERR "user=%s\n" .
              "password=%s\n" .
                  "host=%s\n" .
                "ipaddr=%s\n" .
                  "port=%s\n" .
             "interface=%s\n" .
               "retries=%s\n",
                $self->user(),
            $self->password(),
                $self->host(),
              $self->ipaddr(),
                $self->port(),
           $self->interface(),
             $self->retries();

};


#===============================================================================
#  S S H   F U N C T I O N A L   M E T H O D S
#===============================================================================
#
sub addToPool {  #   <pool_name> <member1> [<priority1>] ... <memberN> [<priorityN>]
   my $self    = shift;
   my $pool    = shift;

   die "BIGIP::addToPool only applicable for SSH interface" unless $self->{interface} eq 'SSH';

   my @members    = qw();
   my @cmdMon     = qw();
   my @responses  = qw();

   my $member     = qw();
   my $retries    = $self->retries();
   my $respRef;

   # check If monitor exists
   my $cmdMon  = 'b monitor mon_' . $pool . ' show';

   $respRef = $self->send($cmdMon,'.*');
   unshift(@responses,@$respRef);
   $monitor = 'mon_' . $pool unless ($self->errorcode() == 1);

   # Add members to the pool
   my $cmdPool = 'b pool ' . $pool . ' add {';
   while (@_)
   {
       $member = shift;
       push(@members,$member);
       $cmdPool .= ' member ' . $member;
       if (@_)
       {
           my $priority = shift;
           $cmdPool .= ' priority ' . $priority if ($priority ne "");
       };
       if ($monitor)
       {
           my $cmdMon = 'b node ' . $member . ' use monitor mon_' . $pool;
           push(@cmdMon,$cmdMon);
           push(@cmdMon,'^$');
       };

   };
   $cmdPool .= ' }';

   my $check = join('|',@members);
   my $cmdShow = 'b pool ' . $pool . ' show';

   do
   {
       $respRef = $self->send($cmdPool,'^$',$cmdShow,$check); # 'POOL');
   }
   until (($$respRef[0] =~ /$check/)
       or ($retries-- == 0)
       or ($self->errorcode() != 0)
       or ($self->debug())
         );

   unshift(@responses,@$respRef);

   $errorcode = ($$respRef[0] =~ /$check/) unless $errorcode;
   printm "BIGIP::addToPool: Could not add members to pool %s\n", $pool if ($errorcode and !$self->debug());

   if ($monitor)
   {
       $respRef = $self->send(@cmdMon);
       unshift(@responses,@$respRef);
   }


   $errorcode = ($$respRef[0] =~ /$check/) unless $errorcode;
   $self->errorcode($errorcode);

   return \@responses;

};


#===============================================================================

sub removeFromPool {  #   <pool_name> <member1>  ... <memberN>
   my $self    = shift;
   my $pool    = shift;

   die "BIGIP::removeFromPool only applicable for SSH interface" unless $self->{interface} eq 'SSH';

   my @members    = @_;
   my @responses  = qw();
   my @cmdMon     = qw();
   my @cmdPool    = qw();

   my $member     = qw();
   my $retries    = $self->retries();
   my $respRef;

   push(@cmdMon, (sprintf("b node %s monitor delete",$_),''))               foreach @members;
   push(@cmdPool,(sprintf("b pool %s delete { member %s }",$pool,$_),'^$')) foreach @members;
   my $cmdShow = 'b pool ' . $pool . ' show';
   my $check   = join('|',@members);
   do
   {
       $respRef = $self->send(@cmdMon,@cmdPool,$cmdShow,'POOL');
       unshift(@responses,@$respRef);
   }
   while (($$respRef[0] =~ /$check/)
      and ($retries-- > 0)
      and ($self->errorcode() == 0)
      and (!$self->debug())
         );

   $errorcode = ($$respRef[0] =~ /$check/) unless $errorcode;
   printm "BIGIP::removeFromPool: Could not remove members to pool %s\n", $pool if ($errorcode and !$self->debug());

   $self->errorcode($errorcode);

   return \@responses;

};


#===============================================================================

sub poolDel {  #   <pool_name>
   my $self    = shift;     # Myself Object Handle
   my $pool    = shift;     # pool name

   die "BIGIP::poolDel only applicable for SSH interface" unless $self->{interface} eq 'SSH';

   my $retries = $self->retries();     # Get # retries
   my $respRef;                        # Used to hold answers
   my @responses = qw();               # Accumulate send responses

   my $cmdShow = 'b pool ' . $pool . ' show';     # Show monitor template bigpipe command
   my $cmdPool = 'b pool ' . $pool . ' delete ';
   do
   {
       $respRef = $self->send($cmdPool,'^$',$cmdShow,'not\sfound');
       unshift(@responses,@$respRef);
   }
   until ( ($respRef->[0] =~ /not\sfound/)
       or  ($retries-- == 0)
       or  ($self->errorcode() != 0)
       or  ($self->debug())
         );


   return \@responses;

};


#===============================================================================

sub addMonitor { # b monitor mon_sg549743Test1 {use tcp interval 10 timeout 16}
   my $self     = shift;
   my $monitor  = shift;

   die "BIGIP::addMonitor only applicable for SSH interface" unless $self->{interface} eq 'SSH';

   my $interval = 10;
   my $timeout  = 16;
   my $use      = 'tcp';

   $interval = shift if (@_);
   $timeout  = shift if (@_);
   $use      = shift if (@_);

   my $cmdMon  = sprintf ("b monitor %s { use %s interval %s timeout %s }"
                         , $monitor, $use, $interval, $timeout
                         );
   my @responses = qw();
   my $respRef   = qw();

   do
   {
       $respRef = $self->send($cmdMon,'^$');
       unshift(@responses,@$respRef);
   }
   until ( ($respRef->[0] =~ /existing/)
       or ($retries-- == 0)
       or ($self->errorcode() != 0)
         );

   return \@responses;


};


#===============================================================================

sub deleteMonitorSSH { # b monitor mon_sg549743Test1 delete
   my $self     = shift;
   my $monitor  = shift;

   die "BIGIP::deleteMonitor only applicable for SSH interface" unless $self->{interface} eq 'SSH';

   my $cmdMon  = sprintf ("b monitor %s delete", $monitor);
   my $cmdShow = sprintf ("b monitor %s show  ", $monitor);
   my @responses = qw();
   my $respRef   = qw();

   do
   {
       $respRef = $self->send($cmdMon,'^$',$cmdShow,'.*');
       unshift(@responses,@$respRef);
   }
   until ( ($retries-- == 0) or ($self->errorcode() == 1) );

   return \@responses;


};


#===============================================================================

sub modifyMonitor { # b monitor mon_sg549743Test1 {use tcp interval 10 timeout 16}
   my $self     = shift;

   die "BIGIP::modifyMonitor only applicable for SSH interface" unless $self->{interface} eq 'SSH';

   return $self->monAdd(@_);
};

#===============================================================================
#===============================================================================
#   M e t h o d s   t h a t   u s e s   i C o n t r o l   I n t e r f a c e   ==
#===============================================================================
#===============================================================================
#===============================================================================


sub inerror {   # iControl ERROR HANDLER ***************************
   my $self     = shift;
   my $soap_response = shift;
   my $quiet = shift if @_;
   if ( $soap_response->fault )
   {
      $self->errorcode($soap_response->faultcode);
      unshift(@{$self->{errorcodes}},$self->errorcode());
      unshift(@{$self->{responses}},\($soap_response->faultstring . "\n"));

      return TRUE if $quiet;

      print STDERR $soap_response->faultcode, " ", $soap_response->faultstring, "\n"
      unless ($self->errorcount(1) > $self->errorlimit());

      exit(1) if $self->{stoponerror};

      die "Error limit exceeded\n"
       if $self->{stoponlimit} and ($self->errorcount(1) > $self->errorlimit());
      return TRUE;
   };
   return FALSE;
}

sub getVersion { # Gets BIGIP software version

   my $self = shift;

   my $soap_response = $self->{iCTLNode} ->get_version();
   return $soap_response->result unless $soap_response->fault;
   return "FAULT : " . $soap_response->faultcode;

};

#--------- NODE OPERATIONS -----------------------------------------------------

sub disableNodes {  # Quiesce NODES so current transaction will not fail

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a list of nodes <ip>:<port>
# returns:  1 if success, 0 on failure
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   my $self = shift;
   my (@nodes) = @_;   # Node format is <ip-address>:<port>

   die "BIGIP::disableNodes only applicable for iControl interface" unless $self->{interface} eq 'iControl';

   my @members = ();
   foreach $node (@nodes)
   {
      if ($node =~ /$NODEFMT/)
      {
          my $node_definition = { address => "$1.$2.$3.$4", port => "$5" };
          push(@members,$node_definition);
      }
   };

   my $soap_response = $self->{iCTLNode} ->set_state
                  ( SOAP::Data->name (node_defs => [@members])
                  , SOAP::Data->name (state     => DISABLED)
                  );

   return 1 unless $self->inerror($soap_response);
   return 0;

}

#===============================================================================

sub enableNodes {  # enable Node to receive transactions

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a list of nodes <ip>:<port>
# returns:  1 if success, 0 on failure
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   my $self = shift;
   my (@nodes) = @_;   # Node format is <ip-address>:<port>

   die "BIGIP::enableNodes only applicable for iControl interface" unless $self->{interface} eq 'iControl';

   my @members = ();
   foreach $node (@nodes)
   {
      if ($node =~ /$NODEFMT/)
      {
          my $node_definition = { address => "$1.$2.$3.$4", port => "$5" };
          push(@members,$node_definition);
      }
   };

   my $soap_response = $self->{iCTLNode} ->set_state
                  ( SOAP::Data->name (node_defs => [@members])
                  , SOAP::Data->name (state     => ENABLED)
                  );

   return 1 unless $self->inerror($soap_response);
   return 0;

}

#===============================================================================

sub bringDownNodes {  # Take node down

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a list of nodes <ip>:<port>
# returns:  1 if success, 0 on failure
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   my $self = shift;
   my (@nodes) = @_;   # Node format is <ip-address>:<port>

   die "BIGIP::bringDownNodes only applicable for iControl interface" unless $self->{interface} eq 'iControl';

   my @members = ();
   foreach $node (@nodes)
   {
      if ($node =~ /$NODEFMT/)
      {
          my $node_definition = { address => "$1.$2.$3.$4", port => "$5" };
          push(@members,$node_definition);
      }
   };

   my $soap_response = $self->{iCTLNode} ->set_availability
                  ( SOAP::Data->name (node_defs => [@members])
                  , SOAP::Data->name (state     => DOWN)
                  );

   return 1 unless $self->inerror($soap_response);
   return 0;

}

#===============================================================================

sub bringUpNodes {  # Bring node up

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a list of nodes <ip>:<port>
# returns:  1 if success, 0 on failure
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   my $self = shift;
   my (@nodes) = @_;   # Node format is <ip-address>:<port>

   die "BIGIP::bringUpNodes only applicable for iControl interface" unless $self->{interface} eq 'iControl';

   my @members = ();
   foreach $node (@nodes)
   {
      if ($node =~ /$NODEFMT/)
      {
          my $node_definition = { address => "$1.$2.$3.$4", port => "$5" };
          push(@members,$node_definition);
      }
   };

   my $soap_response = $self->{iCTLNode} ->set_availability
                  ( SOAP::Data->name (node_defs => [@members])
                  , SOAP::Data->name (state     => UP)
                  );

   return 1 unless $self->inerror($soap_response);
   return 0;
}

#====  MONITOR OPERATIONS =========================================================

sub verifyMonitorTemplateExists { # verify that a monitor template exists

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  template name
# returns:  1 if success, 0 on failure
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self = shift;
    my $template = shift unless scalar @_ == 0;

    die "BIGIP::verifyMonitorTemplateExists only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::verifyMonitorTemplateExists error, missing Template Name" unless $template;

    $soap_response = $self->{iCTLMonitor2} ->get_template_type
                     ( SOAP::Data->name ( template_name => $template ) );
    return 1 unless $self->inerror($soap_response);
    return 0 ;
};

#====  POOL OPERATIONS =========================================================

sub listPool { # Return list of pools of list of members of a pool

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  void or a pool name
# returns:  a list of pools if in is void or a list of nodes <ip>:<port>
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self = shift;
    my $pool = shift unless scalar @_ == 0;

    die "BIGIP::listPool only applicable for iControl interface" unless $self->{interface} eq 'iControl';


    my $soap_response;
    my @members = ();
    if ($pool)   # If there is a pool, return member Nodes else the pool list
    {
       $soap_response = $self->{iCTLPool} ->get_member_list
                        ( SOAP::Data->name ( pool_name => $pool ) );
       my @IPPortDefs = @{$soap_response->result} unless $self->inerror($soap_response);
       foreach $IPPortDef (@IPPortDefs)
       {
           push(@members,$IPPortDef->{address} . ":" . $IPPortDef->{port})
       };
    }
    else
    {
       $soap_response = $self->{iCTLPool} ->get_list();
       @members = @{$soap_response->result} unless $self->inerror($soap_response);
    }

    return sort @members;
};

#===============================================================================

sub verifyPoolExists { # verify that a pool exists

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  pool name
# returns:  0 if pool was verified
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self = shift;
    my $pool = shift unless scalar @_ == 0;

    die "BIGIP::verifyPoolExists only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::verifyPoolExists error, missing Pool Name" unless $pool;

    $soap_response = $self->{iCTLPool} ->get_lb_method
                     ( SOAP::Data->name ( pool_name => $pool ) );
    return 0 unless $self->inerror($soap_response);
    return 1
};

#===============================================================================

sub getAvailabilityOfNodesFromPool { # Returns Availability of Nodes from Pool

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a pool name
# returns:  a list of pairs (nodes (<ip>:<port>) and their availability)
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self = shift;
    my $pool = shift unless scalar @_ == 0;

    die "BIGIP::getAvailabilityOfNodesFromPool only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::getAvailabilityOfNodesFromPool error, missing Pool Name"               unless $pool;


    my $soap_response;
    my @members = ();
    my @members_status = ();

    # Get List of Nodes from pool
    $soap_response = $self->{iCTLPool} ->get_member_list
                     ( SOAP::Data->name ( pool_name => $pool ) );
    my @IPPortDefs = @{$soap_response->result} unless $self->inerror($soap_response);

    # Get Availability of nodes from list
    $soap_response = $self->{iCTLNode} ->get_availability_ex
                     ( SOAP::Data->name (node_defs => [@IPPortDefs]) );
    my @availabilityStates = @{$soap_response->result} unless $self->inerror($soap_response);

    push(@members_status,$NodeAvailMap{$_}) foreach (@availabilityStates);

    my $status_x = 0;
    foreach $IPPortDef (@IPPortDefs)
    {
        my $IPPort = $IPPortDef->{address} . ":" . $IPPortDef->{port};
        if ($IPPort ne '1.1.1.4:1') {
           push(@members,$IPPortDef->{address} . ":" . $IPPortDef->{port});
           push(@members,$members_status[$status_x]);
        };
        $status_x++;
    };


    return @members;
};
#===============================================================================

sub numberOfAvailableNodesFromPool { # Returns number of available nodes from a given pool

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a pool name
# returns:  number of available nodes (AVAILABILITY_UP status)
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self = shift;
    my $pool = shift unless scalar @_ == 0;

    die "BIGIP::numberOfAvailableNodesFromPool only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::numberOfAvailableNodesFromPool error, missing Pool Name"               unless $pool;


    my $soap_response;

    # Get List of Nodes from pool
    $soap_response = $self->{iCTLPool} ->get_member_list
                     ( SOAP::Data->name ( pool_name => $pool ) );
    my @IPPortDefs = @{$soap_response->result} unless $self->inerror($soap_response);

    # Get Availability of nodes from list
    $soap_response = $self->{iCTLNode} ->get_availability_ex
                     ( SOAP::Data->name (node_defs => [@IPPortDefs]) );
    my @availabilityStates = @{$soap_response->result} unless $self->inerror($soap_response);

    my $nodes_up_count = 0;
    foreach (@availabilityStates)
    {
        $nodes_up_count++ if ($NodeAvailMap{$_} eq 'AVAILABILITY_UP');
    };

    return $nodes_up_count;
};


#===============================================================================
sub listNodesFromPool { # List nodes from Pool

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a pool name followed by an optional regex for node selection
# returns:  a list of nodes <ip>:<port>
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   my $self = shift;
   my $pool = shift unless scalar @_ == 0;
   my $regx = shift unless scalar @_ == 0;

   die "BIGIP::listNodesFromPool only applicable for iControl interface" unless $self->{interface} eq 'iControl';
   die "BIGIP::listNodesFromPool error, missing Pool Name"               unless $pool;

   if ($regx)
   {
       return grep(/$regx/, $self->listPool($pool) );
   }
   else
   {
   return $self->listPool($pool);
   };

};

#===============================================================================

sub createPool { # Create a new pool with place holder 1.1.1.4:1

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a pool name, followed by a method name (see %LBMethodMap)
# returns:  1 if success, 0 on failure
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self     = shift;
    my $pool     = shift unless scalar @_ == 0;
    my $lbmethod = shift unless scalar @_ == 0;

    die "BIGIP::createPool only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::createPool error, missing Pool Name"               unless $pool;

    if (exists($LBMethodMap{$lbmethod}))
    {
       $lbmethod = $LBMethodMap{$lbmethod};
    }
    else
    {
       $lbmethod = $LBMethodMap{Default};
    };

    my $node_definition = { address => '1.1.1.4', port => '1' };

    printm "Creating pool %s with placeholder %s\n",
           $pool,
           $node_definition->{address} . ':' . $node_definition->{port}
           if $self->verbose()
           ;
    my $soap_response = $self->{iCTLPool} ->create
                        ( SOAP::Data->name ( pool_name => "$pool" )
                        , SOAP::Data->name ( lb_method => $lbmethod )
                        , SOAP::Data->name ( members   => [$node_definition] )
                        );
    return 1 unless $self->inerror($soap_response);
    return 0;

};

#===============================================================================

sub addNodesToPool { # Add a node to a pool

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a pool name, followed by a list of nodes <ip>:<port>
# returns:  1 if success, 0 on failure
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self     = shift;
    my $pool     = shift unless scalar @_ == 0;
    my @nodes    = @_    unless scalar @_ == 0;

    die "BIGIP::addNodesToPool only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::addNodesToPool error, missing Pool Name"               unless $pool;

    my @members = ();
    foreach $node (@nodes)
    {
       if ($node =~ /$NODEFMT/)
       {
           my $node_definition = { address => "$1.$2.$3.$4", port => "$5" };
           push(@members,$node_definition);
       }
    };

    my $soap_response = $self->{iCTLPool} ->add_members
                        ( SOAP::Data->name ( pool_name => $pool )
                        , SOAP::Data->name ( members   => [@members] )
                        );

    return 1 unless $self->inerror($soap_response);
    return 0;

};

#===============================================================================

sub deleteNodesFromPool { # Remove nodes from a pool

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a pool name, followed by a list of nodes <ip>:<port>
# returns:  1 if success, 0 on failure
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self     = shift;
    my $pool     = shift unless scalar @_ == 0;
    my @nodes    = @_    unless scalar @_ == 0;

    die "BIGIP::deleteNodesFromPool only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::deleteNodesFromPool error, missing Pool Name"               unless $pool;

    my @members = ();
    foreach $node (@nodes)
    {
       if ($node =~ /$NODEFMT/)
       {
           if ("$1.$2.$3.$4:$5" ne "1.1.1.4:1")
           {
               my $node_definition = { address => "$1.$2.$3.$4", port => "$5" };
               push(@members,$node_definition);
           };
       };
    };

    return 0 if (scalar @members == 0);

    my $soap_response = $self->{iCTLPool} ->delete_members
                        ( SOAP::Data->name ( pool_name => $pool )
                        , SOAP::Data->name ( members   => [@members] )
                        );

    return 1 unless $self->inerror($soap_response);
    return 0;

};

#===============================================================================

sub deleteAllNodesFromPool { # Remove All nodes from a pool

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a pool name
# returns:  1 if success, 0 on failure
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self     = shift;
    my $pool     = shift unless scalar @_ == 0;

    die "BIGIP::deleteAllNodesFromPool only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::deleteAllNodesFromPool error, missing Pool Name"               unless $pool;

    my @nodes = $self->listPool($pool);
    return $self->deleteNodesFromPool($pool,@nodes);

};

#===============================================================================

sub bringDownAllNodesFromPool { # Bring down all nodes from a pool

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a pool name
# returns:  1 if success, 0 on failure
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self     = shift;
    my $pool     = shift unless scalar @_ == 0;

    die "BIGIP::bringDownAllNodesFromPool only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::bringDownAllNodesFromPool error, missing Pool Name"               unless $pool;

    my @nodes = $self->listPool($pool);
    return $self->bringDownNodes($pool,@nodes);

};

#===============================================================================

sub bringUpAllNodesFromPool { # Bring Up All nodes from a pool

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a pool name
# returns:  1 if success, 0 on failure
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self     = shift;
    my $pool     = shift unless scalar @_ == 0;

    die "BIGIP::bringUpAllNodesFromPool only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::bringUpAllNodesFromPool error, missing Pool Name"               unless $pool;

    my @nodes = $self->listPool($pool);
    return $self->bringUpNodes($pool,@nodes);

};

#===============================================================================

sub disableAllNodesFromPool { # Disable all nodes from a pool

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a pool name
# returns:  1 if success, 0 on failure
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self     = shift;
    my $pool     = shift unless scalar @_ == 0;

    die "BIGIP::disableAllNodesFromPool only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::disableAllNodesFromPool error, missing Pool Name"               unless $pool;

    my @nodes = $self->listPool($pool);
    return $self->disableNodes($pool,@nodes);

};

#===============================================================================

sub enableAllNodesFromPool { # Enable All nodes from a pool

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a pool name
# returns:  1 if success, 0 on failure
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self     = shift;
    my $pool     = shift unless scalar @_ == 0;

    die "BIGIP::enableAllNodesFromPool only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::enableAllNodesFromPool error, missing Pool Name"               unless $pool;

    my @nodes = $self->listPool($pool);
    return $self->enableNodes($pool,@nodes);

};

sub getCurrentConnectionsFromPoolMembers {

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a pool name, followed by a list of nodes <ip>:<port>
# returns:  a list of pairs [current connections <ip>:<port>], one pair per <ip>:<port> requested
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self     = shift;
    my $pool     = shift unless scalar @_ == 0;
    my @nodes    = @_    unless scalar @_ == 0;

    die "BIGIP::getCurrentConnectionsFromPoolMembers only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::getCurrentConnectionsFromPoolMembers error, missing Pool Name"               unless $pool;
    die "BIGIP::getCurrentConnectionsFromPoolMembers error, missing node(s)  "               unless scalar @nodes;

    my @members = ();
    foreach $node (@nodes)
    {
       if ($node =~ /$NODEFMT/)
       {
           if ("$1.$2.$3.$4:$5" ne "1.1.1.4:1")
           {
               my $node_definition = { address => "$1.$2.$3.$4", port => "$5" };
               push(@members,$node_definition);
           };
       };
    };

    return (0) if (scalar @members == 0);

    # Get all members statistics
    my $soap_response = $self->{iCTLPool} ->get_member_statistics
                        ( SOAP::Data->name ( pool_name   => $pool )
                        , SOAP::Data->name ( member_defs => [@members] )
                        );

    my @results = ();
    my @MemberStatisticsEntry = @{$soap_response->result} unless $self->inerror($soap_response);
    foreach $memberEntry (@MemberStatisticsEntry)
    {
        my $IPPortDef          = $memberEntry->{member_definition};
        my $IPPort = $IPPortDef->{address} . ":" . $IPPortDef->{port};

        my $MemberStatistics   = $memberEntry->{stats};

          my $thruput_stats      = $MemberStatistics->{thruput_stats};
            my $bits_in              = $thruput_stats->{bits_in};
            my $bits_out             = $thruput_stats->{bits_out};
            my $packets_in           = $thruput_stats->{packets_in};
            my $packets_out          = $thruput_stats->{packets_in};

          my $connection_stats = $MemberStatistics->{connection_stats};
            my $current_connections  = $connection_stats->{current_connections};
            my $maximum_connections  = $connection_stats->{maximum_connections};
            my $total_connections    = $connection_stats->{total_connections};

        if ($IPPort ne '1.1.1.4:1')
        {
           push(@results,$current_connections);
           push(@results,$IPPort);
        };

    };

    return @results;

};


#================MONITOR HANDLING===============================================

#===============================================================================

sub listMonitors { # List monitor templates

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  void
# returns:  a list of all monitor template names
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self     = shift;

    die "BIGIP::listMonitors only applicable for iControl interface" unless $self->{interface} eq 'iControl';

    my $soap_response = $self->{iCTLMonitor2} ->get_template_list();

    my @monitorTemplates = @{$soap_response->result} unless $self->inerror($soap_response);
    my @members = qw();

    foreach my $monitorTemplate (@monitorTemplates)
    {
       my $monitorType = $monitorTemplate->{template_type};
       my $monitorTypeName = $TemplateType{$monitorType};
       push(@members,$monitorTemplate->{template_name} . ":" . $monitorTypeName);
    };

    return sort @members;
    return 1 unless $self->inerror($soap_response);
    return 0;

};

#===============================================================================

sub deleteMonitor { # Deletes a monitor template

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a monitor template name
# returns:  1 if success, 0 on failure
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self     = shift;
    my $monitor  = shift unless scalar @_ == 0;

    die "BIGIP::deleteMonitor only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::deleteMonitor error, missing Monitor Name"            unless $monitor;

    my $soap_response = $self->{iCTLMonitor} ->delete_monitor
                        ( SOAP::Data->name ( monitor_name => $monitor )
                        );

    return 1 unless $self->inerror($soap_response);
    return 0;

};

#===============================================================================

sub addMonitorUseTCP { # Creates a monitor template to use TCP

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a monitor template name, followed by send string and Recv String
# returns:  1 if success, 0 on failure
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self     = shift;
    my $monitor  = shift unless scalar @_ == 0;
    my ($send_string, $receive_string) = ("","");
    $send_string = shift unless scalar @_ == 0;
    $recv_string = shift unless scalar @_ == 0;

    die "BIGIP::addMonitorUseTCP only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::addMonitorUseTCP error, missing Monitor Name"            unless $monitor;

    my $soap_response = $self->{iCTLMonitor} ->set_use_tcp
                        ( SOAP::Data->name ( monitor_name => $monitor )
                        , SOAP::Data->name ( send_string  => $send_string )
                        , SOAP::Data->name ( recv_string  => $recv_string )
                        );

    return 1 unless $self->inerror($soap_response);
    return 0;

};

#===============================================================================

sub setMonitorInterval { # Set the interval for a monitor template

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a monitor template name, followed by a interval value
# returns:  1 if success, 0 on failure
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self     = shift;
    my $monitor  = shift unless scalar @_ == 0;
    my $value    = shift unless scalar @_ == 0;

    die "BIGIP::setMonitorInterval only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::setMonitorInterval error, missing Monitor Name"            unless $monitor;

    my $soap_response = $self->{iCTLMonitor2} ->set_template_integer_property
                        ( SOAP::Data->name ( template_name  => $monitor )
                        , SOAP::Data->name ( property_type  => ITYPE_INTERVAL )
                        , SOAP::Data->name ( property_value => $value )
                        );

    return 1 unless $self->inerror($soap_response);
    return 0;

};

#===============================================================================

sub setMonitorTimeout { # Set the timeout for a monitor template

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a monitor template name, followed by a timeout value
# returns:  1 if success, 0 on failure
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self     = shift;
    my $monitor  = shift unless scalar @_ == 0;
    my $value    = shift unless scalar @_ == 0;

    die "BIGIP::setMonitorTimeout only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::setMonitorTimeout error, missing Monitor Name"            unless $monitor;

    my $soap_response = $self->{iCTLMonitor2} ->set_template_integer_property
                        ( SOAP::Data->name ( template_name  => $monitor )
                        , SOAP::Data->name ( property_type  => ITYPE_TIMEOUT )
                        , SOAP::Data->name ( property_value => $value )
                        );

    return 1 unless $self->inerror($soap_response);
    return 0;

};

#===============================================================================

sub getMonitorInterval { # Get the Interval for a monitor template

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a monitor template name
# returns:  The monitor interval value
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self     = shift;
    my $monitor  = shift unless scalar @_ == 0;

    die "BIGIP::getMonitorInterval only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::getMonitorInterval error, missing Monitor Name"            unless $monitor;

    my $soap_response = $self->{iCTLMonitor2} ->get_template_integer_property
                        ( SOAP::Data->name ( template_name  => $monitor )
                        , SOAP::Data->name ( property_type  => ITYPE_INTERVAL )
                        );

    return $soap_response->result unless $self->inerror($soap_response);

};

#===============================================================================

sub getMonitorTimeout { # Set the timeout for a monitor template

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a monitor template name
# returns:  The monitor timeout value
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self     = shift;
    my $monitor  = shift unless scalar @_ == 0;

    die "BIGIP::getMonitorTimeout only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::getMonitorTimeout error, missing Monitor Name"            unless $monitor;

    my $soap_response = $self->{iCTLMonitor2} ->get_template_integer_property
                        ( SOAP::Data->name ( template_name  => $monitor )
                        , SOAP::Data->name ( property_type  => ITYPE_TIMEOUT )
                        );

    return $soap_response->result unless $self->inerror($soap_response);

};


#===============================================================================

sub listMonitorAssociationOfNodes { # List monitor Associated to the nodes

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a list of nodes <ip>:<port>
# returns:  a list of corresponding monitor template names
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self     = shift;
    my @nodes    = @_    unless scalar @_ == 0;

    die "BIGIP::listMonitorToNodes only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::listMonitorToNodes error, missing Monitor Name"            unless @nodes;

    my $count = 0;
    my $soap_response;

    my @responses = qw();

    foreach $node (@nodes)
    {
       if ($node =~ /$NODEFMT/)
       {
           my $ipport_definition   = { address => "$1.$2.$3.$4", port => "$5" };
           my $monitor_ipport      = { address_type => ATYPE_EXPLICIT_ADDRESS_EXPLICIT_PORT
                                     , ipport       => $ipport_definition
                                     };
           $soap_response = $self->{iCTLMonitor2} ->get_monitor_information
                            ( SOAP::Data->name ( node_definition => $monitor_ipport )
                            );

          #my $monitorInfo = $soap_response->result unless $self->inerror($soap_response);
           if ( not $self->inerror($soap_response,'quiet') )
           {
               my $monitorInfo = $soap_response->result;

           my @monitor_instances   = $monitorInfo->{monitor_instances};
           my $monitor_association = $monitorInfo->{monitor_association};

           my $monitor_nodeDef     = $monitor_association->{node_definition};
           my $template_names_ref  = $monitor_association->{template_names};

           my $monitor_addresstype = $monitor_nodeDef->{address_type};
           my $monitor_ipport_def  = $monitor_nodeDef->{ipport};
           my $monitor_ipport_str  = $monitor_ipport_def->{address} . ':' . $monitor_ipport_def->{port};

               push (@responses, join(',',@$template_names_ref));
           };
       }
    };

    return @responses;

};

#===============================================================================

sub listMonitorAssociationOfServices { # List monitor Associated to services

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a list of services <port>
# returns:  a list of corresponding monitor template names
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self     = shift;
    my @services = @_    unless scalar @_ == 0;

    die "BIGIP::listMonitorToServices only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::listMonitorToServices error, missing Service port(s)"         unless @services;

    my $count = 0;
    my $soap_response;

    my @responses = qw();

    foreach $service (@services)
    {
       if ($service =~ /$SERVFMT/)
       {
           my $ipport_definition   = { address => "*", port => $service };
           my $monitor_ipport      = { address_type => ATYPE_STAR_ADDRESS_EXPLICIT_PORT
                                     , ipport       => $ipport_definition
                                     };
           $soap_response = $self->{iCTLMonitor2} ->get_monitor_information
                            ( SOAP::Data->name ( node_definition => $monitor_ipport )
                            );

          #my $monitorInfo = $soap_response->result unless $self->inerror($soap_response);
           if ( not $self->inerror($soap_response) )
           {
               my $monitorInfo = $soap_response->result;

               my @monitor_instances   = $monitorInfo->{monitor_instances};
               my $monitor_association = $monitorInfo->{monitor_association};

               my $monitor_nodeDef     = $monitor_association->{node_definition};
               my $template_names_ref  = $monitor_association->{template_names};

               my $monitor_addresstype = $monitor_nodeDef->{address_type};
               my $monitor_ipport_def  = $monitor_nodeDef->{ipport};
               my $monitor_ipport_str  = $monitor_ipport_def->{address} . ':' . $monitor_ipport_def->{port};

               push (@responses, join(',',@$template_names_ref));
           };
       }
    };

    return @responses;

};

#===============================================================================

sub assignMonitorToNodes { # Associates monitor template to nodes

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a monitor template name followed by a list of nodes <ip>:<port>
# returns:  number of nodes that had successfully the monitor assigned to
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self     = shift;
    my $monitor  = shift unless scalar @_ == 0;
    my @nodes    = @_    unless scalar @_ == 0;

    die "BIGIP::assignMonitorToNodes only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::assignMonitorToNodes error, missing Monitor Name"            unless $monitor;

    my $count = 0;
    my $soap_response;

    foreach $node (@nodes)
    {
       if ($node =~ /$NODEFMT/)
       {
           my $ipport_definition   = { address => "$1.$2.$3.$4", port => "$5" };
           my $monitor_ipport      = { address_type => ATYPE_EXPLICIT_ADDRESS_EXPLICIT_PORT
                                     , ipport       => $ipport_definition
                                     };
           my $monitor_association = { node_definition => $monitor_ipport
                                     , template_names  => [$monitor]
                                     };
           $soap_response = $self->{iCTLMonitor2} ->create_association
                            ( SOAP::Data->name ( monitor_association => $monitor_association )
                            );

           $count++ unless $self->inerror($soap_response);
       }
    };

    return $count;

};

#===============================================================================

sub assignMonitorToServices { # Associates monitor template to Services

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a monitor template name followed by a list of services *:<port>
# returns:  number of nodes that had successfully the monitor assigned to
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self     = shift;
    my $monitor  = shift unless scalar @_ == 0;
    my @services = @_    unless scalar @_ == 0;

    die "BIGIP::assignMonitorToServices only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::assignMonitorToServices error, missing Monitor Name"            unless $monitor;

    my $count = 0;
    my $soap_response;

    foreach $service (@services)
    {
       if ($service =~ /$SERVFMT/)
       {
           printm "Assigning monitor %s to service %s\n", $monitor, $service
                  if $self->verbose();

           my $ipport_definition   = { address => "*", port => "$service" };
           my $monitor_ipport      = { address_type => ATYPE_STAR_ADDRESS_EXPLICIT_PORT
                                     , ipport       => $ipport_definition
                                     };
           my $monitor_association = { node_definition => $monitor_ipport
                                     , template_names  => [$monitor]
                                     };
           $soap_response = $self->{iCTLMonitor2} ->create_association
                            ( SOAP::Data->name ( monitor_association => $monitor_association )
                            );

           $count++ unless $self->inerror($soap_response);
       }
    };

    return $count;

};

#===============================================================================

sub deleteMonitorFromNodes { # Disassociates monitor from nodes

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a list of nodes <ip>:<port> addresses to delete its monitor
# returns:  number of nodes that had successfully deleted its monitor
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self     = shift;
    my @nodes    = @_    unless scalar @_ == 0;

    die "BIGIP::deleteMonitorFromNodes only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::deleteMonitorFromNodes error, missing Monitor Name"            unless @nodes;

    my $count = 0;
    my $soap_response;

    foreach $node (@nodes)
    {
       if ($node =~ /$NODEFMT/)
       {
           my $ipport_definition   = { address => "$1.$2.$3.$4", port => "$5" };
           my $monitor_ipport      = { address_type => ATYPE_EXPLICIT_ADDRESS_EXPLICIT_PORT
                                     , ipport       => $ipport_definition
                                     };
           $soap_response = $self->{iCTLMonitor2} ->delete_association
                            ( SOAP::Data->name ( node_definition => $monitor_ipport )
                            );

           $count++ unless $self->inerror($soap_response);
       }
    };

    return $count;

};

#===============================================================================

sub deleteMonitorFromServices { # Disassociates monitor from services

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a list of service ports <port> numbers to delete its monitor
# returns:  number of nodes that had successfully deleted its monitor
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self     = shift;
    my @services = @_    unless scalar @_ == 0;

    die "BIGIP::deleteMonitorFromServices only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::deleteMonitorFromServices error, missing service port(s)"  unless @services;

    my $count = 0;
    my $soap_response;

    foreach $service (@services)
    {
       if ($service =~ /$SERVFMT/)
       {
           my $ipport_definition   = { address => "*", port => "$service" };
           my $monitor_ipport      = { address_type => ATYPE_STAR_ADDRESS_EXPLICIT_PORT
                                     , ipport       => $ipport_definition
                                     };
           $soap_response = $self->{iCTLMonitor2} ->delete_association
                            ( SOAP::Data->name ( node_definition => $monitor_ipport )
                            );

           $count++ unless $self->inerror($soap_response);
       }
    };

    return $count;

};

#===============================================================================

sub listVirtualServers { # Return list of Virtual Servers

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  void
# returns:  a list of virtual servers <ip>:<port> addresses
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self = shift;

    die "BIGIP::listVirtualServers only applicable for iControl interface" unless $self->{interface} eq 'iControl';


    my $soap_response;
    my @members = qw();
    $soap_response = $self->{iCTLVirtual} ->get_list();
    my @IPPortDefs = @{$soap_response->result} unless $self->inerror($soap_response);
    foreach $IPPortDef (@IPPortDefs)
    {
       push(@members,$IPPortDef->{address} . ":" . $IPPortDef->{port})
    };

    return sort @members;
};

#===============================================================================

sub getPoolFromVirtualServers { # Get each Pool associated with Virtual Server(s)

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a list of virtual servers <ip>:<port> addresses
# returns:  a list with the corresponding pools associated with the virtual servers
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self     = shift;
    my @virtuals = @_ unless scalar @_ == 0;

    die "BIGIP::getPoolFromVirtualServers only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::getPoolFromVirtualServers error, missing Virtual Servers IP/Port" unless @virtuals;


    my @pools = qw();
    my $soap_response;

    foreach $virtual (@virtuals)
    {
       if ($virtual =~ /$NODEFMT/)
       {
           my $ipport_definition   = { address => "$1.$2.$3.$4", port => "$5" };
           $soap_response = $self->{iCTLVirtual} ->get_pool
                            ( SOAP::Data->name ( virtual_server => $ipport_definition )
                            );

           my $pool = qw();
           $pool = $soap_response->result unless $self->inerror($soap_response);
           push(@pools,$pool);
       }
    };

    return @pools;

};
#===============================================================================

sub getVirtualServersFromPool { # Get Virtual Servers that the pool is associated with

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a a pool name
# returns:  a list with the corresponding virtual servers <ip>:<port> addresses
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self     = shift;
    my $pool     = shift unless scalar @_ == 0;

    die "BIGIP::getVirtualServersFromPool only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::getVirtualServersFromPool error, missing Virtual Servers IP/Port" unless $pool;


    my @virtuals = qw();
    my @virtuals_list = $self->listVirtualServers();
    my $soap_response;

    foreach $virtual (@virtuals_list)
    {
       if ($virtual =~ /$NODEFMT/)
       {
           my $ipport_definition   = { address => "$1.$2.$3.$4", port => "$5" };
           $soap_response = $self->{iCTLVirtual} ->get_pool
                            ( SOAP::Data->name ( virtual_server => $ipport_definition )
                            );

           my $search_pool = qw();
           $search_pool = $soap_response->result unless $self->inerror($soap_response);
           push(@virtuals,$virtual) if ($search_pool eq $pool);
       }
    };

    return @virtuals;

};

#===============================================================================

sub getVirtualServersFromPools { # Get pairs Virtual Servers and pools for pools

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a a pool name
# returns:  a list with the corresponding virtual servers <ip>:<port> addresses
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self     = shift;
    my @pools    = @_ unless scalar @_ == 0;


    die "BIGIP::getVirtualServersFromPool only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::getVirtualServersFromPool error, missing Virtual Servers IP/Port" unless @pools;


    my %pool_map = ();
    my @virtual_pool_pairs = qw();
    my $soap_response;

    $pool_map{$_} = 1 foreach (@pools);
    my @virtuals_list = $self->listVirtualServers();

    foreach $virtual (@virtuals_list)
    {
       if ($virtual =~ /$NODEFMT/)
       {
           my $ipport_definition   = { address => "$1.$2.$3.$4", port => "$5" };
           $soap_response = $self->{iCTLVirtual} ->get_pool
                            ( SOAP::Data->name ( virtual_server => $ipport_definition )
                            );

           my $pool = qw();
           $pool = $soap_response->result unless $self->inerror($soap_response);
           push(@virtual_pool_pairs, $virtual, $pool) if ($pool_map{$pool});
       }
    };

    return @virtual_pool_pairs;

};

#===============================================================================

sub assignPoolToVirtualServers { # Assigns one pool to several virtual servers

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a pool name followed by a list of virtual servers <ip>:<port> addresses
# returns:  number of Virtual Servers that was successfully assigned to the poo1
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   my $self     = shift;
   my $pool     = shift unless scalar @_ == 0;
   my @virtuals = @_    unless scalar @_ == 0;

   die "BIGIP::assignPoolToVirtualServers only applicable for iControl interface" unless $self->{interface} eq 'iControl';
   die "BIGIP::assignPoolToVirtualServers error, missing Pool Name"               unless $pool;
   die "BIGIP::assignPoolToVirtualServers error, missing Virtual Servers IP/Port" unless @virtuals;

   my $count = 0;
   my $soap_response;

   foreach $virtual (@virtuals)
   {
      if ($virtual =~ /$NODEFMT/)
      {
          my $ipport_definition   = { address => "$1.$2.$3.$4", port => "$5" };
          $soap_response = $self->{iCTLVirtual} ->set_pool
                           ( SOAP::Data->name ( virtual_server => $ipport_definition )
                           , SOAP::Data->name ( pool_name      => $pool )
                           );
          $count++ unless $self->inerror($soap_response);
      }
   };


   return $count;

};

#===============================================================================

sub disableVirtualServers {  # disable Virtual Servers to receive transactions

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a list of virtual servers <ip>:<port> addresses
# returns:  1 if success, 0 on failure
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   my $self = shift;
   my (@virtuals) = @_;   # Virtual format is <ip-address>:<port>

   die "BIGIP::disableVirtualServers only applicable for iControl interface" unless $self->{interface} eq 'iControl';

   my @members = ();
   foreach $virtual (@virtuals)
   {
      if ($virtual =~ /$NODEFMT/)
      {
          my $ipport_definition = { address => "$1.$2.$3.$4", port => "$5" };
          push(@members,$ipport_definition);
      }
   };

   my $soap_response = $self->{iCTLVirtual} ->set_state
                  ( SOAP::Data->name (virtual_servers => [@members])
                  , SOAP::Data->name (state     => DISABLED)
                  );

   return 1 unless $self->inerror($soap_response);
   return 0;

}

#===============================================================================

sub enableVirtualServers {  # enable Virtual Servers to receive transactions

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a list of virtual servers <ip>:<port> addresses
# returns:  1 if success, 0 on failure
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   my $self = shift;
   my (@virtuals) = @_;   # Node format is <ip-address>:<port>

   die "BIGIP::enableVirtualServers only applicable for iControl interface" unless $self->{interface} eq 'iControl';

   my @members = ();
   foreach $virtual (@virtuals)
   {
      if ($virtual =~ /$NODEFMT/)
      {
          my $ipport_definition = { address => "$1.$2.$3.$4", port => "$5" };
          push(@members,$ipport_definition);
      }
   };

   my $soap_response = $self->{iCTLVirtual} ->set_state
                  ( SOAP::Data->name (virtual_servers => [@members])
                  , SOAP::Data->name (state     => ENABLED)
                  );

   return 1 unless $self->inerror($soap_response);
   return 0;

}

#===============================================================================

sub getStatusOfVirtualServers {  # get status of Virtual Servers

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a list of virtual servers <ip>:<port> addresses
# returns:  a list of correspondent status names transated according to %StatusMap
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   my $self = shift;
   my (@virtuals) = @_;   # Virtual Server format is <ip-address>:<port>

   die "BIGIP::getStatusOfVirtualServers only applicable for iControl interface" unless $self->{interface} eq 'iControl';

   my @members  = ();
   my @status_names = ();

   foreach $virtual (@virtuals)
   {
      if ($virtual =~ /$NODEFMT/)
      {
          my $ipport_definition = { address => "$1.$2.$3.$4", port => "$5" };
          push(@members,$ipport_definition);
      }
   };

   my $soap_response = $self->{iCTLVirtual} ->get_status
                  ( SOAP::Data->name (virtual_servers => [@members])
                  , SOAP::Data->name (state     => ENABLED)
                  );

   my @status_codes = @{$soap_response->result} unless $self->inerror($soap_response);

   foreach (@status_codes) {
      push(@status_names, $StatusMap{$_});
   };

   return @status_names;


}

#===============================================================================

sub getKeysFromDB {  # Get value of each of db Keys

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  The DB Name followed by a list of DBKey names
# returns:  a list of pairs of DBKey values corresponding with the requested ones
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   my $self = shift;
   (my $DBName, my @keys) = @_;   # Keys
   $DBName = $self->defaultDB() unless $DBName;

   die "BIGIP::getKeysFromDB only applicable for iControl interface" unless $self->{interface} eq 'iControl';

   my @key_values = ();


   foreach $key (@keys)
   {
       my $soap_response = $self->{iCTLBigdb} ->get_key
                      ( SOAP::Data->name (db_name  => $DBName)
                      , SOAP::Data->name (key_name => $key)
                      );
       my $key_value = '';
       $key_value = $soap_response->result unless $soap_response->fault;
       push(@key_values, fixString($key_value));
   };

   return @key_values;


}

#===============================================================================

sub getDBKeys {  # Get value of each of db Keys

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a list of DBKey names
# returns:  a list of pairs of DBKey values corresponding with the requested ones
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   my $self = shift;
   (my @keys) = @_;   # Keys

   die "BIGIP::getDBKeys only applicable for iControl interface" unless $self->{interface} eq 'iControl';

   my @responses = $self->getKeysFromDB('',@keys);
   return @responses;

};

#===============================================================================

sub getPeerIPAddress () { # Returns BIGIP peer IP address

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  void
# returns:  The peer BIGIP internal IP Address
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   my $self = shift;
   (my $peerIP) = $self->getDBKeys('Local.Bigip.StateMirror.PeerIPAddr');

   return $peerIP unless $peerIP eq $self->{ipaddr};
   return '';


}


#===============================================================================

sub searchKeysFromDB {  # Get value of each of db Keys found using Regular Expression

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a regular expression, an options DBName
# returns:  a list of pairs of DBKey names and values in the form <name>:<value>
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   my $self = shift;
   (my $regular_expression, my $DBName) = @_;   # Keys
   $regular_expression = '*'    unless $regular_expression;
   $DBName = $self->defaultDB() unless $DBName;

   die "BIGIP::searchKeysFromDB only applicable for iControl interface" unless $self->{interface} eq 'iControl';

   my $soap_response = $self->{iCTLBigdb} ->get_regex_keys
                       ( SOAP::Data->name (db_name        => $DBName)
                       , SOAP::Data->name (reg_expression => $regular_expression)
                       );
   my @key_namevalues = @{$soap_response->result} unless ($soap_response->fault);

   my @responses = qw();
   my $key_namevalue = qw();

   foreach $key_namevalue (@key_namevalues)
   {
       my $key_name  =   &fixName($key_namevalue->{'key_name'});
       my $key_value = &fixString($key_namevalue->{'key_value'});
       my $response  = $key_name . '=' .  "'" . $key_value . "'" . ';' . "\n";

       push(@responses,$response);
   };

   return @responses;


};

#===============================================================================

#===============================================================================
sub swapMonitorsAttributes {  # Swaps Interval and Timeout between 2 monitors

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a list of exactly two (2) monitor template names
# returns:  1 if success, 0 on failure
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   my $self = shift;
   my @monitors = @_;

   die "BIGIP::swapMonitorsAttributes only applicable for iControl interface" unless $self->{interface} eq 'iControl';
   die "BIGIP::swapMonitorsAttributes accepts only 2 monitors"                unless scalar @_ == 2;

   my @intervals = qw();
   my @timeouts  = qw();

   my $count = 0;
   foreach my $monitor ( @monitors )
   {
        $intervals[$count] = $self->getMonitorInterval($monitor);
         $timeouts[$count] =  $self->getMonitorTimeout($monitor);

        printm "Monitor: %s {Interval %d, Timeout %d}\n",
               $monitor,
               $intervals[$count],
               $timeouts[$count]
               if $self->verbose()
               ;
        $count++;
   };

   # Swap monitor Intervals and Timeout if values are different
   my $rc = 0;
   $count = 0;
   if ($intervals[0] != $intervals[1])
   {
        $count += $self->setMonitorInterval($monitors[0], $intervals[1]);
        $count += $self->setMonitorInterval($monitors[1], $intervals[0]);

   };

   $rc += $count;
   $count=0;
   if ($timeouts[0] != $timeouts[1])
   {
        $count += $self->setMonitorTimeout($monitors[0], $timeouts[1]);
        $count += $self->setMonitorTimeout($monitors[1], $timeouts[0]);
   };

   $rc += $count;
   $count=0;
   foreach my $monitor ( @monitors )
   {
        $intervals[$count] = $self->getMonitorInterval($monitor);
         $timeouts[$count] =  $self->getMonitorTimeout($monitor);

        printm "Monitor: %s {Interval %d, Timeout %d} (after swap)\n",
               $monitor,
               $intervals[$count],
               $timeouts[$count]
               if $self->verbose()
               ;
        $count++;
   };

   return $rc;

};

#===============================================================================
sub swapPoolsBetweenVirtualServers {  # Swaps the pools between Virtuals x and y

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a list of exactly two (2) virtual addresses in the form <ip>:<port>
# returns:  1 if success, 0 on failure
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   my $self = shift;
   my @virtuals = qw();
   ($virtuals[0], $virtuals[1]) = @_;   # Keys

   die "BIGIP::swapPoolsBetweenVirtualServers only applicable for iControl interface" unless $self->{interface} eq 'iControl';
   die "BIGIP::swapPoolsBetweenVirtualServers accepts only 2 virtual servers IP/PORT addresses" unless scalar @_ == 2;

   my @pools = $self->getPoolFromVirtualServers(@virtuals);
   printm "Virtual addresses %s and %s will have their respective pool %s and %s swapped to %s and %s\n",
           @virtuals, @pools, $pools[1], $pools[0]
           if $self->verbose()
           ;

   my $count = 0;
   $count += $self->assignPoolToVirtualServers($pools[1],$virtuals[0]);
   $count += $self->assignPoolToVirtualServers($pools[0],$virtuals[1]);

   return ($count == 2);

};


#===============================================================================

sub switchVirtualServers {   # Do the whole switching between virtuals

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a list of exactly two (2) virtual addresses in the form <ip>:<port>
# returns:  1 if success, 0 on failure
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
# *IMPORTANT*  The first Virtual address must represent current state
#              The second Virtual address must be the state we are going to
#              This is necessary to be able to detect if the monitors need to
#              be swapped along with the pools
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   my $self = shift;
   my @virtuals = @_;

   die "BIGIP::switchVirtualServers only applicable for iControl interface" unless $self->{interface} eq 'iControl';
   die "BIGIP::switchVirtualServers accepts only 2 virtual servers IP/PORT addresses" unless scalar @_ == 2;

   my @nodes = qw();

   my @pools = $self->getPoolFromVirtualServers(@virtuals);

   foreach my $pool (@pools)
   {
      my @nodesFromPool = $self->listNodesFromPool($pool);
      push(@nodes,$nodesFromPool[1]) if scalar @nodesFromPool > 1;
   };


   if ($self->swapPoolsBetweenVirtualServers(@virtuals))
   {
      my @monitors = $self->listMonitorAssociationOfNodes(@nodes);
      if ((scalar @monitors == 2) and ($self->getMonitorInterval($monitors[0]) < $self->getMonitorInterval($monitors[1])))
      {
      if ($self->swapMonitorsAttributes(@monitors))
      {
         printm "Swap of monitors %s completed OK\n", join(' and ',@monitors)
          if $self->verbose();
      }
      else
      {
         printm "**ERROR** during SWAP of monitor attributes %s\n", join(' ',@monitors);
         return 0;
      };
      };
   }
   else
   {
      printm "**ERROR** during SWAP of virtual servers %s\n", join(' ',@virtuals);
      return 0;
   };

   return 1;

};



#===============================================================================

sub switchPools {   # Do the whole switching between pools

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a list of exactly two (2) pool names, `current` and `to switch to`
# returns:  1 if success, 0 on failure
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
# *IMPORTANT*  The first Pool must represent current state
#              The second Pool must be the state we are going to
#              This is necessary to be able to detect if the monitors need to
#              be swapped along with the pools
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   my $self = shift;
   my @pools = @_;

   die "BIGIP::switchPools only applicable for iControl interface" unless $self->{interface} eq 'iControl';
   die "BIGIP::switchPools accepts only 2 pools" unless scalar @_ == 2;

   my @nodes = qw();

   my @virtuals_from = qw();
   my @virtuals_to   = qw();
   my $soap_response;

   my @virtuals_list = $self->listVirtualServers();

# Sample a node from both pools for future reference to a monitor template
   foreach my $pool (@pools)
   {
      my @nodesFromPool = $self->listNodesFromPool($pool);
      push(@nodes,$nodesFromPool[1]) if scalar @nodesFromPool > 1;
   };


# grep from all VirtualxPool pairs and build the list of virtuals for both pools
   my $pool = qw();

   foreach my $virtual (@virtuals_list)
   {
      if ($virtual =~ /$NODEFMT/)
      {
          my $ipport_definition   = { address => "$1.$2.$3.$4", port => "$5" };
          $soap_response = $self->{iCTLVirtual} ->get_pool
                           ( SOAP::Data->name ( virtual_server => $ipport_definition )
                           );

          $pool = $soap_response->result unless $self->inerror($soap_response);
          push(@virtuals_from, $virtual) if ($pool eq $pools[0]);
          push(@virtuals_to,   $virtual) if ($pool eq $pools[1]);
      }
   };

# Reassign the virtuals to the checkpoint pool
   my $count = 0;
   $count = $self->assignPoolToVirtualServers($pools[1],@virtuals_from);
   if ($count == 0)
   {
      printm "**ERROR** assigning Pool %s to Virtuals %s\n", $pools[1], join(' ',@virtuals_from);
      return 0
   };
   $count = $self->assignPoolToVirtualServers($pools[0],@virtuals_to);
   if ($count == 0)
   {
      printm "**ERROR** assigning Pool %s to Virtuals %s\n", $pools[0], join(' ',@virtuals_to);
      return 0
   };

# Change monitor template attributes when this is the case (duplicate changes are avoided)
      my @monitors = $self->listMonitorAssociationOfNodes(@nodes);
      if ((scalar @monitors == 2) and ($self->getMonitorInterval($monitors[0]) < $self->getMonitorInterval($monitors[1])))
      {
          if ($self->swapMonitorsAttributes(@monitors))
          {
             printm "Swap of monitors %s completed OK\n", join(' and ',@monitors)
              if $self->verbose();
          }
          else
          {
             printm "**ERROR** during SWAP of monitor attributes %s\n", join(' ',@monitors);
             return 0;
          };
      };

   return 1;

};


#===============================================================================

sub printVirtualServers {   # Do the whole switching between virtuals

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a list of virtual addresses in the form <ip>:<port>
# returns:  1 if success, 0 on failure
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   my $self = shift;
   my @virtuals = @_;

   die "BIGIP::printVirtualServers only applicable for iControl interface" unless $self->{interface} eq 'iControl';
   die "BIGIP::printVirtualServers accepts a lists of virtual servers IP/PORT addresses" unless scalar @_ > 0;

   my @monitors = qw();

   my @pools = $self->getPoolFromVirtualServers(@virtuals);

   foreach my $pool (@pools)
   {
      my @nodesFromPool = $self->listNodesFromPool($pool);
      if (scalar @nodesFromPool > 1)
         {
             (my $monitor) = $self->listMonitorAssociationOfNodes($nodesFromPool[1]);
             push(@monitors,$monitor);
         }
      else
         {
             push(@monitors,undef);
         };
   };


   for (my $i=0; $i < scalar @virtuals; $i++ )
   {
      printm "%s\n", '-' x 80;
      printm "Virtual Server %s:\n", $virtuals[$i];
      my $virtual       = $virtuals[$i];
      my $pool          = $pools[$i];
      my @nodesFromPool = $self->listNodesFromPool($pool);
      if ($monitors[$i])
      {
      my $monitor       = $monitors[$i];
      my $interval      = $self->getMonitorInterval($monitor);
      my $timeout       = $self->getMonitorTimeout($monitor);
      printm "   Pool: %s, Monitor: %s {Timeout %d Interval %d}\n",
             $pools[$i], $monitors[$i], $interval, $timeout;
      }
      else
      {
          printm "   Pool: %s, Not monitored\n", $pools[$i];
      };
      printm "   Nodes are: (%s)\n", join(", ", @nodesFromPool);

   };
   printm "%s\n", '-' x 80;

   return 1;

};


#===============================================================================

sub changeNodePriority { # Changes the priority of node(s)

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a pool name, a priority number and a list of nodes <ip>:<port>
#      returns:  1 if success, 0 on failure
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self     = shift;
    my $pool     = shift unless scalar @_ == 0;
    my $priority = shift unless scalar @_ == 0;
    my @nodes    = @_    unless scalar @_ == 0;

    die "BIGIP::changeNodePriority only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::changeNodePriority error, missing Pool Name"               unless $pool;
    die "BIGIP::changeNodePriority error, missing priority "               unless $priority;
    die "BIGIP::changeNodePriority error, missing nodes "                  unless @nodes;

    my @members = qw();
    foreach $node (@nodes)
     {
        if ($node =~ /$NODEFMT/)
        {
            my $node_definition = { address => "$1.$2.$3.$4", port => "$5" };
            my $member_priority = { member_def => $node_definition, priority => $priority };
            push(@members, $member_priority);
        }
     };
     my $soap_response = $self->{iCTLPool} ->set_member_priorities
                       ( SOAP::Data->name ( pool_name => $pool )
                       , SOAP::Data->name ( member_priorities => [@members] )
                       );

     return 1 unless $self->inerror($soap_response);
     return 0;

 };

#===============================================================================

sub setPoolAscendingNodePriorities { # sets node priorities to ascending values

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a pool name, a priority number at which to start and a list of
#           nodes <ip>:<port>
#      returns:  1 if success, 0 on failure
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self     = shift;
    my $pool     = shift unless scalar @_ == 0;
    my $priority = shift unless scalar @_ == 0;
    my @nodes    = @_    unless scalar @_ == 0;

    die "BIGIP::changeNodePriority only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::changeNodePriority error, missing Pool Name"               unless $pool;
    die "BIGIP::changeNodePriority error, missing priority "               unless $priority;
    die "BIGIP::changeNodePriority error, missing nodes "                  unless @nodes;

    my @members = qw();
    foreach $node (@nodes)
     {
        if ($node =~ /$NODEFMT/)
        {
            my $node_definition = { address => "$1.$2.$3.$4", port => "$5" };
            my $member_priority = { member_def => $node_definition, priority => $priority };
            push(@members, $member_priority);
            $priority++;
        }
     };
     my $soap_response = $self->{iCTLPool} ->set_member_priorities
                       ( SOAP::Data->name ( pool_name => $pool )
                       , SOAP::Data->name ( member_priorities => [@members] )
                       );

     return 1 unless $self->inerror($soap_response);
     return 0;

 };

#===============================================================================

sub changeNodeConnectionLimit { # Changes the connection limit of node(s)

# Parameters: - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#      in:  a connection limit, and a list of nodes <ip>:<port>
#      returns:  1 if success, 0 on failure
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    my $self  = shift;
    my $limit = shift unless scalar @_ == 0;
    my @nodes = @_    unless scalar @_ == 0;

    die "BIGIP::changeNodeConnectionLimit only applicable for iControl interface" unless $self->{interface} eq 'iControl';
    die "BIGIP::changeNodeConnectionLimit error, missing nodes "                  unless @nodes;

    my @members = ();
    foreach $node (@nodes)
    {
       if ($node =~ /$NODEFMT/)
       {
           my $node_definition = { address => "$1.$2.$3.$4", port => "$5" };
           push(@members,$node_definition);
       }
    };

    my $soap_response = $self->{iCTLNode} ->set_limit
                   ( SOAP::Data->name (node_defs => [@members])
                   , SOAP::Data->name (limit     => $limit)
                   );

    return 1 unless $self->inerror($soap_response);
    return 0;

 };

#===============================================================================

return 1;
