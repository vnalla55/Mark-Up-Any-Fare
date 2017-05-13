require Gen::Type;
require Gen::Variable;
require Gen::Writer;

############################################### *.def File Parser
# Parses *.def file fireing proper callbacks alogn the way. Use process() method to parse the file.
#
# Some def syntax is considered deprecated. You can control its behaviour using deprecatedState.
# The default, WARNED, warns of stdout about usage, but still accepts them. ALLOWED is similar,
# but doesn't warn at all. On the other hand, DISALLOWED emits error when it encounters any.

package Gen::DefParser;
use strict;
use warnings;
use Gen::Util qw(error substituteString splitTypename isFactoryType);

our $DEPRECATED_ALLOWED     = 0;
our $DEPRECATED_WARNED      = 1;
our $DEPRECATED_DISALLOWED  = 2;

sub new
{
  my ($class, $fileName) = @_;

  my $self =
  {
    fileName                => $fileName,
    lineNumber              => 0,
    deprecatedState         => $DEPRECATED_WARNED,

    # CallBacks
    initDoneCB              => \&_noop,
    specialAggregateBeginCB => \&_noop,
    specialAggregateEndCB   => \&_noop,
    specialAggregateReadCB  => \&_noop,
    specialAggregateWriteCB => \&_noop,
    specialSectionBeginCB   => \&_noop,
    specialSectionEndCB     => \&_noop,
    specialSectionReadCB    => \&_noop,
    specialSectionWriteCB   => \&_noop,
    specialFieldCB          => \&_noop,
    sectionBeginCB          => \&_noop,
    sectionEndCB            => \&_noop,
    scalarCB                => \&_noop,
    scalarAggregateCB       => \&_noop,
    scalarCompareCB         => \&_noop,
    factoryAggregateCB      => \&_noop,
    factoryCompareCB        => \&_noop,
    preInitCB               => \&_noop,
    postInitCB              => \&_noop,
    checkItemCB             => \&_noop,
    includeCB               => \&_noop,
    nameValueCB             => \&_noop,
    nameFieldCB             => \&_noop,

    ### Internal
    # Lists
    accessors               => {},
    sections                => {},

    # Special Lines
    inSpecialLines          => 0,
    specialLinesName        => '',
    specialLinesEndCallBack => undef,
    specialLinesContent     => undef,
    specialLinesIndent      => 0,

    # Initialization parameters
    className               => '',
    abstract                => 0,
    namespace               => '',
    classPackage            => '',
    typeToPackage           => {},
    outerClass              => '',
    parentFactory           => '',
    children                => {},
    constructorArgs         => [],

    # Special Aggregates
    specialAggregate        => '',
    specialAggregateIsSection => 0,
    specialAggregateReadLoop => undef,
    specialAggregateWriteLoop => undef,

    # Sections
    section                 => '',
  };

  return bless $self, $class;
}

##################################### Getters

sub fileName
{
  my ($self) = @_;
  return $self->{fileName};
}

sub lineNumber
{
  my ($self) = @_;
  return $self->{lineNumber};
}

sub position
{
  my ($self) = @_;
  return $self->{fileName}.':'.$self->{lineNumber};
}

##################################### Setters

sub setDeprecated
{
  my ($self, $deprecatedState) = @_;
  $self->{deprecatedState} = $deprecatedState;
  return $self;
}

##################################### Callback setters

# initDone($self, {className, abstract, namespace, classPackage, typeToPackage, outerClass,
#                  parentFactory, children, constructorArgs})
sub setInitDone
{
  my ($self, $cb) = @_;
  $self->{initDoneCB} = $cb;
  return $self;
}

# specialAggregateBegin($self, {sectionName, attributeName, variable})
sub setSpecialAggregateBegin
{
  my ($self, $cb) = @_;
  $self->{specialAggregateBeginCB} = $cb;
  return $self;
}

# specialAggregateEnd($self, {sectionName})
sub setSpecialAggregateEnd
{
  my ($self, $cb) = @_;
  $self->{specialAggregateEndCB} = $cb;
  return $self;
}

# specialAggregateRead($self, {sectionName, attributeName, content})
sub setSpecialAggregateRead
{
  my ($self, $cb) = @_;
  $self->{specialAggregateReadCB} = $cb;
  return $self;
}

# specialAggregateWrite($self, {sectionName, attributeName, content})
sub setSpecialAggregateWrite
{
  my ($self, $cb) = @_;
  $self->{specialAggregateWriteCB} = $cb;
  return $self;
}

# specialSectionBegin($self, {sectionName, attributeName, variable})
sub setSpecialSectionBegin
{
  my ($self, $cb) = @_;
  $self->{specialSectionBeginCB} = $cb;
  return $self;
}

# specialSectionEnd($self, {sectionName})
sub setSpecialSectionEnd
{
  my ($self, $cb) = @_;
  $self->{specialSectionEndCB} = $cb;
  return $self;
}

# specialSectionRead($self, {sectionName, attributeName, content})
sub setSpecialSectionRead
{
  my ($self, $cb) = @_;
  $self->{specialSectionReadCB} = $cb;
  return $self;
}

# specialSectionWrite($self, {sectionName, attributeName, content})
sub setSpecialSectionWrite
{
  my ($self, $cb) = @_;
  $self->{specialSectionWriteCB} = $cb;
  return $self;
}
# specialField($self, {section, variable, init, omitSetter,})
sub setSpecialField
{
  my ($self, $cb) = @_;
  $self->{specialFieldCB} = $cb;
  return $self;
}

# sectionBegin($self, {sectionName})
sub setSectionBegin
{
  my ($self, $cb) = @_;
  $self->{sectionBeginCB} = $cb;
  return $self;
}

# sectionEnd($self, {sectionName})
sub setSectionEnd
{
  my ($self, $cb) = @_;
  $self->{sectionEndCB} = $cb;
  return $self;
}

# scalar($self, {sectionName, attributeName, accessor, inputType})
sub setScalar
{
  my ($self, $cb) = @_;
  $self->{scalarCB} = $cb;
  return $self;
}

# scalarAggregate($self, {sectionName, accessor, inputType})
sub setScalarAggregate
{
  my ($self, $cb) = @_;
  $self->{scalarAggregateCB} = $cb;
  return $self;
}

# scalarCompare($self, {accessor})
sub setScalarCompare
{
  my ($self, $cb) = @_;
  $self->{scalarCompareCB} = $cb;
  return $self;
}

# factoryAggregate($self, {sectionName, accessor, inputType, willDelete})
sub setFactoryAggregate
{
  my ($self, $cb) = @_;
  $self->{factoryAggregateCB} = $cb;
  return $self;
}

# factoryCompare($self, {accessor})
sub setFactoryCompare
{
  my ($self, $cb) = @_;
  $self->{factoryCompareCB} = $cb;
  return $self;
}

# preInit($self, {content})
sub setPreInit
{
  my ($self, $cb) = @_;
  $self->{preInitCB} = $cb;
  return $self;
}

# postInit($self, {content})
sub setPostInit
{
  my ($self, $cb) = @_;
  $self->{postInitCB} = $cb;
  return $self;
}

# checkItem($self, {content})
sub setCheckItem
{
  my ($self, $cb) = @_;
  $self->{checkItemCB} = $cb;
  return $self;
}

# include($self, {file})
sub setInclude
{
  my ($self, $cb) = @_;
  $self->{includeCB} = $cb;
  return $self;
}

# nameValue($self, {value})
sub setNameValue
{
  my ($self, $cb) = @_;
  $self->{nameValueCB} = $cb;
  return $self;
}


# nameField($self, {field})
sub setNameField
{
  my ($self, $cb) = @_;
  $self->{nameFieldCB} = $cb;
  return $self;
}

##################################### Error handling

sub _assertOutsideBlock
{
  my ($self, $name) = @_;

  if ($self->{specialAggregate} ne '')
  {
    error("Unsupported $name inside special aggregate");
  }
  if ($self->{section} ne '')
  {
    error("Unsupported $name inside section");
  }
}

##################################### Deprecated handling

sub _deprecated
{
  my ($self, $name, $instead) = @_;
  return if $self->{deprecatedState} == $DEPRECATED_ALLOWED;

  my $message = "Deprecated $name; you should use $instead instead";
  if ($self->{deprecatedState} == $DEPRECATED_WARNED)
  {
    print $self->position().": Warning: $message.\n";
    return;
  }

  error($message);
}

##################################### Default No-op callback

sub _noop
{
}

##################################### Accessor & Section management

sub _addAccessor
{
  my ($self, $accessor) = @_;

  if (${$self->{accessors}}{$accessor})
  {
    return ${$self->{accessors}}{$accessor};
  }

  foreach my $arg (@{$self->{constructorArgs}})
  {
    if ($arg eq $accessor)
    {
      return ${$self->{accessors}}{$accessor} = $Gen::Accessor::ACCESSOR_CONSTRUCT;
    }
  }

  return ${$self->{accessors}}{$accessor} = $Gen::Accessor::ACCESSOR_INIT;
}

sub _addSection
{
  my ($self, $section) = @_;

  if (${$self->{sections}}{$section})
  {
    error('Duplicate section');
  }

  ${$self->{sections}}{$section} = 1;
}

##################################### Special Lines

sub _specialLinesBegin
{
  my ($self, $name, $callBack) = @_;

  error('Script logic error') if $self->{inSpecialLines};
  $self->{inSpecialLines} = 1;

  $self->{specialLinesContent} = undef;
  $self->{specialLinesEndCallBack} = $callBack;
  $self->{specialLinesName} = $name;
}

sub _specialLinesEnd
{
  my ($self) = @_;

  $self->{inSpecialLines} = 0;
  if ($self->{specialLinesContent})
  {
    $self->{specialLinesEndCallBack}->($self, $self->{specialLinesContent});
  }
}

sub _specialLinesHandle
{
  my ($self, $line) = @_;
  my $specialLinesEndRegExp = 'End'.$self->{specialLinesName};

  if ($line =~ /^\s*$specialLinesEndRegExp\s*$/i)
  {
    return $self->_specialLinesEnd();
  }

  # Calculate source indent.
  $line =~ /^(\s*)\S.*$/;
  my $indent = length($1);

  if (!$self->{specialLinesContent})
  {
    $self->{specialLinesIndent} = $indent;
    $self->{specialLinesContent} = Gen::Writer->new($self->{specialLinesName});
  }

  # We remove as many spaces as we can.
  if ($self->{specialLinesIndent} < $indent)
  {
    $indent = $self->{specialLinesIndent}
  }
  my $spaces = ' ' x $indent;
  $line =~ s/^$spaces//;

  $self->{specialLinesContent}->writeLine($line);
}

##################################### Initialization

sub _initDone
{
  my ($self) = @_;
  my @children = sort keys %{$self->{children}};

  error('No name given at the end of initialization') if $self->{className} eq '';
  if ($self->{abstract})
  {
    error('No children of abstract class given') if @children == 0;
    error('Constructor for abstract class specified') if @{$self->{constructorArgs}} > 0;
  }

  $self->{initDoneCB}->($self, {
      className       => $self->{className},
      abstract        => $self->{abstract},
      namespace       => $self->{namespace},
      classPackage    => $self->{classPackage},
      typeToPackage   => $self->{typeToPackage},
      outerClass      => $self->{outerClass},
      parentFactory   => $self->{parentFactory},
      children        => \@children,
      constructorArgs => $self->{constructorArgs}
  });
}

##################################### Special Aggregate & Section

sub _specialAggregateBegin
{
  my ($self, $sectionName, $attributeName, $variable, $isSection) = @_;
  my $aType = $isSection ? 'Section' : 'Aggregate';

  $self->_assertOutsideBlock($isSection ? 'special section' : 'special aggregate');
  $self->_addSection($sectionName);

  if ($attributeName eq '')
  {
    $attributeName = 'value';
  }

  $self->{specialAggregate} = $sectionName;
  $self->{specialAggregateAttribute} = $attributeName;
  $self->{specialAggregateIsSection} = $isSection;
  $self->{specialAggregateReadLoop} = undef;
  $self->{specialAggregateWriteLoop} = undef;

  $self->{"special${aType}BeginCB"}->($self, {
    sectionName   => $sectionName,
    attributeName => $attributeName,
    variable      => $variable
  });
}

sub _specialAggregateEnd
{
  my ($self) = @_;
  my $aType = $self->{specialAggregateIsSection} ? 'Section' : 'Aggregate';

  if ($self->{specialAggregate} eq '')
  {
    error('Ending special without opening it');
  }

  my $sectionName = $self->{specialAggregate};
  $self->{specialAggregate} = '';

  $self->{"special${aType}EndCB"}->($self, {sectionName => $sectionName});
}

sub _specialAggregateCodeEnd
{
  my ($self, $type, $content) = @_;

  if ($self->{specialAggregate} eq '')
  {
    error("$type section outside of special aggregate");
  }

  my $aType = $self->{specialAggregateIsSection} ? 'Section' : 'Aggregate';

  $self->{"special${aType}${type}CB"}->($self, {
    sectionName   => $self->{specialAggregate},
    attributeName => $self->{specialAggregateAttribute},
    content       => $content
  });
}

sub _specialAggregateLoopCodeEnd
{
  my ($self, $type, $content) = @_;

  if ($self->{specialAggregate} eq '')
  {
    error("$type loop section outside of special aggregate");
  }
  if ($self->{"specialAggregate${type}Loop"})
  {
    error("$type loop section already defined");
  }

  $self->{"specialAggregate${type}Loop"} = $content;
}

sub _specialAggregateLoopItem
{
  my ($self, $loopItem) = @_;
  my $aType = $self->{specialAggregateIsSection} ? 'Section' : 'Aggregate';

  if ($self->{specialAggregate} eq '')
  {
    error('Loop item outside of special aggregate');
  }
  if (!$self->{specialAggregateReadLoop} && !$self->{specialAggregateWriteLoop})
  {
    error('Loop item before read & write loop sections');
  }

  my @loopItems = split(/\s*\|\s*/, $loopItem);

  foreach my $type (qw(Read Write))
  {
    next if !$self->{"specialAggregate${type}Loop"};
    my $content = $self->{"specialAggregate${type}Loop"}->clone();
    $content->setName($type);

    $content->forEachWord(sub {
      my (undef, $word) = @_;
      my $i = 1;

      foreach my $loopItem (@loopItems)
      {
        $word->setWord(substituteString($word->word, $i, $loopItem));
      }
      continue { ++$i; }
    });

    $self->_specialAggregateCodeEnd($type, $content);
  }
}

sub _specialAggregateLoopItemNumbers
{
  my ($self, $numbers) = @_;

  if ($numbers =~ /^(\d+)$/)
  {
    return $self->_specialAggregateLoopItem($1);
  }

  if ($numbers =~ /^(\d+):(\d+)$/)
  {
    my $i = $1;
    my $end = $2;
    if ($i > $end)
    {
      error('Malformed number sequence');
    }

    while ($i <= $end)
    {
      $self->_specialAggregateLoopItem($i++);
    }
  }
  else
  {
    error('Malformed number sequence');
  }
}

##################################### Special Fields

sub _specialField
{
  my ($self, $variable, $init, $public, $omitSetter) = @_;

  $self->_assertOutsideBlock('special field');

  $self->{specialFieldCB}->($self, {
    section     => $public ? 'public' : 'private',
    variable    => $variable,
    init        => $init,
    omitSetter  => $omitSetter
  });
}

##################################### Sections

sub _sectionBegin
{
  my ($self, $sectionName) = @_;

  $self->_assertOutsideBlock('section');
  $self->_addSection($sectionName);

  $self->{section} = $sectionName;

  $self->{sectionBeginCB}->($self, {sectionName => $sectionName});
}

sub _sectionEnd
{
  my ($self) = @_;

  if ($self->{section} eq '')
  {
    error('Ending section without opening it');
  }

  my $sectionName = $self->{section};
  $self->{section} = '';

  $self->{sectionEndCB}->($self, {sectionName => $sectionName});
}

##################################### Type helper

sub _getTypes
{
  my ($self, $variableType, $input) = @_;

  my $type = Gen::Type->new($variableType);
  my $inputType = ($input ne '' ? Gen::Type->new($input) : $type);

  return ($type, $inputType);
}

sub _parseAccessor
{
  my ($self, $decl) = @_;

  my $inputType = undef;
  if ($decl =~ /\s*Input\s*=\s*(.+)\s*$/i)
  {
    $inputType = Gen::Type->new($1);
    $decl =~ s/\s*Input\s*=.+$//i;
  }

  if ($decl !~ /^\s*(.+)\s+(\S+)\s*$/)
  {
    error('Invalid accessor type');
  }

  my $id = $2;
  my $type = Gen::Type->new($1);
  my $accessorType = $self->_addAccessor($id);

  my $accessor = Gen::Accessor->new({
    name => $id,
    type => $type,
    accessorType => $accessorType
  });

  return ($accessor, $inputType);
}

sub _parseLocalWithDefault
{
  my ($self, $decl) = @_;

  my $init = '';
  if ($decl =~ /^[^=]*\s*=\s*(.+)\s*$/)
  {
    $init = $1;
    $decl =~ s/^([^=]*)\s*=.+$/$1/;
  }

  return ($self->_parseLocal($decl), $init);
}

sub _parseLocal
{
  my ($self, $decl) = @_;
  my ($type, $id) = Gen::Type::parse($decl);

  return Gen::LocalVariable->new({
    name => $id,
    type => $type
  });
}

##################################### Scalars

sub _scalar
{
  my ($self, $attributeName, $accessor, $inputType, $withCompare) = @_;

  my $sectionName = $self->{section};
  $sectionName = $self->{specialAggregate} if $self->{specialAggregate} ne '';

  $self->{scalarCB}->($self, {
    sectionName   => $sectionName,
    attributeName => $attributeName,
    accessor      => $accessor,
    inputType     => $inputType
  });

  if ($withCompare)
  {
    $accessor = $accessor->clone($inputType);
    $self->{scalarCompareCB}->($self, {
      accessor  => $accessor
    });
  }
}

sub _scalarAggregate
{
  my ($self, $sectionName, $accessor, $inputType, $withCompare) = @_;
  $self->_assertOutsideBlock('scalar aggregate');
  $self->_addSection($sectionName);

  $self->{scalarAggregateCB}->($self, {
    sectionName   => $sectionName,
    accessor      => $accessor,
    inputType     => $inputType
  });

  if ($withCompare)
  {
    $accessor = $accessor->clone($inputType);
    $self->{scalarCompareCB}->($self, {
      accessor  => $accessor
    });
  }
}

sub _scalarCompare
{
  my ($self, $accessor) = @_;
  $self->_assertOutsideBlock('scalar compare');

  $self->{scalarCompareCB}->($self, {
    accessor  => $accessor
  });
}

##################################### Factories

sub _factoryAggregate
{
  my ($self, $sectionName, $accessor, $inputType, $willDelete, $withCompare) = @_;
  $self->_assertOutsideBlock('factory aggregate');
  $self->_addSection($sectionName);

  $self->{factoryAggregateCB}->($self, {
    sectionName   => $sectionName,
    accessor      => $accessor,
    inputType     => $inputType,
    willDelete    => $willDelete
  });

  if ($withCompare)
  {
    $accessor = $accessor->clone($inputType);
    $self->{factoryCompareCB}->($self, {
      accessor  => $accessor
    });
  }
}

sub _factoryCompare
{
  my ($self, $accessor) = @_;
  $self->_assertOutsideBlock('factory compare');

  $self->{factoryCompareCB}->($self, {
    accessor  => $accessor
  });
}

##################################### Attributes (Deprecated)

sub _directAttribute
{
  my ($self, $attributeName, $variableType, $accessorName, $pointer, $cast) = @_;

  $self->_assertOutsideBlock('direct attribute');
  $self->_deprecated('direct attribute', 'scalar');

  my $accessorType = $self->_addAccessor($accessorName);

  my ($type, $inputType) = $self->_getTypes($variableType, $cast);
  $type = $type->pointer if $pointer;

  my $accessor = Gen::Accessor->new({
    name => $accessorName,
    type => $type,
    accessorType => $accessorType
  });

  $self->{scalarCB}->($self, {
    sectionName   => '',
    attributeName => $attributeName,
    accessor      => $accessor,
    inputType     => $inputType
  });
}

sub _attribute
{
  my ($self, $attributeName, $variableType, $accessorName, $pointer, $cast) = @_;

  if ($self->{section} eq '' &&
      ($self->{specialAggregate} eq '' || !$self->{specialAggregateIsSection}))
  {
    error('Unexpected attribute outside of section');
  }
  $self->_deprecated('attribute', 'scalar');

  my $accessorType = $self->_addAccessor($accessorName);

  my ($type, $inputType) = $self->_getTypes($variableType, $cast);
  $type = $type->pointer if $pointer;

  my $accessor = Gen::Accessor->new({
    name => $accessorName,
    type => $type,
    accessorType => $accessorType
  });

  $self->{scalarCB}->($self, {
    sectionName   => $self->{section},
    attributeName => $attributeName,
    accessor      => $accessor,
    inputType     => $inputType
  });
}

sub _attributeContainer
{
  my ($self, $sectionName, $container, $variableType, $accessorName, $pointer, $const, $cast) = @_;

  $self->_assertOutsideBlock($container.($pointer ? 'of pointers ' : '').' attribute');
  $self->_deprecated('container of attributes', 'scalar aggregate');

  my $accessorType = $self->_addAccessor($accessorName);
  $self->_addSection($sectionName);

  my ($type, $inputType) = $self->_getTypes($variableType, $cast);
  $type = $type->withConst if $const;
  $type = $type->pointer if $pointer;
  $type = Gen::Type->template('std::'.$container, $type);

  my $accessor = Gen::Accessor->new({
    name => $accessorName,
    type => $type,
    accessorType => $accessorType
  });

  $self->{scalarAggregateCB}->($self, {
    sectionName   => $sectionName,
    accessor      => $accessor,
    inputType     => $inputType
  });
}

sub _attributeCompare
{
  my ($self, $variableType, $accessorName, $pointer) = @_;

  $self->_assertOutsideBlock('attribute compare');
  $self->_deprecated('attribute compare', 'scalar compare');

  my $type = Gen::Type->type($variableType);
  $type = $type->pointer if $pointer;

  my $accessor = Gen::Accessor->new({
    name => $accessorName,
    type => $type
  });

  $self->{scalarCompareCB}->($self, {
    accessor  => $accessor
  });
}

sub _attributeContainerCompare
{
  my ($self, $container, $variableType, $accessorName, $pointer, $const) = @_;

  $self->_assertOutsideBlock($container.($pointer ? 'of pointers ' : '').' attribute compare');
  $self->_deprecated($container.($pointer ? 'of pointers ' : '').' attribute compare',
                     'scalar compare');

  my $type = Gen::Type->type($variableType);
  $type = $type->withConst if $const;
  $type = $type->pointer if $pointer;
  $type = Gen::Type->template('std::'.$container, $type);

  my $accessor = Gen::Accessor->new({
    name => $accessorName,
    type => $type
  });

  $self->{scalarCompareCB}->($self, {
    accessor  => $accessor
  });
}

##################################### Aggregates (Deprecated)

sub _aggregate
{
  my ($self, $sectionName, $variableType, $accessorName, $pointer, $cast, $willDelete) = @_;

  $self->_assertOutsideBlock(($pointer ? 'pointer ' : '').'aggregate');
  $self->_deprecated('aggregate', 'factory aggregate');

  my $accessorType = $self->_addAccessor($accessorName);
  $self->_addSection($sectionName);

  my ($type, $inputType) = $self->_getTypes($variableType, $cast);
  $type = $type->pointer if $pointer;

  my $accessor = Gen::Accessor->new({
    name => $accessorName,
    type => $type,
    accessorType => $accessorType
  });

  $self->{factoryAggregateCB}->($self, {
    sectionName => $sectionName,
    accessor    => $accessor,
    inputType   => $inputType,
    willDelete  => $willDelete
  });
}

sub _aggregateContainer
{
  my ($self, $sectionName, $container, $variableType, $accessorName, $pointer, $const, $cast,
      $willDelete) = @_;

  $self->_assertOutsideBlock($container.($pointer ? 'of pointers ' : '').' aggregate');

  my (undef, $actualType) = splitTypename($variableType);
  if (!isFactoryType($actualType))
  {
    $self->_deprecated('container of non-factory aggregates', 'container of attributes');
    error('The option `willDelete` is not true') if !$willDelete;

    return $self->_attributeContainer($sectionName, $container, $variableType, $accessorName,
                                      $pointer, $const, $cast);
  }
  $self->_deprecated('container of aggregates', 'factory aggregate');

  my $accessorType = $self->_addAccessor($accessorName);
  $self->_addSection($sectionName);

  my ($type, $inputType) = $self->_getTypes($variableType, $cast);
  $type = $type->withConst if $const;
  $type = $type->pointer if $pointer;
  $type = Gen::Type->template('std::'.$container, $type);

  my $accessor = Gen::Accessor->new({
    name => $accessorName,
    type => $type,
    accessorType => $accessorType
  });

  $self->{factoryAggregateCB}->($self, {
    sectionName => $sectionName,
    accessor    => $accessor,
    inputType   => $inputType,
    willDelete  => $willDelete
  });
}

sub _aggregateCompare
{
  my ($self, $variableType, $accessorName, $pointer) = @_;

  $self->_assertOutsideBlock('aggregate compare');
  $self->_deprecated('aggregate compare', 'factory compare');

  my $type = Gen::Type->type($variableType);
  $type = $type->pointer if $pointer;

  my $accessor = Gen::Accessor->new({
    name => $accessorName,
    type => $type
  });

  $self->{factoryCompareCB}->($self, {
    accessor  => $accessor
  });
}

sub _aggregateContainerCompare
{
  my ($self, $container, $variableType, $accessorName, $pointer, $const) = @_;

  $self->_assertOutsideBlock($container.($pointer ? 'of pointers ' : '').' aggregate compare');
  $self->_deprecated($container.($pointer ? 'of pointers ' : '').' aggregate compare',
                     'factory compare');

  my $type = Gen::Type->type($variableType);
  $type = $type->withConst if $const;
  $type = $type->pointer if $pointer;
  $type = Gen::Type->template('std::'.$container, $type);

  my $accessor = Gen::Accessor->new({
    name => $accessorName,
    type => $type
  });

  $self->{factoryCompareCB}->($self, {
    accessor  => $accessor
  });
}

##################################### Pre & Post Init and Check Item lines

sub _preInitLinesEnd
{
  my ($self, $content) = @_;

  $self->{preInitCB}->($self, {content => $content});
}

sub _postInitLinesEnd
{
  my ($self, $content) = @_;

  $self->{postInitCB}->($self, {content => $content});
}

sub _checkItemLinesEnd
{
  my ($self, $content) = @_;

  $self->{checkItemCB}->($self, {content => $content});
}

##################################### Includes

sub _extraInclude
{
  my ($self, $file) = @_;

  $self->{includeCB}->($self, {file => $file});
}

##################################### Names

sub _nameValue
{
  my ($self, $value) = @_;

  $self->{nameValueCB}->($self, {value => $value});
}

sub _nameField
{
  my ($self, $field) = @_;

  $self->{nameFieldCB}->($self, {field => $field});
}

##################################### Parser main loop

sub process
{
  my ($self) = @_;

  $self->{lineNumber} = 1;
  my $oldHandler = Gen::Util::errorHandler;
  Gen::Util::registerErrorHandler(sub {
    $oldHandler->($self->position().': '.$_[0]);
  });

  open(my $fh, '<', $self->{fileName}) || error($!);

  $self->_process($fh);

  close($fh);
  Gen::Util::registerErrorHandler($oldHandler);
}

sub processFromString
{
  my ($self, $input) = @_;

  $self->{lineNumber} = 1;
  my $oldHandler = Gen::Util::errorHandler;
  Gen::Util::registerErrorHandler(sub {
    $oldHandler->($self->position().': '.$_[0]);
  });

  open(my $fh, '<', \$input) || error($!);

  $self->_process($fh);

  close($fh);
  Gen::Util::registerErrorHandler($oldHandler);
}

sub _process
{
  my ($self, $fh) = @_;
  my $initStage = 0;

  while (my $line = <$fh>)
  {
    chomp($line);
    $line =~ s/^([^#]*)\s*#.*$/$1/;

    if ($line =~ /^\s*$/)
    {
      next;
    }

    if ($self->{inSpecialLines})
    {
      $self->_specialLinesHandle($line);
      next;
    }

    ##### Handle the initialization type of lines.
    if ($initStage == 0)
    {
      if ($line =~ /^\s*Name:\s*(\S+)(?:\s+(Abstract))?\s*$/i)
      {
        error('Double name') if $self->{className} ne '';

        $self->{className} = $1;
        $self->{abstract} = $2 ? 1 : 0;
      }
      elsif ($line =~ /^\s*Namespace:\s*(\S+)\s*$/i)
      {
        error('Double namespace') if $self->{namespace} ne '';

        $self->{namespace} = $1;
      }
      elsif ($line =~ /^\s*Package:\s*([\/\w]+)?\s*$/i)
      {
        error('Double package') if $self->{classPackage} ne '';

        $self->{classPackage} = $1;
      }
      elsif ($line =~ /^\s*TypePackage:\s*(\S+)\s+(\S+)\s*$/i)
      {
        ${$self->{typeToPackage}}{$2} = $1;
      }
      elsif ($line =~ /^\s*(Parent|Outer)Class:\s*(\w+)?\s*$/i)
      {
        error('Double parent class') if $self->{outerClass} ne '';
        $self->_deprecated('parent class', 'outer class') if lc($1) eq 'parent';

        $self->{outerClass} = $2;
      }
      elsif ($line =~ /^\s*Parent:\s*(\w+)?\s*$/i)
      {
        error('Double parent') if $self->{parentFactory} ne '';

        $self->{parentFactory} = $1;
      }
      elsif ($line =~ /^\s*Child:\s*(\w+)\s*$/i)
      {
        error('Double child') if ${$self->{children}}{$1};

        ${$self->{children}}{$1} = 1;
      }
      elsif ($line =~ /^\s*Constructor:\s*((?:[^ \t,]+\s*,\s*)*\S+)\s*$/i)
      {
        error('Double constructor') if @{$self->{constructorArgs}};

        my @args = split(/\s*,\s*/, $1);
        $self->{constructorArgs} = \@args;
      }
      else
      {
        # Not an initialization line, so assume we're done.
        $self->_initDone();
        $initStage = 1;
      }
    }

    if ($initStage == 1)
    {
      ##### End of initialization stuff.

      if ($line =~ /^\s*Special(Public)?Field(OmitSetter)?:\s*(.+)\s*$/i)
      {
        my $public = $1 ? 1 : 0;
        my $omitSetter = $2 ? 1 : 0;
        my ($variable, $init) = $self->_parseLocalWithDefault($3);

        $self->_specialField($variable, $init, $public, $omitSetter);
      }
      elsif ($line =~ /^\s*Section:\s*(\w+)\s*$/i)
      {
        my $sectionName = $1;

        $self->_sectionBegin($sectionName);
      }
      elsif ($line =~ /^\s*EndSection\s*$/i)
      {
        $self->_sectionEnd();
      }
      elsif ($line =~ /^\s*Scalar(WithCompare)?:\s*(\S+)\s*:\s*(.+)\s*$/i)
      {
        my $withCompare = ($1 ? 1 : 0);
        my $attributeName = $2;
        my ($accessor, $inputType) = $self->_parseAccessor($3);

        $self->_scalar($attributeName, $accessor, $inputType, $withCompare);
      }
      elsif ($line =~ /^\s*ScalarAggregate(WithCompare)?:\s*(\S+)\s*:\s*(.+)\s*$/i)
      {
        my $withCompare = ($1 ? 1 : 0);
        my $sectionName = $2;
        my ($accessor, $inputType) = $self->_parseAccessor($3);

        $self->_scalarAggregate($sectionName, $accessor, $inputType, $withCompare);
      }
      elsif ($line =~ /^\s*ScalarCompare:\s*(.+)\s*$/i)
      {
        my ($accessor, undef) = $self->_parseAccessor($1);

        $self->_scalarCompare($accessor);
      }
      elsif ($line =~ /^\s*FactoryAggregate(WithCompare)?:\s*(\S+)\s*:\s*(.+)\s*$/i)
      {
        my $withCompare = ($1 ? 1 : 0);
        my $sectionName = $2;
        my $accessorExpr = $3;
        my $willDelete = 1;
        if ($accessorExpr =~ /OwnsPointer\s*$/i)
        {
          $accessorExpr =~ s/\s*OwnsPointer\s*$//i;
          $willDelete = 0;
        }
        my ($accessor, $inputType) = $self->_parseAccessor($accessorExpr);

        $self->_factoryAggregate($sectionName, $accessor, $inputType, $willDelete, $withCompare);
      }
      elsif ($line =~ /^\s*FactoryCompare:\s*(.+)\s*$/i)
      {
        my ($accessor, undef) = $self->_parseAccessor($1);

        $self->_factoryCompare($accessor);
      }
      # Deprecated attributes and aggregates:
      elsif ($line =~ /^\s*DirectAttribute:\s*(\S+)\s+(\S+)\s+(\S+)\s*$/i)
      {
        my $attributeName = $2;
        my $variableType = $1;
        my $accessor = $3;
        my $pointer = 0;
        my $cast = '';

        $self->_directAttribute($attributeName, $variableType, $accessor, $pointer, $cast);
      }
      elsif ($line =~ /^\s*Attribute:\s*(\S+)\s+(\S+)\s+(\S+)\s*$/i)
      {
        my $variableType = $1;
        my $attributeName = $2;
        my $accessor = $3;
        my $pointer = 0;
        my $cast = '';

        $self->_attribute($attributeName, $variableType, $accessor, $pointer, $cast);
      }
      elsif ($line =~ /^\s*(Vector|List|Set)Attribute:\s*(\S+)\s+(\S+)\s+(\S+)\s*$/i)
      {
        my $sectionName = $2;
        my $container = lc($1);
        my $variableType = $3;
        my $accessor = $4;
        my $pointer = 0;
        my $const = 0;
        my $cast = '';

        $self->_attributeContainer($sectionName, $container, $variableType, $accessor, $pointer,
                                   $const, $cast);
      }
      elsif ($line =~ /^\s*(Vector|List|Set)Of(Const)?PointersAttribute:\s*(\S+)\s+(\S+)\s+(\S+)\s*$/i)
      {
        my $sectionName = $3;
        my $container = lc($1);
        my $variableType = $4;
        my $accessor = $5;
        my $pointer = 1;
        my $const = $2 ? 1 : 0;
        my $cast = '';

        $self->_attributeContainer($sectionName, $container, $variableType, $accessor, $pointer,
                                   $const, $cast);
      }
      elsif ($line =~ /^\s*Aggregate:\s*(\S+)\s+(\S+)\s+(\S+)\s*$/i)
      {
        my $sectionName = $1;
        my $variableType = $2;
        my $accessor = $3;
        my $pointer = 0;
        my $cast = '';
        my $willDelete = 1;

        $self->_aggregate($sectionName, $variableType, $accessor, $pointer, $cast, $willDelete);
      }
      elsif ($line =~ /^\s*PointerAggregate:\s*(\S+)\s+(\S+)\s+(\S+)\s*$/i)
      {
        my $sectionName = $1;
        my $variableType = $2;
        my $accessor = $3;
        my $pointer = 1;
        my $cast = '';
        my $willDelete = 1;

        $self->_aggregate($sectionName, $variableType, $accessor, $pointer, $cast, $willDelete);
      }
      elsif ($line =~ /^\s*PointerAggregateCast:\s*(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s*$/i)
      {
        my $sectionName = $1;
        my $variableType = $2;
        my $accessor = $3;
        my $pointer = 1;
        my $cast = $4;
        my $willDelete = 1;

        $self->_aggregate($sectionName, $variableType, $accessor, $pointer, $cast, $willDelete);
      }
      elsif ($line =~ /^\s*(Vector|List|Set)Aggregate:\s*(\S+)\s+(\S+)\s+(\S+)\s*$/i)
      {
        my $sectionName = $2;
        my $container = lc($1);
        my $variableType = $3;
        my $accessor = $4;
        my $pointer = 0;
        my $const = 0;
        my $cast = '';
        my $willDelete = 1;

        $self->_aggregateContainer($sectionName, $container, $variableType, $accessor, $pointer,
                                   $const, $cast, $willDelete);
      }
      elsif ($line =~ /^\s*(Vector|List|Set)Of(Const)?PointersAggregate:\s*(\S+)\s+(\S+)\s+(\S+)(?:\s+(\S+))?\s*$/i)
      {
        my $sectionName = $3;
        my $container = lc($1);
        my $variableType = $4;
        my $accessor = $5;
        my $pointer = 1;
        my $const = $2 ? 1 : 0;
        my $cast = '';
        my $willDelete = ($6 ? ($6 eq 'true' ? 1 : 0) : 1);

        $self->_aggregateContainer($sectionName, $container, $variableType, $accessor, $pointer,
                                   $const, $cast, $willDelete);
      }
      elsif ($line =~ /^\s*KeyField(Pointer)?:\s*(\S+)\s*$/i)
      {
        my $variableType = '';
        my $accessor = $2;
        my $pointer = $1 ? 1 : 0;

        $self->_attributeCompare($variableType, $accessor, $pointer);
      }
      elsif ($line =~ /^\s*KeyField(Pointer)?Aggregate:\s*(\S+)\s+(\S+)\s*$/i)
      {
        my $variableType = $2;
        my $accessor = $3;
        my $pointer = $1 ? 1 : 0;

        $self->_aggregateCompare($variableType, $accessor, $pointer);
      }
      elsif ($line =~ /^\s*KeyField(Vector|List|Set)(Pointer)?Aggregate:\s*(\S+)\s+(\S+)\s*$/i)
      {
        my $container = lc($1);
        my $variableType = $3;
        my $accessor = $4;
        my $pointer = $2 ? 1 : 0;
        my $const = 0;

        $self->_aggregateContainerCompare($container, $variableType, $accessor, $pointer, $const);
      }
      # End of deprecated
      elsif ($line =~ /^\s*Special(Aggregate|Section):\s*(\S+)(?:\s+(\S+))?\s*:\s*(.+)\s*$/i)
      {
        my $sectionName = $2;
        my $attributeName = $3 || '';
        my $variable = $self->_parseLocal($4);
        $variable->setName('value') if !$variable->name;
        my $isSection = (lc($1) eq 'section' ? 1 : 0);

        $self->_specialAggregateBegin($sectionName, $attributeName, $variable, $isSection);
      }
      elsif ($line =~ /^\s*EndSpecial\s*$/i)
      {
        $self->_specialAggregateEnd();
      }
      elsif ($line =~ /^\s*Loop:\s*((?:[^,]+\s*,\s*)*[^,]+)\s*$/i)
      {
        my @loopItems = split(/\s*,\s*/, $1);

        foreach my $loopItem (@loopItems)
        {
          $self->_specialAggregateLoopItem($loopItem);
        }
      }
      elsif ($line =~ /^\s*LoopNumbers:\s*((?:[^ \t,]+\s*,\s*)*\S+)\s*$/i)
      {
        my @loopItems = split(/\s*,\s*/, $1);

        foreach my $loopItem (@loopItems)
        {
          $self->_specialAggregateLoopItemNumbers($loopItem);
        }
      }
      elsif ($line =~ /^\s*Read\s*$/i)
      {
        $self->_specialLinesBegin('Read',
          sub { $self->_specialAggregateCodeEnd('Read', $_[1]); });
      }
      elsif ($line =~ /^\s*Write\s*$/i)
      {
        $self->_specialLinesBegin('Write',
          sub { $self->_specialAggregateCodeEnd('Write', $_[1]); });
      }
      elsif ($line =~ /^\s*ReadLoop\s*$/i)
      {
        $self->_specialLinesBegin('ReadLoop',
          sub { $self->_specialAggregateLoopCodeEnd('Read', $_[1]); });
      }
      elsif ($line =~ /^\s*WriteLoop\s*$/i)
      {
        $self->_specialLinesBegin('WriteLoop',
          sub { $self->_specialAggregateLoopCodeEnd('Write', $_[1]); });
      }
      elsif ($line =~ /^\s*InitLine:(.*)\s*$/i)
      {
        my $codeLine = $1;

        $self->_deprecated('InitLine', '[End]InitLines');
        $self->_preInitLinesEnd(Gen::Writer->new()->writeLine($codeLine));
      }
      elsif ($line =~ /^\s*InitLines\s*$/i)
      {
        $self->_specialLinesBegin('InitLines', \&_preInitLinesEnd);
      }
      elsif ($line =~ /^\s*PostInitLine:(.*)\s*$/i)
      {
        my $codeLine = $1;

        $self->_deprecated('PostInitLine', '[End]PostInitLines');
        $self->_postInitLinesEnd(Gen::Writer->new()->writeLine($codeLine));
      }
      elsif ($line =~ /^\s*PostInitLines\s*$/i)
      {
        $self->_specialLinesBegin('PostInitLines', \&_postInitLinesEnd);
      }
      elsif ($line =~ /^\s*ExtraInclude:\s*(.*)\s*$/i)
      {
        my $file = $1;

        $self->_extraInclude($file);
      }
      elsif ($line =~ /^\s*NameString:\s*(.*)\s*$/i)
      {
        my $value = $1;

        $self->_nameValue($value);
      }
      elsif ($line =~ /^\s*NameField:\s*(\S+)\s*$/i)
      {
        my $field = $1;

        $self->_nameField($field);
      }
      elsif ($line =~ /^\s*CheckItem:(.*)\s*$/i)
      {
        my $codeLine = $1;

        $self->_deprecated('CheckItem', '[End]CheckItemLines');
        $self->_checkItemLinesEnd(Gen::Writer->new()->writeLine($codeLine));
      }
      elsif ($line =~ /^\s*CheckItemLines\s*$/i)
      {
        $self->_specialLinesBegin('CheckItemLines', \&_checkItemLinesEnd);
      }
      else
      {
        error("Unrecognized line: '${line}'");
      }
    }
  }
  continue
  {
    ++$self->{lineNumber};
  }

  if ($initStage != 1)
  {
    $self->_initDone();
  }
}

1;
