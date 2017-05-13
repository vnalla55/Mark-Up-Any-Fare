#include "test/include/CppUnitHelperMacros.h"
#include "FareDisplay/S8Brand.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayResponse.h"
#include "DataModel/FareMarket.h"
#include "Common/TseEnums.h"

#include "BrandedFares/MarketResponse.h"
#include "BrandedFares/BrandProgram.h"
#include "BrandedFares/BrandInfo.h"

#include "FareDisplay/Group.h"

#include "test/include/TestMemHandle.h"

using namespace std;

namespace tse
{
class S8BrandTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(S8BrandTest);

  CPPUNIT_TEST(testBuildOneProgramOneBrand);
  CPPUNIT_TEST(testBuildProgramBrandMap);
  CPPUNIT_TEST(testInitializeS8BrandGroup);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _fTrx = _memHandle.create<FareDisplayTrx>();
    _fTrx->fdResponse() = _memHandle.create<FareDisplayResponse>();

    _group = _memHandle.create<Group>();
    _market = _memHandle.create<FareMarket>();
    _market->governingCarrier() = "AA";
    _fTrx->fareMarket().push_back(_market);

    _mResponse = _memHandle.create<MarketResponse>();
    _bProgram1 = _memHandle.create<BrandProgram>();
    _bProgram2 = _memHandle.create<BrandProgram>();

    buildMarketResponse();
  }
  void tearDown() { _memHandle.clear(); }

  void buildMarketResponse()
  {
    BrandInfo* brand1 = _memHandle.create<BrandInfo>();
    BrandInfo* brand2 = _memHandle.create<BrandInfo>();
    BrandInfo* brand3 = _memHandle.create<BrandInfo>();

    _mResponse->carrier() = "AA";
    _mResponse->brandPrograms().push_back(_bProgram1);
    _mResponse->brandPrograms().push_back(_bProgram2);
    _bProgram1->brandsData().push_back(brand1);
    _bProgram1->brandsData().push_back(brand2);
    _bProgram2->brandsData().push_back(brand3);
    // Program1
    _bProgram1->programCode() = "US";
    _bProgram1->programName() = "DOMESTIC US";
    _bProgram1->passengerType().push_back("RUG");
    _bProgram1->programID() = "AREAONE";
    // Brand1
    brand1->brandCode() = "APP";
    brand1->brandName() = "APPLE";
    brand1->tier() = 99;
    // Brand2
    brand2->brandCode() = "ABB";
    brand2->brandName() = "ABBREVIATE";
    brand2->tier() = 10;

    // Program2 - Brand3
    _bProgram2->programCode() = "AFA";
    _bProgram2->programName() = "FLIGHT ANYWHERE";
    _bProgram2->passengerType().push_back("FLK");
    _bProgram2->programID() = "AREAONE";
    brand3->brandCode() = "FOR";
    brand3->brandName() = "FOREVER";
    brand3->tier() = 55;

    _mResponse->programIDList().push_back(_bProgram1->programID());
    _mResponse->programIDList().push_back(_bProgram2->programID());
    _mResponse->setMarketID() = 1;
  }

  void testBuildOneProgramOneBrand()
  {
    std::vector<OneProgramOneBrand*> spbVec;
    PricingTrx trx;
    S8Brand s8;
    s8.buildOneProgramOneBrand(trx, *_mResponse, spbVec);

    CPPUNIT_ASSERT(!spbVec.empty());
    CPPUNIT_ASSERT_EQUAL(3, (int)spbVec.size());
  }

  void testBuildProgramBrandMap()
  {
    std::vector<MarketResponse*> mR;
    mR.push_back(_mResponse);
    _fTrx->brandedMarketMap().insert(make_pair(1, mR));

    Group grp;
    S8Brand s8;
    s8.buildProgramBrandMap(*_fTrx, grp);

    CPPUNIT_ASSERT(!grp.programBrandMap().empty());
    CPPUNIT_ASSERT_EQUAL(1, (int)grp.programBrandMap().size());
    CPPUNIT_ASSERT_EQUAL(3, (int)(*(grp.programBrandMap().begin())).second.size());
  }

  void testInitializeS8BrandGroup()
  {
    std::vector<Group*> groups;
    S8Brand s8;
    s8.initializeS8BrandGroup(*_fTrx, groups);

    CPPUNIT_ASSERT(!groups.empty());
    CPPUNIT_ASSERT_EQUAL(1, (int)groups.size());
    CPPUNIT_ASSERT(groups[0]->groupType() == Group::GROUP_BY_S8BRAND);
    CPPUNIT_ASSERT(!_fTrx->fdResponse()->groupHeaders().empty());
    CPPUNIT_ASSERT(_fTrx->fdResponse()->groupHeaders()[0] == Group::GROUP_BY_S8BRAND);
  }

private:
  FareDisplayTrx* _fTrx;
  Group* _group;
  FareMarket* _market;
  MarketResponse* _mResponse;
  BrandProgram* _bProgram1;
  BrandProgram* _bProgram2;

  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(S8BrandTest);
}
