#include <string>
#include <time.h>
#include <iostream>
#include <vector>

#include "Common/Config/ConfigMan.h"
#include "Taxes/LegacyTaxes/TripTypesValidator.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/Loc.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Diagnostic/Diagnostic.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Diagnostic/Diag804Collector.h"
#include "Taxes/LegacyTaxes/MirrorImage.h"
#include "Taxes/LegacyTaxes/TransitValidator.h"
#include "Taxes/Common/LocRestrictionValidator.h"
#include "Taxes/LegacyTaxes/TransitValidator.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

using namespace std;

namespace tse
{

namespace
{
class MirrorImageMock : public MirrorImage
{
  uint16_t val;

public:
  virtual bool isMirrorImage(const PricingTrx& trx,
                             const TaxResponse& taxResponse,
                             const TaxCodeReg& taxCodeReg,
                             uint16_t startIndex)
  {
    return val & ((uint16_t)1 << startIndex);
  }

  void setReturnValue(uint16_t value) { val = value; }
};

class TransitValidatorMock : public TransitValidator
{
  uint16_t val;

public:
  virtual bool validateTransitTime(const PricingTrx& /*trx*/,
                                   const TaxResponse& /*taxResponse*/,
                                   const TaxCodeReg& taxCodeReg,
                                   uint16_t startIndex,
                                   bool /*SHOULD_CHECK_OPEN = SHOULD_NOT_CHECK_OPEN*/)
  {
    if ((taxCodeReg.loc1Appl() != LocRestrictionValidator::TAX_ENPLANEMENT) &&
        ((taxCodeReg.loc2Appl() == LocRestrictionValidator::TAX_DEPLANEMENT) ||
         (taxCodeReg.loc2Appl() == LocRestrictionValidator::TAX_DESTINATION)))
    {
      return val & ((uint16_t)1 << startIndex);
    }
    else
    {
      return val & ((uint16_t)1 << (startIndex - 1));
    }
  }

  void setReturnValue(uint16_t value) { val = value; }
};
}

class TripTypesValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TripTypesValidatorTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testTripTypesCase0);
  CPPUNIT_TEST(testTripTypesFromToCase1);
  CPPUNIT_TEST(testTripTypesvalidateBetweenCase2);
  CPPUNIT_TEST(testTripTypesvalidateWithinSpecCase3);
  CPPUNIT_TEST(testTripTypesvalidateWithinWhollyCase4);
  CPPUNIT_TEST(testFindFarthestPointIndexForcedStopOver);
  CPPUNIT_TEST(testFindFarthestPointIndexDomesticFlt);
  CPPUNIT_TEST(testFindTaxStopOverIndexWithForcedStopOverAfterStopOver);
  CPPUNIT_TEST(testFindTaxStopOverIndexWithForcedStopOverOnStopOver);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() { _memHandle.create<TestConfigInitializer>(); }
  void tearDown() { _memHandle.clear(); }

  void testConstructor()
  {
    try { TripTypesValidator val; }
    catch (...)
    {
      // If any error occured at all, then fail.
      CPPUNIT_ASSERT(false);
    }
  }

  /**
   * Case0 is the default case where if the loc[1|2]Types are spaces,
   * then we return true.
   **/
  void testTripTypesCase0()
  {
    TripTypesValidator val;

    PricingTrx trx;
    TaxResponse taxResponse;
    TaxCodeReg taxCodeReg;
    uint16_t startIndex = 0;
    uint16_t endIndex = 0;
    trx.setRequest(_memHandle.create<PricingRequest>());

    // These values are probably defaults, but let's make sure
    taxCodeReg.loc1Type() = LocType(' ');
    taxCodeReg.loc2Type() = LocType(' ');
    taxCodeReg.tripType() = ' ';

    bool result = val.validateTrip(trx, taxResponse, taxCodeReg, startIndex, endIndex);
    CPPUNIT_ASSERT(result == true);
  }

  /**
   * Case1  tests TAX_ORIGIN behavior.
   *  geoMatch is false, but, the loc1ExclInd is yes, so the validation is true.
   **/
  void testTripTypesFromToCase1()
  {
    TripTypesValidator val;

    PricingTrx trx;
    trx.setRequest(_memHandle.create<PricingRequest>());
    uint16_t startIndex = 0;
    uint16_t endIndex = 0;

    Loc origin;
    origin.loc() = string("NYC");
    origin.nation() = string("US");

    Loc destination;
    destination.loc() = string("LIM");
    destination.nation() = string("PE");

    AirSeg ts;
    ts.origin() = &origin;
    ts.destination() = &destination;

    Itin itin;
    itin.travelSeg().push_back(&ts);

    FarePath fp;
    fp.itin() = &itin; // Set the itinerary in the fare path.

    TaxResponse taxResponse;

    Diagnostic* diagroot = _memHandle.insert(new Diagnostic(LegacyTaxDiagnostic24));
    //   diagroot->activate();
    Diag804Collector diag(*diagroot);

    taxResponse.diagCollector() = &diag;
    taxResponse.farePath() = &fp;

    TaxCodeReg taxCodeReg;
    taxCodeReg.loc1Type() = LocType('N');
    taxCodeReg.loc2Type() = LocType('N');
    taxCodeReg.loc1() = LocCode("PE");

    taxCodeReg.loc1ExclInd() = Indicator('Y' /* TAX_EXCLUDE */);

    taxCodeReg.loc2() = LocCode("PE");
    taxCodeReg.loc2ExclInd() = Indicator('N' /* TAX_EXCLUDE */);

    taxCodeReg.tripType() = TripTypesValidator::TAX_FROM_TO;

    bool result = val.validateTrip(trx, taxResponse, taxCodeReg, startIndex, endIndex);
    CPPUNIT_ASSERT(result == true);

    taxCodeReg.loc2ExclInd() = Indicator('Y' /* TAX_EXCLUDE */);

    result = val.validateTrip(trx, taxResponse, taxCodeReg, startIndex, endIndex);

    CPPUNIT_ASSERT(result == false);
  }

  void testTripTypesvalidateBetweenCase2()
  {
    TripTypesValidator val;

    PricingTrx trx;
    trx.setRequest(_memHandle.create<PricingRequest>());
    uint16_t startIndex = 0;
    uint16_t endIndex = 0;

    Loc origin;
    origin.loc() = string("NYC");
    origin.nation() = string("US");

    Loc destination;
    destination.loc() = string("LIM");
    destination.nation() = string("PE");

    AirSeg ts;
    ts.origin() = &origin;
    ts.destination() = &destination;

    Itin itin;
    itin.travelSeg().push_back(&ts);

    FarePath fp;
    fp.itin() = &itin; // Set the itinerary in the fare path.

    TaxResponse taxResponse;
    Diagnostic* diagroot = _memHandle.insert(new Diagnostic(LegacyTaxDiagnostic24));
    //   diagroot->activate();
    Diag804Collector diag(*diagroot);

    taxResponse.diagCollector() = &diag;
    taxResponse.farePath() = &fp;

    TaxCodeReg taxCodeReg;
    taxCodeReg.loc1Type() = LocType('N');
    taxCodeReg.loc2Type() = LocType('N');
    taxCodeReg.loc1() = LocCode("PE");

    taxCodeReg.loc1ExclInd() = Indicator('Y' /* TAX_EXCLUDE */);

    taxCodeReg.loc2() = LocCode("PE");
    taxCodeReg.loc2ExclInd() = Indicator('N' /* TAX_EXCLUDE */);

    taxCodeReg.tripType() = TripTypesValidator::TAX_BETWEEN;

    bool result = val.validateTrip(trx, taxResponse, taxCodeReg, startIndex, endIndex);
    CPPUNIT_ASSERT(result == true);
  }

  void testTripTypesvalidateWithinSpecCase3()
  {
    TripTypesValidator val;

    PricingTrx trx;
    trx.setRequest(_memHandle.create<PricingRequest>());
    uint16_t startIndex = 0;
    uint16_t endIndex = 0;

    Loc origin;
    origin.loc() = string("NYC");
    origin.nation() = string("US");

    Loc destination;
    destination.loc() = string("LIM");
    destination.nation() = string("PE");

    AirSeg ts;
    ts.origin() = &origin;
    ts.destination() = &destination;

    Itin itin;
    itin.travelSeg().push_back(&ts);

    FarePath fp;
    fp.itin() = &itin; // Set the itinerary in the fare path.

    TaxResponse taxResponse;
    Diagnostic* diagroot = _memHandle.insert(new Diagnostic(LegacyTaxDiagnostic24));
    //   diagroot->activate();
    Diag804Collector diag(*diagroot);

    taxResponse.diagCollector() = &diag;
    taxResponse.farePath() = &fp;

    TaxCodeReg taxCodeReg;
    taxCodeReg.loc1Type() = LocType('N');
    taxCodeReg.loc2Type() = LocType('N');
    taxCodeReg.loc1() = LocCode("PE");

    taxCodeReg.loc1ExclInd() = Indicator('Y' /* TAX_EXCLUDE */);

    taxCodeReg.loc2() = LocCode("PE");
    taxCodeReg.loc2ExclInd() = Indicator('N' /* TAX_EXCLUDE */);

    taxCodeReg.tripType() = TripTypesValidator::TAX_WITHIN_SPEC;

    bool result = val.validateTrip(trx, taxResponse, taxCodeReg, startIndex, endIndex);
    CPPUNIT_ASSERT(result == true);
  }

  void testTripTypesvalidateWithinWhollyCase4()
  {
    TripTypesValidator val;

    PricingTrx trx;
    trx.setRequest(_memHandle.create<PricingRequest>());
    uint16_t startIndex = 0;
    uint16_t endIndex = 0;

    Loc origin;
    origin.loc() = string("NYC");
    origin.nation() = string("US");

    Loc destination;
    destination.loc() = string("LIM");
    destination.nation() = string("PE");

    AirSeg ts;
    ts.origin() = &origin;
    ts.destination() = &destination;

    Itin itin;
    itin.travelSeg().push_back(&ts);

    FarePath fp;
    fp.itin() = &itin; // Set the itinerary in the fare path.

    TaxResponse taxResponse;
    Diagnostic* diagroot = _memHandle.insert(new Diagnostic(LegacyTaxDiagnostic24));
    //   diagroot->activate();
    Diag804Collector diag(*diagroot);

    taxResponse.diagCollector() = &diag;
    taxResponse.farePath() = &fp;

    TaxCodeReg taxCodeReg;
    taxCodeReg.loc1Type() = LocType('N');
    taxCodeReg.loc2Type() = LocType('N');
    taxCodeReg.loc1() = LocCode("PE");

    taxCodeReg.loc1ExclInd() = Indicator('Y' /* TAX_EXCLUDE */);

    taxCodeReg.loc2() = LocCode("PE");
    taxCodeReg.loc2ExclInd() = Indicator('N' /* TAX_EXCLUDE */);

    taxCodeReg.tripType() = TripTypesValidator::TAX_WITHIN_WHOLLY;

    bool result = val.validateTrip(trx, taxResponse, taxCodeReg, startIndex, endIndex);
    CPPUNIT_ASSERT(result == false);
  }

  void testFindFarthestPointIndexForcedStopOver()
  {
    PricingTrx trx;
    TaxResponse taxResponse;
    TaxCodeReg taxCodeReg;

    Itin itin;
    FarePath fp;
    fp.itin() = &itin;
    taxResponse.farePath() = &fp;

    AirSeg seg1;
    AirSeg seg2;
    taxResponse.farePath()->itin()->travelSeg().push_back(&seg1);
    taxResponse.farePath()->itin()->travelSeg().push_back(&seg2);
    taxResponse.farePath()->itin()->travelSeg().front()->forcedStopOver() = 'Y';

    TripTypesValidator validator;
    int farthestPointIndex =
        validator.findFarthestPointIndex(trx, taxResponse, taxCodeReg, 0, 0);

    CPPUNIT_ASSERT_EQUAL(farthestPointIndex, 0);
  }

  void testFindFarthestPointIndexDomesticFlt()
  {
    PricingTrx trx;
    TaxResponse taxResponse;
    TaxCodeReg taxCodeReg;

    Itin itin;
    FarePath fp;
    fp.itin() = &itin;
    taxResponse.farePath() = &fp;

    AirSeg seg1;
    AirSeg seg2;

    Loc loc1;
    Loc loc2;

    NationCode Poland = "PL";

    loc1.nation() = Poland;
    loc2.nation() = Poland;

    seg1.destination() = &loc1;
    seg2.destination() = &loc2;

    taxResponse.farePath()->itin()->travelSeg().push_back(&seg1);
    taxResponse.farePath()->itin()->travelSeg().push_back(&seg2);

    TripTypesValidator validator;
    int farthestPointIndex =
        validator.findFarthestPointIndex(trx, taxResponse, taxCodeReg, 0, 0);

    CPPUNIT_ASSERT_EQUAL(farthestPointIndex, 0);
  }

  void testFindTaxStopOverIndexWithForcedStopOverAfterStopOver()
  {
    PricingTrx trx;
    TaxResponse taxResponse;
    TaxCodeReg taxCodeReg;
    MirrorImageMock mirrorImage;
    TransitValidatorMock transitValidator;

    Itin itin;
    FarePath fp;
    fp.itin() = &itin;
    taxResponse.farePath() = &fp;

    taxCodeReg.nextstopoverrestr() = 'Y';
    taxCodeReg.specialProcessNo() = 0;
    taxCodeReg.loc2Appl() = LocRestrictionValidator::TAX_DEPLANEMENT;

    std::vector<AirSeg> segments;

    itin.travelSeg().resize(3);
    segments.resize(3);

    for (int iSeg = 0; iSeg < 3; ++iSeg)
    {
      segments[iSeg].forcedConx() = '\0';
      segments[iSeg].forcedStopOver() = '\0';
      itin.travelSeg()[iSeg] = &(segments[iSeg]);
    }

    segments[1].forcedStopOver() = 'Y';

    transitValidator.setReturnValue(1);
    mirrorImage.setReturnValue(0);

    TripTypesValidator validator;
    int result = validator.findTaxStopOverIndex(
        trx, taxResponse, taxCodeReg, 0, mirrorImage, transitValidator);
    CPPUNIT_ASSERT_EQUAL(0, result);
  }

  void testFindTaxStopOverIndexWithForcedStopOverOnStopOver()
  {
    PricingTrx trx;
    TaxResponse taxResponse;
    TaxCodeReg taxCodeReg;
    MirrorImageMock mirrorImage;
    TransitValidatorMock transitValidator;

    Itin itin;
    FarePath fp;
    fp.itin() = &itin;
    taxResponse.farePath() = &fp;

    taxCodeReg.nextstopoverrestr() = 'Y';
    taxCodeReg.specialProcessNo() = 0;
    taxCodeReg.loc2Appl() = LocRestrictionValidator::TAX_DEPLANEMENT;

    std::vector<AirSeg> segments;

    itin.travelSeg().resize(3);
    segments.resize(3);

    for (int iSeg = 0; iSeg < 3; ++iSeg)
    {
      segments[iSeg].forcedConx() = '\0';
      segments[iSeg].forcedStopOver() = '\0';
      itin.travelSeg()[iSeg] = &(segments[iSeg]);
    }

    segments[1].forcedStopOver() = 'Y';

    transitValidator.setReturnValue(2);
    mirrorImage.setReturnValue(0);

    TripTypesValidator validator;
    int result = validator.findTaxStopOverIndex(
        trx, taxResponse, taxCodeReg, 0, mirrorImage, transitValidator);
    CPPUNIT_ASSERT_EQUAL(1, result);
  }

private:
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TripTypesValidatorTest);

} // tse
