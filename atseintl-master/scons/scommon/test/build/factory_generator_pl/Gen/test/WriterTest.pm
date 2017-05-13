############################################### Writer.pm Test

package Gen::test::WriterTest;
require Gen::Word;
require Gen::Writer;

use strict;
use warnings;
use Gen::test::Test;

sub assertWriterEquals
{
  my ($writer, $string) = @_;

  my $result = $writer->saveToString();
  assertStringEquals($result, $string);
}

sub temporarilyRestrictOutputWidth
{
  my ($func) = @_;

  my $oldOutputWidth = $Gen::Writer::OUTPUT_WIDTH;
  $Gen::Writer::OUTPUT_WIDTH = 2;
  my $result = $func->();
  $Gen::Writer::OUTPUT_WIDTH = $oldOutputWidth;

  return $result;
}

sub test_Writer_constructor
{
  my $writer = Gen::Writer->new('name');

  assertStringEquals($writer->name, 'name');
  assert($writer->empty);
  assertError(sub { $writer->lastLine; });
}

sub test_Writer_setName
{
  my $writer = Gen::Writer->new('old');
  $writer->setName('new');

  assertStringEquals($writer->name, 'new');
}

sub test_Writer_lastLine
{
  my $writer = Gen::Writer->new('name');
  $writer->writeLine('previous');
  $writer->writeLine('word');

  assert(!$writer->empty);
  assertStringEquals(${$writer->lastLine}[0]->word, 'word');
}

sub test_Writer_clone
{
  my $writer = Gen::Writer->new('name');
  $writer->writeLine('one');
  $writer->writeLine('two');

  my $clone = $writer->clone;
  $writer->writeLine('three');
  $writer->setName('name2');
  $clone->writeLine('four');

  assertStringEquals($writer->name, 'name2');
  assertStringEquals($clone->name, 'name');
  assertWriterEquals($writer, <<LINES
one
two
three
LINES
  );
  assertWriterEquals($clone, <<LINES
one
two
four
LINES
  );
}

sub test_Writer_forEachWord
{
  my $writer = Gen::Writer->new('name');
  $writer->writeCode('one two');
  $writer->writeCode('three four');

  my $merge = '';
  $writer->forEachWord(sub {
    $merge .= $_[1]->word;
  });

  assertStringEquals($merge, 'onetwothreefour');
}

sub test_Writer_setSkipCallback
{
  my $writer = Gen::Writer->new('name');
  $writer->writeLine('line');
  my $result1 = $writer->saveToString();

  $writer->setSkipCallback(sub { return 1; });
  my $result2 = $writer->saveToString();

  assertStringEquals($result1, <<LINES
line
LINES
  );
  assertStringEquals($result2, '');
}

sub test_Writer_writeLine
{
  my $writer = Gen::Writer->new('name');
  $writer->writeLine('word');

  assertWriterEquals($writer, <<LINES
word
LINES
  );
}

sub test_Writer_writeLine_space
{
  my $writer = Gen::Writer->new('name');
  $writer->writeLine(Gen::Word::Stream->new()->addCode('word1 word2'));

  assertWriterEquals($writer, <<LINES
word1 word2
LINES
  );
}

sub test_Writer_writeLine_noSpace
{
  my $writer = Gen::Writer->new('name');
  $writer->writeLine(Gen::Word::Stream->new()->addWord('word1')->addWord('word2'));

  assertWriterEquals($writer, <<LINES
word1word2
LINES
  );
}

sub test_Writer_writeLine_recursive
{
  my $writer = Gen::Writer->new('name');

  my $inner = ['inner'];
  my $stream = Gen::Word::Stream->new()->addWord('stream');
  my $line = ['word1', 'word2', $inner, $stream];

  $writer->writeLine($line, 'a');

  assertWriterEquals($writer, <<LINES
word1word2innerstreama
LINES
  );
}

sub test_Writer_writeLine_wrap
{
  my $writer = Gen::Writer->new('name');
  $writer->writeLine(Gen::Word::Stream->new()->addCode('word1 word2'));

  my $result = temporarilyRestrictOutputWidth(sub { return $writer->saveToString(); });

  assertStringEquals($result, <<LINES
word1
  word2
LINES
  );
}

sub test_Writer_writeLine_wrap_prefixPostfix
{
  my $writer = Gen::Writer->new('name');
  $writer->writeLine(Gen::Word::Stream->new({prefix => '>', postfix => '<'})->addCode('word1 word2'));

  my $result = temporarilyRestrictOutputWidth(sub { return $writer->saveToString(); });

  assertStringEquals($result, <<LINES
word1<
  >word2
LINES
  );
}

sub test_Writer_writeLine_wrap_anchor
{
  my $writer = Gen::Writer->new('name');
  my $stream = Gen::Word::Stream->new();
  my $anchor = $stream->addWord('anchor:')->lastWord;
  $writer->writeLine($stream->setIndent(0, $anchor)->addCode('word1 word2'));

  my $result = temporarilyRestrictOutputWidth(sub { return $writer->saveToString(); });

  assertStringEquals($result, <<LINES
anchor:
       word1
       word2
LINES
  );
}

sub test_Writer_writeCode
{
  my $writer1 = Gen::Writer->new('name');
  $writer1->writeLine(Gen::Word::Stream->new()->addCode('for (int i = 0; i < 7; ++i) {'));

  my $writer2 = Gen::Writer->new('name');
  $writer2->writeCode('for (int i = 0; i < 7; ++i) {');

  my $result1 = temporarilyRestrictOutputWidth(sub { return $writer1->saveToString(); });
  my $result2 = temporarilyRestrictOutputWidth(sub { return $writer2->saveToString(); });

  assertStringEquals($result1, $result2);
  assertStringEquals($result1, <<LINES
for
  (
  int
  i
  =
  0;
  i
  <
  7;
  ++i)
  {
LINES
  );
}

sub test_Writer_addSubWriter
{
  my $inner = Gen::Writer->new('name')->writeLine(' inner');
  my $writer = Gen::Writer->new('name');
  $writer
    ->writeLine('before')
    ->addSubWriter($inner)
    ->writeLine('after');
  $inner->writeLine(' writer');

  assertWriterEquals($writer, <<LINES
before
 inner
 writer
after
LINES
  );
}

sub test_Writer_push_popIndent
{
  my $writer = Gen::Writer->new('name');
  $writer
    ->writeLine('before')
    ->pushIndent(2)
    ->writeLine('indented')
    ->popIndent(2)
    ->writeLine('after');

  assertWriterEquals($writer, <<LINES
before
  indented
after
LINES
  );
}

sub test_Writer_wrongIndent_error
{
  my $writer = Gen::Writer->new('name');
  $writer->pushIndent(7);

  assertError($writer->saveToString());
}

sub test_Writer_addSubWriter_indented
{
  my $inner = Gen::Writer->new('name')->writeLine(' inner');
  my $writer = Gen::Writer->new('name');
  $writer
    ->writeLine('before')
    ->pushIndent(2)
    ->addSubWriter($inner)
    ->popIndent(2)
    ->writeLine('after');
  $inner->writeLine(' writer');

  assertWriterEquals($writer, <<LINES
before
   inner
   writer
after
LINES
  );
}

sub test_Writer_addSubWriter_indentedNegative
{
  my $inner = Gen::Writer->new('name');
  $inner
    ->pushIndent(-2)
    ->writeLine('inner')
    ->popIndent(-2);
  my $writer = Gen::Writer->new('name');
  $writer
    ->writeLine('before')
    ->pushIndent(2)
    ->addSubWriter($inner)
    ->popIndent(2)
    ->writeLine('after');
  $inner->writeLine('writer');

  assertWriterEquals($writer, <<LINES
before
inner
  writer
after
LINES
  );
}

sub test_Writer_push_popBlock
{
  my $inner = Gen::Writer->new('name')->writeLine(' inner');
  my $writer = Gen::Writer->new('name');
  $writer
    ->writeLine('before')
    ->pushBlock()
    ->addSubWriter($inner)
    ->popBlock()
    ->writeLine('after');
  $inner->writeLine(' writer');

  assertWriterEquals($writer, <<LINES
before
{
   inner
   writer
}
after
LINES
  );
}

sub test_Writer_newLine_simple
{
  my $writer = Gen::Writer->new('outer');
  my $inner = Gen::Writer->new('inner')->writeLine('inner');
  $writer
    ->writeLine('before')
    ->addSubWriter($inner)
    ->writeLine('after');

  $inner->setBeginNewLine(1);
  $inner->setEndNewLine(1);

  assertWriterEquals($writer, <<LINES
before

inner

after
LINES
  );

  $inner->setBeginNewLine(0);
  $inner->setEndNewLine(1);

  assertWriterEquals($writer, <<LINES
before
inner

after
LINES
  );
}

sub test_Writer_newLine_eatNoop
{
  my $writer = Gen::Writer->new('outer');
  my $inner = Gen::Writer->new('inner')->writeLine('inner');
  $writer
    ->eatNextNewLine()
    ->writeLine('before')
    ->addSubWriter($inner)
    ->eatPreviousNewLine()
    ->eatPreviousNewLine()
    ->writeLine('after');

  $inner->setBeginNewLine(1);
  $inner->setEndNewLine(0);

  assertWriterEquals($writer, <<LINES
before

inner
after
LINES
  );
}

sub test_Writer_newLine_eatOne
{
  my $writer = Gen::Writer->new('outer');
  my $inner = Gen::Writer->new('inner')->writeLine('inner');
  $writer
    ->writeLine('before')
    ->eatNextNewLine()
    ->addSubWriter($inner)
    ->eatPreviousNewLine()
    ->writeLine('after');

  $inner->setBeginNewLine(1);
  $inner->setEndNewLine(1);

  assertWriterEquals($writer, <<LINES
before
inner
after
LINES
  );
}

sub test_Writer_newLine_eatMany
{
  my $writer = Gen::Writer->new('outer');
  my $inner1 = Gen::Writer->new('inner')->writeLine('inner');
  my $inner2 = Gen::Writer->new('empty');
  $writer
    ->writeLine('before')
    ->addSubWriter($inner1)
    ->addSubWriter($inner2)
    ->eatPreviousNewLine()
    ->writeLine('after');

  $inner1->setEndNewLine(1);
  $inner2->setEndNewLine(1);

  assertWriterEquals($writer, <<LINES
before
inner
after
LINES
  );
}

sub test_Writer_newLine_atEnd
{
  my $writer = Gen::Writer->new('outer');
  my $inner = Gen::Writer->new('inner')->writeLine('inner')->setEndNewLine(1);
  $writer
    ->writeLine('before')
    ->addSubWriter($inner);

  assertWriterEquals($writer, <<LINES
before
inner

LINES
  );
}

sub test_Writer_newLine_eatOneLevel
{
  my $inner = Gen::Writer->new('inner')
    ->writeLine('inner')
    ->setEndNewLine(1);
  my $middle = Gen::Writer->new('middle')
    ->writeLine('middle')
    ->addSubWriter($inner);
  my $writer = Gen::Writer->new('outer')
    ->writeLine('outer')
    ->addSubWriter($middle)
    ->eatPreviousNewLine();

  assertWriterEquals($writer, <<LINES
outer
middle
inner

LINES
  );
}

sub test_Writer_newLine_eatEndNewLine
{
  my $writer = Gen::Writer->new('outer')
    ->writeLine('outer')
    ->eatEndNewLine(1);
  my $inner1 = Gen::Writer->new('inner1')
    ->writeLine('inner1')
    ->setEndNewLine(1);
  my $inner2 = Gen::Writer->new('inner2')
    ->writeLine('inner2')
    ->setEndNewLine(1);
  $writer
    ->addSubWriter($inner1)
    ->addSubWriter($inner2);

  assertWriterEquals($writer, <<LINES
outer
inner1

inner2
LINES
  );
}

sub test_Writer_newLine_block
{
  my $writer = Gen::Writer->new('outer');
  my $inner1 = Gen::Writer->new('inner')->writeLine('inner');
  my $inner2 = Gen::Writer->new('empty');
  $writer
    ->writeLine('before')
    ->pushBlock()
    ->addSubWriter($inner1)
    ->addSubWriter($inner2)
    ->popBlock()
    ->writeLine('after');

  $inner1->setEndNewLine(1);
  $inner2->setEndNewLine(1);

  assertWriterEquals($writer, <<LINES
before
{
  inner
}
after
LINES
  );
}

sub test_Writer_addCallback
{
  my $writer = Gen::Writer->new('name');

  $writer->addCallback('cname', sub { $_[0]->writeLine('hello from callback'); });

  assertWriterEquals($writer, <<LINES
hello from callback
LINES
  );
}

sub test_CallbackWriter_works
{
  my $callback = Gen::Writer::CallbackWriter->new('callback', sub { $_[0]->writeLine('-'); });
  my $writer = Gen::Writer->new('name')->addSubWriter($callback);

  assertWriterEquals($writer, <<LINES
-
LINES
  );
}

sub test_CallbackWriter_saveRewrites
{
  my $callback = Gen::Writer::CallbackWriter->new('callback', sub { $_[0]->writeLine('ok'); });
  my $writer = Gen::Writer->new('name')->addSubWriter($callback);
  $callback->writeLine('error');

  assertWriterEquals($writer, <<LINES
ok
LINES
  );
}

registerTest('Gen::test::WriterTest');
