# try read instead of sysread
package Sabre::ShoppingProto;

our $VERSION = 0.2;

use strict;
use warnings;

use Carp qw(croak confess cluck);
use IO::Socket;

use constant { PROTO_VERSION => '2.0',
               HEADER_SIZE   => 12 };

our $verbosity = 1;

sub new {
  
  my $class = shift;
  my $self = { _body => undef, # text 
               _sock => undef, # IO::Socket::INET for syswrite, sysread
               };
               
  $self->{_target} = shift 
    or 
    croak 'ShoppingProto::new($target) requires the argument as "host:port"';
    
  # options hash is optional, -write-header from it is used only
  $self->{_opt} = shift || {};

  bless  $self => $class;
  return $self;
}

sub setRequestBody {
  
  my $self = shift;

  $self->{_body} = shift;
  $self->_parameterizeBody(@_);
}

sub _parameterizeBody {
  
  my $self = shift;
  my %bodyParam = @_;
  
  # parameterize client transaction id
  $self->{_body} =~ s/C01="\d+"/C01="$bodyParam{-client_tid}"/
    if $bodyParam{-client_tid};
  
  # parameterize timeout
  if( $bodyParam{-timeout} ) {
    eval {

      my $shoppingRequestTagRegEx = qr{<ShoppingRequest.+?/>}s; # "." matches "\n" as well
    
      my ($shoppingRequestTag) = $self->{_body} =~ m/($shoppingRequestTagRegEx)/
        or die "regex failed";
      
      if( $shoppingRequestTag =~ m/TST="Y"/ ) {
        
        $shoppingRequestTag =~ s/D70="\d+"/D70="$bodyParam{-timeout}"/
          or die "regex failed";
      } else {
        
        $shoppingRequestTag =~ s/D70="\d+"/D70="$bodyParam{-timeout}" TST="Y"/
          or die "regex failed";
      }
      
      $self->{_body} =~ s/$shoppingRequestTagRegEx/$shoppingRequestTag/ 
        or die "regex failed";
    }; #eval
    
    warn "Cannot adjust timeout: $@\n" if $@ =~ m/^regexp failed/; 
  } #if -timeout 
}

sub send {
  
  my $self = shift;
  my $response;
  
  $self->_connect();
  $self->_sendHeader();
  $self->_sendBody();
  $response = $self->_recvAll();
  $self->_disconnect();
  
  return $response;
}

sub _disconnect { close shift->{_sock} }

sub _connect {
  
  my $self = shift;
  
  $self->{_sock} = new IO::Socket::INET(PeerAddr => $self->{_target},
                                        Proto    => "tcp",
                                        Blocking => 1)
    or
    die "Could not connect target $self->{_target}: $!\n";
}

sub _sendHeader {
  
  my $self   = shift;
  my $header = pack('NA4A3x', 
                    HEADER_SIZE + length $self->{_body},
                    'RQST',
                    PROTO_VERSION);
                    
  confess "Illegal header" 
    if length($header) != HEADER_SIZE;
                    
  syswrite($self->{_sock}, $header, HEADER_SIZE)
    == HEADER_SIZE
    or cluck "Incomplete syswrite";
}

sub _recvHeader {

  my $self = shift;
  
  my $header;
  if( sysread($self->{_sock}, $header, HEADER_SIZE) != HEADER_SIZE ) {
    
    cluck "Incomplete sysread";
    return undef;
  };

  my ($responseLength, $cmd, $version) = unpack('NA4A3', $header);
  my $bodyLength = $responseLength - HEADER_SIZE;
  
  if( $self->{_opt}->{-write_header} ) {
    
    print STDERR "header:responseBodyLength=$bodyLength\n" 
               . "header:cmd=$cmd\n" 
               . "header:version=$version\n";
  }
  
  return $bodyLength;
}

sub _sendBody {
  
  my $self = shift;
  
  syswrite($self->{_sock}, $self->{_body}) 
    == length($self->{_body})
    or cluck "Incomplete syswrite";
}

sub _recvAll {
  
  my $self = shift;
  
  my $bodyLength = $self->_recvHeader();
  return undef if !$bodyLength;
  
  # as far sysread somehow falls to unblock mode, here is a loop as a workaround
  my $body = '';
  for(my ($leftToRead, $read, $buf) = ($bodyLength, 0, '')
      ;
      $leftToRead > 0
      &&
      ($read = sysread($self->{_sock}, $buf, $leftToRead))
      ;
      $leftToRead -= $read)
  {
    $body .= $buf;
  }
  
  if( length($body) != $bodyLength ) {

    cluck sprintf("Incomplete response body read (%d/%d)", 
                  length($body), 
                  $bodyLength);
  }
  
  return $body;
}

1;
