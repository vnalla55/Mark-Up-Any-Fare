require Gen::Type;
require Gen::Word;
require Gen::Writer;

############################################### C++ Field
# It works best with static class/global variables.
# Defines 2 writers: declaration and definition.

package Gen::Field;
use strict;
use warnings;
use Scalar::Util qw(blessed reftype);
use Gen::Util qw(error wrapIn);

# params:
#  name/type || nameType: strings/types for name and type
#  section: class section field resides in
#  initializer
#  modifiers: C++ modifiers (like static or mutable)
sub new
{
  my ($class, $params) = @_;
  if ((reftype($params) || '') ne 'HASH')
  {
    $params = {nameType => $params};
  }

  my $self =
  {
    name        => undef,
    modifiers   => {},
    section     => $$params{section} || '',
    type        => undef,
    initializer => $$params{initializer} || '',
    declaration => undef,
    definition  => undef
  };
  bless $self, $class;

  $self->{declaration} = Gen::Writer::CallbackWriter->new('',
    sub { $self->_writeSignature($_[0], 0); });

  $self->{definition} = Gen::Writer::CallbackWriter->new('',
    sub { $self->_writeSignature($_[0], 1); });
  $self->{definition}->setEndNewLine(1);

  if (defined $$params{nameType})
  {
    $self->setNameType($$params{nameType});
  }
  else
  {
    $self->setName($$params{name} || '');
    $self->setType($$params{type} || '');
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

sub type
{
  my ($self) = @_;
  return $self->{type};
}

sub nameWithType
{
  my ($self) = @_;
  return $self->type->fullName($self->name);
}

sub shortNameWithType
{
  my ($self) = @_;
  return $self->type->fullName($self->shortName);
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

##################################### Setters

sub setSkipCallback
{
  my ($self, $cb) = @_;
  $self->declaration->setSkipCallback($cb);
  $self->definition->setSkipCallback($cb);
  return $self;
}

sub setNameType
{
  my ($self, $nameType) = @_;
  my ($type, $name) = Gen::Type::parse($nameType);
  $self->setName($name);
  $self->setType($type);

  return $self;
}

sub setName
{
  my ($self, $name) = @_;

  my @name = split(/::/, $name);
  $self->{name} = \@name;

  $self->{declaration}->setName("$name declaration");
  $self->{definition}->setName("$name definition");

  return $self;
}

sub setType
{
  my ($self, $type_) = @_;

  my $type = $type_;
  $type = Gen::Type->new($type) if !blessed($type) || !$type->isa('Gen::Type');

  $self->{type} = $type;
  return $self;
}

sub setInitializer
{
  my ($self, $initializer) = @_;
  $self->{initializer} = $initializer;
  return $self;
}

sub addModifier
{
  my ($self, $modifier) = @_;
  if (
    $modifier eq 'static' ||
    $modifier eq 'mutable')
  {
    ${$self->{modifiers}}{$modifier} = 1;
  }
  else
  {
    my $name = $self->nameWithType;
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

sub _writeSignature
{
  my ($self, $writer, $isDefinition) = @_;

  {
    my $stream = Gen::Word::Stream->new();
    $stream->addWord($self->{section}.' ') if $self->{section} ne '' && $isDefinition;

    foreach my $mod ('static', 'mutable')
    {
      $stream->addWord($mod.' ') if ${$self->{modifiers}}{$mod};
    }

    if (!$stream->empty && $isDefinition)
    {
      my $outerStream = Gen::Word::Stream->new();
      $outerStream->addWord('/* ')->addSubStream($stream)->addWord('*/');
      $stream = $outerStream;
    }

    if (!$stream->empty)
    {
      $writer->writeLine($stream);
    }
  }

  my $decl = $isDefinition ? $self->nameWithType : $self->shortNameWithType;
  if ($self->{initializer} ne '')
  {
    $decl .= wrapIn(!$isDefinition, ' /*', ' = '.$self->{initializer}, ' */');
  }
  $decl .= ';';
  $writer->writeCode($decl);
}

1;
