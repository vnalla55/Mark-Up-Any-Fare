package Sabre::Bump;
require Exporter;

our @ISA = qw(Exporter);
our @EXPORT = qw/bump/;

our $VERSION = 0.2;

use strict;
use warnings;

use v5.8.8;
use threads;
use threads::shared;

use Carp qw/croak confess cluck/;
use English;
use Time::HiRes;

our $verbose = 1;

# helper stand-alone function
sub join_all(@) {

  my @pool = @_;

  # wait threads to complete
  if( $PERL_VERSION ge v5.10.1 ) { # is better try can('is_running')

    while(my $count = grep($_->is_running() => @pool)) {

      print STDERR "waiting for $count background requests to complete...\n";
      sleep(1);
    }
  } else {
    printf STDERR ("waiting for %d background requests to complete...\n", scalar @pool);
  }

  # join all, ignore errors on detached threads
  eval{ $_->join() } for (@pool);
}

# helper stand-alone function
sub atom_inc(\[$%];$) {

  my $ref = shift;
  my $hvar = shift;

  if( $hvar ) {
    lock(%$ref);
    ++$ref->{$hvar};
  } else {
    lock($$ref);
    ++$$ref;
  }
}

# helper stand-alone function
sub atom_dec(\[$%];$) {

  my $ref = shift;
  my $hvar = shift;

  if( $hvar ) {
    lock(%$ref);
    --$ref->{$hvar};
  } else {
    lock($$ref);
    --$$ref;
  }
}

sub _new {

  my $class  = shift;
  my %hash :shared = ();
  my $self :shared = \%hash;

  $self->{_public_foo} = shift
    or croak 'foo is required as the 1st arg';
  defined($self->{_bps} = shift) # defined() is use as for 0 as special case
    or croak 'bps is required as the 2nd arg';
  defined($self->{_max_requests} = shift)
    or croak 'max requests arg is required as the 3rd arg';

  $self->{_impl} = shift; # which implementation to use, see _run(), optional

  $self->{_reqs_sent}      = 0;
  $self->{_reqs_completed} = 0;
  $self->{_real_bps}       = 0;

  my %_print_stat :shared  = ( reqs_sent => 0,
                               reqs_completed => 0 );
  $self->{_print_stat} = \%_print_stat;

  bless  $self => $class;
  return $self;
}

# we are single-threaded
sub _bumpInSequence {

  my $self = shift;

  do {

    $self->{_reqs_sent}++;
    $self->_callFoo();
    $self->{_reqs_completed}++;

    $self->_print_stat();

  } for(1..$self->{_max_requests});
}

sub _bumpImpl_1 {

  my $self = shift;
  my $sleep_time = 1 / $self->{_bps}; # assume that time for thread->new ≈ 0

  my %async_pool :shared;
  while($self->{_reqs_sent} < $self->{_max_requests})
  {
    my $t_beforeSplit = Time::HiRes::time();
    my $th = async {

      do{ lock(%async_pool);
          $async_pool{ threads->tid() } = 1; };

      printf STDERR ("async thread creation took %.3f seconds\n",
                     Time::HiRes::time() - $t_beforeSplit);
      $self->_callFoo();

      lock($self);
      $self->{_reqs_completed}++;

      # child has to clean up after itself
      threads->detach();

        lock(%async_pool);
      delete $async_pool{ threads->tid() };
    };

    if ($th) {
      $self->{_reqs_sent}++;
    } else {
      cluck "Boo: $!" unless $th;
    }

    Time::HiRes::sleep($sleep_time);

    my $pool_size;
    do{ lock(%async_pool);
        $pool_size = keys %async_pool; };
    $self->_print_stat($pool_size);
  }

  my $shapshot = sub{ lock(%async_pool);
                      map(threads->object($_), keys %async_pool); }; #thread-safety trick!
  join_all(&$shapshot);
}

sub _bumpImpl_2 {

  require Thread::Queue;

  my $self = shift;
  my $queue = Thread::Queue->new();

  my $num_workers = $self->{_bps}*2; # TODO rework this magic number
  print STDERR "spawning $num_workers threads...";

  my @pool;
  my $num_running :shared = 0;
  do {

    # spawn worker threads
    my $th = threads->new(
      sub{

      while(my $continue_flag = $queue->dequeue()) {

        atom_inc(%$self => '_reqs_sent');

        atom_inc($num_running);
        $self->_callFoo();
        atom_dec($num_running);

        atom_inc(%$self => '_reqs_completed');

      } #while

      printf STDERR ("thread %s: got stop flag\n", threads->tid())
        if $Sabre::Bump::verbose;
    });
    if ($th) {
      push @pool, $th;
      print STDERR ".";
    } else {
      cluck "\nBoo: $!\n";
    }
  } for(1..$num_workers);
  print STDERR  "\n";

  for(my $i = 0;
      $i < $self->{_max_requests}
      &&
      ( !threads->can('is_running') || grep($_->is_running() => @pool) )
      ; )
  {
    $queue->enqueue(1); # feed threads

    # with pauses
    if( ++$i % $self->{_bps} == 0 ) {
      $self->_print_stat($num_running);
      sleep(1);
    }
  }

  # feed stop flag
  $queue->enqueue(0) for(@pool);

  join_all(@pool);
}

{
# in main thread:
# prints statistics no more than once per sec, to do not flood the output

my $last_effective_call_t :shared = time();

sub _print_stat {

  my $self = shift;
  my $num_running = shift; # requests are running at the moment, optional

  my $t_now  = time();
  my $t_span = $t_now - $last_effective_call_t;
  return if $t_span < 1;

  my $prev_stat = $self->{_print_stat};
  return if ($self->{_reqs_sent} ==  $prev_stat->{reqs_sent})
             &&
            ($self->{_reqs_completed} ==  $prev_stat->{reqs_completed});

  my @clauses;
  my $w = length("{$self->{_max_requests}}");
  push(@clauses, sprintf("requests sent: %2d/s (overall: %${w}d/%d)",
                         ($self->{_reqs_sent} - $prev_stat->{reqs_sent}) / $t_span,
                          $self->{_reqs_sent} , $self->{_max_requests}
                        ));
  push(@clauses, sprintf("requests completed: %2d/s (overall: %${w}d/%d)",
                         ($self->{_reqs_completed} - $prev_stat->{reqs_completed}) / $t_span,
                          $self->{_reqs_completed} , $self->{_max_requests}
                        ));
  push(@clauses, "running: $num_running") if defined($num_running);

  print STDERR join(', ' => @clauses), "\n";

  # roll up
  $prev_stat->{reqs_sent}      = $self->{_reqs_sent};
  $prev_stat->{reqs_completed} = $self->{_reqs_completed};
  $last_effective_call_t       = $t_now;
}}

sub _run() {

  my $self = shift;

  if( $self->{_bps} == 0 ) {
    $self->_bumpInSequence();
  } elsif( $self->{_impl} == 1 ) {
    $self->_bumpImpl_1();
  } else {
    $self->_bumpImpl_2();
  }
}

sub _callFoo() {

  no strict 'refs';

  eval{ shift()->{_public_foo}->(); };
  if( $@ ) {
    if( $@ =~ /Oops/ ) {
      die "$@\n";
    } else {
      print STDERR $@ if $@;
    }
  }
}

# stand-alone
sub bump($$$;$) {

  Sabre::Bump->_new(@_)->_run();
}

1;
