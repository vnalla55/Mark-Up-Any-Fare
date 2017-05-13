require Gen::Field;
require Gen::Function;
require Gen::Word;
require Gen::Writer;

############################################### C++ Class
# It's simple class, lacking inheritence, etc. It has 3 visibility sections, where you can
# put Fields and Functions (or constructors).

package Gen::Class;
use strict;
use warnings;

sub new
{
  my ($class, $name) = @_;

  my $self =
  {
    name        => undef,
    declaration => undef,
    definition  => undef,
    public      => undef,
    protected   => undef,
    private     => undef
  };
  bless $self, $class;

  $self->{public} = Gen::Class::Section->new('public')->setClass($self);
  $self->{protected} = Gen::Class::Section->new('protected')->setClass($self);
  $self->{private} = Gen::Class::Section->new('private')->setClass($self);

  $self->{declaration} = Gen::Writer->new('');
  $self->{declaration}
    ->addCallback('head', sub { $self->_writeHead($_[0]); })
    ->pushBlock('{')
    ->addSubWriter($self->{public}->declaration)
    ->addSubWriter($self->{protected}->declaration)
    ->addSubWriter($self->{private}->declaration)
    ->popBlock('};')
    ->setEndNewLine(1);

  $self->{definition} = Gen::Writer->new('');
  $self->{definition}
    ->addSubWriter($self->{public}->definition)
    ->addSubWriter($self->{protected}->definition)
    ->addSubWriter($self->{private}->definition);

  $self->setName(defined $name ? $name : '');

  return $self;
}

##################################### Getters

sub name
{
  my ($self) = @_;
  return $self->{name};
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

sub public
{
  my ($self) = @_;
  return $self->{public};
}

sub protected
{
  my ($self) = @_;
  return $self->{protected};
}

sub private
{
  my ($self) = @_;
  return $self->{private};
}

##################################### Setters

sub setSkipCallback
{
  my ($self, $cb) = @_;
  $self->declaration->setSkipCallback($cb);
  $self->definition->setSkipCallback($cb);
  return $self;
}

sub setName
{
  my ($self, $name) = @_;
  $self->{name} = $name;

  $self->{declaration}->setName("$name declaration");
  $self->{definition}->setName("$name definition");
  return $self;
}

##################################### Internal

sub _writeHead
{
  my ($self, $writer) = @_;
  my $name = $self->{name};

  $writer->writeCode("class $name");
}

############################################### C++ Class Section

package Gen::Class::Section;
use strict;
use warnings;
use Scalar::Util qw(blessed weaken);

sub new
{
  my ($class, $section) = @_;

  my $self =
  {
    section       => $section,
    class         => undef,
    declaration   => undef,
    definition    => undef,
    constructors  => undef,
    functions     => undef,
    fields        => undef
  };
  bless $self, $class;

  $self->{constructors} = [Gen::Writer->new('constructors'), Gen::Writer->new('constructors')];
  $self->{functions} = [Gen::Writer->new('functions'), Gen::Writer->new('functions')];
  $self->{fields} = [Gen::Writer->new('fields'), Gen::Writer->new('fields')];

  $self->{declaration} = Gen::Writer->new($section);
  $self->{declaration}
    ->setSkipCallback(sub { return $self->_skip(); })
    ->pushIndent(-2)
    ->writeLine("$section:")
    ->popIndent(-2)
    ->addSubWriter(${$self->{constructors}}[0]->setEndNewLine(1)->eatEndNewLine(1))
    ->addSubWriter(${$self->{functions}}[0]->setEndNewLine(1)->eatEndNewLine(1))
    ->addSubWriter(${$self->{fields}}[0]->setEndNewLine(1)->eatEndNewLine(1))
    ->eatPreviousNewLine()
    ->setEndNewLine(1);

  $self->{definition} = Gen::Writer->new($section);
  $self->{definition}
    ->addSubWriter(${$self->{constructors}}[1])
    ->addSubWriter(${$self->{functions}}[1])
    ->addSubWriter(${$self->{fields}}[1]);

  return $self;
}

##################################### Getters

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

sub setClass
{
  my ($self, $class) = @_;

  $self->{class} = $class;
  weaken $self->{class};

  return $self;
}

##################################### Internal

sub _skip
{
  my ($self) = @_;
  return ${$self->{constructors}}[0]->empty &&
         ${$self->{functions}}[0]->empty &&
         ${$self->{fields}}[0]->empty;
}

##################################### Methods

# Constructor is a function without name or with a name equal to class name.
# You can skip body of a function (e.g. to represent disabling constructor).
sub addFunction
{
  my ($self, $function_, $skipBody) = @_;
  $skipBody = $skipBody || 0;

  my $function = $function_;
  if (!blessed($function) || !$function->isa('Gen::Function'))
  {
    $function = Gen::Function->new($function);
  }

  my $className = $self->{class}->name;
  my $tab = $self->{functions};
  if ($function->name eq '' || $function->name eq $className)
  {
    $function->setReturnType('');
    $function->setName($className);
    $tab = $self->{constructors};
  }

  $function
    ->setSection($self->{section})
    ->setName("${className}::".$function->name);

  $$tab[0]->addSubWriter($function->declaration);
  if (!$skipBody)
  {
    $$tab[1]->addSubWriter($function->definition);
  }

  return $self;
}

sub addField
{
  my ($self, $field) = @_;

  if (!blessed($field) || !$field->isa('Gen::Field'))
  {
    $field = Gen::Field->new($field);
  }

  my $className = $self->{class}->name;
  my $tab = $self->{fields};

  $field
    ->setSection($self->{section})
    ->setName("${className}::".$field->name);

  $$tab[0]->addSubWriter($field->declaration);

  $$tab[1]->addSubWriter($field->definition) if $field->hasModifier('static');

  return $self;
}

1;
