############################################### Utility functions

package Gen::Util;
use strict;
use warnings;
use Exporter 'import';
use File::Basename 'dirname';
use Cwd 'abs_path';

our @EXPORT_OK = qw(error errorHandler registerErrorHandler substituteString substituteUnaryFunction
                    wrapIn extractParens extractArgument splitTypename isFactoryType);

##################################### Error helpers
# To prepend every error with custom message (to give some context to error),
# save previous handler using errorHandler and register new one using registerErrorHandler.
# At the end, restore old handler using registerErrorHandler again.

my $errorHandler = sub {
  die 'Error: '.$_[0].".\n";
};

sub error
{
  my ($msg) = @_;
  $errorHandler->($msg);
}

sub errorHandler
{
  return $errorHandler;
}

sub registerErrorHandler
{
  my ($newHandler) = @_;

  my $oldHandler = $errorHandler;
  $errorHandler = $newHandler;

  return $oldHandler;
}

##################################### Substitution helpers
# These functions substitute $... in the string.
# The first one, changes $0, $1, ... into value. There is a special syntax "${0%fmt}" that
# substitutes using proper sprintf format.
#
# The second one works on unary functions, like $foo(arg), and calls callBack('arg').

sub substituteString
{
  my ($string, $nr, $value) = @_;

  $string =~ s/\$$nr/$value/g;

  while ($string =~ m/\$\{$nr(%[^}]+)\}/g)
  {
    my $expr = $1;
    my $pos = pos($string);
    $expr = sprintf($expr, $value);
    substr $string, $-[0], $+[0]-$-[0], $expr;
    pos($string) = $pos + length($expr) - ($+[0]-$-[0]);
  }

  return $string;
}

sub substituteUnaryFunction
{
  my ($string, $name, $callBack) = @_;

  while ($string =~ m/\$$name\(\s*([^)]+)\s*\)/g)
  {
    my $expr = $1;
    my $pos = pos($string);

    $expr = $callBack->($expr);

    substr $string, $-[0], $+[0]-$-[0], $expr;
    pos($string) = $pos + length($expr) - ($+[0]-$-[0]);
  }

  return $string;
}

##################################### String helpers
# These two functions count parentheses ('()' and '<>').
# The first one must be called on a string that begins with '(' or '<'. it return a pair
# of what is inside and outside (after closing ')' or '>') of parentheses.
#
# The second one extracts another element of comma-separated argument list. It properly
# handles the case of parentheses, like "7, (8, 9), 10" which has 3 elements.
# It returns a pair of next argument and the rest of list (or '').

sub extractParens
{
  my ($string) = @_;

  my $inside = '';
  $string =~ s/^(.)(.*)$/$2/;
  my @open = ($1 eq '<' ? '>' : ')');

  while ($string =~ /^([^()<>]*)([()<>])(.*)$/)
  {
    $inside .= $1;
    my $paren = $2;
    $string = $3;

    if ($paren eq '<')
    {
      push @open, '>';
    }
    elsif ($paren eq '(')
    {
      push @open, ')';
    }
    elsif ($paren ne $open[-1])
    {
      error('No matching parentheses');
    }
    else
    {
      pop @open;
      if (@open == 0)
      {
        return ($inside, $string);
      }
    }

    $inside .= $paren;
  }
  error('No matching parentheses');
}

sub extractArgument
{
  my ($string) = @_;

  my $arg = '';

outer:
  while ($string =~ /^([^,]+),(.+)$/)
  {
    $arg .= $1;
    $string = $2;

    my $test = $arg;
    while ($test =~ /^([^<(]*)([<(].*)$/)
    {
      my $error = 0;
      my $oldHandler = registerErrorHandler(sub { $error = 1; });
      my (undef, $outside) = extractParens($2);
      registerErrorHandler($oldHandler);

      next outer if $error;

      $test = $outside;
    }
    return ($arg, $string);
  }
  continue { $arg .= ','; }

  return ($arg.$string, '');
}

# Conditionally wraps a string in $before and $after.
sub wrapIn
{
  my ($condition, $before, $content, $after) = @_;
  return $content if !$condition;
  return $before.$content.$after;
}

##################################### Type helpers

# Split Normal type name (like int, std::string) into last '::' component and everything before.
sub splitTypename
{
  my ($variableType) = @_;

  my @array = split(/::/, $variableType);
  my $actualType = pop @array;
  my $outerType = join('::', @array);

  return ($outerType, $actualType);
}

sub _findFile
{
  my ($directory, $file) = @_;

  open my $fh, "find '${directory}' -name '${file}'|";
  my $found = !eof($fh);
  close($fh);

  return $found;
}

# Returns whether the type is factory type or not.
sub isFactoryType
{
  my ($type) = @_;
  my (undef, $actualType) = splitTypename($type);

  my $vobdir = abs_path(dirname(__FILE__).'/../../..');

  return 1 if _findFile("$vobdir/test/testdata/defs", "$actualType.def");
  return 1 if _findFile("$vobdir/test/testdata", "Test${actualType}Factory.cpp");

  return 0;
}

1;
