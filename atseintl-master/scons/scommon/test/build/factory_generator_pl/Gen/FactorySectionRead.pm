require Gen::Variable;
require Gen::Writer;

############################################### Test*Factory.cpp Read Section
# It's a chunk of C++ code used to read from current rootElement. They can be nested, in order
# to support containers.

package Gen::FactorySectionRead;
use strict;
use warnings;
use Gen::Util qw(error);

our @ISA = qw(Gen::Writer);

sub new
{
  my ($class, $params) = @_;

  my $self = Gen::Writer::new($class, $$params{name}.' section');

  $$self{deferAllSets}      = $$params{deferAllSets} || 0;
  $$self{omitFinalSet}      = $$params{omitFinalSet} || 0;
  $$self{rootElement}       = $$params{rootElement} || 'rootElement';

  # Writers
  $$self{declarationWriter} = undef;
  $$self{directWriter}  = undef;
  $$self{loopWriter}    = undef;
  $$self{finalSetWriter}    = undef;

  # State
  $$self{sections}          = {};
  $$self{variables}         = {};

  $self->_initWriters();
  return $self;
}

sub _initWriters
{
  my ($self) = @_;

  $self->{declarationWriter} = Gen::Writer->new('declaration')->setEndNewLine(1);
  $self->addSubWriter($self->{declarationWriter});

  $self->{directWriter} = Gen::Writer->new('direct read')->setEndNewLine(1);
  $self->addSubWriter($self->{directWriter});

  my $rootElement = $self->{rootElement};
  $self->{loopWriter} = Gen::Writer->new('loop read')->setEndNewLine(1);
  my $loop = Gen::Writer->new('loop');
  $loop
    ->setSkipCallback(sub { return $self->{loopWriter}->empty; })
    ->writeCode(<<LINES
      TiXmlNode* child = 0;
      while ((child = $rootElement->IterateChildren(child)))
LINES
    )
    ->pushBlock()
    ->writeCode(<<LINES
        TiXmlElement* element = (TiXmlElement*)child;
        const std::string elementName = element->Value();

        if (false);
LINES
    )
    ->addSubWriter($self->{loopWriter})
    ->popBlock()
    ->setEndNewLine(1);
  $self->addSubWriter($loop);

  $self->{finalSetWriter} = Gen::Writer->new('final set')->setEndNewLine(1);
  if (!$self->{omitFinalSet})
  {
    $self->addSubWriter($self->{finalSetWriter});
  }

  $self->eatEndNewLine(1);
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

sub getAccessor
{
  my ($self, $name) = @_;
  my $rec = ${$self->{variables}}{$name};

  return $$rec{variable} if $rec;
  return undef;
}

sub getSection
{
  my ($self, $sectionName) = @_;

  if (${$self->{sections}}{$sectionName})
  {
    return ${$self->{sections}}{$sectionName};
  }

  my $sectionOuter = Gen::Writer->new("$sectionName if");
  my $section = Gen::FactorySectionRead->new({
    name => $sectionName,
    rootElement => 'element'
  });

  $sectionOuter
    ->writeCode(<<LINES
      else if (elementName == "$sectionName")
LINES
    )
    ->pushBlock()
    ->addSubWriter($section)
    ->popBlock();

  $self->{loopWriter}->addSubWriter($sectionOuter);

  return ${$self->{sections}}{$sectionName} = $section;
}

##################################### Overriden methods

sub empty
{
  my ($self) = @_;

  return $self->{declarationWriter}->empty &&
         $self->{directWriter}->empty &&
         $self->{loopWriter}->empty &&
         $self->{finalSetWriter}->empty;
}

##################################### Methods

# Prepares section to write code for reading variable.
# Target variable is given by pair ($variable, $inputType).
# ($sectionName, $attributeName) control where in rootElement the value is located.
#
# Returns a pair of ($variable, $writer).
# The write to settable $variable should be emited into $writer.
#
# $needsReference may be used to request the resulting $variable to be a reference.
sub prepareRead
{
  my ($self, $variable, $inputType, $sectionName, $attributeName, $needsReference) = @_;
  my $type = $variable->type;

  if ($type->hasContainer)
  {
    $sectionName = 'Element' if !$sectionName;
    $self->_storeVariable($variable, $sectionName, $attributeName);

    if (!$variable->isReference || $self->{deferAllSets})
    {
      # TODO Make the set conditional if not deferAllSets.
      # Well... it may not be a bad idea to initialize something like std::vector<T>*.
      $variable = $self->_createSave($variable);
    }

    my $container = Gen::Container->new($variable);

    my $section = $self->getSection($sectionName);
    return $section->prepareRead($container, $inputType, undef, $attributeName, $needsReference);
  }

  $self->_storeVariable($variable, $sectionName || '', $attributeName);
  my $section = $sectionName ? $self->getSection($sectionName) : $self;
  my $writer = Gen::Writer->new($variable->name);

  my $outerWriter = $writer;
  if ($attributeName ne '')
  {
    my $rootElement = $section->{rootElement};
    $outerWriter = Gen::Writer->new("$attributeName if");
    $outerWriter
      ->writeCode(<<LINES
        if (TestXMLHelper::AttributeExists($rootElement, "$attributeName"))
LINES
      )
      ->pushBlock()
      ->addSubWriter($writer)
      ->popBlock();
  }
  $section->{directWriter}->addSubWriter($outerWriter);

  if ($self->{deferAllSets})
  {
    return ($self->_createSave($variable, $inputType), $writer);
  }

  if (!defined $inputType &&
      (!$needsReference || $variable->isReference))
  {
    return ($variable, $writer);
  }

  my $declWriter = Gen::Writer->new('declaration');
  my $innerWriter = Gen::Writer->new('inner');
  my $setWriter = Gen::Writer->new('set');
  $writer
    ->addSubWriter($declWriter)
    ->addSubWriter($innerWriter)
    ->addSubWriter($setWriter);

  return ($self->_createLocal($declWriter, $setWriter, $variable, $inputType), $innerWriter);
}

##################################### Internal

sub _validateType
{
  my ($self, $type) = @_;

  return $self->_validateType($type->base) if $type->isContainer;
  return 1 if $type->isNormal || $type->isTemplate;
  return 0 if $type->isReference || $type->isArray || $type->isFunction;

  if ($type->isPointer)
  {
    return 0 if $type->base->isPointer; # No double pointers supported.
    return $self->_validateType($type->base);
  }

  error('Unreachable');
}

sub _storeVariable
{
  my ($self, $variable, $sectionName, $attributeName) = @_;

  my $variableRec;

  if (${$self->{variables}}{$variable->name})
  {
    $variableRec = ${$self->{variables}}{$variable->name};
    my $storedVariable = $$variableRec{variable};
    if ($variable->type ne $storedVariable->type)
    {
      error('Variable type doesn\'t match previous declaration');
    }
    if (${$$variableRec{names}}{"$sectionName-$attributeName"})
    {
      error(($attributeName ? 'Attribute' : 'Section').' already exists');
    }
  }
  else
  {
    error('Wrong variable type') if !$self->_validateType($variable->type);

    $variableRec = ${$self->{variables}}{$variable->name} = {
      variable => $variable,
      names => {},
      saveVariable => undef
    };
  }

  ${$$variableRec{names}}{"$sectionName-$attributeName"} = 1;
  return $variableRec;
}

sub _createSave
{
  my ($self, $variable, $inputType) = @_;
  my $variableRec = ${$self->{variables}}{$variable->name};

  if (!$$variableRec{saveVariable})
  {
    $$variableRec{saveVariable} = $self->_createLocal(
      $self->{declarationWriter}, $self->{finalSetWriter}, $variable, $inputType);
  }
  return $$variableRec{saveVariable};
}

sub _createLocal
{
  my ($self, $declWriter, $setWriter, $variable, $inputType) = @_;

  my $type = $variable->type->withoutConst;
  $type = $type->base->withoutConst->pointer if $type->isPointer;

  my $save = $variable;
  my $local;
  if (defined $inputType)
  {
    $inputType = $inputType->pointer if $type->isPointer;

    $local = Gen::LocalVariable->new({
      name => $variable->postfixName('Local'),
      type => $inputType
    });
    $save = Gen::CastedVariable->new({
      variable => $variable,
      type => $inputType,
      isReference => 0
    });
  }
  else
  {
    $local = Gen::LocalVariable->new({
      name => $variable->postfixName('Local'),
      type => $type
    });
  }

  $declWriter->writeCode($local->declaration(1).';');
  $setWriter->writeCode($save->setInstruction($local->getExpression).';');
  return $local;
}

1;
