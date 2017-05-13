#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "FareDisplay/Group.h"
#include "FareDisplay/S8BrandComparator.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"

using namespace std;

namespace tse
{
class S8BrandComparatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(S8BrandComparatorTest);

  CPPUNIT_TEST(testAlphabeticalComparison_TRUE);
  CPPUNIT_TEST(testAlphabeticalComparison_FALSE);
  CPPUNIT_TEST(testAlphabeticalComparison_EQUAL);
  CPPUNIT_TEST(testBrandPriority_Return_zero);
  CPPUNIT_TEST(testBrandPriority_Return_Tier);
  CPPUNIT_TEST(testCompare_Tiers_PriorityMapEmpty_Return_FALSE);
  CPPUNIT_TEST(testCompare_Tiers_Different_Program_Same_Brand_Return_TRUE);
  CPPUNIT_TEST(testCompare_Tiers_Same_Program_Different_Brand_Return_TRUE);
  CPPUNIT_TEST(testCompare_Tiers_Return_FALSE);
  CPPUNIT_TEST(testCompare_Tiers_EQUAL_Different_Program_Return_FALSE);
  CPPUNIT_TEST(testCompare_Tiers_EQUAL_Return_EQUAL);
  CPPUNIT_TEST(testPrepare);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _fTrx = _memHandle.create<FareDisplayTrx>();
    _pFare1 = _memHandle.create<PaxTypeFare>();
    _pFare2 = _memHandle.create<PaxTypeFare>();
    _fDinfo1 = _memHandle.create<FareDisplayInfo>();
    _fDinfo2 = _memHandle.create<FareDisplayInfo>();
    _pFare1->fareDisplayInfo() = _fDinfo1;
    _pFare2->fareDisplayInfo() = _fDinfo2;

    _group = _memHandle.create<Group>();
    _market = _memHandle.create<FareMarket>();
    _market->governingCarrier() = "AA";
    _fTrx->fareMarket().push_back(_market);
  }
  void tearDown() { _memHandle.clear(); }

  void testAlphabeticalComparison_TRUE()
  {
    std::pair<ProgramCode, BrandCode> progBrand1 = make_pair("DU", "APP");
    std::pair<ProgramCode, BrandCode> progBrand2 = make_pair("US", "ABB");
    _fDinfo1->setProgramBrand(progBrand1);
    _fDinfo2->setProgramBrand(progBrand2);

    S8BrandComparator comparator;
    Comparator::Result result;
    result = comparator.alphabeticalComparison(*_pFare1, *_pFare2);
    CPPUNIT_ASSERT(result == Comparator::TRUE);
  }
  void testAlphabeticalComparison_FALSE()
  {
    std::pair<ProgramCode, BrandCode> progBrand1 = make_pair("UX", "APP");
    std::pair<ProgramCode, BrandCode> progBrand2 = make_pair("US", "ABB");
    _fDinfo1->setProgramBrand(progBrand1);
    _fDinfo2->setProgramBrand(progBrand2);

    S8BrandComparator comparator;
    Comparator::Result result;
    result = comparator.alphabeticalComparison(*_pFare1, *_pFare2);
    CPPUNIT_ASSERT(result == Comparator::FALSE);
  }

  void testAlphabeticalComparison_EQUAL()
  {
    std::pair<ProgramCode, BrandCode> progBrand1 = make_pair("UX", "APP");
    std::pair<ProgramCode, BrandCode> progBrand2 = make_pair("UX", "APP");
    _fDinfo1->setProgramBrand(progBrand1);
    _fDinfo2->setProgramBrand(progBrand2);

    S8BrandComparator comparator;
    Comparator::Result result;
    result = comparator.alphabeticalComparison(*_pFare1, *_pFare2);
    CPPUNIT_ASSERT(result == Comparator::EQUAL);
  }

  void testCompare_FALSE()
  {
    std::pair<ProgramCode, BrandCode> progBrand1 = make_pair("UX", "APP");
    std::pair<ProgramCode, BrandCode> progBrand2 = make_pair("US", "ABB");
    _fDinfo1->setProgramBrand(progBrand1);
    _fDinfo2->setProgramBrand(progBrand2);

    S8BrandComparator comparator;
    comparator._priorityMap.clear();
    Comparator::Result result;
    result = comparator.compare(*_pFare1, *_pFare2);
    CPPUNIT_ASSERT(result == Comparator::FALSE);
  }

  void testBrandPriority_Return_zero()
  {
    std::pair<ProgramCode, BrandCode> progBrand1 = make_pair("UX", "APP");

    S8BrandComparator comparator;
    comparator._priorityMap.clear();
    uint64_t result = comparator.brandPriority(progBrand1);
    CPPUNIT_ASSERT(result == 0);
  }

  void testBrandPriority_Return_Tier()
  {
    std::pair<ProgramCode, BrandCode> progBrand1 = make_pair("UX", "APP");
    std::pair<ProgramCode, BrandCode> progBrand2 = make_pair("US", "ABB");
    _fDinfo1->setProgramBrand(progBrand1);
    _fDinfo2->setProgramBrand(progBrand2);

    S8BrandComparator comparator;
    comparator._priorityMap.insert(make_pair(progBrand1, 10));
    comparator._priorityMap.insert(make_pair(progBrand2, 100));
    uint64_t result = comparator.brandPriority(progBrand2);
    CPPUNIT_ASSERT(result == 100);
  }

  void testCompare_Tiers_PriorityMapEmpty_Return_FALSE()
  {
    std::pair<ProgramCode, BrandCode> progBrand1 = make_pair("UX", "APP");
    std::pair<ProgramCode, BrandCode> progBrand2 = make_pair("US", "ABB");
    _fDinfo1->setProgramBrand(progBrand1);
    _fDinfo2->setProgramBrand(progBrand2);

    S8BrandComparator comparator;
    comparator._priorityMap.clear();
    Comparator::Result result;
    result = comparator.compare(*_pFare1, *_pFare2);
    CPPUNIT_ASSERT(result == Comparator::FALSE);
  }

  void testCompare_Tiers_Different_Program_Same_Brand_Return_TRUE()
  {
    std::pair<ProgramCode, BrandCode> progBrand1 = make_pair("US", "APP");
    std::pair<ProgramCode, BrandCode> progBrand2 = make_pair("UX", "APP");
    _fDinfo1->setProgramBrand(progBrand1);
    _fDinfo2->setProgramBrand(progBrand2);

    S8BrandComparator comparator;
    comparator._priorityMap.insert(make_pair(progBrand1, 10));
    comparator._priorityMap.insert(make_pair(progBrand2, 100));
    Comparator::Result result;
    result = comparator.compare(*_pFare1, *_pFare2);
    CPPUNIT_ASSERT(result == Comparator::TRUE);
  }

  void testCompare_Tiers_Same_Program_Different_Brand_Return_TRUE()
  {
    std::pair<ProgramCode, BrandCode> progBrand1 = make_pair("UX", "APP");
    std::pair<ProgramCode, BrandCode> progBrand2 = make_pair("UX", "APY");
    _fDinfo1->setProgramBrand(progBrand1);
    _fDinfo2->setProgramBrand(progBrand2);

    S8BrandComparator comparator;
    comparator._priorityMap.insert(make_pair(progBrand1, 10));
    comparator._priorityMap.insert(make_pair(progBrand2, 100));
    Comparator::Result result;
    result = comparator.compare(*_pFare1, *_pFare2);
    CPPUNIT_ASSERT(result == Comparator::TRUE);
  }

  void testCompare_Tiers_Return_FALSE()
  {
    std::pair<ProgramCode, BrandCode> progBrand1 = make_pair("AA", "AAA");
    std::pair<ProgramCode, BrandCode> progBrand2 = make_pair("XX", "YYY");
    _fDinfo1->setProgramBrand(progBrand1);
    _fDinfo2->setProgramBrand(progBrand2);

    S8BrandComparator comparator;
    comparator._priorityMap.insert(make_pair(progBrand1, 100));
    comparator._priorityMap.insert(make_pair(progBrand2, 10));
    Comparator::Result result;
    result = comparator.compare(*_pFare1, *_pFare2);
    CPPUNIT_ASSERT(result == Comparator::FALSE);
  }

  void testCompare_Tiers_EQUAL_Different_Program_Return_FALSE()
  {
    std::pair<ProgramCode, BrandCode> progBrand1 = make_pair("UX", "APP");
    std::pair<ProgramCode, BrandCode> progBrand2 = make_pair("AA", "APP");
    _fDinfo1->setProgramBrand(progBrand1);
    _fDinfo2->setProgramBrand(progBrand2);

    S8BrandComparator comparator;
    comparator._priorityMap.insert(make_pair(progBrand1, 10));
    comparator._priorityMap.insert(make_pair(progBrand2, 10));
    Comparator::Result result;
    result = comparator.compare(*_pFare1, *_pFare2);
    CPPUNIT_ASSERT(result == Comparator::FALSE);
  }

  void testCompare_Tiers_EQUAL_Return_EQUAL()
  {
    std::pair<ProgramCode, BrandCode> progBrand1 = make_pair("AA", "APP");
    std::pair<ProgramCode, BrandCode> progBrand2 = make_pair("AA", "APP");
    _fDinfo1->setProgramBrand(progBrand1);
    _fDinfo2->setProgramBrand(progBrand2);

    S8BrandComparator comparator;
    comparator._priorityMap.insert(make_pair(progBrand1, 10));
    comparator._priorityMap.insert(make_pair(progBrand2, 10));
    Comparator::Result result;
    result = comparator.compare(*_pFare1, *_pFare2);
    CPPUNIT_ASSERT(result == Comparator::EQUAL);
  }

  void testPrepare()
  {
    buildProgramBrandMap();
    S8BrandComparator comparator;
    comparator.group() = _group;

    comparator.prepare(*_fTrx);
    std::map<std::pair<ProgramCode, BrandCode>,
             std::tuple<ProgramName, BrandNameS8, SystemCode> >::iterator it =
        _fTrx->fdResponse()->programBrandNameMap().begin();

    std::pair<ProgramCode, BrandCode> progBrand1 = make_pair("US", "APP");
    std::pair<ProgramCode, BrandCode> progBrand2 = make_pair("US", "ABB");

    std::pair<ProgramCode, BrandCode> p1 = (*it).first;

    // Items in the map are
    // 1. < <"US", "ABB">, <"DOMESTIC US", "ABBREVIATE"> >  with tier = 10;
    // 2. < <"US", "APP">, <"DOMESTIC US", "APPLE"> >  with tier = 99;
    // 3. < <"XX", "YYY">, <"FLIGHT ANYWHERE", "FOREVER"> >  with tier = 1;

    CPPUNIT_ASSERT(p1.first == progBrand1.first);
    CPPUNIT_ASSERT(p1.second != progBrand1.second);
    CPPUNIT_ASSERT(p1.second == progBrand2.second);

    std::pair<ProgramName, BrandNameS8> pbNames1 = make_pair("DOMESTIC US", "APPLE");
    std::pair<ProgramName, BrandNameS8> pbNames2 = make_pair("DOMESTIC US", "ABBREVIATE");
    std::tuple<ProgramName, BrandNameS8, SystemCode> p2 = (*it).second;

    CPPUNIT_ASSERT(std::get<0>(p2) == pbNames1.first);
    CPPUNIT_ASSERT(std::get<1>(p2) != pbNames1.second);
    CPPUNIT_ASSERT(std::get<1>(p2) == pbNames2.second);

    CPPUNIT_ASSERT(!comparator._priorityMap.empty());
  }

  void buildProgramBrandMap()
  {
    std::vector<OneProgramOneBrand*> spbVec;
    OneProgramOneBrand* onepb1 = _memHandle.create<OneProgramOneBrand>();
    OneProgramOneBrand* onepb2 = _memHandle.create<OneProgramOneBrand>();
    OneProgramOneBrand* onepb3 = _memHandle.create<OneProgramOneBrand>();
    spbVec.push_back(onepb1);
    spbVec.push_back(onepb2);
    spbVec.push_back(onepb3);

    onepb1->carrier() = "AA";
    onepb1->programCode() = "US";
    onepb1->programName() = "DOMESTIC US";
    onepb1->brandCode() = "APP";
    onepb1->brandName() = "APPLE";
    onepb1->tier() = 99;
    onepb1->passengerType().push_back("RUG");

    onepb2->carrier() = "AA";
    onepb2->programCode() = "US";
    onepb2->programName() = "DOMESTIC US";
    onepb2->brandCode() = "ABB";
    onepb2->brandName() = "ABBREVIATE";
    onepb2->tier() = 10;
    onepb2->passengerType().push_back("ALK");

    onepb3->carrier() = "AA";
    onepb3->programCode() = "XX";
    onepb3->programName() = "FLIGHT ANYWHERE";
    onepb3->brandCode() = "YYY";
    onepb3->brandName() = "FOREVER";
    onepb3->tier() = 1;
    onepb3->passengerType().push_back("FRR");
    _group->programBrandMap().insert(std::make_pair("AA", spbVec));
  }

private:
  PaxTypeFare* _pFare1;
  PaxTypeFare* _pFare2;
  FareDisplayInfo* _fDinfo1;
  FareDisplayInfo* _fDinfo2;
  FareDisplayTrx* _fTrx;
  Group* _group;
  FareMarket* _market;

  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(S8BrandComparatorTest);
}
