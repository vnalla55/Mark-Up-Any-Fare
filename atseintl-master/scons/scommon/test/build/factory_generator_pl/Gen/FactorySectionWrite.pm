require Gen::Variable;
require Gen::Writer;

############################################### Test*Factory.cpp Write Section
# It's a chunk of C++ code used to write providedElement into XML element. They can be nested,
# in order to support containers.

package Gen::FactorySectionWrite;
use strict;
use warnings;
use Gen::Util qw(error);

our @ISA = qw(Gen::Writer);

# These two differentiate sections.
# Attributes-section is a normal sections, built inline using SetAttribute and nested sections.
# Aggregate-section is a section which is built completely by another factory.
our $SECTION_ATTRIBUTES = 1;
our $SECTION_AGGREGATE = 2;

sub new
{
  my ($class, $params) = @_;

  my $self = Gen::Writer::new($class, $$params{name} || '');

  # Writers
  $$self{directWriter} = undef;
  $$self{loopWriter}   = undef;

  # Options
  $$self{providedElement} = $$params{providedElement} || 'rootElement';
  $$self{container}       = $$params{container} || undef;
  $$self{depth}           = $$params{depth} || 0;
  $$self{type}            = $$params{type} || $SECTION_ATTRIBUTES;

  # State
  $$self{sections}        = {};
  $$self{variables}       = {};
  $$self{itName}          = '';

  $self->_initWriters();
  return $self;
}

sub _initWriters
{
  my ($self) = @_;

  $self->{directWriter} = Gen::Writer->new('direct write')->setEndNewLine(1);
  $self->{loopWriter} = Gen::Writer->new('children write')->setEndNewLine(1);

  my $writer = $self;
  my $extraPush = 0;
  if (defined $self->{container})
  {
    my $container = $self->{container}->container->type->withoutConst;
    $container = $container->base->withoutConst if $container->isPointer;
    $container = $container->fullName;
    my $begin = $self->{container}->beginFunction;
    my $end = $self->{container}->endFunction;
    my $itName = 'it'.$self->{depth};
    $self->{itName} = $itName;

    my $nWriter = Gen::Writer->new('write loop');
    $writer
      ->pushBlock()
      ->writeCode(<<LINES
        ${container}::const_iterator
          $itName = $begin,
          end = $end;
        for (; $itName != end; ++$itName)
LINES
      )
      ->addSubWriter($nWriter)
      ->popBlock();
    $writer = $nWriter;

    $extraPush = 2;
  }

  if ($self->{type} == $SECTION_AGGREGATE)
  {
    $writer
      ->pushIndent($extraPush)
      ->addSubWriter($self->{directWriter})
      ->eatPreviousNewLine()
      ->popIndent($extraPush);
    return;
  }

  if ($self->{name} ne '')
  {
    my $nWriter = Gen::Writer->new('block');
    $writer
      ->pushBlock()
      ->addSubWriter($nWriter)
      ->popBlock();
    $writer = $nWriter;

    if ($self->{providedElement} ne 'rootElement')
    {
      my $providedElement = $self->{providedElement};
      $writer->writeCode("TiXmlElement* rootElement = $providedElement;");
    }

    my $name = $self->{name};
    $writer->writeCode(<<LINES
      TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(new TiXmlElement("$name"));

LINES
    );
  }

  $writer
    ->addSubWriter($self->{directWriter})
    ->addSubWriter($self->{loopWriter})
    ->eatPreviousNewLine();
}

##################################### Getters

sub directWriter
{
  my ($self) = @_;
  return $self->{directWriter};
}

sub loopWriter
{
  my ($self) = @_;
  return $self->{loopWriter};
}

sub getSection
{
  my ($self, $sectionName, $type, $container) = @_;

  return $self if $sectionName eq '';

  if (${$self->{sections}}{$sectionName})
  {
    my $section = ${$self->{sections}}{$sectionName};
    error('Cannot mix attribute and aggregate sections') if $section->{type} != $type;
    return $section;
  }

  my $section = Gen::FactorySectionWrite->new({
    name            => $sectionName,
    providedElement => !$self->{name} ? 'rootElement' : 'element',
    container       => $container,
    depth           => $self->{depth} + 1,
    type            => $type
  });
  $self->{loopWriter}->addSubWriter($section);
  return ${$self->{sections}}{$sectionName} = $section;
}

##################################### Overriden methods

sub empty
{
  my ($self) = @_;

  return $self->{directWriter}->empty &&
         $self->{loopWriter}->empty;
}

##################################### Methods

# Prepares section to write code for writing variable.
# Target variable is given by pair ($variable, $outputType).
# ($sectionName, $attributeName) control where in providedElement the value is located.
#
# Returns a 3-tuple of ($variable, $writer, $rootElement).
# The read from gettable $variable should be emited into $writer, and $rootElement is
# the XML element where it should be put.
sub prepareWrite
{
  my ($self, $variable, $outputType, $sectionName, $attributeName) = @_;
  return if !$self->_storeVariable($variable);
  my $sectionType = ($attributeName eq '' ? $SECTION_AGGREGATE : $SECTION_ATTRIBUTES);

  if ($variable->type->hasContainer)
  {
    my $container = Gen::Container->new($variable);

    $sectionType = $SECTION_ATTRIBUTES if $container->type->hasContainer;
    my $section = $self->getSection($sectionName || 'Element', $sectionType, $container);

    my $local = Gen::LocalVariable->new({
      name => '*'.$section->{itName},
      type => $container->type
    });

    return $section->prepareWrite($local, $outputType, '', $attributeName);
  }

  my $section = $self->getSection($sectionName, $sectionType);

  my $writer = Gen::Writer->new($variable->name);
  $section->{directWriter}->addSubWriter($writer);

  if (defined $outputType)
  {
    if ($variable->type->isPointer)
    {
      $outputType = $outputType->withConst if $variable->type->base->const;
      $outputType = $outputType->pointer;
    }
    $variable = Gen::CastedVariable->new({
      variable => $variable,
      type => $outputType,
      isReference => 0
    });
  }

  return ($variable, $writer, $section->_rootElement);
}

##################################### Internal

sub _rootElement
{
  my ($self) = @_;
  return $self->{providedElement} if $self->{type} == $SECTION_AGGREGATE;
  return 'element' if $self->{name};
  return 'rootElement';
}

sub _storeVariable # Simplified _storeVariable from FacSecRead, all error handling already done.
{
  my ($self, $variable) = @_;

  if (${$self->{variables}}{$variable->name})
  {
    # Skip every write of variable beyond the first.
    return 0;
  }

  return ${$self->{variables}}{$variable->name} = 1;
}

1;
