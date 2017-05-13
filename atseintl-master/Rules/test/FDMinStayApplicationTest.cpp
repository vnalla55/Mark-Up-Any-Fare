//----------------------------------------------------------------------------
//  File: FDMinStayApplicationTest.cpp
//
//  Author: Partha Kumar Chakraborti
//  Created:      03/30/2005
//  Description:  This is a unit test class for FDMinStayApplication.cpp
//
//  Copyright Sabre 2005
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"
#include "Common/DateTime.h"
#include "Common/FareDisplayUtil.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MinStayRestriction.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "Rules/FDMinStayApplication.h"
#include "test/testdata/TestLocFactory.h"

using namespace boost;

namespace tse
{

class FDMinStayApplicationTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FDMinStayApplicationTest);

  CPPUNIT_TEST(testValidateFailsIfNoFareDisplayTrx);
  CPPUNIT_TEST(testValidateShouldSkipIfUnableToGetMinStayRestriction);
  CPPUNIT_TEST(testValidateShouldFailIfNoFareDisplayInfo);
  CPPUNIT_TEST(testValidateShouldSkipIfInvalidOrEmptyReturnDate);
  CPPUNIT_TEST(testValidateShouldPassWithDollarsIfMinStayNotADayOfWeek);
  CPPUNIT_TEST_SUITE_END();

public:
  void testValidateFailsIfNoFareDisplayTrx()
  {
    FDMinStayApplication fdMinStayApplication;
    Record3ReturnTypes ret = PASS;
    PricingTrx pricingTrx;
    Itin itin;
    PaxTypeFare paxTypeFare;
    FareMarket fareMarket;

    ret = fdMinStayApplication.validate(pricingTrx, itin, paxTypeFare, NULL, fareMarket, true);

    CPPUNIT_ASSERT_EQUAL(FAIL, ret);
  }

  void testValidateShouldSkipIfUnableToGetMinStayRestriction()
  {
    FDMinStayApplication fdMinStayApplication;
    Record3ReturnTypes ret = PASS;
    Itin itin;
    PaxTypeFare paxTypeFare;
    FareMarket fareMarket;
    FareDisplayRequest fareDisplayRequest;
    FareDisplayTrx fareDisplayTrx;
    MinStayRestriction minStayRestriction;
    RuleItemInfo rule;

    fareDisplayTrx.setRequest(&fareDisplayRequest);

    ret = fdMinStayApplication.validate(fareDisplayTrx, itin, paxTypeFare, NULL, fareMarket, true);

    CPPUNIT_ASSERT_EQUAL(SKIP, ret);

    ret = fdMinStayApplication.validate(fareDisplayTrx, itin, paxTypeFare, &rule, fareMarket, true);

    CPPUNIT_ASSERT_EQUAL(SKIP, ret);
  }

  void testValidateShouldFailIfNoFareDisplayInfo()
  {
    FDMinStayApplication fdMinStayApplication;
    Record3ReturnTypes ret = PASS;
    Itin itin;
    PaxTypeFare paxTypeFare;
    FareMarket fareMarket;
    FareDisplayRequest fareDisplayRequest;
    FareDisplayTrx fareDisplayTrx;
    MinStayRestriction minStayRestriction;

    fareDisplayTrx.setRequest(&fareDisplayRequest);
    RuleItemInfo* rule = &minStayRestriction;

    ret = fdMinStayApplication.validate(fareDisplayTrx, itin, paxTypeFare, rule, fareMarket, true);

    CPPUNIT_ASSERT_EQUAL(FAIL, ret);
  }

  void testValidateShouldSkipIfInvalidOrEmptyReturnDate()
  {
    FDMinStayApplication fdMinStayApplication;
    Record3ReturnTypes ret = PASS;
    Itin itin;
    PaxTypeFare paxTypeFare;
    FareMarket fareMarket;
    FareDisplayRequest fareDisplayRequest;
    FareDisplayTrx fareDisplayTrx;
    FareDisplayOptions options;

    fareDisplayTrx.setRequest(&fareDisplayRequest);
    fareDisplayTrx.setOptions(&options);

    std::vector<FareMarket*>& fareMarkets = itin.fareMarket();
    std::vector<TravelSeg*>& itinTravelSegs = itin.travelSeg();

    // ---------------------------------------------------
    //  Create Rule data
    // ---------------------------------------------------

    DateTime testDate(2005, tse::Aug, 25, 22, 15, 34);

    MinStayRestriction rule;
    rule.minStayDate() = testDate; // neg_infin;
    rule.geoTblItemNoFrom() = 1234;
    rule.geoTblItemNoTo() = 5678;
    rule.minStay() = "TUE";
    rule.minStayUnit() = "02";

    // ---------------------------------------------------
    //  Create Fare Market
    // ---------------------------------------------------

    // FareMarket fm1;
    fareMarket.origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    fareMarket.destination() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");
    fareMarket.geoTravelType() = GeoTravelType::Domestic;
    fareMarket.direction() = FMDirection::OUTBOUND;

    // ---------------------------------------------------
    //  Create Air Travel Segs
    // ---------------------------------------------------

    AirSeg airSeg0;
    DateTime date1(2005, tse::Aug, 27);
    DateTime departureDate(date1, posix_time::hours(11) + posix_time::minutes(15));
    airSeg0.departureDT() = departureDate;
    airSeg0.origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    airSeg0.destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");
    airSeg0.segmentOrder() = 0;

    // ---------------------------------------------------
    //  Associate Travel Segs to Itin and FareMarket
    // ---------------------------------------------------

    itinTravelSegs.push_back(&airSeg0);
    fareMarket.travelSeg().push_back(&airSeg0);

    // ---------------------------------------------------
    //  Create Fare
    // ---------------------------------------------------

    Fare fare1;
    FareInfo fareInfo;
    fareInfo._market1 = "DFW";
    fareInfo._market2 = "NYC";
    fareInfo._directionality = FROM;
    fare1.initialize(Fare::FS_Domestic, &fareInfo, fareMarket);

    // ---------------------------------------------------
    //  Create PaxType Fare
    // ---------------------------------------------------

    paxTypeFare.setFare(&fare1);
    paxTypeFare.fareMarket() = &fareMarket;

    // ---------------------------------------------------
    //  Associate fareMarket to itin
    // ---------------------------------------------------

    fareMarkets.push_back(&fareMarket);

    // ---------------------------------------------------
    // Associating FareMarket with PaxTypeFare
    // ---------------------------------------------------

    fareMarket.allPaxTypeFare().push_back(&paxTypeFare);

    // ---------------------------------------------------
    //  Create FareDisplayInfo object
    // ---------------------------------------------------

    // Build FareDisplayInfo object.
    FareDisplayUtil::initFareDisplayInfo(&fareDisplayTrx, paxTypeFare);

    // -------------------------------------------------------------------------
    //  Get newly created FareDisplayInfo object
    // -------------------------------------------------------------------------

    FareDisplayInfo* fareDisplayInfo = paxTypeFare.fareDisplayInfo();

    CPPUNIT_ASSERT(fareDisplayInfo != NULL);

    ret = fdMinStayApplication.validate(fareDisplayTrx, itin, paxTypeFare, &rule, fareMarket, true);

    CPPUNIT_ASSERT_EQUAL(SKIP, ret);
  }

  // ==================================================
  // Expected output: ret = PASS
  // ==================================================
  void testValidateShouldPassWithDollarsIfMinStayNotADayOfWeek()
  {
    const std::string threeDollar = "$$$"; // 3 dollar

    FDMinStayApplication fdMinStayApplication;
    Record3ReturnTypes ret = PASS;

    Itin itin;
    PaxTypeFare paxTypeFare;
    FareMarket fareMarket;

    FareDisplayRequest fareDisplayRequest;
    FareDisplayOptions options;
    FareDisplayTrx fareDisplayTrx;
    fareDisplayTrx.setRequest(&fareDisplayRequest);
    fareDisplayTrx.setOptions(&options);

    MinStayRestriction minStayRestriction;
    RuleItemInfo* rule = &minStayRestriction;

    std::vector<FareMarket*>& fareMarkets = itin.fareMarket();
    std::vector<TravelSeg*>& itinTravelSegs = itin.travelSeg();

    // ------------------------------------
    //  Fill up all fields with data
    // ------------------------------------
    options.validateRules() = 'T';

    // ---------------------------------------------------
    //  Create Rule data -- minStay as Month
    // ---------------------------------------------------

    DateTime testDate(2005, tse::Aug, 25, 22, 15, 34);

    MinStayRestriction* minStayRule = dynamic_cast<MinStayRestriction*>(rule);
    minStayRule->minStayDate() = testDate; // neg_infin;
    minStayRule->geoTblItemNoFrom() = 1234;
    minStayRule->geoTblItemNoTo() = 5678;
    minStayRule->minStay() = "JUN";
    minStayRule->minStayUnit() = "02";

    // ---------------------------------------------------
    //  Create Fare Market
    // ---------------------------------------------------

    // FareMarket fm1;
    fareMarket.origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    fareMarket.destination() =
        TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");
    fareMarket.geoTravelType() = GeoTravelType::Domestic;
    fareMarket.direction() = FMDirection::OUTBOUND;

    // ---------------------------------------------------
    //  Create Air Travel Segs
    // ---------------------------------------------------

    AirSeg airSeg0;
    DateTime date1(2005, tse::Aug, 27);
    DateTime departureDate(date1, posix_time::hours(11) + posix_time::minutes(15));
    airSeg0.departureDT() = departureDate;
    airSeg0.origin() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocDFW.xml");
    airSeg0.destination() = TestLocFactory::create("/vobs/atseintl/test/testdata/data/LocNYC.xml");
    airSeg0.segmentOrder() = 0;

    // ---------------------------------------------------
    //  Associate Travel Segs to Itin and FareMarket
    // ---------------------------------------------------

    itinTravelSegs.push_back(&airSeg0);
    fareMarket.travelSeg().push_back(&airSeg0);

    // ---------------------------------------------------
    //  Create Fare
    // ---------------------------------------------------

    Fare fare1;
    FareInfo fareInfo;
    fareInfo._market1 = "DFW";
    fareInfo._market2 = "NYC";
    fareInfo._directionality = FROM;
    fare1.initialize(Fare::FS_Domestic, &fareInfo, fareMarket);

    // ---------------------------------------------------
    //  Create PaxType Fare
    // ---------------------------------------------------

    paxTypeFare.setFare(&fare1);
    paxTypeFare.fareMarket() = &fareMarket;

    // ---------------------------------------------------
    //  Associate fareMarket to itin
    // ---------------------------------------------------

    fareMarkets.push_back(&fareMarket);

    // ---------------------------------------------------
    // Associating FareMarket with PaxTypeFare
    // ---------------------------------------------------

    fareMarket.allPaxTypeFare().push_back(&paxTypeFare);

    // ---------------------------------------------------
    //  Create FareDisplayInfo object
    // ---------------------------------------------------

    // Build FareDisplayInfo object.
    FareDisplayUtil::initFareDisplayInfo(&fareDisplayTrx, paxTypeFare);

    // -------------------------------------------------------------------------
    //  Get newly created FareDisplayInfo object
    // -------------------------------------------------------------------------

    FareDisplayInfo* fareDisplayInfo = paxTypeFare.fareDisplayInfo();

    CPPUNIT_ASSERT(fareDisplayInfo != NULL);

    // ---------------------------------------------------
    //  Set current value of FareDisplayInfo->minStay = " 94"
    // ---------------------------------------------------

    fareDisplayInfo->minStay() = " 94"; // Set 94 days

    // ------------------------------------------------------
    //    Fillup Return Date
    // ------------------------------------------------------

    DateTime date2(2009, tse::Aug, 29);
    DateTime returnDate1(date2, posix_time::hours(11) + posix_time::minutes(15));
    fareDisplayTrx.getRequest()->returnDate() = returnDate1;

    // ---------------------------------------------------
    //  Call validate method to verify whether MultiMinStay exists
    //      A. minStay = " 94"
    //  Expected result
    //      A. ret = PASS
    //      B. FareDisplayInfo->MinStay = $$$
    // ---------------------------------------------------

    ret = fdMinStayApplication.validate(fareDisplayTrx, itin, paxTypeFare, rule, fareMarket, false);

    // FIXME -- Is currently returning SKIP
    CPPUNIT_ASSERT_EQUAL(PASS, ret);
    CPPUNIT_ASSERT_EQUAL(threeDollar, fareDisplayInfo->minStay());
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(FDMinStayApplicationTest);
}
