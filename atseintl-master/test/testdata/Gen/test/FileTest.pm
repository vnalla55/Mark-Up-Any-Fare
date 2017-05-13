############################################### File.pm Test

package Gen::test::FileTest;
require Gen::File;
require Gen::Class;

use strict;
use warnings;
use Gen::test::Test;

sub assertWriterEquals
{
  my ($writer, $string) = @_;

  my $result = $writer->saveToString();
  assertStringEquals($result, $string);
}

sub assertFileEquals
{
  my ($writer, $string) = @_;

  my $result = $writer->saveToString();

  # Remove date to make result deterministic.
  $result =~ s/on \d{4}-\d{2}-\d{2} at \d{2}:\d{2}:\d{2}/on yyyy-mm-dd at hh:ii:ss/;

  assertStringEquals($result, $string);
}

sub temporarilyRestrictOutputWidth
{
  my ($func) = @_;

  my $oldOutputWidth = $Gen::Writer::OUTPUT_WIDTH;
  $Gen::Writer::OUTPUT_WIDTH = 2;
  my $result = eval { $func->(); };
  $Gen::Writer::OUTPUT_WIDTH = $oldOutputWidth;

  die $@ if $@;

  return $result;
}

sub test_File_constructor_simple
{
  my $file = Gen::File->new({
    name => 'FooBar.h',
    commentDate => 0
  });

  assertStringEquals($file->name, 'FooBar.h');
  assertWriterEquals($file, <<LINES
#ifndef FOO_BAR_H
#define FOO_BAR_H



#endif
LINES
  );
}

sub test_File_constructor_setters
{
  my $file = Gen::File->new('FooBar.cpp');
  $file->setName('FooBar.h');

  assertStringEquals($file->name, 'FooBar.h');
  assertWriterEquals($file, <<LINES
#ifndef FOO_BAR_H
#define FOO_BAR_H



#endif
LINES
  );
}

sub test_File_guard_enable
{
  my $file = Gen::File->new();
  $file->setName('FooBar.cpp', 1);

  assertWriterEquals($file, <<LINES
#ifndef FOO_BAR_CPP
#define FOO_BAR_CPP



#endif
LINES
  );
}

sub test_File_guard_disable
{
  my $file = Gen::File->new();
  $file->setName('FooBar.cpp', 1);

  assertWriterEquals($file, <<LINES
#ifndef FOO_BAR_CPP
#define FOO_BAR_CPP



#endif
LINES
  );
}

sub test_File_commentDate
{
  my $file = Gen::File->new({
    name => 'FooBar.h',
    commentDate => 1
  });

  assertFileEquals($file, <<LINES
#ifndef FOO_BAR_H
#define FOO_BAR_H

/**
 *
 * This file has been generated on yyyy-mm-dd at hh:ii:ss.
 *
 **/



#endif
LINES
  );
}

sub test_File_comment
{
  my $file = Gen::File->new({
    name => 'FooBar.h',
    commentDate => 1
  });
  $file->addComment('Comment message is here!');

  assertFileEquals($file, <<LINES
#ifndef FOO_BAR_H
#define FOO_BAR_H

/**
 *
 * This file has been generated on yyyy-mm-dd at hh:ii:ss.
 *
 * Comment message is here!
 *
 **/



#endif
LINES
  );
}

sub test_File_commentWrap
{
  my $file = Gen::File->new({
    name => 'FooBar.cpp',
    commentDate => 0
  });
  $file->addComment('Comment message is here!');

  temporarilyRestrictOutputWidth(sub { assertFileEquals($file, <<LINES
/**
 *
 *
 * Comment
 * message
 * is
 * here!
 *
 **/


LINES
  );});
}

sub test_File_includes
{
  my $file = Gen::File->new({
    name => 'FooBar.h',
    commentDate => 1
  });
  $file
    ->addCxxInclude('string')
    ->addInclude('"test/testdata/TestFactoryBase.h"')
    ->addInclude('<cstring>')
    ->addOwnInclude('DataModel/Fare.h')
    ->addInclude('<sys/time.h>')
    ->addCInclude('cstdio')
    ->addInclude('<complex>')
    ->addLibraryInclude('unistd.h')
    ->addInclude('"Common/TsePrimitiveTypes.h"')
    ->addOwnInclude('Common/TseConsts.h')
    ->addInclude('<cstdlib>')
    ->addCxxInclude('iostream');

  assertFileEquals($file, <<LINES
#ifndef FOO_BAR_H
#define FOO_BAR_H

/**
 *
 * This file has been generated on yyyy-mm-dd at hh:ii:ss.
 *
 **/

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <complex>
#include <iostream>
#include <string>

#include <sys/time.h>
#include <unistd.h>

#include "Common/TseConsts.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Fare.h"
#include "test/testdata/TestFactoryBase.h"


#endif
LINES
  );
}

sub test_File_includes_duplicate
{
  my $file = Gen::File->new({
    name => 'FooBar.h',
    commentDate => 1
  });
  $file
    ->addInclude('<cstring>')
    ->addInclude('<cstring>');

  assertFileEquals($file, <<LINES
#ifndef FOO_BAR_H
#define FOO_BAR_H

/**
 *
 * This file has been generated on yyyy-mm-dd at hh:ii:ss.
 *
 **/

#include <cstring>


#endif
LINES
  );
}

sub test_File_content
{
  my $file = Gen::File->new({
    name => 'FooBar.h',
    commentDate => 1
  });
  $file
    ->addComment('This is a comment.');
  $file
    ->addInclude('<cstring>')
    ->addInclude('<string>')
    ->addInclude('"test/testdata/TestFactoryBase.h"')
    ->addInclude('"Common/TsePrimitiveTypes.h"');

  my $class = Gen::Class->new('FooBar');
  $class->public->addFunction({}, 1);
  $file
    ->addSubWriter($class->declaration)
    ->eatPreviousNewLine();

  assertFileEquals($file, <<LINES
#ifndef FOO_BAR_H
#define FOO_BAR_H

/**
 *
 * This file has been generated on yyyy-mm-dd at hh:ii:ss.
 *
 * This is a comment.
 *
 **/

#include <cstring>

#include <string>

#include "Common/TsePrimitiveTypes.h"
#include "test/testdata/TestFactoryBase.h"

class FooBar
{
public:
  FooBar();
};

#endif
LINES
  );
}

registerTest('Gen::test::FileTest');
