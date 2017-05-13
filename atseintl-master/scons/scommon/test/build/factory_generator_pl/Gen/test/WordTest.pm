############################################### Word.pm Test

package Gen::test::WordTest;
require Gen::Word;

use strict;
use warnings;
use Gen::test::Test;

sub test_Word_simple
{
  my $word = Gen::Word->new('simple');

  assertStringEquals($word->word, 'simple');
  assertStringEquals($word->space, '');
  assertIsUndef($word->anchor);
  assertEquals($word->relativeIndent, 0);
  assertIsUndef($word->prefix);
  assertIsUndef($word->postfix);
}

sub test_Word_space
{
  my $word = Gen::Word->new('simple ');

  assertStringEquals($word->word, 'simple');
  assertStringEquals($word->space, ' ');
}

sub test_Word_setWord
{
  my $word = Gen::Word->new('simple ');
  $word->setWord('other');

  assertStringEquals($word->word, 'other');
  assertStringEquals($word->space, '');

  $word->setWord('another ');

  assertStringEquals($word->word, 'another');
  assertStringEquals($word->space, ' ');
}

sub test_Word_relativeIntent
{
  my $word = Gen::Word->new('simple')->setIndent(1);

  assertEquals($word->relativeIndent, 1);
}

sub test_Word_anchor
{
  my $anchor = Gen::Word->new('anchor');
  my $word = Gen::Word->new('simple')->setIndent(2, $anchor);

  assertEquals($word->relativeIndent, 2);
  assertEquals($word->anchor, $anchor);
}

sub test_Word_prefix
{
  my $word = Gen::Word->new('simple')->setPrefix('a');

  assertStringEquals($word->prefix, 'a');
}

sub test_Word_postfix
{
  my $word = Gen::Word->new('simple')->setPostfix('b');

  assertStringEquals($word->postfix, 'b');
}

sub test_Word_prefix_postfix
{
  my $word = Gen::Word->new('simple')->setPrefix('a')->setPostfix('b');

  assertStringEquals($word->prefix, 'a');
  assertStringEquals($word->postfix, 'b');
}

sub test_WordStream_empty
{
  my $stream = Gen::Word::Stream->new();

  assert($stream->empty);
}

sub test_WordStream_singleWord
{
  my $stream = Gen::Word::Stream->new();
  $stream->addWord('word');

  assert(!$stream->empty);
  assertStringEquals($stream->lastWord->word, 'word');
}

sub test_WordStream_constructor
{
  my $anchor = Gen::Word->new('anchor');
  my $stream = Gen::Word::Stream->new({
    anchor => $anchor,
    relativeIndent => 7,
    prefix => 'prefix',
    postfix => 'postfix'
  });
  $stream->addWord('word');

  assertEquals($stream->lastWord->anchor, $anchor);
  assertEquals($stream->lastWord->relativeIndent, 7);
  assertStringEquals($stream->lastWord->prefix, 'prefix');
  assertStringEquals($stream->lastWord->postfix, 'postfix');
}

sub test_WordStream_setters
{
  my $anchor = Gen::Word->new('anchor');
  my $stream = Gen::Word::Stream->new();
  $stream
    ->setIndent(7, $anchor)
    ->setPrefix('prefix')
    ->setPostfix('postfix');
  $stream->addWord('word');

  assertEquals($stream->lastWord->anchor, $anchor);
  assertEquals($stream->lastWord->relativeIndent, 7);
  assertStringEquals($stream->lastWord->prefix, 'prefix');
  assertStringEquals($stream->lastWord->postfix, 'postfix');
}

sub test_WordStream_setters_twoWords
{
  my $anchor = Gen::Word->new('anchor');
  my $stream = Gen::Word::Stream->new();
  $stream
    ->setIndent(7, $anchor)
    ->setPrefix('prefix')
    ->setPostfix('postfix');
  $stream->addWord('word');
  $stream->addWord('word');

  assertIsUndef($stream->lastWord->anchor);
  assertEquals($stream->lastWord->relativeIndent, 0);
  assertIsUndef($stream->lastWord->prefix);
  assertIsUndef($stream->lastWord->postfix);
}

sub test_WordStream_addWord_mixed
{
  my $stream = Gen::Word::Stream->new()->setPostfix('postfix');
  $stream->addWord(Gen::Word->new('word')->setPrefix('prefix'));

  assertStringEquals($stream->lastWord->prefix, 'prefix');
  assertStringEquals($stream->lastWord->postfix, 'postfix');
}

sub test_WordStream_addCode_simple
{
  my $stream = Gen::Word::Stream->new();
  $stream->addCode(' a very simple code ');
  my $words = $stream->words;

  assertStringEquals($$words[0]->word, 'a');
  assertStringEquals($$words[0]->space, ' ');
  assertStringEquals($$words[1]->word, 'very');
  assertStringEquals($$words[1]->space, ' ');
  assertStringEquals($$words[2]->word, 'simple');
  assertStringEquals($$words[2]->space, ' ');
  assertStringEquals($$words[3]->word, 'code');
  assertStringEquals($$words[3]->space, ' ');
}

sub test_WordStream_addCode_operators
{
  my $stream = Gen::Word::Stream->new();
  $stream->addCode('std::vector<int> a(returnValue.compute());');
  my $words = $stream->words;

  assertStringEquals($$words[0]->word, 'std::');
  assertStringEquals($$words[0]->space, '');
  assertStringEquals($$words[1]->word, 'vector<int>');
  assertStringEquals($$words[1]->space, ' ');
  assertStringEquals($$words[2]->word, 'a(');
  assertStringEquals($$words[2]->space, '');
  assertStringEquals($$words[3]->word, 'returnValue.compute(');
  assertStringEquals($$words[3]->space, '');
  assertStringEquals($$words[4]->word, '));');
  assertStringEquals($$words[4]->space, '');
}

sub test_WordStream_addCode_string
{
  my $stream = Gen::Word::Stream->new();
  $stream->addCode('"std::vector<int> a(returnValue.compute());"');
  my $words = $stream->words;

  assertStringEquals($$words[0]->word, '"std::vector<int> a(returnValue.compute());"');
  assertStringEquals($$words[0]->space, '');
}

sub test_WordStream_addSubStream
{
  my $stream = Gen::Word::Stream->new();
  my $inner = Gen::Word::Stream->new();
  $stream->addWord('-')->addSubStream($inner)->addWord('-');
  $inner->addWord('inner');
  my $words = $stream->words;

  assertStringEquals($$words[0]->word, '-');
  assertStringEquals($$words[1]->word, 'inner');
  assertStringEquals($$words[2]->word, '-');
}

sub test_WordStream_addSubStream_normalization
{
  my $stream = Gen::Word::Stream->new();
  my $inner = Gen::Word::Stream->new();
  $stream->addWord('-')->addSubStream($inner)->addWord('-');
  $inner->addWord('inner');
  my $words = $stream->words;
  $inner->addWord('another');
  $words = $stream->words;

  assertStringEquals($$words[0]->word, '-');
  assertStringEquals($$words[1]->word, 'inner');
  assertStringEquals($$words[2]->word, '-');

  $words = $inner->words;

  assertStringEquals($$words[0]->word, 'inner');
  assertStringEquals($$words[1]->word, 'another');
}

sub test_WordStream_words_simple
{
  my $stream = Gen::Word::Stream->new();
  $stream->addWord('a')->addWord('b');

  my $words = $stream->words;

  assertStringEquals($$words[0]->word, 'a');
  assertStringEquals($$words[1]->word, 'b');
}

registerTest('Gen::test::WordTest');
