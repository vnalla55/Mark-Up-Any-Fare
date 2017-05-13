#include "test/include/CppUnitHelperMacros.h"
#include <iostream>
#include <vector>
#include "Fares/SpecialRouting.h"
#include "DataModel/FareMarket.h"
#include "Common/DateTime.h"
#include "Common/DateTime.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Fare.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/FareInfo.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxType.h"
#include "DBAccess/TariffCrossRefInfo.h"
#include "DataModel/PricingTrx.h"
#include "Fares/RoutingController.h"
#include "Routing/RoutingInfo.h"
#include "Routing/RtgKey.h"
#include "Routing/RoutingConsts.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/MockDataManager.h"

namespace tse
{
class MockSpecialRouting : public SpecialRouting
{
public:
  MockSpecialRouting() {}
  virtual ~MockSpecialRouting() {}

private:
  void validateMileage(PricingTrx& trx, PaxTypeFare& pFare, const TravelRoute& tvlRoue) const
  {
    Fare* fare = pFare.fare()->clone(trx.dataHandle());
    FareInfo* fInfo = (FareInfo*)pFare.fare()->fareInfo()->clone(trx.dataHandle());
    fInfo->_routingNumber = MILEAGE_ROUTING;
    fare->setFareInfo(fInfo);
    pFare.setFare(fare);

    pFare.setIsRouting(false);
    pFare.setRoutingProcessed(true);
    pFare.setRoutingValid(true);
    pFare.mileageSurchargePctg() = 5;
    pFare.mileageSurchargeAmt() = 100;
  }
};

class SpecificTestConfigInitializer : public TestConfigInitializer
{
public:
  SpecificTestConfigInitializer()
  {
    DiskCache::initialize(_config);
    _memHandle.create<MockDataManager>();
  }

  ~SpecificTestConfigInitializer() { _memHandle.clear(); }

private:
  TestMemHandle _memHandle;
};

class SpecialRoutingTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SpecialRoutingTest);
  CPPUNIT_TEST(testProcess1);
  CPPUNIT_TEST(testProcess2);
  CPPUNIT_TEST(testCat25DomesticwithNoValidFare);
  CPPUNIT_TEST(testCat25IntlwithNoValidFare);
  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;

public:
  void setUp() { _memHandle.create<SpecificTestConfigInitializer>(); }

  void tearDown() { _memHandle.clear(); }

  void testProcess1()
  {
    TravelRoute tvlRoute;
    Fare fare1;
    FareInfo fareInfo1;
    TariffCrossRefInfo tcrInfo;

    PricingTrx trx;
    PaxType paxType;
    PaxTypeFare paxTypeFare1;
    PaxTypeFare paxTypeFare2;
    PaxTypeFare paxTypeFare3;

    fareInfo1._vendor = "ATP";
    fareInfo1._carrier = "BA";
    fareInfo1._routingNumber = "2000";

    // fare.initialize(Fare::FS_Domestic,Domestic,&fareInfo,&tcrInfo);
    fare1.setFareInfo(&fareInfo1);
    fare1.setTariffCrossRefInfo(&tcrInfo);
    // Create the FareMarket
    FareMarket fareMarket;
    paxTypeFare1.initialize(&fare1, &paxType, &fareMarket, trx);
    paxTypeFare2.initialize(&fare1, &paxType, &fareMarket, trx);
    paxTypeFare3.initialize(&fare1, &paxType, &fareMarket, trx);

    paxTypeFare1.setRoutingProcessed(false);
    paxTypeFare1.setRoutingValid(false);
    // paxTypeFare1.setIsMileage(true);
    paxTypeFare1.setIsRouting(false);

    paxTypeFare2.setRoutingProcessed(false);
    paxTypeFare2.setRoutingValid(false);
    // paxTypeFare2.setIsMileage(true);
    paxTypeFare2.setIsRouting(false);

    std::vector<PaxTypeFare*> fbrFares;

    fbrFares.push_back(&paxTypeFare1);
    fbrFares.push_back(&paxTypeFare2);
    fbrFares.push_back(&paxTypeFare3);

    // Create The RoutingInfo

    RtgKey rKey1, rKey2;

    rKey1.vendor() = "ATP";
    rKey1.carrier() = "BA";
    rKey1.routingNumber() = "2000";

    rKey2.vendor() = "ATP";
    rKey2.carrier() = "BA";
    rKey2.routingNumber() = "2000";

    RoutingInfo rInfo1, rInfo2;
    MileageInfo mInfo1;
    mInfo1.valid() = false;
    MapInfo mapInfo1;
    mapInfo1.valid() = true;
    rInfo1.mapInfo() = &mapInfo1;
    rInfo1.routingStatus() = false;

    rInfo2.mapInfo() = &mapInfo1;
    rInfo2.routingStatus() = true;
    RoutingInfos routingInfos;

    routingInfos.insert(RoutingInfos::value_type(rKey1, &rInfo1));
    routingInfos.insert(RoutingInfos::value_type(rKey2, &rInfo2));

    if (routingInfos.size() == 2)
    {
      SpecialRouting validator;
      CPPUNIT_ASSERT(validator.matchKey(trx, paxTypeFare1, rKey2));
      validator.validate(trx, fbrFares, routingInfos, tvlRoute);

      CPPUNIT_ASSERT(paxTypeFare1.isRoutingProcessed() == true);
      CPPUNIT_ASSERT(paxTypeFare2.isRoutingProcessed() == true);
      CPPUNIT_ASSERT(paxTypeFare3.isRoutingProcessed() == true);

      CPPUNIT_ASSERT(paxTypeFare1.isRouting() == true);
      CPPUNIT_ASSERT(paxTypeFare2.isRouting() == true);
      CPPUNIT_ASSERT(paxTypeFare3.isRouting() == true);

      CPPUNIT_ASSERT(paxTypeFare1.routingNumber() == "2000");
      CPPUNIT_ASSERT(paxTypeFare2.routingNumber() == "2000");

      CPPUNIT_ASSERT(paxTypeFare3.routingNumber() == "2000");
    }
  }

  void testProcess2()
  {
    TravelRoute tvlRoute;
    Fare fare1;
    FareInfo fareInfo1;
    TariffCrossRefInfo tcrInfo;

    PricingTrx trx;
    PaxType paxType;
    PaxTypeFare paxTypeFare1;
    PaxTypeFare paxTypeFare2;
    PaxTypeFare paxTypeFare3;

    fareInfo1._vendor = "ATP";
    fareInfo1._carrier = "BA";

    fareInfo1._routingNumber = "SEVN";

    // fare.initialize(Fare::FS_Domestic,Domestic,&fareInfo,&tcrInfo);
    fare1.setFareInfo(&fareInfo1);
    fare1.setTariffCrossRefInfo(&tcrInfo);
    // Create the FareMarket
    //
    FareMarket fareMarket;

    paxTypeFare1.initialize(&fare1, &paxType, &fareMarket, trx);
    paxTypeFare2.initialize(&fare1, &paxType, &fareMarket, trx);
    paxTypeFare3.initialize(&fare1, &paxType, &fareMarket, trx);

    paxTypeFare1.setRoutingProcessed(false);
    paxTypeFare1.setRoutingValid(false);

    paxTypeFare2.setRoutingProcessed(false);
    paxTypeFare2.setRoutingValid(false);

    std::vector<PaxTypeFare*> fbrFares;

    fbrFares.push_back(&paxTypeFare1);
    fbrFares.push_back(&paxTypeFare2);
    fbrFares.push_back(&paxTypeFare3);

    // Create The RoutingInfo

    RtgKey rKey1, rKey2;

    rKey1.vendor() = "ATP";
    rKey1.carrier() = "BA";

    rKey2.vendor() = "ATP";
    rKey2.carrier() = "BA";

    RoutingInfo rInfo1, rInfo2;
    MileageInfo mInfo1, mInfo2;

    mInfo1.valid() = true;
    mInfo2.valid() = true;

    rInfo1.mileageInfo() = &mInfo1;
    rInfo2.mileageInfo() = &mInfo2;

    rInfo1.routingStatus() = true;

    rInfo2.routingStatus() = true;

    RoutingInfos routingInfos;

    routingInfos.insert(RoutingInfos::value_type(rKey1, &rInfo1));
    routingInfos.insert(RoutingInfos::value_type(rKey2, &rInfo2));

    if (routingInfos.size() == 2)
    {
      SpecialRouting validator;
      validator.validate(trx, fbrFares, routingInfos, tvlRoute);

      CPPUNIT_ASSERT(paxTypeFare1.isRoutingProcessed() == true);
      CPPUNIT_ASSERT(paxTypeFare2.isRoutingProcessed() == true);
      CPPUNIT_ASSERT(paxTypeFare3.isRoutingProcessed() == true);

      CPPUNIT_ASSERT(paxTypeFare1.isRoutingValid() == true);
      CPPUNIT_ASSERT(paxTypeFare2.isRoutingValid() == true);
      CPPUNIT_ASSERT(paxTypeFare3.isRoutingValid() == true);
      CPPUNIT_ASSERT(paxTypeFare1.fare()->isRouting() == false);
      CPPUNIT_ASSERT(paxTypeFare2.fare()->isRouting() == false);
      CPPUNIT_ASSERT(paxTypeFare3.fare()->isRouting() == false);
    }
  }

  void testCat25DomesticwithNoValidFare()
  {
    TravelRoute tvlRoute;
    Fare fare1;
    FareInfo fareInfo1;
    TariffCrossRefInfo tcrInfo;

    PricingTrx trx;
    PaxType paxType;
    PaxTypeFare paxTypeFare1;
    PaxTypeFare paxTypeFare2;
    PaxTypeFare paxTypeFare3;

    fareInfo1._vendor = "ATP";
    fareInfo1._carrier = "BA";

    fareInfo1._routingNumber = "EIGH";

    // fare.initialize(Fare::FS_Domestic,Domestic,&fareInfo,&tcrInfo);
    fare1.setFareInfo(&fareInfo1);
    fare1.setTariffCrossRefInfo(&tcrInfo);
    // Create the FareMarket
    //
    FareMarket fareMarket;

    paxTypeFare1.initialize(&fare1, &paxType, &fareMarket, trx);
    paxTypeFare2.initialize(&fare1, &paxType, &fareMarket, trx);
    paxTypeFare3.initialize(&fare1, &paxType, &fareMarket, trx);

    paxTypeFare1.setRoutingProcessed(false);
    paxTypeFare1.setRoutingValid(false);

    paxTypeFare2.setRoutingProcessed(false);
    paxTypeFare2.setRoutingValid(false);

    std::vector<PaxTypeFare*> fbrFares;

    fbrFares.push_back(&paxTypeFare1);
    fbrFares.push_back(&paxTypeFare2);
    fbrFares.push_back(&paxTypeFare3);

    // Create The RoutingInfo

    RtgKey rKey1, rKey2;

    rKey1.vendor() = "ATP";
    rKey1.carrier() = "BA";

    rKey2.vendor() = "SITA";
    rKey2.carrier() = "BA";

    RoutingInfo rInfo1, rInfo2;
    MapInfo mapInfo1;
    mapInfo1.valid() = true;
    rInfo1.mapInfo() = &mapInfo1;

    rInfo1.routingStatus() = true;

    MapInfo mapInfo2;
    mapInfo2.valid() = true;
    rInfo2.mapInfo() = &mapInfo1;
    rInfo2.routingStatus() = true;

    RoutingInfos routingInfos;

    routingInfos.insert(RoutingInfos::value_type(rKey1, &rInfo1));
    routingInfos.insert(RoutingInfos::value_type(rKey2, &rInfo2));

    CPPUNIT_ASSERT(routingInfos.size() == 2);

    SpecialRouting validator;

    validator.validate(trx, fbrFares, routingInfos, tvlRoute);

    CPPUNIT_ASSERT(paxTypeFare1.isRoutingProcessed() == true);
    CPPUNIT_ASSERT(paxTypeFare2.isRoutingProcessed() == true);
    CPPUNIT_ASSERT(paxTypeFare3.isRoutingProcessed() == true);

    CPPUNIT_ASSERT(paxTypeFare1.isRoutingValid() == true);
    CPPUNIT_ASSERT(paxTypeFare2.isRoutingValid() == true);
    CPPUNIT_ASSERT(paxTypeFare3.isRoutingValid() == true);

    CPPUNIT_ASSERT(paxTypeFare1.isRouting() == true);
    CPPUNIT_ASSERT(paxTypeFare2.isRouting() == true);
    CPPUNIT_ASSERT(paxTypeFare3.isRouting() == true);
  }

  void testCat25IntlwithNoValidFare()
  {
    TravelRoute tvlRoute;
    Fare fare1;
    FareInfo fareInfo1;
    TariffCrossRefInfo tcrInfo;

    PricingTrx trx;
    PaxType paxType;
    PaxTypeFare paxTypeFare1;
    PaxTypeFare paxTypeFare2;
    PaxTypeFare paxTypeFare3;

    fareInfo1._vendor = "ATP";
    fareInfo1._carrier = "BA";

    fareInfo1._routingNumber = CAT25_INTERNATIONAL;

    // fare.initialize(Fare::FS_Domestic,Domestic,&fareInfo,&tcrInfo);
    fare1.setFareInfo(&fareInfo1);
    fare1.setTariffCrossRefInfo(&tcrInfo);
    // Create the FareMarket
    //
    FareMarket fareMarket;

    paxTypeFare1.initialize(&fare1, &paxType, &fareMarket, trx);

    paxTypeFare1.setRoutingProcessed(false);
    paxTypeFare1.setRoutingValid(false);

    std::vector<PaxTypeFare*> fbrFares;

    fbrFares.push_back(&paxTypeFare1);

    // Create The RoutingInfo

    RtgKey rKey1, rKey2;

    rKey1.vendor() = "ATP";
    rKey1.carrier() = "BA";

    rKey2.vendor() = "SITA";
    rKey2.carrier() = "BA";

    RoutingInfo rInfo1, rInfo2;
    MapInfo mapInfo1;
    mapInfo1.valid() = false;
    rInfo1.mapInfo() = &mapInfo1;

    rInfo1.routingStatus() = false;

    MapInfo mapInfo2;
    mapInfo2.valid() = false;
    rInfo2.mapInfo() = &mapInfo1;
    rInfo2.routingStatus() = true;

    RoutingInfos routingInfos;

    routingInfos.insert(RoutingInfos::value_type(rKey1, &rInfo1));
    routingInfos.insert(RoutingInfos::value_type(rKey2, &rInfo2));

    CPPUNIT_ASSERT(routingInfos.size() == 2);

    MockSpecialRouting validator;

    validator.validate(trx, fbrFares, routingInfos, tvlRoute);

    CPPUNIT_ASSERT(paxTypeFare1.isRoutingProcessed() == true);

    CPPUNIT_ASSERT(paxTypeFare1.isRoutingValid() == true);

    // CPPUNIT_ASSERT(paxTypeFare1.isMileage()==true);
    CPPUNIT_ASSERT(paxTypeFare1.mileageSurchargePctg() == 5);
    CPPUNIT_ASSERT(paxTypeFare1.mileageSurchargeAmt() == 100);
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(SpecialRoutingTest);
}
