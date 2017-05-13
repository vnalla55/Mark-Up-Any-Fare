############################################### C++ Type
# Represents C++ type. It forms a tree of types, like for example int* -> int.
# Every type object is a unique constant. It means that you can check two types
# for equivalence just comparing their references ($type1 == $type2).
#
# There are 6 basic type classes:
#  - Null (return type of ctor/dtor, i.e. nothing)
#  - Normal (int, const bool, void, std::string)
#  - PtrRef (int*, const long&, std::vector<int>*)
#  - Array (int[], long[7])
#  - Function (void(), int(int, int*))
#  - Template (std::pair<int, int>, std::list<double>)
#
# Use Gen::Type->new() to get a new type from string.
# Use Gen::Type::parse() to get a pair of ($type, $id). E.g. parse('int a') = (new('int'), 'a').
#
# By default, functions still contain their arguments' names and defaults. It means, that
# new('int foo(int a)') != new('int foo(int)'). It's useful when you want to parse function
# signature (see Gen::Function). Use normalize() method to remove any function argument
# name/default.

package Gen::Type;
use strict;
use warnings;
use Scalar::Util qw(blessed reftype);
use Gen::Util qw(error extractParens extractArgument);

my %Types = ();

sub printAllTypes
{
  foreach my $type (keys %Types)
  {
    my $full = $Types{$type}->fullName('foo');
    print "$type: $full\n";
  }
}

##################################### Type parser

sub parse
{
  my ($typeString) = @_;
  my $id = '';

  my $oldHandler = Gen::Util::errorHandler;
  Gen::Util::registerErrorHandler(sub {
    $oldHandler->("$typeString: ".$_[0]);
  });

  $typeString =~ s/\n/ /gm; # Convert new-lines to spaces.
  if (!($typeString =~ /^\s*(const\s+)?((?:\w|\s*::\s*)+)\s*(.+)?\s*$/))
  {
    return (Gen::Type::Null->new(), '') if $typeString =~ /^\s*$/;
    error('Unknown type');
  }
  my $type = Gen::Type::Normal->new($2, ($1 ? 1 : 0));
  my $decl = $3 || '';

  while ($decl !~ /^\s*$/)
  {
    if ($decl =~ /^\s*\(\s*(.*)\s*$/)
    {
      my ($in, $out) = extractParens('('.$1);
      if (($in =~ /^\s*$/ || $in =~ /^\s*(?:const\s+)?(?:\w|\s*::\s*)+.*\s*$/))
      {
        # It's a function (without an id).
        $type = _parsePostfix($type, "($in)$out");
        last;
      }
      else
      {
        # Otherwise, it's just a grouping.
        $decl = $in;
        $type = _parsePostfix($type, $out);
      }
    }
    elsif ($decl =~ /^\s*([*&])(\s*const)?(.*)\s*$/)
    {
      my $char = $1;
      my $const = ($2 ? 1 : 0);
      $decl = $3;
      if ($const && $decl =~ /^\w/)
      {
        # Not really a const, just an identifier that begins with const.
        $decl = "const$decl";
        $const = 0;
      }

      $type = $type->ptrref($char, $const);
    }
    elsif ($decl =~ /^\s*<\s*(.*)\s*$/)
    {
      error('Malformed type') if !$type->isNormal;

      my ($in, $out) = extractParens('<'.$1);
      $decl = $out;
      my @args = ();

      while ($in !~ /^\s*$/)
      {
        my ($arg, $rest) = extractArgument($in);
        $in = $rest;
        push @args, Gen::Type->new($arg);
      }

      $type = $type->template(@args);
    }
    elsif ($decl =~ /^\s*((?:\w|\s*::\s*|\.)+)\s*(.*)\s*$/)
    {
      $id = $1;
      $type = _parsePostfix($type, $2);
      last;
    }
    else
    {
      $type = _parsePostfix($type, $decl);
      last;
    }
  }

  Gen::Util::registerErrorHandler($oldHandler);
  return ($type, $id);
}

sub _parsePostfix
{
  my ($type, $decl) = @_;

  while ($decl !~ /^\s*$/)
  {
    if ($decl =~ /^\s*\(\s*(.*)\s*$/)
    {
      my ($in, $out) = extractParens('('.$1);
      $decl = $out;
      my @args = ();

      while ($in !~ /^\s*$/)
      {
        my ($arg, $rest) = extractArgument($in);
        $in = $rest;

        push @args, $arg;
      }

      $type = $type->function(@args);
    }
    elsif ($decl =~ /^\s*\[(\d*)\]\s*(.*)\s*$/)
    {
      my $size = $1 eq '' ? -1 : int($1);
      $decl = $2;

      $type = $type->array($size);
    }
    else
    {
      error('Malformed type');
    }
  }

  return $type;
}

##################################### Constructor

sub new
{
  my ($class, $fullName) = @_;
  my ($type, undef) = parse($fullName);
  return $type;
}

##################################### Getters

# Return name of type with id. For int* [] type:
# fullName()     eq 'int*[]'
# fullName('id') eq 'int* id[]'
sub fullName
{
  my ($self, $id) = @_;
  $id = $id || '';
  my $pattern = $self->{fullName};

  my $name;

  if ($pattern =~ s/\#%s/%s/ && $id =~ /^\w/)
  {
    $name = sprintf($pattern, " $id");
  }
  elsif ($pattern =~ s/\$%s/%s/)
  {
    if ($id =~ /^\W/ && $id !~ /^(%s|\#|\$)/)
    {
      $name = sprintf($pattern, " ($id)");
    }
    elsif ($id =~ /^\w/)
    {
      $name = sprintf($pattern, " $id");
    }
    else
    {
      $name = sprintf($pattern, $id);
    }
  }
  else
  {
    $name = sprintf($pattern, $id);
  }

  while ($name =~ s/>>/> >/) {}

  return $name;
}

sub isNull
{
  return 0;
}

sub isNormal
{
  return 0;
}

sub isPtrRef
{
  return 0;
}

sub isPointer
{
  return 0;
}

sub isReference
{
  return 0;
}

sub isArray
{
  return 0;
}

sub isFunction
{
  return 0;
}

sub isTemplate
{
  return 0;
}

sub isContainer
{
  return 0;
}

sub isNormalized
{
  return 1;
}

##################################### Derivative Types
# Use these methods to create a new type derived from the current one.
# new('int')->pointer({const => 1}) is 'int* const'.

# One optional argument: param hash OR bool isConst/
sub pointer
{
  return Gen::Type::Pointer->new(@_);
}

sub reference
{
  return Gen::Type::Reference->new(@_);
}

sub ptrref
{
  my ($self, $char, @args) = @_;
  return $self->pointer(@args) if $char eq '*';
  return $self->reference(@args) if $char eq '&';

  error('Unknown char of pointer/reference: '.$char);
}

# One argument: array size/
sub array
{
  return Gen::Type::Array->new(@_);
}

# Arguments will be parsed as arguments to function, $self is return type.
sub function
{
  return Gen::Type::Function->new(@_);
}

# Arguments will be parsed as template parameters, $self is Normal of template (like 'std::vector').
sub template
{
  return Gen::Type::Template->new(@_);
}

sub withoutConst
{
  my ($self) = @_;
  return $self;
}

sub normalized
{
  my ($self) = @_;
  return $self;
}

##################################### Recursive Getters

# Whether type has container somewhere in DAG.
sub hasContainer
{
  return 0;
}

##################################### C++ Statements

sub declaration
{
  my ($self, $name) = @_;
  return $self->fullName($name);
}

##################################### Internal

sub _register
{
  my ($self) = @_;
  if ($Types{$self->{fullName}})
  {
    return $Types{$self->{fullName}};
  }
  return $Types{$self->{fullName}} = $self;
}

sub _parseArgsWithBase
{
  my $class = shift;
  my $base = shift;

  if (!blessed($base) || !$base->isa('Gen::Type'))
  {
    $base = Gen::Type->new($base);
  }

  my $params = shift;
  $params = {} if !defined $params;
  return ($class, $base, $params);
}

sub _parseArgsWithArgs
{
  my $dontNormalize = shift;
  my $class = shift;
  my $base = shift;
  my @arguments = ();

  if (!blessed($base) || !$base->isa('Gen::Type'))
  {
    $base = Gen::Type->new($base);
  }

  while (@_)
  {
    my $arg = shift;

    if ((blessed($arg) && $arg->isa('Gen::Type')) ||
        (reftype($arg) || '') eq 'ARRAY')
    {
      push @arguments, $arg;
    }
    else
    {
      my $initializer = '';
      if ($arg =~ /^\s*(.+)\s*=\s*(.+)\s*$/)
      {
        $initializer = $2;
        $arg = $1;
      }

      my ($type, $id) = parse($arg);

      if ($dontNormalize && ($id ne '' || $initializer ne ''))
      {
        push @arguments, [$type, $id, $initializer];
      }
      else
      {
        push @arguments, $type;
      }
    }
  }

  return ($class, $base, \@arguments);
}

sub _getArgumentsToString
{
  my ($arguments) = @_;
  my $args = '';
  my $first = 1;

  foreach my $arg (@$arguments)
  {
    $args .= ', ' if !$first;
    $first = 0 if $first;

    if (reftype($arg) eq 'ARRAY')
    {
      $args .= $$arg[0]->fullName($$arg[1].($$arg[2] ne '' ? ' = '.$$arg[2] : ''));
    }
    else
    {
      $args .= $arg->fullName;
    }
  }

  return $args;
}

############################################### C++ Null Type (defined as return type of ctor/dtor)

package Gen::Type::Null;

our @ISA = qw(Gen::Type);

sub new
{
  my ($class) = @_;

  my $self = {
    fullName => '%s'
  };
  bless $self, $class;

  return Gen::Type::_register($self);
}

##################################### Getters

sub isNull
{
  return 1;
}

############################################### C++ Normal Type (void/int/std::string)

package Gen::Type::Normal;
use Scalar::Util qw(reftype);

our @ISA = qw(Gen::Type);

sub new
{
  my ($class, $name, $params) = @_;
  $name = _translateVariableType($name);
  $params = $params || {};
  $params = {const => $params} if (reftype($params) || '') ne 'HASH';

  my $self = {
    name => $name,
    const => $$params{const} || 0
  };
  bless $self, $class;

  $self->{fullName} = ($self->{const} ? 'const ' : '').$name.'#%s';

  return Gen::Type::_register($self);
}

##################################### Getters

sub isNormal
{
  return 1;
}

sub name
{
  my ($self) = @_;
  return $self->{name};
}

sub const
{
  my ($self) = @_;
  return $self->{const};
}

##################################### Derivative Types

sub withConst
{
  my ($self) = @_;
  return $self if $self->const;
  return Gen::Type::Normal->new($self->{name}, {const => 1});
}

sub withoutConst
{
  my ($self) = @_;
  return $self if !$self->const;
  return Gen::Type::Normal->new($self->{name});
}

##################################### Recursive Getters

sub innerMostType
{
  my ($self) = @_;
  return $self;
}

##################################### Internal

sub _translateVariableType
{
  my ($variableType) = @_;

  return 'std::string' if $variableType eq 'string';
  return $variableType;
}

############################################### C++ Pointer/Reference Type

package Gen::Type::PtrRef;
use Scalar::Util qw(reftype);

our @ISA = qw(Gen::Type);

sub new
{
  my ($class, $base, $params) = Gen::Type::_parseArgsWithBase @_;
  $params = {const => $params} if (reftype($params) || '') ne 'HASH';

  my $self = {
    base => $base,
    const => $$params{const} || 0
  };
  bless $self, $class;

  $self->{fullName} = $base->fullName($self->char.($self->{const} ? ' const' : '').'#%s');

  return Gen::Type::_register($self);
}

##################################### Getters

sub isPtrRef
{
  return 1;
}

sub isNormalized
{
  my ($self) = @_;
  return $self->base->isNormalized;
}

sub base
{
  my ($self) = @_;
  return $self->{base};
}

sub const
{
  my ($self) = @_;
  return $self->{const};
}

##################################### Derivative Types

sub withConst
{
  my ($self) = @_;
  return $self if $self->const;
  return $self->{base}->ptrref($self->char, {const => 1});
}

sub withoutConst
{
  my ($self) = @_;
  return $self if !$self->const;
  return $self->{base}->ptrref($self->char);
}

sub normalized
{
  my ($self) = @_;
  return $self if $self->isNormalized;
  return $self->{base}->normalized->ptrref($self->char, $self->const);
}

##################################### Recursive Getters

sub innerMostType
{
  my ($self) = @_;
  return $self->base->innerMostType;
}

sub hasContainer
{
  my ($self) = @_;
  return $self->base->hasContainer;
}

############################################### C++ Pointer Type

package Gen::Type::Pointer;

our @ISA = qw(Gen::Type::PtrRef);

sub new
{
  return Gen::Type::PtrRef::new(@_);
}

##################################### Getters

sub isPointer
{
  return 1;
}

sub char
{
  return '*';
}

##################################### C++ Statements

sub declaration
{
  my ($self, $name, $initialize) = @_;
  $initialize = $initialize || 0;

  my $decl = $self->fullName($name);
  $decl .= ' = new '.$self->base->fullName if $initialize;

  return $decl;
}

############################################### C++ Reference Type

package Gen::Type::Reference;

our @ISA = qw(Gen::Type::PtrRef);

sub new
{
  return Gen::Type::PtrRef::new(@_);
}

##################################### Getters

sub isReference
{
  return 1;
}

sub char
{
  return '&';
}

############################################### C++ Array Type

package Gen::Type::Array;
use Scalar::Util qw(reftype);

our @ISA = qw(Gen::Type);

sub new
{
  my ($class, $base, $params) = Gen::Type::_parseArgsWithBase(@_);
  $params = {size => $params} if (reftype($params) || '') ne 'HASH';

  my $self = {
    base => $base,
    size => (defined $$params{size} ? $$params{size} : -1)
  };
  bless $self, $class;

  $self->{fullName} = $base->fullName('$%s['.$self->sizeString.']');

  return Gen::Type::_register($self);
}

##################################### Getters

sub isArray
{
  return 1;
}

sub isNormalized
{
  my ($self) = @_;
  return $self->base->isNormalized;
}

sub base
{
  my ($self) = @_;
  return $self->{base};
}

sub size
{
  my ($self) = @_;
  return $self->{size};
}

sub sizeString
{
  my ($self) = @_;
  my $size = $self->size;

  return '' if ($size == -1);
  return "$size";
}

##################################### Derivative Types

sub normalized
{
  my ($self) = @_;
  return $self if $self->isNormalized;
  return $self->{base}->normalized->array($self->size);
}

##################################### Recursive Getters

sub innerMostType
{
  my ($self) = @_;
  return $self->base->innerMostType;
}

sub hasContainer
{
  my ($self) = @_;
  return $self->base->hasContainer;
}

############################################### C++ Function Type

package Gen::Type::Function;
use Scalar::Util qw(reftype);

our @ISA = qw(Gen::Type);

sub new
{
  my ($class, $return, $arguments) = Gen::Type::_parseArgsWithArgs(1, @_);

  my $self = {
    returnType => $return,
    arguments => $arguments
  };
  bless $self, $class;

  my $args = Gen::Type::_getArgumentsToString($arguments);

  $self->{fullName} = $return->fullName('$%s('.$args.')');

  return Gen::Type::_register($self);
}

##################################### Getters

sub isFunction
{
  return 1;
}

sub isNormalized
{
  my ($self) = @_;

  return 0 if !$self->returnType->isNormalized;

  foreach my $arg (@{$self->{arguments}})
  {
    if (reftype($arg) eq 'ARRAY')
    {
      return 0;
    }
    return 0 if !$arg->isNormalized;
  }

  return 1;
}

sub returnType
{
  my ($self) = @_;
  return $self->{returnType};
}

# When function is not normalized, some arguments may be arrays of 3 elements: [type, id, init].
sub arguments
{
  my ($self) = @_;
  return $self->{arguments};
}

##################################### Derivative Types

sub normalized
{
  my ($self) = @_;
  return $self if $self->isNormalized;

  my @nArgs = ();
  foreach my $arg_ (@{$self->{arguments}})
  {
    my $arg = $arg_;
    $arg = $$arg[0] if reftype($arg) eq 'ARRAY';
    push @nArgs, $arg->normalized;
  }

  return $self->returnType->normalized->function(@nArgs);
}

##################################### Recursive Getters

sub hasContainer
{
  my ($self) = @_;

  return 1 if $self->returnType->hasContainer;
  foreach my $arg_ (@{$self->arguments})
  {
    my $arg = $arg_;
    $arg = $$arg[0] if reftype($arg) eq 'ARRAY';
    return 1 if $arg->hasContainer;
  }

  return 0;
}

############################################### C++ Template Type

package Gen::Type::Template;
use Gen::Util qw(error);

our @ISA = qw(Gen::Type);

sub new
{
  my ($class, $type, $arguments) = Gen::Type::_parseArgsWithArgs(0, @_);

  error('Wrong base type of template') if !$type->isa('Gen::Type::Normal');

  my $self = {
    name => $type->{name},
    const => $type->{const},
    arguments => $arguments
  };
  bless $self, $class;

  my $args = Gen::Type::_getArgumentsToString($arguments);

  $self->{fullName} = $type->fullName('<'.$args.'>#%s');

  return Gen::Type::_register($self);
}

##################################### Getters

sub isTemplate
{
  return 1;
}

sub isContainer
{
  my ($self) = @_;
  my %Containers = (
    'std::vector' => 1,
    'std::list' => 1,
    'std::set' => 1,
    'std::multiset' => 1,
    'std::map' => 1,
    'std::multimap' => 1
  );
  return $Containers{$self->{name}};
}

sub isNormalized
{
  my ($self) = @_;

  foreach my $arg (@{$self->{arguments}})
  {
    return 0 if !$arg->isNormalized;
  }

  return 1;
}

sub name
{
  my ($self) = @_;
  return $self->{name};
}

sub base
{
  my ($self) = @_;
  error('Type '.$self->fullName.' doesn\'t have any base type') if !$self->isContainer;

  return ${$self->arguments}[1] if $self->name eq 'std::map';
  return ${$self->arguments}[0];
}

sub const
{
  my ($self) = @_;
  return $self->{const};
}

sub arguments
{
  my ($self) = @_;
  return $self->{arguments};
}

##################################### Derivative Types

sub withConst
{
  my ($self) = @_;
  return $self if $self->const;
  return Gen::Type::Normal->new($self->{name}, {const => 1})->template(@{$self->{arguments}});
}

sub withoutConst
{
  my ($self) = @_;
  return $self if !$self->const;
  return Gen::Type::Normal->new($self->{name})->template(@{$self->{arguments}});
}

sub normalized
{
  my ($self) = @_;
  return $self if $self->isNormalized;

  my @nArgs = ();
  foreach my $arg (@{$self->{arguments}})
  {
    push @nArgs, $arg->normalized;
  }

  return Gen::Type::Normal->new($self->{name}, $self->const)->template(@nArgs);
}

##################################### Recursive Getters

sub innerMostType
{
  my ($self) = @_;
  return $self->base->innerMostType if $self->isContainer;
  return $self;
}

sub hasContainer
{
  my ($self) = @_;

  return 1 if $self->isContainer;
  foreach my $arg (@{$self->arguments})
  {
    return 1 if $arg->hasContainer;
  }

  return 0;
}

##################################### C++ Statements

sub insertInstruction
{
  my ($self, $value) = @_;
  error('Type '.$self->fullName.' is not a container') if !$self->isContainer;

  my $ins = $self->_containerInsertMethod."($value)";
  return "$ins";
}

##################################### Internal

sub _containerInsertMethod
{
  my ($self) = @_;

  return 'insert' if $self->name =~ /^std::(?:multi)?(?:set|map)$/;
  return 'push_back';
}

1;
