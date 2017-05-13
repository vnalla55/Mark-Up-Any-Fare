#include <iostream>
#include <vector>

#include "Common/IAIbfUtils.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestDataBuilders.h"
#include "test/include/TestMemHandle.h"

using namespace std;
namespace tse
{
  class VectorIntoRangeSplitterTest : public CppUnit::TestFixture
  {
    CPPUNIT_TEST_SUITE(VectorIntoRangeSplitterTest);

    CPPUNIT_TEST(testSplitterWithIntegers);
    CPPUNIT_TEST(testSplitterWithEmptyVector);
    CPPUNIT_TEST(checkODCtoFMMapInversion);
    CPPUNIT_TEST_SUITE_END();

    public:
    void testSplitterWithIntegers()
    {
      std::vector<int> integers = { 1,2,3,4,3,5,7,4,1 };
      VectorIntoRangeSplitter<int> intSplitter = VectorIntoRangeSplitter<int>(integers, true);
      // range splitter without predicates should return range spanning whole vector
      CPPUNIT_ASSERT(intSplitter.hasMore() == true);
      std::pair<int, int> range = intSplitter.getNextRange();
      CPPUNIT_ASSERT(range == std::make_pair(0, 8));
      // this action exhausts the iterator
      CPPUNIT_ASSERT(intSplitter.hasMore() == false);
      // test anew, this time with a predicate
      intSplitter.reset();
      CPPUNIT_ASSERT(intSplitter.hasMore() == true);
      // split should happen if following element is smaller that the previous one
      intSplitter.addPredicate([](int a, int b){return (b < a);});
      CPPUNIT_ASSERT(intSplitter.getNextRange() == std::make_pair(0, 3));
      CPPUNIT_ASSERT(intSplitter.getNextRange() == std::make_pair(4, 6));
      CPPUNIT_ASSERT(intSplitter.getNextRange() == std::make_pair(7, 7));
      CPPUNIT_ASSERT(intSplitter.hasMore() == true);
      CPPUNIT_ASSERT(intSplitter.getNextRange() == std::make_pair(8, 8));
      CPPUNIT_ASSERT(intSplitter.hasMore() == false);
      CPPUNIT_ASSERT(intSplitter.getNextRange() == std::make_pair(-1, -1));
      // test again, this time with an additional predicate: split on encountering '2'
      intSplitter.reset();
      intSplitter.addPredicate([](int a ,int b){ return b == 2; });
      CPPUNIT_ASSERT(intSplitter.getNextRange() == std::make_pair(0, 0));
      CPPUNIT_ASSERT(intSplitter.getNextRange() == std::make_pair(1, 3));
      CPPUNIT_ASSERT(intSplitter.getNextRange() == std::make_pair(4, 6));
      CPPUNIT_ASSERT(intSplitter.getNextRange() == std::make_pair(7, 7));
      CPPUNIT_ASSERT(intSplitter.getNextRange() == std::make_pair(8, 8));
      CPPUNIT_ASSERT(intSplitter.hasMore() == false);
      CPPUNIT_ASSERT(intSplitter.getNextRange() == std::make_pair(-1, -1));

      // test debug
      std::vector<std::string> expectedDiagOutput;
      expectedDiagOutput.push_back("SPLITTER INITIALIZED WITH 9 ITEMS\n");
      expectedDiagOutput.push_back("NEXT RANGE: 0-8\n");
      expectedDiagOutput.push_back("RESET\n");
      expectedDiagOutput.push_back("NEXT RANGE: 0-3\n");
      expectedDiagOutput.push_back("NEXT RANGE: 4-6\n");
      expectedDiagOutput.push_back("NEXT RANGE: 7-7\n");
      expectedDiagOutput.push_back("NEXT RANGE: 8-8\n");
      expectedDiagOutput.push_back("NEXT RANGE: -1--1\n");
      expectedDiagOutput.push_back("RESET\n");
      expectedDiagOutput.push_back("NEXT RANGE: 0-0\n");
      expectedDiagOutput.push_back("NEXT RANGE: 1-3\n");
      expectedDiagOutput.push_back("NEXT RANGE: 4-6\n");
      expectedDiagOutput.push_back("NEXT RANGE: 7-7\n");
      expectedDiagOutput.push_back("NEXT RANGE: 8-8\n");
      expectedDiagOutput.push_back("NEXT RANGE: -1--1\n");
      CPPUNIT_ASSERT(intSplitter.getDebugMessages() == expectedDiagOutput);
    }

    void testSplitterWithEmptyVector()
    {
      std::vector<int> emptyVector;
      VectorIntoRangeSplitter<int> intSplitter = VectorIntoRangeSplitter<int>(emptyVector, false);
      CPPUNIT_ASSERT(intSplitter.hasMore() == false);
      CPPUNIT_ASSERT(intSplitter.getNextRange() == std::make_pair(-1, -1));
      // resetting doesn't change anything. still no valid range should be returned
      intSplitter.reset();
      CPPUNIT_ASSERT(intSplitter.hasMore() == false);
      CPPUNIT_ASSERT(intSplitter.getNextRange() == std::make_pair(-1, -1));
    }

    void checkODCtoFMMapInversion()
    {
      IAIbfUtils::FMsForBranding testMap;
      IAIbfUtils::OdcTuple tuple1("AAA", "BBB", "C1");
      IAIbfUtils::OdcTuple tuple2("AAA", "CCC", "C1");
      IAIbfUtils::OdcTuple tuple3("AAA", "DDD", "C1");

      FareMarket fm1, fm2, fm3;

      testMap[tuple1].push_back(&fm1);
      testMap[tuple2].push_back(&fm1);
      testMap[tuple3].push_back(&fm1);

      testMap[tuple1].push_back(&fm2);
      testMap[tuple2].push_back(&fm2);
      testMap[tuple3].push_back(&fm3);

      std::set<IAIbfUtils::OdcTuple> allTuples;
      allTuples.insert(tuple1);
      allTuples.insert(tuple2);
      allTuples.insert(tuple3);

      IAIbfUtils::OdcsForBranding resultMap;
      IAIbfUtils::invertOdcToFmMap(testMap, resultMap);

      CPPUNIT_ASSERT(resultMap.size() == 3);
      CPPUNIT_ASSERT(resultMap.at(&fm1).size() == 3);
      CPPUNIT_ASSERT(resultMap.at(&fm2).size() == 2);
      CPPUNIT_ASSERT(resultMap.at(&fm3).size() == 1);

      CPPUNIT_ASSERT(resultMap.at(&fm3).find(tuple3) != resultMap.at(&fm3).end());

      CPPUNIT_ASSERT(resultMap.at(&fm2).find(tuple1) != resultMap.at(&fm2).end());
      CPPUNIT_ASSERT(resultMap.at(&fm2).find(tuple2) != resultMap.at(&fm2).end());

      CPPUNIT_ASSERT(resultMap.at(&fm1).find(tuple1) != resultMap.at(&fm1).end());
      CPPUNIT_ASSERT(resultMap.at(&fm1).find(tuple2) != resultMap.at(&fm1).end());
      CPPUNIT_ASSERT(resultMap.at(&fm1).find(tuple3) != resultMap.at(&fm1).end());
    }

  };
  CPPUNIT_TEST_SUITE_REGISTRATION(VectorIntoRangeSplitterTest);

  class IAIbfUtilsTest : public CppUnit::TestFixture
  {
    CPPUNIT_TEST_SUITE(IAIbfUtilsTest);

    CPPUNIT_TEST(legInfoToString);
    CPPUNIT_TEST_SUITE_END();

    public:
    void legInfoToString()
    {
      AirSeg seg1, seg2, seg3, seg4, seg5, seg6, seg7, seg8;
      seg1.legId() = 0;
      seg1.setBrandCode("SV");
      seg1.segmentOrder() = 1;
      seg2.legId() = 0;
      seg2.setBrandCode("SV");
      seg2.segmentOrder() = 2;
      seg3.legId() = 0;
      seg3.setBrandCode("SV");
      seg3.segmentOrder() = 3;
      seg4.legId() = 1;
      seg4.segmentType() = Arunk;
      seg4.segmentOrder() = 4;
      seg5.legId() = 2;
      seg5.setBrandCode("FL");
      seg5.segmentOrder() = 5;
      seg6.legId() = 2;
      seg6.setBrandCode("FL");
      seg6.segmentOrder() = 6;
      seg7.legId() = 2;
      seg7.setBrandCode("FL");
      seg7.segmentOrder() = 7;
      seg8.legId() = 3;
      seg8.setBrandCode("BZ");
      seg8.segmentOrder() = 8;
      std::vector<TravelSeg*> leg1 = { &seg1, &seg2, &seg3 };
      std::vector<TravelSeg*> leg2 = { &seg4 };
      std::vector<TravelSeg*> leg3 = { &seg5, &seg6, &seg7 };
      std::vector<TravelSeg*> leg4 = { &seg8 };

      ItinLegs itinLegs;
      itinLegs.push_back(leg1);
      itinLegs.push_back(leg2);
      itinLegs.push_back(leg3);
      itinLegs.push_back(leg4);

      std::string expectedResult = "S1-3*BRSV*LG1$S5-7*BRFL*LG2$S8*BRBZ*LG3";
      CPPUNIT_ASSERT(IAIbfUtils::legInfoToString(itinLegs) == expectedResult);
    }
  };
  CPPUNIT_TEST_SUITE_REGISTRATION(IAIbfUtilsTest);
}
