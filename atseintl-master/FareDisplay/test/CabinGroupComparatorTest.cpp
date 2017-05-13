#include "test/include/CppUnitHelperMacros.h"
#include "FareDisplay/CabinGroupComparator.h"
#include "Common/TseStringTypes.h"
#include "Common/TseEnums.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/FareMarket.h"
#include "FareDisplay/Group.h"

#include "test/include/TestMemHandle.h"

using namespace std;

namespace tse
{
class CabinGroupComparatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(CabinGroupComparatorTest);

  CPPUNIT_TEST(testCabin_Priority_Empty_Return_One);
  CPPUNIT_TEST(testCabin_Priority_Return_InclCode);
  CPPUNIT_TEST(testCabin_Priority_Not_Return_InclCode);
  CPPUNIT_TEST(testCompare_Cabin_InclCodes_VecEmpty_Return_EQUAL);
  CPPUNIT_TEST(testCompare_Fare1_Fail_InclCode4_Fare2_Pass_InclCode4_Return_TRUE);
  CPPUNIT_TEST(testCompare_Fare1_Pass_InclCode4_Fare2_Fail_InclCode4_Return_TRUE);
  CPPUNIT_TEST(testCompare_Fare1_Pass_InclCode4_Fare2_Pass_InclCode4_Return_EQUAL);
  CPPUNIT_TEST(testPrepare_AB);
  CPPUNIT_TEST(testPrepare_YBPBSBJBFB);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _fTrx = _memHandle.create<FareDisplayTrx>();
    _fRequest = _memHandle.create<FareDisplayRequest>();
    _pFare1 = _memHandle.create<PaxTypeFare>();
    _pFare2 = _memHandle.create<PaxTypeFare>();
    _fDinfo1 = _memHandle.create<FareDisplayInfo>();
    _fDinfo2 = _memHandle.create<FareDisplayInfo>();
    _pFare1->fareDisplayInfo() = _fDinfo1;
    _pFare2->fareDisplayInfo() = _fDinfo2;

    _fTrx->setRequest(_fRequest);
    _fDinfo1->setFareDisplayTrx(_fTrx);
    _fDinfo2->setFareDisplayTrx(_fTrx);

    _group = _memHandle.create<Group>();
  }
  void tearDown() { _memHandle.clear(); }

  void testCabin_Priority_Empty_Return_One()
  {
    std::map<uint8_t, std::pair<uint8_t, std::string> > _priorityMap;

    CabinGroupComparator comparator;
    comparator._priorityMap.clear();
    uint8_t result = comparator.inclusionNumPassed(*_pFare1);
    CPPUNIT_ASSERT(result == 1);
  }

  void testCabin_Priority_Return_InclCode()
  {
    uint8_t inclusionNumber1 = 1; // PB
    uint8_t inclusionNumber2 = 4; // BB
    uint8_t inclusionNumber3 = 6; // YB
    _pFare1->setFareStatusForInclCode( inclusionNumber3 , true);

    std::pair<uint8_t, std::string> inclCode1 = make_pair(inclusionNumber1, "PB");
    std::pair<uint8_t, std::string> inclCode2 = make_pair(inclusionNumber2, "BB");
    std::pair<uint8_t, std::string> inclCode3 = make_pair(inclusionNumber3, "YB");

    CabinGroupComparator comparator;
    comparator._priorityMap[0] = inclCode1;
    comparator._priorityMap[1] = inclCode2;
    comparator._priorityMap[2] = inclCode3;
    uint8_t result = comparator.inclusionNumPassed(*_pFare1);
    CPPUNIT_ASSERT(result == 2);
  }

  void testCabin_Priority_Not_Return_InclCode()
  {
    uint8_t inclusionNumber = 4; // BB
    _pFare1->setFareStatusForInclCode( inclusionNumber , false);

    std::pair<uint8_t, std::string> inclCode1 = make_pair(1, "PB");
    std::pair<uint8_t, std::string> inclCode2 = make_pair(4, "BB");

    CabinGroupComparator comparator;
    comparator._priorityMap[0] = inclCode1;
    comparator._priorityMap[1] = inclCode2;
    uint8_t result = comparator.inclusionNumPassed(*_pFare1);
    CPPUNIT_ASSERT(result == 3);
  }

  void testCompare_Cabin_InclCodes_VecEmpty_Return_EQUAL()
  {
    CabinGroupComparator comparator;
    comparator._priorityMap.clear();
    Comparator::Result result;
    result = comparator.compare(*_pFare1, *_pFare2);
    CPPUNIT_ASSERT(result == Comparator::EQUAL);
  }

  void testCompare_Fare1_Fail_InclCode4_Fare2_Pass_InclCode4_Return_TRUE()
  {
    uint8_t inclusionNumber = 4; // BB
    _pFare1->setFareStatusForInclCode( inclusionNumber , false);
    _pFare2->setFareStatusForInclCode( inclusionNumber , true);

    std::pair<uint8_t, std::string> inclCode1 = make_pair(1, "PB");
    std::pair<uint8_t, std::string> inclCode2 = make_pair(4, "BB");
    std::pair<uint8_t, std::string> inclCode3 = make_pair(6, "YB");

    CabinGroupComparator comparator;
    comparator._priorityMap[0] = inclCode1;
    comparator._priorityMap[1] = inclCode2;
    comparator._priorityMap[2] = inclCode3;
    Comparator::Result result;
    result = comparator.compare(*_pFare1, *_pFare2);
    CPPUNIT_ASSERT(result == Comparator::FALSE);
  }

  void testCompare_Fare1_Pass_InclCode4_Fare2_Fail_InclCode4_Return_TRUE()
  {
    uint8_t inclusionNumber = 4; // FB
    _pFare1->setFareStatusForInclCode( inclusionNumber , true);
    _pFare2->setFareStatusForInclCode( inclusionNumber , false);

    std::pair<uint8_t, std::string> inclCode1 = make_pair(1, "PB");
    std::pair<uint8_t, std::string> inclCode2 = make_pair(4, "BB");
    std::pair<uint8_t, std::string> inclCode3 = make_pair(6, "YB");

    CabinGroupComparator comparator;
    comparator._priorityMap[0] = inclCode1;
    comparator._priorityMap[1] = inclCode2;
    comparator._priorityMap[2] = inclCode3;
    Comparator::Result result;
    result = comparator.compare(*_pFare1, *_pFare2);
    CPPUNIT_ASSERT(result == Comparator::TRUE);
  }

  void testCompare_Fare1_Pass_InclCode4_Fare2_Pass_InclCode4_Return_EQUAL()
  {
    uint8_t inclusionNumber = 4; // JB
    _pFare1->setFareStatusForInclCode( inclusionNumber , true);
    _pFare2->setFareStatusForInclCode( inclusionNumber , true);

    std::pair<uint8_t, std::string> inclCode1 = make_pair(1, "PB");
    std::pair<uint8_t, std::string> inclCode2 = make_pair(4, "BB");
    std::pair<uint8_t, std::string> inclCode3 = make_pair(6, "YB");

    CabinGroupComparator comparator;
    comparator._priorityMap[0] = inclCode1;
    comparator._priorityMap[1] = inclCode2;
    comparator._priorityMap[2] = inclCode3;
    Comparator::Result result;
    result = comparator.compare(*_pFare1, *_pFare2);
    CPPUNIT_ASSERT(result == Comparator::EQUAL);
  }

  void testPrepare_AB()
  {
    _fRequest->requestedInclusionCode() = "PBFBJBBBSBYB";

    CabinGroupComparator comparator;
    comparator.group() = _group;

    comparator.prepare(*_fTrx);

    CPPUNIT_ASSERT(!comparator._priorityMap.empty());

    std::map<uint8_t, std::pair<uint8_t, std::string> >::const_iterator itr =
                                                         comparator._priorityMap.begin();
    CPPUNIT_ASSERT(itr->first == 0);

    std::pair<uint8_t, std::string> p1 = (*itr).second;

    CPPUNIT_ASSERT(p1.first == 1);
    CPPUNIT_ASSERT(p1.second == "PB");

    itr = comparator._priorityMap.find(2);
    p1 = (*itr).second;
    CPPUNIT_ASSERT(p1.first == 3);
    CPPUNIT_ASSERT(p1.second == "JB");

    std::vector<uint8_t> multiInclusionCabins = _fTrx->fdResponse()->multiInclusionCabins();

    CPPUNIT_ASSERT(multiInclusionCabins.at(0) == 1);
    CPPUNIT_ASSERT(multiInclusionCabins.at(5) == 6);
  }

  void testPrepare_YBPBSBJBFB()
  {
    _fRequest->requestedInclusionCode() = "YBPBSBJBFB";
    // YBPBSBJBFB - inclusion enum - 61532
    CabinGroupComparator comparator;
    comparator.group() = _group;

    comparator.prepare(*_fTrx);

    CPPUNIT_ASSERT(!comparator._priorityMap.empty());

    std::map<uint8_t, std::pair<uint8_t, std::string> >::const_iterator itr =
                                                         comparator._priorityMap.begin();
    CPPUNIT_ASSERT(itr->first == 0);

    std::pair<uint8_t, std::string> p1 = (*itr).second;

    CPPUNIT_ASSERT(p1.first == 6);
    CPPUNIT_ASSERT(p1.second == "YB");

    itr = comparator._priorityMap.find(2);
    p1 = (*itr).second;
    CPPUNIT_ASSERT(p1.first == 5);
    CPPUNIT_ASSERT(p1.second == "SB");

    std::vector<uint8_t> multiInclusionCabins = _fTrx->fdResponse()->multiInclusionCabins();

    CPPUNIT_ASSERT(multiInclusionCabins.at(0) == 6);
    CPPUNIT_ASSERT(multiInclusionCabins.at(4) == 2);
  }

private:
  PaxTypeFare* _pFare1;
  PaxTypeFare* _pFare2;
  FareDisplayInfo* _fDinfo1;
  FareDisplayInfo* _fDinfo2;
  FareDisplayTrx* _fTrx;
  FareDisplayRequest* _fRequest;
  Group* _group;

  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(CabinGroupComparatorTest);
}
