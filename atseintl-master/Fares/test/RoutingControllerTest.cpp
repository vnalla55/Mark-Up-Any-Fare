#include "test/include/CppUnitHelperMacros.h"
#include <iostream>
#include <log4cxx/helpers/objectptr.h>
#include <map>
#include <set>

#include "Common/Config/ConfigMan.h"
#include "Common/DateTime.h"
#include "Common/Global.h"
#include "Common/RoutingUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/Vendor.h"
#include "DBAccess/AirlineAllianceCarrierInfo.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/FltTrkCntryGrp.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Mileage.h"
#include "DBAccess/MultiTransport.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/Routing.h"
#include "DBAccess/RoutingMap.h"
#include "DBAccess/RoutingRestriction.h"
#include "DBAccess/TariffCrossRefInfo.h"
#include "DBAccess/TpdPsr.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Diversity.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/PricingOptions.h"
#include "Fares/RoutingController.h"
#include "Routing/MapNode.h"
#include "Routing/MileageInfo.h"
#include "Routing/RoutingConsts.h"
#include "Routing/RoutingInfo.h"
#include "Routing/RtgKey.h"
#include "Routing/TravelRoute.h"
#include "Routing/TravelRouteBuilder.h"
#include "Rules/RuleConst.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/MockGlobal.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestCarrierPreferenceFactory.h"
#include "test/testdata/TestContainerFactory.h"
#include "test/testdata/TestFareMarketFactory.h"
#include "test/testdata/TestPricingTrxFactory.h"
#include "test/testdata/TestRoutingFactory.h"
#include "test/include/TestFallbackUtil.h"

namespace tse
{
FIXEDFALLBACKVALUE_DECL(fallbackMileageSurchargeExceptionValidation);

namespace
{
class RoutingControllerMock : public RoutingController
{
public:
  RoutingControllerMock(PricingTrx& trx) : RoutingController(trx) {};
  virtual ~RoutingControllerMock() {};

protected:
  const Routing* getRoutingData(PricingTrx& trx, PaxTypeFare& paxTypeFare) const { return 0; }
};

class RoutingControllerProcessPaxTypeFareMock : public RoutingController
{
public:
  RoutingControllerProcessPaxTypeFareMock(PricingTrx& trx)
    : RoutingController(trx), _passedTravelRoute(0)
  {
  }
  virtual ~RoutingControllerProcessPaxTypeFareMock() {}

protected:
  bool processRoutingValidation(RtgKey& rKey,
                                PricingTrx& trx,
                                TravelRoute& tvlRoute,
                                PaxTypeFare* paxTypeFare,
                                const Routing* routing,
                                const Routing* origAddOnRouting,
                                const Routing* destAddOnRouting,
                                RoutingInfos& routingInfos,
                                const DateTime& travelDate)
  {
    _passedTravelRoute = &tvlRoute;
    return true;
  }

  void getRoutings(PricingTrx& trx,
                   PaxTypeFare& paxTypeFare,
                   const Routing*& routing,
                   const Routing*& origAddOnRouting,
                   const Routing*& destAddOnRouting)
  {
    routing = &_routing;
  }

public:
  Routing _routing;
  TravelRoute* _passedTravelRoute;
};

class MockRoutingController : public RoutingController
{
public:
  MockRoutingController(PricingTrx& trx) : RoutingController(trx) {};
  virtual ~MockRoutingController() {};

  const Routing* getRoutingData(RtgKey&, PricingTrx& trx)
  {
    Routing* routing = new Routing();

    RoutingRestriction* res1 = new RoutingRestriction();
    RoutingRestriction* res2 = new RoutingRestriction();
    RoutingMap map1, map2;

    res1->restriction() = "3";

    routing->rests().push_back(res1);

    res2->restriction() = "12";

    routing->rests().push_back(res2);
    routing->rmaps().push_back(&map1);
    routing->rmaps().push_back(&map2);
    return routing;
  }

  bool validateMileage(TravelRoute& tvlRoute) { return true; }

  bool validateMileage(PricingTrx& trx,
                       PaxTypeFare& paxTypeFare,
                       const TravelRoute& tvlRoute,
                       MileageInfo& mileageInfo,
                       bool fOutboun)
  {
    return true;
  }

  bool validateRestrictions(PricingTrx& trx, const RoutingRestriction& rests, TravelRoute& tvlRoute)
  {
    return true;
  }

  bool validateRoutingMaps(const Routing* routing,
                           TravelRoute& tvlRoute,
                           PricingTrx& trx,
                           const RtgKeyMap&)
  {
    return true;
  }
};

class RoutingControllerTestSetRtMapValidFlag : public RoutingController
{
public:
  RoutingControllerTestSetRtMapValidFlag(PricingTrx& trx) : RoutingController(trx) {};

  void updatePaxTypeFareAccess(RtgKey& rKey, PaxTypeFare& paxTypeFare, RoutingInfos& routingInfos)
  {
    updatePaxTypeFare(rKey, paxTypeFare, routingInfos);
  }

  bool processRoutingCheckMissingData(RtgKey& rKey,
                                      RoutingInfo* routingInfo,
                                      const Routing* routing,
                                      const Routing* origAddOnRouting,
                                      const Routing* destAddOnRouting)
  {
    return true;
  }

  void processRoutingBreakTravel(RtgKey& rKey,
                                 PricingTrx& trx,
                                 TravelRoute& tvlRoute,
                                 RoutingInfo* routingInfo,
                                 TravelRoute& baseTvlRoute,
                                 TravelRoute& origAddOnTvlRoute,
                                 TravelRoute& destAddOnTvlRoute,
                                 const Routing* origAddOnRouting,
                                 const Routing* destAddOnRouting)
  {
  }

  void processRoutingProcessRestrictions(RtgKey& rKey,
                                         PricingTrx& trx,
                                         TravelRoute& tvlRoute,
                                         PaxTypeFare* paxTypeFare,
                                         RoutingInfo* routingInfo,
                                         TravelRoute& origAddOnTvlRoute,
                                         TravelRoute& destAddOnTvlRoute,
                                         const Routing* routing,
                                         const Routing* origAddOnRouting,
                                         const Routing* destAddOnRouting,
                                         bool& isRoutingValid)
  {
    isRoutingValid = true;
  }

  void processRoutingMileageRouting(RtgKey& rKey,
                                    PricingTrx& trx,
                                    TravelRoute& tvlRoute,
                                    PaxTypeFare* paxTypeFare,
                                    RoutingInfo* routingInfo,
                                    bool& isRoutingValid,
                                    bool& isMileageValid)
  {
    isMileageValid = false;
  }

  void processRoutingRoutMapValidation(RtgKey& rKey,
                                       PricingTrx& trx,
                                       TravelRoute& tvlRoute,
                                       PaxTypeFare* paxTypeFare,
                                       RoutingInfo* routingInfo,
                                       TravelRoute& baseTvlRoute,
                                       TravelRoute& origAddOnTvlRoute,
                                       TravelRoute& destAddOnTvlRoute,
                                       const Routing* routing,
                                       const Routing* origAddOnRouting,
                                       const Routing* destAddOnRouting,
                                       const DateTime& travelDate,
                                       bool& isRoutingValid)
  {
    isRoutingValid = true;
  }
};

class FareInfoMock : public FareInfo
{
public:
  FareInfoMock() { directionality() = FROM; }
};

class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;
  Mileage* getMil(int mil, Indicator mType)
  {
    Mileage* ret = _memHandle.create<Mileage>();
    ret->mileageType() = mType;
    ret->mileage() = mil;
    return ret;
  }
  MultiTransport* getMultiTr(const LocCode& city, LocCode loc)
  {
    MultiTransport* ret = _memHandle.create<MultiTransport>();
    ret->multitransLoc() = loc;
    ret->multitranscity() = city;
    ret->intlAppl() = 'Y';
    ret->domAppl() = 'Y';
    return ret;
  }
  TpdPsr* getTpdPsr(LocTypeCode lk1, LocCode lc1, LocTypeCode lk2, LocCode lc2)
  {
    TpdPsr* ret = _memHandle.create<TpdPsr>();
    ret->seqNo() = 1;
    ret->globalDir() = GlobalDirection::AT;
    ret->psrHip() = 'Y';
    ret->loc1().locType() = lk1;
    ret->loc1().loc() = lc1;
    ret->loc2().locType() = lk2;
    ret->loc2().loc() = lc2;
    ret->thisCarrierRestr() = 'N';
    ret->thruViaLocRestr() = 'N';
    ret->thruViaMktSameCxr() = 'N';
    ret->thruMktCarrierExcept() = 'N';
    ret->tpdThruViaMktOnlyInd() = 'N';
    return ret;
  }

public:
  const std::vector<Routing*>& getRouting(const VendorCode& vendor,
                                          const CarrierCode& carrier,
                                          const TariffNumber& routingTariff,
                                          const RoutingNumber& routingNumber,
                                          const DateTime& date)
  {
    if ((vendor == "ATP" && carrier == "BA" && routingTariff == 4 && routingNumber == "0000") ||
        (vendor == "ATP" && carrier == "BA" && routingTariff == 4 && routingNumber == "9576") ||
        (vendor == "ATP" && carrier == "BA" && routingTariff == 4 && routingNumber == "9910") ||
        (vendor == "ATP" && carrier == "BA" && routingTariff == 4 && routingNumber == "1000") ||
        (vendor == "ATP" && carrier == "BA" && routingTariff == 4 && routingNumber == "9041"))
      return *TestVectorFactory<TestRoutingFactory, Routing>::create(
                 "/vobs/atseintl/Fares/test/data/routingcontroller/Routing_ATP_BA_4_0000.xml");
    else if (routingTariff == -1)
      return *_memHandle.create<std::vector<Routing*> >();
    else if (vendor == "SITA" && carrier == "BA")
      return *_memHandle.create<std::vector<Routing*> >();
    else if (vendor == "ATP" && carrier == "AA" && routingTariff == 0)
      return *_memHandle.create<std::vector<Routing*> >();
    else if (vendor == "ATP" && carrier == "AA" && routingTariff == 99)
    {
      if(!_emptyRouting.empty() && (_emptyRouting == routingNumber || _emptyRouting == "BOTH"))
      {
        return *_memHandle.create<std::vector<Routing*> >();
      } else if(!_failRouting.empty() && (_failRouting == routingNumber || _failRouting == "BOTH"))
      {
        std::vector<Routing*>* rtgV = _memHandle.create<std::vector<Routing*> >();
        Routing* routing = _memHandle.create<Routing>();
        rtgV->push_back(routing);
        routing->rmaps().push_back(new RoutingMap());
        return *rtgV;
      } else
        return *TestVectorFactory<TestRoutingFactory, Routing>::create(
                 "/vobs/atseintl/Fares/test/data/routingcontroller/Routing_ATP_AA_99_0002.xml");
    } else if (vendor == "SITA" && carrier == "AA" && routingTariff == 0)
      return *_memHandle.create<std::vector<Routing*> >();
    else if (vendor == "SITA" && carrier == "AA" && routingTariff == 3)
      return *_memHandle.create<std::vector<Routing*> >();
    else if (vendor == "ATP" && carrier == "AA" && routingTariff == 3 && routingNumber == "0000")
      return *_memHandle.create<std::vector<Routing*> >();

    return DataHandleMock::getRouting(vendor, carrier, routingTariff, routingNumber, date);
  }
  const Mileage* getMileage(const LocCode& origin,
                            const LocCode& dest,
                            Indicator mileageType,
                            const GlobalDirection globalDir,
                            const DateTime& date)
  {
    if (origin == "LHR" && dest == "BOM" && mileageType == 'M')
      return getMil(5660, 'M');
    else if (origin == "LHR" && dest == "BOM" && mileageType == 'T')
      return getMil(4469, 'T');
    else if (origin == "RIO" && dest == "NYC" && mileageType == 'T')
      return getMil(4816, 'T');
    else if (origin == "NYC" && dest == "LON" && mileageType == 'T')
      return getMil(3458, 'T');
    else if (origin == "RIO" && dest == "LON" && mileageType == 'M')
      return getMil(6920, 'M');

    return DataHandleMock::getMileage(origin, dest, mileageType, globalDir, date);
  }
  const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                          const CarrierCode& carrierCode,
                                          GeoTravelType tvlType,
                                          const DateTime& tvlDate)
  {
    if (locCode == "RIO")
      return "RIO";
    else if (locCode == "NYC")
      return "NYC";
    else if (locCode == "SEA")
      return "SEA";
    else if (locCode == "SDQ")
      return "SDQ";
    else if (locCode == "SCL")
      return "SCL";
    else if (locCode == "LSC")
      return "LSC";
    else if (locCode == "CUN")
      return "CUN";
    else if (locCode == "MIA")
      return "MIA";
    else if (locCode == "LAS")
      return "LAS";
    else if (locCode == "EYW")
      return "EYW";

    return DataHandleMock::getMultiTransportCityCode(locCode, carrierCode, tvlType, tvlDate);
  }
  const std::vector<MultiTransport*>& getMultiTransportCity(const LocCode& locCode,
                                                            const CarrierCode& carrierCode,
                                                            GeoTravelType tvlType,
                                                            const DateTime& tvlDate)
  {
    if (locCode == "NYC")
    {
      std::vector<MultiTransport*>* ret = _memHandle.create<std::vector<MultiTransport*> >();
      ret->push_back(getMultiTr(locCode, "EWR"));
      ret->push_back(getMultiTr(locCode, "JFK"));
      ret->push_back(getMultiTr(locCode, "JRB"));
      ret->push_back(getMultiTr(locCode, "LGA"));
      ret->push_back(getMultiTr(locCode, "TSS"));
      return *ret;
    }
    else if (locCode == "LON")
    {
      std::vector<MultiTransport*>* ret = _memHandle.create<std::vector<MultiTransport*> >();
      ret->push_back(getMultiTr(locCode, "LCY"));
      ret->push_back(getMultiTr(locCode, "LGW"));
      ret->push_back(getMultiTr(locCode, "LHR"));
      ret->push_back(getMultiTr(locCode, "LTN"));
      ret->push_back(getMultiTr(locCode, "QQP"));
      ret->push_back(getMultiTr(locCode, "QQS"));
      ret->push_back(getMultiTr(locCode, "QQU"));
      ret->push_back(getMultiTr(locCode, "QQW"));
      ret->push_back(getMultiTr(locCode, "STN"));
      ret->push_back(getMultiTr(locCode, "XQE"));
      return *ret;
    }
    else if (locCode == "RIO")
    {
      std::vector<MultiTransport*>* ret = _memHandle.create<std::vector<MultiTransport*> >();
      ret->push_back(getMultiTr(locCode, "GIG"));
      ret->push_back(getMultiTr(locCode, "SDU"));
      return *ret;
    }
    return DataHandleMock::getMultiTransportCity(locCode, carrierCode, tvlType, tvlDate);
  }
  const std::vector<TpdPsr*>& getTpdPsr(Indicator applInd,
                                        const CarrierCode& carrier,
                                        Indicator area1,
                                        Indicator area2,
                                        const DateTime& date)
  {
    if (applInd == 'P' && carrier == "AA")
    {
      std::vector<TpdPsr*>* ret = _memHandle.create<std::vector<TpdPsr*> >();
      ret->push_back(getTpdPsr('C', "FRA", 'C', "BUE"));
      ret->push_back(getTpdPsr('C', "BUE", 'C', "PAR"));
      ret->push_back(getTpdPsr('C', "BUE", 'C', "MAN"));
      ret->push_back(getTpdPsr('C', "BUE", 'C', "BHX"));
      return *ret;
    }
    else if (applInd == 'T' && carrier == "AA")
    {
      std::vector<TpdPsr*>* ret = _memHandle.create<std::vector<TpdPsr*> >();
      ret->push_back(getTpdPsr('C', "CHI", 'C', "SNN"));
      ret->push_back(getTpdPsr('C', "DTT", 'C', "DUB"));
      ret->push_back(getTpdPsr('C', "FOR", 'C', "BRU"));
      ret->push_back(getTpdPsr('C', "FOR", 'C', "FRA"));
      ret->push_back(getTpdPsr('C', "SCL", 'C', "PAR"));
      ret->push_back(getTpdPsr('C', "SCL", 'C', "LON"));
      ret->push_back(getTpdPsr('C', "LIM", 'C', "PAR"));
      ret->push_back(getTpdPsr('C', "LIM", 'C', "LON"));
      ret->push_back(getTpdPsr('S', "USIN", 'C', "SNN"));
      ret->push_back(getTpdPsr('S', "USMI", 'C', "SNN"));
      return *ret;
    }
    return DataHandleMock::getTpdPsr(applInd, carrier, area1, area2, date);
  }
  const std::vector<TPMExclusion*>& getTPMExclus(const CarrierCode& carrier)
  {
    if (carrier == "AA")
      return *_memHandle.create<std::vector<TPMExclusion*> >();
    return DataHandleMock::getTPMExclus(carrier);
  }
  const FltTrkCntryGrp* getFltTrkCntryGrp(const CarrierCode& carrier, const DateTime& date)
  {
    if (carrier == "AA")
    {
      FltTrkCntryGrp* ret = _memHandle.create<FltTrkCntryGrp>();
      ret->flttrkApplInd() = 'B';
      ret->nations().push_back("US");
      ret->nations().push_back("CA");
      return ret;
    }
    else if (carrier == "LA")
      return 0;
    return DataHandleMock::getFltTrkCntryGrp(carrier, date);
  }

  const std::vector<AirlineAllianceCarrierInfo*>&
  getAirlineAllianceCarrier(const CarrierCode& carrierCode)
  {
    if(carrierCode == "AA")
      return *_memHandle.create<std::vector<AirlineAllianceCarrierInfo*> >();
    if(carrierCode == "C1")
    {
      std::vector<AirlineAllianceCarrierInfo*>* vec = _memHandle.create<std::vector<AirlineAllianceCarrierInfo*> >();
      AirlineAllianceCarrierInfo* al = _memHandle.create<AirlineAllianceCarrierInfo>();
      al->carrier() = carrierCode;
      vec->push_back(al);
      return *vec;
    }
    return DataHandleMock::getAirlineAllianceCarrier(carrierCode);
  }

  RoutingNumber _emptyRouting;
  RoutingNumber _failRouting;
};
}

class RoutingControllerTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RoutingControllerTest);

  CPPUNIT_TEST(testProcess);

  CPPUNIT_TEST(testProcessWhenPass);
  CPPUNIT_TEST(testProcessWhenRtwGenericAndSpecifiedPass);
  CPPUNIT_TEST(testProcessWhenRtwGenericPassAndNoSpecified);
  CPPUNIT_TEST(testProcessWhenRtwGenericPassAndSpecifiedFail);
  CPPUNIT_TEST(testProcessWhenRtwNoGenericAndSpecifiedPass);
  CPPUNIT_TEST(testProcessWhenRtwNoGenericAndSpecifiedFail);
  CPPUNIT_TEST(testProcessWhenRtwNoGenericAndNoSpecified);
  CPPUNIT_TEST(testProcessWhenRtwGenericFailAndSpecifiedPass);
  CPPUNIT_TEST(testProcessWhenRtwGenericFailAndNoSpecified);
  CPPUNIT_TEST(testProcessWhenRtwGenericFailAndSpecifiedFail);
  CPPUNIT_TEST(testProcessRoutingValidation);
  CPPUNIT_TEST(testValidateRestrictions);

  CPPUNIT_TEST(testMileageRoutingRTW);

  CPPUNIT_TEST(testLocMatchesOrigin);
  CPPUNIT_TEST(testLocMatchesDestination);
  CPPUNIT_TEST(testLocMatchesDestinationB);
  CPPUNIT_TEST(testBuildTravelSegs);
  CPPUNIT_TEST(testBuildTravelSegsB);
  CPPUNIT_TEST(testLocMatchesTvlSeg);
  CPPUNIT_TEST(testBuildTravelSegsC);
  CPPUNIT_TEST(testSurchargeException);
  CPPUNIT_TEST(testUpdateFareSurcharge);
  CPPUNIT_TEST(testValidateMileage);
  CPPUNIT_TEST(testValidateMileage_Diagnostic452);
  CPPUNIT_TEST(validateRoutingMaps_Pass);
  CPPUNIT_TEST(validateRoutingMaps_FailZone);
  CPPUNIT_TEST(validateRoutingMaps_FailNation);
  CPPUNIT_TEST(testOriginAddonRTGSpecifiedRTGDestinationMileage);
  CPPUNIT_TEST(testSetRtMapValidFlag);

  CPPUNIT_TEST(testGetRoutingsForSITAMileageOrigAddon);
  CPPUNIT_TEST(testGetRoutingsForSITAMileageDestAddon);
  CPPUNIT_TEST(testGetRoutingsNoApplicationForATPCOMileageAddon);

  CPPUNIT_TEST(testProcessRestrictions_matchKeyLogic);
  CPPUNIT_TEST(testProcessRestrictions_matchKeyLogicWithFlip);

  CPPUNIT_TEST(testisRestSeqUseAndLogic_AllNeg);
  CPPUNIT_TEST(testisRestSeqUseAndLogic_Mixed);
  CPPUNIT_TEST(testisRestSeqUseAndLogic_AllPositive);

  CPPUNIT_TEST(testProcessPaxTypeFare_EntryExitPtsTerminalPointsOnly);
  CPPUNIT_TEST(testProcessPaxTypeFare_EntryExitPtsAnyPoints);
  CPPUNIT_TEST(testProcessPaxTypeFare_EntryExitPtsCrxPref);

  CPPUNIT_TEST(testProcessPaxTypeFare_TktPtsTicketingOnly);
  CPPUNIT_TEST(testProcessPaxTypeFare_TktPtsAny);
  CPPUNIT_TEST(testProcessPaxTypeFare_TktPtsIgnoreInd);

  CPPUNIT_TEST(testProcessPaxTypeFareWhenUseTvlRouteWithoutHiddenPts);

  CPPUNIT_TEST(testValidateMileageSurchargeExceptionOldImplementation);
  CPPUNIT_TEST(testValidateMileageSurchargeExceptionNewImplementation);

  CPPUNIT_TEST(testValidateRestriction16_RtwPass);
  CPPUNIT_TEST(testValidateRestriction17_Pass);
  CPPUNIT_TEST(testValidateRestriction17_Fail);
  CPPUNIT_TEST(testValidateRestriction17_PassSurface);
  CPPUNIT_TEST(testValidateRestriction17_PassRtw);

  CPPUNIT_TEST(testRestriction12_noRTW);
  CPPUNIT_TEST(testRestriction12_singleNonStop);
  CPPUNIT_TEST(testRestriction12_multiNonStop);
  CPPUNIT_TEST(testRestriction12_multiRest12);
  CPPUNIT_TEST(testRestriction12_nation);
  CPPUNIT_TEST(testRestriction12_zone);
  CPPUNIT_TEST(testRestriction12_genericCityCode);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _myDataHandle = _memHandle.create<MyDataHandle>();
    TestConfigInitializer::setValue("FULL_MAP_ROUTING_ACTIVATION_DATE", "2013-06-16", "PRICING_SVC", true);
    _trx = _memHandle.create<PricingTrx>();
    _trx->setTravelDate(DateTime::localTime());
    _trx->setOptions(_memHandle.create<PricingOptions>());
    PricingRequest* request = _memHandle.insert(new PricingRequest);
    request->ticketingDT() = DateTime::localTime();
    _trx->setRequest(request);
    _rtC = _memHandle.insert(new RoutingController(*_trx));
    _mockedRtgController = _memHandle.insert(new RoutingControllerMock(*_trx));
    _tvlRoute = _memHandle.create<TravelRoute>();
    _tvlRoute->travelRouteTktOnly() = _memHandle.create<TravelRoute>();
    _paxTypeFare = _memHandle.create<PaxTypeFare>();
    _fareMarket = _memHandle.create<FareMarket>();
    _fare = _memHandle.create<Fare>();
    _constructedFareInfo = _memHandle.create<ConstructedFareInfo>();
    _fareInfo = _memHandle.create<FareInfo>();
    _tariffCrossRef = _memHandle.create<TariffCrossRefInfo>();

    _fare->setFareInfo(_fareInfo);
    _fare->constructedFareInfo() = _constructedFareInfo;
    _fare->setTariffCrossRefInfo(_tariffCrossRef);
    _paxTypeFare->setFare(_fare);

    _fbrItemInfo.fareInd() = FareByRuleItemInfo::SPECIFIED; // FBR specified
    _fbrPaxTypeFareRuleData.ruleItemInfo() = &_fbrItemInfo;
    _paxTypeFare->setRuleData(
        RuleConst::FARE_BY_RULE, _trx->dataHandle(), &_fbrPaxTypeFareRuleData);

    _pptRtgCtrl = _memHandle.insert(new RoutingControllerProcessPaxTypeFareMock(*_trx));

    _diversity = _memHandle.create<Diversity>();
    _shoppingTrx = _memHandle.create<ShoppingTrx>(_diversity);
    _paxTypeFare->fareMarket() = _fareMarket;
    _routingInfos = _memHandle.create<RoutingInfos>();
    _rtgInfo = _memHandle.create<RoutingInfo>();
    _rtgInfo->restrictions() = _memHandle.create<RestrictionInfos>();
  }

  void tearDown()
  {
    _memHandle.clear();
  }

  void testProcess()
  {
    PricingTrx* trx = TestPricingTrxFactory::create(
        "/vobs/atseintl/Fares/test/data/FareCurrencySelection/IntlPrime/trx.xml");
    trx->setOptions(_memHandle.create<PricingOptions>());
    FareMarket* fareMarket = TestFareMarketFactory::create(
        "/vobs/atseintl/Fares/test/data/FareCurrencySelection/IntlPrime/fareMarket.xml");

    std::vector<PaxTypeFare*>::iterator itr = fareMarket->allPaxTypeFare().begin();
    for (; itr != fareMarket->allPaxTypeFare().end(); itr++)
    {
      PaxTypeFare& paxTypeFare = **itr;
      paxTypeFare.setRuleData(
          RuleConst::FARE_BY_RULE, _trx->dataHandle(), &_fbrPaxTypeFareRuleData);
    }

    trx->diagnostic().diagnosticType() = Diagnostic450;
    TravelRoute travelRoute;
    RoutingInfos routingInfos;

    RoutingController rtC(*trx);
    CPPUNIT_ASSERT(rtC.process(*fareMarket, travelRoute, routingInfos));
  }

  void testProcessPaxTypeFare_EntryExitPtsTerminalPointsOnly()
  {
    Routing& routing = static_cast<RoutingControllerProcessPaxTypeFareMock*>(_pptRtgCtrl)->_routing;
    routing.entryExitPointInd() = ENTRYEXITONLY;

    FareMarket fareMarket;
    RoutingInfos routingInfos;
    RtgKeyMap rtMap;

    const Routing* origAddOnRouting = 0;
    const Routing* destAddOnRouting = 0;

    _pptRtgCtrl->processPaxTypeFare(*_paxTypeFare,
                                    *_tvlRoute,
                                    fareMarket,
                                    routingInfos,
                                    rtMap,
                                    &routing,
                                    origAddOnRouting,
                                    destAddOnRouting);

    CPPUNIT_ASSERT_EQUAL(size_t(1), rtMap.size());
    // CPPUNIT_ASSERT_EQUAL(ENTRYEXITONLY, rtMap.begin()->first.entryExitPoint());
  }

  void testProcessPaxTypeFare_EntryExitPtsAnyPoints()
  {
    Routing& routing = static_cast<RoutingControllerProcessPaxTypeFareMock*>(_pptRtgCtrl)->_routing;
    routing.entryExitPointInd() = '1';

    FareMarket fareMarket;
    RoutingInfos routingInfos;
    RtgKeyMap rtMap;

    const Routing* origAddOnRouting = 0;
    const Routing* destAddOnRouting = 0;

    _pptRtgCtrl->processPaxTypeFare(*_paxTypeFare,
                                    *_tvlRoute,
                                    fareMarket,
                                    routingInfos,
                                    rtMap,
                                    &routing,
                                    origAddOnRouting,
                                    destAddOnRouting);

    CPPUNIT_ASSERT_EQUAL(size_t(1), rtMap.size());
    // CPPUNIT_ASSERT_EQUAL('1', rtMap.begin()->first.entryExitPoint());
  }

  void testProcessPaxTypeFare_EntryExitPtsCrxPref()
  {
    Routing& routing = static_cast<RoutingControllerProcessPaxTypeFareMock*>(_pptRtgCtrl)->_routing;
    routing.entryExitPointInd() = GETTERMPTFROMCRXPREF;

    FareMarket fareMarket;
    RoutingInfos routingInfos;
    RtgKeyMap rtMap;

    const Routing* origAddOnRouting = 0;
    const Routing* destAddOnRouting = 0;

    _pptRtgCtrl->processPaxTypeFare(*_paxTypeFare,
                                    *_tvlRoute,
                                    fareMarket,
                                    routingInfos,
                                    rtMap,
                                    &routing,
                                    origAddOnRouting,
                                    destAddOnRouting);

    CPPUNIT_ASSERT_EQUAL(size_t(1), rtMap.size());
    // CPPUNIT_ASSERT_EQUAL(GETTERMPTFROMCRXPREF, rtMap.begin()->first.entryExitPoint());
  }

  void testProcessPaxTypeFare_TktPtsTicketingOnly()
  {
    Routing& routing = static_cast<RoutingControllerProcessPaxTypeFareMock*>(_pptRtgCtrl)->_routing;
    routing.unticketedPointInd() = TKTPTS_TKTONLY;

    FareMarket fareMarket;
    RoutingInfos routingInfos;
    RtgKeyMap rtMap;

    const Routing* origAddOnRouting = 0;
    const Routing* destAddOnRouting = 0;

    _pptRtgCtrl->processPaxTypeFare(*_paxTypeFare,
                                    *_tvlRoute,
                                    fareMarket,
                                    routingInfos,
                                    rtMap,
                                    &routing,
                                    origAddOnRouting,
                                    destAddOnRouting);

    CPPUNIT_ASSERT_EQUAL(size_t(1), rtMap.size());
    // CPPUNIT_ASSERT_EQUAL(TKTPTS_TKTONLY, rtMap.begin()->first.unticketedPoint());
  }

  void testProcessPaxTypeFare_TktPtsAny()
  {
    Routing& routing = static_cast<RoutingControllerProcessPaxTypeFareMock*>(_pptRtgCtrl)->_routing;
    routing.unticketedPointInd() = TKTPTS_ANY;

    FareMarket fareMarket;
    RoutingInfos routingInfos;
    RtgKeyMap rtMap;

    const Routing* origAddOnRouting = 0;
    const Routing* destAddOnRouting = 0;

    _pptRtgCtrl->processPaxTypeFare(*_paxTypeFare,
                                    *_tvlRoute,
                                    fareMarket,
                                    routingInfos,
                                    rtMap,
                                    &routing,
                                    origAddOnRouting,
                                    destAddOnRouting);

    CPPUNIT_ASSERT_EQUAL(size_t(1), rtMap.size());
    // CPPUNIT_ASSERT_EQUAL(TKTPTS_ANY, rtMap.begin()->first.unticketedPoint());
  }

  void testProcessPaxTypeFare_TktPtsIgnoreInd()
  {
    Routing& routing = static_cast<RoutingControllerProcessPaxTypeFareMock*>(_pptRtgCtrl)->_routing;
    routing.unticketedPointInd() = IGNORE_TKTPTSIND;

    FareMarket fareMarket;
    RoutingInfos routingInfos;
    RtgKeyMap rtMap;

    const Routing* origAddOnRouting = 0;
    const Routing* destAddOnRouting = 0;

    _pptRtgCtrl->processPaxTypeFare(*_paxTypeFare,
                                    *_tvlRoute,
                                    fareMarket,
                                    routingInfos,
                                    rtMap,
                                    &routing,
                                    origAddOnRouting,
                                    destAddOnRouting);

    CPPUNIT_ASSERT_EQUAL(size_t(1), rtMap.size());
    // CPPUNIT_ASSERT_EQUAL(IGNORE_TKTPTSIND, rtMap.begin()->first.unticketedPoint());
  }

  void testProcessRoutingValidation()
  {
    Fare fare;
    FareInfo fareInfo;
    TariffCrossRefInfo tcrInfo;
    PaxType paxType;
    PaxTypeFare paxTypeFare1;

    fare.setFareInfo(&fareInfo);
    fare.setTariffCrossRefInfo(&tcrInfo);
    FareMarket fareMarket;

    paxTypeFare1.initialize(&fare, &paxType, &fareMarket, *_trx);

    std::map<RtgKey, bool> rtMap;

    RtgKey rKey;
    std::vector<const Routing*> rtList;

    MockRoutingController rtC(*_trx);

    Routing rtg;
    Routing rtgAddOn1;
    Routing rtgAddOn2;
    RoutingInfos routingInfos;
    const DateTime travelDate = DateTime::localTime();
    TravelRoute tvlRoute;
    createTravelSeg(tvlRoute.mileageTravelRoute(),
                    "DFW", "US", "AA", "STL", "US", DateTime::localTime());

    CPPUNIT_ASSERT(rtC.processRoutingValidation(
        rKey, *_trx, tvlRoute, &paxTypeFare1, &rtg, 0, 0, routingInfos, travelDate));
  }

  void prepareProcess()
  {
    PaxType* paxType = _memHandle.create<PaxType>();

    _fareInfo->_vendor = "ATP";
    _tariffCrossRef->_carrier = "AA";
    _fareInfo->_routingNumber = "0002";
    _tariffCrossRef->_routingTariff1 = 99;
    _tariffCrossRef->_routingTariff2 = 0;

    Loc* locLAS = _memHandle.create<Loc>();
    Loc* locMIA = _memHandle.create<Loc>();
    Loc* locEYW = _memHandle.create<Loc>();

    locLAS->loc() = "LAS";
    locMIA->loc() = "MIA";
    locEYW->loc() = "EYW";

    AirSeg* seg1 = _memHandle.create<AirSeg>();
    seg1->origAirport() = "LAS";
    seg1->destAirport() = "MIA";
    seg1->carrier() = "AA";
    seg1->origin() = locLAS;
    seg1->destination() = locMIA;
    seg1->departureDT() = DateTime::localTime();

    AirSeg* seg2 = _memHandle.create<AirSeg>();
    seg2->origAirport() = "MIA";
    seg2->destAirport() = "EYW";
    seg2->carrier() = "AA";
    seg2->origin() = locMIA;
    seg2->destination() = locEYW;
    seg2->departureDT() = DateTime::localTime();

    _paxTypeFare->initialize(_fare, paxType, _fareMarket, *_trx);

    _fareMarket->geoTravelType() = GeoTravelType::Domestic;
    _fareMarket->travelSeg().push_back(seg1);
    _fareMarket->travelSeg().push_back(seg2);
    _fareMarket->allPaxTypeFare().push_back(_paxTypeFare);
    _fareMarket->setFltTrkIndicator(false);
    _fareMarket->governingCarrier() = "AA";
    GlobalDirection dir;
    strToGlobalDirection(dir, "RW");
    _fareMarket->setGlobalDirection(dir);
    _fareInfo->globalDirection() = dir;

    _trx->travelSeg().push_back(seg1);
    _trx->travelSeg().push_back(seg2);

    _paxTypeFare->setRoutingProcessed(false);
    _paxTypeFare->setCategoryValid(1);
    _paxTypeFare->setCategoryValid(15);
    _paxTypeFare->cat25BasePaxFare() = true;
  }

  void testProcessWhenPass()
  {
    prepareProcess();

    CPPUNIT_ASSERT(!_paxTypeFare->isRoutingProcessed());
    CPPUNIT_ASSERT(_rtC->process(*_fareMarket, *_tvlRoute, *_routingInfos));
    CPPUNIT_ASSERT(_paxTypeFare->isRoutingProcessed());
    CPPUNIT_ASSERT(_paxTypeFare->isRoutingValid());
  }

  void testProcessWhenRtwGenericAndSpecifiedPass()
  {
    prepareProcess();
    _trx->getOptions()->setRtw(true);

    CPPUNIT_ASSERT(!_paxTypeFare->isRoutingProcessed());
    CPPUNIT_ASSERT(_rtC->process(*_fareMarket, *_tvlRoute, *_routingInfos));
    CPPUNIT_ASSERT(_paxTypeFare->isRoutingProcessed());
    CPPUNIT_ASSERT(_paxTypeFare->isRoutingValid());
  }

  void testProcessWhenRtwGenericPassAndNoSpecified()
  {
    prepareProcess();
    _trx->getOptions()->setRtw(true);

    static_cast<MyDataHandle*>(_myDataHandle)->_emptyRouting = "0002";

    CPPUNIT_ASSERT(!_paxTypeFare->isRoutingProcessed());
    CPPUNIT_ASSERT(_rtC->process(*_fareMarket, *_tvlRoute, *_routingInfos));
    CPPUNIT_ASSERT(_paxTypeFare->isRoutingProcessed());
    CPPUNIT_ASSERT(_paxTypeFare->isRoutingValid());
  }

  void testProcessWhenRtwGenericPassAndSpecifiedFail()
  {
    prepareProcess();
    _trx->getOptions()->setRtw(true);

    static_cast<MyDataHandle*>(_myDataHandle)->_failRouting = "0002";

    CPPUNIT_ASSERT(!_paxTypeFare->isRoutingProcessed());

    CPPUNIT_ASSERT(_rtC->process(*_fareMarket, *_tvlRoute, *_routingInfos));
    CPPUNIT_ASSERT(_paxTypeFare->isRoutingProcessed());
    CPPUNIT_ASSERT(!_paxTypeFare->isRoutingValid());
  }

  void testProcessWhenRtwNoGenericAndSpecifiedPass()
  {
    prepareProcess();
    _trx->getOptions()->setRtw(true);

    static_cast<MyDataHandle*>(_myDataHandle)->_emptyRouting = "4444";

    CPPUNIT_ASSERT(!_paxTypeFare->isRoutingProcessed());

    CPPUNIT_ASSERT(_rtC->process(*_fareMarket, *_tvlRoute, *_routingInfos));
    CPPUNIT_ASSERT(_paxTypeFare->isRoutingProcessed());
    CPPUNIT_ASSERT(_paxTypeFare->isRoutingValid());
  }

  void testProcessWhenRtwNoGenericAndSpecifiedFail()
  {
    prepareProcess();
    _trx->getOptions()->setRtw(true);

    static_cast<MyDataHandle*>(_myDataHandle)->_emptyRouting = "4444";
    static_cast<MyDataHandle*>(_myDataHandle)->_failRouting = "0002";

    CPPUNIT_ASSERT(!_paxTypeFare->isRoutingProcessed());

    CPPUNIT_ASSERT(_rtC->process(*_fareMarket, *_tvlRoute, *_routingInfos));
    CPPUNIT_ASSERT(_paxTypeFare->isRoutingProcessed());
    CPPUNIT_ASSERT(!_paxTypeFare->isRoutingValid());
  }

  void testProcessWhenRtwNoGenericAndNoSpecified()
  {
    prepareProcess();
    _trx->getOptions()->setRtw(true);

    static_cast<MyDataHandle*>(_myDataHandle)->_emptyRouting = "BOTH";

    CPPUNIT_ASSERT(!_paxTypeFare->isRoutingProcessed());

    CPPUNIT_ASSERT(_rtC->process(*_fareMarket, *_tvlRoute, *_routingInfos));
    CPPUNIT_ASSERT(_paxTypeFare->isRoutingProcessed());
    CPPUNIT_ASSERT(!_paxTypeFare->isRoutingValid());
  }

  void testProcessWhenRtwGenericFailAndSpecifiedPass()
  {
    prepareProcess();
    _trx->getOptions()->setRtw(true);

    static_cast<MyDataHandle*>(_myDataHandle)->_failRouting = "4444";

    CPPUNIT_ASSERT(!_paxTypeFare->isRoutingProcessed());

    CPPUNIT_ASSERT(_rtC->process(*_fareMarket, *_tvlRoute, *_routingInfos));
    CPPUNIT_ASSERT(_paxTypeFare->isRoutingProcessed());
    CPPUNIT_ASSERT(!_paxTypeFare->isRoutingValid());
  }

  void testProcessWhenRtwGenericFailAndNoSpecified()
  {
    prepareProcess();
    _trx->getOptions()->setRtw(true);

    static_cast<MyDataHandle*>(_myDataHandle)->_failRouting = "4444";
    static_cast<MyDataHandle*>(_myDataHandle)->_emptyRouting = "0002";

    CPPUNIT_ASSERT(!_paxTypeFare->isRoutingProcessed());

    CPPUNIT_ASSERT(_rtC->process(*_fareMarket, *_tvlRoute, *_routingInfos));
    CPPUNIT_ASSERT(_paxTypeFare->isRoutingProcessed());
    CPPUNIT_ASSERT(!_paxTypeFare->isRoutingValid());
  }

  void testProcessWhenRtwGenericFailAndSpecifiedFail()
  {
    prepareProcess();
    _trx->getOptions()->setRtw(true);

    static_cast<MyDataHandle*>(_myDataHandle)->_failRouting = "BOTH";

    CPPUNIT_ASSERT(!_paxTypeFare->isRoutingProcessed());

    CPPUNIT_ASSERT(_rtC->process(*_fareMarket, *_tvlRoute, *_routingInfos));
    CPPUNIT_ASSERT(_paxTypeFare->isRoutingProcessed());
    CPPUNIT_ASSERT(!_paxTypeFare->isRoutingValid());
  }

  void testValidateRestrictions()
  {
    _trx->diagnostic().diagnosticType() = Diagnostic455;

    RoutingRestriction rest;
    rest.restrSeqNo() = 1;
    rest.restriction() = "9";
    rest.marketAppl() = ' '; // B=Between City1 and City2, T=To/From City1
    rest.negViaAppl() = ' '; // R=Required, P=Permitted, N=Not Permitted, Blank=Not Applicable
    rest.viaType() = ' '; // A=Airline, C=City, Blank=Not Applicable
    rest.nonStopDirectInd() = ' '; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
    rest.airSurfaceInd() = ' '; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
    rest.market1() = ""; // City1
    rest.market2() = ""; // City2
    rest.viaMarket() = ""; // Via City
    rest.viaCarrier() = "   "; // Via Carrier

    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "CHI";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "BOM";
    offCty.isHiddenCity() = false;

    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.govCxr() = "AA";
    strToGlobalDirection(tvlRoute.globalDir(), "AT");
    tvlRoute.travelRoute().push_back(cityCarrier);

    Routing routing;
    routing.vendor() = "ATP";
    routing.carrier() = "AA";
    routing.routingTariff() = 99;
    routing.routing() = "0519";
    routing.effDateStr() = " ";
    routing.linkNo() = 0;
    routing.noofheaders() = 1;
    routing.noofRestrictions() = 1;
    routing.nooftexts() = 0;
    routing.validityInd() = ' ';
    routing.inhibit() = ' ';
    routing.directionalInd() = ' ';
    routing.domRtgvalInd() = ' ';
    routing.commonPointInd() = ' ';
    routing.jointRoutingOpt() = ' ';

    RoutingInfo* routingInfo = _memHandle.create<RoutingInfo>();
    routingInfo->routingStatus() = true;
    Routing* rtgPtr = &routing;
    routingInfo->routing() = rtgPtr;

    RestrictionInfo resInfo;
    resInfo.processed() = true;
    resInfo.valid() = true;

    PaxTypeFare paxTypeFare;

    RoutingController rc(*_trx);
    CPPUNIT_ASSERT(rc.validateRestriction(*_trx, paxTypeFare, rest, tvlRoute, routingInfo));

    rest.restriction() = "32";
    CPPUNIT_ASSERT(!(rc.validateRestriction(*_trx, paxTypeFare, rest, tvlRoute, routingInfo)));
  }

  void testMileageRoutingRTW()
  {
    RtgKey rtgKey;
    rtgKey.routingNumber() = MILEAGE_ROUTING;

    TravelRoute tvlRoute;
    RoutingInfo* rtgInfo = _memHandle.create<RoutingInfo>();
    bool rtgValid = false;
    bool mileageValid = true;

    RoutingController rc(*_trx);
    _trx->getOptions()->setRtw(true);

    rc.processRoutingMileageRouting(rtgKey, *_trx, tvlRoute, _paxTypeFare, rtgInfo, rtgValid, mileageValid);
    CPPUNIT_ASSERT(!mileageValid);
    CPPUNIT_ASSERT(!rtgInfo->mileageInfo());
  }

  void testLocMatchesOrigin()
  {
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "YTO";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "NYC";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "NYC";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "MIA";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "MIA";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "SJU";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "SJU";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "AUA";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "AUA";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "CUR";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    tvlRoute.govCxr() = "AA";
    strToGlobalDirection(tvlRoute.globalDir(), "WH");
    tvlRoute.origin() = "YTO";
    tvlRoute.originNation() = "CA";
    tvlRoute.destination() = "CUR";
    tvlRoute.destinationNation() = "CU";
    tvlRoute.travelDate() = DateTime::localTime();

    AirSeg aS1, aS2, aS3, aS4, aS5;
    LocCode loc1, loc2, loc3, loc4, loc5, loc6;

    loc1 = "YTO";
    loc2 = "NYC";
    loc3 = "MIA";
    loc4 = "SJU";
    loc5 = "AUA";
    loc6 = "CUR";

    aS1.carrier() = "AA";
    aS2.carrier() = "AA";
    aS3.carrier() = "AA";
    aS4.carrier() = "AA";
    aS5.carrier() = "AA";

    aS1.boardMultiCity() = loc1;
    aS2.boardMultiCity() = loc2;
    aS3.boardMultiCity() = loc3;
    aS4.boardMultiCity() = loc4;
    aS5.boardMultiCity() = loc5;

    Loc lc1, lc2, lc3, lc4, lc5, lc6;

    lc1.nation() = "CA";
    lc2.nation() = "US";
    lc3.nation() = "US";
    lc4.nation() = "US";
    lc4.state() = "PR";
    lc5.nation() = "AU";
    lc6.nation() = "CU";
    aS1.origin() = &lc1;
    aS2.origin() = &lc2;
    aS3.origin() = &lc3;
    aS4.origin() = &lc4;
    aS5.origin() = &lc5;
    aS1.destination() = &lc2;
    aS2.destination() = &lc3;
    aS3.destination() = &lc4;
    aS4.destination() = &lc5;
    aS5.destination() = &lc6;
    aS1.offMultiCity() = loc2;
    aS2.offMultiCity() = loc3;
    aS3.offMultiCity() = loc4;
    aS4.offMultiCity() = loc5;
    aS5.offMultiCity() = loc6;

    aS1.carrier() = "AA";
    aS2.carrier() = "AA";
    aS3.carrier() = "AA";
    aS4.carrier() = "AA";
    aS5.carrier() = "AA";

    std::vector<TravelSeg*> tvlSegs;
    FareMarket fm;

    tvlSegs.push_back(&aS1);
    tvlSegs.push_back(&aS2);
    tvlSegs.push_back(&aS3);
    tvlSegs.push_back(&aS4);
    tvlSegs.push_back(&aS5);

    fm.setFltTrkIndicator(false);
    fm.travelSeg() = tvlSegs;
    copy(
        fm.travelSeg().begin(), fm.travelSeg().end(), back_inserter(tvlRoute.mileageTravelRoute()));

    tvlRoute.primarySector() = &aS4;

    const Loc* loc = _trx->dataHandle().getLoc("BOS", tvlRoute.travelDate());

    RoutingController rc(*_trx);
    CPPUNIT_ASSERT(RoutingUtil::locMatchesOrigin(loc, tvlRoute));

    loc = _trx->dataHandle().getLoc("YTO", tvlRoute.travelDate());
    CPPUNIT_ASSERT(RoutingUtil::locMatchesOrigin(loc, tvlRoute));

    loc = _trx->dataHandle().getLoc("STT", tvlRoute.travelDate());
    CPPUNIT_ASSERT(RoutingUtil::locMatchesOrigin(loc, tvlRoute));

    loc = _trx->dataHandle().getLoc("FRA", tvlRoute.travelDate());
    CPPUNIT_ASSERT(!(RoutingUtil::locMatchesOrigin(loc, tvlRoute)));
  }

  void testLocMatchesDestination()
  {
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "YTO";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "NYC";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "NYC";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "MIA";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "MIA";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "SJU";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "SJU";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "AUA";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "AUA";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "CUR";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    tvlRoute.govCxr() = "AA";
    strToGlobalDirection(tvlRoute.globalDir(), "WH");
    tvlRoute.origin() = "YTO";
    tvlRoute.originNation() = "CA";
    tvlRoute.destination() = "CUR";
    tvlRoute.destinationNation() = "CU";
    tvlRoute.travelDate() = DateTime::localTime();

    AirSeg aS1, aS2, aS3, aS4, aS5;
    LocCode loc1, loc2, loc3, loc4, loc5, loc6;

    loc1 = "YTO";
    loc2 = "NYC";
    loc3 = "MIA";
    loc4 = "SJU";
    loc5 = "AUA";
    loc6 = "CUR";

    aS1.carrier() = "AA";
    aS2.carrier() = "AA";
    aS3.carrier() = "AA";
    aS4.carrier() = "AA";
    aS5.carrier() = "AA";

    aS1.boardMultiCity() = loc1;
    aS2.boardMultiCity() = loc2;
    aS3.boardMultiCity() = loc3;
    aS4.boardMultiCity() = loc4;
    aS5.boardMultiCity() = loc5;

    Loc lc1, lc2, lc3, lc4, lc5, lc6;

    lc1.nation() = "CA";
    lc2.nation() = "US";
    lc3.nation() = "US";
    lc4.nation() = "US";
    lc4.state() = "PR";
    lc5.nation() = "AU";
    lc6.nation() = "CU";
    aS1.origin() = &lc1;
    aS2.origin() = &lc2;
    aS3.origin() = &lc3;
    aS4.origin() = &lc4;
    aS5.origin() = &lc5;
    aS1.destination() = &lc2;
    aS2.destination() = &lc3;
    aS3.destination() = &lc4;
    aS4.destination() = &lc5;
    aS5.destination() = &lc6;
    aS1.offMultiCity() = loc2;
    aS2.offMultiCity() = loc3;
    aS3.offMultiCity() = loc4;
    aS4.offMultiCity() = loc5;
    aS5.offMultiCity() = loc6;

    aS1.carrier() = "AA";
    aS2.carrier() = "AA";
    aS3.carrier() = "AA";
    aS4.carrier() = "AA";
    aS5.carrier() = "AA";

    std::vector<TravelSeg*> tvlSegs;
    FareMarket fm;

    tvlSegs.push_back(&aS1);
    tvlSegs.push_back(&aS2);
    tvlSegs.push_back(&aS3);
    tvlSegs.push_back(&aS4);
    tvlSegs.push_back(&aS5);

    fm.setFltTrkIndicator(false);
    fm.travelSeg() = tvlSegs;
    copy(
        fm.travelSeg().begin(), fm.travelSeg().end(), back_inserter(tvlRoute.mileageTravelRoute()));

    tvlRoute.primarySector() = &aS4;

    const Loc* loc = _trx->dataHandle().getLoc("SJU", tvlRoute.travelDate());
    RoutingController rc(*_trx);
    CPPUNIT_ASSERT(!(RoutingUtil::locMatchesDestination(loc, tvlRoute)));

    loc = _trx->dataHandle().getLoc("YTO", tvlRoute.travelDate());
    CPPUNIT_ASSERT(!(RoutingUtil::locMatchesDestination(loc, tvlRoute)));

    loc = _trx->dataHandle().getLoc("MSP", tvlRoute.travelDate());
    CPPUNIT_ASSERT(!(RoutingUtil::locMatchesDestination(loc, tvlRoute)));

    loc = _trx->dataHandle().getLoc("STT", tvlRoute.travelDate());
    CPPUNIT_ASSERT(!(RoutingUtil::locMatchesDestination(loc, tvlRoute)));

    loc = _trx->dataHandle().getLoc("MUC", tvlRoute.travelDate());
    CPPUNIT_ASSERT(!(RoutingUtil::locMatchesDestination(loc, tvlRoute)));
  }

  void testLocMatchesDestinationB()
  {
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "DEN";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "CHI";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "UA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "CHI";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "NYC";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "UA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "NYC";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "YTO";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "UA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    tvlRoute.govCxr() = "UA";
    strToGlobalDirection(tvlRoute.globalDir(), "WH");
    tvlRoute.origin() = "DEN";
    tvlRoute.originNation() = "UA";
    tvlRoute.destination() = "YTO";
    tvlRoute.destinationNation() = "CA";
    tvlRoute.travelDate() = DateTime::localTime();

    AirSeg aS1, aS2, aS3;
    LocCode loc1, loc2, loc3, loc4;

    loc1 = "DEN";
    loc2 = "CHI";
    loc3 = "NYC";
    loc4 = "YTO";

    aS1.carrier() = "UA";
    aS2.carrier() = "UA";
    aS3.carrier() = "UA";

    aS1.boardMultiCity() = loc1;
    aS2.boardMultiCity() = loc2;
    aS3.boardMultiCity() = loc3;

    Loc lc1, lc2, lc3, lc4;

    lc1.nation() = "US";
    lc2.nation() = "US";
    lc3.nation() = "US";
    lc4.nation() = "CA";

    aS1.origin() = &lc1;
    aS2.origin() = &lc2;
    aS3.origin() = &lc3;

    aS1.destination() = &lc2;
    aS2.destination() = &lc3;
    aS3.destination() = &lc4;

    aS1.offMultiCity() = loc2;
    aS2.offMultiCity() = loc3;
    aS3.offMultiCity() = loc4;

    aS1.carrier() = "UA";
    aS2.carrier() = "UA";
    aS3.carrier() = "UA";

    std::vector<TravelSeg*> tvlSegs;
    FareMarket fm;

    tvlSegs.push_back(&aS1);
    tvlSegs.push_back(&aS2);
    tvlSegs.push_back(&aS3);

    fm.setFltTrkIndicator(false);
    fm.travelSeg() = tvlSegs;
    copy(
        fm.travelSeg().begin(), fm.travelSeg().end(), back_inserter(tvlRoute.mileageTravelRoute()));

    tvlRoute.primarySector() = &aS3;

    const Loc* loc = _trx->dataHandle().getLoc("SJU", tvlRoute.travelDate());
    RoutingController rc(*_trx);
    CPPUNIT_ASSERT(RoutingUtil::locMatchesDestination(loc, tvlRoute));

    loc = _trx->dataHandle().getLoc("YTO", tvlRoute.travelDate());
    CPPUNIT_ASSERT(RoutingUtil::locMatchesDestination(loc, tvlRoute));

    loc = _trx->dataHandle().getLoc("MSP", tvlRoute.travelDate());
    CPPUNIT_ASSERT(RoutingUtil::locMatchesDestination(loc, tvlRoute));

    loc = _trx->dataHandle().getLoc("STT", tvlRoute.travelDate());
    CPPUNIT_ASSERT(RoutingUtil::locMatchesDestination(loc, tvlRoute));

    loc = _trx->dataHandle().getLoc("ATL", tvlRoute.travelDate());
    CPPUNIT_ASSERT(RoutingUtil::locMatchesDestination(loc, tvlRoute));
  }

  void testBuildTravelSegs()
  {
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "STL";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "DFW";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "DFW";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "CHI";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "CHI";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "NYC";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "NYC";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "LON";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "BA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "LON";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "MOW";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "BA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    tvlRoute.govCxr() = "BA";
    strToGlobalDirection(tvlRoute.globalDir(), "AT");
    tvlRoute.origin() = "STL";
    tvlRoute.originNation() = "US";
    tvlRoute.destination() = "MOW";
    tvlRoute.destinationNation() = "RU";
    tvlRoute.travelDate() = DateTime::localTime();

    AirSeg aS1, aS2, aS3, aS4, aS5;
    LocCode loc1, loc2, loc3, loc4, loc5, loc6;

    loc1 = "STL";
    loc2 = "DFW";
    loc3 = "CHI";
    loc4 = "NYC";
    loc5 = "LON";
    loc6 = "MOW";

    aS1.carrier() = "AA";
    aS2.carrier() = "AA";
    aS3.carrier() = "AA";
    aS4.carrier() = "BA";
    aS5.carrier() = "BA";

    aS1.boardMultiCity() = loc1;
    aS2.boardMultiCity() = loc2;
    aS3.boardMultiCity() = loc3;
    aS4.boardMultiCity() = loc4;
    aS5.boardMultiCity() = loc5;

    Loc lc1, lc2, lc3, lc4, lc5, lc6;

    lc1.nation() = "US";
    lc2.nation() = "US";
    lc3.nation() = "US";
    lc4.nation() = "US";
    lc5.nation() = "UK";
    lc6.nation() = "RU";
    aS1.origin() = &lc1;
    aS2.origin() = &lc2;
    aS3.origin() = &lc3;
    aS4.origin() = &lc4;
    aS5.origin() = &lc5;
    aS1.destination() = &lc2;
    aS2.destination() = &lc3;
    aS3.destination() = &lc4;
    aS4.destination() = &lc5;
    aS1.offMultiCity() = loc2;
    aS2.offMultiCity() = loc3;
    aS3.offMultiCity() = loc4;
    aS4.offMultiCity() = loc5;
    aS5.offMultiCity() = loc6;

    aS1.carrier() = "AA";
    aS2.carrier() = "AA";
    aS3.carrier() = "AA";
    aS4.carrier() = "BA";
    aS5.carrier() = "BA";

    std::vector<TravelSeg*> tvlSegs;
    FareMarket fm;

    tvlSegs.push_back(&aS1);
    tvlSegs.push_back(&aS2);
    tvlSegs.push_back(&aS3);
    tvlSegs.push_back(&aS4);
    tvlSegs.push_back(&aS5);

    fm.setFltTrkIndicator(false);
    fm.travelSeg() = tvlSegs;
    copy(
        fm.travelSeg().begin(), fm.travelSeg().end(), back_inserter(tvlRoute.mileageTravelRoute()));

    tvlRoute.primarySector() = &aS4;

    const Loc* loc = _trx->dataHandle().getLoc("STL", tvlRoute.travelDate());

    std::vector<TravelSeg*> newTvlSegs;

    RoutingController rc(*_trx);
    CPPUNIT_ASSERT(rc.buildTravelSegs(tvlRoute, loc, newTvlSegs));
  }

  void testBuildTravelSegsB()
  {
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "YTO";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "NYC";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "NYC";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "MIA";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "MIA";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "SJU";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "SJU";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "AUA";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "AUA";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "CUR";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    tvlRoute.govCxr() = "AA";
    strToGlobalDirection(tvlRoute.globalDir(), "WH");
    tvlRoute.origin() = "YTO";
    tvlRoute.originNation() = "CA";
    tvlRoute.destination() = "CUR";
    tvlRoute.destinationNation() = "CU";
    tvlRoute.travelDate() = DateTime::localTime();

    AirSeg aS1, aS2, aS3, aS4, aS5;
    LocCode loc1, loc2, loc3, loc4, loc5, loc6;

    loc1 = "YTO";
    loc2 = "NYC";
    loc3 = "MIA";
    loc4 = "SJU";
    loc5 = "AUA";
    loc6 = "CUR";

    aS1.carrier() = "AA";
    aS2.carrier() = "AA";
    aS3.carrier() = "AA";
    aS4.carrier() = "AA";
    aS5.carrier() = "AA";

    aS1.boardMultiCity() = loc1;
    aS2.boardMultiCity() = loc2;
    aS3.boardMultiCity() = loc3;
    aS4.boardMultiCity() = loc4;
    aS5.boardMultiCity() = loc5;

    Loc lc1, lc2, lc3, lc4, lc5, lc6;

    lc1.nation() = "CA";
    lc2.nation() = "US";
    lc3.nation() = "US";
    lc4.nation() = "US";
    lc4.state() = "PR";
    lc5.nation() = "AU";
    lc6.nation() = "CU";
    aS1.origin() = &lc1;
    aS2.origin() = &lc2;
    aS3.origin() = &lc3;
    aS4.origin() = &lc4;
    aS5.origin() = &lc5;
    aS1.destination() = &lc2;
    aS2.destination() = &lc3;
    aS3.destination() = &lc4;
    aS4.destination() = &lc5;
    aS1.offMultiCity() = loc2;
    aS2.offMultiCity() = loc3;
    aS3.offMultiCity() = loc4;
    aS4.offMultiCity() = loc5;
    aS5.offMultiCity() = loc6;

    aS1.carrier() = "AA";
    aS2.carrier() = "AA";
    aS3.carrier() = "AA";
    aS4.carrier() = "AA";
    aS5.carrier() = "AA";

    std::vector<TravelSeg*> tvlSegs;
    FareMarket fm;

    tvlSegs.push_back(&aS1);
    tvlSegs.push_back(&aS2);
    tvlSegs.push_back(&aS3);
    tvlSegs.push_back(&aS4);
    tvlSegs.push_back(&aS5);

    fm.setFltTrkIndicator(false);
    fm.travelSeg() = tvlSegs;
    copy(
        fm.travelSeg().begin(), fm.travelSeg().end(), back_inserter(tvlRoute.mileageTravelRoute()));

    tvlRoute.primarySector() = &aS4;

    const Loc* loc = _trx->dataHandle().getLoc("YYZ", tvlRoute.travelDate());

    std::vector<TravelSeg*> newTvlSegs;

    RoutingController rc(*_trx);
    CPPUNIT_ASSERT(rc.buildTravelSegs(tvlRoute, loc, newTvlSegs));
  }

  void testLocMatchesTvlSeg()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelDate() = DateTime::localTime();

    AirSeg aS1;
    LocCode loc1, loc2;

    loc1 = "MSP";
    loc2 = "YVR";
    aS1.carrier() = "AA";
    aS1.boardMultiCity() = loc1;
    aS1.offMultiCity() = loc2;

    Loc lc1, lc2;
    lc1.nation() = "US";
    lc2.nation() = "CA";

    aS1.origin() = &lc1;
    aS1.destination() = &lc2;
    aS1.carrier() = "AA";

    const Loc* loc = _trx->dataHandle().getLoc("BOS", tvlRoute.travelDate());

    RoutingController rc(*_trx);
    CPPUNIT_ASSERT(RoutingUtil::locMatchesTvlSeg(loc, aS1));

    loc = _trx->dataHandle().getLoc("YTO", tvlRoute.travelDate());
    CPPUNIT_ASSERT(RoutingUtil::locMatchesTvlSeg(loc, aS1));

    loc = _trx->dataHandle().getLoc("STT", tvlRoute.travelDate());
    CPPUNIT_ASSERT(RoutingUtil::locMatchesTvlSeg(loc, aS1));

    loc = _trx->dataHandle().getLoc("SJU", tvlRoute.travelDate());
    CPPUNIT_ASSERT(RoutingUtil::locMatchesTvlSeg(loc, aS1));

    loc = _trx->dataHandle().getLoc("MEX", tvlRoute.travelDate());
    CPPUNIT_ASSERT(!(RoutingUtil::locMatchesTvlSeg(loc, aS1)));

    loc = _trx->dataHandle().getLoc("ASU", tvlRoute.travelDate());
    CPPUNIT_ASSERT(!(RoutingUtil::locMatchesTvlSeg(loc, aS1)));

    loc = _trx->dataHandle().getLoc("TYO", tvlRoute.travelDate());
    CPPUNIT_ASSERT(!(RoutingUtil::locMatchesTvlSeg(loc, aS1)));
  }

  void testBuildTravelSegsC()
  {
    // Build Travel Route
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;

    boardCty.loc() = "BEL";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "SNN";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "EI";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "SNN";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "DUB";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "EI";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "DUB";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "MAN";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "EI";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "MAN";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "LON";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "BA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "LON";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "PAR";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "BA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "PAR";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "NCE";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AF";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "NCE";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "MRS";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AF";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    tvlRoute.govCxr() = "EI";
    strToGlobalDirection(tvlRoute.globalDir(), "WH");
    tvlRoute.origin() = "BEL";
    tvlRoute.originNation() = "IR";
    tvlRoute.destination() = "MRS";
    tvlRoute.destinationNation() = "FR";
    tvlRoute.travelDate() = DateTime::localTime();

    AirSeg aS1, aS2, aS3, aS4, aS5, aS6, aS7;
    LocCode loc1, loc2, loc3, loc4, loc5, loc6, loc7, loc8;

    loc1 = "BEL";
    loc2 = "SNN";
    loc3 = "DUB";
    loc4 = "MAN";
    loc5 = "LON";
    loc6 = "PAR";
    loc7 = "NCE";
    loc8 = "MRS";

    aS1.carrier() = "EI";
    aS2.carrier() = "EI";
    aS3.carrier() = "EI";
    aS4.carrier() = "BA";
    aS5.carrier() = "BA";
    aS6.carrier() = "AF";
    aS7.carrier() = "AF";

    aS1.boardMultiCity() = loc1;
    aS2.boardMultiCity() = loc2;
    aS3.boardMultiCity() = loc3;
    aS4.boardMultiCity() = loc4;
    aS5.boardMultiCity() = loc5;
    aS6.boardMultiCity() = loc6;
    aS7.boardMultiCity() = loc7;

    Loc lc1, lc2, lc3, lc4, lc5, lc6, lc7, lc8;

    lc1.nation() = "IR";
    lc2.nation() = "IR";
    lc3.nation() = "IR";
    lc4.nation() = "UK";
    lc5.nation() = "UK";
    lc6.nation() = "FR";
    lc7.nation() = "FR";
    lc8.nation() = "FR";

    aS1.origin() = &lc1;
    aS2.origin() = &lc2;
    aS3.origin() = &lc3;
    aS4.origin() = &lc4;
    aS5.origin() = &lc5;
    aS6.origin() = &lc6;
    aS7.origin() = &lc7;

    aS1.destination() = &lc2;
    aS2.destination() = &lc3;
    aS3.destination() = &lc4;
    aS4.destination() = &lc5;
    aS5.destination() = &lc6;
    aS6.destination() = &lc7;
    aS7.destination() = &lc8;

    aS1.offMultiCity() = loc2;
    aS2.offMultiCity() = loc3;
    aS3.offMultiCity() = loc4;
    aS4.offMultiCity() = loc5;
    aS5.offMultiCity() = loc6;
    aS6.offMultiCity() = loc7;
    aS7.offMultiCity() = loc8;

    std::vector<TravelSeg*> tvlSegs;
    FareMarket fm;

    tvlSegs.push_back(&aS1);
    tvlSegs.push_back(&aS2);
    tvlSegs.push_back(&aS3);
    tvlSegs.push_back(&aS4);
    tvlSegs.push_back(&aS5);
    tvlSegs.push_back(&aS6);
    tvlSegs.push_back(&aS7);

    fm.setFltTrkIndicator(false);
    fm.travelSeg() = tvlSegs;
    copy(
        fm.travelSeg().begin(), fm.travelSeg().end(), back_inserter(tvlRoute.mileageTravelRoute()));

    tvlRoute.primarySector() = &aS3;
    TravelRoute newTvlRoute;
    RtgKey rKey;

    const Routing* origAddOnRouting = 0;
    const Routing* destAddOnRouting = 0;

    std::vector<TravelSeg*> newTvlSegs;
    rKey.addOnRouting1() = MILEAGE_ROUTING;
    RoutingController rc(*_trx);
    CPPUNIT_ASSERT(rc.buildTravelSegs(
        origAddOnRouting, destAddOnRouting, tvlRoute, newTvlRoute, newTvlSegs, rKey));

    std::vector<TravelSeg*> newTvlSegs2;
    rKey.addOnRouting1() = "1234";
    rKey.addOnRouting2() = MILEAGE_ROUTING;
    CPPUNIT_ASSERT(rc.buildTravelSegs(
        origAddOnRouting, destAddOnRouting, tvlRoute, newTvlRoute, newTvlSegs2, rKey));

    std::vector<TravelSeg*> newTvlSegs3;
    rKey.addOnRouting1() = MILEAGE_ROUTING;
    rKey.addOnRouting2() = MILEAGE_ROUTING;
    CPPUNIT_ASSERT(rc.buildTravelSegs(
        origAddOnRouting, destAddOnRouting, tvlRoute, newTvlRoute, newTvlSegs3, rKey));
  }

  void testSurchargeException()
  {
    TravelRoute tvlRoute;
    std::vector<PaxTypeFare*> pFares;
    RtgKey rKey;
    RoutingInfos infos;
    RoutingInfo rInfo;
    MileageInfo mInfo;
    mInfo.valid() = true;
    mInfo.surchargeAmt() = 10;
    rInfo.mileageInfo() = &mInfo;
    infos.insert(RoutingInfos::value_type(rKey, &rInfo));

    RoutingController rc(*_trx);
    rc.processSurchargeException(*_trx, pFares, infos, tvlRoute);
  }

  void testUpdateFareSurcharge()
  {
    PaxTypeFare paxTypeFare1, paxTypeFare2, paxTypeFare3;
    RoutingController rtC(*_trx);
    MileageInfo mInfo;

    mInfo.valid() = false;
    mInfo.surchargeAmt() = 10;
    mInfo.surchargeAmtSAException() = 15;
    mInfo.totalApplicableTPMSAException() = NO_TPM;

    mInfo.valid() = true;
    paxTypeFare2.nucFareAmount() = 5050;
    rtC.updateFareSurcharge(paxTypeFare2, mInfo);
    CPPUNIT_ASSERT_EQUAL(uint16_t(10), paxTypeFare2.mileageSurchargePctg());
    CPPUNIT_ASSERT_EQUAL(double(505), paxTypeFare2.mileageSurchargeAmt());

    paxTypeFare3.nucFareAmount() = 5050;
    mInfo.totalApplicableTPMSAException() = 10838;
    rtC.updateFareSurcharge(paxTypeFare3, mInfo);
    CPPUNIT_ASSERT_EQUAL(uint16_t(10), paxTypeFare3.mileageSurchargePctg());
    CPPUNIT_ASSERT_EQUAL(double(505), paxTypeFare3.mileageSurchargeAmt());

    FareInfo fInfo;
    Fare fare;
    fInfo.carrier() = INDUSTRY_CARRIER;
    fare.setFareInfo(&fInfo);
    paxTypeFare3.setFare(&fare);
    paxTypeFare3.setIsRouting();
    paxTypeFare3.nucFareAmount() = 5050;
    rtC.updateFareSurcharge(paxTypeFare3, mInfo);
    CPPUNIT_ASSERT_EQUAL(uint16_t(15), paxTypeFare3.mileageSurchargePctg());
    CPPUNIT_ASSERT_EQUAL(double(757.5), paxTypeFare3.mileageSurchargeAmt());
  }

  void testValidateMileage()
  {
    PricingRequest pricingRequest;
    PaxTypeFare paxTypeFare;
    RoutingController rtC(*_trx);
    MileageInfo* mileageInfo;
    DataHandle datahandle;
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;
    CarrierCode carCode1, carCode2;
    Fare fare;
    FareInfoMock fareInfo;
    FareMarket fareMarket;
    PaxType paxType;

    fare.setFareInfo(&fareInfo);
    paxTypeFare.setFare(&fare);

    const DateTime ticketingDate = DateTime::localTime();

    pricingRequest.ticketingDT() = ticketingDate;
    _trx->setRequest(&pricingRequest);

    boardCty.loc() = "RIO";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "NYC";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "NYC";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "LON";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    tvlRoute.govCxr() = "AA";
    strToGlobalDirection(tvlRoute.globalDir(), "AT");
    tvlRoute.origin() = "RIO";
    tvlRoute.originNation() = "BR";
    tvlRoute.destination() = "LON";
    tvlRoute.destinationNation() = "UK";
    tvlRoute.travelDate() = DateTime::localTime();

    _trx->dataHandle().get(mileageInfo);
    carCode1 = "AA";
    carCode2 = "AA";
    AirSeg aS1, aS2;
    LocCode loc1, loc2, loc3;

    aS1.origin() = datahandle.getLoc("RIO", tvlRoute.travelDate());
    aS2.origin() = aS1.destination() = datahandle.getLoc("NYC", tvlRoute.travelDate());
    aS2.destination() = datahandle.getLoc("LON", tvlRoute.travelDate());

    loc1 = "RIO";
    loc2 = "NYC";
    loc3 = "LON";
    aS1.boardMultiCity() = loc1;
    aS1.offMultiCity() = loc2;
    aS2.boardMultiCity() = loc2;
    aS2.offMultiCity() = loc3;

    aS1.carrier() = carCode1;
    aS2.carrier() = carCode2;
    tvlRoute.mileageTravelRoute().push_back(&aS1);
    tvlRoute.mileageTravelRoute().push_back(&aS2);

    paxTypeFare.fareMarket() = &fareMarket;
    rtC.validateMileage(*_trx, paxTypeFare, tvlRoute, *mileageInfo);

    //        CPPUNIT_ASSERT_EQUAL(double(5050), paxTypeFare.fare()->nucFareAmount());
    CPPUNIT_ASSERT_EQUAL(uint16_t(6920), paxTypeFare.mileageInfo()->totalApplicableMPM());
  }

  void testValidateMileage_Diagnostic452()
  {
    PricingRequest pricingRequest;
    PaxTypeFare paxTypeFare;
    RoutingController rtC(*_trx);
    MileageInfo* mileageInfo;
    DataHandle datahandle;
    TravelRoute tvlRoute;
    TravelRoute::CityCarrier cityCarrier;
    TravelRoute::City boardCty, offCty;
    CarrierCode carCode1, carCode2;
    Fare fare;
    FareInfoMock fareInfo;
    FareMarket fareMarket;
    PaxType paxType;

    fare.setFareInfo(&fareInfo);
    paxTypeFare.setFare(&fare);

    const DateTime ticketingDate = DateTime::localTime();

    pricingRequest.ticketingDT() = ticketingDate;
    _trx->setRequest(&pricingRequest);

    boardCty.loc() = "RIO";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "NYC";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    boardCty.loc() = "NYC";
    boardCty.isHiddenCity() = false;
    offCty.loc() = "LON";
    offCty.isHiddenCity() = false;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = "AA";
    cityCarrier.offCity() = offCty;
    tvlRoute.travelRoute().push_back(cityCarrier);

    tvlRoute.govCxr() = "AA";
    strToGlobalDirection(tvlRoute.globalDir(), "AT");
    tvlRoute.origin() = "RIO";
    tvlRoute.originNation() = "BR";
    tvlRoute.destination() = "LON";
    tvlRoute.destinationNation() = "UK";
    tvlRoute.travelDate() = DateTime::localTime();

    _trx->dataHandle().get(mileageInfo);
    carCode1 = "AA";
    carCode2 = "AA";
    AirSeg aS1, aS2;
    LocCode loc1, loc2, loc3;

    loc1 = "RIO";
    loc2 = "NYC";
    loc3 = "LON";

    aS1.origin() = datahandle.getLoc("RIO", tvlRoute.travelDate());
    aS2.origin() = aS1.destination() = datahandle.getLoc("NYC", tvlRoute.travelDate());
    aS2.destination() = datahandle.getLoc("LON", tvlRoute.travelDate());

    aS1.carrier() = carCode1;
    aS2.carrier() = carCode2;
    tvlRoute.mileageTravelRoute().push_back(&aS1);
    tvlRoute.mileageTravelRoute().push_back(&aS2);
    paxTypeFare.fareMarket() = &fareMarket;

    _trx->diagnostic().diagnosticType() = Diagnostic452;
    _trx->diagnostic().activate();

    rtC.validateMileage(*_trx, paxTypeFare, tvlRoute, *mileageInfo);
    CPPUNIT_ASSERT_EQUAL(0,
                         (int)_trx->diagnostic().toString().find(
                             "*******************  TPM EXCLUSION TABLE  *********************"));
  }

  void prepareRoutingMap()
  {
    _routing = _memHandle.create<Routing>();
    _routing->vendor() = "ATP";
    _routing->carrier() = "AA";
    _routing->routingTariff() = 5;
    _routing->routing() = "0756";

    createMap(*_routing, 1, '1', 2, 0, 'C', "SEA");
    createMap(*_routing, 2, ' ', 4, 3, 'C', "DFW");
    createMap(*_routing, 3, ' ', 4, 0, 'C', "MIA");
    createMap(*_routing, 4, 'X', 0, 0, 'C', "CUN");

    _tvlRoute->travelRoute().clear();
    _tvlRoute->travelDate() = DateTime::localTime();
    createTravelRouteAndTravelSeg(*_tvlRoute, "SEA", "US", "AA", "DFW", "US");
    createTravelRouteAndTravelSeg(*_tvlRoute, "DFW", "US", "AA", "MIA", "US");
    createTravelRouteAndTravelSeg(*_tvlRoute, "MIA", "US", "AA", "CUN", "MX");
    _tvlRoute->primarySector() = _tvlRoute->mileageTravelRoute()[2];
  }

  void preparePtfForValidateRoutingMaps()
  {
    _fareMarket->travelSeg().push_back(_tvlRoute->mileageTravelRoute()[0]);
    _fareMarket->travelSeg().push_back(_tvlRoute->mileageTravelRoute()[1]);
    _fareMarket->governingCarrier() = "AA";
    _trx->fareMarket().push_back(_fareMarket);

    _tariffCrossRef->_routingTariff1 = 99;
    _tariffCrossRef->_routingTariff2 = 0;

    _fareInfo->_globalDirection = GlobalDirection::ZZ;
    _fareInfo->_owrt = ONE_WAY_MAY_BE_DOUBLED;
    _fareInfo->routingNumber() = "0002";
    _fareInfo->vendor() = "ATP";
    _fare->initialize(Fare::FS_International, _fareInfo, *_fareMarket, _tariffCrossRef);
    _fare->status().set(Fare::FS_PublishedFare);

    PaxType* paxType = _memHandle.create<PaxType>();
    PaxTypeInfo* paxTypeInfo = _memHandle.create<PaxTypeInfo>();
    paxType->paxTypeInfo() = paxTypeInfo;

    _paxTypeFare->initialize(_fare, paxType, _fareMarket, *_trx);
    _fareMarket->allPaxTypeFare().push_back(_paxTypeFare);
  }

  void validateRoutingMaps_Pass()
  {
    prepareRoutingMap();
    preparePtfForValidateRoutingMaps();
    MapInfo mapInfo;

    CPPUNIT_ASSERT(_rtC->validateRoutingMaps(*_trx,
                                             *_paxTypeFare,
                                             *_tvlRoute,
                                             &mapInfo,
                                             _routing,
                                             DateTime::localTime(),
                                             NULL,
                                             NULL));
  }

  void validateRoutingMaps_FailZone()
  {
    prepareRoutingMap();
    preparePtfForValidateRoutingMaps();
    MapInfo mapInfo;

    _trx->getOptions()->setRtw(true);
    RoutingMap* rMap = new RoutingMap();
    rMap->loc1().loc() = "210";
    rMap->loc1().locType() = MapNode::ZONE;
    _routing->rmaps().push_back(rMap);

    CPPUNIT_ASSERT(!_rtC->validateRoutingMaps(*_trx,
                                              *_paxTypeFare,
                                              *_tvlRoute,
                                              &mapInfo,
                                              _routing,
                                              DateTime::localTime(),
                                              NULL,
                                              NULL));
  }

  void validateRoutingMaps_FailNation()
  {
    prepareRoutingMap();
    preparePtfForValidateRoutingMaps();
    MapInfo mapInfo;

    _trx->getOptions()->setRtw(true);
    RoutingMap* rMap = new RoutingMap();
    rMap->loc1().loc() = "DE";
    rMap->loc1().locType() = MapNode::ZONE;
    _routing->rmaps().push_back(rMap);

    CPPUNIT_ASSERT(!_rtC->validateRoutingMaps(*_trx,
                                              *_paxTypeFare,
                                              *_tvlRoute,
                                              &mapInfo,
                                              _routing,
                                              DateTime::localTime(),
                                              NULL,
                                              NULL));
  }

  void testOriginAddonRTGSpecifiedRTGDestinationMileage()
  {
    TravelRoute tvlRoute;
    tvlRoute.travelRoute().clear();
    tvlRoute.travelDate() = DateTime::localTime();
    createTravelRouteAndTravelSeg(tvlRoute, "LSC", "CL", "LA", "SCL", "CL");
    createTravelRouteAndTravelSeg(tvlRoute, "SCL", "CL", "LA", "MIA", "US");
    createTravelRouteAndTravelSeg(tvlRoute, "MIA", "US", "AA", "SDQ", "DO");
    tvlRoute.primarySector() = tvlRoute.mileageTravelRoute()[1];

    Routing addonRTG;
    addonRTG.vendor() = "ATP";
    addonRTG.carrier() = "LA";
    addonRTG.routingTariff() = 17;
    addonRTG.routing() = "0097";

    createMap(addonRTG, 1, '1', 2, 0, 'C', "ESR");
    createMap(addonRTG, 2, ' ', 3, 0, 'C', "CPO");
    createMap(addonRTG, 3, ' ', 4, 0, 'C', "LSC");
    createMap(addonRTG, 4, ' ', 5, 0, 'A', "LA");
    createMap(addonRTG, 5, ' ', 6, 0, 'C', "SCL");
    createMap(addonRTG, 6, ' ', 7, 0, 'A', "LP");
    createMap(addonRTG, 7, ' ', 0, 0, 'C', "LIM");

    Routing specRTG;
    specRTG.vendor() = "ATP";
    specRTG.carrier() = "LA";
    specRTG.routingTariff() = 17;
    specRTG.routing() = "0193";

    Routing destRTG;
    destRTG.routing() = "0000";
    destRTG.carrier() = "LA";

    createMap(specRTG, 1, '1', 2, 0, 'C', "SCL");
    createMap(specRTG, 2, ' ', 4, 3, 'A', "LA");
    createMap(specRTG, 3, ' ', 4, 0, 'A', "LP");
    createMap(specRTG, 4, ' ', 5, 0, 'C', "MIA");
    createMap(specRTG, 5, ' ', 9, 6, 'A', "LA");
    createMap(specRTG, 6, ' ', 9, 7, 'A', "LP");
    createMap(specRTG, 7, ' ', 9, 8, 'A', "AA");
    createMap(specRTG, 8, ' ', 9, 0, 'A', "4M");
    createMap(specRTG, 9, 'X', 0, 0, 'C', "PUJ");

    RtgKey rKey;
    rKey.vendor() = "ATP";
    rKey.carrier() = "LA";
    rKey.routingNumber() = "0193";
    rKey.routingTariff() = 17;
    rKey.addOnRouting1() = "0097";
    rKey.addOnRouting2() = "0000";

    RoutingInfos routingInfos;
    Fare fare;
    FareInfo fareInfo;
    TariffCrossRefInfo tcrInfo;
    PaxType paxType;
    PaxTypeFare paxTypeFare1;

    // fare.initialize(Fare::FS_Domestic,Domestic,&fareInfo,&tcrInfo);
    fare.setFareInfo(&fareInfo);
    fare.setTariffCrossRefInfo(&tcrInfo);
    // Create the FareMarket
    FareMarket fareMarket;
    MileageInfo milInfo;
    fareMarket.mileageInfo(true) = &milInfo;
    fareMarket.mileageInfo(false) = &milInfo;

    paxTypeFare1.initialize(&fare, &paxType, &fareMarket, *_trx);

    MockRoutingController routingController(*_trx);

    const DateTime travelDate = DateTime::localTime();

    CPPUNIT_ASSERT(routingController.processRoutingValidation(rKey,
                                                              *_trx,
                                                              tvlRoute,
                                                              &paxTypeFare1,
                                                              &specRTG,
                                                              &addonRTG,
                                                              &destRTG,
                                                              routingInfos,
                                                              travelDate));
  }

  void testSetRtMapValidFlag()
  {
    TravelRoute tvlRoute;

    Routing addonRTG;
    Routing specRTG;
    Routing destRTG;
    RtgKey rKey;

    RoutingInfos routingInfos;
    Fare fare;
    FareInfo fareInfo;
    TariffCrossRefInfo tcrInfo;
    PaxType paxType;
    PaxTypeFare paxTypeFare1;

    fare.setFareInfo(&fareInfo);
    fare.setTariffCrossRefInfo(&tcrInfo);
    FareMarket fareMarket;

    paxTypeFare1.initialize(&fare, &paxType, &fareMarket, *_trx);

    RoutingControllerTestSetRtMapValidFlag routingController(*_trx);

    const DateTime travelDate = DateTime::localTime();

    bool result = routingController.processRoutingValidation(rKey,
                                                             *_trx,
                                                             tvlRoute,
                                                             &paxTypeFare1,
                                                             &specRTG,
                                                             &addonRTG,
                                                             &destRTG,
                                                             routingInfos,
                                                             travelDate);

    routingController.updatePaxTypeFareAccess(rKey, paxTypeFare1, routingInfos);

    CPPUNIT_ASSERT(result);

    // fail mileage, pass routing validation and set flag
    CPPUNIT_ASSERT(paxTypeFare1.isRoutingMapValid());
  }

  void createMap(Routing& routing,
                 int loc1No,
                 Indicator loctag,
                 int nextLocNo,
                 int altLocNo,
                 char loc1Type,
                 const char* loc1Code)
  {
    RoutingMap* map = new RoutingMap();
    map->vendor() = routing.vendor();
    map->carrier() = routing.carrier();
    map->routingTariff() = routing.routingTariff();
    map->routing() = routing.routing();
    map->effDate() = DateTime::localTime();
    map->lnkmapsequence() = loc1No;
    map->loc1No() = loc1No;
    map->loctag() = loctag;
    map->nextLocNo() = nextLocNo;
    map->altLocNo() = altLocNo;
    map->loc1().locType() = loc1Type;
    map->loc1().loc() = loc1Code;
    map->localRouting() = "";
    routing.rmaps().push_back(map);
  }

  void createTravelRouteAndTravelSeg(TravelRoute& travelRoute,
                                     const char* boardCity,
                                     const char* boardNation,
                                     const char* carrier,
                                     const char* offCity,
                                     const char* offNation,
                                     DateTime departureDT = DateTime::localTime())
  {
    createTravelRoute(
        travelRoute.travelRoute(), boardCity, boardNation, carrier, offCity, offNation);
    createTravelSeg(travelRoute.mileageTravelRoute(),
                    boardCity,
                    boardNation,
                    carrier,
                    offCity,
                    offNation,
                    departureDT);
  }

  void createTravelRoute(std::vector<TravelRoute::CityCarrier>& travelRoute,
                         const char* boardCity,
                         const char* boardNation,
                         const char* carrier,
                         const char* offCity,
                         const char* offNation)
  {
    TravelRoute::CityCarrier cc;
    cc.boardCity().loc() = boardCity;
    cc.boardCity().isHiddenCity() = false; // don't care really
    cc.offCity().loc() = offCity;
    cc.offCity().isHiddenCity() = false; // don't care really
    cc.carrier() = carrier;
    cc.stopover() = false; // don't care really
    cc.boardNation() = boardNation;
    cc.offNation() = offNation;
    travelRoute.push_back(cc);
  }

  void createTravelSeg(std::vector<TravelSeg*>& travelSeg,
                       const char* boardCity,
                       const char* boardNation,
                       const char* carrier,
                       const char* offCity,
                       const char* offNation,
                       DateTime departureDT)
  {
    AirSeg* as = _memHandle.create<AirSeg>();
    Loc* lc1 = _memHandle.create<Loc>();
    lc1->loc() = boardCity;
    lc1->nation() = boardNation;
    as->origin() = lc1;
    as->origAirport() = boardCity;
    Loc* lc2 = _memHandle.create<Loc>();
    lc2->loc() = offCity;
    lc2->nation() = offNation;
    as->destination() = lc2;
    as->destAirport() = offCity;
    as->carrier() = carrier;
    as->departureDT() = departureDT;
    travelSeg.push_back(as);
  }

  void setupTestGetRoutingsForSITAMileageAddon()
  {
    _fare->status().set(Fare::FS_ConstructedFare, true);
    _constructedFareInfo->origAddonRouting() = MILEAGE_ROUTING;
    _constructedFareInfo->destAddonRouting() = MILEAGE_ROUTING;
    _fareInfo->vendor() = Vendor::SITA;
    _tvlRoute->govCxr() = "AA";
    _fareMarket->governingCarrier() = "AA";
    _tariffCrossRef->routingTariff1() = 3;
  }

  void testGetRoutingsForSITAMileageOrigAddon()
  {
    const Routing* routing = 0;
    const Routing* origAddOnRouting = 0;
    const Routing* destAddOnRouting = 0;

    setupTestGetRoutingsForSITAMileageAddon();
    _constructedFareInfo->constructionType() = ConstructedFareInfo::SINGLE_ORIGIN;

    _fareMarket->travelSeg().clear();

    createTravelSeg(
        _fareMarket->travelSeg(), "DFW", "US", "AA", "STL", "US", DateTime::localTime());

    _paxTypeFare->setRuleData(
        RuleConst::FARE_BY_RULE, _trx->dataHandle(), &_fbrPaxTypeFareRuleData);

    _mockedRtgController->getRoutings(
        *_trx, *_paxTypeFare, routing, origAddOnRouting, destAddOnRouting);

    CPPUNIT_ASSERT(origAddOnRouting);
    CPPUNIT_ASSERT_EQUAL(_paxTypeFare->vendor(), origAddOnRouting->vendor());
    CPPUNIT_ASSERT_EQUAL(_tvlRoute->govCxr(), origAddOnRouting->carrier());
    CPPUNIT_ASSERT_EQUAL(_paxTypeFare->tcrRoutingTariff1(), origAddOnRouting->routingTariff());
    CPPUNIT_ASSERT_EQUAL(MILEAGE_ROUTING, origAddOnRouting->routing());
  }

  void testGetRoutingsForSITAMileageDestAddon()
  {
    const Routing* routing = 0;
    const Routing* origAddOnRouting = 0;
    const Routing* destAddOnRouting = 0;

    setupTestGetRoutingsForSITAMileageAddon();
    _constructedFareInfo->constructionType() = ConstructedFareInfo::SINGLE_DESTINATION;

    _fareMarket->travelSeg().clear();

    createTravelSeg(
        _fareMarket->travelSeg(), "DFW", "US", "AA", "STL", "US", DateTime::localTime());

    _mockedRtgController->getRoutings(
        *_trx, *_paxTypeFare, routing, origAddOnRouting, destAddOnRouting);

    CPPUNIT_ASSERT(destAddOnRouting);
    CPPUNIT_ASSERT_EQUAL(_paxTypeFare->vendor(), destAddOnRouting->vendor());
    CPPUNIT_ASSERT_EQUAL(_tvlRoute->govCxr(), destAddOnRouting->carrier());
    CPPUNIT_ASSERT_EQUAL(_paxTypeFare->tcrRoutingTariff1(), destAddOnRouting->routingTariff());
    CPPUNIT_ASSERT_EQUAL(MILEAGE_ROUTING, destAddOnRouting->routing());
  }

  void testGetRoutingsNoApplicationForATPCOMileageAddon()
  {
    const Routing* routing = 0;
    const Routing* origAddOnRouting = 0;
    const Routing* destAddOnRouting = 0;

    setupTestGetRoutingsForSITAMileageAddon();
    _constructedFareInfo->constructionType() = ConstructedFareInfo::SINGLE_DESTINATION;
    _fareInfo->vendor() = Vendor::ATPCO;

    _fareMarket->travelSeg().clear();

    createTravelSeg(
        _fareMarket->travelSeg(), "DFW", "US", "AA", "STL", "US", DateTime::localTime());

    _mockedRtgController->getRoutings(
        *_trx, *_paxTypeFare, routing, origAddOnRouting, destAddOnRouting);

    CPPUNIT_ASSERT(!destAddOnRouting);
  }

  void testProcessRestrictions_matchKeyLogic()
  {
    RoutingController::RestrictionValidity validRestrictions;
    RoutingController::RestrictionKey key1("1", "KRK", "LON", false);
    RoutingController::RestrictionKey key2("1", "KRK", "LHR", false);
    RoutingController::RestrictionKey key3("1", "KTW", "LON", false);
    RoutingController::RestrictionKey key4("1", "KRK", "LON", false);
    RoutingController::RestrictionKey key5("1", "LON", "KRK", false);
    RoutingController::RestrictionKey key6("2", "KRK", "LON", false);
    validRestrictions.insert(RoutingController::RestrictionValidity::value_type(key1, true));
    CPPUNIT_ASSERT(validRestrictions.find(key2) == validRestrictions.end());
    CPPUNIT_ASSERT(validRestrictions.find(key3) == validRestrictions.end());
    CPPUNIT_ASSERT(validRestrictions.find(key4) != validRestrictions.end());
    CPPUNIT_ASSERT(validRestrictions.find(key5) == validRestrictions.end());
    CPPUNIT_ASSERT(validRestrictions.find(key6) == validRestrictions.end());
  }
  void testProcessRestrictions_matchKeyLogicWithFlip()
  {
    RoutingController::RestrictionValidity validRestrictions;
    RoutingController::RestrictionKey key1("1", "KRK", "LON", true);
    RoutingController::RestrictionKey key2("1", "KRK", "LHR", true);
    RoutingController::RestrictionKey key3("1", "KTW", "LON", true);
    RoutingController::RestrictionKey key4("1", "KRK", "LON", true);
    RoutingController::RestrictionKey key5("1", "LON", "KRK", true);
    RoutingController::RestrictionKey key6("2", "KRK", "LON", true);
    validRestrictions.insert(RoutingController::RestrictionValidity::value_type(key1, true));
    CPPUNIT_ASSERT(validRestrictions.find(key2) == validRestrictions.end());
    CPPUNIT_ASSERT(validRestrictions.find(key3) == validRestrictions.end());
    CPPUNIT_ASSERT(validRestrictions.find(key4) != validRestrictions.end());
    CPPUNIT_ASSERT(validRestrictions.find(key5) != validRestrictions.end());
    CPPUNIT_ASSERT(validRestrictions.find(key6) == validRestrictions.end());
  }

  RoutingRestriction* createRestriction(RestrictionNumber restNo, Indicator negViaAppl)
  {
    RoutingRestriction* rest = _memHandle.create<RoutingRestriction>();
    rest->restriction() = restNo;
    rest->negViaAppl() = negViaAppl;
    return rest;
  }

  TravelRoute::CityCarrier
  prepareCityCarrier(LocCode bordCity, LocCode offCity, CarrierCode carrierCode = "AA",
                     NationCode offNation = "", NationCode boardNation = "")
  {
    TravelRoute::City boardCty, offCty;
    boardCty.loc() = bordCity;
    boardCty.isHiddenCity() = false;
    offCty.loc() = offCity;
    offCty.isHiddenCity() = false;

    TravelRoute::CityCarrier cityCarrier;
    cityCarrier.boardCity() = boardCty;
    cityCarrier.carrier() = carrierCode;
    cityCarrier.offCity() = offCty;

    cityCarrier.offNation() = offNation;
    cityCarrier.boardNation() = boardNation;

    return cityCarrier;
  }

  RoutingRestriction* prepareRestriction(int seqNo, RestrictionNumber res, Indicator marketAppl,
                                        Indicator nonStopDirectInd, Indicator airSurfaceInd,
                                        LocCode market1, LocCode market2, LocCode viaMarket,
                                        LocCode viaCarrier, Indicator viaType = 'C',
                                        Indicator market1type = 'C', Indicator market2type = 'C')
  {
      RoutingRestriction* rest = new RoutingRestriction();
      rest->restrSeqNo() = seqNo;
      rest->restriction() = res;
      rest->marketAppl() = marketAppl; // B=Between City1 and City2, T=To/From City1
      rest->viaType() = viaType; // A=Airline, C=City, Blank=Not Applicable
      rest->nonStopDirectInd() = nonStopDirectInd; // N=Nonstop, D=Direct, E=Either Nonstop or Direct
      rest->airSurfaceInd() = airSurfaceInd; // A=Air, S=Surface, E=Either Air or Surface, Blank=Not Applicable
      rest->market1() = market1; // City1
      rest->market2() = market2; // City2
      rest->viaMarket() = viaMarket; // Via City
      rest->viaCarrier() = viaCarrier; // Via Carrier
      rest->market1type() = market1type;  // A=Airline, C=City, Blank=Not Applicable
      rest->market2type() = market2type; // A=Airline, C=City, Blank=Not Applicable
      rest->negViaAppl() = REQUIRED;

      return rest;
  }

  void testisRestSeqUseAndLogic_AllNeg()
  {
    std::vector<RoutingRestriction*> rests;
    rests.push_back(createRestriction("02", 'N'));
    rests.push_back(createRestriction("02", 'N'));
    rests.push_back(createRestriction("02", 'N'));
    CPPUNIT_ASSERT_EQUAL(true, _mockedRtgController->isRestSeqUseAndLogic(rests));
  }
  void testisRestSeqUseAndLogic_Mixed()
  {
    std::vector<RoutingRestriction*> rests;
    rests.push_back(createRestriction("02", 'R'));
    rests.push_back(createRestriction("02", 'P'));
    rests.push_back(createRestriction("02", 'N'));
    CPPUNIT_ASSERT_EQUAL(true, _mockedRtgController->isRestSeqUseAndLogic(rests));
  }
  void testisRestSeqUseAndLogic_AllPositive()
  {
    std::vector<RoutingRestriction*> rests;
    rests.push_back(createRestriction("02", 'R'));
    rests.push_back(createRestriction("02", 'P'));
    rests.push_back(createRestriction("02", 'R'));
    CPPUNIT_ASSERT_EQUAL(false, _mockedRtgController->isRestSeqUseAndLogic(rests));
  }

  void testProcessPaxTypeFareWhenUseTvlRouteWithoutHiddenPts()
  {
    RoutingControllerProcessPaxTypeFareMock* pptRtgCtrl =
        static_cast<RoutingControllerProcessPaxTypeFareMock*>(_pptRtgCtrl);
    pptRtgCtrl->_routing.unticketedPointInd() = TKTPTS_TKTONLY;

    TravelRoute tvlRouteTktOnly;
    _tvlRoute->travelRouteTktOnly() = &tvlRouteTktOnly;

    FareMarket fareMarket;
    RoutingInfos routingInfos;
    RtgKeyMap rtMap;

    const Routing* origAddOnRouting = 0;
    const Routing* destAddOnRouting = 0;

    _pptRtgCtrl->processPaxTypeFare(*_paxTypeFare,
                                    *_tvlRoute,
                                    fareMarket,
                                    routingInfos,
                                    rtMap,
                                    &pptRtgCtrl->_routing,
                                    origAddOnRouting,
                                    destAddOnRouting);

    CPPUNIT_ASSERT_EQUAL(size_t(1), rtMap.size());
    CPPUNIT_ASSERT_EQUAL(&tvlRouteTktOnly, pptRtgCtrl->_passedTravelRoute);
  }

  void testValidateMileageSurchargeExceptionOldImplementation()
  {
    fallback::fixed::value::fallbackMileageSurchargeExceptionValidation.set(true);

    CPPUNIT_ASSERT(!_pptRtgCtrl->validateMileageSurchargeException(
        *_shoppingTrx, *_paxTypeFare, *_tvlRoute, *_rtgInfo));
  }

  void testValidateMileageSurchargeExceptionNewImplementation()
  {
    fallback::fixed::value::fallbackMileageSurchargeExceptionValidation.set(false);

    CPPUNIT_ASSERT(_pptRtgCtrl->validateMileageSurchargeException(
        *_shoppingTrx, *_paxTypeFare, *_tvlRoute, *_rtgInfo));
  }

  void testValidateRestriction16_RtwPass()
  {
    RoutingRestriction restriction16;
    restriction16.restriction() = "16";
    _trx->getOptions()->setRtw(true);

    CPPUNIT_ASSERT(_pptRtgCtrl->validateRestriction(*_trx, *_paxTypeFare, restriction16,
                                                    *_tvlRoute, _rtgInfo));
  }

  void testValidateRestriction17_Pass()
  {
    TravelRoute::CityCarrier cityCarrier;
    cityCarrier.carrier() = "OK";

    std::set<CarrierCode> cxrSet;
    cxrSet.insert("OK");
    std::vector<const RoutingRestriction*> list17;
    list17.push_back(createRestriction("17", REQUIRED));
    _tvlRoute->travelRoute().push_back(cityCarrier);

    CPPUNIT_ASSERT(_pptRtgCtrl->validateRestriction17(*_tvlRoute, *_trx, _rtgInfo, cxrSet, list17));
  }

  void testValidateRestriction17_Fail()
  {
    TravelRoute::CityCarrier cityCarrier;
    cityCarrier.carrier() = "C1";

    std::set<CarrierCode> cxrSet;
    cxrSet.insert("C2");
    std::vector<const RoutingRestriction*> list17;
    list17.push_back(createRestriction("17", REQUIRED));
    _tvlRoute->travelRoute().push_back(cityCarrier);

    CPPUNIT_ASSERT(!_pptRtgCtrl->validateRestriction17(*_tvlRoute, *_trx, _rtgInfo, cxrSet, list17));
  }

  void testValidateRestriction17_PassSurface()
  {
    TravelRoute::CityCarrier cityCarrier;
    cityCarrier.carrier() = SURFACE_CARRIER;

    std::set<CarrierCode> cxrSet;
    cxrSet.insert("C2");
    std::vector<const RoutingRestriction*> list17;
    list17.push_back(createRestriction("17", REQUIRED));
    _tvlRoute->travelRoute().push_back(cityCarrier);

    CPPUNIT_ASSERT(_pptRtgCtrl->validateRestriction17(*_tvlRoute, *_trx, _rtgInfo, cxrSet, list17));
  }

  void testValidateRestriction17_PassRtw()
  {
    TravelRoute::CityCarrier cityCarrier;
    cityCarrier.carrier() = "C1";
    _trx->getOptions()->setRtw(true);

    std::set<CarrierCode> cxrSet;
    cxrSet.insert(ONE_WORLD_ALLIANCE);
    std::vector<const RoutingRestriction*> list17;
    list17.push_back(createRestriction("17", REQUIRED));
    _tvlRoute->travelRoute().push_back(cityCarrier);

    CPPUNIT_ASSERT(_pptRtgCtrl->validateRestriction17(*_tvlRoute, *_trx, _rtgInfo, cxrSet, list17));
  }

  void testRestriction12_noRTW()
  {
    _trx->getOptions()->setRtw(false);

    MockRoutingController routingController(*_trx);
    Routing routing;
    routing.rests().push_back(prepareRestriction(1, "12", ' ', ' ', ' ', "", "", WestCoastCode, "", 'C'));
    RoutingController::RoutingComponent rtgComponent=0;

    CPPUNIT_ASSERT(routingController.processAddOnRestrictions(&routing, *_paxTypeFare, *_trx,
        *_tvlRoute, _rtgInfo, rtgComponent));
  }

  void testRestriction12_singleNonStop()
  {
    _trx->getOptions()->setRtw(true);

    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("LAX", "WAS"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("WAS", "LON"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("LON", "ROM"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ROM", "DXB"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("DXB", "SYD"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("SYD", "LAX"));

    MockRoutingController routingController(*_trx);
    Routing routing;
    routing.rests().push_back(prepareRestriction(1, "12", ' ', ' ', ' ', "LAX", "WAS", "", "", 'C'));
    RoutingController::RoutingComponent rtgComponent=0;

    CPPUNIT_ASSERT(routingController.processAddOnRestrictions(&routing, *_paxTypeFare, *_trx,
        tvlRoute, _rtgInfo, rtgComponent));
  }

  void testRestriction12_multiNonStop()
  {
    _trx->getOptions()->setRtw(true);

    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("LAX", "WAS"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("WAS", "LON"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("LON", "ROM"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ROM", "DXB"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("DXB", "SYD"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("SYD", "WAS"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("WAS", "LAX"));

    MockRoutingController routingController(*_trx);
    Routing routing;
    routing.rests().push_back(prepareRestriction(1, "12", ' ', ' ', ' ', "LAX", "WAS", "", "", 'C'));
    RoutingController::RoutingComponent rtgComponent=0;

    CPPUNIT_ASSERT(!routingController.processAddOnRestrictions(&routing, *_paxTypeFare, *_trx,
        tvlRoute, _rtgInfo, rtgComponent));
  }

  void testRestriction12_multiRest12()
  {
    _trx->getOptions()->setRtw(true);

    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("LAX", "NYC"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("NYC", "WAS"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("WAS", "LON"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("LON", "ROM"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ROM", "DXB"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("DXB", "SYD"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("SYD", "WAS"));

    MockRoutingController routingController(*_trx);
    Routing routing;
    routing.rests().push_back(prepareRestriction(1, "12", ' ', ' ', ' ', "LAX", "NYC", "", "", 'C'));
    routing.rests().push_back(prepareRestriction(1, "12", ' ', ' ', ' ', "WAS", "NYC", "", "", 'C'));
    RoutingController::RoutingComponent rtgComponent=0;

    CPPUNIT_ASSERT(!routingController.processAddOnRestrictions(&routing, *_paxTypeFare, *_trx,
        tvlRoute, _rtgInfo, rtgComponent));
  }

  void testRestriction12_nation()
  {
    _trx->getOptions()->setRtw(true);

    TravelRoute tvlRoute;
    tvlRoute.travelRoute().push_back(prepareCityCarrier("LAX", "WAS", "AA", "US", "US"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("WAS", "LON", "AA", "UK", "US"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("LON", "ROM", "AA", "IT", "UK"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("ROM", "DXB", "AA", "UE", "IT"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("DXB", "SYD", "AA", "AU", "UE"));
    tvlRoute.travelRoute().push_back(prepareCityCarrier("SYD", "LAX", "AA", "US", "AU"));

    MockRoutingController routingController(*_trx);
    Routing routing;
    routing.rests().push_back(prepareRestriction(1, "12", ' ', ' ', ' ', "US", "US", "US", "", 'N', 'N', 'N'));
    RoutingController::RoutingComponent rtgComponent=0;

    CPPUNIT_ASSERT(routingController.processAddOnRestrictions(&routing, *_paxTypeFare, *_trx,
        tvlRoute, _rtgInfo, rtgComponent));
  }

  void testRestriction12_zone()
  {
    _trx->getOptions()->setRtw(true);

     TravelRoute tvlRoute;
     tvlRoute.travelRoute().push_back(prepareCityCarrier("LAX", "WAS"));
     tvlRoute.travelRoute().push_back(prepareCityCarrier("WAS", "LON"));
     tvlRoute.travelRoute().push_back(prepareCityCarrier("LON", "ROM"));
     tvlRoute.travelRoute().push_back(prepareCityCarrier("ROM", "DXB"));
     tvlRoute.travelRoute().push_back(prepareCityCarrier("DXB", "SYD"));
     tvlRoute.travelRoute().push_back(prepareCityCarrier("SYD", "WAS"));
     tvlRoute.travelRoute().push_back(prepareCityCarrier("WAS", "LAX"));

     MockRoutingController routingController(*_trx);
     Routing routing;
     routing.rests().push_back(prepareRestriction(1, "12", ' ', ' ', ' ', "210", "210", "", "", 'Z', 'Z', 'Z'));
     RoutingController::RoutingComponent rtgComponent=0;

     CPPUNIT_ASSERT(routingController.processAddOnRestrictions(&routing, *_paxTypeFare, *_trx,
         tvlRoute, _rtgInfo, rtgComponent));
  }

  void testRestriction12_genericCityCode()
  {
    _trx->getOptions()->setRtw(true);

     TravelRoute tvlRoute;
     tvlRoute.travelRoute().push_back(prepareCityCarrier("LAX", "WAS", "AA", "USVA", "USCA"));
     tvlRoute.travelRoute().push_back(prepareCityCarrier("WAS", "LON", "AA", "UK**", "USVA"));
     tvlRoute.travelRoute().push_back(prepareCityCarrier("LON", "ROM", "AA", "IT**", "UK**"));
     tvlRoute.travelRoute().push_back(prepareCityCarrier("ROM", "SYD", "AA", "UE**", "UE**"));
     tvlRoute.travelRoute().push_back(prepareCityCarrier("SYD", "LAX", "AA", "USCA", "AU**"));

     MockRoutingController routingController(*_trx);
     Routing routing;
     routing.rests().push_back(prepareRestriction(1, "12", ' ', ' ', ' ', WestCoastCode,
          EastCoastCode, "", "", 'C'));
     RoutingController::RoutingComponent rtgComponent=0;

     CPPUNIT_ASSERT(routingController.processAddOnRestrictions(&routing, *_paxTypeFare, *_trx,
         tvlRoute, _rtgInfo, rtgComponent));
  }


private:
  DataHandleMock* _myDataHandle;
  PricingTrx* _trx;
  RoutingController* _rtC;
  RoutingController* _mockedRtgController;
  TravelRoute* _tvlRoute;
  PaxTypeFare* _paxTypeFare;
  FareMarket* _fareMarket;
  FBRPaxTypeFareRuleData _fbrPaxTypeFareRuleData;
  FareByRuleItemInfo _fbrItemInfo;
  Fare* _fare;
  ConstructedFareInfo* _constructedFareInfo;
  FareInfo* _fareInfo;
  TariffCrossRefInfo* _tariffCrossRef;
  TestMemHandle _memHandle;
  RoutingController* _pptRtgCtrl;
  Diversity* _diversity;
  ShoppingTrx* _shoppingTrx;
  RoutingInfo *_rtgInfo;
  RoutingInfos* _routingInfos;
  Routing* _routing;
};

CPPUNIT_TEST_SUITE_REGISTRATION(RoutingControllerTest);
} // namespace tse
