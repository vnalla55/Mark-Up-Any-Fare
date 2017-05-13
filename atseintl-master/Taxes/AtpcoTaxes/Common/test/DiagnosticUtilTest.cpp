// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"

#include "Common/DiagnosticUtil.h"

#include <memory>

namespace tax
{

class DiagnosticUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DiagnosticUtilTest);

  CPPUNIT_TEST(testSplitAndMatchSeqValues_value);
  CPPUNIT_TEST(testSplitAndMatchSeqValues_fromValue);
  CPPUNIT_TEST(testSplitAndMatchSeqValues_range);
  CPPUNIT_TEST(testSplitAndMatchSeqValues_invalidRange);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _seq.reset(new type::SeqNo);
    _seqLimit.reset(new type::SeqNo);
    _isSeqRange.reset(new bool);
  }

  void tearDown() {}

  void testSplitAndMatchSeqValues_value()
  {
    const std::string line("101");
    DiagnosticUtil::splitSeqValues(line, *_seq, *_seqLimit, *_isSeqRange);

    CPPUNIT_ASSERT_EQUAL(type::SeqNo(101), *_seq);
    CPPUNIT_ASSERT_EQUAL(type::SeqNo(), *_seqLimit);
    CPPUNIT_ASSERT_EQUAL(false, *_isSeqRange);

    CPPUNIT_ASSERT_EQUAL(
        false,
        DiagnosticUtil::isSeqNoMatching(type::SeqNo(100), *_seq, *_seqLimit, *_isSeqRange));
    CPPUNIT_ASSERT_EQUAL(
        true,
        DiagnosticUtil::isSeqNoMatching(type::SeqNo(101), *_seq, *_seqLimit, *_isSeqRange));
    CPPUNIT_ASSERT_EQUAL(
        false,
        DiagnosticUtil::isSeqNoMatching(type::SeqNo(102), *_seq, *_seqLimit, *_isSeqRange));
  }

  void testSplitAndMatchSeqValues_fromValue()
  {
    const std::string line("102-");
    DiagnosticUtil::splitSeqValues(line, *_seq, *_seqLimit, *_isSeqRange);

    CPPUNIT_ASSERT_EQUAL(type::SeqNo(102), *_seq);
    CPPUNIT_ASSERT_EQUAL(type::SeqNo(), *_seqLimit);
    CPPUNIT_ASSERT_EQUAL(true, *_isSeqRange);

    CPPUNIT_ASSERT_EQUAL(
        false,
        DiagnosticUtil::isSeqNoMatching(type::SeqNo(101), *_seq, *_seqLimit, *_isSeqRange));
    CPPUNIT_ASSERT_EQUAL(
        true,
        DiagnosticUtil::isSeqNoMatching(type::SeqNo(102), *_seq, *_seqLimit, *_isSeqRange));
    CPPUNIT_ASSERT_EQUAL(
        true,
        DiagnosticUtil::isSeqNoMatching(type::SeqNo(110), *_seq, *_seqLimit, *_isSeqRange));
  }

  void testSplitAndMatchSeqValues_range()
  {
    const std::string line("103-105");
    DiagnosticUtil::splitSeqValues(line, *_seq, *_seqLimit, *_isSeqRange);

    CPPUNIT_ASSERT_EQUAL(type::SeqNo(103), *_seq);
    CPPUNIT_ASSERT_EQUAL(type::SeqNo(105), *_seqLimit);
    CPPUNIT_ASSERT_EQUAL(true, *_isSeqRange);

    CPPUNIT_ASSERT_EQUAL(
        false,
        DiagnosticUtil::isSeqNoMatching(type::SeqNo(102), *_seq, *_seqLimit, *_isSeqRange));
    CPPUNIT_ASSERT_EQUAL(
        true,
        DiagnosticUtil::isSeqNoMatching(type::SeqNo(103), *_seq, *_seqLimit, *_isSeqRange));
    CPPUNIT_ASSERT_EQUAL(
        true,
        DiagnosticUtil::isSeqNoMatching(type::SeqNo(104), *_seq, *_seqLimit, *_isSeqRange));
    CPPUNIT_ASSERT_EQUAL(
        true,
        DiagnosticUtil::isSeqNoMatching(type::SeqNo(105), *_seq, *_seqLimit, *_isSeqRange));
    CPPUNIT_ASSERT_EQUAL(
        false,
        DiagnosticUtil::isSeqNoMatching(type::SeqNo(106), *_seq, *_seqLimit, *_isSeqRange));
  }

  void testSplitAndMatchSeqValues_invalidRange()
  {
    const std::string line("103-101");
    DiagnosticUtil::splitSeqValues(line, *_seq, *_seqLimit, *_isSeqRange);

    CPPUNIT_ASSERT_EQUAL(type::SeqNo(103), *_seq);
    CPPUNIT_ASSERT_EQUAL(type::SeqNo(101), *_seqLimit);
    CPPUNIT_ASSERT_EQUAL(true, *_isSeqRange);

    CPPUNIT_ASSERT_EQUAL(
        false,
        DiagnosticUtil::isSeqNoMatching(type::SeqNo(100), *_seq, *_seqLimit, *_isSeqRange));
    CPPUNIT_ASSERT_EQUAL(
        false,
        DiagnosticUtil::isSeqNoMatching(type::SeqNo(101), *_seq, *_seqLimit, *_isSeqRange));
    CPPUNIT_ASSERT_EQUAL(
        false,
        DiagnosticUtil::isSeqNoMatching(type::SeqNo(102), *_seq, *_seqLimit, *_isSeqRange));
    CPPUNIT_ASSERT_EQUAL(
        false,
        DiagnosticUtil::isSeqNoMatching(type::SeqNo(103), *_seq, *_seqLimit, *_isSeqRange));
    CPPUNIT_ASSERT_EQUAL(
        false,
        DiagnosticUtil::isSeqNoMatching(type::SeqNo(104), *_seq, *_seqLimit, *_isSeqRange));
  }

private:
  std::unique_ptr<type::SeqNo> _seq;
  std::unique_ptr<type::SeqNo> _seqLimit;
  std::unique_ptr<bool> _isSeqRange;
};

CPPUNIT_TEST_SUITE_REGISTRATION(DiagnosticUtilTest);

} // namespace tax
