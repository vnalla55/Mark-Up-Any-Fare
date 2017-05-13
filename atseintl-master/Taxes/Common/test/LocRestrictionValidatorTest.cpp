#include <string>
#include <vector>

#include "test/include/CppUnitHelperMacros.h"

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxCodeReg.h"
#include "Diagnostic/Diag804Collector.h"
#include "Diagnostic/Diagnostic.h"
#include "Taxes/Common/LocRestrictionValidator.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

class LocRestrictionValidatorTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(LocRestrictionValidatorTest);
  CPPUNIT_TEST(testConstructor);
  CPPUNIT_TEST(testValidateLocationCase0);
  CPPUNIT_TEST(testValidateLocationCase1);
  CPPUNIT_TEST(testValidateLocationCase2);
  CPPUNIT_TEST(testValidateLocationCase3);
  CPPUNIT_TEST(testValidateLocationCase4);
  CPPUNIT_TEST(testValidateLocationCase5);
  CPPUNIT_TEST(testValidateLocationCase6);
  CPPUNIT_TEST(testValidateLocationCase7);
  CPPUNIT_TEST(testValidateLocationCase8);
  CPPUNIT_TEST(testValidateLocationCase9);
  CPPUNIT_TEST(testValidateLocationCase10);
  CPPUNIT_TEST(testValidateLocationCase11);
  CPPUNIT_TEST(testValidateLocationCase12);
  CPPUNIT_TEST(testValidateLocationCase13);
  CPPUNIT_TEST(testValidateLocationCase14);
  CPPUNIT_TEST(testValidateLocationCase15);
  CPPUNIT_TEST(testValidateLocationCase16);
  CPPUNIT_TEST(testValidateLocationCase17);
  CPPUNIT_TEST(testValidateLocationCase18);
  CPPUNIT_TEST(testValidateLocationCase19);
  CPPUNIT_TEST(testValidateLocationCase20);
  CPPUNIT_TEST(testValidateLocationCase21);
  CPPUNIT_TEST(testValidateLocationCase22);
  CPPUNIT_TEST(testValidateLocationCase23);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _trx = _memHandle.create<PricingTrx>();
    _val = _memHandle.create<LocRestrictionValidatorTestAdapter>();
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _taxResponse = _memHandle.create<TaxResponse>();
    _startIndex = 0;
    _endIndex = 0;

    _origin = _memHandle.create<Loc>();
    _origin->loc() = std::string("NYC");
    _origin->nation() = std::string("US");

    _destination = _memHandle.create<Loc>();
    _destination->loc() = std::string("LIM");
    _destination->nation() = std::string("PE");

    _ts = _memHandle.create<AirSeg>();
    _ts->origin() = _origin;
    _ts->destination() = _destination;

    _itin = _memHandle.create<Itin>();
    _itin->travelSeg().push_back(_ts);

    _fp = _memHandle.create<FarePath>();
    _fp->itin() = _itin; // Set the itinerary in the fare path.

    _taxResponse->farePath() = _fp;

    _diagroot = _memHandle.insert(new Diagnostic(LegacyTaxDiagnostic24));
    _diagroot->activate();
    _diag = _memHandle.insert(new Diag804Collector(*_diagroot));

    _taxResponse->diagCollector() = _diag;

    _taxCodeReg = _memHandle.create<TaxCodeReg>();
    _taxCodeReg->loc1Type() = tse::LocType(' ');
    _taxCodeReg->loc2Type() = tse::LocType(' ');
    _taxCodeReg->occurrence() = LocRestrictionValidator::TAX_ORIGIN;
    _taxCodeReg->loc1() = tse::LocCode("PE");
    _taxCodeReg->loc2() = tse::LocCode("PE");
    _taxCodeReg->loc1ExclInd() = tse::Indicator('Y');
    _taxCodeReg->loc2ExclInd() = tse::Indicator('Y');
  }

  void tearDown() { _memHandle.clear(); }

  void testConstructor()
  {
    try { tse::LocRestrictionValidator val; }
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
  void testValidateLocationCase0()
  {
    bool result =
        _val->validateLocation(*_trx, *_taxResponse, *_taxCodeReg, _startIndex, _endIndex);
    CPPUNIT_ASSERT_EQUAL(true, result);
  }

  /**
   * Case1  tests TAX_ORIGIN behavior.
   *  geoMatch is false, but, the loc1ExclInd is yes, so the validation is true.
   **/
  void testValidateLocationCase1()
  {
    _taxCodeReg->loc1Type() = tse::LocType('N');
    bool result =
        _val->validateLocation(*_trx, *_taxResponse, *_taxCodeReg, _startIndex, _endIndex);
    CPPUNIT_ASSERT_EQUAL(true, result);
  }

  /**
   * Case2  tests TAX_ORIGIN behavior.
   *  geoMatch is false, but, the loc1ExclInd is no, so the validation is false.
   **/
  void testValidateLocationCase2()
  {
    _taxCodeReg->loc1Type() = tse::LocType('N');
    _taxCodeReg->loc1ExclInd() = tse::Indicator('N');
    bool result =
        _val->validateLocation(*_trx, *_taxResponse, *_taxCodeReg, _startIndex, _endIndex);
    CPPUNIT_ASSERT_EQUAL(true, result);
  }

  /**
   * Case3  tests TAX_ORIGIN behavior.
   *  geoMatch is true (PE -> PE), and tax is excluded, so
   *  validationLocation should return false.
   **/
  void testValidateLocationCase3()
  {
    _taxCodeReg->loc1Type() = tse::LocType('N');
    _origin->loc() = std::string("LIM");
    _origin->nation() = std::string("PE");
    bool result =
        _val->validateLocation(*_trx, *_taxResponse, *_taxCodeReg, _startIndex, _endIndex);
    CPPUNIT_ASSERT_EQUAL(true, result);
  }

  /**
   * Case4  tests TAX_ORIGIN behavior.
   *  geoMatch is true (PE -> PE), and tax is not excluded, so
   *  validationLocation should return true.
   **/
  void testValidateLocationCase4()
  {
    _taxCodeReg->loc1Type() = tse::LocType('N');
    _origin->loc() = std::string("LIM");
    _origin->nation() = std::string("PE");
    _taxCodeReg->loc1ExclInd() = tse::Indicator('N');
    bool result =
        _val->validateLocation(*_trx, *_taxResponse, *_taxCodeReg, _startIndex, _endIndex);
    CPPUNIT_ASSERT_EQUAL(true, result);
  }

  /**
   * Case5  tests TAX_ORIGIN behavior.
   *  geoMatch is true (PE -> PE), and we are doing a taxCodeReg of zone,
   *  the result should depend on the tax exclusion. If tax is excluded,
   * validateLocation should return false; if tax is not excluded, it should
   * return true.
   **/
  void testValidateLocationCase5()
  {
    _origin->loc() = std::string("LIM");
    _origin->nation() = std::string("PE");
    _taxCodeReg->loc1ExclInd() = tse::Indicator('N');
    bool result =
        _val->validateLocation(*_trx, *_taxResponse, *_taxCodeReg, _startIndex, _endIndex);
    CPPUNIT_ASSERT_EQUAL(true, result);
  }

  void testValidateLocationCase6()
  {
    _taxCodeReg->loc1ExclInd() = tse::Indicator('N');
    _taxCodeReg->loc2ExclInd() = tse::Indicator('N');
    bool result = _val->validateOriginP(*_trx, *_taxResponse, *_taxCodeReg, _startIndex, _endIndex);
    CPPUNIT_ASSERT_EQUAL(true, result);
  }

  void testValidateLocationCase7()
  {
    _taxCodeReg->loc1ExclInd() = tse::Indicator('N');
    _taxCodeReg->loc2ExclInd() = tse::Indicator('N');
    bool result =
        _val->validateEnplanementP(*_trx, *_taxResponse, *_taxCodeReg, _startIndex, _endIndex);
    CPPUNIT_ASSERT_EQUAL(true, result);
  }

  void testValidateLocationCase8()
  {
    _taxCodeReg->loc1ExclInd() = tse::Indicator('N');
    _taxCodeReg->loc2ExclInd() = tse::Indicator('N');
    bool result =
        _val->validateDeplanementP(*_trx, *_taxResponse, *_taxCodeReg, _startIndex, _endIndex);
    CPPUNIT_ASSERT_EQUAL(true, result);
  }

  void testValidateLocationCase9()
  {
    _taxCodeReg->loc1ExclInd() = tse::Indicator('N');
    _taxCodeReg->loc2ExclInd() = tse::Indicator('N');
    bool result =
        _val->validateDestinationP(*_trx, *_taxResponse, *_taxCodeReg, _startIndex, _endIndex);
    CPPUNIT_ASSERT_EQUAL(true, result);
  }

  void testValidateLocationCase10()
  {
    _taxCodeReg->loc1ExclInd() = tse::Indicator('N');
    _taxCodeReg->loc2ExclInd() = tse::Indicator('N');
    bool result =
        _val->validateTerminationP(*_trx, *_taxResponse, *_taxCodeReg, _startIndex, _endIndex);
    CPPUNIT_ASSERT_EQUAL(true, result);
  }

  void testValidateLocationCase11()
  {
    bool result = _val->validateOriginP(*_trx, *_taxResponse, *_taxCodeReg, _startIndex, _endIndex);
    CPPUNIT_ASSERT_EQUAL(false, result);
  }

  void testValidateLocationCase12()
  {
    bool result =
        _val->validateEnplanementP(*_trx, *_taxResponse, *_taxCodeReg, _startIndex, _endIndex);
    CPPUNIT_ASSERT_EQUAL(false, result);
  }

  void testValidateLocationCase13()
  {
    bool result =
        _val->validateDeplanementP(*_trx, *_taxResponse, *_taxCodeReg, _startIndex, _endIndex);
    CPPUNIT_ASSERT_EQUAL(false, result);
  }

  void testValidateLocationCase14()
  {
    bool result =
        _val->validateDestinationP(*_trx, *_taxResponse, *_taxCodeReg, _startIndex, _endIndex);
    CPPUNIT_ASSERT_EQUAL(false, result);
  }

  void testValidateLocationCase15()
  {
    bool result =
        _val->validateTerminationP(*_trx, *_taxResponse, *_taxCodeReg, _startIndex, _endIndex);
    CPPUNIT_ASSERT_EQUAL(false, result);
  }

  void testValidateLocationCase16()
  {
    _origin->loc() = std::string("LIM");
    _origin->nation() = std::string("PE");
    bool result =
        _val->validateLocation(*_trx, *_taxResponse, *_taxCodeReg, _startIndex, _endIndex);
    CPPUNIT_ASSERT_EQUAL(true, result);
  }

  void testValidateLocationCase17()
  {
    _taxCodeReg->loc1Type() = tse::LocType('N');
    _taxCodeReg->loc1Appl() = LocRestrictionValidator::TAX_TERMINATION;
    _taxCodeReg->loc2Appl() = LocRestrictionValidator::TAX_TERMINATION;
    bool result =
        _val->validateLocation(*_trx, *_taxResponse, *_taxCodeReg, _startIndex, _endIndex);
    CPPUNIT_ASSERT_EQUAL(false, result);
  }

  void testValidateLocationCase18()
  {
    _startIndex = 1;
    _endIndex = 1;
    _itin->travelSeg().push_back(_ts);
    _itin->travelSeg().push_back(_ts);
    bool result =
        _val->validateTerminationP(*_trx, *_taxResponse, *_taxCodeReg, _startIndex, _endIndex);
    CPPUNIT_ASSERT_EQUAL(false, result);
  }

  void testValidateLocationCase19()
  {
    _startIndex = 1;
    _endIndex = 1;
    _itin->travelSeg().push_back(_ts);
    _itin->travelSeg().push_back(_ts);
    bool result = _val->validateOriginP(*_trx, *_taxResponse, *_taxCodeReg, _startIndex, _endIndex);
    CPPUNIT_ASSERT_EQUAL(false, result);
  }

  void testValidateLocationCase20()
  {
    _taxCodeReg->loc1Type() = tse::LocType('N');
    _taxCodeReg->loc1Appl() = LocRestrictionValidator::TAX_ENPLANEMENT;
    bool result =
        _val->validateLocation(*_trx, *_taxResponse, *_taxCodeReg, _startIndex, _endIndex);
    CPPUNIT_ASSERT_EQUAL(true, result);
  }

  void testValidateLocationCase21()
  {
    _taxCodeReg->loc1Type() = tse::LocType('N');
    _taxCodeReg->loc1Appl() = LocRestrictionValidator::TAX_DEPLANEMENT;
    _taxCodeReg->loc2Appl() = LocRestrictionValidator::TAX_DEPLANEMENT;
    bool result =
        _val->validateLocation(*_trx, *_taxResponse, *_taxCodeReg, _startIndex, _endIndex);
    CPPUNIT_ASSERT_EQUAL(false, result);
  }

  void testValidateLocationCase22()
  {
    _taxCodeReg->loc1Type() = tse::LocType('N');
    _taxCodeReg->loc1Appl() = LocRestrictionValidator::TAX_DESTINATION;
    _taxCodeReg->loc2Appl() = LocRestrictionValidator::TAX_DESTINATION;
    bool result =
        _val->validateLocation(*_trx, *_taxResponse, *_taxCodeReg, _startIndex, _endIndex);
    CPPUNIT_ASSERT_EQUAL(false, result);
  }

  void testValidateLocationCase23()
  {
    _taxCodeReg->loc1Type() = tse::LocType('N');
    _taxCodeReg->loc1Appl() = LocRestrictionValidator::TAX_ORIGIN;
    _taxCodeReg->loc2Appl() = LocRestrictionValidator::TAX_DESTINATION;
    bool result =
        _val->validateLocation(*_trx, *_taxResponse, *_taxCodeReg, _startIndex, _endIndex);
    CPPUNIT_ASSERT_EQUAL(false, result);
  }

private:
  class LocRestrictionValidatorTestAdapter : public LocRestrictionValidator
  {

  public:
    bool validateOriginP(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         TaxCodeReg& taxCodeReg,
                         uint16_t& startIndex,
                         uint16_t& endIndex)
    {
      return validateOrigin(trx, taxResponse, taxCodeReg, startIndex, endIndex);
    }

    bool validateEnplanementP(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              TaxCodeReg& taxCodeReg,
                              uint16_t& startIndex,
                              uint16_t& endIndex)
    {
      return validateEnplanement(trx, taxResponse, taxCodeReg, startIndex, endIndex);
    }

    bool validateDeplanementP(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              TaxCodeReg& taxCodeReg,
                              uint16_t& startIndex,
                              uint16_t& endIndex)
    {
      return validateDeplanement(trx, taxResponse, taxCodeReg, startIndex, endIndex);
    }

    bool validateDestinationP(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              TaxCodeReg& taxCodeReg,
                              uint16_t& startIndex,
                              uint16_t& endIndex)
    {
      return validateDestination(trx, taxResponse, taxCodeReg, startIndex, endIndex);
    }

    bool validateTerminationP(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              TaxCodeReg& taxCodeReg,
                              uint16_t& startIndex,
                              uint16_t& endIndex)
    {
      return validateTermination(trx, taxResponse, taxCodeReg, startIndex, endIndex);
    }
  };

  PricingTrx* _trx;
  LocRestrictionValidatorTestAdapter* _val;
  uint16_t _startIndex;
  uint16_t _endIndex;
  Loc* _origin;
  Loc* _destination;
  TravelSeg* _ts;
  Itin* _itin;
  FarePath* _fp;
  tse::TaxResponse* _taxResponse;
  Diagnostic* _diagroot;
  tse::TaxCodeReg* _taxCodeReg;
  Diag804Collector* _diag;
  TestMemHandle _memHandle;
};

CPPUNIT_TEST_SUITE_REGISTRATION(LocRestrictionValidatorTest);

} // tse
