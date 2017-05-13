#include "test/include/CppUnitHelperMacros.h"

#include "Common/TseConsts.h"
#include "Taxes/LegacyTaxes/TaxJP1_00.h"
#include "DataModel/AirSeg.h"
#include "DBAccess/Loc.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/LegacyTaxes/TaxJP1_00.h"
#include "DataModel/FareUsage.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "DBAccess/TaxSpecConfigReg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/RepricingTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/ArunkSeg.h"

using namespace std;
namespace tse
{

class TaxJP1_00Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxJP1_00Test);
  CPPUNIT_TEST(testBothLocsAreJapanPass);
  CPPUNIT_TEST(testFirstLocIsJapanSecondLocIsNotJapanFail);
  CPPUNIT_TEST(testFirstLocIsNotJapanSecondLocIsJapanFail);
  CPPUNIT_TEST(testBothLocsAreNotJapanFail);
  CPPUNIT_TEST(testGetAmountFromRepricedFare);
  CPPUNIT_TEST(testIsTerminationPt_NoStopOver);
  CPPUNIT_TEST(testIsTerminationPt_NoAirSeg);
  CPPUNIT_TEST(testHaveOnlyJpSegs_NotOnlyJpSegs);
  CPPUNIT_TEST(testHaveOnlyJpSegs_OnlyJpSegs);
  CPPUNIT_TEST(testHaveOriginAndTerminationInJp_NotTerminationInJp);
  CPPUNIT_TEST(testHaveOriginAndTerminationInJp_TerminationInJp);
  CPPUNIT_TEST(testApplyForcedQualifierOnStopOver_ForcedStopOver);
  CPPUNIT_TEST(testApplyForcedQualifierOnStopOver_ForcedConx);
  CPPUNIT_TEST(testMappedPaxTypeCode);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _tax = new TaxJP1_00;
    _travelSeg = new AirSeg;
    _travelSegNoJP = new AirSeg;
    _orig = new Loc;
    _dest = new Loc;
    _origNotJP = new Loc;
    _destNotJP = new Loc;
    _fareUsage = new FareUsage;

    _farePath.paxType() = &_paxType;
    _taxResponse.farePath() = &_farePath;

    _origNotJP->nation() = "PL";
    _destNotJP->nation() = "US";

    _travelSeg->origin() = _orig;
    _travelSeg->destination() = _dest;
    _travelSegNoJP->origin() = _origNotJP;
    _travelSegNoJP->destination() = _destNotJP;
  }

  void tearDown()
  {
    delete _tax;
    delete _travelSeg;
    delete _travelSegNoJP;
    delete _orig;
    delete _dest;
    delete _fareUsage;
  }

  void testBothLocsAreJapanPass()
  {
    _orig->nation() = JAPAN;
    _dest->nation() = JAPAN;

    CPPUNIT_ASSERT_EQUAL(true, _tax->isTravelSegWhollyJapan(_travelSeg));
  }

  void testFirstLocIsJapanSecondLocIsNotJapanFail()
  {
    _orig->nation() = JAPAN;
    _dest->nation() = KOREA;

    _travelSeg->origin() = _orig;
    _travelSeg->destination() = _dest;

    CPPUNIT_ASSERT_EQUAL(false, _tax->isTravelSegWhollyJapan(_travelSeg));
  }

  void testFirstLocIsNotJapanSecondLocIsJapanFail()
  {
    _orig->nation() = KOREA;
    _dest->nation() = JAPAN;

    _travelSeg->origin() = _orig;
    _travelSeg->destination() = _dest;

    CPPUNIT_ASSERT_EQUAL(false, _tax->isTravelSegWhollyJapan(_travelSeg));
  }

  void testBothLocsAreNotJapanFail()
  {
    _orig->nation() = KOREA;
    _dest->nation() = KOREA;

    _travelSeg->origin() = _orig;
    _travelSeg->destination() = _dest;

    CPPUNIT_ASSERT_EQUAL(false, _tax->isTravelSegWhollyJapan(_travelSeg));
  }

  void testGetAmountFromRepricedFare()
  {
    RepricingTrx retrx;
    TravelSeg* travelSeg = 0;
    MoneyAmount taxableFare;
    CabinType orgTravelSegCabin;
    DateTime travelDate;
    bool bIgnoreCabinCheck = false;

    retrx.fareMarket().push_back(&_fareMarket);

    CPPUNIT_ASSERT(!_tax->getAmountFromRepricedFare(_taxResponse.paxTypeCode(),
                                                    &retrx,
                                                    travelSeg,
                                                    taxableFare,
                                                    orgTravelSegCabin,
                                                    travelDate,
                                                    bIgnoreCabinCheck));
  }

  void testIsTerminationPt_NoStopOver()
  {
    _travelSeg->forcedStopOver() = 'F';
    _travelSeg->forcedConx() = 'T';

    Itin itin;
    itin.travelSeg().push_back(_travelSeg);
    itin.travelSeg().push_back(_travelSeg);
    itin.travelSeg().push_back(_travelSeg);

    CPPUNIT_ASSERT(!_tripTypesValidatorJP1.isTerminationPt(&itin, 1));
  }

  void testIsTerminationPt_NoAirSeg()
  {
    Itin itin;

    ArunkSeg seg;
    itin.travelSeg().push_back(_travelSeg);
    itin.travelSeg().push_back(&seg);
    itin.travelSeg().push_back(_travelSeg);

    CPPUNIT_ASSERT(_tripTypesValidatorJP1.isTerminationPt(&itin, 1));
  }

  void testHaveOnlyJpSegs_NotOnlyJpSegs()
  {
    Itin itin;
    _orig->nation() = JAPAN;
    _dest->nation() = JAPAN;

    itin.travelSeg().push_back(_travelSeg);
    itin.travelSeg().push_back(_travelSegNoJP);
    itin.travelSeg().push_back(_travelSeg);

    CPPUNIT_ASSERT(!_tripTypesValidatorJP1.haveOnlyJpSegs(&itin, 0, 2));
  }

  void testHaveOnlyJpSegs_OnlyJpSegs()
  {
    Itin itin;
    _orig->nation() = JAPAN;
    _dest->nation() = JAPAN;

    itin.travelSeg().push_back(_travelSeg);
    itin.travelSeg().push_back(_travelSeg);
    itin.travelSeg().push_back(_travelSeg);

    CPPUNIT_ASSERT(_tripTypesValidatorJP1.haveOnlyJpSegs(&itin, 0, 2));
  }

  void testHaveOriginAndTerminationInJp_NotTerminationInJp()
  {
    Itin itin;
    _orig->nation() = JAPAN;
    _dest->nation() = JAPAN;

    itin.travelSeg().push_back(_travelSeg);
    itin.travelSeg().push_back(_travelSegNoJP);

    CPPUNIT_ASSERT(!_tripTypesValidatorJP1.haveOriginAndTerminationInJp(&itin));
  }

  void testHaveOriginAndTerminationInJp_TerminationInJp()
  {
    Itin itin;
    _orig->nation() = JAPAN;
    _dest->nation() = JAPAN;

    itin.travelSeg().push_back(_travelSeg);
    itin.travelSeg().push_back(_travelSegNoJP);
    itin.travelSeg().push_back(_travelSeg);

    CPPUNIT_ASSERT(_tripTypesValidatorJP1.haveOriginAndTerminationInJp(&itin));
  }

  void testApplyForcedQualifierOnStopOver_ForcedStopOver()
  {
    bool isStopOver = false;
    _travelSeg->forcedStopOver() = 'T';
    _travelSeg->forcedConx() = 'F';
    _tripTypesValidatorJP1.applyForcedQualifierOnStopOver(_travelSeg, isStopOver);

    CPPUNIT_ASSERT(isStopOver);
  }

  void testApplyForcedQualifierOnStopOver_ForcedConx()
  {
    bool isStopOver = false;
    _travelSeg->forcedStopOver() = 'F';
    _travelSeg->forcedConx() = 'T';
    _tripTypesValidatorJP1.applyForcedQualifierOnStopOver(_travelSeg, isStopOver);

    CPPUNIT_ASSERT(!isStopOver);
  }

  void testMappedPaxTypeCode()
  {
    CPPUNIT_ASSERT_EQUAL(ADULT, _tax->getMappedPaxTypeCode(JCB));
  }

private:
  class JP1DataHandleMock : public DataHandleMock
  {
    std::vector<TaxSpecConfigReg*> _ret;

  public:
    std::vector<TaxSpecConfigReg*>& getTaxSpecConfigForFallback(const TaxSpecConfigName& name)
    {
      return _ret;
    }
  };

  TaxJP1_00* _tax;
  AirSeg* _travelSeg;
  AirSeg* _travelSegNoJP;
  Loc* _orig;
  Loc* _dest;
  Loc* _origNotJP;
  Loc* _destNotJP;
  JP1DataHandleMock _dhm;
  PricingTrx _trx;
  TaxResponse _taxResponse;
  FareUsage* _fareUsage;
  FareMarket _fareMarket;
  PaxType _paxType;
  FarePath _farePath;
  TripTypesValidatorJP1 _tripTypesValidatorJP1;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxJP1_00Test);
};
