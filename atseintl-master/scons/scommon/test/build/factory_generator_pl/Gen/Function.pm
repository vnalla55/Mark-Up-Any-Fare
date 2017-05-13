require Gen::Type;
require Gen::Word;
require Gen::Writer;

############################################### C++ Function
# Defines 2 writers: declaration and definition. Insert function body into ->body writer.

package Gen::Function;
use strict;
use warnings;
use Scalar::Util qw(blessed reftype);
use Gen::Util qw(error wrapIn);

# params:
#  name/returnType/arguments || signature: strings/types for signature of function
#  section: class section function resides in
#  modifiers: C++ modifiers (like static or const)
sub new
{
  my ($class, $params) = @_;
  if ((reftype($params) || '') ne 'HASH')
  {
    $params = {signature => $params};
  }

  my $self =
  {
    name        => [],
    modifiers   => {},
    section     => $$params{section} || '',
    type        => undef,
    declaration => undef,
    definition  => undef,
    body        => undef
  };
  bless $self, $class;

  $self->{declaration} = Gen::Writer::CallbackWriter->new('',
    sub { $self->_writeSignature($_[0], 0); });
  $self->{declaration}->setEndNewLine(1);

  $self->{definition} = Gen::Writer->new('');
  $self->{definition}
    ->addCallback('signature', sub { $self->_writeSignature($_[0], 1); })
    ->pushBlock()
    ->addSubWriter($self->{body} = Gen::Writer->new('body'))
    ->popBlock()
    ->setEndNewLine(1);

  if (defined $$params{signature})
  {
    $self->setSignature($$params{signature});
  }
  else
  {
    $self->setReturnType($$params{returnType} || 'void');
    $self->setName($$params{name} || '');

    if ($$params{arguments})
    {
      foreach my $arg (@{$$params{arguments}})
      {
        if ((reftype($arg) || '') eq 'ARRAY')
        {
          $self->addArgument(@$arg);
        }
        else
        {
          $self->addArgument($arg);
        }
      }
    }
  }

  if ($$params{modifiers})
  {
    foreach my $mod (@{$$params{modifiers}})
    {
      $self->addModifier($mod);
    }
  }

  return $self;
}

##################################### Getters

sub name
{
  my ($self) = @_;
  return join('::', @{$self->{name}});
}

sub shortName
{
  my ($self) = @_;
  return ${$self->{name}}[-1];
}

sub signature
{
  my ($self) = @_;
  return $self->{type};
}

sub returnType
{
  my ($self) = @_;
  return $self->{type}->returnType;
}

sub arguments
{
  my ($self) = @_;
  return $self->{type}->arguments;
}

sub hasModifier
{
  my ($self, $mod) = @_;
  return defined ${$self->{modifiers}}{$mod} ? 1 : 0;
}

sub declaration
{
  my ($self) = @_;
  return $self->{declaration};
}

sub definition
{
  my ($self) = @_;
  return $self->{definition};
}

sub body
{
  my ($self) = @_;
  return $self->{body};
}

##################################### Setters

sub setSkipCallback
{
  my ($self, $cb) = @_;
  $self->declaration->setSkipCallback($cb);
  $self->definition->setSkipCallback($cb);
  return $self;
}

sub setSignature
{
  my ($self, $signature) = @_;

  my ($type, $name) = Gen::Type::parse($signature);
  error('Wrong signature') if !$type->isFunction;

  $self->{type} = $type;
  $self->setName($name);

  return $self;
}

sub setName
{
  my ($self, $name) = @_;

  my @name = split(/::/, $name);
  $self->{name} = \@name;

  $self->_updateNames();
  return $self;
}

sub setReturnType
{
  my ($self, $type_) = @_;

  my $type = $type_;
  $type = Gen::Type->new($type) if !blessed($type) || !$type->isa('Gen::Type');

  if (!defined $self->{type})
  {
    $self->{type} = $type->function();
  }
  else
  {
    $self->{type} = $type->function(@{$self->{type}->arguments});
  }

  $self->_updateNames();
  return $self;
}

sub addArgument
{
  my $self = shift;
  my ($type, $name, $initializer);

  $type = shift;
  if (blessed($type) && $type->isa('Gen::Type'))
  {
    $name = shift || '';
  }
  else
  {
    ($type, $name) = Gen::Type::parse($type);
  }
  $initializer = shift || '';

  my $arg = [$type, $name, $initializer];
  $self->{type} = $self->{type}->returnType->function(@{$self->{type}->arguments}, $arg);

  $self->_updateNames();
  return $self;
}

sub addModifier
{
  my ($self, $modifier) = @_;
  if (
    $modifier eq 'explicit' ||
    $modifier eq 'static' ||
    $modifier eq 'virtual' ||
    $modifier eq 'const')
  {
    ${$self->{modifiers}}{$modifier} = 1;
  }
  else
  {
    my $name = $self->name;
    error("$name: Unknown modifier '${modifier}'");
  }
  return $self;
}

sub setSection
{
  my ($self, $section) = @_;
  $self->{section} = $section;
  return $self;
}

##################################### Internal

sub _updateNames
{
  my ($self) = @_;

  my @argumentTypes = ();
  foreach my $arg (@{$self->{arguments}})
  {
    push @argumentTypes, $$arg[0];
  }

  my $type = $self->returnType->function(@argumentTypes);
  my $name = $type->fullName($self->name);

  $self->{declaration}->setName("$name declaration");
  $self->{definition}->setName("$name definition");
}

sub _writeSignature
{
  my ($self, $writer, $isDefinition) = @_;

  {
    my $stream = Gen::Word::Stream->new();
    $stream->addWord($self->{section}.' ') if $self->{section} ne '' && $isDefinition;

    foreach my $mod ('explicit', 'static', 'virtual')
    {
      $stream->addWord($mod.' ') if ${$self->{modifiers}}{$mod};
    }

    if (!$stream->empty && $isDefinition)
    {
      my $outerStream = Gen::Word::Stream->new();
      $outerStream->addWord('/* ')->addSubStream($stream)->addWord('*/');
      $stream = $outerStream;
    }

    if (!$self->returnType->isNull)
    {
      $stream->lastWord->setWord($stream->lastWord->word.' ') if !$stream->empty;
      $stream->addCode($self->returnType->fullName);
    }

    if (!$stream->empty)
    {
      $writer->writeLine($stream);
    }
  }

  my $decl = Gen::Word::Stream->new();
  $decl->addCode(($isDefinition ? $self->name : $self->shortName).'(');
  $decl->setIndent(0, $decl->lastWord);

  my $i = 0;
  my $count = @{$self->arguments};
  for (my $i = 0; $i < $count; ++$i)
  {
    my $comma = ($i+1 == $count) ? '' : ', ';
    my $arg = ${$self->arguments}[$i];
    $arg = [$arg] if (reftype($arg) || '') ne 'ARRAY';

    my $argStr = $$arg[0]->fullName($$arg[1]);
    $argStr .= wrapIn($isDefinition, ' /*', ' = '.$$arg[2], ' */') if (
        defined $$arg[2] && $$arg[2] ne '');

    $decl->addWord($argStr.$comma);
  }

  my $const = ${$self->{modifiers}}{const} ? ' const' : '';
  my $semicolon = $isDefinition ? '' : ';';
  $decl
    ->setIndent(-1)
    ->addWord(")$const$semicolon");

  $writer->writeLine($decl);
}

1;
