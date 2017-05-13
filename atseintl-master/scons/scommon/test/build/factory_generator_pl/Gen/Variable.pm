require Gen::Type;

############################################### Variable
# Represents typed C++ entity that can be read and/or written.
# Such entities can be local variables, class accessor pairs, etc.

package Gen::Variable;
use strict;
use warnings;
use Scalar::Util qw(reftype);
use Gen::Util qw(error);

##################################### Getters

sub name
{
  my ($self) = @_;
  return $self->{name};
}

sub type
{
  my ($self) = @_;
  return $self->{type};
}

# Returns a name with given postfix. Used to create derived variables.
sub postfixName
{
  my ($self, $postfix) = @_;
  return $self->{postfixName}.$postfix;
}

# If variable is reference, it means that getExpression returns a writable reference,
# i.e. it can be used to write to variable.
sub isReference
{
  return 0;
}

sub gettable
{
  return 0;
}

sub settable
{
  return 0;
}

##################################### C++ Statements

sub getExpression
{
  error('Variable is not gettable');
}

sub getExpressionReference
{
  my ($self) = @_;
  return '*'.$self->getExpression if $self->type->isPointer;

  return $self->getExpression;
}

sub getExpressionPointer
{
  my ($self) = @_;
  return $self->getExpression if $self->type->isPointer;

  return '&'.$self->getExpression;
}

sub setInstruction
{
  my ($self, $value) = @_;
  error('Variable is not settable') if !$self->gettable || !$self->isReference;

  return $self->getExpression.' = '.$value;
}

sub setInstructionPointer
{
  my ($self, $value) = @_;
  $value = '*'.$value if !$self->type->isPointer;
  return $self->setInstruction($value);
}

##################################### Setters

sub setType
{
  my ($self, $type) = @_;
  $self->{type} = $type;
  return $self;
}

############################################### LocalVariable
# Simple local (or not) variable.

package Gen::LocalVariable;
use strict;
use warnings;
use Scalar::Util qw(reftype);

our @ISA = qw(Gen::Variable);

sub new
{
  my ($class, $params) = @_;

  my $self =
  {
    name        => '',
    postfixName => '',
    type        => undef
  };
  bless $self, $class;
  $self->setName($$params{name}) if defined $$params{name};
  $self->setType($$params{type}) if defined $$params{type};

  return $self;
}

##################################### Getters

sub isReference
{
  return 1;
}

sub gettable
{
  return 1;
}

sub settable
{
  return 1;
}

##################################### C++ Statements

sub getExpression
{
  my ($self) = @_;
  return $self->{name};
}

sub declaration
{
  my ($self, $initialize) = @_;
  $initialize = $initialize || 0;

  return $self->{type}->declaration($self->name, $initialize);
}

##################################### Setters

sub setName
{
  my ($self, $name) = @_;

  $self->{name} = $name;
  $self->{postfixName} = $name;

  return $self;
}

############################################### CastedVariable
# This variable is just a casted interface to other existing variable.
# The type of CastedVariable is the type visible to users of it. For example:
#
# $v = LocalVariable({name => 'v', type => Type->new('int')});
# $cast = CastedVariable({variable => $v, type => Type->new('unsigned')});
#
# $v->getExpression        eq 'v';
# $cast->getExpression     eq '(unsigned)v';
# $v->setInstruction(1)    eq 'v = 1';
# $cast->setInstruction(1) eq 'v = (int)1';

package Gen::CastedVariable;
use strict;
use warnings;

our @ISA = qw(Gen::Variable);

sub new
{
  my ($class, $params) = @_;

  my $self =
  {
    name        => '',
    postfixName => '',
    type        => undef,
    variable    => undef
  };
  bless $self, $class;

  $self->setType($$params{type}) if defined $$params{type};
  $self->setVariable($$params{variable}) if defined $$params{variable};

  return $self;
}

##################################### Getters

sub gettable
{
  my ($self) = @_;
  return $self->variable->gettable;
}

sub settable
{
  my ($self) = @_;
  return $self->variable->settable;
}

sub variable
{
  my ($self) = @_;
  return $self->{variable};
}

##################################### Setters

sub setVariable
{
  my ($self, $variable) = @_;

  $self->{variable} = $variable;
  $self->{name} = $variable->name;
  $self->{postfixName} = $variable->postfixName('');

  return $self;
}

##################################### C++ Statements

sub getExpression
{
  my ($self) = @_;
  return $self->variable->getExpression if $self->variable->type == $self->type;
  return '('.$self->type->fullName.')'.$self->variable->getExpression;
}

sub setInstruction
{
  my ($self, $value) = @_;
  return $self->variable->setInstruction($value) if $self->variable->type == $self->type;
  return $self->variable->setInstruction('('.$self->variable->type->fullName.')'.$value);
}

############################################### Container
# This Variable is the interface for a Container variable (variable of type which isContainer).
# Container->setInstruction inserts new element into container. Container is not gettable.
# begin/endFunction return begin() and end() expressions, and can be used to read from container.

package Gen::Container;
use strict;
use warnings;
use Gen::Util qw(error);

our @ISA = qw(Gen::Variable);

sub new
{
  my ($class, $variable) = @_;

  my $self =
  {
    name        => '',
    postfixName => '',
    type        => undef,
    container   => undef,
    method      => '',
    insert      => ''
  };
  bless $self, $class;

  $self->setContainer($variable) if defined $variable;

  return $self;
}

##################################### Getters

sub settable
{
  return 1;
}

sub container
{
  my ($self) = @_;
  return $self->{container};
}

##################################### Setters

sub setContainer
{
  my ($self, $variable) = @_;

  error('Reference needed for container') if !$variable->isReference;
  error('Variable must be gettable') if !$variable->gettable;

  my $prefix = $variable->getExpression;
  $prefix = "($prefix)" if $prefix !~ /^\w/;
  my $container = $variable->type;

  if ($container->isPointer)
  {
    $prefix .= '->';
    $container = $container->base;
  }
  else
  {
    $prefix .= '.';
  }

  error('Not a container') if !$container->isContainer;

  $self->{name} = $variable->name.'Container';
  $self->{postfixName} = $variable->postfixName('');
  $self->{type} = $container->base;
  $self->{prefix} = $prefix;
  $self->{insert} = $prefix.$container->insertInstruction('<INSERT>');
  $self->{container} = $variable;

  return $self;
}

sub beginFunction
{
  my ($self) = @_;
  return $self->{prefix}.'begin()';
}

sub endFunction
{
  my ($self) = @_;
  return $self->{prefix}.'end()';
}

##################################### C++ Statements

sub setInstruction
{
  my ($self, $value) = @_;
  my $insert = $self->{insert};
  $insert =~ s/<INSERT>/$value/;
  return $insert;
}

############################################### Accessor
# Accessor is a pair of getter and setter (as described in HOWTO.txt).

package Gen::Accessor;
use strict;
use warnings;
use Scalar::Util qw(reftype);

our @ISA = qw(Gen::Variable);

our $ACCESSOR_CONSTRUCT = 1;
our $ACCESSOR_INIT      = 2;

sub new
{
  my ($class, $params) = @_;

  my $self =
  {
    name        => '',
    postfixName => '',
    prefix      => '',
    accessorType => 0,
    type        => undef,
    getter      => '',
    setter      => '',
    isReference => 0,
    incomplete  => (defined $$params{incomplete} ? $$params{incomplete} : 1)
  };
  bless $self, $class;

  $self->setAccessorType($$params{accessorType}) if defined $$params{accessorType};
  $self->setName($$params{name}) if defined $$params{name};
  $self->setType($$params{type}) if defined $$params{type};
  $self->setPrefix(defined $$params{prefix} ? $$params{prefix} : 'item->');

  return $self;
}

# Clone an accessor, optionally changing its type.
sub clone
{
  my ($self, $type) = @_;
  $type = $self->type if !defined $type;

  return Gen::Accessor->new({
    name => $self->name,
    type => $type,
    accessorType => $self->accessorType,
    prefix => $self->prefix
  });
}

##################################### Getters

sub isReference
{
  my ($self) = @_;

  # Accessor to pointer is never a reference for incomplete objects, because it's null.
  return 0 if $self->type->isPointer && $self->{incomplete};
  return $self->{isReference};
}

sub gettable
{
  return 1;
}

sub settable
{
  return 1;
}

# Accessor type indicates whether it's used in construct or init phase of reading.
sub accessorType
{
  my ($self) = @_;
  return $self->{accessorType};
}

# Prefix is an expression that prepends every accessor.
sub prefix
{
  my ($self) = @_;
  return $self->{prefix};
}

# Indicates whether object has been already completely created, or not; Used to distinguish
# between null and non-null pointers.
sub incomplete
{
  my ($self) = @_;
  return $self->{incomplete};
}

##################################### Setters

sub setName
{
  my ($self, $name) = @_;

  $self->{name} = $name;
  $self->{isReference} = 0;
  if ($name =~ /^\s*get(\w+)\s*$/)
  {
    $self->{getter} = "$name()";
    $self->{setter} = "set$1(<SET>)";
    $self->{postfixName} = lcfirst($1);
  }
  elsif ($name =~ /^\s*set(\w+)\s*$/)
  {
    $self->{getter} = lcfirst($1).'()';
    $self->{setter} = "${name}(<SET>)";
    $self->{postfixName} = lcfirst($1);
  }
  else
  {
    $self->{getter} = $name;
    $self->{setter} = "${name} = <SET>";
    $name =~ s/^\s*(.*)\s*\(\s*\)\s*$/$1/;
    $name =~ s/\(|\)|\[|\]|\{|\}|\.|->/_/g;
    $self->{postfixName} = lcfirst($name);
    $self->{isReference} = 1;
  }

  return $self;
}

sub setPrefix
{
  my ($self, $prefix) = @_;
  $self->{prefix} = $prefix;
  return $self;
}

sub setAccessorType
{
  my ($self, $accessorType) = @_;
  $self->{accessorType} = $accessorType;
  return $self;
}

sub setIncomplete
{
  my ($self, $inc) = @_;
  $self->{incomplete} = $inc;
  return $self;
}

##################################### C++ Statements

sub getExpression
{
  my ($self) = @_;
  return $self->{prefix}.$self->{getter};
}

sub setInstruction
{
  my ($self, $value) = @_;
  my $set = $self->{setter};
  $set =~ s/<SET>/$value/;
  return $self->{prefix}.$set;
}

1;
