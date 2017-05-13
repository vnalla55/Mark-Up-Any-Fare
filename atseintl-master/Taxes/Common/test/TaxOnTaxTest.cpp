#include "test/include/CppUnitHelperMacros.h"
#include "Taxes/LegacyTaxes/TaxOnTax.h"

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Diagnostic/DCFactory.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/CalculationDetails.h"
#include "Taxes/Common/TaxSplitDetails.h"

#include "test/include/TestMemHandle.h"
#include "DataModel/AirSeg.h"
#include "test/testdata/TestAirSegFactory.h"
#include "DataModel/FarePath.h"
#include "test/include/TestConfigInitializer.h"
#include "Taxes/Common/TaxOnTaxCommon.h"

namespace tse
{

class PricingTrx;
class DiagCollector;

class TaxOnTaxTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TaxOnTaxTest);
  CPPUNIT_TEST(testCalculateTaxOnTaxNoTaxOnTax);
  CPPUNIT_TEST(testCalculateTaxOnTaxWithFare);
  CPPUNIT_TEST(testCalculateTaxOnTaxWithoutFare);
  CPPUNIT_TEST(testCalculateTaxOnTaxWithoutFareNoMatchingTaxItem);
  CPPUNIT_TEST(testUseTaxOnTax_FlatTax);
  CPPUNIT_TEST(testUseTaxOnTax_ExclBaseFare);
  CPPUNIT_TEST(testUseTaxOnTax_IncludeBaseFare);
  CPPUNIT_TEST(testUseTaxOnTax_IncludeBaseFare_NotEmptyTaxOnTaxVector);
  CPPUNIT_TEST(testTaxOnTaxItems);
  CPPUNIT_TEST(testTaxOnTaxCalculationDetails);
  CPPUNIT_TEST(testSkipTaxOnTaxIfNoFare_SetToFalse);
  CPPUNIT_TEST(testSkipTaxOnTaxIfNoFare_SetToTrue);
  CPPUNIT_TEST(testRequireTaxOnDomTaxMatch_SetToFalse);
  CPPUNIT_TEST(testRequireTaxOnDomTaxMatch_SetToTrue);
  CPPUNIT_TEST(testValidateShoppingRestrictions_pricingRequest);
  CPPUNIT_TEST(testValidateShoppingRestrictions_restrictionsDisabled);
  CPPUNIT_TEST(testValidateShoppingRestrictions_yqyrExcluded);
  CPPUNIT_TEST(testValidateShoppingRestrictions_yqyrIncluded);
  CPPUNIT_TEST(testValidateShoppingRestrictions_notInVector);
  CPPUNIT_TEST(testValidateShoppingRestrictions_inVector);
  CPPUNIT_TEST(testCalculateTaxFromTaxItem_ExternalTaxes);
  CPPUNIT_TEST(testCheckTaxCode_2_letters);
  CPPUNIT_TEST(testCheckTaxCode_2_letters_and_asterisk);
  CPPUNIT_TEST(testCheckTaxCode_3_letters);
  CPPUNIT_TEST_SUITE_END();

public:

  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    taxCodeReg = _memHandle.create<TaxCodeReg>();
    taxResponse = _memHandle.create<TaxResponse>();
    taxItem = _memHandle.create<MockTaxItem>();
    trx = _memHandle.create<PricingTrx>();
    factory = DCFactory::instance();
    diagCollector = factory->create(*trx);
    taxResponse->diagCollector() = diagCollector;
    details = _memHandle.create<CalculationDetails>();
    _taxSplitDetails = _memHandle.create<TaxSplitDetails>();
    taxOnTax = _memHandle.create<TaxOnTax>(*details, *_taxSplitDetails);
  }

  void tearDown() { _memHandle.clear(); }

  void testCalculateTaxOnTaxNoTaxOnTax()
  {
    MoneyAmount taxAmount = 20.0;
    MoneyAmount taxableFare = 100.0;
    taxCodeReg->taxOnTaxExcl() = YES;

    taxOnTax->calculateTaxOnTax(
        *trx, *taxResponse, taxAmount, taxableFare, *taxCodeReg, taxOnTaxItems, 0);

    CPPUNIT_ASSERT_EQUAL(taxAmount, 0.0);
  }

  void testCalculateTaxOnTaxWithFare()
  {
    MoneyAmount taxAmount = 20.0;
    MoneyAmount taxableFare = 100.0;

    TaxCode taxCode("DY");
    taxCodeReg->taxOnTaxCode().push_back(taxCode);
    taxCodeReg->taxCode() = "DY";
    taxCodeReg->taxAmt() = 0.15;
    taxItem->setTaxCodeReg(taxCodeReg);
    taxItem->setTaxAmount(15.00);

    taxResponse->taxItemVector().push_back(taxItem);

    taxOnTax->calculateTaxOnTax(
        *trx, *taxResponse, taxAmount, taxableFare, *taxCodeReg, taxOnTaxItems, 0);

    CPPUNIT_ASSERT_EQUAL(taxAmount, 17.25);
  }

  void testCalculateTaxOnTaxWithoutFare()
  {
    MoneyAmount taxAmount = 20.0;
    MoneyAmount taxableFare = 100.0;
    // CurrencyCode paymentCurrency = "USD";

    TaxCode taxCode("DY");
    taxCodeReg->taxOnTaxCode().push_back(taxCode);
    taxCodeReg->taxCode() = "DY";
    taxCodeReg->taxAmt() = 0.15;
    taxItem->setTaxCodeReg(taxCodeReg);
    taxItem->setTaxAmount(15.00);

    taxResponse->taxItemVector().push_back(taxItem);
    taxCodeReg->taxOnTaxExcl() = 'Y';

    taxOnTax->calculateTaxOnTax(
        *trx, *taxResponse, taxAmount, taxableFare, *taxCodeReg, taxOnTaxItems, 0);

    CPPUNIT_ASSERT_EQUAL(taxAmount, 2.25);
  }

  void testCalculateTaxOnTaxWithoutFareNoMatchingTaxItem()
  {
    MoneyAmount taxAmount = 20.0;
    MoneyAmount taxableFare = 100.0;
    // CurrencyCode paymentCurrency = "USD";

    TaxCode taxCode("YQF");
    taxCodeReg->taxOnTaxCode().push_back(taxCode);
    taxCodeReg->taxCode() = "DY";
    taxCodeReg->taxAmt() = 0.15;
    taxItem->setTaxCodeReg(taxCodeReg);
    taxItem->setTaxAmount(15.00);

    taxResponse->taxItemVector().push_back(taxItem);
    taxCodeReg->taxOnTaxExcl() = 'Y';

    taxOnTax->calculateTaxOnTax(
        *trx, *taxResponse, taxAmount, taxableFare, *taxCodeReg, taxOnTaxItems, 0);

    CPPUNIT_ASSERT_EQUAL(taxAmount, 0.00);
  }
  void testUseTaxOnTax_FlatTax()
  {
    taxCodeReg->taxType() = 'F';

    CPPUNIT_ASSERT(!TaxOnTax::useTaxOnTax(*taxCodeReg));
  }
  void testUseTaxOnTax_ExclBaseFare()
  {
    taxCodeReg->taxType() = 'P';
    taxCodeReg->taxOnTaxExcl() = YES;

    CPPUNIT_ASSERT(TaxOnTax::useTaxOnTax(*taxCodeReg));
  }

  void testUseTaxOnTax_IncludeBaseFare()
  {
    taxCodeReg->taxType() = 'P';
    taxCodeReg->taxOnTaxExcl() = NO;

    CPPUNIT_ASSERT(!TaxOnTax::useTaxOnTax(*taxCodeReg));
  }

  void testUseTaxOnTax_IncludeBaseFare_NotEmptyTaxOnTaxVector()
  {
    taxCodeReg->taxType() = 'P';
    taxCodeReg->taxOnTaxExcl() = NO;
    taxCodeReg->taxOnTaxCode().push_back("US1");

    CPPUNIT_ASSERT(TaxOnTax::useTaxOnTax(*taxCodeReg));
  }

  void testTaxOnTaxItems()
  {
    MoneyAmount taxAmount = 20.0;
    MoneyAmount taxableFare = 100.0;

    TaxCode taxCode1("DY");
    taxCodeReg->taxOnTaxCode().push_back(taxCode1);
    taxCodeReg->taxCode() = "DY";
    taxCodeReg->taxAmt() = 0.15;
    taxItem->setTaxCodeReg(taxCodeReg);
    taxItem->setTaxAmount(15.00);

    taxResponse->taxItemVector().push_back(taxItem);

    taxCodeReg->taxOnTaxExcl() = 'Y';

    taxOnTax->calculateTaxOnTax(
        *trx, *taxResponse, taxAmount, taxableFare, *taxCodeReg, taxOnTaxItems, 0);

    CPPUNIT_ASSERT(taxOnTaxItems.size() == 1);
    CPPUNIT_ASSERT(taxOnTaxItems.front()->taxAmount() == 15.00);
  }

  void testTaxOnTaxCalculationDetails()
  {
    MoneyAmount taxAmount = 20.0;
    MoneyAmount taxableFare = 100.0;

    TaxCode taxCode1("DY");
    taxCodeReg->taxOnTaxCode().push_back(taxCode1);
    taxCodeReg->taxCode() = "DY";
    taxCodeReg->taxAmt() = 0.15;
    taxItem->setTaxCodeReg(taxCodeReg);
    taxItem->setTaxAmount(15.00);

    taxResponse->taxItemVector().push_back(taxItem);

    taxCodeReg->taxOnTaxExcl() = 'Y';

    taxOnTax->calculateTaxOnTax(
        *trx, *taxResponse, taxAmount, taxableFare, *taxCodeReg, taxOnTaxItems, 0);

    CPPUNIT_ASSERT(details->taxableTaxes.size() == 1);
    CPPUNIT_ASSERT(details->taxableTaxes.begin()->first == "DY");
    CPPUNIT_ASSERT(details->taxableTaxes.begin()->second == 15.00);
    CPPUNIT_ASSERT(details->taxableTaxSumAmount == 15.00);
  }

  void testSkipTaxOnTaxIfNoFare_SetToFalse()
  {
    MoneyAmount taxAmount = 0.0;
    MoneyAmount taxableFare = 0.0;

    bool skipTaxOnTaxIfNoFare = false;
    taxOnTax->setSkipTaxOnTaxIfNoFare(skipTaxOnTaxIfNoFare);

    TaxCode taxCode("YQF");
    taxCodeReg->taxOnTaxCode().push_back(taxCode);
    taxCodeReg->taxCode() = "YQF";
    taxCodeReg->taxAmt() = 0.10;
    taxCodeReg->taxOnTaxExcl() = 'Y';

    taxItem->setTaxCodeReg(taxCodeReg);
    taxItem->setTaxAmount(20.00);
    taxResponse->taxItemVector().push_back(taxItem);

    taxOnTax->calculateTaxOnTax(
        *trx, *taxResponse, taxAmount, taxableFare, *taxCodeReg, taxOnTaxItems, 0);

    CPPUNIT_ASSERT_EQUAL(taxAmount, 2.0);
  }

  void testSkipTaxOnTaxIfNoFare_SetToTrue()
  {
    MoneyAmount taxAmount = 0.0;
    MoneyAmount taxableFare = 0.0;

    bool skipTaxOnTaxIfNoFare = true;
    taxOnTax->setSkipTaxOnTaxIfNoFare(skipTaxOnTaxIfNoFare);

    TaxCode taxCode("YQF");
    taxCodeReg->taxOnTaxCode().push_back(taxCode);
    taxCodeReg->taxCode() = "YQF";
    taxCodeReg->taxAmt() = 0.10;
    taxCodeReg->taxOnTaxExcl() = 'Y';

    taxItem->setTaxCodeReg(taxCodeReg);
    taxItem->setTaxAmount(20.00);
    taxResponse->taxItemVector().push_back(taxItem);

    taxOnTax->calculateTaxOnTax(
        *trx, *taxResponse, taxAmount, taxableFare, *taxCodeReg, taxOnTaxItems, 0);

    CPPUNIT_ASSERT_EQUAL(taxAmount, 0.0);
  }

  void testRequireTaxOnDomTaxMatch_SetToFalse()
  {
    MoneyAmount taxAmount = 0.0;
    MoneyAmount taxableFare = 0.0;

    bool requireTaxOnDomTaxMatch = false;
    taxOnTax->setRequireTaxOnDomTaxMatch(requireTaxOnDomTaxMatch);

    TaxCode taxCode("YQF");
    taxCodeReg->taxOnTaxCode().push_back(taxCode);
    taxCodeReg->taxCode() = "YQF";
    taxCodeReg->taxAmt() = 0.10;
    taxCodeReg->taxOnTaxExcl() = 'Y';
    taxCodeReg->nation() = std::string("US");

    taxItem->setTaxCodeReg(taxCodeReg);
    taxItem->setTaxAmount(20.00);
    taxResponse->taxItemVector().push_back(taxItem);

    taxItem->setTravelSegStartIndex(0);
    taxItem->setTravelSegEndIndex(0);

    Loc origin;
    Loc destination;

    origin.loc() = std::string("KUL");
    origin.nation() = std::string("MY");

    destination.loc() = std::string("BKI");
    destination.nation() = std::string("MY");

    AirSeg airSeg;
    TravelSeg* travelSeg = &airSeg;
    travelSeg->origin() = &origin;
    travelSeg->destination() = &destination;

    Itin itin;
    FarePath farePath;
    farePath.itin() = &itin;
    farePath.itin()->travelSeg().push_back(travelSeg);
    itin.farePath().push_back(&farePath);

    taxResponse->farePath() = &farePath;

    taxOnTax->calculateTaxOnTax(
        *trx, *taxResponse, taxAmount, taxableFare, *taxCodeReg, taxOnTaxItems, 0);

    CPPUNIT_ASSERT_EQUAL(taxAmount, 2.0);
  }

  void testRequireTaxOnDomTaxMatch_SetToTrue()
  {

    MoneyAmount taxAmount = 0.0;
    MoneyAmount taxableFare = 0.0;

    bool requireTaxOnDomTaxMatch = true;
    taxOnTax->setRequireTaxOnDomTaxMatch(requireTaxOnDomTaxMatch);

    TaxCode taxCode("YQF");
    taxCodeReg->taxOnTaxCode().push_back(taxCode);
    taxCodeReg->taxCode() = "YQF";
    taxCodeReg->taxAmt() = 0.10;
    taxCodeReg->taxOnTaxExcl() = 'Y';
    taxCodeReg->nation() = std::string("US");

    taxItem->setTaxCodeReg(taxCodeReg);
    taxItem->setTaxAmount(20.00);
    taxResponse->taxItemVector().push_back(taxItem);

    taxItem->setTravelSegStartIndex(0);
    taxItem->setTravelSegEndIndex(0);

    Loc origin;
    Loc destination;

    origin.loc() = std::string("KUL");
    origin.nation() = std::string("MY");

    destination.loc() = std::string("BKI");
    destination.nation() = std::string("MY");

    AirSeg airSeg;
    TravelSeg* travelSeg = &airSeg;
    travelSeg->origin() = &origin;
    travelSeg->destination() = &destination;

    Itin itin;
    FarePath farePath;
    farePath.itin() = &itin;
    farePath.itin()->travelSeg().push_back(travelSeg);
    itin.farePath().push_back(&farePath);

    taxResponse->farePath() = &farePath;

    taxOnTax->calculateTaxOnTax(
        *trx, *taxResponse, taxAmount, taxableFare, *taxCodeReg, taxOnTaxItems, 0);

    CPPUNIT_ASSERT_EQUAL(taxAmount, 0.0);
  }

  void addTaxItemToCalculationDetails(const std::string& taxCode)
  {
    TaxItem* taxItem = _memHandle.create<MockTaxItem>();
    taxItem->taxCode() = taxCode;
    details->taxableTaxItems.push_back(taxItem);
  }

  std::unique_ptr<TaxOnTaxCommon>
  getTaxOnTaxCommon()
  {
    std::pair<uint16_t, uint16_t> indexRange;
    std::unique_ptr<TaxOnTaxCommon> result(
        new TaxOnTaxCommon(false, false, false, indexRange, *details, *_taxSplitDetails));
    return result;
  }

  void testValidateShoppingRestrictions_pricingRequest()
  {
    CPPUNIT_ASSERT(getTaxOnTaxCommon()->validateShoppingRestrictions("YQF", false));
  }

  void testValidateShoppingRestrictions_restrictionsDisabled()
  {
    _taxSplitDetails->setTotShopRestEnabled(false);
    CPPUNIT_ASSERT(getTaxOnTaxCommon()->validateShoppingRestrictions("YQF", true));
  }

  void testValidateShoppingRestrictions_yqyrExcluded()
  {
    _taxSplitDetails->setTotShopRestEnabled(true);
    _taxSplitDetails->setTotShopRestIncludeBaseFare(false);
    CPPUNIT_ASSERT(!getTaxOnTaxCommon()->validateShoppingRestrictions("YQF", true));
  }

  void testValidateShoppingRestrictions_yqyrIncluded()
  {
    _taxSplitDetails->setTotShopRestEnabled(true);
    _taxSplitDetails->setTotShopRestIncludeBaseFare(true);
    CPPUNIT_ASSERT(getTaxOnTaxCommon()->validateShoppingRestrictions("YQF", true));
  }

  void testValidateShoppingRestrictions_notInVector()
  {
    _taxSplitDetails->setTotShopRestEnabled(true);
    addTaxItemToCalculationDetails("XG");
    addTaxItemToCalculationDetails("XG3");
    CPPUNIT_ASSERT(!getTaxOnTaxCommon()->validateShoppingRestrictions("SQ3", true));
  }

  void testValidateShoppingRestrictions_inVector()
  {
    _taxSplitDetails->setTotShopRestEnabled(true);
    addTaxItemToCalculationDetails("XG");
    addTaxItemToCalculationDetails("XG3");
    CPPUNIT_ASSERT(getTaxOnTaxCommon()->validateShoppingRestrictions("XG3", true));
  }

  void testCalculateTaxFromTaxItem_ExternalTaxes()
  {

    const NationCode nation("US");
    Itin itin;
    FarePath farePath;
    TaxItem exItemYQI;
    TaxItem exItemYQR;
    exItemYQI.taxCode() = "YQI";
    exItemYQI.taxAmount() = 5.0;
    exItemYQI.taxCode() = "YQR";
    exItemYQI.taxAmount() = 15.0;
    farePath.getMutableExternalTaxes().push_back(&exItemYQI);
    farePath.getMutableExternalTaxes().push_back(&exItemYQR);
    farePath.itin() = &itin;
    itin.farePath().push_back(&farePath);

    MockTaxItem* taxItemAR = _memHandle.create<MockTaxItem>();
    taxItemAR->taxCode() = "AR";
    taxItemAR->taxAmount() = 1.0;
    MockTaxItem* taxItemYQI = _memHandle.create<MockTaxItem>();
    taxItemYQI->taxCode() = "YQI";
    taxItemYQI->taxAmount() = 2.0;
    MockTaxItem* taxItemYQR = _memHandle.create<MockTaxItem>();
    taxItemYQR->taxCode() = "YQR";
    taxItemYQR->taxAmount() = 3.0;

    taxResponse->taxItemVector().push_back(taxItemAR);
    taxResponse->taxItemVector().push_back(taxItemYQI);
    taxResponse->taxItemVector().push_back(taxItemYQR);

    std::vector<std::string> taxOnTaxCode;
    taxOnTaxCode.push_back("YQR");

    std::vector<TaxItem*> taxOnTaxItems;

    MoneyAmount taxAmount = taxOnTax->calculateTaxFromTaxItem(*taxResponse, taxOnTaxCode, nation, taxOnTaxItems, false, true /* isShoppingTaxRequest */);
    CPPUNIT_ASSERT_EQUAL(taxAmount, 3.0);

    taxResponse->farePath() = &farePath;
    taxAmount = taxOnTax->calculateTaxFromTaxItem(*taxResponse, taxOnTaxCode, nation, taxOnTaxItems, false, true /* isShoppingTaxRequest */);
    CPPUNIT_ASSERT_EQUAL(taxAmount, 18.0);

    taxAmount = taxOnTax->calculateTaxFromTaxItem(*taxResponse, taxOnTaxCode, nation, taxOnTaxItems, false, false /* isShoppingTaxRequest */);
    CPPUNIT_ASSERT_EQUAL(taxAmount, 3.0);
  }

  void
  testCheckTaxCode_2_letters()
  {
    MockTaxItem* taxItem = _memHandle.create<MockTaxItem>();
    taxItem->taxCode() = "YQ";
    CPPUNIT_ASSERT(getTaxOnTaxCommon()->checkTaxCode("YQ", taxItem));
  }

  void
  testCheckTaxCode_2_letters_and_asterisk()
  {
    MockTaxItem* taxItem = _memHandle.create<MockTaxItem>();
    taxItem->taxCode() = "YQ2";
    CPPUNIT_ASSERT(getTaxOnTaxCommon()->checkTaxCode("YQ*", taxItem));

    taxItem->taxCode() = "YQ5";
    CPPUNIT_ASSERT(getTaxOnTaxCommon()->checkTaxCode("YQ*", taxItem));

    taxItem->taxCode() = "YQ6";
    CPPUNIT_ASSERT(getTaxOnTaxCommon()->checkTaxCode("YQ*", taxItem));

    taxItem->taxCode() = "YR6";
    CPPUNIT_ASSERT(!getTaxOnTaxCommon()->checkTaxCode("YQ*", taxItem));
  }

  void
  testCheckTaxCode_3_letters()
  {
    MockTaxItem* taxItem = _memHandle.create<MockTaxItem>();
    taxItem->taxCode() = "YQI";
    CPPUNIT_ASSERT(getTaxOnTaxCommon()->checkTaxCode("YQI", taxItem));

    taxItem->taxCode() = "YQR";
    CPPUNIT_ASSERT(getTaxOnTaxCommon()->checkTaxCode("YQR", taxItem));

    taxItem->taxCode() = "YQR";
    CPPUNIT_ASSERT(!getTaxOnTaxCommon()->checkTaxCode("YQI", taxItem));
  }

private:
  class MockTaxItem : public TaxItem
  {
  public:
    MockTaxItem() : TaxItem() {};
    virtual ~MockTaxItem() {};

    void setTaxCodeReg(TaxCodeReg* taxCodeReg)
    {
      //_taxCodeReg = taxCodeReg;
      this->taxOnTaxCode().push_back(taxCodeReg->taxCode());
      this->taxCode() = taxCodeReg->taxCode();
      this->taxAmt() = taxCodeReg->taxAmt();
    };

    void setFailCode(char failCode)
    {
      _failCode = failCode;
    };
    void setTaxAmount(MoneyAmount taxAmount)
    {
      _taxAmount = taxAmount;
    };
    void setTaxableFare(MoneyAmount taxableFare)
    {
      _taxableFare = taxableFare;
    };
    void setPaymentCurrency(CurrencyCode paymentCurrency)
    {
      _paymentCurrency = paymentCurrency;
    };
    void setTravelSegStartIndex(uint16_t startIndex)
    {
      _travelSegStartIndex = startIndex;
    };
    void setTravelSegEndIndex(uint16_t endIndex)
    {
      _travelSegEndIndex = endIndex;
    };
    void setTaxMilesLocal(uint32_t miles)
    {
      _taxMilesLocal = miles;
    };
    void setTaxMilesThru(uint32_t miles)
    {
      _taxMilesThru = miles;
    };
    void setTaxLocalBoard(LocCode taxLocalBoard)
    {
      _taxLocalBoard = taxLocalBoard;
    };
    void setTaxLocalOff(LocCode taxLocalOff)
    {
      _taxLocalOff = taxLocalOff;
    };
    void setTaxThruBoard(LocCode taxThruBoard)
    {
      _taxThruBoard = taxThruBoard;
    };
    void setTaxThruOff(LocCode taxThruOff)
    {
      _taxThruOff = taxThruOff;
    };
    void setTaxDescription(TaxDescription taxDescription)
    {
      _taxDescription = taxDescription;
    };
    void setTaxOnTaxInfo(TaxOnTaxInfo info)
    {
      _taxOnTaxInfo = info;
    };
    void setTaxRecProcessed(bool processed)
    {
      _taxRecProcessed = processed;
    };
    void setInterline(bool interline)
    {
      _interline = interline;
    };
    void setGstTax(bool gstTax)
    {
      _isGstTax = gstTax;
    };
  };

  TaxOnTax* taxOnTax;
  TaxCodeReg* taxCodeReg;
  TaxResponse* taxResponse;
  MockTaxItem* taxItem;
  PricingTrx* trx;
  DCFactory* factory;
  DiagCollector* diagCollector;
  CalculationDetails* details;
  TaxSplitDetails* _taxSplitDetails;
  TestMemHandle _memHandle;
  std::vector<TaxItem*> taxOnTaxItems;
};
CPPUNIT_TEST_SUITE_REGISTRATION(TaxOnTaxTest);
}
