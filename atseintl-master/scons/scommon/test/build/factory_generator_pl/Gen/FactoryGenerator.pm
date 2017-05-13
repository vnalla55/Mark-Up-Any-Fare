require File::Spec;
require Gen::Class;
require Gen::FactorySectionRead;
require Gen::FactorySectionWrite;
require Gen::Field;
require Gen::File;
require Gen::Function;
require Gen::Variable;
require Gen::Writer;

############################################### Test*Factory.* Generator
# This class generates Test*Factory.{h,cpp} files. It uses FactorySection{Read,Write}
# for construct/init and write methods.

package Gen::FactoryGenerator;
use strict;
use warnings;
use Gen::Util qw(error substituteUnaryFunction splitTypename isFactoryType);

sub new
{
  my ($class, $params) = @_;

  my $self =
  {
    # Factory parameters
    className       => $$params{className},
    abstract        => $$params{abstract} || 0,
    namespace       => $$params{namespace} || '',
    classPackage    => $$params{classPackage} || '',
    typeToPackage   => $$params{typeToPackage} || {},
    outerClass      => $$params{outerClass} || '',
    parentFactory   => $$params{parentFactory} || '',
    children        => $$params{children} || [],
    constructorArgs => $$params{constructorArgs} || [],

    # Derived parameters
    fullClassName   => '',

    # Files
    headerFile      => undef,
    sourceFile      => undef,

    # Class
    class           => undef,

    ### Functions
    constuctWriter  => undef,
    preInitWriter   => undef,
    initWriter      => undef,
    postInitWriter  => undef,
    writeWriter     => undef,
    writeLoopWriter => undef,
    compareWriter   => undef,
    checkItemWriter => undef,
    getNameWriter   => undef,
  };

  bless $self, $class;
  $self->_initDerivedParameters();
  $self->_initClass();
  $self->_initHeaderFile();
  $self->_initSourceFile();
  return $self;
}

sub _initDerivedParameters
{
  my ($self) = @_;

  if ($self->{namespace} ne '')
  {
    if ($self->{outerClass} ne '')
    {
      $self->{fullClassName} = $self->{namespace}.'::'.$self->{outerClass}.'::'.$self->{className};
    }
    else
    {
      $self->{fullClassName} = $self->{namespace}.'::'.$self->{className};
    }
  }
  else
  {
    $self->{fullClassName} = $self->{className};
  }
}

sub _initClass
{
  my ($self) = @_;

  $self->{class} = Gen::Class->new($self->_factoryName($self->{className}));

  $self->_initClearCacheFunction();
  $self->_initCreateFunctions();
  $self->_initConstructFunction();
  $self->_initPreInitFunction();
  $self->_initInitFunction();
  $self->_initPostInitFunction();
  $self->_initWriteFunctions();
  $self->_initWriteFunction();
  $self->_initCompareFunction();
  $self->_initCheckItemFunction();
  $self->_initGetNameFunction();

  # Private constructor, effectively disable creation of factory.
  $self->{class}->private->addFunction({}, 1);
}

sub _initClearCacheFunction
{
  my ($self) = @_;
  my $fullClassName = $self->{fullClassName};
  my $factoryName = $self->_factoryName($self->{className});
  my $function;

  $self->{class}->public->addFunction($function = Gen::Function->new({
    modifiers => ['static'], signature => 'void clearCache()'}));

  $function->body->writeCode(<<LINES
    TestFactoryBase<${factoryName}>::destroyAll<${fullClassName}>();
LINES
  );
}

sub _initCreateFunctions
{
  my ($self) = @_;
  my $className = $self->{className};
  my $fullClassName = $self->{fullClassName};
  my $factoryName = $self->_factoryName($self->{className});
  my $function;

  $self->{class}->public->addFunction($function = Gen::Function->new({
    modifiers => ['static'], signature =>
      "$fullClassName* create(const std::string& fileName, bool unique = false)"}));

  $function->body->writeCode(<<LINES
    using namespace TestFactoryFlags;
    return ${factoryName}::create(fileName, unique ? UNIQUE : DEFAULT);
LINES
  );


  $self->{class}->public->addFunction($function = Gen::Function->new({
    modifiers => ['static'], signature =>
      "$fullClassName* create(const std::string& fileName, int flags)"}));

  if (@{$self->{children}} != 0)
  {
    $function->body->writeCode("${fullClassName}* item = 0;");
    foreach my $child (@{$self->{children}})
    {
      my $childFactory = $self->_factoryName($child);
      $function->body->writeCode(<<LINES
        if ((item = ${childFactory}::create(fileName, flags)))
          return item;
LINES
      );
    }
  }

  if ($self->{abstract})
  {
    $function->body->writeCode('return 0;');
  }
  else
  {
    $function->body->writeCode(<<LINES
      return TestFactoryBase<${factoryName}>::create<${fullClassName}>(fileName, "${className}", flags);
LINES
    );
  }


  $self->{class}->public->addFunction($function = Gen::Function->new({
    modifiers => ['static'], signature =>
      "$fullClassName* create(const std::string& fileName, TiXmlElement* rootElement, int flags)"}));

  $function->body->writeCode(<<LINES
    return TestFactoryBase<${factoryName}>::create<${fullClassName}>(fileName, rootElement, flags);
LINES
  );
}

sub _initConstructFunction
{
  my ($self) = @_;
  my $fullClassName = $self->{fullClassName};
  my $function;

  $self->{class}->public->addFunction($function = Gen::Function->new({
    modifiers => ['static'], signature =>
      "$fullClassName* construct(const std::string& fileName, TiXmlElement* rootElement, int flags)"}));

  $function->body->writeCode('using namespace TestFactoryFlags;');
  $self->{constructWriter} = Gen::FactorySectionRead->new({
    name         => 'construct',
    deferAllSets => 1,
    omitFinalSet => 1
  });
  $function->body->addSubWriter($self->{constructWriter}->setEndNewLine(1));

  if ($self->{abstract})
  {
    $function->body->writeCode('return 0;');
  }
  else
  {
    if (@{$self->{constructorArgs}} == 0)
    {
      $function->body->writeCode("return new ${fullClassName};");
    }
    else
    {
      $function->body->writeCode("return new ${fullClassName}(");
      $function->body->pushIndent();

      my $count = @{$self->{constructorArgs}};
      for (my $i = 0; $i < $count; ++$i)
      {
        $function->body->addCallback('argument '.($i+1), $self->_bindConstructorArg($i));
      }

      $function->body->popIndent();
      $function->body->writeCode(");");
    }
  }
}

sub _initPreInitFunction
{
  my ($self) = @_;
  my $fullClassName = $self->{fullClassName};
  my $function;

  $self->{class}->public->addFunction($function = Gen::Function->new({
    modifiers => ['static'], signature => "void preInit($fullClassName* item)"}));

  $function->body->eatNextNewLine();

  if ($self->{parentFactory})
  {
    my $parentFactory = $self->_factoryName($self->{parentFactory});
    $function->body->writeCode(<<LINES
      ${parentFactory}::preInit(item);
LINES
    );
  }

  my $body = $self->{preInitWriter} = Gen::Writer->new('pre init');
  $function->body->addSubWriter($body->setBeginNewLine(1));
}

sub _initInitFunction
{
  my ($self) = @_;
  my $fullClassName = $self->{fullClassName};
  my $function;

  $self->{class}->public->addFunction($function = Gen::Function->new({
    modifiers => ['static'], signature =>
      "void init($fullClassName* item, const std::string& fileName, TiXmlElement* rootElement,
                 int flags)"}));

  $function->body->eatNextNewLine();

  $function->body->writeCode('using namespace TestFactoryFlags;');
  if ($self->{parentFactory})
  {
    my $parentFactory = $self->_factoryName($self->{parentFactory});
    $function->body->writeCode(<<LINES
      ${parentFactory}::init(item, fileName, rootElement, flags);
LINES
    );
  }

  my $body = $self->{initWriter} = Gen::FactorySectionRead->new({name => 'init'});
  $function->body->addSubWriter($body->setBeginNewLine(1));
}

sub _initPostInitFunction
{
  my ($self) = @_;
  my $fullClassName = $self->{fullClassName};
  my $function;

  $self->{class}->public->addFunction($function = Gen::Function->new({
    modifiers => ['static'], signature => "void postInit($fullClassName* item)"}));

  $function->body->eatNextNewLine();

  if ($self->{parentFactory})
  {
    my $parentFactory = $self->_factoryName($self->{parentFactory});
    $function->body->writeCode(<<LINES
      ${parentFactory}::postInit(item);
LINES
    );
  }

  my $body = $self->{postInitWriter} = Gen::Writer->new('post init');
  $function->body->addSubWriter($body->setBeginNewLine(1));
}

sub _initWriteFunctions
{
  my ($self) = @_;
  my $className = $self->{className};
  my $fullClassName = $self->{fullClassName};
  my $factoryName = $self->_factoryName($className);
  my $function;

  $self->{class}->public->addFunction($function = Gen::Function->new({
    modifiers => ['static'], signature =>
      "void write(const std::string& fullPathName, const $fullClassName& item)"}));

  foreach my $child (@{$self->{children}})
  {
    my $childFactory = $self->_factoryName($child);
    $function->body->writeCode(<<LINES
      if (dynamic_cast<const ${child}*> (&item))
      {
        ${childFactory}::write(fullPathName, static_cast<const ${child}&> (item));
        return;
      }
LINES
    );
  }

  if (!$self->{abstract})
  {
    $function->body->writeCode(<<LINES
      TestFactoryBase<${factoryName}>::write<${fullClassName}>(fullPathName, item);
LINES
    );
  }

  $self->{class}->public->addFunction($function = Gen::Function->new({
    modifiers => ['static'], signature =>
      "void writeSubItem(const std::string& sectionName, const $fullClassName& item,
                         TiXmlNode* node, const std::string& filePrefix)"}));

  $function->body->writeCode(<<LINES
    if (!checkItem(item))
      return;
LINES
  );

  foreach my $child (@{$self->{children}})
  {
    my $childFactory = $self->_factoryName($child);
    $function->body->writeCode(<<LINES
      if (dynamic_cast<const ${child}*> (&item))
      {
        ${childFactory}::writeSubItem(sectionName, static_cast<const ${child}&> (item), node, filePrefix);
        return;
      }
LINES
    );
  }

  if (!$self->{abstract})
  {
    $function->body->writeCode(<<LINES
      TestFactoryBase<${factoryName}>::writeSubItem<${fullClassName}>(sectionName, item, node, filePrefix, "${className}");
LINES
    );
  }

  $self->{class}->public->addFunction($function = Gen::Function->new({
    modifiers => ['static'], signature =>
      "void write(const $fullClassName& item, TiXmlNode* node, const std::string& filePrefix)"}));

  foreach my $child (@{$self->{children}})
  {
    my $childFactory = $self->_factoryName($child);
    $function->body->writeCode(<<LINES
      if (dynamic_cast<const ${child}*> (&item))
      {
        ${childFactory}::write(static_cast<const ${child}&> (item), node, filePrefix);
        return;
      }
LINES
    );
  }

  if (!$self->{abstract})
  {
    $function->body->writeCode(<<LINES
      TestFactoryBase<${factoryName}>::write<${fullClassName}>("$className", item, node, filePrefix);
LINES
    );
  }
}

sub _initWriteFunction
{
  my ($self) = @_;
  my $fullClassName = $self->{fullClassName};
  my $function;

  $self->{class}->public->addFunction($function = Gen::Function->new({
    modifiers => ['static'], signature =>
      "void write(const $fullClassName& item, TiXmlElement* rootElement,
                  const std::string& filePrefix)"}));

  my $body = $self->{writeWriter} = Gen::FactorySectionWrite->new({});
  $function->body->addSubWriter($body);

  if ($self->{parentFactory})
  {
    my $parentFactory = $self->_factoryName($self->{parentFactory});
    $function->body->writeCode(<<LINES
      ${parentFactory}::write(item, rootElement, filePrefix);
LINES
    );
  }
}

sub _initCompareFunction
{
  my ($self) = @_;
  my $fullClassName = $self->{fullClassName};
  my $function;

  $self->{class}->public->addFunction($function = Gen::Function->new({
    modifiers => ['static'], signature =>
      "bool compare(const $fullClassName* i1, const $fullClassName* i2)"}));

  $function->body->writeCode(<<LINES
      if (i1 == i2)
        return true;
LINES
    );

  foreach my $child (@{$self->{children}})
  {
    my $childFactory = $self->_factoryName($child);
    $function->body->writeCode(<<LINES
      if (dynamic_cast<const ${child}*> (i1) && dynamic_cast<const ${child}*> (i2))
        return ${childFactory}::compare(static_cast<const ${child}*> (i1), static_cast<const ${child}*> (i2));
LINES
    );
  }

  my $body = $self->{compareWriter} = Gen::Writer->new('compare');
  $function->body->addSubWriter($body->setBeginNewLine(1));
  $function->body->addCallback('final return', sub { $self->_compareFinalReturn($_[0]); });
}

sub _initCheckItemFunction
{
  my ($self) = @_;
  my $fullClassName = $self->{fullClassName};
  my $function;

  $self->{class}->public->addFunction($function = Gen::Function->new({
    modifiers => ['static'], signature => "bool checkItem(const $fullClassName& item)"}));

  $function->body->writeCode(<<LINES
      if (!&item)
        return false;
LINES
    );

  my $body = $self->{checkItemWriter} = Gen::Writer->new('check item');
  $function->body
    ->addSubWriter($body)
    ->writeCode('return true;');
}

sub _initGetNameFunction
{
  my ($self) = @_;
  my $fullClassName = $self->{fullClassName};
  my $function;

  $self->{class}->public->addFunction($function = Gen::Function->new({
    modifiers => ['static'], signature =>
      "void getName(std::ostringstream& os, const std::string& className,
                    const $fullClassName& item)"}));

  my $body = $self->{getNameWriter} = Gen::Writer->new('get name');
  $function
    ->setSkipCallback(sub { return $self->{getNameWriter}->empty; })
    ->body->addSubWriter($body);
}

sub _initHeaderFile
{
  my ($self) = @_;

  my $factoryHeaderFile = $self->{headerFile} = Gen::File->new({
    name => $self->_factoryName($self->{className}).'.h',
    commentDate => 1
  });

  $factoryHeaderFile->addComment(<<'LINE'
    This Factory is for the creation of test locations in
    order to facilitate unit testing. Particular cities are
    supported (DFW, LAX, LON, for example); use of the class
    is by invoking the static method for the location desired.
LINE
  );
  $factoryHeaderFile
    ->addInclude('"Common/TseTypes.h"')
    ->addInclude('<string>')
    ->addInclude('<iostream>');

  my $file = ($self->{outerClass} eq '' ? $self->{className} : $self->{outerClass}).'.h';
  $file = $self->{classPackage}.'/'.$file if $self->{classPackage} ne '';
  $factoryHeaderFile->addInclude('"'.$file.'"');

  $factoryHeaderFile
    ->writeLine('class TiXmlNode;')
    ->writeLine('class TiXmlElement;')
    ->writeLine()
    ->addSubWriter($self->{class}->declaration)
    ->eatPreviousNewLine();
}

sub _initSourceFile
{
  my ($self) = @_;

  my $factorySourceFile = $self->{sourceFile} = Gen::File->new({
    name => $self->_factoryName($self->{className}).'.cpp',
    commentDate => 1
  });

  $factorySourceFile
    ->addInclude('"test/testdata/TestFactoryBase.h"')
    ->addInclude('"test/testdata/TestXMLHelper.h"')
    ->addInclude('"test/testdata/tinyxml/tinyxml.h"')
    ->addInclude('"Common/TseConsts.h"');
  $self->_addTestFactoryInclude($self->{className});

  if ($self->{parentFactory} ne '')
  {
    $self->_addTestFactoryInclude($self->{parentFactory});
  }

  foreach my $child (@{$self->{children}})
  {
    $self->_addTestFactoryInclude($child);
  }

  $factorySourceFile
    ->writeLine('using namespace tse;')
    ->writeLine()
    ->addSubWriter($self->{class}->definition);
}

##################################### Methods

sub save
{
  my ($self, $saveDirectory_) = @_;

   if (defined $saveDirectory_)
   {
      $self->{headerFile}->save(File::Spec->join($saveDirectory_, $self->{headerFile}->name()));
      $self->{sourceFile}->save(File::Spec->join($saveDirectory_, $self->{sourceFile}->name()));


   }
   else
   {
      $self->{headerFile}->save();
      $self->{sourceFile}->save();
   }
}

sub saveToString
{
  my ($self) = @_;

  return (
    $self->{headerFile}->saveToString(),
    $self->{sourceFile}->saveToString()
  );
}

##################################### Modifiers

sub addSpecialField
{
  my ($self, $classSection, $variable, $init, $omitSetter) = @_;
  my $name = $variable->name;
  my $Name = ucfirst($name);

  $self->{class}->private->addField({
    modifiers => ['static'],
    name => "_$name",
    type => $variable->type,
    initializer => $init
  });

  my $section = $self->{class}->$classSection;
  my $function;

  $section->addFunction($function = Gen::Function->new({
    modifiers => ['static'],
    signature => $variable->type->function()->fullName("get$Name")
  }));

  $function->body->writeCode("return _$name;");

  if (!$omitSetter)
  {
    $section->addFunction($function = Gen::Function->new({
      modifiers => ['static'],
      signature => Gen::Type::Normal->new('void')->function(
        $variable->type->withConst->reference()->fullName($name))->fullName("set$Name")
    }));

    $function->body->writeCode("_$name = $name;");
  }

  return $self;
}

sub addAttribute
{
  my ($self, $sectionName, $attributeName, $accessor, $inputType) = @_;

  $self->_addScalarAggregateRead($sectionName, $attributeName, $accessor, $inputType);
  $self->_addScalarAggregateWrite($sectionName, $attributeName, $accessor, $inputType);

  return $self;
}

sub addScalarSection
{
  my ($self, $sectionName, $accessor, $inputType) = @_;

  $self->_addScalarAggregateRead($sectionName, 'value', $accessor, $inputType);
  $self->_addScalarAggregateWrite($sectionName, 'value', $accessor, $inputType);

  return $self;
}

sub addFactorySection
{
  my ($self, $sectionName, $accessor, $inputType, $willDelete) = @_;

  $self->_addTestFactoryInclude($self->_getFactoryType($accessor, $inputType)->fullName);
  $self->_addFactoryAggregateRead($sectionName, $accessor, $inputType, $willDelete);
  $self->_addFactoryAggregateWrite($sectionName, $accessor, $inputType);

  return $self;
}

sub addScalarCompare
{
  my ($self, $accessor) = @_;
  $accessor->setPrefix('i%d->');

  my ($variable, $writer) = $self->_prepareCompare($accessor, $self->{compareWriter});
  $self->_addCompare($writer, $variable->getExpressionReference,
                     '%s != %s');
  return $self;
}

sub addFactoryCompare
{
  my ($self, $accessor) = @_;
  $accessor->setPrefix('i%d->');

  my $factoryType = $self->_getFactoryType($accessor);
  $self->_addTestFactoryInclude($factoryType->fullName);
  my $factoryName = $self->_factoryName($factoryType->fullName);

  my ($variable, $writer) = $self->_prepareCompare($accessor, $self->{compareWriter});
  $self->_addCompare($writer, $variable->getExpressionPointer,
                     "!${factoryName}::compare(%s, %s)");
  return $self;
}

sub addSpecial
{
  my ($self, $sectionName, $attributeName, $special, $variable) = @_;

  my $section = $self->{initWriter}->getSection($sectionName);

  my $actualType = $self->_getFactoryType($variable)->fullName;
  error("Unsupported factory type") if isFactoryType($actualType);

  my $decl = $variable->declaration(1);
  my $getter = $variable->getExpressionReference;

  $section->directWriter->writeCode(<<LINES
    $decl;
    TestXMLHelper::Attribute(element, "$attributeName", $getter);
LINES
  );

  return $self;
}

sub addSpecialRead
{
  my ($self, $sectionName, $attributeName, $special, $content) = @_;

  my $section = $self->{initWriter}->getSection($sectionName);

  $section->addSubWriter($content);

  return $self;
}

sub addSpecialWrite
{
  my ($self, $sectionName, $attributeName, $special, $content) = @_;

  my $callBack;
  my $sectionType;
  if ($special eq 'Aggregate')
  {
    $sectionType = $Gen::FactorySectionWrite::SECTION_AGGREGATE;

    $callBack = sub {
      my ($value) = @_;
      return <<LINES
do {
      TiXmlElement* element = (TiXmlElement*)rootElement->LinkEndChild(
        new TiXmlElement("$sectionName"));
      element->SetAttribute("$attributeName", $value);
    } while (0)
LINES
      ;
    };
  }
  else
  {
    $sectionType = $Gen::FactorySectionWrite::SECTION_ATTRIBUTES;

    $callBack = sub {
      my ($value) = @_;
      return "element->SetAttribute(\"$attributeName\", $value)";
    };
  }

  my $section = $self->{writeWriter}->getSection($sectionName, $sectionType);

  $content->forEachWord(sub {
    my (undef, $word) = @_;
    $word->setWord(substituteUnaryFunction($word->word, "write", $callBack));
  });

  $section->directWriter->addSubWriter($content);

  return $self;
}

sub addPreInit
{
  my ($self, $content) = @_;
  $self->{preInitWriter}->addSubWriter($content);
  return $self;
}

sub addPostInit
{
  my ($self, $content) = @_;
  $self->{postInitWriter}->addSubWriter($content);
  return $self;
}

sub addCheckItem
{
  my ($self, $content) = @_;
  $self->{checkItemWriter}->addSubWriter($content);
  return $self;
}

sub addInclude
{
  my $self = shift;
  $self->{sourceFile}->addInclude('"'.join('/', @_).'"');
  return $self;
}

sub addNameValue
{
  my ($self, $value) = @_;
  $self->{getNameWriter}->writeCode("os << $value;");
  return $self;
}

sub addNameField
{
  my ($self, $field) = @_;
  $self->{getNameWriter}->writeCode("os << item.$field;");
  return $self;
}

##################################### Internal

sub _factoryName
{
  my ($self, $name) = @_;
  my (undef, $actualType) = splitTypename($name);
  return "Test${actualType}Factory";
}

sub _compareFinalReturn
{
  my ($self, $writer) = @_;

  if ($self->{compareWriter}->empty)
  {
    $writer->writeCode('return false;');
  }
  else
  {
    $writer->writeCode('return true;');
  }
}

sub _getSectionFromType
{
  my ($self, $accessor) = @_;

  if ($accessor->accessorType == $Gen::Accessor::ACCESSOR_CONSTRUCT)
  {
    return $self->{constructWriter};
  }
  elsif ($accessor->accessorType == $Gen::Accessor::ACCESSOR_INIT)
  {
    return $self->{initWriter};
  }
  else
  {
    error('Unknown accessor type: '.$accessor->accessorType);
  }
}

sub _getFactoryType
{
  my ($self, $variable, $inputType) = @_;

  return $inputType->withoutConst if defined $inputType;
  return $variable->type->innerMostType->withoutConst;
}

sub _addScalarAggregateRead
{
  my ($self, $sectionName, $attributeName, $accessor, $inputType) = @_;

  my $section = $self->_getSectionFromType($accessor);
  my ($variable, $writer) =
    $section->prepareRead($accessor, $inputType, $sectionName, $attributeName, 1);

  my $rootElement = 'element';
  $rootElement = 'rootElement' if $sectionName eq '';

  my $reference = $variable->getExpressionReference;

  $writer->writeCode(<<LINES
      TestXMLHelper::Attribute($rootElement, "$attributeName", $reference);
LINES
  );
}

sub _addFactoryAggregateRead
{
  my ($self, $sectionName, $accessor, $inputType, $willDelete) = @_;

  my $section = $self->_getSectionFromType($accessor);
  my ($variable, $writer) =
    $section->prepareRead($accessor, $inputType, $sectionName, '', 0);

  my $flags = 'flags & UNIQUE';
  $flags = 'OWNS_PTR' if !$willDelete;

  my $factoryType = $self->_getFactoryType($accessor, $inputType);
  my $type = $factoryType->fullName;
  my $factoryName = $self->_factoryName($type);

  $writer->writeCode($variable->setInstructionPointer(
    "TestFactoryBase<$factoryName>::create<$type> (fileName, element, $flags)").';');
}

sub _addScalarAggregateWrite
{
  my ($self, $sectionName, $attributeName, $accessor, $outputType) = @_;
  $accessor = $accessor->clone()->setIncomplete(0)->setPrefix('item.');

  my $section = $self->{writeWriter}; # Only one writer exists.
  my ($variable, $writer, $rootElement) =
    $section->prepareWrite($accessor, $outputType, $sectionName, $attributeName);
  return if !$variable;

  my $reference = $variable->getExpressionReference;

  $writer->writeCode(<<LINES
    $rootElement->SetAttribute("$attributeName", TestXMLHelper::format($reference));
LINES
  );
}

sub _addFactoryAggregateWrite
{
  my ($self, $sectionName, $accessor, $outputType) = @_;
  $accessor = $accessor->clone()->setIncomplete(0)->setPrefix('item.');

  my $section = $self->{writeWriter}; # Only one writer exists.
  my ($variable, $writer, $rootElement) =
    $section->prepareWrite($accessor, $outputType, $sectionName, '');

  my $factoryType = $self->_getFactoryType($accessor, $outputType);
  my $factoryName = $self->_factoryName($factoryType->fullName);
  my $reference = $variable->getExpressionReference;

  $writer->writeCode(<<LINES
    ${factoryName}::writeSubItem("$sectionName", $reference, $rootElement, filePrefix);
LINES
  );
}

sub _addTestFactoryInclude
{
  my ($self, $name) = @_;
  $self->addInclude('test/testdata', $self->_factoryName($name).'.h');
  return $self;
}

sub _writeConstructorArg
{
  my ($self, $writer, $i) = @_;
  my $arg = ${$self->{constructorArgs}}[$i];
  my $comma = (@{$self->{constructorArgs}} == $i+1 ? '' : ',');

  my $accessor = $self->{constructWriter}->getAccessor($arg);
  if ($accessor)
  {
    $writer->writeLine($accessor->postfixName('Local').$comma);
  }
  else
  {
    $writer->writeLine($arg.$comma);
  }
}

sub _bindConstructorArg
{
  my ($self, $i) = @_;
  return sub { $self->_writeConstructorArg($_[0], $i); }
}

sub _prepareCompare
{
  my ($self, $variable, $writer, $depth) = @_;
  $depth = 1 if !defined $depth;

  my $type = $variable->type;
  $type = $type->base if $type->isPointer;

  if ($type->isContainer)
  {
    my $container = Gen::Container->new($variable);
    my $innerWriter = Gen::Writer->new('compare loop');
    my $innerVariable = Gen::LocalVariable->new({
      name => "*it${depth}_%d",
      type => $type->base
    });
    my $containerName = $type->fullName;
    my $v1begin = sprintf($container->beginFunction, 1);
    my $v1end = sprintf($container->endFunction, 1);
    my $v2begin = sprintf($container->beginFunction, 2);
    my $v2end = sprintf($container->endFunction, 2);

    $writer
      ->pushBlock()
      ->writeCode(<<LINES
        ${containerName}::const_iterator
          it${depth}_1 = $v1begin, e1 = $v1end,
          it${depth}_2 = $v2begin, e2 = $v2end;
        for (; it${depth}_1 != e1 && it${depth}_2 != e2; ++it${depth}_1, ++it${depth}_2)
LINES
      )
      ->pushIndent()
      ->addSubWriter($innerWriter)
      ->popIndent()
      ->writeLine()
      ->writeCode(<<LINES
        if (it${depth}_1 != e1 || it${depth}_2 != e2)
          return false;
LINES
      )
      ->popBlock();

    return $self->_prepareCompare($innerVariable, $innerWriter, $depth+1);
  }
  elsif ($type->isNormal || $type->isTemplate)
  {
    return ($variable, $writer);
  }
  else
  {
    error('Unsupported type for compare: '.$type->fullName);
  }
}

sub _addCompare
{
  my ($self, $writer, $getter, $compareExpression) = @_;
  $compareExpression = sprintf($compareExpression, sprintf($getter, 1), sprintf($getter, 2));

  $writer->writeCode(<<LINES
    if ($compareExpression)
      return false;
LINES
  );
}

1;
