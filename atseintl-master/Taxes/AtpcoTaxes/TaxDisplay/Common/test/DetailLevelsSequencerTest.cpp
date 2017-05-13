// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "TaxDisplay/Common/DetailLevelsSequencer.h"
#include "TaxDisplay/Common/Types.h"
#include "test/include/CppUnitHelperMacros.h"

#include <functional>

namespace tax
{
namespace display
{

class DetailLevelsSequencerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(DetailLevelsSequencerTest);
  CPPUNIT_TEST(testDetailLevelsSplit);
  CPPUNIT_TEST(testSequenceRun);
  CPPUNIT_TEST(testSequenceRunMoreCallbacksThanDetailLevels);
  CPPUNIT_TEST(testSequenceRunMoreDetailLevelsThanCallbacks);
  CPPUNIT_TEST(testSequenceCallbackReturningFalse);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}
  void tearDown() {}

  bool callback(DetailEntryNo detailEntryNo,
                bool isThisLastCallback,
                DetailEntryNo& outDetailEntryNo,
                bool& outIsThisLastCallback)
  {
    outDetailEntryNo = detailEntryNo;
    outIsThisLastCallback = isThisLastCallback;

    return true;
  }

  void testDetailLevelsSplit()
  {
    DetailLevels detailLevels("10|4|2|9");
    DetailLevelsSequencer sequencer(detailLevels);

    CPPUNIT_ASSERT_EQUAL(true, sequencer.hasDetailEntries());
    CPPUNIT_ASSERT_EQUAL(size_t(4), sequencer._detailEntries.size());
    CPPUNIT_ASSERT_EQUAL(10u, sequencer._detailEntries[0]);
    CPPUNIT_ASSERT_EQUAL(4u, sequencer._detailEntries[1]);
    CPPUNIT_ASSERT_EQUAL(2u, sequencer._detailEntries[2]);
    CPPUNIT_ASSERT_EQUAL(9u, sequencer._detailEntries[3]);
  }

  void testSequenceRun()
  {
    DetailLevels detailLevels("10|4|2");
    DetailLevelsSequencer sequencer(detailLevels);

    DetailEntryNo cb1DetailEntryNo, cb2DetailEntryNo, cb3DetailEntryNo;
    bool cb1IsThisLastCallback, cb2IsThisLastCallback, cb3IsThisLastCallback;

    using namespace std::placeholders;
    sequencer.pushCallback(std::bind(&DetailLevelsSequencerTest::callback,
                                     this, _1, _2, std::ref(cb1DetailEntryNo), std::ref(cb1IsThisLastCallback)));
    sequencer.pushCallback(std::bind(&DetailLevelsSequencerTest::callback,
                                     this, _1, _2, std::ref(cb2DetailEntryNo), std::ref(cb2IsThisLastCallback)));
    sequencer.pushCallback(std::bind(&DetailLevelsSequencerTest::callback,
                                     this, _1, _2, std::ref(cb3DetailEntryNo), std::ref(cb3IsThisLastCallback)));

    CPPUNIT_ASSERT_EQUAL(true, sequencer.run());
    CPPUNIT_ASSERT_EQUAL(10u, cb1DetailEntryNo);
    CPPUNIT_ASSERT_EQUAL(false, cb1IsThisLastCallback);
    CPPUNIT_ASSERT_EQUAL(4u, cb2DetailEntryNo);
    CPPUNIT_ASSERT_EQUAL(false, cb2IsThisLastCallback);
    CPPUNIT_ASSERT_EQUAL(2u, cb3DetailEntryNo);
    CPPUNIT_ASSERT_EQUAL(true, cb3IsThisLastCallback);
  }

  void testSequenceRunMoreCallbacksThanDetailLevels()
  {
    DetailLevels detailLevels("10|4");
    DetailLevelsSequencer sequencer(detailLevels);

    DetailEntryNo cb1DetailEntryNo, cb2DetailEntryNo, cb3DetailEntryNo = 99;
    bool cb1IsThisLastCallback, cb2IsThisLastCallback, cb3IsThisLastCallback = false;

    using namespace std::placeholders;
    sequencer.pushCallback(std::bind(&DetailLevelsSequencerTest::callback,
                                     this, _1, _2, std::ref(cb1DetailEntryNo), std::ref(cb1IsThisLastCallback)));
    sequencer.pushCallback(std::bind(&DetailLevelsSequencerTest::callback,
                                     this, _1, _2, std::ref(cb2DetailEntryNo), std::ref(cb2IsThisLastCallback)));
    sequencer.pushCallback(std::bind(&DetailLevelsSequencerTest::callback,
                                     this, _1, _2, std::ref(cb3DetailEntryNo), std::ref(cb3IsThisLastCallback)));

    CPPUNIT_ASSERT_EQUAL(true, sequencer.run());
    CPPUNIT_ASSERT_EQUAL(10u, cb1DetailEntryNo);
    CPPUNIT_ASSERT_EQUAL(false, cb1IsThisLastCallback);
    CPPUNIT_ASSERT_EQUAL(4u, cb2DetailEntryNo);
    CPPUNIT_ASSERT_EQUAL(true, cb2IsThisLastCallback);
    CPPUNIT_ASSERT_EQUAL(99u, cb3DetailEntryNo);
    CPPUNIT_ASSERT_EQUAL(false, cb3IsThisLastCallback);
  }

  void testSequenceRunMoreDetailLevelsThanCallbacks()
  {
    DetailLevels detailLevels("10|4|2|9");
    DetailLevelsSequencer sequencer(detailLevels);

    DetailEntryNo cb1DetailEntryNo = 99u, cb2DetailEntryNo = 98u, cb3DetailEntryNo = 97u;
    bool cb1IsThisLastCallback = false, cb2IsThisLastCallback = false, cb3IsThisLastCallback = false;

    using namespace std::placeholders;
    sequencer.pushCallback(std::bind(&DetailLevelsSequencerTest::callback,
                                     this, _1, _2, std::ref(cb1DetailEntryNo), std::ref(cb1IsThisLastCallback)));
    sequencer.pushCallback(std::bind(&DetailLevelsSequencerTest::callback,
                                     this, _1, _2, std::ref(cb2DetailEntryNo), std::ref(cb2IsThisLastCallback)));
    sequencer.pushCallback(std::bind(&DetailLevelsSequencerTest::callback,
                                     this, _1, _2, std::ref(cb3DetailEntryNo), std::ref(cb3IsThisLastCallback)));

    CPPUNIT_ASSERT_EQUAL(false, sequencer.run());
    CPPUNIT_ASSERT_EQUAL(99u, cb1DetailEntryNo);
    CPPUNIT_ASSERT_EQUAL(false, cb1IsThisLastCallback);
    CPPUNIT_ASSERT_EQUAL(98u, cb2DetailEntryNo);
    CPPUNIT_ASSERT_EQUAL(false, cb2IsThisLastCallback);
    CPPUNIT_ASSERT_EQUAL(97u, cb3DetailEntryNo);
    CPPUNIT_ASSERT_EQUAL(false, cb3IsThisLastCallback);
  }

  void testSequenceCallbackReturningFalse()
  {
//    tx_type::DetailLevels detailLevels("10|4|2");
//    DetailLevelsSequencer sequencer(detailLevels);
//
//    tx_type::DetailEntryNo cb1DetailEntryNo, cb2DetailEntryNo = 98, cb3DetailEntryNo = 99;
//    bool cb1IsThisLastCallback, cb2IsThisLastCallback = false, cb3IsThisLastCallback = false;
//
//    using namespace std::placeholders;
//    sequencer.pushCallback(std::bind(&DetailLevelsSequencerTest::callback,
//                                     this, _1, _2, cb1DetailEntryNo, cb1IsThisLastCallback, false));
//    sequencer.pushCallback(std::bind(&DetailLevelsSequencerTest::callback,
//                                     this, _1, _2, cb2DetailEntryNo, cb2IsThisLastCallback));
//    sequencer.pushCallback(std::bind(&DetailLevelsSequencerTest::callback,
//                                     this, _1, _2, cb3DetailEntryNo, cb3IsThisLastCallback));
//
//    CPPUNIT_ASSERT_EQUAL(false, sequencer.run());
//    CPPUNIT_ASSERT_EQUAL(10u, cb1DetailEntryNo);
//    CPPUNIT_ASSERT_EQUAL(false, cb1IsThisLastCallback);
//    CPPUNIT_ASSERT_EQUAL(98u, cb2DetailEntryNo);
//    CPPUNIT_ASSERT_EQUAL(false, cb2IsThisLastCallback);
//    CPPUNIT_ASSERT_EQUAL(99u, cb3DetailEntryNo);
//    CPPUNIT_ASSERT_EQUAL(false, cb3IsThisLastCallback);
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(DetailLevelsSequencerTest);
} /* namespace display */
} /* namespace tax */
