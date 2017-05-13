#include "test/include/CppUnitHelperMacros.h"
#include "Taxes/LegacyTaxes/TaxSP76.cpp"
#include "DataModel/AirSeg.h"

using namespace std;

namespace tse
{
class TaxSP76Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxSP76Test);
  CPPUNIT_TEST(testOneSegmentsWithValidCarrierPass);
  CPPUNIT_TEST(testOneSegmentsWithInvalidCarrierFail);
  CPPUNIT_TEST(testAllSegmentsWithValidCarrierPass);
  CPPUNIT_TEST(testSomeSegmentsWithInvalidCarrierFail);
  CPPUNIT_TEST_SUITE_END();

  TaxSP76* _tax;
  PricingTrx* _trx;
  TaxResponse* _taxResponse;
  TaxCodeReg* _taxCodeReg;
  TaxExemptionCarrier* _exemptionCxr;

public:
  void setUp()
  {
    _tax = new TaxSP76;
    _trx = new PricingTrx;

    Itin* itin = new Itin();

    FarePath* farePath = new FarePath();
    farePath->itin() = itin;

    _taxResponse = new TaxResponse();
    _taxResponse->farePath() = farePath;

    _taxCodeReg = new TaxCodeReg;
    _exemptionCxr = new TaxExemptionCarrier();

    _exemptionCxr->carrier() = "JJ";

    _taxCodeReg->exemptionCxr().push_back(*_exemptionCxr);
  }

  void tearDown()
  {
    delete _tax;
    delete _trx;
    delete _taxResponse;
    delete _taxCodeReg;
    delete _exemptionCxr;
  }

  void testOneSegmentsWithValidCarrierPass()
  {
    AirSeg travelSeg;
    travelSeg.setMarketingCarrierCode("JJ");

    _taxResponse->farePath()->itin()->travelSeg().push_back(&travelSeg);

    CPPUNIT_ASSERT_EQUAL(true,
                         _tax->validateCarrierExemption(*_trx, *_taxResponse, *_taxCodeReg, 0));
  }

  void testOneSegmentsWithInvalidCarrierFail()
  {
    AirSeg travelSeg;
    travelSeg.setMarketingCarrierCode("GG");

    _taxResponse->farePath()->itin()->travelSeg().push_back(&travelSeg);

    CPPUNIT_ASSERT_EQUAL(false,
                         _tax->validateCarrierExemption(*_trx, *_taxResponse, *_taxCodeReg, 0));
  }

  void testAllSegmentsWithValidCarrierPass()
  {
    AirSeg travelSeg1, travelSeg2;
    travelSeg1.setMarketingCarrierCode("JJ");
    travelSeg2.setMarketingCarrierCode("JJ");

    _taxResponse->farePath()->itin()->travelSeg().push_back(&travelSeg1);
    _taxResponse->farePath()->itin()->travelSeg().push_back(&travelSeg2);

    CPPUNIT_ASSERT_EQUAL(true,
                         _tax->validateCarrierExemption(*_trx, *_taxResponse, *_taxCodeReg, 0));
  }

  void testSomeSegmentsWithInvalidCarrierFail()
  {
    AirSeg travelSeg1, travelSeg2;
    travelSeg1.setMarketingCarrierCode("JJ");
    travelSeg2.setMarketingCarrierCode("GG");

    _taxResponse->farePath()->itin()->travelSeg().push_back(&travelSeg1);
    _taxResponse->farePath()->itin()->travelSeg().push_back(&travelSeg2);

    CPPUNIT_ASSERT_EQUAL(false,
                         _tax->validateCarrierExemption(*_trx, *_taxResponse, *_taxCodeReg, 0));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxSP76Test);
};
