#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>

#include "DataModel/PricingTrx.h"
#include "DataModel/Itin.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingOptions.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/FareByRuleApp.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Fares/FareByRuleAppValidator.h"
#include "DBAccess/Loc.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/Billing.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/Diag208Collector.h"
#include "Fares/FareByRuleProcessingInfo.h"
#include "DBAccess/MarkupSecFilter.h"
#include "Rules/RuleUtil.h"
#include "Common/TrxUtil.h"
#include "DBAccess/Customer.h"
#include <vector>
#include "DBAccess/TariffCrossRefInfo.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"

using namespace std;
using namespace boost::assign;

namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  const std::vector<TariffCrossRefInfo*>& getTariffXRefByRuleTariff(const VendorCode& vendor,
                                                                    const CarrierCode& carrier,
                                                                    const RecordScope& crossRefType,
                                                                    const TariffNumber& ruleTariff,
                                                                    const DateTime& date)
  {
    if (vendor == "ATP" && carrier == "LH" && ruleTariff == 680)
    {
      std::vector<TariffCrossRefInfo*>* ret =
          _memHandle.create<std::vector<TariffCrossRefInfo*> >();
      if (crossRefType == DOMESTIC)
      {
        TariffCrossRefInfo* t = _memHandle.create<TariffCrossRefInfo>();
        t->vendor() = "ATP";
        t->carrier() = "LH";
        t->crossRefType() = DOMESTIC;
        t->globalDirection() = GlobalDirection::NA;
        t->fareTariff() = -680;
        t->tariffCat() = 0;
        t->ruleTariff() = 680;
        t->ruleTariffCode() = "FBRN15J";
        t->governingTariff() = t->routingTariff1() = t->routingTariff2() = t->addonTariff1() =
            t->addonTariff2() = -1;
        t->zoneNo() = "0000014";
        t->zoneVendor() = "SABR";
        t->zoneType() = 'R';
        ret->push_back(t);
      }
      return *ret;
    }
    else if (vendor == "ATP" && carrier == "AA" && ruleTariff == 191)
    {
      std::vector<TariffCrossRefInfo*>* ret =
          _memHandle.create<std::vector<TariffCrossRefInfo*> >();
      if (crossRefType == DOMESTIC)
      {
        TariffCrossRefInfo* t = _memHandle.create<TariffCrossRefInfo>();
        t->vendor() = "ATP";
        t->carrier() = "AA";
        t->crossRefType() = DOMESTIC;
        t->globalDirection() = GlobalDirection::NA;
        t->fareTariff() = -191;
        t->tariffCat() = 1;
        t->ruleTariff() = 191;
        t->ruleTariffCode() = "FBRNAPV";
        t->governingTariff() = t->routingTariff1() = t->routingTariff2() = t->addonTariff1() =
            t->addonTariff2() = -1;
        t->zoneNo() = "0000016";
        t->zoneVendor() = "SABR";
        t->zoneType() = 'R';
        ret->push_back(t);
      }
      return *ret;
    }
    return DataHandleMock::getTariffXRefByRuleTariff(
        vendor, carrier, crossRefType, ruleTariff, date);
  }
  const Indicator getTariffInhibit(const VendorCode& vendor,
                                   const Indicator tariffCrossRefType,
                                   const CarrierCode& carrier,
                                   const TariffNumber& fareTariff,
                                   const TariffCode& ruleTariffCode)
  {
    if (vendor == "ATP" && tariffCrossRefType == 'D' && carrier == "LH" && fareTariff == -680 &&
        ruleTariffCode == "FBRN15J")
      return 'N';
    else if (vendor == "ATP" && tariffCrossRefType == 'D' && carrier == "AA" &&
             fareTariff == -191 && ruleTariffCode == "FBRNAPV")
      return 'N';

    return DataHandleMock::getTariffInhibit(
        vendor, tariffCrossRefType, carrier, fareTariff, ruleTariffCode);
  }
};
}

class FareByRuleAppValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareByRuleAppValidatorTest);
  CPPUNIT_TEST(testIsValid);
  CPPUNIT_TEST(testMatchTariffXRef);
  CPPUNIT_TEST(testMatchPassengerStatus);
  CPPUNIT_TEST(testMatchGeo);
  CPPUNIT_TEST(testMatchLocation);
  CPPUNIT_TEST(testMatchWhollyWithin);
  CPPUNIT_TEST(testValidateMSFPassWhenAgencyMatchesOnPcc);
  CPPUNIT_TEST(testValidateMSFFailWhenAgencyDoNotMatchOnPcc);
  CPPUNIT_TEST(testValidateMSFPassWhenAgencyMatchesOnCrsAndLocWithPccIndOn);
  CPPUNIT_TEST(testValidateMSFPassWhenAgencyMatchesOnCrsAndLocWithPccIndOff);
  CPPUNIT_TEST(testValidateMSFPassWhenAbacusAgencyMatchesOnPccAndCrs);
  CPPUNIT_TEST(testValidateMSFPassWhenAirlineAgencyMatchesOnCarrier);
  CPPUNIT_TEST(testValidateMSFPassWhenAirlineAgencyDoNotMatchOnCarrier);
  CPPUNIT_TEST(testValidateEnhancedMSFPassWhenAgencyMatchesOnPcc);
  CPPUNIT_TEST(testValidateEnhancedMSFFailWhenAgencyDoNotMatchOnPcc);
  CPPUNIT_TEST(testValidateEnhancedMSFPassWhenAgencyMatchesOnCrsAndLocWithPccIndOn);
  CPPUNIT_TEST(testValidateEnhancedMSFPassWhenAgencyMatchesOnCrsAndLocWithPccIndOff);
  CPPUNIT_TEST(testValidateEnhancedMSFPassWhenAbacusAgencyMatchesOnPccAndCrs);
  CPPUNIT_TEST(testValidateEnhancedMSFPassWhenAirlineAgencyMatchesOnCarrier);
  CPPUNIT_TEST(testValidateEnhancedMSFFailWhenAirlineAgencyDoNotMatchOnCarrier);
  CPPUNIT_TEST_SUITE_END();

public:
  FareByRuleAppValidator _fbrAppValidator;
  PricingTrx _trx;
  Itin _itin;
  FareByRuleApp _fbrApp;
  FareByRuleProcessingInfo _fbrProcessingInfo;
  FareMarket _fareMarket;
  PricingRequest _request;
  Agent _agent;
  Agent* _abacusAgent;
  Customer* _abacusTJR;

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _agent.agentLocation() = _memHandle.create<Loc>();
  }

  void tearDown() { _memHandle.clear(); }

  //-----------------------------------------------------------------------------
  // testIsValid()
  //-----------------------------------------------------------------------------
  void testIsValid()
  {
    MyDataHandle mdh;
    PricingTrx trx;

    PricingOptions* options = _memHandle.create<PricingOptions>();
    trx.setOptions(options);
    options->employment() = "    ";

    Itin itin;
    DateTime travelDate = DateTime::localTime();
    AirSeg travelSeg;
    travelSeg.departureDT() = travelDate;
    itin.setTravelDate(travelDate);
    itin.travelSeg().push_back(&travelSeg);
    trx.travelSeg().push_back(&travelSeg);

    FareByRuleAppValidator fbrAppValidator;
    bool result;

    FareByRuleApp fbrApp;
    fbrApp.vendor() = "ATP";
    fbrApp.carrier() = "LH";
    fbrApp.ruleTariff() = 680;
    fbrApp.fareLoc1().locType() = ' ';
    fbrApp.fareLoc1().loc() = "DFW";
    fbrApp.loc1zoneItemNo() = "0000000";

    fbrApp.fareLoc2().locType() = ' ';
    fbrApp.fareLoc2().loc() = "LON";
    fbrApp.loc2zoneItemNo() = "0000000";

    fbrApp.directionality() = ' ';
    fbrApp.globalDir() = GlobalDirection::ZZ;

    FareMarket fareMarket;
    fareMarket.geoTravelType() = GeoTravelType::Domestic;

    Loc origin;
    Loc destination;

    //------------------------------------
    // Test for a match
    //------------------------------------
    origin.loc() = "SPI";
    origin.area() = "1";
    origin.nation() = "US";
    origin.state() = "TX";
    origin.cityInd() = true;

    destination.loc() = "PIT";
    destination.area() = "1";
    destination.nation() = "US";
    //  destination.state().clean();
    destination.cityInd() = true;

    fareMarket.origin() = &origin;
    fareMarket.destination() = &destination;

    fareMarket.boardMultiCity() = "SPI";
    fareMarket.offMultiCity() = "PIT";

    //  trx.diagnostic().diagnosticType() = Diagnostic208;
    //  trx.diagnostic().activate();

    PaxType paxType;
    paxType.paxType() = "ADT";

    PaxTypeBucket paxTypeCortege;
    paxTypeCortege.requestedPaxType() = &paxType;
    fareMarket.paxTypeCortege().push_back(paxTypeCortege);

    FareByRuleProcessingInfo fbrProcessingInfo;

    fbrProcessingInfo.initialize(&trx, &itin, &fareMarket, &fbrApp, 0, 0, 0);

    std::map<std::string, bool> ruleTariffMap;
    result = fbrAppValidator.isValid(fbrProcessingInfo, ruleTariffMap);

    CPPUNIT_ASSERT(result == true);
  }

  //-----------------------------------------------------------------------------
  // testMatchTariffXRef()
  //-----------------------------------------------------------------------------
  void testMatchTariffXRef()
  {
    MyDataHandle mdh;
    PricingTrx trx;
    PricingOptions* options = _memHandle.create<PricingOptions>();
    trx.setOptions(options);

    Itin itin;
    DateTime travelDate = DateTime::localTime();
    AirSeg travelSeg;
    travelSeg.departureDT() = travelDate;
    itin.setTravelDate(travelDate);
    itin.travelSeg().push_back(&travelSeg);

    FareByRuleAppValidator fbrAppValidator;
    bool result;

    FareByRuleApp fbrApp;
    fbrApp.vendor() = "ATP";
    fbrApp.carrier() = "AA";
    fbrApp.ruleTariff() = 191;

    FareMarket fareMarket;
    fareMarket.geoTravelType() = GeoTravelType::Domestic;

    Loc origin;
    Loc destination;

    //------------------------------------
    // Test for a match
    //------------------------------------
    origin.loc() = "SPI";
    origin.area() = "1";
    origin.nation() = "US";
    origin.state() = "TX";
    origin.cityInd() = true;

    destination.loc() = "PIT";
    destination.area() = "1";
    destination.nation() = "US";
    //  destination.state().clean();
    destination.cityInd() = true;

    fareMarket.origin() = &origin;
    fareMarket.destination() = &destination;

    fareMarket.boardMultiCity() = "SPI";
    fareMarket.offMultiCity() = "PIT";

    FareByRuleProcessingInfo fbrProcessingInfo;

    fbrProcessingInfo.initialize(&trx, &itin, &fareMarket, &fbrApp, 0, 0, 0);

    std::map<std::string, bool> ruleTariffMap;

    result = fbrAppValidator.matchTariffXRef(fbrProcessingInfo, ruleTariffMap);

    CPPUNIT_ASSERT(result == true);

    //------------------------------------
    // Test for no match
    //------------------------------------
    origin.loc() = "SPI";
    origin.area() = "1";
    origin.nation() = "US";
    origin.state() = "TX";
    origin.cityInd() = true;

    destination.loc() = "LON";
    destination.area() = "2";
    destination.nation() = "GB";
    //  destination.state().clean();
    destination.cityInd() = true;

    fareMarket.origin() = &origin;
    fareMarket.destination() = &destination;
    fareMarket.geoTravelType() = GeoTravelType::International;

    fbrProcessingInfo.initialize(&trx, &itin, &fareMarket, &fbrApp, 0, 0, 0);

    result = fbrAppValidator.matchTariffXRef(fbrProcessingInfo, ruleTariffMap);

    CPPUNIT_ASSERT(result == false);
  }

  //-----------------------------------------------------------------------------
  // testMatchPassengerStatus()
  //-----------------------------------------------------------------------------
  void testMatchPassengerStatus()
  {
    FareByRuleApp fbrApp;

    PricingTrx trx;
    FareMarket fareMarket;

    PricingOptions* options = _memHandle.create<PricingOptions>();
    trx.setOptions(options);

    PaxType paxType;
    paxType.paxType() = "ADT";

    PaxTypeBucket paxTypeCortege;
    paxTypeCortege.requestedPaxType() = &paxType;
    fareMarket.paxTypeCortege().push_back(paxTypeCortege);

    FareByRuleAppValidator fbrAppValidator;
    bool result;

    //------------------------------------
    // Test positive match - pass
    //------------------------------------
    fbrApp.paxInd() = 'E';
    fbrApp.negPaxStatusInd() = ' ';
    fbrApp.paxLoc().locType() = 'S';
    fbrApp.paxLoc().loc() = "USTX";

    options->employment() = "USTX";
    FareByRuleProcessingInfo fbrInfo;
    fbrInfo.fareMarket() = &fareMarket;

    result = fbrAppValidator.matchPassengerStatus(fbrApp, trx, fbrInfo);

    CPPUNIT_ASSERT(result == true);

    //------------------------------------
    // Test positive match - fail
    //------------------------------------
    fbrApp.paxInd() = 'E';
    fbrApp.negPaxStatusInd() = ' ';
    fbrApp.paxLoc().locType() = 'N';
    fbrApp.paxLoc().loc() = "CA";

    options->employment() = "US";

    result = fbrAppValidator.matchPassengerStatus(fbrApp, trx, fbrInfo);

    CPPUNIT_ASSERT(result == false);

    //------------------------------------
    // Test for negative match - fail
    //------------------------------------
    fbrApp.paxInd() = 'E';
    fbrApp.negPaxStatusInd() = 'N';
    fbrApp.paxLoc().locType() = 'S';
    fbrApp.paxLoc().loc() = "USTX";

    options->employment() = "USTX";

    result = fbrAppValidator.matchPassengerStatus(fbrApp, trx, fbrInfo);

    CPPUNIT_ASSERT(result == false);

    //------------------------------------
    // Test for negative match - pass
    //------------------------------------
    fbrApp.paxInd() = 'E';
    fbrApp.negPaxStatusInd() = 'N';
    fbrApp.paxLoc().locType() = 'S';
    fbrApp.paxLoc().loc() = "USTX";

    options->employment() = "USCA";

    result = fbrAppValidator.matchPassengerStatus(fbrApp, trx, fbrInfo);

    CPPUNIT_ASSERT(result == true);

    //------------------------------------
    // Test for invalid paxLocType
    //------------------------------------
    fbrApp.paxInd() = 'S';
    fbrApp.negPaxStatusInd() = ' ';
    fbrApp.paxLoc().locType() = 'S';
    fbrApp.paxLoc().loc() = "USTX";

    options->employment() = "US";

    result = fbrAppValidator.matchPassengerStatus(fbrApp, trx, fbrInfo);

    CPPUNIT_ASSERT(result == false);
  }

  //-----------------------------------------------------------------------------
  // testMatchGeo()
  //-----------------------------------------------------------------------------
  void testMatchGeo()
  {
    FareByRuleApp fbrApp;
    PricingTrx trx;
    PricingRequest request;
    request.ticketingDT() = DateTime::localTime();
    trx.setRequest(&request);

    FareByRuleAppValidator fbrAppValidator;
    Loc origin;
    Loc destination;
    bool result;

    //---------------------------------------
    // Test for blank locations in Record 8
    //---------------------------------------
    fbrApp.vendor() = "ATP";

    fbrApp.fareLoc1().locType() = ' ';
    fbrApp.fareLoc1().loc() = "ABC";
    fbrApp.loc1zoneItemNo() = "0000000";

    fbrApp.fareLoc2().locType() = ' ';
    fbrApp.fareLoc2().loc() = "ABC";

    fbrApp.loc2zoneItemNo() = "0000000";

    origin.loc() = "DFW";
    origin.area() = "1";
    origin.nation() = "US";
    origin.state() = "TX";
    origin.cityInd() = true;

    destination.loc() = "LON";
    destination.area() = "2";
    destination.nation() = "GB";
    //  destination.state().clean();
    destination.cityInd() = true;

    // result = fbrAppValidator.matchGeo (fbrApp,origin,destination);
    result = fbrAppValidator.matchGeo(trx, fbrApp, origin, destination, GeoTravelType::UnknownGeoTravelType);

    CPPUNIT_ASSERT(result == true);

    origin.loc() = "DFW";
    origin.area() = "1";
    origin.nation() = "US";
    origin.state() = "TX";
    origin.cityInd() = true;

    destination.loc() = "LON";
    destination.area() = "2";
    destination.nation() = "GB";
    //  destination.state().clean();
    destination.cityInd() = true;

    //------------------------------------
    // Test for positive match
    //------------------------------------
    fbrApp.vendor() = "ATP";

    fbrApp.fareLoc1().locType() = ' ';
    fbrApp.fareLoc1().loc() = "ABC";
    fbrApp.loc1zoneItemNo() = "0000016";

    fbrApp.fareLoc2().locType() = 'N';
    fbrApp.fareLoc2().loc() = "GB";
    fbrApp.loc2zoneItemNo() = "0000000";

    result = fbrAppValidator.matchGeo(trx, fbrApp, origin, destination, GeoTravelType::UnknownGeoTravelType);

    CPPUNIT_ASSERT(result == true);

    //------------------------------------
    // Test for negative match
    //------------------------------------
    fbrApp.vendor() = "ATP";

    fbrApp.fareLoc1().locType() = ' ';
    fbrApp.fareLoc1().loc() = "ABC";
    fbrApp.loc1zoneItemNo() = "0000016";

    fbrApp.fareLoc2().locType() = ' ';
    fbrApp.fareLoc2().loc() = "GB";
    fbrApp.loc2zoneItemNo() = "0000016";

    result = fbrAppValidator.matchGeo(trx, fbrApp, origin, destination, GeoTravelType::UnknownGeoTravelType);

    CPPUNIT_ASSERT(result == false);

    //------------------------------------
    // Test for zone match
    //------------------------------------
    fbrApp.vendor() = "ATP";

    fbrApp.fareLoc1().locType() = ' ';
    fbrApp.fareLoc1().loc() = "ABC";
    fbrApp.loc1zoneItemNo() = "0000016";

    fbrApp.fareLoc2().locType() = 'N';
    fbrApp.fareLoc2().loc() = "GB";
    fbrApp.loc2zoneItemNo() = "0000016";

    result = fbrAppValidator.matchGeo(trx, fbrApp, origin, destination, GeoTravelType::UnknownGeoTravelType);

    CPPUNIT_ASSERT(result == true);
  }

  //-----------------------------------------------------------------------------
  // testMatchLocation()
  //-----------------------------------------------------------------------------
  void testMatchLocation()
  {
    FareByRuleApp fbrApp;
    PricingTrx trx;
    PricingRequest request;
    request.ticketingDT() = DateTime::localTime();
    trx.setRequest(&request);

    FareByRuleAppValidator fbrAppValidator;

    FareMarket fareMarket;

    Loc origin;
    Loc destination;

    origin.loc() = "DFW";
    origin.area() = "1";
    origin.nation() = "US";
    origin.state() = "TX";
    origin.cityInd() = true;

    destination.loc() = "LON";
    destination.area() = "2";
    destination.nation() = "GB";
    //  destination.state().clean();
    destination.cityInd() = true;

    fareMarket.origin() = &origin;
    fareMarket.destination() = &destination;

    fareMarket.boardMultiCity() = "DFW";
    fareMarket.offMultiCity() = "LON";

    bool result;

    fbrApp.vendor() = "ATP";

    //------------------------------------
    // Test for blank
    //------------------------------------
    fbrApp.fareLoc1().locType() = ' ';
    fbrApp.fareLoc1().loc() = "DFW";
    fbrApp.loc1zoneItemNo() = "0000000";

    fbrApp.fareLoc2().locType() = ' ';
    fbrApp.fareLoc2().loc() = "LON";
    fbrApp.loc2zoneItemNo() = "0000000";

    fbrApp.directionality() = ' ';
    FareByRuleProcessingInfo fbrProcessingInfo;
    result = fbrAppValidator.matchLocation(trx, fbrApp, fareMarket, fbrProcessingInfo);

    CPPUNIT_ASSERT(result == true);

    //------------------------------------
    // Test for match with no direction
    //------------------------------------
    fbrApp.fareLoc1().locType() = 'C';
    fbrApp.fareLoc1().loc() = "DFW";
    fbrApp.loc1zoneItemNo() = "0000000";

    fbrApp.fareLoc2().locType() = 'C';
    fbrApp.fareLoc2().loc() = "LON";
    fbrApp.loc2zoneItemNo() = "0000000";

    fbrApp.directionality() = ' ';

    result = fbrAppValidator.matchLocation(trx, fbrApp, fareMarket, fbrProcessingInfo);

    CPPUNIT_ASSERT(result == true);

    //------------------------------------
    // Test for match with no direction
    //------------------------------------
    fbrApp.fareLoc1().locType() = 'C';
    fbrApp.fareLoc1().loc() = "LON";
    fbrApp.loc1zoneItemNo() = "0000000";

    fbrApp.fareLoc2().locType() = 'C';
    fbrApp.fareLoc2().loc() = "DFW";
    fbrApp.loc2zoneItemNo() = "0000000";

    fbrApp.directionality() = ' ';

    result = fbrAppValidator.matchLocation(trx, fbrApp, fareMarket, fbrProcessingInfo);

    CPPUNIT_ASSERT(result == true);

    //------------------------------------
    // Test for no match
    //------------------------------------
    fbrApp.fareLoc1().locType() = 'C';
    fbrApp.fareLoc1().loc() = "LON";
    fbrApp.loc1zoneItemNo() = "0000000";

    fbrApp.fareLoc2().locType() = 'C';
    fbrApp.fareLoc2().loc() = "LAX";
    fbrApp.loc2zoneItemNo() = "0000000";

    fbrApp.directionality() = ' ';

    result = fbrAppValidator.matchLocation(trx, fbrApp, fareMarket, fbrProcessingInfo);

    CPPUNIT_ASSERT(result == false);

    //------------------------------------
    // Test for no match
    //------------------------------------
    fbrApp.fareLoc1().locType() = 'C';
    fbrApp.fareLoc1().loc() = "LAX";
    fbrApp.loc1zoneItemNo() = "0000000";

    fbrApp.fareLoc2().locType() = 'C';
    fbrApp.fareLoc2().loc() = "LON";
    fbrApp.loc2zoneItemNo() = "0000000";

    fbrApp.directionality() = ' ';

    result = fbrAppValidator.matchLocation(trx, fbrApp, fareMarket, fbrProcessingInfo);

    CPPUNIT_ASSERT(result == false);

    //------------------------------------
    // Test for match with direction "F"
    //------------------------------------
    fbrApp.fareLoc1().locType() = 'C';
    fbrApp.fareLoc1().loc() = "DFW";
    fbrApp.loc1zoneItemNo() = "0000000";

    fbrApp.fareLoc2().locType() = 'C';
    fbrApp.fareLoc2().loc() = "LON";
    fbrApp.loc2zoneItemNo() = "0000000";

    fbrApp.directionality() = 'F';

    result = fbrAppValidator.matchLocation(trx, fbrApp, fareMarket, fbrProcessingInfo);

    CPPUNIT_ASSERT(result == true);
    /*
    // directioanlityt no longer used for this
      //------------------------------------
      // Test for match with direction "F"
      //------------------------------------
      fbrApp.fareLoc1().locType() = 'C';
      fbrApp.fareLoc1().loc() = "LON";
      fbrApp.loc1zoneItemNo() = "0000000";

      fbrApp.fareLoc2().locType() = 'C';
      fbrApp.fareLoc2().loc() = "DFW";
      fbrApp.loc2zoneItemNo() = "0000000";

      fbrApp.directionality() = 'F';

      result = fbrAppValidator.matchLocation(trx, fbrApp, fareMarket, fbrProcessingInfo);

      CPPUNIT_ASSERT(result == false);
      */
    //------------------------------------
    // Test for match with direction "T"
    //------------------------------------
    fbrApp.fareLoc1().locType() = 'C';
    fbrApp.fareLoc1().loc() = "LON";
    fbrApp.loc1zoneItemNo() = "0000000";

    fbrApp.fareLoc2().locType() = 'C';
    fbrApp.fareLoc2().loc() = "DFW";
    fbrApp.loc2zoneItemNo() = "0000000";

    fbrApp.directionality() = 'T';

    result = fbrAppValidator.matchLocation(trx, fbrApp, fareMarket, fbrProcessingInfo);

    CPPUNIT_ASSERT(result == true);
  }

  //-----------------------------------------------------------------------------
  // testMatchWhollyWithin()
  //-----------------------------------------------------------------------------
  void testMatchWhollyWithin()
  {
    PricingTrx trx;

    FareByRuleApp fbrApp;
    fbrApp.vendor() = "ATP";
    fbrApp.whollyWithinLoc().locType() = 'N';
    fbrApp.whollyWithinLoc().loc() = "US";

    FareByRuleAppValidator fbrAppValidator;
    bool result;

    FareMarket fareMarket;

    //------------------------------------
    // Test for a match
    //------------------------------------
    AirSeg seg1;
    AirSeg seg2;

    Loc seg1Orig;
    Loc seg1Dest;
    seg1.origin() = &seg1Orig;
    seg1.destination() = &seg1Dest;

    Loc seg2Orig;
    Loc seg2Dest;
    seg2.origin() = &seg2Orig;
    seg2.destination() = &seg2Dest;

    seg1Orig.nation() = "US";
    seg1Dest.nation() = "US";

    seg2Orig.nation() = "US";
    seg2Dest.nation() = "US";

    std::vector<TravelSeg*> tvlSegs;
    tvlSegs.push_back(&seg1);
    tvlSegs.push_back(&seg2);

    fareMarket.travelSeg() = tvlSegs;

    result = fbrAppValidator.matchWhollyWithin(fbrApp, fareMarket);

    CPPUNIT_ASSERT(result == true);

    //------------------------------------
    // Test for a fail
    //------------------------------------
    seg1Orig.nation() = "US";
    seg1Dest.nation() = "CA";

    seg2Orig.nation() = "CA";
    seg2Dest.nation() = "US";

    tvlSegs.push_back(&seg1);
    tvlSegs.push_back(&seg2);

    fareMarket.travelSeg() = tvlSegs;

    result = fbrAppValidator.matchWhollyWithin(fbrApp, fareMarket);

    CPPUNIT_ASSERT(result == false);

    //------------------------------------
    // @todo Test for Hidden stops
    //------------------------------------
  }

  void testValidateMSFPassWhenAgencyMatchesOnPcc()
  {
    vector<MarkupSecFilter*> filters;
    MarkupSecFilter msf1, msf2;
    createMSFFilter(msf1, YES, RuleConst::TRAVEL_AGENCY, "5KAD", "", ' ', "");
    createMSFFilter(msf2, YES, RuleConst::TRAVEL_AGENCY, "80K2", "", ' ', "");
    filters += &msf1, &msf2;

    _agent.tvlAgencyPCC() = "80K2";
    createFbrProcessingInfo();

    CPPUNIT_ASSERT(_fbrAppValidator.validateMarkUpSecFilter(filters, _fbrProcessingInfo));
    CPPUNIT_ASSERT(!_fbrProcessingInfo.failCat35MSF());
  }

  void testValidateMSFFailWhenAgencyDoNotMatchOnPcc()
  {
    vector<MarkupSecFilter*> filters;
    MarkupSecFilter msf1, msf2;
    createMSFFilter(msf1, YES, RuleConst::TRAVEL_AGENCY, "5KAD", "", ' ', "");
    createMSFFilter(msf2, YES, RuleConst::TRAVEL_AGENCY, "80K2", "", ' ', "");
    filters += &msf1, &msf2;

    _agent.tvlAgencyPCC() = "AF11";
    createFbrProcessingInfo();

    CPPUNIT_ASSERT(_fbrAppValidator.validateMarkUpSecFilter(filters, _fbrProcessingInfo));
    CPPUNIT_ASSERT(_fbrProcessingInfo.failCat35MSF());
  }

  void testValidateMSFPassWhenAgencyMatchesOnCrsAndLocWithPccIndOn()
  {
    vector<MarkupSecFilter*> filters;
    MarkupSecFilter msf1, msf2;
    createMSFFilter(msf1, YES, ' ', "", "1S", 'N', "VN");
    createMSFFilter(msf2, YES, RuleConst::TRAVEL_AGENCY, "80K2", "", ' ', "");
    filters += &msf1, &msf2;

    _agent.tvlAgencyPCC() = "9MGC";
    Loc locAgent;
    locAgent.nation() = "VN";
    _agent.agentLocation() = &locAgent;
    createFbrProcessingInfo();

    CPPUNIT_ASSERT(_fbrAppValidator.validateMarkUpSecFilter(filters, _fbrProcessingInfo));
    CPPUNIT_ASSERT(!_fbrProcessingInfo.failCat35MSF());
  }

  void testValidateMSFPassWhenAgencyMatchesOnCrsAndLocWithPccIndOff()
  {
    vector<MarkupSecFilter*> filters;
    MarkupSecFilter msf1, msf2;
    createMSFFilter(msf1, ' ', ' ', "", "1S", 'N', "VN");
    createMSFFilter(msf2, YES, RuleConst::TRAVEL_AGENCY, "80K2", "", ' ', "");
    filters += &msf1, &msf2;

    _agent.tvlAgencyPCC() = "9MGC";
    Loc locAgent;
    locAgent.nation() = "VN";
    _agent.agentLocation() = &locAgent;
    createFbrProcessingInfo();

    CPPUNIT_ASSERT(_fbrAppValidator.validateMarkUpSecFilter(filters, _fbrProcessingInfo));
    CPPUNIT_ASSERT(!_fbrProcessingInfo.failCat35MSF());
  }

  void testValidateMSFPassWhenAbacusAgencyMatchesOnPccAndCrs()
  {
    setUpAbacusUser();
    vector<MarkupSecFilter*> filters;
    MarkupSecFilter msf1, msf2;
    createMSFFilter(msf1, YES, RuleConst::TRAVEL_AGENCY, "M4LC", "1S", ' ', "");
    createMSFFilter(msf2, YES, RuleConst::HOME_TRAVEL_AGENCY, "GK88", "1B", ' ', "");
    filters += &msf1, &msf2;

    createFbrProcessingInfo();
    _request.ticketingAgent() = _abacusAgent;

    CPPUNIT_ASSERT(_fbrAppValidator.validateMarkUpSecFilter(filters, _fbrProcessingInfo));
    CPPUNIT_ASSERT(!_fbrProcessingInfo.failCat35MSF());
  }

  void testValidateMSFPassWhenAirlineAgencyMatchesOnCarrier()
  {
    vector<MarkupSecFilter*> filters;
    MarkupSecFilter msf;
    createMSFFilter(msf, ' ', ' ', "", "NW", ' ', "");
    filters += &msf;

    Billing billing;
    _trx.billing() = &billing;
    _trx.billing()->partitionID() = "NW";
    createFbrProcessingInfo();

    CPPUNIT_ASSERT(_fbrAppValidator.validateMarkUpSecFilter(filters, _fbrProcessingInfo));
    CPPUNIT_ASSERT(!_fbrProcessingInfo.failCat35MSF());
  }

  void testValidateMSFPassWhenAirlineAgencyDoNotMatchOnCarrier()
  {
    vector<MarkupSecFilter*> filters;
    MarkupSecFilter msf;
    createMSFFilter(msf, ' ', ' ', "", "NW", ' ', "");
    filters += &msf;

    Billing billing;
    _trx.billing() = &billing;
    _trx.billing()->partitionID() = "AA";
    createFbrProcessingInfo();

    CPPUNIT_ASSERT(_fbrAppValidator.validateMarkUpSecFilter(filters, _fbrProcessingInfo));
    CPPUNIT_ASSERT(_fbrProcessingInfo.failCat35MSF());
  }

  void testValidateEnhancedMSFPassWhenAgencyMatchesOnPcc()
  {
    vector<MarkupSecFilter*> filters;
    MarkupSecFilter msf1, msf2;
    createMSFFilter(msf1, YES, RuleConst::TRAVEL_AGENCY, "5KAD", "", ' ', "");
    createMSFFilter(msf2, YES, RuleConst::TRAVEL_AGENCY, "80K2", "", ' ', "");
    filters += &msf1, &msf2;

    _agent.tvlAgencyPCC() = "80K2";
    createFbrProcessingInfo();

    CPPUNIT_ASSERT(_fbrAppValidator.validateMarkUpSecFilterEnhanced(filters, _fbrProcessingInfo));
    CPPUNIT_ASSERT(!_fbrProcessingInfo.failCat35MSF());
  }

  void testValidateEnhancedMSFFailWhenAgencyDoNotMatchOnPcc()
  {
    vector<MarkupSecFilter*> filters;
    MarkupSecFilter msf1, msf2;
    createMSFFilter(msf1, YES, RuleConst::TRAVEL_AGENCY, "5KAD", "", ' ', "");
    createMSFFilter(msf2, YES, RuleConst::TRAVEL_AGENCY, "80K2", "", ' ', "");
    filters += &msf1, &msf2;

    _agent.tvlAgencyPCC() = "AF11";
    createFbrProcessingInfo();

    CPPUNIT_ASSERT(!_fbrAppValidator.validateMarkUpSecFilterEnhanced(filters, _fbrProcessingInfo));
    CPPUNIT_ASSERT(_fbrProcessingInfo.failCat35MSF());
  }

  void testValidateEnhancedMSFPassWhenAgencyMatchesOnCrsAndLocWithPccIndOn()
  {
    vector<MarkupSecFilter*> filters;
    MarkupSecFilter msf1, msf2;
    createMSFFilter(msf1, YES, ' ', "", "1S", 'N', "VN");
    createMSFFilter(msf2, YES, RuleConst::TRAVEL_AGENCY, "80K2", "", ' ', "");
    filters += &msf1, &msf2;

    _agent.tvlAgencyPCC() = "9MGC";
    Loc locAgent;
    locAgent.nation() = "VN";
    _agent.agentLocation() = &locAgent;
    createFbrProcessingInfo();

    CPPUNIT_ASSERT(_fbrAppValidator.validateMarkUpSecFilterEnhanced(filters, _fbrProcessingInfo));
    CPPUNIT_ASSERT(!_fbrProcessingInfo.failCat35MSF());
  }

  void testValidateEnhancedMSFPassWhenAgencyMatchesOnCrsAndLocWithPccIndOff()
  {
    vector<MarkupSecFilter*> filters;
    MarkupSecFilter msf1, msf2;
    createMSFFilter(msf1, ' ', ' ', "", "1S", 'N', "VN");
    createMSFFilter(msf2, YES, RuleConst::TRAVEL_AGENCY, "80K2", "", ' ', "");
    filters += &msf1, &msf2;

    _agent.tvlAgencyPCC() = "9MGC";
    Loc locAgent;
    locAgent.nation() = "VN";
    _agent.agentLocation() = &locAgent;
    createFbrProcessingInfo();

    CPPUNIT_ASSERT(_fbrAppValidator.validateMarkUpSecFilterEnhanced(filters, _fbrProcessingInfo));
    CPPUNIT_ASSERT(!_fbrProcessingInfo.failCat35MSF());
  }

  void testValidateEnhancedMSFPassWhenAbacusAgencyMatchesOnPccAndCrs()
  {
    setUpAbacusUser();
    vector<MarkupSecFilter*> filters;
    MarkupSecFilter msf1, msf2;
    createMSFFilter(msf1, YES, RuleConst::TRAVEL_AGENCY, "M4LC", "1S", ' ', "");
    createMSFFilter(msf2, YES, RuleConst::HOME_TRAVEL_AGENCY, "GK88", "1B", ' ', "");
    filters += &msf1, &msf2;

    createFbrProcessingInfo();
    _request.ticketingAgent() = _abacusAgent;

    CPPUNIT_ASSERT(_fbrAppValidator.validateMarkUpSecFilterEnhanced(filters, _fbrProcessingInfo));
    CPPUNIT_ASSERT(!_fbrProcessingInfo.failCat35MSF());
  }

  void testValidateEnhancedMSFPassWhenAirlineAgencyMatchesOnCarrier()
  {
    vector<MarkupSecFilter*> filters;
    MarkupSecFilter msf;
    createMSFFilter(msf, ' ', ' ', "", "NW", ' ', "");
    filters += &msf;

    Billing billing;
    _trx.billing() = &billing;
    _trx.billing()->partitionID() = "NW";
    createFbrProcessingInfo();

    CPPUNIT_ASSERT(_fbrAppValidator.validateMarkUpSecFilterEnhanced(filters, _fbrProcessingInfo));
    CPPUNIT_ASSERT(!_fbrProcessingInfo.failCat35MSF());
  }

  void testValidateEnhancedMSFFailWhenAirlineAgencyDoNotMatchOnCarrier()
  {
    vector<MarkupSecFilter*> filters;
    MarkupSecFilter msf;
    createMSFFilter(msf, ' ', ' ', "", "NW", ' ', "");
    filters += &msf;

    Billing billing;
    _trx.billing() = &billing;
    _trx.billing()->partitionID() = "AA";
    createFbrProcessingInfo();

    CPPUNIT_ASSERT(!_fbrAppValidator.validateMarkUpSecFilterEnhanced(filters, _fbrProcessingInfo));
    CPPUNIT_ASSERT(_fbrProcessingInfo.failCat35MSF());
  }

  void createMSFFilter(MarkupSecFilter& msf,
                       const Indicator& pseudoCityInd,
                       const Indicator& pseudoCityType,
                       const PseudoCityCode& pseudoCity,
                       const CarrierCode& carrierCrs,
                       const LocTypeCode& locTypeCode,
                       const LocCode& locCode)
  {
    msf.pseudoCityInd() = pseudoCityInd;
    msf.pseudoCityType() = pseudoCityType;
    msf.pseudoCity() = pseudoCity;
    msf.carrierCrs() = carrierCrs;
    msf.loc().locType() = locTypeCode;
    msf.loc().loc() = locCode;
  }

  void setUpAbacusUser()
  {
    TrxUtil::enableAbacus();

    _trx.dataHandle().get(_abacusTJR);
    CPPUNIT_ASSERT(_abacusTJR);
    _abacusTJR->crsCarrier() = "1B";
    _abacusTJR->hostName() = "ABAC";

    _trx.dataHandle().get(_abacusAgent);
    CPPUNIT_ASSERT(_abacusAgent);
    _abacusAgent->agentTJR() = _abacusTJR;
    Loc* loc(0);
    _trx.dataHandle().get(loc);
    _abacusAgent->agentLocation() = loc;
    _abacusAgent->mainTvlAgencyPCC() = "GK88";
    _abacusAgent->tvlAgencyPCC() = "GK88";
  }

  void createFbrProcessingInfo()
  {
    _request.ticketingAgent() = &_agent;
    _trx.setRequest(&_request);
    _fbrApp.vendor() = "ATP";
    _fbrProcessingInfo.initialize(&_trx, &_itin, &_fareMarket, &_fbrApp, 0, 0, 0);
  }

private:
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(FareByRuleAppValidatorTest);
};
