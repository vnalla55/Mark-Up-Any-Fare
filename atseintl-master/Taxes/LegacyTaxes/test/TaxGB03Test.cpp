#include <string>
#include <time.h>
#include <iostream>

#include "Taxes/LegacyTaxes/TaxGB03.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DBAccess/Loc.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/PricingTrx.h"
#include "Common/DateTime.h"
#include "DBAccess/TaxCodeCabin.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Common/TseEnums.h"

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

using namespace std;

namespace tse
{

class TaxGB03Test : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(TaxGB03Test);
  CPPUNIT_TEST(testTaxGB_validateTripTypes);
  CPPUNIT_TEST(testTaxGB_validateTransit);
  CPPUNIT_TEST(testTaxGB_LowPremium_AnyCarrier);
  CPPUNIT_TEST(testTaxGB_HighPremium_AnyCarrier);
  CPPUNIT_TEST(testTaxGB_LowPremium);
  CPPUNIT_TEST(testTaxGB_HighPremium);
  CPPUNIT_TEST(testTaxGB_WithinGB);
  CPPUNIT_TEST(testTaxGB_FromToGB);
  CPPUNIT_TEST(testTaxGB_isForcedStopover);
  CPPUNIT_TEST(testTaxGB_isForcedStopover_Open);
  CPPUNIT_TEST(testTaxGB_isStopOverNoTrainNoStop_Pass);
  CPPUNIT_TEST(testTaxGB_isStopOverTrainIsStop_Pass);
  CPPUNIT_TEST(testIsDomSeg_Dom);
  CPPUNIT_TEST(testIsDomSeg_Int);
  CPPUNIT_TEST(testFindIntSurface_NoSurface);
  CPPUNIT_TEST(testIsNotUKJourney);
  CPPUNIT_TEST(testIsUKJourney);
  CPPUNIT_TEST_SUITE_END();

public:
  void testTaxGB_validateTripTypes()
  {
    uint16_t startIndex = 0;
    uint16_t endIndex = 1;

    TaxCodeReg taxCodeReg;

    bool rc = _taxGB.validateTripTypes(*_trx, *_taxResponse, taxCodeReg, startIndex, endIndex);

    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTaxGB_validateTransit()
  {
    TaxCodeReg taxCodeReg;
    uint16_t travelSegIndex = 0;

    bool rc = _taxGB.validateTransit(*_trx, *_taxResponse, taxCodeReg, travelSegIndex);

    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTaxGB_LowPremium_AnyCarrier()
  {
    // Set up the travel segment

    Loc* origin = _memHandle.create<Loc>();
    origin->loc() = string("LHR");
    origin->nation() = string("GB");

    Loc* destination = _memHandle.create<Loc>();
    destination->loc() = string("ORD");
    destination->nation() = string("US");

    AirSeg* travelSeg = _memHandle.create<AirSeg>();
    travelSeg->origin() = origin;
    travelSeg->destination() = destination;
    travelSeg->carrier() = "AA";
    travelSeg->setBookingCode("Y");

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(travelSeg);

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->itin() = itin;

    // Done setting up the farepath

    _taxResponse = _memHandle.create<TaxResponse>();
    _taxResponse->farePath() = farePath;

    // Start setting up Restriction Transit Table Under TaxCodeReg

    uint16_t travelSegIndex = 0;

    TaxCodeReg taxCodeReg;
    taxCodeReg.taxCode() = "GB3";

    TaxCodeCabin taxCodeCabin;
    taxCodeCabin.carrier() = "";
    taxCodeCabin.classOfService() = "*";
    taxCodeCabin.loc1().locType() = ' ';
    taxCodeCabin.loc1().loc() = ' ';
    taxCodeCabin.loc2().locType() = ' ';
    taxCodeCabin.loc2().loc() = ' ';
    taxCodeCabin.exceptInd() = 'N';

    bool rc = _taxGB.validateFareClass(*_trx, *_taxResponse, taxCodeReg, travelSegIndex);

    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTaxGB_HighPremium_AnyCarrier()
  {
    // Set up the travel segment

    Loc* origin = _memHandle.create<Loc>();
    origin->loc() = string("LHR");
    origin->nation() = string("GB");

    Loc* destination = _memHandle.create<Loc>();
    destination->loc() = string("ORD");
    destination->nation() = string("US");

    AirSeg* travelSeg = _memHandle.create<AirSeg>();
    travelSeg->origin() = origin;
    travelSeg->destination() = destination;
    travelSeg->carrier() = "AA";
    travelSeg->setBookingCode("C");

    // Set up the fare path

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(travelSeg);

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->itin() = itin;

    // Done setting up the farepath

    _taxResponse = _memHandle.create<TaxResponse>();
    _taxResponse->farePath() = farePath;

    // Start setting up Restriction Transit Table Under TaxCodeReg

    uint16_t travelSegIndex = 0;

    TaxCodeReg taxCodeReg;
    taxCodeReg.taxCode() = "GB1";

    TaxCodeCabin taxCodeCabin;
    taxCodeCabin.carrier() = "";
    taxCodeCabin.classOfService() = "C";
    taxCodeCabin.loc1().locType() = ' ';
    taxCodeCabin.loc1().loc() = ' ';
    taxCodeCabin.loc2().locType() = ' ';
    taxCodeCabin.loc2().loc() = ' ';
    taxCodeCabin.exceptInd() = 'N';

    bool rc = _taxGB.validateFareClass(*_trx, *_taxResponse, taxCodeReg, travelSegIndex);

    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTaxGB_LowPremium_AnyCarrier_Except()
  {
    // Set up the travel segment

    Loc* origin = _memHandle.create<Loc>();
    origin->loc() = string("LHR");
    origin->nation() = string("GB");

    Loc* destination = _memHandle.create<Loc>();
    destination->loc() = string("YYZ");
    destination->nation() = string("CA");

    AirSeg* travelSeg = _memHandle.create<AirSeg>();
    travelSeg->origin() = origin;
    travelSeg->destination() = destination;
    travelSeg->carrier() = "AC";
    travelSeg->setBookingCode("C");

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(travelSeg);

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->itin() = itin;

    // Done setting up the farepath

    _taxResponse = _memHandle.create<TaxResponse>();
    _taxResponse->farePath() = farePath;

    // Start setting up Restriction Transit Table Under TaxCodeReg

    uint16_t travelSegIndex = 0;

    TaxCodeReg taxCodeReg;
    taxCodeReg.taxCode() = "GB3";

    TaxCodeCabin taxCodeCabin;
    taxCodeCabin.carrier() = "";
    taxCodeCabin.classOfService() = "C";
    taxCodeCabin.loc1().locType() = ' ';
    taxCodeCabin.loc1().loc() = ' ';
    taxCodeCabin.loc2().locType() = ' ';
    taxCodeCabin.loc2().loc() = ' ';
    taxCodeCabin.exceptInd() = 'Y';

    bool rc = _taxGB.validateFareClass(*_trx, *_taxResponse, taxCodeReg, travelSegIndex);

    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTaxGB_HighPremium_AnyCarrier_Except()
  {
    // Set up the travel segment

    Loc* origin = _memHandle.create<Loc>();
    origin->loc() = string("LHR");
    origin->nation() = string("GB");

    Loc* destination = _memHandle.create<Loc>();
    destination->loc() = string("YYZ");
    destination->nation() = string("CA");

    AirSeg* travelSeg = _memHandle.create<AirSeg>();
    travelSeg->origin() = origin;
    travelSeg->destination() = destination;
    travelSeg->carrier() = "AC";
    travelSeg->setBookingCode("Y");

    // Set up the fare path

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(travelSeg);

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->itin() = itin;

    // Done setting up the farepath

    _taxResponse = _memHandle.create<TaxResponse>();
    _taxResponse->farePath() = farePath;

    // Start setting up Restriction Transit Table Under TaxCodeReg

    uint16_t travelSegIndex = 0;

    TaxCodeReg taxCodeReg;
    taxCodeReg.taxCode() = "GB1";

    TaxCodeCabin taxCodeCabin;
    taxCodeCabin.carrier() = "";
    taxCodeCabin.classOfService() = "*";
    taxCodeCabin.loc1().locType() = ' ';
    taxCodeCabin.loc1().loc() = ' ';
    taxCodeCabin.loc2().locType() = ' ';
    taxCodeCabin.loc2().loc() = ' ';
    taxCodeCabin.exceptInd() = 'Y';

    bool rc = _taxGB.validateFareClass(*_trx, *_taxResponse, taxCodeReg, travelSegIndex);

    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTaxGB_LowPremium()
  {
    // Set up the travel segment

    Loc* origin = _memHandle.create<Loc>();
    origin->loc() = string("LHR");
    origin->nation() = string("GB");

    Loc* destination = _memHandle.create<Loc>();
    destination->loc() = string("MAN");
    destination->nation() = string("GB");

    AirSeg* travelSeg = _memHandle.create<AirSeg>();
    travelSeg->origin() = origin;
    travelSeg->destination() = destination;
    travelSeg->carrier() = "AC";
    travelSeg->setBookingCode("Z");

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(travelSeg);

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->itin() = itin;

    // Done setting up the farepath

    _taxResponse = _memHandle.create<TaxResponse>();
    _taxResponse->farePath() = farePath;

    // Start setting up Restriction Transit Table Under TaxCodeReg

    uint16_t travelSegIndex = 0;

    TaxCodeReg taxCodeReg;
    taxCodeReg.taxCode() = "GB3";

    TaxCodeCabin taxCodeCabin;
    taxCodeCabin.carrier() = "AC";
    taxCodeCabin.classOfService() = "Z";
    taxCodeCabin.directionalInd() = BETWEEN;
    taxCodeCabin.loc1().locType() = ' ';
    taxCodeCabin.loc1().loc() = ' ';
    taxCodeCabin.loc2().locType() = ' ';
    taxCodeCabin.loc2().loc() = ' ';
    taxCodeCabin.exceptInd() = 'N';

    bool rc = _taxGB.validateFareClass(*_trx, *_taxResponse, taxCodeReg, travelSegIndex);

    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTaxGB_HighPremium()
  {
    // Set up the travel segment

    Loc* origin = _memHandle.create<Loc>();
    origin->loc() = string("LHR");
    origin->nation() = string("GB");

    Loc* destination = _memHandle.create<Loc>();
    destination->loc() = string("ORD");
    destination->nation() = string("US");

    AirSeg* travelSeg = _memHandle.create<AirSeg>();
    travelSeg->origin() = origin;
    travelSeg->destination() = destination;
    travelSeg->carrier() = "AA";
    travelSeg->setBookingCode("U");

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(travelSeg);

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->itin() = itin;

    // Done setting up the farepath

    _taxResponse = _memHandle.create<TaxResponse>();
    _taxResponse->farePath() = farePath;

    // Start setting up Restriction Transit Table Under TaxCodeReg

    uint16_t travelSegIndex = 0;

    TaxCodeReg taxCodeReg;
    taxCodeReg.taxCode() = "GB1";

    TaxCodeCabin taxCodeCabin;
    taxCodeCabin.carrier() = "AA";
    taxCodeCabin.classOfService() = "U";
    taxCodeCabin.directionalInd() = BETWEEN;
    taxCodeCabin.loc1().locType() = ' ';
    taxCodeCabin.loc1().loc() = ' ';
    taxCodeCabin.loc2().locType() = ' ';
    taxCodeCabin.loc2().loc() = ' ';
    taxCodeCabin.exceptInd() = 'N';

    bool rc = _taxGB.validateFareClass(*_trx, *_taxResponse, taxCodeReg, travelSegIndex);

    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTaxGB_LowPremium_Except()
  {
    // Set up the travel segment

    Loc* origin = _memHandle.create<Loc>();
    origin->loc() = string("LHR");
    origin->nation() = string("GB");

    Loc* destination = _memHandle.create<Loc>();
    destination->loc() = string("ORD");
    destination->nation() = string("US");

    AirSeg* travelSeg = _memHandle.create<AirSeg>();
    travelSeg->origin() = origin;
    travelSeg->destination() = destination;
    travelSeg->carrier() = "AA";
    travelSeg->setBookingCode("U");

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(travelSeg);

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->itin() = itin;

    // Done setting up the farepath

    _taxResponse = _memHandle.create<TaxResponse>();
    _taxResponse->farePath() = farePath;

    // Start setting up Restriction Transit Table Under TaxCodeReg

    uint16_t travelSegIndex = 0;

    TaxCodeReg taxCodeReg;
    taxCodeReg.taxCode() = "GB3";

    TaxCodeCabin taxCodeCabin;
    taxCodeCabin.carrier() = "AA";
    taxCodeCabin.classOfService() = "U";
    taxCodeCabin.directionalInd() = BETWEEN;
    taxCodeCabin.loc1().locType() = ' ';
    taxCodeCabin.loc1().loc() = ' ';
    taxCodeCabin.loc2().locType() = ' ';
    taxCodeCabin.loc2().loc() = ' ';
    taxCodeCabin.exceptInd() = 'Y';

    bool rc = _taxGB.validateFareClass(*_trx, *_taxResponse, taxCodeReg, travelSegIndex);

    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTaxGB_HighPremium_Except()
  {
    // Set up the travel segment

    Loc* origin = _memHandle.create<Loc>();
    origin->loc() = string("LHR");
    origin->nation() = string("GB");

    Loc* destination = _memHandle.create<Loc>();
    destination->loc() = string("ORD");
    destination->nation() = string("US");

    AirSeg* travelSeg = _memHandle.create<AirSeg>();
    travelSeg->origin() = origin;
    travelSeg->destination() = destination;
    travelSeg->carrier() = "AC";
    travelSeg->setBookingCode("Z");

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(travelSeg);

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->itin() = itin;

    // Done setting up the farepath

    _taxResponse = _memHandle.create<TaxResponse>();
    _taxResponse->farePath() = farePath;

    // Start setting up Restriction Transit Table Under TaxCodeReg

    uint16_t travelSegIndex = 0;

    TaxCodeReg taxCodeReg;
    taxCodeReg.taxCode() = "GB1";

    TaxCodeCabin taxCodeCabin;
    taxCodeCabin.carrier() = "AC";
    taxCodeCabin.classOfService() = "Z";
    taxCodeCabin.directionalInd() = BETWEEN;
    taxCodeCabin.loc1().locType() = ' ';
    taxCodeCabin.loc1().loc() = ' ';
    taxCodeCabin.loc2().locType() = ' ';
    taxCodeCabin.loc2().loc() = ' ';
    taxCodeCabin.exceptInd() = 'Y';

    bool rc = _taxGB.validateFareClass(*_trx, *_taxResponse, taxCodeReg, travelSegIndex);

    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTaxGB_WithinGB_Except()
  {
    // Set up the travel segment

    Loc* loc1 = _memHandle.create<Loc>();
    loc1->loc() = string("MAN");
    loc1->nation() = string("GB");

    Loc* loc2 = _memHandle.create<Loc>();
    loc2->loc() = string("LGW");
    loc2->nation() = string("GB");

    Loc* loc3 = _memHandle.create<Loc>();
    loc3->loc() = string("GLA");
    loc3->nation() = string("GB");

    AirSeg* travelSeg1 = _memHandle.create<AirSeg>();
    travelSeg1->origin() = loc1;
    travelSeg1->destination() = loc2;
    travelSeg1->carrier() = "BA";
    travelSeg1->setBookingCode("N");
    travelSeg1->departureDT() = (DateTime)172800;
    travelSeg1->arrivalDT() = (DateTime)86400;

    AirSeg* travelSeg2 = _memHandle.create<AirSeg>();
    travelSeg2->origin() = loc2;
    travelSeg2->destination() = loc3;
    travelSeg2->carrier() = "BA";
    travelSeg2->setBookingCode("V");
    travelSeg2->departureDT() = (DateTime)345600;
    travelSeg2->arrivalDT() = (DateTime)259200;

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(travelSeg1);
    itin->travelSeg().push_back(travelSeg2);

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->itin() = itin;

    // Done setting up the farepath

    _taxResponse = _memHandle.create<TaxResponse>();
    _taxResponse->farePath() = farePath;

    // Start setting up Restriction Transit Table Under TaxCodeReg

    uint16_t travelSegIndex = 0;

    TaxCodeReg taxCodeReg;
    taxCodeReg.taxCode() = "GB1";

    TaxCodeCabin taxCodeCabin;
    taxCodeCabin.carrier() = "BA";
    taxCodeCabin.classOfService() = "*";
    taxCodeCabin.directionalInd() = WITHIN;
    taxCodeCabin.loc1().locType() = 'N';
    taxCodeCabin.loc1().loc() = "GB";
    taxCodeCabin.loc2().locType() = ' ';
    taxCodeCabin.loc2().loc() = ' ';
    taxCodeCabin.exceptInd() = 'Y';

    bool rc = _taxGB.validateFareClass(*_trx, *_taxResponse, taxCodeReg, travelSegIndex);

    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTaxGB_WithinGB()
  {
    // Set up the travel segment

    Loc* loc1 = _memHandle.create<Loc>();
    loc1->loc() = string("MAN");
    loc1->nation() = string("GB");

    Loc* loc2 = _memHandle.create<Loc>();
    loc2->loc() = string("LGW");
    loc2->nation() = string("GB");

    Loc* loc3 = _memHandle.create<Loc>();
    loc3->loc() = string("GLA");
    loc3->nation() = string("GB");

    AirSeg* travelSeg1 = _memHandle.create<AirSeg>();
    travelSeg1->origin() = loc1;
    travelSeg1->destination() = loc2;
    travelSeg1->carrier() = "BA";
    travelSeg1->setBookingCode("N");
    travelSeg1->departureDT() = (DateTime)172800;
    travelSeg1->arrivalDT() = (DateTime)86400;

    AirSeg* travelSeg2 = _memHandle.create<AirSeg>();
    travelSeg2->origin() = loc2;
    travelSeg2->destination() = loc3;
    travelSeg2->carrier() = "BA";
    travelSeg2->setBookingCode("V");
    travelSeg2->departureDT() = (DateTime)345600;
    travelSeg2->arrivalDT() = (DateTime)259200;

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(travelSeg1);
    itin->travelSeg().push_back(travelSeg2);

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->itin() = itin;

    // Done setting up the farepath

    _taxResponse = _memHandle.create<TaxResponse>();
    _taxResponse->farePath() = farePath;

    // Start setting up Restriction Transit Table Under TaxCodeReg

    uint16_t travelSegIndex = 0;

    TaxCodeReg taxCodeReg;
    taxCodeReg.taxCode() = "GB3";

    TaxCodeCabin taxCodeCabin;
    taxCodeCabin.carrier() = "BA";
    taxCodeCabin.classOfService() = "*";
    taxCodeCabin.directionalInd() = WITHIN;
    taxCodeCabin.loc1().locType() = 'N';
    taxCodeCabin.loc1().loc() = "GB";
    taxCodeCabin.loc2().locType() = ' ';
    taxCodeCabin.loc2().loc() = ' ';
    taxCodeCabin.exceptInd() = 'N';

    bool rc = _taxGB.validateFareClass(*_trx, *_taxResponse, taxCodeReg, travelSegIndex);

    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTaxGB_FromToGB()
  {
    // Set up the travel segment

    Loc* loc1 = _memHandle.create<Loc>();
    loc1->loc() = string("MAN");
    loc1->nation() = string("GB");

    Loc* loc2 = _memHandle.create<Loc>();
    loc2->loc() = string("ORD");
    loc2->nation() = string("US");

    AirSeg* travelSeg1 = _memHandle.create<AirSeg>();
    travelSeg1->origin() = loc1;
    travelSeg1->destination() = loc2;
    travelSeg1->carrier() = "BD";
    travelSeg1->setBookingCode("U");

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(travelSeg1);

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->itin() = itin;

    // Done setting up the farepath

    _taxResponse = _memHandle.create<TaxResponse>();
    _taxResponse->farePath() = farePath;

    // Start setting up Restriction Transit Table Under TaxCodeReg

    uint16_t travelSegIndex = 0;

    TaxCodeReg taxCodeReg;
    taxCodeReg.taxCode() = "GB1";

    TaxCodeCabin taxCodeCabin;
    taxCodeCabin.carrier() = "BD";
    taxCodeCabin.classOfService() = "U";
    taxCodeCabin.directionalInd() = FROM;
    taxCodeCabin.loc1().locType() = 'N';
    taxCodeCabin.loc1().loc() = "GB";
    taxCodeCabin.loc2().locType() = 'N';
    taxCodeCabin.loc2().loc() = "US";
    taxCodeCabin.exceptInd() = 'N';

    bool rc = _taxGB.validateFareClass(*_trx, *_taxResponse, taxCodeReg, travelSegIndex);

    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTaxGB_FromToGB_Except()
  {
    // Set up the travel segment

    Loc* loc1 = _memHandle.create<Loc>();
    loc1->loc() = string("MAN");
    loc1->nation() = string("GB");

    Loc* loc2 = _memHandle.create<Loc>();
    loc2->loc() = string("ORD");
    loc2->nation() = string("US");

    AirSeg* travelSeg1 = _memHandle.create<AirSeg>();
    travelSeg1->origin() = loc1;
    travelSeg1->destination() = loc2;
    travelSeg1->carrier() = "BD";
    travelSeg1->setBookingCode("Y");

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(travelSeg1);

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->itin() = itin;

    // Done setting up the farepath

    _taxResponse = _memHandle.create<TaxResponse>();
    _taxResponse->farePath() = farePath;

    // Start setting up Restriction Transit Table Under TaxCodeReg

    uint16_t travelSegIndex = 0;

    TaxCodeReg taxCodeReg;
    taxCodeReg.taxCode() = "GB1";

    TaxCodeCabin taxCodeCabin;
    taxCodeCabin.carrier() = "BD";
    taxCodeCabin.classOfService() = "*";
    taxCodeCabin.directionalInd() = FROM;
    taxCodeCabin.loc1().locType() = 'N';
    taxCodeCabin.loc1().loc() = "GB";
    taxCodeCabin.loc2().locType() = 'N';
    taxCodeCabin.loc2().loc() = "US";
    taxCodeCabin.exceptInd() = 'Y';

    bool rc = _taxGB.validateFareClass(*_trx, *_taxResponse, taxCodeReg, travelSegIndex);

    CPPUNIT_ASSERT_EQUAL(false, rc);
  }

  void testTaxGB_isForcedStopover()
  {
    AirSeg* travelSeg1 = _memHandle.create<AirSeg>();
    travelSeg1->forcedStopOver() = 'Y';

    bool rc = _taxGB.isStopOver(travelSeg1, travelSeg1);

    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTaxGB_isForcedStopover_Open()
  {
    AirSeg* travelSeg1 = _memHandle.create<AirSeg>();
    travelSeg1->segmentType() = Open;

    bool rc = _taxGB.isStopOver(travelSeg1, travelSeg1);

    CPPUNIT_ASSERT_EQUAL(true, rc);
  }

  void testTaxGB_isStopOverNoTrainNoStop_Pass()
  {
    Loc* origin1 = _memHandle.create<Loc>();
    origin1->loc() = string("LHR");
    origin1->nation() = string("GB");

    Loc* destination1 = _memHandle.create<Loc>();
    destination1->loc() = string("MAN");
    destination1->nation() = string("GB");

    AirSeg* travelSeg1 = _memHandle.create<AirSeg>();
    travelSeg1->origin() = origin1;
    travelSeg1->destination() = destination1;

    travelSeg1->carrier() = "BA";
    travelSeg1->setBookingCode("W");
    travelSeg1->departureDT() = DateTime(2004, 11, 21, 8, 0, 0);
    travelSeg1->arrivalDT() = DateTime(2004, 11, 21, 14, 0, 0);

    bool rc = _taxGB.isStopOver(travelSeg1, travelSeg1, true);

    CPPUNIT_ASSERT_MESSAGE("Empty equipment type - domestic", !rc);

    travelSeg1->equipmentType() = "TRS";
    rc = _taxGB.isStopOver(travelSeg1, travelSeg1, true);

    CPPUNIT_ASSERT_MESSAGE("TRS type - domestic", !rc);
  }

  void testTaxGB_isStopOverTrainIsStop_Pass()
  {

    Loc* origin1 = _memHandle.create<Loc>();
    origin1->loc() = string("LHR");
    origin1->nation() = string("GB");

    Loc* destination1 = _memHandle.create<Loc>();
    destination1->loc() = string("MAN");
    destination1->nation() = string("GB");

    AirSeg* travelSeg1 = _memHandle.create<AirSeg>();
    travelSeg1->origin() = origin1;
    travelSeg1->destination() = destination1;
    travelSeg1->carrier() = "BA";
    travelSeg1->setBookingCode("W");
    travelSeg1->departureDT() = DateTime(2004, 11, 21, 8, 0, 0);
    travelSeg1->arrivalDT() = DateTime(2004, 11, 21, 14, 0, 0);
    travelSeg1->equipmentType() = "TGV";

    bool rc = _taxGB.isDomStopOver(travelSeg1, travelSeg1);

    CPPUNIT_ASSERT_MESSAGE("TGV type", rc);

    travelSeg1->equipmentType() = "TRN";
    rc = _taxGB.isDomStopOver(travelSeg1, travelSeg1);

    CPPUNIT_ASSERT_MESSAGE("TRN type", rc);

    travelSeg1->equipmentType() = "ICE";
    rc = _taxGB.isDomStopOver(travelSeg1, travelSeg1);

    CPPUNIT_ASSERT_MESSAGE("ICE type", rc);

  }

  void testIsDomSeg_Dom()
  {
    Loc* origin1 = _memHandle.create<Loc>();
    origin1->loc() = string("LHR");
    origin1->nation() = string("GB");

    Loc* destination1 = _memHandle.create<Loc>();
    destination1->loc() = string("MAN");
    destination1->nation() = string("GB");

    AirSeg* travelSeg1 = _memHandle.create<AirSeg>();
    travelSeg1->origin() = origin1;
    travelSeg1->destination() = destination1;

    CPPUNIT_ASSERT(_taxGB.isDomSeg(travelSeg1));
  }

  void testIsDomSeg_Int()
  {
    CPPUNIT_ASSERT(!_taxGB.isDomSeg(_taxResponse->farePath()->itin()->travelSeg()[0]));
  }

  void testFindIntSurface_NoSurface()
  {
    Itin itin;
    AirSeg airseg1;
    AirSeg airseg2;
    itin.travelSeg().push_back(&airseg1);
    itin.travelSeg().push_back(&airseg2);
    itin.travelSeg().push_back(&airseg1);

    uint16_t startIndex = 0;
    uint16_t endIndex = 2;

    CPPUNIT_ASSERT_EQUAL(endIndex, _taxGB.findIntSurface(&itin, startIndex, endIndex));
  }

  void testIsNotUKJourney()
  {
    CPPUNIT_ASSERT(!_taxGB.isUKJourney(*_taxResponse->farePath()->itin(), 0, 0));
  }

  void testIsUKJourney()
  {
    Loc* origin1 = _memHandle.create<Loc>();
    origin1->loc() = string("LHR");
    origin1->nation() = string("GB");

    Loc* destination1 = _memHandle.create<Loc>();
    destination1->loc() = string("MAN");
    destination1->nation() = string("GB");

    AirSeg* travelSeg1 = _memHandle.create<AirSeg>();
    travelSeg1->origin() = origin1;
    travelSeg1->destination() = destination1;

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(travelSeg1);

    CPPUNIT_ASSERT(_taxGB.isUKJourney(*itin, 0, 0));
  }

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    // Set up the PricingTrx
    _trx = _memHandle.create<PricingTrx>();

    Loc* agentLocation = _memHandle.create<Loc>();
    agentLocation->loc() = string("LON");
    agentLocation->nation() = string("GB");

    Agent* agent = _memHandle.create<Agent>();
    agent->agentLocation() = agentLocation;

    PricingRequest* request = _memHandle.create<PricingRequest>();
    request->ticketingAgent() = agent;

    _trx->setRequest(request);

    // Finished setting up trx.

    // Set up the travel segment

    Loc* _origin = _memHandle.create<Loc>();
    _origin->loc() = string("LHR");
    _origin->nation() = string("GB");

    Loc* _destination = _memHandle.create<Loc>();
    _destination->loc() = string("CDG");
    _destination->nation() = string("FR");

    AirSeg* _travelSeg = _memHandle.create<AirSeg>();
    _travelSeg->origin() = _origin;
    _travelSeg->destination() = _destination;
    _travelSeg->carrier() = "BA";
    _travelSeg->setBookingCode("F");

    // Set up the fare path

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(_travelSeg);

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->itin() = itin;

    // Done setting up the farepath

    _taxResponse = _memHandle.create<TaxResponse>();

    DCFactory* factory = DCFactory::instance();
    DiagCollector& diagCollector = *(factory->create(*_trx));

    _taxResponse->diagCollector() = &diagCollector;

    _taxResponse->farePath() = farePath;

    TaxCodeReg taxCodeReg;
    taxCodeReg.specialProcessNo() = 38;
    taxCodeReg.expireDate() = DateTime(2004, 12, 31, 12, 0, 0);
    taxCodeReg.loc1Type() = tse::LocType('Z');
    taxCodeReg.loc2Type() = tse::LocType('Z');
    taxCodeReg.loc1() = tse::LocCode("00331");
    taxCodeReg.loc1ExclInd() = tse::Indicator('N' /* TAX_EXCLUDE */);
    taxCodeReg.loc2() = tse::LocCode("00332");
    taxCodeReg.loc2ExclInd() = tse::Indicator('N' /* TAX_EXCLUDE */);
    taxCodeReg.taxCode() = std::string("GB1");
    taxCodeReg.effDate() = DateTime(2004, 7, 13, 12, 0, 0);
    taxCodeReg.discDate() = DateTime(2004, 12, 31, 12, 0, 0);
    ;
    taxCodeReg.firstTvlDate() = DateTime(2004, 7, 14, 12, 0, 0);
    taxCodeReg.lastTvlDate() = DateTime(2004, 12, 31, 23, 59, 59);
    taxCodeReg.nation() = std::string("GB");
    taxCodeReg.taxfullFareInd() = tse::Indicator('N');
    taxCodeReg.taxequivAmtInd() = tse::Indicator('N');
    taxCodeReg.taxexcessbagInd() = tse::Indicator('N');
    taxCodeReg.tvlDateasoriginInd() = tse::Indicator('N');
    taxCodeReg.displayonlyInd() = tse::Indicator('N');
    taxCodeReg.feeInd() = tse::Indicator('N');
    taxCodeReg.interlinableTaxInd() = tse::Indicator('Y');
    taxCodeReg.showseparateInd() = tse::Indicator('N');

    taxCodeReg.posExclInd() = tse::Indicator('N');
    taxCodeReg.posLocType() = tse::LocType(' ');
    taxCodeReg.posLoc() = tse::LocCode("");

    taxCodeReg.poiExclInd() = tse::Indicator('N');
    taxCodeReg.poiLocType() = tse::LocType(' ');
    taxCodeReg.poiLoc() = tse::LocCode("");

    taxCodeReg.sellCurExclInd() = tse::Indicator('N');
    taxCodeReg.sellCur() = tse::CurrencyCode("");

    taxCodeReg.occurrence() = tse::Indicator(' ');
    taxCodeReg.freeTktexempt() = tse::Indicator('N');
    taxCodeReg.idTvlexempt() = tse::Indicator('N');

    taxCodeReg.taxCur() = tse::CurrencyCode("GBP");
    taxCodeReg.taxAmt() = 100000;
    taxCodeReg.taxType() = 'F';

    taxCodeReg.fareclassExclInd() = tse::Indicator('N');
    taxCodeReg.tktdsgExclInd() = tse::Indicator('N');
    taxCodeReg.valcxrExclInd() = tse::Indicator('N');

    taxCodeReg.exempequipExclInd() = tse::Indicator('Y');
    taxCodeReg.psgrExclInd() = tse::Indicator('Y');
    taxCodeReg.fareTypeExclInd() = tse::Indicator('N');

    taxCodeReg.originLocExclInd() = tse::Indicator('N');
    taxCodeReg.originLocType() = tse::LocType(' ');
    taxCodeReg.originLoc() = tse::LocCode("");
    taxCodeReg.loc1Appl() = tse::Indicator('E');
    taxCodeReg.loc2Appl() = tse::Indicator(' ');
    taxCodeReg.tripType() = tse::Indicator('F');
    taxCodeReg.travelType() = tse::Indicator(' ');
    taxCodeReg.itineraryType() = tse::Indicator(' ');
    taxCodeReg.formOfPayment() = tse::Indicator('A');
    taxCodeReg.taxOnTaxExcl() = tse::Indicator('N');

    TaxCodeCabin taxCodeCabin;
    taxCodeCabin.carrier() = "";
    taxCodeCabin.classOfService() = "*";
    taxCodeCabin.loc1().locType() = ' ';
    taxCodeCabin.loc1().loc() = ' ';
    taxCodeCabin.loc2().locType() = ' ';
    taxCodeCabin.loc2().loc() = ' ';
    taxCodeCabin.exceptInd() = ' ';

    Loc* _origin1 = _memHandle.create<Loc>();
    _origin1->loc() = string("LHR");
    _origin1->nation() = string("GB");

    Loc* _destination1 = _memHandle.create<Loc>();
    _destination1->loc() = string("MAN");
    _destination1->nation() = string("GB");

    AirSeg* _travelSeg1 = _memHandle.create<AirSeg>();
    _travelSeg1->origin() = _origin1;
    _travelSeg1->destination() = _destination1;
  }

  void tearDown() { _memHandle.clear(); }

private:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  TaxResponse* _taxResponse;
  TaxGB03 _taxGB;

  Loc* _origin;
  Loc* _destination;
  AirSeg* _travelSeg;

  Loc* _origin1;
  Loc* _destination1;
  AirSeg* _travelSeg1;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxGB03Test);
}
