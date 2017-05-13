#!/usr/bin/perl

use strict;
use warnings;

require Gen::DefParser;
require Gen::FactoryGenerator;

if (@ARGV < 1)
{
  print "Usage:\n  Generator.pl <file.def>\n";
  print "  Generator.pl <file.def> <output_dir>\n";
  exit(1);
}

my $saveDir = undef;
if (@ARGV == 2)
{
   $saveDir = $ARGV[1];
}

my $generator;
my $parser = Gen::DefParser->new($ARGV[0]);

$parser
  ->setInitDone(sub {
    $generator = Gen::FactoryGenerator->new($_[1]);
  })
  ->setSpecialAggregateBegin(sub {
    my $args = $_[1];
    $generator->addSpecial($$args{sectionName}, $$args{attributeName}, 'Aggregate',
                           $$args{variable});
  })
  ->setSpecialAggregateRead(sub {
    my $args = $_[1];
    $generator->addSpecialRead($$args{sectionName}, $$args{attributeName}, 'Aggregate',
                               $$args{content});
  })
  ->setSpecialAggregateWrite(sub {
    my $args = $_[1];
    $generator->addSpecialWrite($$args{sectionName}, $$args{attributeName}, 'Aggregate',
                                $$args{content});
  })
  ->setSpecialSectionBegin(sub {
    my $args = $_[1];
    $generator->addSpecial($$args{sectionName}, $$args{attributeName}, 'Section',
                           $$args{variable});
  })
  ->setSpecialSectionRead(sub {
    my $args = $_[1];
    $generator->addSpecialRead($$args{sectionName}, $$args{attributeName}, 'Section',
                               $$args{content});
  })
  ->setSpecialSectionWrite(sub {
    my $args = $_[1];
    $generator->addSpecialWrite($$args{sectionName}, $$args{attributeName}, 'Section',
                                $$args{content});
  })
  ->setSpecialField(sub {
    my $args = $_[1];
    $generator->addSpecialField($$args{section}, $$args{variable}, $$args{init}, $$args{omitSetter});
  })
  ->setScalar(sub {
    my $args = $_[1];
    $generator->addAttribute($$args{sectionName}, $$args{attributeName}, $$args{accessor},
                             $$args{inputType});
  })
  ->setScalarAggregate(sub {
    my $args = $_[1];
    $generator->addScalarSection($$args{sectionName}, $$args{accessor}, $$args{inputType});
  })
  ->setScalarCompare(sub {
    my $args = $_[1];
    $generator->addScalarCompare($$args{accessor});
  })
  ->setFactoryAggregate(sub {
    my $args = $_[1];
    $generator->addFactorySection($$args{sectionName}, $$args{accessor}, $$args{inputType},
                                  $$args{willDelete});
  })
  ->setFactoryCompare(sub {
    my $args = $_[1];
    $generator->addFactoryCompare($$args{accessor});
  })
  ->setPreInit(sub {
    my $args = $_[1];
    $generator->addPreInit($$args{content});
  })
  ->setPostInit(sub {
    my $args = $_[1];
    $generator->addPostInit($$args{content});
  })
  ->setCheckItem(sub {
    my $args = $_[1];
    $generator->addCheckItem($$args{content});
  })
  ->setInclude(sub {
    my $args = $_[1];
    $generator->addInclude($$args{file});
  })
  ->setNameValue(sub {
    my $args = $_[1];
    $generator->addNameValue($$args{value});
  })
  ->setNameField(sub {
    my $args = $_[1];
    $generator->addNameField($$args{field});
  });

$parser->process();

$generator->save($saveDir);

exit(0);
