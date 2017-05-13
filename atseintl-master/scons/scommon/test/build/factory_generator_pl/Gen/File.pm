require Gen::Word;
require Gen::Writer;

############################################### C++ File
# A file can have optional include guard around it (*.h files have it by default).
# The file consists of:
#  - comment block
#  - includes
#  - content
# of which every part is optional (along with include guard).
#
# Comment can include date of creation.
# Includes are grouped into C, C++, other libraries and own (local).

package Gen::File;
use strict;
use warnings;
use Scalar::Util qw(reftype);
use Gen::Util qw(error);

our @ISA = qw(Gen::Writer);

sub new
{
  my ($class, $params) = @_;
  if ((reftype($params) || '') ne 'HASH')
  {
    $params = {name => $params || ''};
  }

  my $self = Gen::Writer->new('');
  bless $self, $class;

  $self->setSkipCallback(sub { return 0; });

  $self->_initGuard();
  $self->_initComment();
  $self->_initIncludes();

  if ($$params{name} ne '')
  {
    $self->setName($$params{name});
  }
  $self->setCommentDateEnable($$params{commentDate} || 0);

  return $self;
}

sub _initGuard
{
  my ($self) = @_;

  $self->{guard} = 0;
  $self->{guardWriter} = Gen::Writer->new('guard');

  $self->{guardWriter}
    ->addCallback('header', sub { $self->_writeGuard($_[0]); })
    ->writeLine()
    ->eatNextNewLine()
    ->addSubWriter($self)
    ->eatPreviousNewLine()
    ->writeLine()
    ->writeLine('#endif');
}

sub _initComment
{
  my ($self) = @_;

  $self->{commentDateEnable} = 0;

  $self->{comment} = Gen::Writer->new('comment');
  $self->{comment}
    ->setSkipCallback(sub { return $self->_skipComment(); })
    ->writeLine('/**')
    ->pushIndent(1)
    ->addCallback('date', sub { $self->_writeDate($_[0]); })
    ->addSubWriter($self->{commentContent} = Gen::Writer->new('content'))
    ->writeLine('*')
    ->writeLine('**/')
    ->popIndent(1)
    ->setEndNewLine(1);

  $self->addSubWriter($self->{comment})
}

sub _initIncludes
{
  my ($self) = @_;

  $self->{includesC} = {};
  $self->{includesCxx} = {};
  $self->{includesOther} = {};
  $self->{includesOwn} = {};

  $self->{includes} = Gen::Writer->new('includes');

  $self->_initInclude('C library', 'C');
  $self->_initInclude('C++ library', 'Cxx');
  $self->_initInclude('other', 'Other');
  $self->_initInclude('own', 'Own');
  $self->{includes}
    ->eatPreviousNewLine()
    ->setEndNewLine(1);

  $self->addSubWriter($self->{includes});
}

sub _initInclude
{
  my ($self, $name, $postfix) = @_;

  my $callback = Gen::Writer::CallbackWriter->new($name,
    sub { $self->_writeIncludes($_[0], $self->{"includes$postfix"}); });
  $callback->setEndNewLine(1);
  $callback->setSkipCallback(sub { return keys %{$self->{"includes$postfix"}} == 0; });
  $self->{includes}->addSubWriter($callback);
}

##################################### Getters

sub name
{
  my ($self) = @_;
  return $self->{name};
}

##################################### Setters

sub setName
{
  my ($self, $name, $guard) = @_;
  if (@_ == 2)
  {
    $guard = ($name =~ /\.h$/);
  }

  $self->{name} = $name;
  $self->{guard} = $guard;

  return $self;
}

sub setCommentDateEnable
{
  my ($self, $e) = @_;
  $self->{commentDateEnable} = $e;
  return $self;
}

##################################### Modifiers

sub addComment
{
  my ($self, $message_) = @_;

  my $message = $message_;
  $message =~ s/\n/ /gm;
  $message =~ s/\s+/ /g;
  $message =~ s/^\s*(.*)\s*$/$1/;

  $self->{commentContent}
    ->writeLine('*')
    ->writeLine(Gen::Word::Stream->new({prefix => '* ', relativeIndent => -2})
      ->addWord('* ')->addCode($message));

  return $self;
}

sub addInclude
{
  my ($self, $include) = @_;

  if ($include =~ /^<(.+)>$/)
  {
    my $file = $1;
    if ($file =~ /^c[^.]+(?!\.h)$/ && $file ne 'complex')
    {
      return $self->addCInclude($file);
    }
    if ($file =~ /^[^.]+(?!\.h)$/)
    {
      return $self->addCxxInclude($file);
    }
    return $self->addLibraryInclude($file);
  }
  if ($include =~ /^"(.+)"$/)
  {
    return $self->addOwnInclude($1);
  }

  error("Malformed include: $include");
}

sub addCInclude
{
  my ($self, $include) = @_;
  ${$self->{includesC}}{"<$include>"} = 1;
  return $self;
}

sub addCxxInclude
{
  my ($self, $include) = @_;
  ${$self->{includesCxx}}{"<$include>"} = 1;
  return $self;
}

sub addLibraryInclude
{
  my ($self, $include) = @_;
  ${$self->{includesOther}}{"<$include>"} = 1;
  return $self;
}

sub addOwnInclude
{
  my ($self, $include) = @_;
  ${$self->{includesOwn}}{"\"$include\""} = 1;
  return $self;
}

##################################### Overriden Gen::Writer methods

sub save
{
  my ($self, $fileName_) = @_;
  my $fileName = (defined $fileName_ ? $fileName_ : $self->{name});

  return $self->{guardWriter}->save($fileName) if $self->{guard};
  return Gen::Writer::save($self, $fileName);
}

sub saveToString
{
  my ($self) = @_;

  return $self->{guardWriter}->saveToString() if $self->{guard};
  return Gen::Writer::saveToString($self);
}

##################################### Internal

sub _writeGuard
{
  my ($self, $writer, $prefix) = @_;

  my $guard = lcfirst($self->{name});
  $guard =~ s/[.\/]/_/g;
  $guard =~ s/([A-Z])/_$1/g;
  $guard = uc($guard);

  $writer
    ->writeCode("#ifndef $guard")
    ->writeCode("#define $guard");
}

sub _skipComment
{
  my ($self) = @_;
  return !$self->{commentDateEnable} && $self->{commentContent}->empty;
}

sub _writeDate
{
  my ($self, $writer) = @_;
  return if !$self->{commentDateEnable};

  my ($sec, $min, $hour, $mday, $mon, $year) = localtime(time);
  $year += 1900;
  $mon += 1;

  my $stream = Gen::Word::Stream->new()->setPrefix('* ');
  $stream->addCode(sprintf('* This file has been generated on %d-%02d-%02d at %02d:%02d:%02d.',
                           $year, $mon, $mday, $hour, $min, $sec));

  $writer
    ->writeLine('*')
    ->writeLine($stream);
}

sub _writeIncludes
{
  my ($self, $writer, $hash) = @_;

  my @keys = sort(keys(%$hash));
  foreach my $include (@keys)
  {
    $writer->writeCode("#include $include");
  }
}

1;
