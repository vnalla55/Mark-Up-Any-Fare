#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestMemHandle.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/FarePath.h"
#include "Diagnostic/Diag801Collector.h"
#include "Taxes/LegacyTaxes/TaxItem.h"

namespace tse
{
class Diag801CollectorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(Diag801CollectorTest);
  CPPUNIT_TEST(testStreamingOperatorTaxItemOut);
  CPPUNIT_TEST_SUITE_END();

private:
  PricingTrx* _trx;
  PricingRequest* _request;
  Diag801Collector* _collector;
  Diagnostic* _diagroot;
  TaxResponse* _taxResponse;
  TestMemHandle _memHandle;

public:
  void setUp()
  {
    _diagroot = _memHandle(new Diagnostic(AllPassTaxDiagnostic281));
    _diagroot->activate();
    _collector = _memHandle(new Diag801Collector(*_diagroot));
    _collector->enable(AllPassTaxDiagnostic281);
    _trx = _memHandle.create<PricingTrx>();
    _collector->trx() = _trx;
    _trx->setTrxType(PricingTrx::PRICING_TRX);
    _request = _memHandle.create<PricingRequest>();
    _trx->setRequest(_request);
    _request->owPricingRTTaxProcess() = false;
    _trx->setValidatingCxrGsaApplicable(false);

    // TaxCodeReg *taxCodeReg = _memHandle.create<TaxCodeReg>();
    // taxCodeReg->taxCode() = "DY";
    // taxCodeReg->taxType() = 'F';
    // taxCodeReg->taxAmt() = 15;
    // taxCodeReg->specialProcessNo() = 0;
    // taxCodeReg->taxcdRoundUnit() = 0;
    // taxCodeReg->taxcdRoundRule() = NONE;
    // taxCodeReg->taxcdRoundUnitNodec() = 0;

    Loc* origin = _memHandle.create<Loc>();
    origin->loc() = std::string("NYC");
    origin->nation() = std::string("US");

    Loc* destination = _memHandle.create<Loc>();
    destination->loc() = std::string("LIM");
    destination->nation() = std::string("PE");

    TravelSeg* travelSeg = _memHandle.create<AirSeg>();
    travelSeg->origin() = origin;
    travelSeg->destination() = destination;

    Itin* itin = _memHandle.create<Itin>();
    itin->travelSeg().push_back(travelSeg);

    FarePath* farePath = _memHandle.create<FarePath>();
    farePath->itin() = itin;

    _taxResponse = _memHandle.create<TaxResponse>();
    _taxResponse->farePath() = farePath;
    _taxResponse->paxTypeCode() = "ADT";

    // Use a MockTaxItem so we can completely set the state of the tax item
    MockTaxItem* ti = _memHandle.create<MockTaxItem>();
    ti->taxCode() = "DY";
    ti->taxType() = 'F';
    ti->taxAmt() = 15;
    ti->specialProcessNo() = 0;
    ti->taxcdRoundUnit() = 0;
    ti->taxcdRoundRule() = NONE;
    ti->taxcdRoundUnitNodec() = 0;
    ti->setTaxAmount(15.00);
    ti->setTaxDescription("INTERNATIONAL ARRIVAL TAX");
    ti->setPaymentCurrency("USD");
    ti->setTaxLocalBoard("NYC");
    ti->setTaxLocalOff("LIM");
    ti->setTaxThruBoard("NYC");
    ti->setTaxThruOff("LIM");

    _taxResponse->taxItemVector().push_back(ti);
    // uint32_t name = taxOut.intializeTaxItemOut();
  }

  void tearDown() { _memHandle.clear(); }

  void testStreamingOperatorTaxItemOut()
  {
    std::string expected = "\n"
                           "***************************************************************\n"
                           " ---- TAX OUT VECTOR  1 ITEMS --- PSGR ADT----        \n"
                           "***************************************************************\n"
                           "\n"
                           "                                           THRU   THRU   RUR    \n"
                           "  CODE FC TYP  TXAMT      TXTTL   TXFARE   BOARD  OFF    CTY  A \n"
                           "                      \n"
                           "1 DY   0  F    15.00USD   15.00     0.00    NYC   LIM     \n"
                           "\n"
                           "    INTERNATIONAL ARRIVAL TAX\n"
                           "        NO ROUNDING SPECIFIED\n"
                           "        SEQUENCE NUMBER: 0 FOR CARRIER CODE: \n";
    (*_collector) << *_taxResponse;

    CPPUNIT_ASSERT_EQUAL(expected, _collector->str());
  }

  class MockTaxItem : public TaxItem
  {
  public:
    MockTaxItem() : TaxItem() {}

    virtual ~MockTaxItem() {}

    void setFailCode(char failCode) { _failCode = failCode; }

    void setTaxAmount(MoneyAmount taxAmount) { _taxAmount = taxAmount; }

    void setTaxableFare(MoneyAmount taxableFare) { _taxableFare = taxableFare; }

    void setPaymentCurrency(CurrencyCode paymentCurrency) { _paymentCurrency = paymentCurrency; }

    void setTravelSegStartIndex(uint16_t startIndex) { _travelSegStartIndex = startIndex; }

    void setTravelSegEndIndex(uint16_t endIndex) { _travelSegEndIndex = endIndex; }

    void setTaxMilesLocal(uint32_t miles) { _taxMilesLocal = miles; }

    void setTaxMilesThru(uint32_t miles) { _taxMilesThru = miles; }

    void setTaxLocalBoard(LocCode taxLocalBoard) { _taxLocalBoard = taxLocalBoard; }

    void setTaxLocalOff(LocCode taxLocalOff) { _taxLocalOff = taxLocalOff; }

    void setTaxThruBoard(LocCode taxThruBoard) { _taxThruBoard = taxThruBoard; }

    void setTaxThruOff(LocCode taxThruOff) { _taxThruOff = taxThruOff; }

    void setTaxDescription(TaxDescription taxDescription) { _taxDescription = taxDescription; }

    void setTaxOnTaxInfo(TaxOnTaxInfo info) { _taxOnTaxInfo = info; }

    void setTaxRecProcessed(bool processed) { _taxRecProcessed = processed; }

    void setInterline(bool interline) { _interline = interline; }

    void setGstTax(bool gstTax) { _isGstTax = gstTax; }
  };
};
CPPUNIT_TEST_SUITE_REGISTRATION(Diag801CollectorTest);
}
