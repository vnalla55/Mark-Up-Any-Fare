#include <iostream>
#include <vector>
#include "test/include/CppUnitHelperMacros.h"
#include "Common/TseEnums.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingOptions.h"
#include "DBAccess/Loc.h"
#include "DataModel/SurfaceSeg.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/MileageTrx.h"
#include "DataModel/PricingTrx.h"
#include "Routing/MileageRouteItem.h"
#include "Routing/MileageRouteBuilder.h"
#include "Routing/MileageRoute.h"
#include "DBAccess/Customer.h"
#include "DBAccess/Mileage.h"
#include "DBAccess/MultiTransport.h"
#include "Routing/MileageValidator.h"
#include "DBAccess/DataHandle.h"
#include "Common/DateTime.h"
#include "Routing/Collector.h"
#include "Routing/TPMCollectorWN.h"
#include "Routing/MPMCollectorWN.h"
#include "Routing/MPMCollectorMV.h"
#include "Routing/TPMCollectorMV.h"
#include "Routing/TravelRoute.h"
#include "Routing/RoutingInfo.h"
#include "Routing/FareCalcVisitor.h"
#include "test/include/MockGlobal.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"

namespace tse
{

class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;
  Mileage* getMil(int mileage, GlobalDirection gd)
  {
    Mileage* ret = _memHandle.create<Mileage>();
    ret->mileage() = mileage;
    ret->globaldir() = gd;
    return ret;
  }
  MultiTransport* getMC(LocCode city, LocCode loc)
  {
    MultiTransport* ret = _memHandle.create<MultiTransport>();
    ret->multitranscity() = city;
    ret->multitransLoc() = loc;
    return ret;
  }

public:
  const Mileage* getMileage(const LocCode& origin,
                            const LocCode& dest,
                            Indicator mileageType,
                            const GlobalDirection globalDir,
                            const DateTime& date)
  {
    if ((origin == "BUE" && dest == "LON" && mileageType == 'M') ||
        (origin == "LON" && dest == "BUE" && mileageType == 'M'))
      return getMil(8341, GlobalDirection::AT);
    else if (origin == "BUE" && dest == "LON" && mileageType == 'T')
      return getMil(6951, GlobalDirection::AT);
    else if (origin == "DEL" && dest == "BOM" && mileageType == 'T')
      return getMil(708, GlobalDirection::EH);
    else if (origin == "DEL" && dest == "BOM" && mileageType == 'M')
      return 0;
    else if (origin == "DEL" && dest == "BUE" && mileageType == 'M')
      return getMil(12738, GlobalDirection::AT);
    else if (origin == "DEL" && dest == "EZE" && mileageType == 'M')
      return 0;
    else if (origin == "DEL" && dest == "JFK" && mileageType == 'M')
      return 0;
    else if (origin == "DEL" && dest == "NYC" && mileageType == 'M')
      return getMil(9558, GlobalDirection::AT);
    else if (origin == "DEL" && dest == "SAO" && mileageType == 'M')
      return getMil(11473, GlobalDirection::AT);
    else if (origin == "DFW" && dest == "MAN" && mileageType == 'M')
      return getMil(5539, GlobalDirection::AT);
    else if (origin == "DFW" && dest == "MAN" && mileageType == 'T')
      return 0;
    else if (origin == "JFK")
      return 0;
    else if (origin == "LON" && dest == "FRA" && mileageType == 'M')
      return getMil(475, GlobalDirection::EH);
    else if (origin == "LON" && dest == "FRA" && mileageType == 'T')
      return getMil(396, GlobalDirection::EH);
    else if (origin == "MAN" && dest == "FRA" && mileageType == 'M')
      return getMil(621, GlobalDirection::EH);
    else if (origin == "MAN" && dest == "FRA" && mileageType == 'T')
      return getMil(518, GlobalDirection::EH);
    else if (origin == "MIA" && dest == "RIO" && mileageType == 'M')
      return getMil(5014, GlobalDirection::WH);
    else if ((origin == "MIA" && dest == "RIO" && mileageType == 'T') ||
             (origin == "RIO" && dest == "MIA" && mileageType == 'T'))
      return getMil(4179, GlobalDirection::WH);
    else if (origin == "NYC" && dest == "DFW" && mileageType == 'M')
      return 0;
    else if (origin == "NYC" && dest == "DFW" && mileageType == 'T')
      return getMil(1378, GlobalDirection::WH);
    else if (origin == "NYC" && dest == "FRA" && mileageType == 'M')
      return getMil(4621, GlobalDirection::AT);
    else if ((origin == "PIT" && dest == "RIO" && mileageType == 'M') ||
             (origin == "RIO" && dest == "PIT" && mileageType == 'M'))
      return getMil(5995, GlobalDirection::WH);
    else if (origin == "TYO" && dest == "AMS" && mileageType == 'M')
      return getMil(9592, GlobalDirection::EH);
    else if (origin == "TYO" && dest == "BJS" && mileageType == 'M')
      return getMil(1575, GlobalDirection::EH);
    else if (origin == "TYO" && dest == "FRA" && mileageType == 'M')
      return getMil(7113, GlobalDirection::TS);
    else if (origin == "TYO" && dest == "LON" && mileageType == 'M')
      return getMil(7464, GlobalDirection::TS);
    else if (origin == "TYO" && dest == "LON" && mileageType == 'T')
      return getMil(6220, GlobalDirection::TS);
    else if (origin == "TYO" && dest == "SHA" && mileageType == 'M')
      return getMil(1333, GlobalDirection::EH);

    return DataHandleMock::getMileage(origin, dest, mileageType, globalDir, date);
  }
  const std::vector<Mileage*>& getMileage(const LocCode& origin,
                                          const LocCode& dest,
                                          const DateTime& date,
                                          Indicator mileageType = 'T')
  {
    std::vector<Mileage*>& ret = *_memHandle.create<std::vector<Mileage*> >();
    if (origin == "BJS" && dest == "SHA")
    {
      if (mileageType == 'T')
        ret.push_back(getMil(676, GlobalDirection::EH));
      return ret;
    }
    else if (origin == "BOM" && dest == "JFK")
      return ret;
    else if (origin == "DEL" && dest == "BOM")
    {
      if (mileageType == 'T')
        ret.push_back(getMil(708, GlobalDirection::EH));
      return ret;
    }
    else if (origin == "EZE" && dest == "SAO")
      return ret;
    else if (origin == "FRA" && dest == "AMS" && mileageType == 'M')
    {
      ret.push_back(getMil(273, GlobalDirection::EH));
      return ret;
    }
    else if (origin == "FRA" && dest == "AMS" && mileageType == 'T')
    {
      ret.push_back(getMil(228, GlobalDirection::EH));
      return ret;
    }
    else if (origin == "JFK" && dest == "EZE")
      return ret;
    else if (origin == "SHA" && dest == "FRA" && mileageType == 'M')
    {
      ret.push_back(getMil(12746, GlobalDirection::AP));
      ret.push_back(getMil(7767, GlobalDirection::EH));
      return ret;
    }
    else if (origin == "SHA" && dest == "FRA" && mileageType == 'T')
    {
      ret.push_back(getMil(5495, GlobalDirection::EH));
      return ret;
    }
    else if (origin == "TYO" && dest == "BJS" && mileageType == 'M')
    {
      ret.push_back(getMil(1575, GlobalDirection::EH));
      return ret;
    }
    else if (origin == "TYO" && dest == "BJS" && mileageType == 'T')
    {
      ret.push_back(getMil(1313, GlobalDirection::EH));
      return ret;
    }

    return DataHandleMock::getMileage(origin, dest, date, mileageType);
  }
  const LocCode getMultiTransportCityCode(const LocCode& locCode,
                                          const CarrierCode& carrierCode,
                                          GeoTravelType tvlType,
                                          const DateTime& tvlDate)
  {
    if (locCode == "FRA")
      return "FRA";
    else if (locCode == "JFK")
      return "NYC";
    else if (locCode == "MAN")
      return "MAN";
    else if (locCode == "SHA")
      return "SHA";
    return DataHandleMock::getMultiTransportCityCode(locCode, carrierCode, tvlType, tvlDate);
  }
  const std::vector<MultiTransport*>& getMultiTransportCity(const LocCode& locCode,
                                                            const CarrierCode& carrierCode,
                                                            GeoTravelType tvlType,
                                                            const DateTime& tvlDate)
  {
    std::vector<MultiTransport*>& ret = *_memHandle.create<std::vector<MultiTransport*> >();
    if (locCode == "AMS")
      return ret;
    else if (locCode == "BJS")
      return ret;
    else if (locCode == "BOM")
      return ret;
    else if (locCode == "DEL")
      return ret;
    else if (locCode == "EZE")
    {
      ret.push_back(getMC("BUE", "EZE"));
      return ret;
    }
    else if (locCode == "FRA")
    {
      ret.push_back(getMC("FRA", "FRA"));
      return ret;
    }
    else if (locCode == "JFK")
    {
      ret.push_back(getMC("NYC", "JFK"));
      return ret;
    }
    if (locCode == "SAO")
      return ret;
    else if (locCode == "SHA")
    {
      ret.push_back(getMC("SHA", "SHA"));
      return ret;
    }
    else if (locCode == "TYO")
      return ret;
    else if (locCode == "LON")
    {
      ret.push_back(getMC("LON", "LCY"));
      ret.push_back(getMC("LON", "LGW"));
      ret.push_back(getMC("LON", "LHR"));
      ret.push_back(getMC("LON", "LTN"));
      ret.push_back(getMC("LON", "QQP"));
      ret.push_back(getMC("LON", "QQS"));
      ret.push_back(getMC("LON", "QQU"));
      ret.push_back(getMC("LON", "QQW"));
      ret.push_back(getMC("LON", "STN"));
      ret.push_back(getMC("LON", "XQE"));
      return ret;
    }
    else if (locCode == "DFW")
    {
      ret.push_back(getMC("DFW", "DAL"));
      ret.push_back(getMC("DFW", "DFW"));
      ret.push_back(getMC("DFW", "QDF"));
      return ret;
    }
    else if (locCode == "MAN")
    {
      ret.push_back(getMC("MAN", "MAN"));
      ret.push_back(getMC("MAN", "QQM"));
      return ret;
    }

    return DataHandleMock::getMultiTransportCity(locCode, carrierCode, tvlType, tvlDate);
  }
  const MileageSubstitution* getMileageSubstitution(const LocCode& key, const DateTime& date)
  {
    if (key == "BOM" || key == "JFK" || key == "DEL" || key == "DFW" || key == "EZE" ||
        key == "MAN" || key == "SAO")
      return 0;
    return DataHandleMock::getMileageSubstitution(key, date);
  }
  const TariffMileageAddon* getTariffMileageAddon(const CarrierCode& carrier,
                                                  const LocCode& unpublishedAddonLoc,
                                                  const GlobalDirection& globalDir,
                                                  const DateTime& date)
  {
    if (unpublishedAddonLoc == "BOM" || unpublishedAddonLoc == "DEL")
      return 0;
    return DataHandleMock::getTariffMileageAddon(carrier, unpublishedAddonLoc, globalDir, date);
  }
  const SurfaceSectorExempt*
  getSurfaceSectorExempt(const LocCode& origLoc, const LocCode& destLoc, const DateTime& date)
  {
    if (origLoc == "BOM" || origLoc == "JFK" || origLoc == "NYC" || origLoc == "EZE" ||
        origLoc == "BUE" || origLoc == "SHA" || origLoc == "FRA")
      return 0;
    else if (origLoc == "BJS" && destLoc == "SHA")
      return 0;
    return DataHandleMock::getSurfaceSectorExempt(origLoc, destLoc, date);
  }
  const std::vector<TpdPsr*>& getTpdPsr(Indicator applInd,
                                        const CarrierCode& carrier,
                                        Indicator area1,
                                        Indicator area2,
                                        const DateTime& date)
  {
    if (applInd == 'T' && carrier == "AA" && area1 == '1' && area2 == '2')
      return *_memHandle.create<std::vector<TpdPsr*> >();
    else if (applInd == 'P' && carrier == "YY" && area1 == '3')
      return *_memHandle.create<std::vector<TpdPsr*> >();
    else if (carrier == "BA" && area2 == '2')
      return *_memHandle.create<std::vector<TpdPsr*> >();
    return DataHandleMock::getTpdPsr(applInd, carrier, area1, area2, date);
  }
  const std::vector<TPMExclusion*>& getTPMExclus(const CarrierCode& carrier)
  {
    if (carrier == "BA")
      return *_memHandle.create<std::vector<TPMExclusion*> >();
    return DataHandleMock::getTPMExclus(carrier);
  }
};

class MockMileageValidator : public MileageValidator
{
public:
  MockMileageValidator() {};
  virtual ~MockMileageValidator() {};
  virtual bool getTPD(MileageRoute& mRoute, FareCalcVisitor& visitor) const { return true; }
};

class MileageValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MileageValidatorTest);
  CPPUNIT_TEST(testvalidateFalse);
  CPPUNIT_TEST(testvalidateTrue);
  CPPUNIT_TEST(testwithMileageEqualization);
  CPPUNIT_TEST(testwithMileageEqualizationErrorCondition);
  CPPUNIT_TEST(testcalculateMileage);
  CPPUNIT_TEST(testfillInDiagnosticInfo);
  CPPUNIT_TEST(testwithSouthAtlantic);
  CPPUNIT_TEST(testcalculateMileageForSAException);

  CPPUNIT_TEST_SUITE_END();

  // helper methods
public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandle>();
  }

  void tearDown() { _memHandle.clear(); }

  // tests
public:
  void testvalidateTrue()
  {
    // create some Loc objects, we need real objecst with lattitude and longitude
    // Hence using dataHandle to get these out of the database.
    DataHandle dataHandle;
    const Loc* loc1 = dataHandle.getLoc("TYO", DateTime::localTime());
    const Loc* loc2 = dataHandle.getLoc("LON", DateTime::localTime());
    const Loc* loc3 = dataHandle.getLoc("FRA", DateTime::localTime());
    MileageRoute mRoute;
    AirSeg as1, as2;
    SurfaceSeg ss;
    TravelRoute tvlRoute;
    // create AirSegs

    as1.origin() = const_cast<Loc*>(loc1);
    as1.destination() = const_cast<Loc*>(loc2);
    as1.boardMultiCity() = "TYO";
    as1.offMultiCity() = "LON";
    as1.carrier() = "AA";

    as2.origin() = const_cast<Loc*>(loc2);
    as2.destination() = const_cast<Loc*>(loc3);
    as2.boardMultiCity() = "LON";
    as2.offMultiCity() = "FRA";
    as2.carrier() = "BA";

    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.push_back(&as1);
    tvlSegs.push_back(&as2);
    tvlRoute.mileageTravelRoute() = tvlSegs;

    tvlRoute.globalDir() = GlobalDirection::TS;
    tvlRoute.govCxr() = "BA";
    tvlRoute.travelDate() = DateTime::localTime();

    DateTime ticketingDT = DateTime::localTime();

    PricingRequest request;
    request.ticketingDT() = DateTime::localTime();

    PricingTrx trx;
    trx.setRequest(&request);
    trx.setOptions(_memHandle.create<PricingOptions>());

    MileageRouteBuilder mBuilder;
    bool rc = mBuilder.buildMileageRoute(trx, tvlRoute, mRoute, dataHandle, ticketingDT);
    CPPUNIT_ASSERT(rc);
    CPPUNIT_ASSERT(mRoute.mileageRouteItems().size() == 2);

    MileageInfo mInfo;
    MileageValidator mValidator;
    // const Mileage* mil = trx.dataHandle().getMileage("FAR", "TYO", 'M', DateTime::localTime());
    mValidator.validate(trx, mInfo, tvlRoute);

    /*MileageRouteItems::iterator itr(mRoute.mileageRouteItems().begin());
    MileageRouteItems::iterator end(mRoute.mileageRouteItems().end());
    for(; itr!= end; ++itr)
    {
      std::cout<< " " <<std::endl;
      std::cout<< itr->city1()->loc() <<std::endl;
      std::cout<< itr->city2()->loc() <<std::endl;
      std::string MPMglobalDirectionStr;
      std::string TPMglobalDirectionStr;
      globalDirectionToStr(TPMglobalDirectionStr, itr->tpmGlobalDirection());
      globalDirectionToStr(MPMglobalDirectionStr, itr->mpmGlobalDirection());
      std::cout<< " MPM GlobalDir "   << MPMglobalDirectionStr << std::endl;
      std::cout<< " TPM GlobalDir "   << TPMglobalDirectionStr << std::endl;
      std::cout<< " Segment Carrier " << itr->segmentCarrier() << std::endl;
      std::cout<< " TPM        "     << itr->tpm() << std::endl;
      std::cout<< " MPM        "     << itr->mpm() << std::endl;
      std::cout<<"------------------------------"<<std::endl;
    }

    std::cout<<"" <<std::endl;
    std::cout<<"--------MILEAGE--------------"<<std::endl;
    std::cout<< "  TPM        "     << mInfo.totalApplicableTPM() << std::endl;
    std::cout<< "  MPM        "     << mInfo.totalApplicableMPM()   << std::endl;
    std::cout<< "  EMS        "     << mInfo.surchargeAmt()   << std::endl;
    std::cout<<"------------------------------"<<std::endl; */

    CPPUNIT_ASSERT(mInfo.valid());
  }

  void testvalidateFalse()
  {

    // create some Loc objects, we need real objecst with lattitude and longitude
    // Hence using dataHandle to get these out of the database.
    DataHandle dataHandle;
    const Loc* loc1 = dataHandle.getLoc("JFK", DateTime::localTime());
    const Loc* loc2 = dataHandle.getLoc("DFW", DateTime::localTime());
    const Loc* loc3 = dataHandle.getLoc("MAN", DateTime::localTime());
    const Loc* loc4 = dataHandle.getLoc("FRA", DateTime::localTime());
    MileageRoute mRoute;
    AirSeg as1, as2;
    SurfaceSeg ss;
    TravelRoute tvlRoute;
    // create AirSegs

    as1.origin() = const_cast<Loc*>(loc1);
    as1.destination() = const_cast<Loc*>(loc2);
    as1.boardMultiCity() = "NYC";
    as1.offMultiCity() = "DFW";
    as1.carrier() = "AA";

    as2.origin() = const_cast<Loc*>(loc2);
    as2.destination() = const_cast<Loc*>(loc3);
    as2.boardMultiCity() = "DFW";
    as2.offMultiCity() = "LON";
    as2.carrier() = "BA";

    ss.origin() = const_cast<Loc*>(loc3);
    ss.destination() = const_cast<Loc*>(loc4);
    ss.boardMultiCity() = "LON";
    ss.offMultiCity() = "FRA";
    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.push_back(&as1);
    tvlSegs.push_back(&as2);
    tvlSegs.push_back(&ss);
    tvlRoute.mileageTravelRoute() = tvlSegs;

    tvlRoute.globalDir() = GlobalDirection::AT;
    tvlRoute.govCxr() = "BA";
    tvlRoute.travelDate() = DateTime::localTime();

    DateTime ticketingDT = DateTime::localTime();

    PricingRequest request;
    request.ticketingDT() = DateTime::localTime();

    PricingTrx trx;
    trx.setRequest(&request);
    trx.setOptions(_memHandle.create<PricingOptions>());
    MileageRouteBuilder mBuilder;
    bool rc = mBuilder.buildMileageRoute(trx, tvlRoute, mRoute, dataHandle, ticketingDT);
    CPPUNIT_ASSERT(rc);
    CPPUNIT_ASSERT(mRoute.mileageRouteItems().size() == 3);

    MileageInfo mInfo;
    MileageValidator mValidator;

    mValidator.validate(trx, mInfo, tvlRoute);

    /*MileageRouteItems::iterator itr(mRoute.mileageRouteItems().begin());
    MileageRouteItems::iterator end(mRoute.mileageRouteItems().end());
    for(; itr!= end; ++itr)
    {
      std::cout<< " " <<std::endl;
      std::cout<< itr->city1()->loc() <<std::endl;
      std::cout<< itr->city2()->loc() <<std::endl;
      std::string MPMglobalDirectionStr;
      std::string TPMglobalDirectionStr;
      globalDirectionToStr(TPMglobalDirectionStr, itr->tpmGlobalDirection());
      globalDirectionToStr(MPMglobalDirectionStr, itr->mpmGlobalDirection());
      std::cout<< " MPM GlobalDir "   << MPMglobalDirectionStr << std::endl;
      std::cout<< " TPM GlobalDir "   << TPMglobalDirectionStr << std::endl;
      std::cout<< " Segment Carrier " << itr->segmentCarrier() << std::endl;
      std::cout<< " TPM        "     << itr->tpm() << std::endl;
      std::cout<< " MPM        "     << itr->mpm() << std::endl;
      std::cout<<"------------------------------"<<std::endl;
    }

    std::cout<<"" <<std::endl;
    std::cout<<"--------MILEAGE--------------"<<std::endl;
    std::cout<< "  TPM        "     << mInfo.totalApplicableTPM() << std::endl;
    std::cout<< "  MPM        "     << mInfo.totalApplicableMPM()   << std::endl;
    std::cout<<"------------------------------"<<std::endl; */
    CPPUNIT_ASSERT(!mInfo.valid());
    CPPUNIT_ASSERT(mInfo.surchargeAmt() > 25);
  }

  void testwithSouthAtlantic()
  {
    DataHandle dataHandle;
    const Loc* loc1 = dataHandle.getLoc("DEL", DateTime::localTime());
    const Loc* loc2 = dataHandle.getLoc("BOM", DateTime::localTime());
    const Loc* loc3 = dataHandle.getLoc("JFK", DateTime::localTime());
    const Loc* loc4 = dataHandle.getLoc("EZE", DateTime::localTime());
    const Loc* loc5 = dataHandle.getLoc("SAO", DateTime::localTime());

    MileageRoute mRoute;
    MileageTrx trx;
    MileageTrx::MileageItem item1, item2, item3, item4, item5;
    item1.cityLoc = loc1;
    item2.cityLoc = loc2;
    item3.cityLoc = loc3;
    item4.cityLoc = loc4;
    item5.cityLoc = loc5;

    trx.items().push_back(&item1);
    trx.items().push_back(&item2);
    trx.items().push_back(&item3);
    trx.items().push_back(&item4);
    trx.items().push_back(&item5);
    trx.inputDT() = DateTime::localTime();
    MileageRouteBuilder mBuilder;
    bool rc = mBuilder.buildMileageRoute(trx, mRoute, 0);
    CPPUNIT_ASSERT(rc);
    CPPUNIT_ASSERT(mRoute.mileageRouteItems().size() == 4);
    const Collector<TPMCollectorWN>& tpmCollector(
        tse::Singleton<Collector<TPMCollectorWN> >::instance());
    const Collector<MPMCollectorWN>& mpmCollector(
        tse::Singleton<Collector<MPMCollectorWN> >::instance());
    CPPUNIT_ASSERT(tpmCollector.collectMileage(mRoute));
    CPPUNIT_ASSERT(mpmCollector.collectMileage(mRoute));
  }

  void testfillInDiagnosticInfo()
  {
    MileageRoute mRoute;
    mRoute.globalDirection() = GlobalDirection::AT;
    mRoute.governingCarrier() = "AA";
    mRoute.mileageRouteMPM() = 6477;
    MileageRouteItem item1, item2, item3;
    DataHandle dataHandle;
    PricingTrx trx;
    trx.setOptions(_memHandle.create<PricingOptions>());
    mRoute.dataHandle() = &dataHandle;
    const Loc* loc1 = dataHandle.getLoc("SEA", DateTime::localTime());
    const Loc* loc2 = dataHandle.getLoc("MIA", DateTime::localTime());
    const Loc* loc3 = dataHandle.getLoc("PAR", DateTime::localTime());
    const Loc* loc4 = dataHandle.getLoc("VIE", DateTime::localTime());

    item1.city1() = const_cast<Loc*>(loc1);
    item1.city2() = const_cast<Loc*>(loc2);
    item1.globalDirection(TPM) = GlobalDirection::WH;
    item1.globalDirection(MPM) = GlobalDirection::WH;
    item1.tpm() = 2722;
    item1.mpm() = 0;

    item2.city1() = const_cast<Loc*>(loc2);
    item2.city2() = const_cast<Loc*>(loc3);
    item2.globalDirection(TPM) = GlobalDirection::AT;
    item2.globalDirection(MPM) = GlobalDirection::AT;
    item2.tpm() = 4579;
    item2.mpm() = 6010;

    item3.city1() = const_cast<Loc*>(loc3);
    item3.city2() = const_cast<Loc*>(loc4);
    item2.globalDirection(TPM) = GlobalDirection::EH;
    item2.globalDirection(MPM) = GlobalDirection::AT;
    item3.tpm() = 647;
    item3.mpm() = 6477;
    mRoute.mileageRouteItems().push_back(item1);
    mRoute.mileageRouteItems().push_back(item2);
    mRoute.mileageRouteItems().push_back(item3);

    MileageValidator mValidator;
    MileageInfo mInfo;
    FareCalcVisitor visitor;
    mValidator.fillInDiagnosticInfo(mInfo, mRoute, visitor);
    mValidator.calculateMileage(mInfo, mRoute, visitor, trx);
    CPPUNIT_ASSERT(mInfo.totalApplicableTPM() == 7948);
    CPPUNIT_ASSERT(mInfo.totalApplicableMPM() == 6477);
    CPPUNIT_ASSERT(mInfo.surchargeAmt() == 25);

    // Apply SurfaceSectorExempt

    item1.tpmSurfaceSectorExempt() = true;

    mValidator.calculateMileage(mInfo, mRoute, visitor, trx);
    mValidator.fillInDiagnosticInfo(mInfo, mRoute, visitor);
    CPPUNIT_ASSERT(mInfo.totalApplicableTPM() == 7948);

    // Apply TPD
    MockMileageValidator mockValidator;
    mRoute.tpd() = 948;
    mockValidator.calculateMileage(mInfo, mRoute, visitor, trx);
    mockValidator.fillInDiagnosticInfo(mInfo, mRoute, visitor);
    CPPUNIT_ASSERT(mInfo.totalApplicableTPM() == 7000);
    CPPUNIT_ASSERT(mInfo.tpd() == 948);
    CPPUNIT_ASSERT(mInfo.surchargeAmt() == 10);
  }

  void testcalculateMileage()
  {
    MileageRoute mRoute;
    mRoute.globalDirection() = GlobalDirection::AT;
    mRoute.governingCarrier() = "AA";
    mRoute.mileageRouteMPM() = 6477;
    MileageRouteItem item1, item2, item3;
    DataHandle dataHandle;
    PricingTrx trx;
    trx.setOptions(_memHandle.create<PricingOptions>());
    const Loc* loc1 = dataHandle.getLoc("SEA", DateTime::localTime());
    const Loc* loc2 = dataHandle.getLoc("MIA", DateTime::localTime());
    const Loc* loc3 = dataHandle.getLoc("PAR", DateTime::localTime());
    const Loc* loc4 = dataHandle.getLoc("VIE", DateTime::localTime());

    item1.city1() = const_cast<Loc*>(loc1);
    item1.city2() = const_cast<Loc*>(loc2);
    item1.globalDirection(TPM) = GlobalDirection::WH;
    item1.globalDirection(MPM) = GlobalDirection::WH;
    item1.tpm() = 2722;
    item1.mpm() = 0;
    item1.tpmSurfaceSectorExempt() = true;

    item2.city1() = const_cast<Loc*>(loc2);
    item2.city2() = const_cast<Loc*>(loc3);
    item2.globalDirection(TPM) = GlobalDirection::AT;
    item2.globalDirection(MPM) = GlobalDirection::AT;
    item2.tpm() = 4579;
    item2.mpm() = 6010;

    item3.city1() = const_cast<Loc*>(loc3);
    item3.city2() = const_cast<Loc*>(loc4);
    item2.globalDirection(TPM) = GlobalDirection::EH;
    item2.globalDirection(MPM) = GlobalDirection::AT;
    item3.tpm() = 647;
    item3.mpm() = 6477;
    mRoute.mileageRouteItems().push_back(item1);
    mRoute.mileageRouteItems().push_back(item2);
    mRoute.mileageRouteItems().push_back(item3);

    MileageValidator mValidator;
    MileageInfo mInfo;
    FareCalcVisitor visitor;
    mValidator.calculateMileage(mInfo, mRoute, visitor, trx);
    mValidator.fillInDiagnosticInfo(mInfo, mRoute, visitor);
    CPPUNIT_ASSERT(mInfo.totalApplicableTPM() == 5226);
    CPPUNIT_ASSERT(mInfo.totalApplicableMPM() == 6477);
    CPPUNIT_ASSERT(mInfo.surchargeAmt() == 0);
    CPPUNIT_ASSERT(mInfo.surfaceSectorExemptCities().empty() != true);
  }

  void testcalculateMileageForSAException()
  {
    MileageRoute mRoute;
    mRoute.globalDirection() = GlobalDirection::AT;
    mRoute.governingCarrier() = "AA";
    mRoute.mileageRouteMPM() = 8342;
    MileageRouteItem item1, item2, item3;
    DataHandle dataHandle;
    PricingRequest request;
    request.ticketingDT() = DateTime::localTime();
    PricingTrx trx;
    trx.setOptions(_memHandle.create<PricingOptions>());
    trx.setRequest(&request);
    const Loc* loc1 = dataHandle.getLoc("BUE", DateTime::localTime());
    const Loc* loc2 = dataHandle.getLoc("DFW", DateTime::localTime());
    const Loc* loc3 = dataHandle.getLoc("MIA", DateTime::localTime());
    const Loc* loc4 = dataHandle.getLoc("LON", DateTime::localTime());

    // This is not a valid data. Used only to test this case.
    item1.city1() = const_cast<Loc*>(loc1);
    item1.city2() = const_cast<Loc*>(loc2);
    item1.southAtlanticExclusion() = true;
    item1.tpm() = 5293;
    item1.mpm() = 6373;

    item2.city1() = const_cast<Loc*>(loc2);
    item2.city2() = const_cast<Loc*>(loc3);
    // item2.southAtlanticExclusion() = true;
    item2.tpm() = 1115;
    item2.mpm() = 5300;

    item3.city1() = const_cast<Loc*>(loc3);
    item3.city2() = const_cast<Loc*>(loc4);
    item3.southAtlanticExclusion() = true;
    item3.tpm() = 4430;
    item3.mpm() = 8342;

    mRoute.mileageRouteItems().push_back(item1);
    mRoute.mileageRouteItems().push_back(item2);
    mRoute.mileageRouteItems().push_back(item3);
    mRoute.southAtlanticExceptionApplies() = true;
    mRoute.dataHandle() = &dataHandle;

    MileageValidator mValidator;
    MileageInfo mInfo;
    FareCalcVisitor visitor;
    mValidator.calculateMileage(mInfo, mRoute, visitor, trx);
    // CPPUNIT_ASSERT(mInfo.totalApplicableTPM() == 6890);
    // CPPUNIT_ASSERT(mInfo.totalApplicableMPM() == 6477);
    // CPPUNIT_ASSERT(mInfo.surchargeAmt() == 0);
    CPPUNIT_ASSERT(mInfo.totalApplicableTPMSAException() == 10838);
    CPPUNIT_ASSERT(mInfo.surchargeAmtSAException() == 30);
  }

  void testwithMileageEqualization()
  {
    DataHandle dataHandle;
    const Loc* loc1 = dataHandle.getLoc("PIT", DateTime::localTime());
    const Loc* loc2 = dataHandle.getLoc("NYC", DateTime::localTime());
    const Loc* loc3 = dataHandle.getLoc("BNA", DateTime::localTime());
    const Loc* loc4 = dataHandle.getLoc("MIA", DateTime::localTime());
    const Loc* loc5 = dataHandle.getLoc("SAO", DateTime::localTime());

    MileageRoute mRoute;
    mRoute.ticketingDT() = DateTime::localTime();
    mRoute.travelDT() = DateTime::localTime();

    MileageRouteItem item1, item2, item3, item4;
    // create item1
    item1.city1() = const_cast<Loc*>(loc1);
    item1.city2() = const_cast<Loc*>(loc2);
    item1.travelDate() = DateTime::localTime();
    item1.globalDirection(TPM) = GlobalDirection::WH;
    item1.globalDirection(MPM) = GlobalDirection::WH;
    item1.tpm() = 331;
    item1.mpm() = 0;

    item2.city1() = const_cast<Loc*>(loc2);
    item2.city2() = const_cast<Loc*>(loc3);
    item2.travelDate() = DateTime::localTime();
    item2.globalDirection(TPM) = GlobalDirection::WH;
    item2.globalDirection(MPM) = GlobalDirection::WH;
    item2.tpm() = 758;
    item2.mpm() = 0;

    item3.city1() = const_cast<Loc*>(loc3);
    item3.city2() = const_cast<Loc*>(loc4);
    item3.travelDate() = DateTime::localTime();
    item3.globalDirection(TPM) = GlobalDirection::WH;
    item3.globalDirection(MPM) = GlobalDirection::WH;
    item3.tpm() = 708;
    item3.mpm() = 0;

    item4.city1() = const_cast<Loc*>(loc4);
    item4.city2() = const_cast<Loc*>(loc5);
    item4.travelDate() = DateTime::localTime();
    item4.globalDirection(TPM) = GlobalDirection::WH;
    item4.globalDirection(MPM) = GlobalDirection::WH;
    item4.tpm() = 4170;
    item4.mpm() = 5911;

    mRoute.ems() = 5;
    mRoute.dataHandle() = &dataHandle;
    mRoute.globalDirection() = GlobalDirection::WH;
    mRoute.mileageRouteItems().push_back(item1);
    mRoute.mileageRouteItems().push_back(item2);
    mRoute.mileageRouteItems().push_back(item3);
    mRoute.mileageRouteItems().push_back(item4);

    MileageValidator validator;
    MileageInfo mInfo;
    mInfo.surchargeAmt() = 5;
    mInfo.totalApplicableTPM() = mRoute.mileageRouteTPM() = 5967;
    mInfo.totalApplicableMPM() = mRoute.mileageRouteMPM() = 5911;

    bool rc = validator.applyMileageEqualization(mRoute, mInfo);

    CPPUNIT_ASSERT(rc);

    CPPUNIT_ASSERT_EQUAL((short unsigned int)5995, mRoute.mileageRouteItems().back().mpm());
    CPPUNIT_ASSERT_EQUAL((short unsigned int)4179, mRoute.mileageRouteItems().back().tpm());

    CPPUNIT_ASSERT_EQUAL((short unsigned int)5995, mInfo.totalApplicableMPM());
    CPPUNIT_ASSERT_EQUAL((short unsigned int)5976, mInfo.totalApplicableTPM());
    CPPUNIT_ASSERT_EQUAL((short unsigned int)0, mInfo.surchargeAmt());

    // Now Test the whole thing in Reverse order
    MileageRouteItem i1, i2, i3, i4;
    MileageRoute mRouteReverse;
    MileageInfo mInfoReverse;
    i1.city1() = const_cast<Loc*>(loc5);
    i1.city2() = const_cast<Loc*>(loc4);
    i1.travelDate() = DateTime::localTime();
    i1.globalDirection(TPM) = GlobalDirection::WH;
    i1.globalDirection(MPM) = GlobalDirection::WH;
    i1.tpm() = 4170;
    i1.mpm() = 0;

    i2.city1() = const_cast<Loc*>(loc4);
    i2.city2() = const_cast<Loc*>(loc3);
    i2.travelDate() = DateTime::localTime();
    i2.globalDirection(TPM) = GlobalDirection::WH;
    i2.globalDirection(MPM) = GlobalDirection::WH;
    i2.tpm() = 708;
    i2.mpm() = 0;

    i3.city1() = const_cast<Loc*>(loc3);
    i3.city2() = const_cast<Loc*>(loc2);
    i3.travelDate() = DateTime::localTime();
    i3.globalDirection(TPM) = GlobalDirection::WH;
    i3.globalDirection(MPM) = GlobalDirection::WH;
    i3.tpm() = 758;
    i3.mpm() = 0;

    i4.city1() = const_cast<Loc*>(loc2);
    i4.city2() = const_cast<Loc*>(loc1);
    i4.travelDate() = DateTime::localTime();
    i4.globalDirection(TPM) = GlobalDirection::WH;
    i4.globalDirection(MPM) = GlobalDirection::WH;
    i4.tpm() = 331;
    i4.mpm() = 5911;

    mRouteReverse.ems() = 5;
    mRouteReverse.dataHandle() = &dataHandle;
    mRouteReverse.globalDirection() = GlobalDirection::WH;
    mRouteReverse.mileageRouteItems().push_back(i1);
    mRouteReverse.mileageRouteItems().push_back(i2);
    mRouteReverse.mileageRouteItems().push_back(i3);
    mRouteReverse.mileageRouteItems().push_back(i4);

    mInfoReverse.surchargeAmt() = 5;
    mInfoReverse.totalApplicableTPM() = mRouteReverse.mileageRouteTPM() = 5967;
    mInfoReverse.totalApplicableMPM() = mRouteReverse.mileageRouteMPM() = 5911;

    bool rc1 = validator.applyMileageEqualization(mRouteReverse, mInfoReverse);

    CPPUNIT_ASSERT(rc1);

    CPPUNIT_ASSERT_EQUAL((short unsigned int)5995, mRouteReverse.mileageRouteItems().back().mpm());
    CPPUNIT_ASSERT_EQUAL((short unsigned int)4179, mRouteReverse.mileageRouteItems().front().tpm());

    CPPUNIT_ASSERT_EQUAL((short unsigned int)5995, mInfoReverse.totalApplicableMPM());
    CPPUNIT_ASSERT_EQUAL((short unsigned int)5976, mInfoReverse.totalApplicableTPM());
    CPPUNIT_ASSERT_EQUAL((short unsigned int)0, mInfoReverse.surchargeAmt());
    CPPUNIT_ASSERT_EQUAL((short unsigned int)5, mInfoReverse.equalizationSurcharges().first);
    CPPUNIT_ASSERT_EQUAL((short unsigned int)0, mInfoReverse.equalizationSurcharges().second);
  }

  void testwithMileageEqualizationErrorCondition()
  {
    DataHandle dataHandle;
    const Loc* loc1 = dataHandle.getLoc("PIT", DateTime::localTime());
    const Loc* loc2 = dataHandle.getLoc("NYC", DateTime::localTime());
    const Loc* loc3 = dataHandle.getLoc("BNA", DateTime::localTime());
    MileageRoute mRoute;
    MileageInfo mInfo;
    MileageValidator validator;
    MileageRouteItem item1, item2;

    item1.city1() = const_cast<Loc*>(loc1);
    item1.city2() = const_cast<Loc*>(loc2);

    item2.city1() = const_cast<Loc*>(loc2);
    item2.city2() = const_cast<Loc*>(loc3);

    mRoute.mileageRouteItems().push_back(item1);
    mRoute.mileageRouteItems().push_back(item2);

    bool rc = validator.applyMileageEqualization(mRoute, mInfo);

    CPPUNIT_ASSERT(!rc);
  }

private:
  TestMemHandle _memHandle;
}; // class

CPPUNIT_TEST_SUITE_REGISTRATION(MileageValidatorTest);
} // namespace
