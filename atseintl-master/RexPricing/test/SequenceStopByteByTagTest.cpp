//-------------------------------------------------------------------
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------

#include "DataModel/ProcessTagInfo.h"
#include "Diagnostic/DiagCollector.h"
#include "RexPricing/SequenceStopByteByTag.h"
#include "test/include/CppUnitHelperMacros.h"

namespace tse
{

class SequenceStopByteByTagTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SequenceStopByteByTagTest);
  CPPUNIT_TEST(testHasStopByte);
  CPPUNIT_TEST(testIsSequenceGreater);
  CPPUNIT_TEST(testStopByteSave);
  CPPUNIT_TEST(testStopByteSkip);
  CPPUNIT_TEST(testStopByteSkipBypassed);
  CPPUNIT_TEST_SUITE_END();

public:
  void testHasStopByte()
  {
    SequenceStopByteByTag sequenceStopByteByTag;
    ProcessTagInfo* processTagInfo(new ProcessTagInfo);
    ReissueSequence* reissueSequence(new ReissueSequence);
    processTagInfo->reissueSequence()->orig() = reissueSequence;
    reissueSequence->stopInd() = 'X';
    CPPUNIT_ASSERT(SequenceStopByteByTag::hasStopByte(processTagInfo));
    reissueSequence->stopInd() = ' ';
    CPPUNIT_ASSERT(!SequenceStopByteByTag::hasStopByte(processTagInfo));
  }

  void testIsSequenceGreater()
  {
    SequenceStopByteByTag sequenceStopByteByTag;

    std::set<std::pair</*t988 seq*/ int, /*perm #*/ int> > sequences;
    sequences.insert(std::make_pair(1100, 2));
    sequences.insert(std::make_pair(1300, 1));

    ProcessTagInfo* processTagInfo(new ProcessTagInfo);
    processTagInfo->reissueSequence()->orig() = new ReissueSequence;
    CPPUNIT_ASSERT(sequenceStopByteByTag.isSequenceGreater(sequences, 1200));
    CPPUNIT_ASSERT(!sequenceStopByteByTag.isSequenceGreater(sequences, 1000));
    std::pair</*t988 seq*/ int, /*perm #*/ int> seqPermNo;

    CPPUNIT_ASSERT(sequenceStopByteByTag.findSequenceGreater(sequences, 1300, seqPermNo));
    CPPUNIT_ASSERT(seqPermNo.first == 1100);
    CPPUNIT_ASSERT(seqPermNo.second == 2);
    CPPUNIT_ASSERT(!sequenceStopByteByTag.findSequenceGreater(sequences, 1000, seqPermNo));
  }

  void testStopByteSave()
  {
    SequenceStopByteByTag sequenceStopByteByTag;
    ProcessTagInfo* processTagInfo(new ProcessTagInfo);
    ReissueSequence* reissueSequence(new ReissueSequence);
    processTagInfo->reissueSequence()->orig() = reissueSequence;
    processTagInfo->fareCompInfo() = 0;
    reissueSequence->processingInd() = 1;
    reissueSequence->stopInd() = 'X';
    reissueSequence->seqNo() = 1200;
    ProcessTagPermutation permutation;
    permutation.processTags().push_back(processTagInfo);
    sequenceStopByteByTag.saveStopByteInfo(permutation);
    CPPUNIT_ASSERT(SequenceStopByteByTag::hasStopByte(processTagInfo));
    CPPUNIT_ASSERT(!sequenceStopByteByTag.skipByStopByte(permutation));
    std::pair<ProcessTag, FareCompInfo*> key((ProcessTag)1, (FareCompInfo*)0);
    int sequence(1200);
    CPPUNIT_ASSERT(sequenceStopByteByTag.hasSequence(
        sequenceStopByteByTag.tagFcWithStopBytes()[key], sequence));
  }

  void testStopByteSkip()
  {
    SequenceStopByteByTag sequenceStopByteByTag;
    ProcessTagInfo* processTagInfo(new ProcessTagInfo);
    ReissueSequence* reissueSequence(new ReissueSequence);

    processTagInfo->reissueSequence()->orig() = reissueSequence;
    processTagInfo->fareCompInfo() = 0;
    reissueSequence->processingInd() = 1;
    reissueSequence->stopInd() = 'X';
    reissueSequence->seqNo() = 1200;
    ProcessTagPermutation permutation;
    permutation.processTags().push_back(processTagInfo);
    sequenceStopByteByTag.saveStopByteInfo(permutation);
    CPPUNIT_ASSERT(SequenceStopByteByTag::hasStopByte(processTagInfo));
    CPPUNIT_ASSERT(!sequenceStopByteByTag.skipByStopByte(permutation));
    std::pair<ProcessTag, FareCompInfo*> key((ProcessTag)1, (FareCompInfo*)0);
    CPPUNIT_ASSERT(
        sequenceStopByteByTag.hasSequence(sequenceStopByteByTag.tagFcWithStopBytes()[key], 1200));

    reissueSequence->seqNo() = 1300;
    int sequence(1300);
    CPPUNIT_ASSERT(!sequenceStopByteByTag.hasSequence(
        sequenceStopByteByTag.tagFcWithStopBytes()[key], sequence));
    CPPUNIT_ASSERT(sequenceStopByteByTag.skipByStopByte(permutation));
  }

  void testStopByteSkipBypassed()
  {
    SequenceStopByteByTag sequenceStopByteByTag;
    ProcessTagInfo* processTagInfo(new ProcessTagInfo);
    ReissueSequence* reissueSequence(new ReissueSequence);

    processTagInfo->reissueSequence()->orig() = reissueSequence;
    processTagInfo->fareCompInfo() = 0;
    reissueSequence->processingInd() = 1;
    reissueSequence->stopInd() = 'X';
    reissueSequence->seqNo() = 1200;
    ProcessTagPermutation permutation;
    permutation.processTags().push_back(processTagInfo);
    sequenceStopByteByTag.saveStopByteInfo(permutation);
    CPPUNIT_ASSERT(SequenceStopByteByTag::hasStopByte(processTagInfo));
    CPPUNIT_ASSERT(!sequenceStopByteByTag.skipByStopByte(permutation));
    std::pair<ProcessTag, FareCompInfo*> key((ProcessTag)1, (FareCompInfo*)0);
    CPPUNIT_ASSERT(
        sequenceStopByteByTag.hasSequence(sequenceStopByteByTag.tagFcWithStopBytes()[key], 1200));

    reissueSequence->seqNo() = 1300;
    int sequence(1300);
    CPPUNIT_ASSERT(!sequenceStopByteByTag.hasSequence(
        sequenceStopByteByTag.tagFcWithStopBytes()[key], sequence));
    CPPUNIT_ASSERT(sequenceStopByteByTag.skipByStopByte(permutation));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SequenceStopByteByTagTest);

} // tse
