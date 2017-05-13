require Gen::Word;

############################################### Writer
# Writer is a class representing chunk of code lines.
#
# You can write lines one after another using writeLine/writeCode.
# You can also nest writers within each other, creating tree of writers. This way, you can
# write "in the middle of writer", by preparing sub writer beforehand and writing to it afterwards.
#
# Use addCallback to create a new CallbackWriter. More about it below.
#
# {push,pop}Indent are used to change current indent of code. {push,pop}Block enclose C++ block,
# i.e. '{' ... '}', also with automatic indent.
#
# *NewLine methods are used to control new-line placement.
# set{Begin,End}NewLine(1) control whether new line should be placed before or after writer. These
# functions just set state of writer.
# Enclosing writer can eat these new-lines, using eat{Next,Previous}NewLine. These must be used
# before/after offending writer(s). You can also use eatEndNewLine(1) to set a state that
# automatically eat last pending new-line.
#
# Every writer can be skipped. It's controlled using skipCallback, the default being
# skip-when-empty. Skipped writer doesn't contribute to new-line control logic described above;
# It's treated as it never existed.
#
# Lines consist of Words, which are used for wrap control; more about them in Word.pm.

package Gen::Writer;
use strict;
use warnings;
use Storable qw(dclone);
use Scalar::Util qw(blessed reftype);
use Gen::Util qw(error);

our $NEW_LINE_NONE = 0;
our $NEW_LINE_BEGIN = 1 << 0;
our $NEW_LINE_END = 1 << 1;
our $NEW_LINE_EAT_BEGIN = 1 << 2;
our $NEW_LINE_EAT_END = 1 << 3;

our $OUTPUT_WIDTH = 100;

# Enable or disable debug output, showing the Writer tree.
our $DEBUG_OUTPUT = 0;

sub new
{
  my ($class, $name) = @_;
  my $self =
  {
    name           => (defined $name ? $name : ''),
    lines          => [],
    lineIndents    => [],
    indent         => 0,
    newLineControl => $NEW_LINE_NONE,
    skipCallback   => \&_skip
  };

  return bless $self, $class;
}

##################################### Utilities

sub clone
{
  my ($self) = @_;
  my $clone = Gen::Writer->new($self->name);

  $clone->{lines} = dclone($self->{lines});
  $clone->{lineIndents} = dclone($self->{lineIndents});
  $clone->{indent} = $self->{indent};
  $clone->{newLineControl} = $self->{newLineControl};
  $clone->{skipCallback} = $self->{skipCallback};

  return $clone;
}

sub forEachWord
{
  my ($self, $callback) = @_;

  foreach my $line (@{$self->{lines}})
  {
    foreach my $word (@{$line})
    {
      $callback->($self, $word);
    }
  }

  return $self;
}

##################################### Getters

sub name
{
  my ($self) = @_;
  return $self->{name};
}

sub empty
{
  my ($self) = @_;
  return @{$self->{lines}} == 0;
}

sub lastLine
{
  my ($self) = @_;
  error('No line written') if $self->empty;
  return ${$self->{lines}}[-1];
}

##################################### Setters

sub setBeginNewLine
{
  my ($self, $state) = @_;
  $self->{newLineControl} |= $NEW_LINE_BEGIN if $state;
  $self->{newLineControl} &= ~$NEW_LINE_BEGIN if !$state;
  return $self;
}

sub setEndNewLine
{
  my ($self, $state) = @_;
  $self->{newLineControl} |= $NEW_LINE_END if $state;
  $self->{newLineControl} &= ~$NEW_LINE_END if !$state;
  return $self;
}

sub eatEndNewLine
{
  my ($self, $state) = @_;
  $self->{newLineControl} |= $NEW_LINE_EAT_END if $state;
  $self->{newLineControl} &= ~$NEW_LINE_EAT_END if !$state;
  return $self;
}

sub setSkipCallback
{
  my ($self, $cb) = @_;
  $self->{skipCallback} = $cb;
  return $self;
}

sub setName
{
  my ($self, $name) = @_;
  $self->{name} = $name;
  return $self;
}

##################################### Write

sub writeLine
{
  my ($self, @words) = @_;

  my $wordsCopy = [];
  $self->_writeLineRecursion($wordsCopy, \@words);

  push @{$self->{lines}}, $wordsCopy;
  push @{$self->{lineIndents}}, $self->{indent};

  return $self;
}

sub writeCode
{
  my ($self, @lines_) = @_;
  my $lines = join("\n", @lines_);
  chomp($lines);
  my @lines = split(/^/, $lines);

  my $firstIndent = -1;
  my $lastIndent = 0;
  foreach my $line (@lines)
  {
    if (!($line =~ /^(\s*)(\S.*)$/))
    {
      $self->writeLine();
      next;
    }

    my $indent = length($1);
    if ($firstIndent == -1)
    {
      $firstIndent = $lastIndent = $indent;
    }
    elsif ($indent != $lastIndent)
    {
      $self->{indent} += ($indent - $lastIndent);
      $lastIndent = $indent;
    }

    $self->writeLine(Gen::Word::Stream->new()->addCode($2));
  }
  $self->{indent} -= ($lastIndent - $firstIndent);

  return $self;
}

sub addSubWriter
{
  my ($self, $writer) = @_;
  error('Internal error') if !$writer->isa('Gen::Writer');

  push @{$self->{lines}}, $writer;
  push @{$self->{lineIndents}}, $self->{indent};

  return $self;
}

sub addCallback
{
  my ($self, $name, $callback) = @_;
  my $writer = Gen::Writer::CallbackWriter->new($name, $callback);
  return $self->addSubWriter($writer);
}

sub eatPreviousNewLine
{
  my ($self) = @_;
  push @{$self->{lines}}, $NEW_LINE_EAT_END;
  push @{$self->{lineIndents}}, 0;
  return $self;
}

sub eatNextNewLine
{
  my ($self) = @_;
  push @{$self->{lines}}, $NEW_LINE_EAT_BEGIN;
  push @{$self->{lineIndents}}, 0;
  return $self;
}

sub pushIndent
{
  my $self = shift;
  my $amount = shift;
  $amount = 2 if !defined $amount;

  $self->{indent} += $amount;
  return $self;
}

sub popIndent
{
  my $self = shift;
  my $amount = shift;
  $amount = 2 if !defined $amount;

  $self->{indent} -= $amount;
  return $self;
}

sub pushBlock
{
  my ($self, $line) = @_;
  return $self
    ->writeLine(defined $line ? $line : '{')
    ->pushIndent()
    ->eatNextNewLine();
}

sub popBlock
{
  my ($self, $line) = @_;
  return $self
    ->eatPreviousNewLine()
    ->popIndent()
    ->writeLine(defined $line ? $line : '}');
}

##################################### Save

sub save
{
  my ($self, $fileName) = @_;

  my $oldHandler = Gen::Util::errorHandler;
  Gen::Util::registerErrorHandler(sub {
    $oldHandler->($fileName.': '.$_[0]);
  });

  open(my $fh, '>', $fileName) || error($!);
  $self->_save($fh);
  close $fh;

  Gen::Util::registerErrorHandler($oldHandler);
}

sub saveToString
{
  my ($self) = @_;

  my $string = '';

  open(my $fh, '>', \$string) || error($!);
  $self->_save($fh);
  close $fh;

  return $string;
}

##################################### Protected save method

sub _saveTo
{
  my ($self, $fh, $indent) = @_;
  my $name = $self->name;

  my $oldHandler = Gen::Util::errorHandler;
  Gen::Util::registerErrorHandler(sub {
    $oldHandler->("\nWriter($name): ".$_[0]);
  });

  my $lines = $self->{lines};
  my $indents = $self->{lineIndents};

  error('Indent is non-zero at the end of writer') if $self->{indent} != 0;
  error('Malformed internal state') if @$lines != @$indents;

  my $eatNextNewLine = 0;
  my $pendingNewLine = 0;
  for (my $i = 0; $i < @$lines; ++$i)
  {
    my $line = $$lines[$i];
    my $localIndent = $indent + $$indents[$i];
    if (!blessed($line))
    {
      if ((reftype($line) || '') eq 'ARRAY')
      {
        print $fh $self->_debugPrefix(' PENDING NL')."\n" if $pendingNewLine;
        $pendingNewLine = 0;
        $eatNextNewLine = 0;

        print $fh $self->_debugPrefix();
        $self->_saveLine($fh, $line, $localIndent)
      }
      else
      {
        if ($line == $NEW_LINE_EAT_BEGIN)
        {
          $eatNextNewLine = 1;
        }
        elsif ($line == $NEW_LINE_EAT_END)
        {
          $pendingNewLine = 0;
        }
        else
        {
          error('Wrong line');
        }
      }
    }
    else
    {
      next if $line->{skipCallback}->($line);
      my $control = $line->{newLineControl};

      print $fh $self->_debugPrefix(' PENDING NL')."\n" if $pendingNewLine;
      $pendingNewLine = ($control & $NEW_LINE_END ? 1 : 0);

      print $fh $self->_debugPrefix(' BEGIN NL')."\n" if ($control & $NEW_LINE_BEGIN) && !$eatNextNewLine;
      $eatNextNewLine = 0;

      $line->_saveTo($fh, $localIndent);
    }
  }

  print $fh $self->_debugPrefix(' PENDING NL')."\n" if $pendingNewLine &&
                                                       !($self->{newLineControl} & $NEW_LINE_EAT_END);

  Gen::Util::registerErrorHandler($oldHandler);
}

##################################### Default skip callback

sub _skip
{
  my ($self) = @_;
  return $self->empty;
}

##################################### Internal methods

sub _save
{
  my ($self, $fh) = @_;
  my $name = $self->name;

  if (!$self->{skipCallback}->($self))
  {
    print $fh "Writer(\"$name\"): BEGIN" if $DEBUG_OUTPUT && $self->{newLineControl} & $NEW_LINE_BEGIN;
    print $fh "\n" if $self->{newLineControl} & $NEW_LINE_BEGIN;

    $self->_saveTo($fh, 0);

    print $fh "Writer(\"$name\"): END" if $DEBUG_OUTPUT && $self->{newLineControl} & $NEW_LINE_END;
    print $fh "\n" if $self->{newLineControl} & $NEW_LINE_END;
  }
}

sub _debugPrefix
{
  my ($self, $line) = @_;
  $line = $line || '';
  return '' if !$DEBUG_OUTPUT;
  return 'Writer("'.$self->name.'"):'.$line;
}

sub _writeLineRecursion
{
  my ($self, $wordsCopy, $words) = @_;

  foreach my $word (@$words)
  {
    if ((reftype($word) || '') eq 'ARRAY')
    {
      $self->_writeLineRecursion($wordsCopy, $word);
    }
    elsif (blessed($word) && $word->isa('Gen::Word::Stream'))
    {
      $self->_writeLineRecursion($wordsCopy, $word->words);
    }
    elsif (!blessed($word) || !$word->isa('Gen::Word'))
    {
      push @$wordsCopy, Gen::Word->new($word);
    }
    else
    {
      push @$wordsCopy, $word;
    }
  }
}

sub _saveLine
{
  my ($self, $fh, $line, $anchor) = @_;
  if (@$line == 0)
  {
    print $fh "\n";
    return;
  }

  my $empty = 1;

  my $space = '';
  my $prefix = '';
  my $postfix = '';
  my %indents = ();
  my $length = $anchor;
  $anchor += 2;
  print $fh (' ' x $length);

  foreach my $word (@$line)
  {
    if ($word->anchor)
    {
      $anchor = $indents{$word->anchor} || error('Invalid anchor');
    }
    $anchor += $word->relativeIndent;

    if (defined $word->prefix)
    {
      $prefix = $word->prefix;
    }

    my $projectedLength = $length + length($word->word) + length($space) + length($postfix);
    if (!$empty && $projectedLength > $OUTPUT_WIDTH)
    {
      $length = $anchor;
      print $fh $postfix, "\n", (' ' x $length), $prefix;
      $length += length($prefix);
      $space = '';
    }

    print $fh $space, $word->word;
    $length += length($space) + length($word->word);
    $empty = 0;
    $space = $word->space;

    if (defined $word->postfix)
    {
      $postfix = $word->postfix;
    }

    $indents{$word} = $length;
  }
  print $fh "\n";
}

############################################### Callback Writer
# Callback writer is a writer with a callback called at every save.
# The callback works with an empty $writer as its only argument. It's task is to fill it
# with lines before returing.

package Gen::Writer::CallbackWriter;
use strict;
use warnings;

our @ISA = qw(Gen::Writer);

sub new
{
  my ($class, $name, $callback) = @_;

  my $self = Gen::Writer::new($class, $name);
  $self->{skipCallback} = sub { return 0; }; # Never skip
  $self->{callback} = $callback;

  return $self;
}

sub _saveTo
{
  my ($self, $fh, $indent, $flags) = @_;

  $self->{lines} = [];
  $self->{lineIndents} = [];
  $self->{indent} = 0;

  $self->{callback}->($self);

  return Gen::Writer::_saveTo($self, $fh, $indent, $flags);
}

1;
