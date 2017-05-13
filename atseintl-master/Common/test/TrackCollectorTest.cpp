#include "test/include/CppUnitHelperMacros.h"

#include "Diagnostic/DiagCollector.h"
#include "Common/TrackCollector.h"

#include "test/include/TestMemHandle.h"

namespace tse
{

class TrackCollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TrackCollectorTest);

  CPPUNIT_TEST(testCollectHit_zeroHit);
  CPPUNIT_TEST(testCollectHit_oneHit);
  CPPUNIT_TEST(testCollectHit_tooManyHits);
  CPPUNIT_TEST(testCollectHit_trimmingLabel);

  CPPUNIT_TEST(testFormat_noLabels);
  CPPUNIT_TEST(testFormat_oneLabel);
  CPPUNIT_TEST(testFormat_twoLabels);
  CPPUNIT_TEST(testFormat_threeLabels);
  CPPUNIT_TEST(testFormat_fourLabels);
  CPPUNIT_TEST(testFormat_fullLine);

  CPPUNIT_TEST(testGetLabels);
  CPPUNIT_TEST(testAssign);

  CPPUNIT_TEST_SUITE_END();

protected:
  typedef TrackCollector::Counter Counter;
  typedef TrackCollector::Label Label;

  static const std::string LABEL, LABEL1, LABEL2, LONG_LABEL;

  TrackCollector* _tracker;
  TestMemHandle _memH;

public:
  void setUp() { _tracker = _memH(new TrackCollector); }

  void tearDown() { _memH.clear(); }

  void testCollectHit_zeroHit() { CPPUNIT_ASSERT_EQUAL(Counter(0), (*_tracker)[LABEL]); }

  void testCollectHit_oneHit()
  {
    _tracker->collect(LABEL);

    CPPUNIT_ASSERT_EQUAL(Counter(1), (*_tracker)[LABEL]);
  }

  void testCollectHit_tooManyHits()
  {
    (*_tracker)[LABEL] = TrackCollector::MAX_COUNTER;
    _tracker->collect(LABEL);
    _tracker->collect(LABEL);

    CPPUNIT_ASSERT_EQUAL(TrackCollector::MAX_COUNTER, (*_tracker)[LABEL]);
  }

  void testCollectHit_trimmingLabel()
  {
    _tracker->collect(LONG_LABEL);

    CPPUNIT_ASSERT_EQUAL(Counter(0), (*_tracker)[LONG_LABEL]);

    std::string trimmed_label = LONG_LABEL.substr(0, TrackCollector::LABEL_LENGTH);
    CPPUNIT_ASSERT_EQUAL(Counter(1), (*_tracker)[trimmed_label]);
  }

  DiagCollector& createDiag()
  {
    DiagCollector* diag = _memH(new DiagCollector);
    diag->activate();
    return *diag;
  }

  void testFormat_noLabels()
  {
    DiagCollector& diag = createDiag();
    _tracker->print(diag);

    CPPUNIT_ASSERT_EQUAL(std::string(), diag.str());
  }

  void testFormat_oneLabel()
  {
    _tracker->collect(LABEL);

    DiagCollector& diag = createDiag();
    _tracker->print(diag);

    CPPUNIT_ASSERT_EQUAL(std::string("     LABEL0 :    1  \n"), diag.str());
  }

  void testFormat_twoLabels()
  {
    (*_tracker)[LABEL] = (*_tracker)[LABEL1] = TrackCollector::MAX_COUNTER;

    DiagCollector& diag = createDiag();
    _tracker->print(diag);

    CPPUNIT_ASSERT_EQUAL(std::string("     LABEL0 : 65535       LABEL1 : 65535  \n"), diag.str());
  }

  void testFormat_threeLabels()
  {
    _tracker->collect(LABEL);
    _tracker->collect(LABEL1);
    _tracker->collect(LONG_LABEL);

    DiagCollector& diag = createDiag();
    _tracker->print(diag);

    CPPUNIT_ASSERT_EQUAL(
        std::string("     LABEL0 :    1       LABEL1 :    1   VERY_LONG_ :    1\n"), diag.str());
  }

  void testFormat_fourLabels()
  {
    _tracker->collect(LABEL);
    _tracker->collect(LABEL1);
    _tracker->collect(LONG_LABEL);
    _tracker->collect(LABEL2);

    DiagCollector& diag = createDiag();
    _tracker->print(diag);

    CPPUNIT_ASSERT_EQUAL(std::string("     LABEL0 :    1       LABEL1 :    1       LABEL2 :    1\n"
                                     " VERY_LONG_ :    1  \n"),
                         diag.str());
  }

  void testFormat_fullLine()
  {
    (*_tracker)[_tracker->cutLabel(LABEL + LONG_LABEL)] =
        (*_tracker)[_tracker->cutLabel(LABEL1 + LONG_LABEL)] =
            (*_tracker)[_tracker->cutLabel(LABEL2 + LONG_LABEL)] = TrackCollector::MAX_COUNTER;

    DiagCollector& diag = createDiag();
    _tracker->print(diag);

    CPPUNIT_ASSERT_EQUAL(
        std::string(" LABEL0VERY : 65535   LABEL1VERY : 65535   LABEL2VERY : 65535\n"), diag.str());
  }

  void testGetLabels()
  {
    _tracker->collect(LABEL);
    _tracker->collect(LABEL1);
    _tracker->collect(LABEL2);

    std::vector<Label> labels, expect;
    expect.push_back(LABEL);
    expect.push_back(LABEL1);
    expect.push_back(LABEL2);

    _tracker->getLabels(labels);

    CPPUNIT_ASSERT_EQUAL(expect, labels);
  }

  void testAssign()
  {
    TrackCollector tc;
    tc.collect(LABEL);
    tc.collect(LABEL1);
    tc.collect(LABEL2);

    _tracker->assign(tc);

    DiagCollector& diag = createDiag();
    _tracker->print(diag);

    CPPUNIT_ASSERT_EQUAL(
        std::string("     LABEL0 : 10001       LABEL1 : 10001       LABEL2 : 10001\n"), diag.str());
  }
};

const std::string TrackCollectorTest::LABEL = "LABEL0", TrackCollectorTest::LABEL1 = "LABEL1",
                  TrackCollectorTest::LABEL2 = "LABEL2",
                  TrackCollectorTest::LONG_LABEL = "VERY_LONG_LABEL";

CPPUNIT_TEST_SUITE_REGISTRATION(TrackCollectorTest);

} // tse

namespace
{

std::ostream& operator<<(std::ostream& os, const std::vector<tse::TrackCollector::Label>& labels)
{
  std::copy(labels.begin(), labels.end(), std::ostream_iterator<std::string>(os, " "));
  return os;
}

} // namespace
