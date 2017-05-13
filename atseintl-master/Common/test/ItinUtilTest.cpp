#include <iostream>
#include <time.h>
#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "Common/ItinUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TrxUtil.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Customer.h"
#include "DBAccess/GlobalDirSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Mileage.h"
#include "DBAccess/AirlineAllianceCarrierInfo.h"
#include "DBAccess/AirlineAllianceContinentInfo.h"
#include "DBAccess/ZoneInfo.h"
#include "Diagnostic/Diag192Collector.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestDataBuilders.h"
#include "test/include/TestMemHandle.h"
#include "test/testdata/TestLocFactory.h"
#include "test/LocGenerator.h"

using namespace boost::assign;
using namespace std;

namespace tse
{
namespace
{
const CurrencyCode GBP = "GBP";
static const std::vector<FareMarket*>::size_type one = 1;
static const std::vector<const TravelSeg*>::size_type oneTs = 1;
static const std::vector<const TravelSeg*>::size_type threeTs = 3;

class MyDataHandleMock : public DataHandleMock
{
  TestMemHandle _memHandle;

  const Mileage* getMileage(const LocCode& origin,
                            const LocCode& dest,
                            Indicator mileageType,
                            const GlobalDirection globalDir,
                            const DateTime& date)
  {
    static Mileage ret;
    ret.orig() = "DFW";
    ret.dest() = "SFO";
    ret.mileageType() = 'T';
    ret.globaldir() = GlobalDirection::WH;

    if (dest == "NYC")
      ret.mileage() = 2572;
    else
      ret.mileage() = 1468;

    ret.vendor() = "IATA";
    return &ret;
  }

  const std::vector<AirlineAllianceContinentInfo*>&
  getAirlineAllianceContinent(const GenericAllianceCode& genericAllianceCode,
                              bool reduceTemporaryVectorsFallback = false)
  {
    if (genericAllianceCode == "*O")
    {
      AirlineAllianceContinentInfo* continentInfo;
      std::vector<AirlineAllianceContinentInfo*>* ret =
          _memHandle.create<std::vector<AirlineAllianceContinentInfo*> >();

      for (int idx = 0; idx < 6; ++idx)
      {
        continentInfo = _memHandle.create<AirlineAllianceContinentInfo>();
        continentInfo->locType() = 'Z';
        continentInfo->genericAllianceCode() = "*O";
        continentInfo->effDate() = DateTime::emptyDate();
        continentInfo->expireDate() = DateTime(boost::date_time::pos_infin);
        continentInfo->discDate() = DateTime(boost::date_time::pos_infin);

        switch (idx)
        {
        case 0:
          continentInfo->continent() = getContinentByAllianceContinentCode(1);
          continentInfo->locCode() = "1178";
          break;

        case 1:
          continentInfo->continent() = getContinentByAllianceContinentCode(2);
          continentInfo->locCode() = "9967";
          break;

        case 2:
          continentInfo->continent() = getContinentByAllianceContinentCode(4);
          continentInfo->locCode() = "09971";
          break;

        case 3:
          continentInfo->continent() = getContinentByAllianceContinentCode(5);
          continentInfo->locCode() = "09960";
          break;

        case 4:
          continentInfo->continent() = getContinentByAllianceContinentCode(6);
          continentInfo->locCode() = "8246";
          break;

        case 5:
          continentInfo->continent() = getContinentByAllianceContinentCode(7);
          continentInfo->locCode() = "4329";
          break;
        }
        ret->push_back(continentInfo);
      }
      return *ret;
    }
    else if (genericAllianceCode == "*A")
    {
      AirlineAllianceContinentInfo* continentInfo;
      std::vector<AirlineAllianceContinentInfo*>* ret =
          _memHandle.create<std::vector<AirlineAllianceContinentInfo*> >();
      continentInfo = _memHandle.create<AirlineAllianceContinentInfo>();

      for (int idx = 0; idx < 2; ++idx)
      {
        continentInfo = _memHandle.create<AirlineAllianceContinentInfo>();
        continentInfo->locType() = 'Z';
        continentInfo->genericAllianceCode() = "*A";

        switch (idx)
        {
        case 0:
          continentInfo->continent() = getContinentByAllianceContinentCode(1);
          continentInfo->locCode() = "9999999";
          break;

        case 1:
          continentInfo->continent() = getContinentByAllianceContinentCode(8);
          continentInfo->locCode() = "0000000";
          break;
        }
        ret->push_back(continentInfo);
      }
      return *ret;
    }
    else if (genericAllianceCode == "")
    {
      getContinentByAllianceContinentCode(0);
    }
    return DataHandleMock::getAirlineAllianceContinent(genericAllianceCode);
  }

  const ZoneInfo*
  getZone(const VendorCode& vendor, const Zone& zone, Indicator zoneType, const DateTime& date)
  {
    if (vendor == Vendor::SABRE && zoneType == MANUAL)
    {
      ZoneInfo* ret = _memHandle.create<ZoneInfo>();
      std::vector<ZoneInfo::ZoneSeg>* segs = _memHandle.create<std::vector<ZoneInfo::ZoneSeg> >();
      ZoneInfo::ZoneSeg* seg = _memHandle.create<ZoneInfo::ZoneSeg>();

      if (zone == "0009967")
      {
        ret->zone() = "0009967";
        seg->loc() = "SG";
      }
      else if (zone == "0009971")
      {
        ret->zone() = "0009971";
        seg->loc() = "GB";
      }
      else if (zone == "0009960")
      {
        ret->zone() = "0009960";
        seg->loc() = "US";
      }
      else if (zone == "0004329")
      {
        ret->zone() = "0004329";
        seg->loc() = "AU";
      }
      else if (zone == "9999999" || zone == "0000000")
        return 0;

      seg->locType() = LOCTYPE_NATION;
      segs->push_back(*seg);
      ret->sets().push_back(*segs);
      ret->zoneType() = MANUAL;
      ret->vendor() = Vendor::SABRE;
      return ret;
    }
    return DataHandleMock::getZone(vendor, zone, zoneType, date);
  }
};
}

class ItinUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ItinUtilTest);
  CPPUNIT_TEST(testGetFirstValidTravelSeg);
  CPPUNIT_TEST(testGetLastValidTravelSeg);
  CPPUNIT_TEST(testIsDomesticUS);
  CPPUNIT_TEST(testSetFurthestPoint);
  CPPUNIT_TEST(testGetFurthestPoint1);
  CPPUNIT_TEST(testGetFurthestPoint2);
  CPPUNIT_TEST(testOriginNation);
  CPPUNIT_TEST(testIsOpenSegAfterDatedSeg);
  CPPUNIT_TEST(testSwapValidatingCarrier_SwapKLtoNW_ifPointOfSaleIsJM_beforeNWbecomeDL);
  CPPUNIT_TEST(testSwapValidatingCarrier_SwapKLtoDL_ifPointOfSaleIsJM_afterNWbecomeDL);
  CPPUNIT_TEST(testSwapValidatingCarrier_NoSwapKL_ifPointOfSaleIsNotInList);
  CPPUNIT_TEST(testSwapValidatingCarrier_NoSwapNW_ifPointOfSaleIsJM);
  CPPUNIT_TEST(testSwapValidatingCarrier_SwapKLtoDL_ifPointOfSaleIsMX_afterNWbecomeDL);
  CPPUNIT_TEST(testSwapValidatingCarrier_NoSwapKLtoDL_ifPointOfSaleIsMX_beforeNWbecomeDL);
  CPPUNIT_TEST(testSwapValidatingCarrier_NoSwapNWtoDL);
  CPPUNIT_TEST(testSwapValidatingCarrier_SwapNWToKL_ifPointOfSaleIsNotInList);
  CPPUNIT_TEST(testSwapValidatingCarrier_SwapAPtoAZ_for1SUserSubscriber);
  CPPUNIT_TEST(testSwapValidatingCarrier_NoSwapAPtoAZ_for1SUserNotSubscriber);
  CPPUNIT_TEST(testSwapValidatingCarrier_NoSwapAPtoAZ_forAxesUser);
  CPPUNIT_TEST(testSwapValidatingCarrier_NoSwapAPtoAZ_forAbacusUser);

  CPPUNIT_TEST(testConnectionExceptionReturnTrueWhenCurrentFlightOriginHAWAIIAndCxrNW);
  CPPUNIT_TEST(testConnectionExceptionReturnFalseWhenJourneyLengthTwoAndNoThirdFlight); // tests for
  // journey
  // length 2
  CPPUNIT_TEST(testConnectionExceptionReturnFalseWhenNextSegNonMultiAirportArunk);
  CPPUNIT_TEST(testConnectionExceptionReturnFalseWhenLastSegOfItinIsArunk);
  CPPUNIT_TEST(testConnectionExceptionReturnFalseWhenNextSegInterline);
  CPPUNIT_TEST(testConnectionExceptionReturnTrueWhenNextSegConnecttimeLessThan24Hr);
  CPPUNIT_TEST(testConnectionExceptionReturnFalseWhenNextSegConnecttimeMoreThan24Hr);
  CPPUNIT_TEST(testConnectionExceptionReturnFalseWhenNextSegDLdomestic);
  CPPUNIT_TEST(testConnectionExceptionReturnTrueWhenNextSegDLIntlConnecttimeLessThan24Hr);
  CPPUNIT_TEST(testConnectionExceptionReturnFalseWhenNextSegDLIntlConnecttimeMoreThan24Hr);
  CPPUNIT_TEST(testConnectionExceptionReturnFalseWhenPrevSegFirstJnyLength3); // tests for journey
  // length 3
  CPPUNIT_TEST(testConnectionExceptionReturnFalseWhenArunkNotMultiAirport);
  CPPUNIT_TEST(testConnectionExceptionReturnFalseWhenPrevPrevSegDLdomestic);
  CPPUNIT_TEST(testConnectionExceptionReturnTrueWhenPrevPrevSegDLInternational);
  CPPUNIT_TEST(testConnectionExceptionReturnTrueWhenPrevPrevSegNWhawaii);
  CPPUNIT_TEST(testConnectionExceptionReturnFalseWhenPrevPrevSegNWnotHawaiiAlaska);

  CPPUNIT_TEST(testJourneyConnectionReturnFalseWhenFlightZero);
  CPPUNIT_TEST(testJourneyConnectionReturnFalseWhenConnectionExceptionButConnectimeMoreThan24Hr);
  CPPUNIT_TEST(testJourneyConnectionReturnTrueWhenConnectionExceptionAndConnectimeEqual24Hr);
  CPPUNIT_TEST(testJourneyConnectionReturnTrueWhenConnectionExceptionAndConnectimeLessThan24Hr);
  CPPUNIT_TEST(testJourneyConnectionReturnFalseWhenJnyLengthZeroAndConnectimeMoreThan4Hr);
  CPPUNIT_TEST(testJourneyConnectionReturnTrueWhenJnyLengthZeroAndConnectimeEqual4Hr);
  CPPUNIT_TEST(testJourneyConnectionReturnTrueWhenJnyLengthZeroAndConnectimeLessThan4Hr);
  CPPUNIT_TEST(testJourneyConnectionReturnFalseWhenInternationalConnectimeMoreThan24Hr);
  CPPUNIT_TEST(testJourneyConnectionReturnTrueWhenInternationalConnectimeEqual24Hr);
  CPPUNIT_TEST(testJourneyConnectionReturnTrueWhenInternationalConnectimeLessThan24Hr);
  CPPUNIT_TEST(testJourneyConnectionReturnFalseWhenInternationalAAConnectimeMoreThan13Hr);
  CPPUNIT_TEST(testJourneyConnectionReturnTrueWhenInternationalAAConnectimeEqual13Hr);
  CPPUNIT_TEST(testJourneyConnectionReturnTrueWhenInternationalAAConnectimeLessThan13Hr);

  CPPUNIT_TEST(testStartFlowReturn0WhenStartTvlSegNotFound);
  CPPUNIT_TEST(testStartFlowReturn1WhenSecondFlightDiffCxr);
  CPPUNIT_TEST(testStartFlowReturn1WhenSecondFlightNonMultiAirportArunk);
  CPPUNIT_TEST(testStartFlowReturn1WhenNonJourneyCxr);
  CPPUNIT_TEST(testStartFlowReturn1WhenSecondFlightNotConfirm);
  CPPUNIT_TEST(testStartFlowReturn1WhenSecondNotJourneyConnection);
  CPPUNIT_TEST(testStartFlowReturn4WhenAllFlightsConnect);
  CPPUNIT_TEST(testSetItinCurrenciesWithinUS);
  CPPUNIT_TEST(testSetItinCurrenciesIntl);
  CPPUNIT_TEST(testSetItinCurrenciesIntlCalcCurrOverride);

  CPPUNIT_TEST(testStayTimeWithinSameDay);
  CPPUNIT_TEST(testStayTimeNotInSameDay);
  CPPUNIT_TEST(testStayTimeWithinNextDay);
  CPPUNIT_TEST(testStayTimeOverNextDay);
  CPPUNIT_TEST(testStayTimeWithin2Days);
  CPPUNIT_TEST(testStayTimeOver2Days);

  CPPUNIT_TEST(testIsStopover_openSegmentWithoutDateFrom);
  CPPUNIT_TEST(testIsStopover_openSegmentWithoutDateTo);
  CPPUNIT_TEST(testIsStopover_openSegmentWithoutDateBoth);
  CPPUNIT_TEST(testIsStopover_openSegmentDifferentDate);

  CPPUNIT_TEST(testIsStopover_timeUnitBlankInternationalConnection);
  CPPUNIT_TEST(testIsStopover_timeUnitBlankInternationalStopover);
  CPPUNIT_TEST(testIsStopover_timeUnitBlankDomesticConnection);
  CPPUNIT_TEST(testIsStopover_timeUnitBlankDomesticStopover);

  CPPUNIT_TEST(testIsStopover_timeValueZeroInternationalConnection);
  CPPUNIT_TEST(testIsStopover_timeValueZeroInternationalStopover);
  CPPUNIT_TEST(testIsStopover_timeValueZeroDomesticConnection);
  CPPUNIT_TEST(testIsStopover_timeValueZeroDomesticStopover);

  CPPUNIT_TEST(testIsStopover_Connection);
  CPPUNIT_TEST(testIsStopover_Stopover);

  CPPUNIT_TEST(testIsStopover_unitDaysSameDayConnection);
  CPPUNIT_TEST(testIsStopover_unitDays4DaysConnection);
  CPPUNIT_TEST(testIsStopover_unitDays4DaysStopover);

  CPPUNIT_TEST(testRtwDomestic);
  CPPUNIT_TEST(testRtwDomesticRussia);
  CPPUNIT_TEST(testRtwSingleInternational);
  CPPUNIT_TEST(testRtwAirThreeAreas);
  CPPUNIT_TEST(testRtwAirTwoAreasSurfaceAcrossAreasInside);
  CPPUNIT_TEST(testRtwAirOneAreaSurfaceAcrossAreasInside);
  CPPUNIT_TEST(testRtwTwoSectorsBetweenArea2And3);

  CPPUNIT_TEST(testRtwCtWesternHemisphereOnly);
  CPPUNIT_TEST(testRtwCtEasternHemisphereOnly);
  CPPUNIT_TEST(testRtwCtDoubleAtlantic);
  CPPUNIT_TEST(testRtwCtDoublePacyfic);
  CPPUNIT_TEST(testRtwCtEasternHemisphereOnly_embeddedSuraface);
  CPPUNIT_TEST(testRtwRwSubArea33);



  CPPUNIT_TEST(testCountContinents);
  CPPUNIT_TEST(testCountContinents_fromEuropeToAustralia);
  CPPUNIT_TEST(testCountContinents_Error);
  CPPUNIT_TEST(testCountContinents_NoContinents);

  CPPUNIT_TEST(testGcmFurthestPoint);

  CPPUNIT_TEST(testRemoveAtaeMarkets);
  CPPUNIT_TEST(testRemoveAtaeMarkets_NoRtwFm);
  CPPUNIT_TEST(testRemoveAtaeMarketsRex);

  CPPUNIT_TEST(testAreaCrossingDeterminatorNoCrossing);
  CPPUNIT_TEST(testAreaCrossingDeterminatorBothOceans);

  CPPUNIT_TEST(testGenerateNextFareMarketList_simple);
  CPPUNIT_TEST(testGenerateNextFareMarketList_manyFm);
  CPPUNIT_TEST(testGenerateNextFareMarketList_sideTrip);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _memHandle.create<MyDataHandleMock>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _request = _memHandle.create<PricingRequest>();
    _trx->setRequest(_request);
    _locSFO = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocSFO.xml");
    _locDFW = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    _locNYC = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");
    _locATL = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocATL.xml");
    _locLON = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocLON.xml");

    _acd = _memHandle.create<AreaCrossingDeterminator>();
  }

  void tearDown() { _memHandle.clear(); }

  void testGetFirstValidTravelSeg()
  {
    Itin itin;
    buildTwoStopItin(itin, _locSFO, _locDFW, _locNYC);
    const TravelSeg* travelSeg = ItinUtil::getFirstValidTravelSeg(&itin);
    CPPUNIT_ASSERT(travelSeg == itin.travelSeg().front());
  }

  void testGetLastValidTravelSeg()
  {
    Itin itin;
    buildTwoStopItin(itin, _locSFO, _locDFW, _locNYC);
    const TravelSeg* travelSeg = ItinUtil::getLastValidTravelSeg(&itin);
    CPPUNIT_ASSERT(travelSeg == itin.travelSeg().back());
  }

  void testIsDomesticUS()
  {
    Itin itin;
    buildTwoStopItin(itin, _locSFO, _locDFW, _locNYC);
    CPPUNIT_ASSERT(ItinUtil::isDomesticOfNation(&itin, UNITED_STATES));
  }

  void testSetFurthestPoint()
  {
    Itin itin;
    MyDataHandleMock mdh;
    buildTwoStopItin(itin, _locSFO, _locDFW, _locNYC);
    ItinUtil::setFurthestPoint(*_trx, &itin);
    // test to see that the furthest point got set
    //
    CPPUNIT_ASSERT_MESSAGE("Error setting furthest point for seg 1",
                           !itin.travelSeg().front()->furthestPoint(itin));
    CPPUNIT_ASSERT_MESSAGE("Error setting furthest point for seg 2",
                           itin.travelSeg().back()->furthestPoint(itin));
  }

  void testGetFurthestPoint1()
  {
    Itin itin;
    MyDataHandleMock mdh;
    buildTwoStopItin(itin, _locSFO, _locDFW, _locNYC);
    ItinUtil::setFurthestPoint(*_trx, &itin);
    const TravelSeg* travelSeg = ItinUtil::getFurthestPoint(&itin);
    CPPUNIT_ASSERT(travelSeg);
    // test to see that the furthest point got set
    //
    CPPUNIT_ASSERT_MESSAGE("Error getting furthest point",
                           travelSeg->destination()->loc() == "NYC");
  }

  void testGetFurthestPoint2()
  {
    Itin itin;
    const TravelSeg* travelSeg = ItinUtil::getFurthestPoint(&itin);
    // test to see that the furthest point got set
    //
    CPPUNIT_ASSERT_MESSAGE("Error getting furthest point", travelSeg == 0);
  }

  void testOriginNation()
  {
    Itin itin;
    buildTwoStopItin(itin, _locDFW, _locATL, _locNYC);
    // copy the fare markets from the _trx to the itin
    const std::vector<FareMarket*>& fareMarketVec = _trx->fareMarket();
    std::vector<FareMarket*>::const_iterator i = fareMarketVec.begin();
    std::vector<FareMarket*>::const_iterator j = fareMarketVec.end();

    for (; i != j; ++i)
    {
      itin.fareMarket().push_back((*i));
    }

    NationCode nationCode = ItinUtil::originNation(itin);
    // test to get the nation
    //
    CPPUNIT_ASSERT_MESSAGE("Error getting origin nation", nationCode == "US");
  }

  void testIsOpenSegAfterDatedSeg()
  {
    AirSeg tvlSeg1;
    tvlSeg1.segmentType() = Air;
    tvlSeg1.pssDepartureDate() = "2005-05-06";
    AirSeg tvlSeg2;
    tvlSeg2.segmentType() = Open;
    tvlSeg2.pssDepartureDate();
    AirSeg tvlSeg3;
    tvlSeg3.segmentType() = Open;
    tvlSeg3.pssDepartureDate() = "2005-06-06";
    AirSeg tvlSeg4;
    tvlSeg4.segmentType() = Open;
    Itin itin;
    itin.travelSeg().push_back(&tvlSeg1);
    itin.travelSeg().push_back(&tvlSeg2);
    itin.travelSeg().push_back(&tvlSeg3);
    itin.travelSeg().push_back(&tvlSeg4);
    CPPUNIT_ASSERT(!ItinUtil::isOpenSegAfterDatedSeg(itin, &tvlSeg1));
    CPPUNIT_ASSERT(!ItinUtil::isOpenSegAfterDatedSeg(itin, &tvlSeg2));
    CPPUNIT_ASSERT(!ItinUtil::isOpenSegAfterDatedSeg(itin, &tvlSeg3));
    CPPUNIT_ASSERT(ItinUtil::isOpenSegAfterDatedSeg(itin, &tvlSeg4));
    Itin itin1;
    itin1.travelSeg().push_back(&tvlSeg2);
    itin1.travelSeg().push_back(&tvlSeg3);
    itin1.travelSeg().push_back(&tvlSeg4);
    CPPUNIT_ASSERT(!ItinUtil::isOpenSegAfterDatedSeg(itin1, &tvlSeg2));
    CPPUNIT_ASSERT(!ItinUtil::isOpenSegAfterDatedSeg(itin1, &tvlSeg3));
    CPPUNIT_ASSERT(ItinUtil::isOpenSegAfterDatedSeg(itin1, &tvlSeg4));
    Itin itin2;
    itin2.travelSeg().push_back(&tvlSeg2);
    itin2.travelSeg().push_back(&tvlSeg4);
    CPPUNIT_ASSERT(!ItinUtil::isOpenSegAfterDatedSeg(itin2, &tvlSeg2));
    CPPUNIT_ASSERT(ItinUtil::isOpenSegAfterDatedSeg(itin2, &tvlSeg4));
  }

  void testSwapValidatingCarrier_SwapKLtoNW_ifPointOfSaleIsJM_beforeNWbecomeDL()
  {
    buildPricingTrxFromPOS(*_trx, "JM");
    Itin itin;
    itin.validatingCarrier() = _carrierKL;
    _trx->ticketingDate() = DateTime(2010, 1, 10, 1, 1, 0);
    ItinUtil::swapValidatingCarrier(*_trx, itin);
    CPPUNIT_ASSERT_EQUAL(_carrierDL, itin.validatingCarrier());
  }

  void testSwapValidatingCarrier_SwapKLtoDL_ifPointOfSaleIsJM_afterNWbecomeDL()
  {
    buildPricingTrxFromPOS(*_trx, "JM");
    Itin itin;
    itin.validatingCarrier() = _carrierKL;
    _trx->ticketingDate() = DateTime(2010, 2, 10, 1, 1, 0);
    ItinUtil::swapValidatingCarrier(*_trx, itin);
    CPPUNIT_ASSERT_EQUAL(_carrierDL, itin.validatingCarrier());
  }

  void testSwapValidatingCarrier_NoSwapNWtoDL()
  {
    buildPricingTrxFromPOS(*_trx, "CN");
    Itin itin;
    itin.validatingCarrier() = _carrierNW;
    ItinUtil::swapValidatingCarrier(*_trx, itin);
    CPPUNIT_ASSERT_EQUAL(_carrierNW, itin.validatingCarrier());
  }

  void testSwapValidatingCarrier_NoSwapKL_ifPointOfSaleIsNotInList()
  {
    buildPricingTrxFromPOS(*_trx, "CN");
    Itin itin;
    itin.validatingCarrier() = _carrierKL;
    ItinUtil::swapValidatingCarrier(*_trx, itin);
    CPPUNIT_ASSERT_EQUAL(_carrierKL, itin.validatingCarrier());
  }

  void testSwapValidatingCarrier_NoSwapNW_ifPointOfSaleIsJM()
  {
    buildPricingTrxFromPOS(*_trx, "JM");
    Itin itin;
    itin.validatingCarrier() = _carrierNW;
    ItinUtil::swapValidatingCarrier(*_trx, itin);
    CPPUNIT_ASSERT_EQUAL(_carrierNW, itin.validatingCarrier());
  }

  void testSwapValidatingCarrier_SwapNWToKL_ifPointOfSaleIsNotInList()
  {
    buildPricingTrxFromPOS(*_trx, "MX");
    Itin itin;
    itin.validatingCarrier() = _carrierNW;
    ItinUtil::swapValidatingCarrier(*_trx, itin);
    CPPUNIT_ASSERT_EQUAL(_carrierNW, itin.validatingCarrier());
  }

  void setUser(PricingRequest& request, std::string user)
  {
    Agent* agent = _memHandle.create<Agent>();
    agent->tvlAgencyPCC() = "ABCD"; // dummy
    Customer* agentTJR = _memHandle.create<Customer>();
    agentTJR->crsCarrier() = user;

    if (user == "1B")
    {
      TrxUtil::enableAbacus();
      agentTJR->hostName() = "ABAC";
    }
    else if (user == "1J")
      agentTJR->hostName() = "AXES";

    agent->agentTJR() = agentTJR;
    _request->ticketingAgent() = agent;
  }

  void testSwapValidatingCarrier_SwapAPtoAZ_for1SUserSubscriber()
  {
    Itin itin;
    itin.validatingCarrier() = _carrierAP;
    setUser(*_request, std::string("1S"));
    ItinUtil::swapValidatingCarrier(*_trx, itin);
    CPPUNIT_ASSERT_EQUAL(_carrierAZ, itin.validatingCarrier());
  }

  void testSwapValidatingCarrier_NoSwapAPtoAZ_for1SUserNotSubscriber()
  {
    Itin itin;
    itin.validatingCarrier() = _carrierAP;
    setUser(*_request, std::string("1S"));
    _request->ticketingAgent()->tvlAgencyPCC() = ""; // not subscriber
    ItinUtil::swapValidatingCarrier(*_trx, itin);
    CPPUNIT_ASSERT_EQUAL(_carrierAP, itin.validatingCarrier());
  }

  void testSwapValidatingCarrier_NoSwapAPtoAZ_forAxesUser()
  {
    Itin itin;
    itin.validatingCarrier() = _carrierAP;
    setUser(*_request, std::string("1J"));
    ItinUtil::swapValidatingCarrier(*_trx, itin);
    CPPUNIT_ASSERT_EQUAL(_carrierAP, itin.validatingCarrier());
  }

  void testSwapValidatingCarrier_NoSwapAPtoAZ_forAbacusUser()
  {
    Itin itin;
    itin.validatingCarrier() = _carrierAP;
    setUser(*_request, std::string("1B"));
    ItinUtil::swapValidatingCarrier(*_trx, itin);
    CPPUNIT_ASSERT_EQUAL(_carrierAP, itin.validatingCarrier());
  }

  void testSwapValidatingCarrier_SwapKLtoDL_ifPointOfSaleIsMX_afterNWbecomeDL()
  {
    buildPricingTrxFromPOS(*_trx, "MX");
    Itin itin;
    itin.validatingCarrier() = _carrierKL;
    ItinUtil::swapValidatingCarrier(*_trx, itin);
    CPPUNIT_ASSERT_EQUAL(_carrierDL, itin.validatingCarrier());
  }

  void testSwapValidatingCarrier_NoSwapKLtoDL_ifPointOfSaleIsMX_beforeNWbecomeDL()
  {
    buildPricingTrxFromPOS(*_trx, "MX");
    Itin itin;
    itin.validatingCarrier() = _carrierKL;
    ItinUtil::swapValidatingCarrier(*_trx, itin);
    CPPUNIT_ASSERT_EQUAL(_carrierDL, itin.validatingCarrier());
  }

  void testConnectionExceptionReturnTrueWhenCurrentFlightOriginHAWAIIAndCxrNW()
  {
    Itin* itin = buildItinWithThreeSegs();
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[0]);
    currFlt->carrier() = "NW";
    AirSeg* prevFlt = 0;
    CPPUNIT_ASSERT(ItinUtil::connectionException(*itin, currFlt, prevFlt, 0));
  }

  void testConnectionExceptionReturnFalseWhenJourneyLengthTwoAndNoThirdFlight()
  {
    Itin* itin = buildItinWithThreeSegs();
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[3]);
    AirSeg* prevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[1]);
    CPPUNIT_ASSERT(!ItinUtil::connectionException(*itin, currFlt, prevFlt, 2));
  }

  void testConnectionExceptionReturnFalseWhenNextSegNonMultiAirportArunk()
  {
    Itin* itin = buildItinWithThreeSegs();
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[1]);
    AirSeg* prevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[0]);
    itin->travelSeg()[2]->boardMultiCity() = LOC_KUL;
    CPPUNIT_ASSERT(!ItinUtil::connectionException(*itin, currFlt, prevFlt, 2));
  }

  void testConnectionExceptionReturnFalseWhenLastSegOfItinIsArunk()
  {
    Itin* itin = buildItinWithThreeSegs();
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[1]);
    AirSeg* prevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[0]);
    itin->travelSeg().resize(3);
    CPPUNIT_ASSERT(!ItinUtil::connectionException(*itin, currFlt, prevFlt, 2));
  }

  void testConnectionExceptionReturnFalseWhenNextSegInterline()
  {
    Itin* itin = buildItinWithThreeSegs();
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[1]);
    AirSeg* prevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[0]);
    dynamic_cast<AirSeg*>(itin->travelSeg()[3])->carrier() = "NW";
    CPPUNIT_ASSERT(!ItinUtil::connectionException(*itin, currFlt, prevFlt, 2));
  }

  void testConnectionExceptionReturnTrueWhenNextSegConnecttimeLessThan24Hr()
  {
    Itin* itin = buildItinWithThreeSegs();
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[1]);
    currFlt->carrier() = "NW";
    AirSeg* prevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[0]);
    const_cast<Loc*>(prevFlt->origin())->state() = "USTX";
    AirSeg* nextFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[3]);
    nextFlt->carrier() = "NW";
    const_cast<Loc*>(nextFlt->destination())->state() = HAWAII;
    CPPUNIT_ASSERT(ItinUtil::connectionException(*itin, currFlt, prevFlt, 2));
  }

  void testConnectionExceptionReturnFalseWhenNextSegConnecttimeMoreThan24Hr()
  {
    Itin* itin = buildItinWithThreeSegs();
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[1]);
    currFlt->carrier() = "NW";
    AirSeg* prevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[0]);
    const_cast<Loc*>(prevFlt->origin())->state() = "USTX";
    AirSeg* nextFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[3]);
    nextFlt->carrier() = "NW";
    const_cast<Loc*>(nextFlt->destination())->state() = HAWAII;
    nextFlt->departureDT() = currFlt->arrivalDT().addSeconds(86500); // 86400 makes 24 hrs
    CPPUNIT_ASSERT(!ItinUtil::connectionException(*itin, currFlt, prevFlt, 2));
  }

  void testConnectionExceptionReturnFalseWhenNextSegDLdomestic()
  {
    Itin* itin = buildItinWithThreeSegs();
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[1]);
    AirSeg* prevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[0]);
    CPPUNIT_ASSERT(!ItinUtil::connectionException(*itin, currFlt, prevFlt, 2));
  }

  void testConnectionExceptionReturnTrueWhenNextSegDLIntlConnecttimeLessThan24Hr()
  {
    Itin* itin = buildItinWithThreeSegs();
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[1]);
    AirSeg* prevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[0]);
    AirSeg* nextFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[3]);
    nextFlt->geoTravelType() = GeoTravelType::International;
    CPPUNIT_ASSERT(ItinUtil::connectionException(*itin, currFlt, prevFlt, 2));
  }

  void testConnectionExceptionReturnFalseWhenNextSegDLIntlConnecttimeMoreThan24Hr()
  {
    Itin* itin = buildItinWithThreeSegs();
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[1]);
    AirSeg* prevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[0]);
    AirSeg* nextFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[3]);
    nextFlt->geoTravelType() = GeoTravelType::International;
    nextFlt->departureDT() = currFlt->arrivalDT().addSeconds(86500); // 86400 makes 24 hrs
    CPPUNIT_ASSERT(!ItinUtil::connectionException(*itin, currFlt, prevFlt, 2));
  }

  void testConnectionExceptionReturnFalseWhenPrevSegFirstJnyLength3()
  {
    Itin* itin = buildItinWithThreeSegs();
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[1]);
    AirSeg* prevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[0]);
    CPPUNIT_ASSERT(!ItinUtil::connectionException(*itin, currFlt, prevFlt, 3));
  }

  void testConnectionExceptionReturnFalseWhenArunkNotMultiAirport()
  {
    Itin* itin = buildItinWithThreeSegs();
    makeSecondSegArunk(itin);
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[3]);
    AirSeg* prevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[2]);
    itin->travelSeg()[1]->boardMultiCity() = LOC_KUL;
    CPPUNIT_ASSERT(!ItinUtil::connectionException(*itin, currFlt, prevFlt, 3));
  }

  void testConnectionExceptionReturnFalseWhenPrevPrevSegDLdomestic()
  {
    Itin* itin = buildItinWithThreeSegs();
    makeSecondSegArunk(itin);
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[3]);
    AirSeg* prevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[2]);
    CPPUNIT_ASSERT(!ItinUtil::connectionException(*itin, currFlt, prevFlt, 3));
  }

  void testConnectionExceptionReturnTrueWhenPrevPrevSegDLInternational()
  {
    Itin* itin = buildItinWithThreeSegs();
    makeSecondSegArunk(itin);
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[3]);
    AirSeg* prevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[2]);
    AirSeg* prevPrevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[0]);
    prevPrevFlt->geoTravelType() = GeoTravelType::International;
    CPPUNIT_ASSERT(ItinUtil::connectionException(*itin, currFlt, prevFlt, 3));
  }

  void testConnectionExceptionReturnTrueWhenPrevPrevSegNWhawaii()
  {
    Itin* itin = buildItinWithThreeSegs();
    makeSecondSegArunk(itin);
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[3]);
    AirSeg* prevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[2]);
    AirSeg* prevPrevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[0]);
    currFlt->carrier() = "NW";
    prevFlt->carrier() = "NW";
    prevPrevFlt->carrier() = "NW";
    CPPUNIT_ASSERT(ItinUtil::connectionException(*itin, currFlt, prevFlt, 3));
  }

  void testConnectionExceptionReturnFalseWhenPrevPrevSegNWnotHawaiiAlaska()
  {
    Itin* itin = buildItinWithThreeSegs();
    makeSecondSegArunk(itin);
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[3]);
    AirSeg* prevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[2]);
    AirSeg* prevPrevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[0]);
    currFlt->carrier() = "NW";
    prevFlt->carrier() = "NW";
    prevPrevFlt->carrier() = "NW";
    const_cast<Loc*>(prevPrevFlt->origin())->state() = "USTX";
    CPPUNIT_ASSERT(!ItinUtil::connectionException(*itin, currFlt, prevFlt, 3));
  }

  void testJourneyConnectionReturnFalseWhenFlightZero()
  {
    Itin* itin = buildItinWithThreeSegs();
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[1]);
    AirSeg* prevFlt = 0;
    CPPUNIT_ASSERT(!ItinUtil::journeyConnection(*itin, currFlt, prevFlt, 0));
  }

  void testJourneyConnectionReturnFalseWhenConnectionExceptionButConnectimeMoreThan24Hr()
  {
    Itin* itin = buildItinWithThreeSegs();
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[1]);
    AirSeg* prevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[0]);
    currFlt->carrier() = "NW";
    prevFlt->carrier() = "NW";
    currFlt->departureDT() = prevFlt->arrivalDT().addSeconds(86500); // 86400 makes 24 hrs
    CPPUNIT_ASSERT(!ItinUtil::journeyConnection(*itin, currFlt, prevFlt, 1));
  }

  void testJourneyConnectionReturnTrueWhenConnectionExceptionAndConnectimeEqual24Hr()
  {
    Itin* itin = buildItinWithThreeSegs();
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[1]);
    AirSeg* prevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[0]);
    currFlt->carrier() = "NW";
    prevFlt->carrier() = "NW";
    currFlt->departureDT() = prevFlt->arrivalDT().addSeconds(86400); // 86400 makes 24 hrs
    CPPUNIT_ASSERT(ItinUtil::journeyConnection(*itin, currFlt, prevFlt, 1));
  }

  void testJourneyConnectionReturnTrueWhenConnectionExceptionAndConnectimeLessThan24Hr()
  {
    Itin* itin = buildItinWithThreeSegs();
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[1]);
    AirSeg* prevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[0]);
    currFlt->carrier() = "NW";
    prevFlt->carrier() = "NW";
    currFlt->departureDT() = prevFlt->arrivalDT().addSeconds(86399); // 86400 makes 24 hrs
    CPPUNIT_ASSERT(ItinUtil::journeyConnection(*itin, currFlt, prevFlt, 1));
  }

  void testJourneyConnectionReturnFalseWhenJnyLengthZeroAndConnectimeMoreThan4Hr()
  {
    Itin* itin = buildItinWithThreeSegs();
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[1]);
    AirSeg* prevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[0]);
    currFlt->departureDT() = prevFlt->arrivalDT().addSeconds(14401); // 14400 makes 4 hrs
    CPPUNIT_ASSERT(!ItinUtil::journeyConnection(*itin, currFlt, prevFlt, 0));
  }

  void testJourneyConnectionReturnTrueWhenJnyLengthZeroAndConnectimeEqual4Hr()
  {
    Itin* itin = buildItinWithThreeSegs();
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[1]);
    AirSeg* prevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[0]);
    currFlt->departureDT() = prevFlt->arrivalDT().addSeconds(14400); // 14400 makes 4 hrs
    CPPUNIT_ASSERT(ItinUtil::journeyConnection(*itin, currFlt, prevFlt, 0));
  }

  void testJourneyConnectionReturnTrueWhenJnyLengthZeroAndConnectimeLessThan4Hr()
  {
    Itin* itin = buildItinWithThreeSegs();
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[1]);
    AirSeg* prevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[0]);
    currFlt->departureDT() = prevFlt->arrivalDT().addSeconds(14399); // 14400 makes 4 hrs
    CPPUNIT_ASSERT(ItinUtil::journeyConnection(*itin, currFlt, prevFlt, 0));
  }

  void testJourneyConnectionReturnFalseWhenInternationalConnectimeMoreThan24Hr()
  {
    Itin* itin = buildItinWithThreeSegs();
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[1]);
    AirSeg* prevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[0]);
    currFlt->geoTravelType() = GeoTravelType::International;
    currFlt->departureDT() = prevFlt->arrivalDT().addSeconds(86500); // 86400 makes 24 hrs
    CPPUNIT_ASSERT(!ItinUtil::journeyConnection(*itin, currFlt, prevFlt, 0));
  }

  void testJourneyConnectionReturnTrueWhenInternationalConnectimeEqual24Hr()
  {
    Itin* itin = buildItinWithThreeSegs();
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[1]);
    AirSeg* prevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[0]);
    currFlt->geoTravelType() = GeoTravelType::International;
    currFlt->departureDT() = prevFlt->arrivalDT().addSeconds(86400); // 86400 makes 24 hrs
    CPPUNIT_ASSERT(ItinUtil::journeyConnection(*itin, currFlt, prevFlt, 0));
  }

  void testJourneyConnectionReturnTrueWhenInternationalConnectimeLessThan24Hr()
  {
    Itin* itin = buildItinWithThreeSegs();
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[1]);
    AirSeg* prevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[0]);
    currFlt->geoTravelType() = GeoTravelType::International;
    currFlt->departureDT() = prevFlt->arrivalDT().addSeconds(86399); // 86400 makes 24 hrs
    CPPUNIT_ASSERT(ItinUtil::journeyConnection(*itin, currFlt, prevFlt, 0));
  }

  void testJourneyConnectionReturnFalseWhenInternationalAAConnectimeMoreThan13Hr()
  {
    Itin* itin = buildItinWithThreeSegs();
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[1]);
    AirSeg* prevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[0]);
    currFlt->carrier() = "AA";
    currFlt->geoTravelType() = GeoTravelType::International;
    currFlt->departureDT() = prevFlt->arrivalDT().addSeconds(46801); // 46800 makes 13 hrs
    CPPUNIT_ASSERT(!ItinUtil::journeyConnection(*itin, currFlt, prevFlt, 0));
  }

  void testJourneyConnectionReturnTrueWhenInternationalAAConnectimeEqual13Hr()
  {
    Itin* itin = buildItinWithThreeSegs();
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[1]);
    AirSeg* prevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[0]);
    currFlt->carrier() = "AA";
    currFlt->geoTravelType() = GeoTravelType::International;
    currFlt->departureDT() = prevFlt->arrivalDT().addSeconds(46800); // 46800 makes 13 hrs
    CPPUNIT_ASSERT(ItinUtil::journeyConnection(*itin, currFlt, prevFlt, 0));
  }

  void testJourneyConnectionReturnTrueWhenInternationalAAConnectimeLessThan13Hr()
  {
    Itin* itin = buildItinWithThreeSegs();
    AirSeg* currFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[1]);
    AirSeg* prevFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[0]);
    currFlt->carrier() = "AA";
    currFlt->geoTravelType() = GeoTravelType::International;
    currFlt->departureDT() = prevFlt->arrivalDT().addSeconds(46799); // 46800 makes 13 hrs
    CPPUNIT_ASSERT(ItinUtil::journeyConnection(*itin, currFlt, prevFlt, 0));
  }

  void testStartFlowReturn0WhenStartTvlSegNotFound()
  {
    Itin* itin = buildItinWithThreeSegs();
    AirSeg airSeg;
    TravelSeg* startTvlSeg = dynamic_cast<TravelSeg*>(&airSeg);
    size_t expectedValue = 0;
    CPPUNIT_ASSERT_EQUAL(expectedValue, ItinUtil::startFlow(*itin, *_trx, startTvlSeg));
  }

  void testStartFlowReturn1WhenSecondFlightDiffCxr()
  {
    Itin* itin = buildItinWithThreeSegs();
    AirSeg* secondFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[1]);
    secondFlt->carrier() = "NW";
    size_t expectedValue = 1;
    CPPUNIT_ASSERT_EQUAL(expectedValue, ItinUtil::startFlow(*itin, *_trx, itin->travelSeg()[0]));
  }

  void testStartFlowReturn1WhenSecondFlightNonMultiAirportArunk()
  {
    Itin* itin = buildItinWithThreeSegs();
    makeSecondSegArunk(itin);
    itin->travelSeg()[1]->boardMultiCity() = LOC_KUL;
    size_t expectedValue = 1;
    CPPUNIT_ASSERT_EQUAL(expectedValue, ItinUtil::startFlow(*itin, *_trx, itin->travelSeg()[0]));
  }

  void testStartFlowReturn1WhenNonJourneyCxr()
  {
    Itin* itin = buildItinWithThreeSegs();
    // not flow journey carrier
    const_cast<CarrierPreference*>(itin->travelSeg()[1]->carrierPref())->flowMktJourneyType() = NO;
    size_t expectedValue = 1;
    CPPUNIT_ASSERT_EQUAL(expectedValue, ItinUtil::startFlow(*itin, *_trx, itin->travelSeg()[0]));
  }

  void testStartFlowReturn1WhenSecondFlightNotConfirm()
  {
    Itin* itin = buildItinWithThreeSegs();
    AirSeg* secondFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[1]);
    secondFlt->resStatus() = QF_RES_STATUS;
    size_t expectedValue = 1;
    CPPUNIT_ASSERT_EQUAL(expectedValue, ItinUtil::startFlow(*itin, *_trx, itin->travelSeg()[0]));
  }

  void testStartFlowReturn1WhenSecondNotJourneyConnection()
  {
    Itin* itin = buildItinWithThreeSegs();
    AirSeg* firstFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[0]);
    AirSeg* secondFlt = dynamic_cast<AirSeg*>(itin->travelSeg()[1]);
    secondFlt->departureDT() = firstFlt->arrivalDT().addSeconds(14401); // 14400 makes 4 hrs
    size_t expectedValue = 1;
    CPPUNIT_ASSERT_EQUAL(expectedValue, ItinUtil::startFlow(*itin, *_trx, itin->travelSeg()[0]));
  }

  void testStartFlowReturn4WhenAllFlightsConnect()
  {
    Itin* itin = buildItinWithThreeSegs();
    size_t expectedValue = 4;
    CPPUNIT_ASSERT_EQUAL(expectedValue, ItinUtil::startFlow(*itin, *_trx, itin->travelSeg()[0]));
  }

  void testSetItinCurrenciesWithinUS()
  {
    Itin itin;
    buildTwoStopItin(itin, _locSFO, _locDFW, _locNYC);

    ItinUtil::setItinCurrencies(itin, _trx->ticketingDate());

    CurrencyCode expectedValue = USD;
    CPPUNIT_ASSERT_EQUAL(expectedValue, itin.originationCurrency());
    expectedValue = NUC;
    CPPUNIT_ASSERT_EQUAL(expectedValue, itin.calculationCurrency());
  }

  void testSetItinCurrenciesIntl()
  {
    Itin itin;
    buildTwoStopItin(itin, _locLON, _locDFW, _locNYC);

    ItinUtil::setItinCurrencies(itin, _trx->ticketingDate());

    CurrencyCode expectedValue = GBP;
    CPPUNIT_ASSERT_EQUAL(expectedValue, itin.originationCurrency());
    expectedValue = NUC;
    CPPUNIT_ASSERT_EQUAL(expectedValue, itin.calculationCurrency());
  }

  void testSetItinCurrenciesIntlCalcCurrOverride()
  {
    Itin itin;
    buildTwoStopItin(itin, _locLON, _locDFW, _locNYC);

    itin.calcCurrencyOverride() = USD;
    ItinUtil::setItinCurrencies(itin, _trx->ticketingDate());

    CurrencyCode expectedValue = GBP;
    CPPUNIT_ASSERT_EQUAL(expectedValue, itin.originationCurrency());
    expectedValue = USD;
    CPPUNIT_ASSERT_EQUAL(expectedValue, itin.calculationCurrency());
  }

  void testStayTimeWithinSameDay()
  {
    DateTime arrivalDT(2007, 07, 01, 22, 59, 59);
    DateTime departDT(2007, 07, 01, 23, 59, 59);
    CPPUNIT_ASSERT_EQUAL(true, ItinUtil::isStayTimeWithinDays(arrivalDT, departDT, 0));
  }

  void testStayTimeNotInSameDay()
  {
    DateTime arrivalDT(2007, 07, 01, 23, 59, 59);
    DateTime departDT(2007, 07, 02, 00, 59, 59);
    CPPUNIT_ASSERT_EQUAL(false, ItinUtil::isStayTimeWithinDays(arrivalDT, departDT, 0));
  }

  void testStayTimeWithinNextDay()
  {
    DateTime arrivalDT(2007, 07, 01, 12, 59, 59);
    DateTime departDT(2007, 07, 02, 13, 59, 59);
    CPPUNIT_ASSERT_EQUAL(true, ItinUtil::isStayTimeWithinDays(arrivalDT, departDT, 1));
  }

  void testStayTimeOverNextDay()
  {
    DateTime arrivalDT(2007, 07, 01, 12, 59, 59);
    DateTime departDT(2007, 07, 03, 01, 59, 59);
    CPPUNIT_ASSERT_EQUAL(false, ItinUtil::isStayTimeWithinDays(arrivalDT, departDT, 1));
  }

  void testStayTimeWithin2Days()
  {
    DateTime arrivalDT(2007, 07, 01, 12, 59, 59);
    DateTime departDT(2007, 07, 03, 13, 59, 59);
    CPPUNIT_ASSERT_EQUAL(true, ItinUtil::isStayTimeWithinDays(arrivalDT, departDT, 2));
  }

  void testStayTimeOver2Days()
  {
    DateTime arrivalDT(2007, 07, 01, 12, 59, 59);
    DateTime departDT(2007, 07, 04, 01, 59, 59);
    CPPUNIT_ASSERT_EQUAL(false, ItinUtil::isStayTimeWithinDays(arrivalDT, departDT, 2));
  }

  void testIsStopover_openSegmentWithoutDateFrom()
  {
    AirSeg from;
    from.segmentType() = tse::Open;
    from.pssDepartureDate().clear();

    AirSeg to;
    to.segmentType() = tse::Air;
    to.pssDepartureDate() = "2013-02-18";

    GeoTravelType someTravelType = GeoTravelType::UnknownGeoTravelType;
    TimeAndUnit someMinTime;
    CPPUNIT_ASSERT(ItinUtil::isStopover(from, to, someTravelType, someMinTime));
  }

  void testIsStopover_openSegmentWithoutDateTo()
  {
    AirSeg from;
    from.segmentType() = tse::Arunk;
    from.pssDepartureDate() = "2013-02-18";

    AirSeg to;
    to.segmentType() = tse::Open;
    to.pssDepartureDate().clear();

    GeoTravelType someTravelType = GeoTravelType::Domestic;
    TimeAndUnit someMinTime;
    CPPUNIT_ASSERT(ItinUtil::isStopover(from, to, someTravelType, someMinTime));
  }

  void testIsStopover_openSegmentWithoutDateBoth()
  {
    AirSeg from;
    from.segmentType() = tse::Open;
    from.pssDepartureDate().clear();

    AirSeg to;
    to.segmentType() = tse::Open;
    to.pssDepartureDate().clear();

    GeoTravelType someTravelType = GeoTravelType::International;
    TimeAndUnit someMinTime;
    CPPUNIT_ASSERT(ItinUtil::isStopover(from, to, someTravelType, someMinTime));
  }

  void testIsStopover_openSegmentDifferentDate()
  {
    AirSeg from;
    from.segmentType() = tse::Open;
    from.arrivalDT() = DateTime(2013, 02, 18);

    AirSeg to;
    to.segmentType() = tse::Open;
    to.departureDT() = DateTime(2013, 03, 18);

    GeoTravelType someTravelType = GeoTravelType::Transborder;
    TimeAndUnit someMinTime;
    CPPUNIT_ASSERT(ItinUtil::isStopover(from, to, someTravelType, someMinTime));
  }

  ////////////////////////////////////////////////////////////////

  void testIsStopover_timeUnitBlankInternationalConnection()
  {
    AirSeg from;
    from.segmentType() = tse::Air;
    from.arrivalDT() = DateTime(2013, 2, 18, 4, 0, 0);

    // 22 hours
    AirSeg to;
    to.segmentType() = tse::Air;
    to.departureDT() = DateTime(2013, 2, 19, 2, 0, 0);

    TimeAndUnit minTime;
    minTime.set(30, RuleConst::STOPOVER_TIME_UNIT_BLANK);
    CPPUNIT_ASSERT(!ItinUtil::isStopover(from, to, GeoTravelType::International, minTime));
  }

  void testIsStopover_timeUnitBlankInternationalStopover()
  {
    AirSeg from;
    from.segmentType() = tse::Air;
    from.arrivalDT() = DateTime(2013, 2, 18, 4, 0, 0);

    // 26 hours
    AirSeg to;
    to.segmentType() = tse::Air;
    to.departureDT() = DateTime(2013, 2, 19, 6, 0, 0);

    TimeAndUnit minTime;
    minTime.set(30, RuleConst::STOPOVER_TIME_UNIT_BLANK);
    CPPUNIT_ASSERT(ItinUtil::isStopover(from, to, GeoTravelType::International, minTime));
  }

  void testIsStopover_timeUnitBlankDomesticConnection()
  {
    AirSeg from;
    from.segmentType() = tse::Air;
    from.arrivalDT() = DateTime(2013, 2, 18, 4, 0, 0);

    // 2 hours connection
    AirSeg to;
    to.segmentType() = tse::Air;
    to.departureDT() = DateTime(2013, 2, 18, 6, 0, 0);

    TimeAndUnit minTime;
    minTime.set(30, RuleConst::STOPOVER_TIME_UNIT_BLANK);
    CPPUNIT_ASSERT(!ItinUtil::isStopover(from, to, GeoTravelType::Domestic, minTime));
  }

  void testIsStopover_timeUnitBlankDomesticStopover()
  {
    AirSeg from;
    from.segmentType() = tse::Air;
    from.arrivalDT() = DateTime(2013, 2, 18, 4, 0, 0);

    // 6 hours connection
    AirSeg to;
    to.segmentType() = tse::Air;
    to.departureDT() = DateTime(2013, 2, 18, 10, 0, 0);

    TimeAndUnit minTime;
    minTime.set(30, RuleConst::STOPOVER_TIME_UNIT_BLANK);
    CPPUNIT_ASSERT(ItinUtil::isStopover(from, to, GeoTravelType::Domestic, minTime));
  }

  void testIsStopover_timeValueZeroInternationalConnection()
  {
    AirSeg from;
    from.segmentType() = tse::Air;
    from.arrivalDT() = DateTime(2013, 2, 18, 4, 0, 0);

    // 22 hours connection
    AirSeg to;
    to.segmentType() = tse::Air;
    to.departureDT() = DateTime(2013, 2, 19, 2, 0, 0);

    TimeAndUnit minTime;
    minTime.set(0, RuleConst::STOPOVER_TIME_UNIT_HOURS);
    CPPUNIT_ASSERT(!ItinUtil::isStopover(from, to, GeoTravelType::International, minTime));
  }

  void testIsStopover_timeValueZeroInternationalStopover()
  {
    AirSeg from;
    from.segmentType() = tse::Air;
    from.arrivalDT() = DateTime(2013, 2, 18, 4, 0, 0);

    // 26 hours connection
    AirSeg to;
    to.segmentType() = tse::Air;
    to.departureDT() = DateTime(2013, 2, 19, 6, 0, 0);

    TimeAndUnit minTime;
    minTime.set(0, RuleConst::STOPOVER_TIME_UNIT_HOURS);
    CPPUNIT_ASSERT(ItinUtil::isStopover(from, to, GeoTravelType::International, minTime));
  }

  void testIsStopover_timeValueZeroDomesticConnection()
  {
    AirSeg from;
    from.segmentType() = tse::Air;
    from.arrivalDT() = DateTime(2013, 2, 18, 4, 0, 0);

    // 2 hours connection
    AirSeg to;
    to.segmentType() = tse::Air;
    to.departureDT() = DateTime(2013, 2, 18, 6, 0, 0);

    TimeAndUnit minTime;
    minTime.set(0, RuleConst::STOPOVER_TIME_UNIT_HOURS);
    CPPUNIT_ASSERT(!ItinUtil::isStopover(from, to, GeoTravelType::Domestic, minTime));
  }

  void testIsStopover_timeValueZeroDomesticStopover()
  {
    AirSeg from;
    from.segmentType() = tse::Air;
    from.arrivalDT() = DateTime(2013, 2, 18, 4, 0, 0);

    // 6 hours connection
    AirSeg to;
    to.segmentType() = tse::Air;
    to.departureDT() = DateTime(2013, 2, 18, 10, 0, 0);

    TimeAndUnit minTime;
    minTime.set(0, RuleConst::STOPOVER_TIME_UNIT_HOURS);
    CPPUNIT_ASSERT(ItinUtil::isStopover(from, to, GeoTravelType::Domestic, minTime));
  }

  void testIsStopover_Connection()
  {
    AirSeg from;
    from.segmentType() = tse::Air;
    from.arrivalDT() = DateTime(2013, 2, 18, 4, 0, 0);

    // 2 hours connection
    AirSeg to;
    to.segmentType() = tse::Air;
    to.departureDT() = DateTime(2013, 2, 18, 6, 0, 0);

    TimeAndUnit minTime;
    minTime.set(4, RuleConst::STOPOVER_TIME_UNIT_HOURS);
    CPPUNIT_ASSERT(!ItinUtil::isStopover(from, to, GeoTravelType::Domestic, minTime));

    // 2 months connection
    to.segmentType() = tse::Air;
    to.departureDT() = DateTime(2013, 4, 18, 4, 0, 0);

    minTime.set(4, RuleConst::STOPOVER_TIME_UNIT_MONTHS);
    CPPUNIT_ASSERT(!ItinUtil::isStopover(from, to, GeoTravelType::Domestic, minTime));
  }

  void testIsStopover_Stopover()
  {
    AirSeg from;
    from.segmentType() = tse::Air;
    from.arrivalDT() = DateTime(2013, 2, 18, 4, 0, 0);

    // 6 hours connection
    AirSeg to;
    to.segmentType() = tse::Air;
    to.departureDT() = DateTime(2013, 2, 18, 10, 0, 0);

    TimeAndUnit minTime;
    minTime.set(4, RuleConst::STOPOVER_TIME_UNIT_HOURS);
    CPPUNIT_ASSERT(ItinUtil::isStopover(from, to, GeoTravelType::Domestic, minTime));

    // 6 months connection
    to.segmentType() = tse::Air;
    to.departureDT() = DateTime(2013, 8, 18, 4, 0, 0);

    minTime.set(4, RuleConst::STOPOVER_TIME_UNIT_MONTHS);
    CPPUNIT_ASSERT(ItinUtil::isStopover(from, to, GeoTravelType::Domestic, minTime));
  }

  void testIsStopover_unitDaysSameDayConnection()
  {
    AirSeg from;
    from.segmentType() = tse::Air;
    from.arrivalDT() = DateTime(2013, 2, 18, 4, 0, 0);

    // same day connection
    AirSeg to;
    to.segmentType() = tse::Air;
    to.departureDT() = DateTime(2013, 2, 18, 10, 0, 0);

    TimeAndUnit minTime;
    minTime.set(0, RuleConst::STOPOVER_TIME_UNIT_DAYS);
    CPPUNIT_ASSERT(!ItinUtil::isStopover(from, to, GeoTravelType::Domestic, minTime));
  }

  void testIsStopover_unitDays4DaysConnection()
  {
    AirSeg from;
    from.segmentType() = tse::Air;
    from.arrivalDT() = DateTime(2013, 2, 18, 4, 0, 0);

    // 2 days connection
    AirSeg to;
    to.segmentType() = tse::Air;
    to.departureDT() = DateTime(2013, 2, 20, 10, 0, 0);

    TimeAndUnit minTime;
    minTime.set(4, RuleConst::STOPOVER_TIME_UNIT_DAYS);
    CPPUNIT_ASSERT(!ItinUtil::isStopover(from, to, GeoTravelType::Domestic, minTime));
  }

  void testIsStopover_unitDays4DaysStopover()
  {
    AirSeg from;
    from.segmentType() = tse::Air;
    from.arrivalDT() = DateTime(2013, 2, 18, 4, 0, 0);

    // 6 days connection
    AirSeg to;
    to.segmentType() = tse::Air;
    to.departureDT() = DateTime(2013, 2, 24, 10, 0, 0);

    TimeAndUnit minTime;
    minTime.set(4, RuleConst::STOPOVER_TIME_UNIT_DAYS);
    CPPUNIT_ASSERT(ItinUtil::isStopover(from, to, GeoTravelType::Domestic, minTime));
  }

  void setUpRTW()
  {
    PricingOptions* pro = _memHandle.create<PricingOptions>();
    pro->setRtw(true);
    _trx->setOptions(pro);
    _itin = _memHandle.create<Itin>();
    FareMarket* fm = _memHandle.create<FareMarket>();
    _itin->fareMarket().push_back(fm);
  }

  Loc* getLoc(const IATAAreaCode& area, const NationCode& nation)
  {
    Loc* loc = _memHandle.create<Loc>();
    loc->area() = area;
    loc->subarea() = std::string(area) + "1";
    loc->nation() = nation;
    return loc;
  }

  TravelSeg* addSeg(const Loc* org, const Loc* dst)
  {
    AirSeg* air = _memHandle.create<AirSeg>();
    air->origin() = org;
    air->destination() = dst;
    return air;
  }

  TravelSeg* addSeg(IATAAreaCode oa, IATAAreaCode da)
  {
    return addSeg(getLoc(oa, "N1"), getLoc(da, "N2"));
  }

  void testRtwDomestic()
  {
    setUpRTW();
    _itin->travelSeg() +=
        addSeg(getLoc(IATA_AREA1, NATION_US), getLoc(IATA_AREA1, NATION_US)),
        addSeg(getLoc(IATA_AREA1, NATION_US), getLoc(IATA_AREA1, NATION_CA)),
        addSeg(getLoc(IATA_AREA1, NATION_CA), getLoc(IATA_AREA1, NATION_US));

    CPPUNIT_ASSERT_THROW(ItinUtil::setRoundTheWorld(*_trx, *_itin), ErrorResponseException);
  }

  void testRtwDomesticRussia()
  {
    setUpRTW();
    _itin->travelSeg() +=
        addSeg(getLoc(IATA_AREA2, "RU"), getLoc(IATA_AREA3, "XU")),
        addSeg(getLoc(IATA_AREA3, "XU"), getLoc(IATA_AREA2, "RU"));

    CPPUNIT_ASSERT_THROW(ItinUtil::setRoundTheWorld(*_trx, *_itin), ErrorResponseException);
  }

  void testRtwSingleInternational()
  {
    setUpRTW();
    _itin->travelSeg() +=
        addSeg(getLoc(IATA_AREA1, NATION_US), getLoc(IATA_AREA2, NATION_FRANCE));
    ArunkSeg* arunk = _memHandle.create<ArunkSeg>();
    arunk->origin() = getLoc(IATA_AREA2, NATION_FRANCE);
    arunk->destination() = getLoc(IATA_AREA1, NATION_US);
    arunk->setRtwDynamicSupplementalArunk(true);
    _itin->travelSeg() += arunk;

    CPPUNIT_ASSERT_THROW(ItinUtil::setRoundTheWorld(*_trx, *_itin), ErrorResponseException);
  }

  void testRtwAirThreeAreas()
  {
    setUpRTW();
    _itin->travelSeg() += addSeg(IATA_AREA1, IATA_AREA2);
    _itin->travelSeg() += addSeg(IATA_AREA2, IATA_AREA3);
    _itin->travelSeg() += addSeg(IATA_AREA3, IATA_AREA1);

    ItinUtil::setRoundTheWorld(*_trx, *_itin);
    CPPUNIT_ASSERT(_itin->tripCharacteristics().isSet(Itin::RW_SFC));
    CPPUNIT_ASSERT(!_itin->tripCharacteristics().isSet(Itin::CT_SFC));
  }

  void testRtwAirTwoAreasSurfaceAcrossAreasInside()
  {
    setUpRTW();
    _itin->travelSeg() += addSeg(IATA_AREA1, IATA_AREA2);
    _itin->travelSeg() += addSeg(IATA_AREA2, IATA_AREA3);
    _itin->travelSeg() += addSeg(IATA_AREA3, IATA_AREA1);
    _itin->travelSeg()[1]->segmentType() = Surface;

    ItinUtil::setRoundTheWorld(*_trx, *_itin);
    CPPUNIT_ASSERT(_itin->tripCharacteristics().isSet(Itin::RW_SFC));
    CPPUNIT_ASSERT(!_itin->tripCharacteristics().isSet(Itin::CT_SFC));
  }

  void testRtwAirOneAreaSurfaceAcrossAreasInside()
  {
    setUpRTW();
    _itin->travelSeg() += addSeg(IATA_AREA1, IATA_AREA2);
    _itin->travelSeg() += addSeg(IATA_AREA2, IATA_AREA3);
    _itin->travelSeg() += addSeg(IATA_AREA3, IATA_AREA1);
    _itin->travelSeg() += addSeg(IATA_AREA1, IATA_AREA1);
    _itin->travelSeg()[1]->segmentType() = Surface;
    _itin->travelSeg()[2]->segmentType() = Surface;

    ItinUtil::setRoundTheWorld(*_trx, *_itin);
    CPPUNIT_ASSERT(_itin->tripCharacteristics().isSet(Itin::RW_SFC));
    CPPUNIT_ASSERT(!_itin->tripCharacteristics().isSet(Itin::CT_SFC));
  }

  void testRtwTwoSectorsBetweenArea2And3()
  {
    setUpRTW();
    _itin->travelSeg() += addSeg(IATA_AREA3, IATA_AREA2);
    _itin->travelSeg() += addSeg(IATA_AREA2, IATA_AREA1);
    _itin->travelSeg() += addSeg(IATA_AREA1, IATA_AREA3);
    _itin->travelSeg() += addSeg(IATA_AREA3, IATA_AREA2);

    CPPUNIT_ASSERT_THROW(ItinUtil::setRoundTheWorld(*_trx, *_itin), ErrorResponseException);
  }

  void testRtwCtWesternHemisphereOnly()
  {
    setUpRTW();
    _itin->travelSeg() += addSeg(IATA_AREA1, IATA_AREA1);
    _itin->travelSeg() += addSeg(IATA_AREA1, IATA_AREA1);
    _itin->travelSeg()[1]->segmentType() = Surface;

    ItinUtil::setRoundTheWorld(*_trx, *_itin);
    CPPUNIT_ASSERT(!_itin->tripCharacteristics().isSet(Itin::RW_SFC));
    CPPUNIT_ASSERT(_itin->tripCharacteristics().isSet(Itin::CT_SFC));
  }

  void testRtwCtEasternHemisphereOnly()
  {
    setUpRTW();
    _itin->travelSeg() += addSeg(IATA_AREA3, IATA_AREA2);
    _itin->travelSeg() += addSeg(IATA_AREA2, IATA_AREA3);

    ItinUtil::setRoundTheWorld(*_trx, *_itin);
    CPPUNIT_ASSERT(!_itin->tripCharacteristics().isSet(Itin::RW_SFC));
    CPPUNIT_ASSERT(_itin->tripCharacteristics().isSet(Itin::CT_SFC));
  }

  void testRtwCtDoubleAtlantic()
  {
    setUpRTW();
    _itin->travelSeg() += addSeg(IATA_AREA1, IATA_AREA2);
    _itin->travelSeg() += addSeg(IATA_AREA2, IATA_AREA1);

    ItinUtil::setRoundTheWorld(*_trx, *_itin);
    CPPUNIT_ASSERT(!_itin->tripCharacteristics().isSet(Itin::RW_SFC));
    CPPUNIT_ASSERT(_itin->tripCharacteristics().isSet(Itin::CT_SFC));
  }

  void testRtwCtDoublePacyfic()
  {
    setUpRTW();
    _itin->travelSeg() += addSeg(IATA_AREA3, IATA_AREA1);
    _itin->travelSeg() += addSeg(IATA_AREA1, IATA_AREA3);

    ItinUtil::setRoundTheWorld(*_trx, *_itin);
    CPPUNIT_ASSERT(!_itin->tripCharacteristics().isSet(Itin::RW_SFC));
    CPPUNIT_ASSERT(_itin->tripCharacteristics().isSet(Itin::CT_SFC));
  }

  void testRtwCtEasternHemisphereOnly_embeddedSuraface()
  {
    setUpRTW();
    _itin->travelSeg() += addSeg(IATA_AREA3, IATA_AREA2);
    _itin->travelSeg() += addSeg(IATA_AREA2, IATA_AREA3);
    _itin->travelSeg() += addSeg(IATA_AREA3, IATA_AREA2);
    _itin->travelSeg() += addSeg(IATA_AREA2, IATA_AREA3);
    _itin->travelSeg()[1]->segmentType() = Surface;

    CPPUNIT_ASSERT_THROW(ItinUtil::setRoundTheWorld(*_trx, *_itin), ErrorResponseException);
  }

  void testRtwRwSubArea33()
  {
    setUpRTW();

    LocBuilder lb(_memHandle);
    const auto locs = {lb.withArea(IATA_AREA3, IATA_SUB_ARE_33()).withNation("N1").build(),
                       lb.withArea(IATA_AREA1, IATA_SUB_AREA_11()).withNation("N2").build(),
                       lb.withArea(IATA_AREA3, IATA_SUB_ARE_31()).withNation("N3").build()};

    for (auto locIt = locs.begin(); locIt + 1 < locs.end(); ++locIt)
      _itin->travelSeg().push_back(addSeg(*locIt, *(locIt + 1)));

    ItinUtil::setRoundTheWorld(*_trx, *_itin);
    CPPUNIT_ASSERT(_itin->tripCharacteristics().isSet(Itin::RW_SFC));
    CPPUNIT_ASSERT(!_itin->tripCharacteristics().isSet(Itin::CT_SFC));
  }

  void testCountContinents()
  {
    setUpFareMarketForRtw(*_trx);
    AirlineAllianceCarrierInfo* carrierInfo;
    _trx->dataHandle().get(carrierInfo);
    carrierInfo->genericAllianceCode() = "*O";
    carrierInfo->carrier() = "AA";
    carrierInfo->genericName() = "SKY TEAM";

    CPPUNIT_ASSERT_EQUAL(
        3, ItinUtil::countContinents(*_trx, *_trx->fareMarket().front(), *carrierInfo));
  }

  void testCountContinents_fromEuropeToAustralia()
  {
    setUpFareMarketForRtw(*_trx, true);

    AirlineAllianceCarrierInfo* carrierInfo;
    _trx->dataHandle().get(carrierInfo);
    carrierInfo->genericAllianceCode() = "*O";
    carrierInfo->carrier() = "AA";
    carrierInfo->genericName() = "SKY TEAM";

    CPPUNIT_ASSERT_EQUAL(
        4, ItinUtil::countContinents(*_trx, *_trx->fareMarket().front(), *carrierInfo));
  }

  void testCountContinents_Error()
  {
    setUpFareMarketForRtw(*_trx);
    AirlineAllianceCarrierInfo* carrierInfo;
    _trx->dataHandle().get(carrierInfo);
    carrierInfo->genericAllianceCode() = "";

    try { ItinUtil::countContinents(*_trx, *_trx->fareMarket().front(), *carrierInfo); }
    catch (const ErrorResponseException& e)
    {
      if (e.code() == ErrorResponseException::DATA_ERROR_DETECTED &&
          !strcmp(e.what(), "INVALID CONTINENT CODE"))
      {
        CPPUNIT_ASSERT(1);
        return;
      }
    }

    CPPUNIT_ASSERT(0);
  }

  void testCountContinents_NoContinents()
  {
    setUpFareMarketForRtw(*_trx);

    AirlineAllianceCarrierInfo* carrierInfo;
    _trx->dataHandle().get(carrierInfo);
    carrierInfo->genericAllianceCode() = "*S";
    carrierInfo->carrier() = "AA";
    carrierInfo->genericName() = "SKY TEAM";

    CPPUNIT_ASSERT_EQUAL(
        0, ItinUtil::countContinents(*_trx, *_trx->fareMarket().front(), *carrierInfo));
  }

  void addSeg(const std::pair<Loc*, Loc*>& od)
  {
    if (!_itin)
      _itin = _memHandle.create<Itin>();

    AirSeg* air = _memHandle.create<AirSeg>();
    air->origin() = od.first;
    air->destination() = od.second;
    _itin->travelSeg().push_back(air);
  }

  void testGcmFurthestPoint()
  {
    addSeg(_locGenerator.distance118SameHem());
    addSeg(_locGenerator.distance2641SameLat());
    addSeg(_locGenerator.distance11613DiffHem());
    addSeg(_locGenerator.distance9657SameLng());
    _itin->travelSeg().push_back(_memHandle.create<AirSeg>());

    CPPUNIT_ASSERT(!_itin->travelSeg()[2]->furthestPoint());
    Diag192Collector* dc = _memHandle.create<Diag192Collector>();
    int16_t so = ItinUtil::gcmBasedFurthestPoint(*_itin, dc);
    //      std::cout << dc->str();
    //      std::cout << "\n" << so;
    CPPUNIT_ASSERT_EQUAL(static_cast<int16_t>(3), so);
    CPPUNIT_ASSERT(_itin->travelSeg()[2]->furthestPoint());
  }

  FareMarket* createFm(const Itin* itin, size_t tsBeg, size_t tsEnd)
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    for (; tsBeg < tsEnd; ++tsBeg)
      fm->travelSeg() += itin->travelSeg()[tsBeg];
    return fm;
  }

  void addFmSegments(const Itin* itin, FareMarket* fm) {}

  template <typename... Indices>
  void addFmSegments(const Itin* itin, FareMarket* fm, size_t first, Indices... indices)
  {
    fm->travelSeg() += itin->travelSeg()[first];
    addFmSegments(itin, fm, indices...);
  }

  template <typename... Indices>
  FareMarket* createFmSegments(const Itin* itin, Indices... indices)
  {
    FareMarket* fm = _memHandle.create<FareMarket>();
    addFmSegments(itin, fm, indices...);
    return fm;
  }

  void setupRemoveAtaeMarketsRtw()
  {
    _trx->getOptions()->setRtw(true);
    _itin = _memHandle.create<Itin>();
    _trx->itin().push_back(_itin);
  }

  void testRemoveAtaeMarkets()
  {
    setupRemoveAtaeMarketsRtw();
    _itin->travelSeg() += addSeg(_locDFW, _locLON), addSeg(_locLON, _locDFW);
    _itin->fareMarket() += createFm(_itin, 0, 1), createFm(_itin, 1, 2), createFm(_itin, 0, 2);

    FareMarket* last = _itin->fareMarket().back();
    ItinUtil::removeAtaeMarkets(*_trx);

    CPPUNIT_ASSERT_EQUAL(one, _itin->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(one, _trx->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(last, _itin->fareMarket().front());
    CPPUNIT_ASSERT_EQUAL(last, _trx->fareMarket().front());
  }

  void testRemoveAtaeMarkets_NoRtwFm()
  {
    setupRemoveAtaeMarketsRtw();
    _itin->travelSeg() += addSeg(_locDFW, _locLON), addSeg(_locLON, _locDFW);
    _itin->fareMarket() += createFm(_itin, 0, 1), createFm(_itin, 1, 2);

    CPPUNIT_ASSERT_THROW(ItinUtil::removeAtaeMarkets(*_trx), ErrorResponseException);
  }

  void testRemoveAtaeMarketsRex()
  {
    RexPricingTrx* rtx = _memHandle.create<RexPricingTrx>();
    _trx = rtx;
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->setRequest(_request);
    setupRemoveAtaeMarketsRtw();

    _itin->travelSeg() += addSeg(_locDFW, _locLON), addSeg(_locLON, _locDFW);
    _itin->fareMarket() += createFm(_itin, 0, 1), createFm(_itin, 1, 2), createFm(_itin, 0, 2);

    ExcItin* ei = _memHandle.create<ExcItin>();
    rtx->exchangeItin().push_back(ei);
    ei->travelSeg() = _itin->travelSeg();
    ei->fareMarket() = _itin->fareMarket();

    FareMarket* rtwFm = _itin->fareMarket().back();
    ItinUtil::removeAtaeMarkets(*_trx);

    CPPUNIT_ASSERT_EQUAL(one, _itin->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(one, ei->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(one, rtx->fareMarket().size());
    CPPUNIT_ASSERT_EQUAL(rtwFm, _itin->fareMarket().front());
    CPPUNIT_ASSERT_EQUAL(rtwFm, ei->fareMarket().front());
    CPPUNIT_ASSERT_EQUAL(rtwFm, rtx->fareMarket().front());
  }

  inline const TravelSeg* const constMe(const TravelSeg* const ts) { return ts; }

  void testAreaCrossingDeterminatorNoCrossing()
  {
    std::vector<TravelSeg*> ts;

    ts += addSeg(IATA_AREA1, IATA_AREA1);
    ts.back()->hiddenStops() += getLoc(IATA_AREA1, "NA");
    ts += addSeg(IATA_AREA2, IATA_AREA2);
    ts.back()->hiddenStops() += getLoc(IATA_AREA2, "NA");
    ts += addSeg(IATA_AREA3, IATA_AREA3);
    ts.back()->hiddenStops() += getLoc(IATA_AREA3, "NA");

    _acd->determine(*_trx, ts);

    CPPUNIT_ASSERT(_acd->transpacificSectors().empty());
    CPPUNIT_ASSERT(_acd->transatlanticSectors().empty());
    CPPUNIT_ASSERT(_acd->area2area3crossing().empty());

    ts[0]->segmentType() = Arunk;
    CPPUNIT_ASSERT(!_acd->isTransoceanicSurface(*ts[0]));
    ts[1]->segmentType() = Surface;
    CPPUNIT_ASSERT(!_acd->isTransoceanicSurface(*ts[1]));
  }

  void testAreaCrossingDeterminatorBothOceans()
  {
    std::vector<TravelSeg*> ts;

    ts += addSeg(IATA_AREA2, IATA_AREA1);
    ts.back()->hiddenStops() += getLoc(IATA_AREA3, "NA");
    ts += addSeg(IATA_AREA1, IATA_AREA3);
    ts.back()->hiddenStops() += getLoc(IATA_AREA2, "NA");
    ts += addSeg(IATA_AREA2, IATA_AREA2);
    ts.back()->hiddenStops() += getLoc(IATA_AREA3, "NA");

    _acd->determine(*_trx, ts);

    CPPUNIT_ASSERT_EQUAL(oneTs, _acd->transpacificSectors().size());
    CPPUNIT_ASSERT_EQUAL(constMe(ts[0]), _acd->transpacificSectors()[0]);
    CPPUNIT_ASSERT_EQUAL(oneTs, _acd->transatlanticSectors().size());
    CPPUNIT_ASSERT_EQUAL(constMe(ts[1]), _acd->transatlanticSectors()[0]);
    CPPUNIT_ASSERT_EQUAL(threeTs, _acd->area2area3crossing().size());

    for (int idx = 0; idx < 3; ++idx)
      CPPUNIT_ASSERT_EQUAL(constMe(ts[idx]), _acd->area2area3crossing()[idx]);

    ts[0]->segmentType() = Arunk;
    CPPUNIT_ASSERT(_acd->isTransoceanicSurface(*ts[0]));
    CPPUNIT_ASSERT(!_acd->isTransoceanicSurface(*ts[1]));
  }

  void testGenerateNextFareMarketList_simple()
  {
    Itin itin;
    itin.travelSeg() = {
        addSeg(_locDFW, _locNYC),
        addSeg(_locNYC, _locLON),
        addSeg(_locLON, _locDFW),
    };
    itin.fareMarket() = {
        createFm(&itin, 0, 2),
        createFm(&itin, 2, 3),
    };

    std::vector<int32_t> nexts = ItinUtil::generateNextFareMarketList(itin);

    CPPUNIT_ASSERT_EQUAL(size_t(2), nexts.size());
    CPPUNIT_ASSERT_EQUAL(1, nexts[0]);
    CPPUNIT_ASSERT_EQUAL(-1, nexts[1]);
  }

  void testGenerateNextFareMarketList_manyFm()
  {
    Itin itin;
    itin.travelSeg() = {
        addSeg(_locDFW, _locNYC),
        addSeg(_locNYC, _locATL),
        addSeg(_locATL, _locNYC),
        addSeg(_locNYC, _locDFW),
    };
    itin.fareMarket() = {
        createFm(&itin, 0, 1),
        createFm(&itin, 3, 4),
        createFm(&itin, 1, 2),
        createFm(&itin, 2, 3),
    };

    std::vector<int32_t> nexts = ItinUtil::generateNextFareMarketList(itin);

    CPPUNIT_ASSERT_EQUAL(size_t(4), nexts.size());
    CPPUNIT_ASSERT_EQUAL(2, nexts[0]);
    CPPUNIT_ASSERT_EQUAL(-1, nexts[1]);
    CPPUNIT_ASSERT_EQUAL(3, nexts[2]);
    CPPUNIT_ASSERT_EQUAL(1, nexts[3]);
  }

  void testGenerateNextFareMarketList_sideTrip()
  {
    Itin itin;
    itin.travelSeg() = {
        addSeg(_locDFW, _locNYC),
        addSeg(_locNYC, _locATL),
        addSeg(_locATL, _locNYC),
        addSeg(_locNYC, _locLON),
        addSeg(_locLON, _locDFW),
    };
    itin.fareMarket() = {
        createFmSegments(&itin, 0, 3),
        createFm(&itin, 4, 5),
        createFm(&itin, 1, 2),
        createFm(&itin, 2, 3),
    };

    std::vector<int32_t> nexts = ItinUtil::generateNextFareMarketList(itin);

    CPPUNIT_ASSERT_EQUAL(size_t(4), nexts.size());
    CPPUNIT_ASSERT_EQUAL(1, nexts[0]);
    CPPUNIT_ASSERT_EQUAL(-1, nexts[1]);
    CPPUNIT_ASSERT_EQUAL(3, nexts[2]);
    CPPUNIT_ASSERT_EQUAL(-1, nexts[3]);
  }

  // Other methods
  void initAirSeg(AirSeg& travelSeg,
                  GeoTravelType geoTravelType,
                  const Loc* orig,
                  const Loc* dest,
                  CarrierCode cxr)
  {
    travelSeg.geoTravelType() = geoTravelType;
    travelSeg.origin() = orig;
    travelSeg.destination() = dest;
    travelSeg.origAirport() = orig->loc();
    travelSeg.destAirport() = dest->loc();
    travelSeg.boardMultiCity() = orig->loc();
    travelSeg.offMultiCity() = dest->loc();
    travelSeg.carrier() = cxr;
    travelSeg.stopOver() = true;
  }

  void setUpFareMarketForRtw(PricingTrx& trx, bool fromEUMEToPac = false)
  {
    // Initialize Location
    const Loc* loc1 = trx.dataHandle().getLoc(fromEUMEToPac ? "SYD" : "SIN", DateTime::localTime());
    const Loc* loc2 = trx.dataHandle().getLoc("DFW", DateTime::localTime());
    const Loc* loc3 = trx.dataHandle().getLoc("MIA", DateTime::localTime());
    const Loc* loc4 = trx.dataHandle().getLoc("LON", DateTime::localTime());
    const Loc* loc5 = loc1;

    AirSeg* travelSeg1, *travelSeg2, *travelSeg3, *travelSeg4;

    trx.dataHandle().get(travelSeg1);
    trx.dataHandle().get(travelSeg2);
    trx.dataHandle().get(travelSeg3);
    trx.dataHandle().get(travelSeg4);

    initAirSeg(*travelSeg1, GeoTravelType::International, loc1, loc2, "AA");
    travelSeg1->departureDT() = DateTime(2012, 12, 31);
    initAirSeg(*travelSeg2, GeoTravelType::International, loc2, loc3, "AA");
    travelSeg2->forcedSideTrip() = true;
    initAirSeg(*travelSeg3, GeoTravelType::International, loc3, loc4, "AA");
    travelSeg2->forcedSideTrip() = true;
    initAirSeg(*travelSeg4, GeoTravelType::International, loc4, loc5, "AA");

    FareMarket* fareMarket;
    trx.dataHandle().get(fareMarket);
    Itin* itin;
    trx.dataHandle().get(itin);

    fareMarket->travelSeg() += travelSeg1, travelSeg2, travelSeg3, travelSeg4;
    trx.fareMarket().push_back(fareMarket);
    itin->travelSeg() += travelSeg1, travelSeg2, travelSeg3, travelSeg4;
    trx.itin().push_back(itin);

    itin->geoTravelType() = GeoTravelType::International;
    itin->setTravelDate(travelSeg1->departureDT());
  }

  void buildPricingTrxFromPOS(PricingTrx& trx, const std::string& nation)
  {
    PricingRequest* request;
    trx.dataHandle().get(request);
    request->validatingCarrier().clear();
    trx.setRequest(request);
    Loc* agentLoc;
    trx.dataHandle().get(agentLoc);
    agentLoc->nation() = nation;
    Agent* agent;
    trx.dataHandle().get(agent);
    agent->agentLocation() = agentLoc;
    request->ticketingAgent() = agent;
  }

  void buildTwoStopItin(Itin& itin, const Loc* stop1, const Loc* stop2, const Loc* stop3)
  {
    // create the travel segments
    //
    AirSeg* air1 = _memHandle.create<AirSeg>();
    air1->pnrSegment() = 1;
    air1->segmentOrder() = 1;
    air1->origin() = stop1;
    air1->origAirport() = stop1->loc();
    air1->destination() = stop2;
    air1->destAirport() = stop2->loc();
    AirSeg* air2 = _memHandle.create<AirSeg>();
    air2->pnrSegment() = 2;
    air2->segmentOrder() = 2;
    air2->origin() = stop2;
    air2->origAirport() = stop2->loc();
    air2->destination() = stop3;
    air2->destAirport() = stop3->loc();
    // attach the travel segments to the transaction
    //
    _trx->travelSeg().push_back(air1);
    _trx->travelSeg().push_back(air2);
    // call buildFareMarket
    //
    TrxUtil::buildFareMarket(*_trx, _trx->travelSeg());
    itin.travelSeg().push_back(air1);
    itin.travelSeg().push_back(air2);
  }

  Itin* buildItinWithThreeSegs()
  {
    Itin* itin = _memHandle.create<Itin>();
    AirSeg* a1 = _memHandle.create<AirSeg>();
    AirSeg* a2 = _memHandle.create<AirSeg>();
    ArunkSeg* arunkSeg = _memHandle.create<ArunkSeg>();
    arunkSeg->boardMultiCity() = LOC_EWR;
    arunkSeg->offMultiCity() = LOC_NYC;
    AirSeg* a3 = _memHandle.create<AirSeg>();
    CarrierPreference* cxrPref = _memHandle.create<CarrierPreference>();
    cxrPref->flowMktJourneyType() = YES; // flow journey carrier
    a1->carrierPref() = a2->carrierPref() = a3->carrierPref() = cxrPref;
    a1->resStatus() = a2->resStatus() = a3->resStatus() = CONFIRM_RES_STATUS;
    a1->geoTravelType() = GeoTravelType::Domestic;
    a2->geoTravelType() = GeoTravelType::Domestic;
    a3->geoTravelType() = GeoTravelType::Domestic;
    Loc* a1Orig = _memHandle.create<Loc>();
    Loc* a1Dest = _memHandle.create<Loc>();
    a1Orig->state() = HAWAII;
    a1->origin() = a1Orig;
    a1->destination() = a1Dest;
    Loc* a2Orig = _memHandle.create<Loc>();
    Loc* a2Dest = _memHandle.create<Loc>();
    a2->origin() = a2Orig;
    a2->destination() = a2Dest;
    Loc* a3Orig = _memHandle.create<Loc>();
    Loc* a3Dest = _memHandle.create<Loc>();
    a3->origin() = a3Orig;
    a3->destination() = a3Dest;
    a1->carrier() = a2->carrier() = a3->carrier() = "DL";
    itin->travelSeg() += (TravelSeg*)a1, (TravelSeg*)a2, (TravelSeg*)arunkSeg, (TravelSeg*)a3;
    DateTime departFirstSeg = DateTime(2009, 6, 26, 6, 15, 0);
    a1->departureDT() = departFirstSeg;
    a1->arrivalDT() = departFirstSeg.addSeconds(3600);
    a2->departureDT() = a1->arrivalDT().addSeconds(3600);
    a2->arrivalDT() = a2->departureDT().addSeconds(3600);
    a3->departureDT() = a2->arrivalDT().addSeconds(3600);
    a3->arrivalDT() = a3->departureDT().addSeconds(3600);
    return itin;
  }

  void makeSecondSegArunk(Itin* itin)
  {
    TravelSeg* tmpTvl = itin->travelSeg()[2];
    itin->travelSeg()[2] = itin->travelSeg()[1];
    itin->travelSeg()[1] = tmpTvl;
  }

private:
  static const CarrierCode _carrierKL;
  static const CarrierCode _carrierNW;
  static const CarrierCode _carrierDL;
  static const CarrierCode _carrierAP;
  static const CarrierCode _carrierAZ;
  PricingTrx* _trx;
  PricingRequest* _request;
  Agent* _agent;
  Itin* _itin;
  const Loc* _locSFO;
  const Loc* _locDFW;
  const Loc* _locNYC;
  const Loc* _locATL;
  const Loc* _locLON;
  AreaCrossingDeterminator* _acd;
  TestMemHandle _memHandle;
  LocGenerator _locGenerator;
};
const CarrierCode ItinUtilTest::_carrierKL = "KL";
const CarrierCode ItinUtilTest::_carrierNW = "NW";
const CarrierCode ItinUtilTest::_carrierDL = "DL";
const CarrierCode ItinUtilTest::_carrierAP = "AP";
const CarrierCode ItinUtilTest::_carrierAZ = "AZ";
CPPUNIT_TEST_SUITE_REGISTRATION(ItinUtilTest);
}
