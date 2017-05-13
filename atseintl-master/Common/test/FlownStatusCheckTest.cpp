#include "test/include/CppUnitHelperMacros.h"
#include "Common/FlownStatusCheck.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"

#include "test/include/TestMemHandle.h"

namespace tse
{
class FlownStatusCheckTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FlownStatusCheckTest);
  CPPUNIT_TEST(testStatus_AllUnflown);
  CPPUNIT_TEST(testStatus_AllFlown);
  CPPUNIT_TEST(testStatus_FirstFlown);
  CPPUNIT_TEST(testStatus_LastUnflown);
  CPPUNIT_TEST(testStatus_Empty);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _itin = _memHandle.insert(new Itin);
    _travelSegments = _memHandle.insert(new std::vector<TravelSeg*>);
  }

  void tearDown() { _memHandle.clear(); }

  struct PrepareSegment
  {
    PrepareSegment(uint8_t howManyFlowns, TestMemHandle& memHandle)
      : _flownCounter(howManyFlowns), _memHandle(memHandle)
    {
    }

    TravelSeg* operator()()
    {
      TravelSeg* ts = _memHandle.insert(new AirSeg);
      ts->unflown() = _flownCounter < 1;
      --_flownCounter;
      return ts;
    }

    uint8_t _flownCounter;
    TestMemHandle& _memHandle;
  };

  struct AssertSegment
  {
    AssertSegment(uint8_t howManyFlowns) : _flownCounter(howManyFlowns) {}

    void operator()(const TravelSeg* ts)
    {
      CPPUNIT_ASSERT_EQUAL(_flownCounter < 1, ts->unflown());
      --_flownCounter;
    }

    uint8_t _flownCounter;
  };

  void generateSegments(uint8_t howManySegments, uint8_t howManyFlowns)
  {
    std::generate_n(std::back_inserter(*_travelSegments),
                    howManySegments,
                    PrepareSegment(howManyFlowns, _memHandle));

    std::for_each(_travelSegments->begin(), _travelSegments->end(), AssertSegment(howManyFlowns));

    _itin->travelSeg() = *_travelSegments;
    _flownStatusCheck = _memHandle.insert(new FlownStatusCheck(*_itin));
  }

  void testStatus_AllUnflown()
  {
    generateSegments(5, 0);
    CPPUNIT_ASSERT_EQUAL(FlownStatusCheck::Unflown, _flownStatusCheck->status());
  }

  void testStatus_AllFlown()
  {
    generateSegments(4, 4);
    CPPUNIT_ASSERT_EQUAL(FlownStatusCheck::FullyFlown, _flownStatusCheck->status());
  }

  void testStatus_FirstFlown()
  {
    generateSegments(6, 1);
    CPPUNIT_ASSERT_EQUAL(FlownStatusCheck::PartiallyFlown, _flownStatusCheck->status());
  }

  void testStatus_LastUnflown()
  {
    generateSegments(6, 5);
    CPPUNIT_ASSERT_EQUAL(FlownStatusCheck::PartiallyFlown, _flownStatusCheck->status());
  }

  void testStatus_Empty()
  {
    generateSegments(0, 0);
    CPPUNIT_ASSERT_EQUAL(FlownStatusCheck::Empty, _flownStatusCheck->status());
  }

private:
  TestMemHandle _memHandle;
  FlownStatusCheck* _flownStatusCheck;
  std::vector<TravelSeg*>* _travelSegments;
  Itin* _itin;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FlownStatusCheckTest);
};
