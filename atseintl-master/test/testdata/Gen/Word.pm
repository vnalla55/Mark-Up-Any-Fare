############################################### Word
# Word is a string of text that must be written continously. It's used by Writer to control
# wrapping. You can chunk piece of code into Words automatically using Writer::writeCode or
# Word::WordStream::addCode. It splits roughly according to C++ rules and simple logic.
#
# Every word has some state associated with it, the most important one being content.
# Word->space controls white-space placement between words. It's used only when two words
# are written on the same line.
# Word->anchor and Word->relativeIndent indicate where the current word and consecutive ones
# should start after they wrapped to the next line. Anchor is a Word which end shall mark
# the start of indent, and relativeIndent can move it by signed integer amount.
# Word->postfix/prefix select what should put at the end of current line/start of next line,
# respectievely, when wrapping.
#
# anchor, relativeIndent, prefix and postfix states are transitive; it means that in a stream
# of words they are used until the next change. So you can set prefix of only the first word
# to set it in the entire stream.

package Gen::Word;
use strict;
use warnings;

sub new
{
  my ($class, $word) = @_;

  my $self =
  {
    word            => '',
    space           => '',
    anchor          => undef,
    relativeIndent  => 0,
    prefix          => undef,
    postfix         => undef
  };
  bless $self, $class;

  return $self->setWord($word);
}

##################################### Getters

sub word
{
  my ($self) = @_;
  return $self->{word};
}

sub space
{
  my ($self) = @_;
  return $self->{space};
}

sub anchor
{
  my ($self) = @_;
  return $self->{anchor};
}

sub relativeIndent
{
  my ($self) = @_;
  return $self->{relativeIndent};
}

sub prefix
{
  my ($self) = @_;
  return $self->{prefix};
}

sub postfix
{
  my ($self) = @_;
  return $self->{postfix};
}

##################################### Setters

sub setWord
{
  my ($self, $word) = @_;
  my $space = '';
  if ($word =~ /^(.+)( )$/)
  {
    $word = $1;
    $space = $2;
  }

  $self->{word} = $word;
  $self->{space} = $space;
  return $self;
}

sub setIndent
{
  my $self = shift;
  my $relativeIndent = shift;
  my $anchor = shift;

  $self->{anchor} = $anchor;
  $self->{relativeIndent} = $relativeIndent;

  return $self;
}

sub setPrefix
{
  my ($self, $prefix) = @_;
  $self->{prefix} = $prefix;
  return $self;
}

sub setPostfix
{
  my ($self, $postfix) = @_;
  $self->{postfix} = $postfix;
  return $self;
}

############################################### Word Stream
# This class is used to easily menage a stream of words. Sub-streams can help structure
# the output. addCode tokenizes string into words on its own, adhering to simplified C++ logic.
# Use $stream->words to extract Word array out of stream.

package Gen::Word::Stream;
use strict;
use warnings;
use Scalar::Util qw(blessed);

sub new
{
  my ($class, $params) = @_;
  $params = $params || {};

  my $self =
  {
    words           => [],
    anchor          => $$params{anchor} || undef,
    relativeIndent  => $$params{relativeIndent} || 0,
    prefix          => $$params{prefix} || undef,
    postfix         => $$params{postfix} || undef
  };
  return bless $self, $class;
}

##################################### Getter

sub words
{
  my ($self) = @_;
  $self->_normalize();
  return $self->{words};
}

sub empty
{
  my ($self) = @_;
  return @{$self->{words}} == 0;
}

sub lastWord
{
  my ($self) = @_;
  return ${$self->{words}}[-1];
}

##################################### Setters

sub setIndent
{
  my $self = shift;
  my $relativeIndent = shift;
  my $anchor = shift;

  $self->{anchor} = $anchor;
  $self->{relativeIndent} = $relativeIndent;

  return $self;
}

sub setPrefix
{
  my ($self, $prefix) = @_;
  $self->{prefix} = $prefix;
  return $self;
}

sub setPostfix
{
  my ($self, $postfix) = @_;
  $self->{postfix} = $postfix;
  return $self;
}

##################################### Modifier

sub addWord
{
  my ($self, $word_) = @_;
  my $word = $word_;
  if (!blessed($word) || !$word->isa('Gen::Word'))
  {
    $word = Gen::Word->new($word);
  }

  $word->setIndent($self->{relativeIndent}, $self->{anchor}) if defined $self->{anchor} ||
                                                                $self->{relativeIndent} != 0;
  $self->{anchor} = undef;
  $self->{relativeIndent} = 0;

  $word->setPrefix($self->{prefix}) if defined $self->{prefix};
  $self->{prefix} = undef;

  $word->setPostfix($self->{postfix}) if defined $self->{postfix};
  $self->{postfix} = undef;

  push @{$self->{words}}, $word;
  return $self;
}

sub addCode
{
  my ($self, $string_) = @_;
  my $string = $string_;

  my $characters = '\\(|\\[|\\{|::|->';

  # Remove stray whitespace.
  $string =~ s/^\s*(.*)\s*$/$1/;

  # Escape string.
  $string =~ s/"((?:[^"\\]|\\.)*)"/'"'._escapeChars($characters, $1).'"'/eg;

  # Mark additional word boundaries.
  $string =~ s/(?<!\\)($characters|\s)/$1\@;\@-\@/g;

  # Split string into words.
  my @words = split(/(?:\@-\@)+/, $string);
  my $last = pop @words;

  # Fixup words' endings.
  foreach my $word (@words)
  {
    if ($word !~ s/\@;$//)
    {
      $word = "$word ";
    }
  }
  $last =~ s/\@;$//;
  push @words, $last;

  foreach my $word (@words)
  {
    # Unescape words.
    $word =~ s/"((?:[^"\\]|\\.)*)"/'"'._unescapeChars($1).'"'/eg;

    $self->addWord($word);
  }

  return $self;
}

sub addSubStream
{
  my ($self, $stream) = @_;
  push @{$self->{words}}, $stream;
  return $self;
}

##################################### Internal

sub _normalize
{
  my ($self) = @_;

  my @nWords = ();

  my $touched = 0;
  foreach my $word (@{$self->{words}})
  {
    if (blessed($word) && $word->isa('Gen::Word::Stream'))
    {
      $word->_normalize();
      my $words = $word->words;
      push @nWords, @$words;
      $touched = 1;
    }
    else
    {
      push @nWords, $word;
    }
  }
  $self->{words} = \@nWords if $touched;

  return $self;
}

sub _escapeChars
{
  my ($characters, $string) = @_;
  $string =~ s/($characters|\\|\s)/\\$1/g;
  return $string;
}

sub _unescapeChars
{
  my ($string) = @_;
  $string =~ s/\\(.)/$1/g;
  return $string;
}

1;
