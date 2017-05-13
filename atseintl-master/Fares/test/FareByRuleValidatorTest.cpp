//-------------------------------------------------------------------
//
//  File:        FareByRuleValidatorTest.cpp
//  Created:     May 24, 2004
//  Design:
//  Authors:     Doug Boeving
//
//  Description:
//
//  Updates:
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TseEnums.h"
#include "DataModel/Agent.h"
#include "DataModel/FareMarket.h"
#include "Fares/FareByRuleValidator.h"
#include "DataModel/PricingOptions.h"
#include "DBAccess/Loc.h"
#include "Common/LocUtil.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/FareByRuleApp.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingTrx.h"
#include "Fares/FareByRuleProcessingInfo.h"
#include "DataModel/PricingRequest.h"
#include "DBAccess/FareByRuleCtrlInfo.h"
#include "Rules/RuleApplicationBase.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/DiagManager.h"
#include "DBAccess/Customer.h"
#include "Diagnostic/DiagManager.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "DBAccess/DateOverrideRuleItem.h"
#include "test/include/TestConfigInitializer.h"

using namespace tse;
using namespace std;

namespace tse
{
namespace
{
class MyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;

public:
  const std::vector<DateOverrideRuleItem*>&
  getDateOverrideRuleItem(const VendorCode& vendor,
                          int itemNumber,
                          const DateTime& applDate = DateTime::emptyDate())
  {
    if (vendor == "ATP" && itemNumber == 17489)
    {
      std::vector<DateOverrideRuleItem*>* ret =
          _memHandle.create<std::vector<DateOverrideRuleItem*> >();
      DateOverrideRuleItem* i = _memHandle.create<DateOverrideRuleItem>();
      i->vendor() = "ATP";
      i->itemNo() = 17489;
      i->tvlEffDate() = DateTime(1980, 1, 1);
      i->tvlDiscDate() = DateTime(2003, 10, 31);
      i->tktEffDate() = DateTime(1980, 1, 1);
      i->tktDiscDate() = DateTime(9999, 12, 31);
      i->resEffDate() = DateTime(1980, 1, 1);
      i->resDiscDate() = DateTime(9999, 12, 31);
      return *ret;
    }
    return DataHandleMock::getDateOverrideRuleItem(vendor, itemNumber, applDate);
  }
  bool isStateInArea(const std::string& nationState, const LocCode& area)
  {
    if (nationState == "USTX" && area == "1")
      return true;
    return DataHandleMock::isStateInArea(nationState, area);
  }
  bool isStateInSubArea(const std::string& nationState, const LocCode& subArea)
  {
    if (nationState == "USTX" && subArea == "11")
      return true;
    return DataHandleMock::isStateInSubArea(nationState, subArea);
  }
  bool
  isStateInZone(const VendorCode& vendor, int zone, char zoneType, const std::string& nationState)
  {
    if (vendor == "ATP" && zone == 0 && zoneType == 'R' && nationState == "USTX")
      return true;
    return DataHandleMock::isStateInZone(vendor, zone, zoneType, nationState);
  }
};
}
class FareByRuleValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareByRuleValidatorTest);

  CPPUNIT_TEST(testValidate);
  CPPUNIT_TEST(testMatchPaxType);
  CPPUNIT_TEST(testMatchUnavaiTag);
  CPPUNIT_TEST(testMatchPassengerStatusTrueIfPaxLocTypeEqualNationPaxIndicator);
  CPPUNIT_TEST(testMatchPassengerStatusTrueIfPaxLocAreaInNation);
  CPPUNIT_TEST(testMatchPassengerStatus);
  CPPUNIT_TEST(testMatchNbrOfFltSegs);
  CPPUNIT_TEST(testMatchWhollyWithin);
  CPPUNIT_TEST(testMatchWhollyWithin2);
  CPPUNIT_TEST(testMatchWhollyWithin3);
  CPPUNIT_TEST(testMatchWhollyWithin4);
  CPPUNIT_TEST(testCheckResultingInfo);
  CPPUNIT_TEST(testCheckResultingInfoAxessDispTypeLTC);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _fbrv = _memHandle.create<FareByRuleValidator>();
    _trx = _memHandle.create<PricingTrx>();
    _options = _memHandle.create<PricingOptions>();
    _request = _memHandle.create<PricingRequest>();
    _trx->setOptions(_options);
    _trx->setRequest(_request);
  }

  void tearDown() { _memHandle.clear(); }

  //-----------------------------------------------------------------------------
  //  testValidate()
  //-----------------------------------------------------------------------------
  void testValidate()
  {
    MyDataHandle mdh;
    FareByRuleProcessingInfo fbrProcessingInfo;
    FareByRuleItemInfo fbrItemInfo;
    FareByRuleApp fbrApp;
    FareMarket fareMarket;

    fbrItemInfo.paxType() = "NEG";
    fbrApp.primePaxType() = "ADT";

    fbrProcessingInfo.fbrItemInfo() = &fbrItemInfo;
    fbrProcessingInfo.trx() = _trx;
    fbrProcessingInfo.fbrApp() = &fbrApp;
    fbrProcessingInfo.fareMarket() = &fareMarket;

    _trx->diagnostic().diagnosticType() = Diagnostic325;
    _trx->diagnostic().activate();

    DiagManager diag(*_trx, _trx->diagnostic().diagnosticType());
    fbrProcessingInfo.diagManager() = &diag;

    CPPUNIT_ASSERT(_fbrv->validate(fbrProcessingInfo) == FAIL);

    fbrItemInfo.paxType() = "ADT";
    fbrItemInfo.vendor() = "ATP";
    fbrItemInfo.overrideDateTblItemNo() = 0;

    std::string tmp("2009-08-15 11:55:47");
    DateTime tktDate(tmp);
    _request->ticketingDT() = tktDate;

    AirSeg airseg;
    airseg.departureDT() = tktDate;
    fareMarket.travelSeg().push_back(&airseg);

    Agent agent;
    _trx->getRequest()->ticketingAgent() = &agent;

    fbrItemInfo.overrideDateTblItemNo() = 17489;
    fbrItemInfo.unavailtag() = RuleApplicationBase::dataUnavailable;
    CPPUNIT_ASSERT(_fbrv->validate(fbrProcessingInfo) == FAIL);

    fbrItemInfo.overrideDateTblItemNo() = 0;
    fbrItemInfo.unavailtag() = BLANK;
    fbrItemInfo.passengerInd() = BLANK;
    fbrItemInfo.fltSegCnt() = 0;
    fbrItemInfo.whollyWithinLoc().locType() = BLANK;
    fbrItemInfo.discountInd() = FareByRuleValidator::NO_DISCOUNT;
    CPPUNIT_ASSERT(_fbrv->validate(fbrProcessingInfo) == STOP);

    fbrItemInfo.discountInd() = BLANK;
    fbrItemInfo.minMileage() = 0;
    fbrItemInfo.maxMileage() = 0;
    fbrItemInfo.resultglobalDir() = GlobalDirection::XX;
    CPPUNIT_ASSERT(_fbrv->validate(fbrProcessingInfo) == PASS);
  }

  //-----------------------------------------------------------------------------
  //  testMatchPaxType()
  //-----------------------------------------------------------------------------
  void testMatchPaxType()
  {
    FareByRuleItemInfo fbrItemInfo;
    FareByRuleApp fbrApp;

    fbrItemInfo.paxType() = "ADT";

    fbrApp.primePaxType() = "ADT";

    CPPUNIT_ASSERT(_fbrv->matchPaxType(fbrItemInfo, fbrApp));

    fbrApp.primePaxType() = "MIL";

    CPPUNIT_ASSERT(!_fbrv->matchPaxType(fbrItemInfo, fbrApp));

    fbrItemInfo.paxType() = "MIL";

    CPPUNIT_ASSERT(_fbrv->matchPaxType(fbrItemInfo, fbrApp));
  }

  //-----------------------------------------------------------------------------
  //  testMatchUnavaiTag()
  //-----------------------------------------------------------------------------
  void testMatchUnavaiTag()
  {
    FareByRuleItemInfo fbrItemInfo;
    fbrItemInfo.negPsgstatusInd() = FareByRuleValidator::NO_DISCOUNT;

    Indicator x = 'X';
    Indicator y = 'Y';
    Indicator z = 'Z';

    fbrItemInfo.unavailtag() = x;

    CPPUNIT_ASSERT(!_fbrv->matchUnavaiTag(fbrItemInfo));

    fbrItemInfo.unavailtag() = y;

    CPPUNIT_ASSERT(!_fbrv->matchUnavaiTag(fbrItemInfo));

    fbrItemInfo.unavailtag() = z;

    CPPUNIT_ASSERT(_fbrv->matchUnavaiTag(fbrItemInfo));

    fbrItemInfo.unavailtag() = ' ';

    CPPUNIT_ASSERT(_fbrv->matchUnavaiTag(fbrItemInfo));
  }

  void testMatchPassengerStatusTrueIfPaxLocTypeEqualNationPaxIndicator()
  {
    FareByRuleProcessingInfo fbrProcessingInfo;
    FareMarket fareMarket;
    fbrProcessingInfo.fareMarket() = &fareMarket;

    PaxType pt;
    pt.paxType() = "GST";
    pt.stateCode() = "TX";

    PaxTypeBucket paxTypeCortege;
    paxTypeCortege.requestedPaxType() = &pt;
    fareMarket.paxTypeCortege().push_back(paxTypeCortege);
    fareMarket.geoTravelType() = GeoTravelType::UnknownGeoTravelType;

    FareByRuleItemInfo fbrItemInfo;
    fbrItemInfo.negPsgstatusInd() = BLANK;
    fbrItemInfo.passengerInd() = LocUtil::PAX_SHIP_REGISTRY;
    fbrItemInfo.vendor() = "ATP";

    _options->fareByRuleShipRegistry() = "US";
    _options->nationality() = "US";
    _options->residency() = "US";
    _options->employment() = "US";

    // test match
    LocKey lk;
    lk.loc() = "US";
    lk.locType() = LOCTYPE_NATION;
    fbrItemInfo.psgLoc1() = lk;

    CPPUNIT_ASSERT(_fbrv->matchPassengerStatus(fbrItemInfo, *_trx, fbrProcessingInfo));
  }

  void testMatchPassengerStatusTrueIfPaxLocAreaInNation()
  {
    MyDataHandle mdh;
    FareByRuleProcessingInfo fbrProcessingInfo;
    FareMarket fareMarket;
    fbrProcessingInfo.fareMarket() = &fareMarket;

    PaxType pt;
    pt.paxType() = "GST";
    pt.stateCode() = "TX";

    PaxTypeBucket paxTypeCortege;
    paxTypeCortege.requestedPaxType() = &pt;
    fareMarket.paxTypeCortege().push_back(paxTypeCortege);
    fareMarket.geoTravelType() = GeoTravelType::UnknownGeoTravelType;

    FareByRuleItemInfo fbrItemInfo;
    fbrItemInfo.negPsgstatusInd() = BLANK;
    fbrItemInfo.passengerInd() = LocUtil::PAX_SHIP_REGISTRY;
    fbrItemInfo.vendor() = "ATP";

    _options->fareByRuleShipRegistry() = "US";
    _options->nationality() = "US";
    _options->residency() = "US";
    _options->employment() = "US";

    // test match
    LocKey lk;
    lk.loc() = "1";
    lk.locType() = LOCTYPE_AREA;
    fbrItemInfo.psgLoc1() = lk;

    CPPUNIT_ASSERT(_fbrv->matchPassengerStatus(fbrItemInfo, *_trx, fbrProcessingInfo));
  }

  //-----------------------------------------------------------------------------
  //  testMatchPassengerStatus()
  //-----------------------------------------------------------------------------
  void testMatchPassengerStatus()
  {
    MyDataHandle mdh;
    FareByRuleProcessingInfo fbrProcessingInfo;
    FareMarket fareMarket;
    fbrProcessingInfo.fareMarket() = &fareMarket;

    PaxType pt;
    pt.paxType() = "GST";
    pt.stateCode() = "TX";

    PaxTypeBucket paxTypeCortege;
    paxTypeCortege.requestedPaxType() = &pt;
    fareMarket.paxTypeCortege().push_back(paxTypeCortege);
    fareMarket.geoTravelType() = GeoTravelType::UnknownGeoTravelType;

    FareByRuleItemInfo fbrItemInfo;
    fbrItemInfo.negPsgstatusInd() = BLANK;
    fbrItemInfo.passengerInd() = LocUtil::PAX_SHIP_REGISTRY;
    fbrItemInfo.vendor() = "ATP";

    _options->fareByRuleShipRegistry() = "US";
    _options->nationality() = "US";
    _options->residency() = "US";
    _options->employment() = "US";

    // test match
    LocKey lk;
    lk.loc() = "11";
    lk.locType() = LOCTYPE_SUBAREA;
    fbrItemInfo.psgLoc1() = lk;
    CPPUNIT_ASSERT(_fbrv->matchPassengerStatus(fbrItemInfo, *_trx, fbrProcessingInfo));

    lk.loc() = "00000";
    lk.locType() = LOCTYPE_ZONE;
    fbrItemInfo.psgLoc1() = lk;
    CPPUNIT_ASSERT(_fbrv->matchPassengerStatus(fbrItemInfo, *_trx, fbrProcessingInfo));

    // test not match
    lk.loc() = "CA";
    lk.locType() = LOCTYPE_NATION;
    fbrItemInfo.psgLoc1() = lk;
    CPPUNIT_ASSERT(!_fbrv->matchPassengerStatus(fbrItemInfo, *_trx, fbrProcessingInfo));

    fbrItemInfo.passengerInd() = LocUtil::PAX_NATIONALITY;
    lk.loc() = "US";
    lk.locType() = LOCTYPE_NATION;
    fbrItemInfo.psgLoc1() = lk;
    CPPUNIT_ASSERT(_fbrv->matchPassengerStatus(fbrItemInfo, *_trx, fbrProcessingInfo));

    // test Rec 3 Cat 25 pax status is smaller than input
    lk.loc() = "USMO";
    lk.locType() = LOCTYPE_STATE;
    fbrItemInfo.psgLoc1() = lk;
    CPPUNIT_ASSERT(!_fbrv->matchPassengerStatus(fbrItemInfo, *_trx, fbrProcessingInfo));

    lk.loc() = "DFW";
    lk.locType() = LOCTYPE_CITY;
    fbrItemInfo.psgLoc1() = lk;
    CPPUNIT_ASSERT(!_fbrv->matchPassengerStatus(fbrItemInfo, *_trx, fbrProcessingInfo));

    // test input with state - WPNT/USTX
    _options->nationality() = "USTX";

    lk.loc() = "USTX";
    lk.locType() = LOCTYPE_STATE;
    fbrItemInfo.psgLoc1() = lk;
    CPPUNIT_ASSERT(_fbrv->matchPassengerStatus(fbrItemInfo, *_trx, fbrProcessingInfo));

    lk.loc() = "USMO";
    lk.locType() = LOCTYPE_STATE;
    fbrItemInfo.psgLoc1() = lk;
    CPPUNIT_ASSERT(!_fbrv->matchPassengerStatus(fbrItemInfo, *_trx, fbrProcessingInfo));

    lk.loc() = "1";
    lk.locType() = LOCTYPE_AREA;
    fbrItemInfo.psgLoc1() = lk;
    CPPUNIT_ASSERT(_fbrv->matchPassengerStatus(fbrItemInfo, *_trx, fbrProcessingInfo));

    lk.loc() = "11";
    lk.locType() = LOCTYPE_SUBAREA;
    fbrItemInfo.psgLoc1() = lk;
    CPPUNIT_ASSERT(_fbrv->matchPassengerStatus(fbrItemInfo, *_trx, fbrProcessingInfo));

    lk.loc() = "00000";
    lk.locType() = LOCTYPE_ZONE;
    fbrItemInfo.psgLoc1() = lk;
    CPPUNIT_ASSERT(_fbrv->matchPassengerStatus(fbrItemInfo, *_trx, fbrProcessingInfo));

    lk.loc() = "DFW";
    lk.locType() = LOCTYPE_CITY;
    fbrItemInfo.psgLoc1() = lk;
    CPPUNIT_ASSERT(!_fbrv->matchPassengerStatus(fbrItemInfo, *_trx, fbrProcessingInfo));

    fbrItemInfo.passengerInd() = LocUtil::PAX_RESIDENCY;
    lk.loc() = "US";
    lk.locType() = LOCTYPE_NATION;
    fbrItemInfo.psgLoc1() = lk;
    CPPUNIT_ASSERT(_fbrv->matchPassengerStatus(fbrItemInfo, *_trx, fbrProcessingInfo));

    fbrItemInfo.passengerInd() = LocUtil::PAX_EMPLOYEE;
    lk.loc() = "US";
    lk.locType() = LOCTYPE_NATION;
    fbrItemInfo.psgLoc1() = lk;
    CPPUNIT_ASSERT(_fbrv->matchPassengerStatus(fbrItemInfo, *_trx, fbrProcessingInfo));

    // test match with negative
    fbrItemInfo.negPsgstatusInd() = LocUtil::NOT_ALLOWED;
    CPPUNIT_ASSERT(!_fbrv->matchPassengerStatus(fbrItemInfo, *_trx, fbrProcessingInfo));

    // test not match with negative
    fbrItemInfo.passengerInd() = LocUtil::PAX_EMPLOYEE;
    lk.loc() = "CA";
    lk.locType() = LOCTYPE_NATION;
    fbrItemInfo.psgLoc1() = lk;
    CPPUNIT_ASSERT(_fbrv->matchPassengerStatus(fbrItemInfo, *_trx, fbrProcessingInfo));

    // test input with no qualifier - FAIL if positive but MATCH if negative
    _options->fareByRuleShipRegistry() = "  ";
    _options->nationality() = "  ";
    _options->residency() = "  ";
    _options->employment() = "  ";

    fbrItemInfo.passengerInd() = LocUtil::PAX_SHIP_REGISTRY;
    fbrItemInfo.negPsgstatusInd() = LocUtil::NOT_ALLOWED;
    CPPUNIT_ASSERT(_fbrv->matchPassengerStatus(fbrItemInfo, *_trx, fbrProcessingInfo));
    fbrItemInfo.negPsgstatusInd() = BLANK;
    CPPUNIT_ASSERT(!_fbrv->matchPassengerStatus(fbrItemInfo, *_trx, fbrProcessingInfo));

    fbrItemInfo.passengerInd() = LocUtil::PAX_NATIONALITY;
    fbrItemInfo.negPsgstatusInd() = LocUtil::NOT_ALLOWED;
    CPPUNIT_ASSERT(_fbrv->matchPassengerStatus(fbrItemInfo, *_trx, fbrProcessingInfo));
    fbrItemInfo.negPsgstatusInd() = BLANK;
    CPPUNIT_ASSERT(!_fbrv->matchPassengerStatus(fbrItemInfo, *_trx, fbrProcessingInfo));

    fbrItemInfo.passengerInd() = LocUtil::PAX_RESIDENCY;
    fbrItemInfo.negPsgstatusInd() = LocUtil::NOT_ALLOWED;
    CPPUNIT_ASSERT(_fbrv->matchPassengerStatus(fbrItemInfo, *_trx, fbrProcessingInfo));
    fbrItemInfo.negPsgstatusInd() = BLANK;
    CPPUNIT_ASSERT(!_fbrv->matchPassengerStatus(fbrItemInfo, *_trx, fbrProcessingInfo));

    fbrItemInfo.passengerInd() = LocUtil::PAX_EMPLOYEE;
    fbrItemInfo.negPsgstatusInd() = LocUtil::NOT_ALLOWED;
    CPPUNIT_ASSERT(_fbrv->matchPassengerStatus(fbrItemInfo, *_trx, fbrProcessingInfo));
    fbrItemInfo.negPsgstatusInd() = BLANK;
    CPPUNIT_ASSERT(!_fbrv->matchPassengerStatus(fbrItemInfo, *_trx, fbrProcessingInfo));
  }

  //-----------------------------------------------------------------------------
  //  testMatchNbrOfFltSegs()
  //-----------------------------------------------------------------------------
  void testMatchNbrOfFltSegs()
  {
    FareByRuleItemInfo fbrItemInfo;
    fbrItemInfo.negPsgstatusInd() = FareByRuleValidator::NO_DISCOUNT;
    FareMarket fareMarket;
    AirSeg ts1;
    AirSeg ts2;
    AirSeg ts3;

    fareMarket.travelSeg().push_back(&ts1);
    fareMarket.travelSeg().push_back(&ts2);
    fareMarket.travelSeg().push_back(&ts3);

    fbrItemInfo.fltSegCnt() = 3;

    CPPUNIT_ASSERT(_fbrv->matchNbrOfFltSegs(fbrItemInfo, fareMarket));

    fbrItemInfo.fltSegCnt() = 2;

    CPPUNIT_ASSERT(!_fbrv->matchNbrOfFltSegs(fbrItemInfo, fareMarket));
  }

  //-----------------------------------------------------------------------------
  //  testMatchWhollyWithin()
  //-----------------------------------------------------------------------------
  void testMatchWhollyWithin()
  {
    FareByRuleItemInfo fbrItemInfo;
    FareMarket fareMarket;

    std::string tmp("2009-08-15 11:55:47");

    DateTime tktDate(tmp);
    _request->ticketingDT() = tktDate;

    LocKey whollyLocKey;

    whollyLocKey.loc() = "US";
    whollyLocKey.locType() = LOCTYPE_NATION;

    fbrItemInfo.whollyWithinLoc() = whollyLocKey;
    fbrItemInfo.vendor() = "ATP";

    AirSeg ts1;
    AirSeg ts2;
    AirSeg ts3;

    std::vector<const Loc*> hiddenStops;

    Loc l1;
    Loc l2;

    l1.loc() = "DB";
    l1.nation() = "US";
    l1.state() = "DB";

    l2.loc() = "SB";
    l2.nation() = "US";
    l2.state() = "SB";

    hiddenStops.push_back(&l1);
    hiddenStops.push_back(&l2);

    Loc origin;
    Loc dest;

    origin.loc() = "DFW";
    origin.nation() = "US";
    origin.state() = "TX";

    dest.loc() = "LAX";
    dest.nation() = "US";
    dest.state() = "CA";

    ts1.origin() = (&origin);
    ts1.destination() = (&dest);

    ts2.origin() = (&origin);
    ts2.destination() = (&dest);

    ts3.origin() = (&origin);
    ts3.destination() = (&dest);

    ts1.hiddenStops() = (hiddenStops);
    ts2.hiddenStops() = (hiddenStops);
    ts3.hiddenStops() = (hiddenStops);

    fareMarket.travelSeg().push_back(&ts1);
    fareMarket.travelSeg().push_back(&ts2);
    fareMarket.travelSeg().push_back(&ts3);

    CPPUNIT_ASSERT(_fbrv->matchWhollyWithin(*_trx, fbrItemInfo, fareMarket));

    fbrItemInfo.whollyWithinLoc().locType() = BLANK;
    CPPUNIT_ASSERT(_fbrv->matchWhollyWithin(*_trx, fbrItemInfo, fareMarket));

    fbrItemInfo.whollyWithinLoc().locType() = LOCTYPE_CITY;
    CPPUNIT_ASSERT(!_fbrv->matchWhollyWithin(*_trx, fbrItemInfo, fareMarket));
  }

  //-----------------------------------------------------------------------------
  //  testMatchWhollyWithin2()
  //-----------------------------------------------------------------------------
  void testMatchWhollyWithin2()
  {
    FareByRuleItemInfo fbrItemInfo;
    FareMarket fareMarket;

    std::string tmp("2009-08-15 11:55:47");

    DateTime tktDate(tmp);
    _request->ticketingDT() = tktDate;

    LocKey whollyLocKey;

    whollyLocKey.loc() = "US";
    whollyLocKey.locType() = LOCTYPE_NATION;

    fbrItemInfo.whollyWithinLoc() = whollyLocKey;
    fbrItemInfo.vendor() = "ATP";

    AirSeg ts1;
    AirSeg ts2;
    AirSeg ts3;

    Loc origin;
    Loc dest;

    origin.loc() = "LON";
    origin.nation() = "GB";
    origin.state() = "  ";

    dest.loc() = "LAX";
    dest.nation() = "US";
    dest.state() = "CA";

    ts1.origin() = (&origin);
    ts1.destination() = (&dest);

    ts2.origin() = (&origin);
    ts2.destination() = (&dest);

    ts3.origin() = (&origin);
    ts3.destination() = (&dest);

    fareMarket.travelSeg().push_back(&ts1);
    fareMarket.travelSeg().push_back(&ts2);
    fareMarket.travelSeg().push_back(&ts3);

    CPPUNIT_ASSERT(!_fbrv->matchWhollyWithin(*_trx, fbrItemInfo, fareMarket));
  }

  //-----------------------------------------------------------------------------
  //  testMatchWhollyWithin3()
  //-----------------------------------------------------------------------------
  void testMatchWhollyWithin3()
  {
    FareByRuleItemInfo fbrItemInfo;
    FareMarket fareMarket;

    std::string tmp("2009-08-15 11:55:47");

    DateTime tktDate(tmp);
    _request->ticketingDT() = tktDate;

    LocKey whollyLocKey;

    whollyLocKey.loc() = "US";
    whollyLocKey.locType() = LOCTYPE_NATION;

    fbrItemInfo.whollyWithinLoc() = whollyLocKey;
    fbrItemInfo.vendor() = "ATP";

    AirSeg ts1;
    AirSeg ts2;
    AirSeg ts3;

    Loc origin;
    Loc dest;

    origin.loc() = "DFW";
    origin.nation() = "US";
    origin.state() = "TX";

    dest.loc() = "LON";
    dest.nation() = "GB";
    dest.state() = "  ";

    ts1.origin() = (&origin);
    ts1.destination() = (&dest);

    ts2.origin() = (&origin);
    ts2.destination() = (&dest);

    ts3.origin() = (&origin);
    ts3.destination() = (&dest);

    fareMarket.travelSeg().push_back(&ts1);
    fareMarket.travelSeg().push_back(&ts2);
    fareMarket.travelSeg().push_back(&ts3);

    CPPUNIT_ASSERT(!_fbrv->matchWhollyWithin(*_trx, fbrItemInfo, fareMarket));
  }

  //-----------------------------------------------------------------------------
  //  testMatchWhollyWithin4()
  //-----------------------------------------------------------------------------
  void testMatchWhollyWithin4()
  {
    FareByRuleItemInfo fbrItemInfo;
    FareMarket fareMarket;

    std::string tmp("2009-08-15 11:55:47");

    DateTime tktDate(tmp);
    _request->ticketingDT() = tktDate;

    LocKey whollyLocKey;

    whollyLocKey.loc() = "US";
    whollyLocKey.locType() = LOCTYPE_NATION;

    fbrItemInfo.whollyWithinLoc() = whollyLocKey;
    fbrItemInfo.vendor() = "ATP";

    AirSeg ts1;
    AirSeg ts2;
    AirSeg ts3;

    std::vector<const Loc*> hiddenStops;

    Loc l1;
    Loc l2;

    l1.loc() = "DB";
    l1.nation() = "MX";
    l1.state() = "DB";

    l2.loc() = "SB";
    l2.nation() = "US";
    l2.state() = "SB";

    hiddenStops.push_back(&l1);
    hiddenStops.push_back(&l2);

    Loc origin;
    Loc dest;

    origin.loc() = "DFW";
    origin.nation() = "US";
    origin.state() = "TX";

    dest.loc() = "LAX";
    dest.nation() = "US";
    dest.state() = "CA";

    ts1.origin() = (&origin);
    ts1.destination() = (&dest);

    ts2.origin() = (&origin);
    ts2.destination() = (&dest);

    ts3.origin() = (&origin);
    ts3.destination() = (&dest);

    ts1.hiddenStops() = (hiddenStops);
    ts2.hiddenStops() = (hiddenStops);
    ts3.hiddenStops() = (hiddenStops);

    fareMarket.travelSeg().push_back(&ts1);
    fareMarket.travelSeg().push_back(&ts2);
    fareMarket.travelSeg().push_back(&ts3);

    CPPUNIT_ASSERT(!_fbrv->matchWhollyWithin(*_trx, fbrItemInfo, fareMarket));
  }

  //-----------------------------------------------------------------------------
  //  testCheckResultingInfo()
  //-----------------------------------------------------------------------------
  void testCheckResultingInfo()
  {
    FareMarket fareMarket;
    FareByRuleProcessingInfo fbrProcessingInfo;
    FareByRuleItemInfo fbrItemInfo;

    _options->normalFare() = 'T';

    Agent agent;
    _trx->getRequest()->ticketingAgent() = &agent;

    fbrProcessingInfo.fbrItemInfo() = &fbrItemInfo;
    fbrProcessingInfo.trx() = _trx;
    fbrProcessingInfo.fareMarket() = &fareMarket;

    fareMarket.setGlobalDirection(GlobalDirection::XX);
    fbrItemInfo.resultglobalDir() = GlobalDirection::XX;
    fbrItemInfo.resultpricingcatType() = ' ';

    CPPUNIT_ASSERT(_fbrv->checkResultingInfo(fbrProcessingInfo));

    fbrItemInfo.resultpricingcatType() = 'S';

    CPPUNIT_ASSERT(!_fbrv->checkResultingInfo(fbrProcessingInfo));

    fareMarket.setGlobalDirection(GlobalDirection::AT);
    fbrItemInfo.resultglobalDir() = GlobalDirection::PA;

    CPPUNIT_ASSERT(!_fbrv->checkResultingInfo(fbrProcessingInfo));
  }

  void testCheckResultingInfoAxessDispTypeLTC()
  {
    PricingOptions po;
    po.normalFare() = ' ';
    PricingTrx trx;
    trx.setOptions(&po);

    PricingRequest pr;
    Agent tktAgent;
    Customer customer;
    tktAgent.agentTJR() = &customer;
    tktAgent.agentTJR()->crsCarrier() = "1J";
    tktAgent.agentTJR()->hostName() = "AXES";
    pr.ticketingAgent() = &tktAgent;
    pr.wpNettRequested() = 'Y';
    trx.setRequest(&pr);

    FareMarket fareMarket;
    fareMarket.setGlobalDirection(GlobalDirection::NO_DIR);
    FareByRuleValidator fbrv;
    FareByRuleProcessingInfo fbrProcessingInfo;
    FareByRuleItemInfo fbrItemInfo;

    fbrProcessingInfo.fbrItemInfo() = &fbrItemInfo;
    fbrProcessingInfo.trx() = &trx;
    fbrProcessingInfo.fareMarket() = &fareMarket;

    fbrItemInfo.resultglobalDir() = GlobalDirection::NO_DIR;
    fbrItemInfo.resultpricingcatType() = 'S';
    fbrItemInfo.resultDisplaycatType() = RuleConst::SELLING_FARE;

    CPPUNIT_ASSERT(fbrv.checkResultingInfo(fbrProcessingInfo));

    fbrItemInfo.resultDisplaycatType() = RuleConst::NET_SUBMIT_FARE;
    CPPUNIT_ASSERT(fbrv.checkResultingInfo(fbrProcessingInfo));

    fbrItemInfo.resultDisplaycatType() = RuleConst::NET_SUBMIT_FARE_UPD;
    CPPUNIT_ASSERT(fbrv.checkResultingInfo(fbrProcessingInfo));
  }

private:
  PricingTrx* _trx;
  PricingOptions* _options;
  PricingRequest* _request;
  FareByRuleValidator* _fbrv;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(FareByRuleValidatorTest);
}
