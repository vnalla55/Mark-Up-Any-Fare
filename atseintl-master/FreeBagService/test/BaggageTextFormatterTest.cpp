#include <boost/assign/std/vector.hpp>
#include "test/include/CppUnitHelperMacros.h"

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/ServicesDescription.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/TaxText.h"
#include "DataModel/Agent.h"
#include "DataModel/BaggageCharge.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/Billing.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "FreeBagService/BaggageTextFormatter.h"
#include "FreeBagService/test/AirSegBuilder.h"
#include "FreeBagService/test/BaggageTravelBuilder.h"
#include "FreeBagService/test/S5Builder.h"
#include "FreeBagService/test/S7Builder.h"
#include "ServiceFees/OCFees.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

using boost::assign::operator+=;

class BaggageTextFormatterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BaggageTextFormatterTest);

  CPPUNIT_TEST(testFormatPQAllowanceText_NoS7);
  CPPUNIT_TEST(testFormatPQAllowanceText_WeightKilos);
  CPPUNIT_TEST(testFormatPQAllowanceText_WeightPounds);
  CPPUNIT_TEST(testFormatPQAllowanceText_Pieces);
  CPPUNIT_TEST(testFormatPQAllowanceText_NoAllowance);
  CPPUNIT_TEST(testFormatPQAllowanceText_PcsAndWeight);
  CPPUNIT_TEST(testFormatPQAllowanceText_PcsAndWeight_UsDot);

  CPPUNIT_TEST(testFormatAllowanceValueText_NoAllowance);
  CPPUNIT_TEST(testFormatAllowanceValueText_PcsOnly);
  CPPUNIT_TEST(testFormatAllowanceValueText_WeightOnly);
  CPPUNIT_TEST(testFormatAllowanceValueText_PcsAndWeight);

  CPPUNIT_TEST(testPopulatePQWeight_Kilos);
  CPPUNIT_TEST(testPopulatePQWeight_Pounds);
  CPPUNIT_TEST(testPopulatePQWeight_Empty);

  CPPUNIT_TEST(testPopulateWeight_Kilos);
  CPPUNIT_TEST(testPopulateWeight_Pounds);
  CPPUNIT_TEST(testPopulateWeight_Empty);

  CPPUNIT_TEST(testPopulatePQPieces);
  CPPUNIT_TEST(testPopulatePQPieces_Empty);
  CPPUNIT_TEST(testPopulatePieces);

  CPPUNIT_TEST(testGetS5Record_NonExist);
  CPPUNIT_TEST(testGetS5Record_Exist);

  CPPUNIT_TEST(testGetServiceSubCodes_Pcs_Matched);
  CPPUNIT_TEST(testGetServiceSubCodes_Pcs_Not_Matched);
  CPPUNIT_TEST(testGetServiceSubCodes_Pcs_Invalid_Format);

  CPPUNIT_TEST(testShouldAddApplicabilityIndicator_Pass);
  CPPUNIT_TEST(testShouldAddApplicabilityIndicator_Fail_Null);
  CPPUNIT_TEST(testShouldAddApplicabilityIndicator_Fail_Per_Baggage_Travel);

  CPPUNIT_TEST(testFormatChargeText_Asterisks_Present_UsDot);
  CPPUNIT_TEST(testFormatChargeText_Asterisks_Present_NonUsDot);
  CPPUNIT_TEST(testFormatChargeText_Asterisks_Not_Present);
  CPPUNIT_TEST(testFormatChargeText_Asterisks_Not_Present_S7_Null);
  CPPUNIT_TEST(testFormatChargeText_NoCharge);
  CPPUNIT_TEST(testFormatChargeText);
  CPPUNIT_TEST(testFormatChargeText_NoAvailNoCharges_X);

  CPPUNIT_TEST(testFormatAllowanceText_NoS7);
  CPPUNIT_TEST(testFormatAllowanceText_S7_NoAllowanceData);
  CPPUNIT_TEST(testFormatAllowanceText);

  CPPUNIT_TEST(testGetChargeDescription_NoS7);
  CPPUNIT_TEST(testGetChargeDescription);

  CPPUNIT_TEST(testGetTravelText);

  CPPUNIT_TEST(testGetCarrierText_CxrTravelSeg_usDot);
  CPPUNIT_TEST(testGetCarrierText_CxrTravelSeg_nonUsDot_open);
  CPPUNIT_TEST(testGetCarrierText_CxrTravelSeg_nonUsDot_notOpen);
  CPPUNIT_TEST(testGetCarrierText_CxrTravelSeg_nonUsDot_notOpen_defer);

  CPPUNIT_TEST(testGetCarrierText_1stTravelSeg_usDot);
  CPPUNIT_TEST(testGetCarrierText_1stTravelSeg_nonUsDot_open);
  CPPUNIT_TEST(testGetCarrierText_1stTravelSeg_nonUsDot_notOpen);

  CPPUNIT_TEST(testGetDescriptionText_NoS5);
  CPPUNIT_TEST(testGetDescriptionText_NoDesc1);
  CPPUNIT_TEST(testGetDescriptionText_Desc1);
  CPPUNIT_TEST(testGetDescriptionText_Desc1And2);

  CPPUNIT_TEST(testGetChargeAmountAndCurrencyText_NoCharge);
  CPPUNIT_TEST(testGetChargeAmountAndCurrencyText_NotPermitted);
  CPPUNIT_TEST(testGetChargeAmountAndCurrencyText_CurrencyAndAmount);
  CPPUNIT_TEST(testGetChargeAmountAndCurrencyText_CurrencyAndAmount_noS7);
  CPPUNIT_TEST(testGetChargeAmountAndCurrencyText_Unknown);

  CPPUNIT_TEST(testGetAllowanceDescription_NoS7);
  CPPUNIT_TEST(testGetAllowanceDescription_NoTaxTbl);
  CPPUNIT_TEST(testGetAllowanceDescription_NoFreePcs);
  CPPUNIT_TEST(testGetAllowanceDescription_BlankFreePcs);
  CPPUNIT_TEST(testGetAllowanceDescription_NoTaxText);
  CPPUNIT_TEST(testGetAllowanceDescription_NoSubCodes);
  CPPUNIT_TEST(testGetAllowanceDescription);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _mdh = _memHandle.create<MyDataHandle>();
    _mdh->initialize();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->getRequest()->majorSchemaVersion() = 1;
    _trx->getRequest()->minorSchemaVersion() = 1;
    _trx->getRequest()->revisionSchemaVersion() = 1;
    _trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    _trx->ticketingDate() = DateTime(9999, 1, 1);
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->billing() = _memHandle.create<Billing>();
    _itin = _memHandle.create<Itin>();
    _itin->setBaggageTripType(BaggageTripType::OTHER);
    _farePath = _memHandle.create<FarePath>();
    _farePath->itin() = _itin;
    _processor = _memHandle.create<BaggageTextFormatter>(*_trx);
  }

  void tearDown() { _memHandle.clear(); }

protected:
  BaggageCharge* createBaggageCharge(OptionalServicesInfo* s7 = 0)
  {
    BaggageCharge* charge = _memHandle.create<BaggageCharge>();
    charge->optFee() = s7;
    charge->farePath() = _farePath;
    return charge;
  }

  OCFees* createOCFees(OptionalServicesInfo* s7 = 0) { return createBaggageCharge(s7); }

public:
  // TESTS
  void testFormatPQAllowanceText_NoS7()
  {
    std::string allowance = _processor->formatPQAllowanceText(0, false);

    CPPUNIT_ASSERT(allowance.empty());
  }

  void testFormatPQAllowanceText_WeightKilos()
  {
    std::string allowance = _processor->formatPQAllowanceText(
        S7Builder(&_memHandle).withBaggageWeight(30, 'K').build(), false);

    CPPUNIT_ASSERT_EQUAL(std::string("30K"), allowance);
  }

  void testFormatPQAllowanceText_WeightPounds()
  {
    std::string allowance;
    std::string expected = "30";
    expected += BaggageTextFormatter::POUNDS_UNIT_CODE;

    allowance = _processor->formatPQAllowanceText(
        S7Builder(&_memHandle).withBaggageWeight(30, BaggageTextFormatter::POUNDS_UNIT).build(),
        false);

    CPPUNIT_ASSERT_EQUAL(expected, allowance);
  }

  void testFormatPQAllowanceText_Pieces()
  {
    std::string allowance;
    std::string expected = "03";
    expected += BaggageTextFormatter::PIECES_UNIT_CODE;

    allowance =
        _processor->formatPQAllowanceText(S7Builder(&_memHandle).withBaggagePcs(3).build(), false);

    CPPUNIT_ASSERT_EQUAL(expected, allowance);
  }

  void testFormatPQAllowanceText_NoAllowance()
  {
    std::string allowance;

    allowance =
        _processor->formatPQAllowanceText(S7Builder(&_memHandle).withBaggagePcs(0).build(), false);

    CPPUNIT_ASSERT_EQUAL(BaggageTextFormatter::NO_ALLOWANCE_DATA, allowance);
  }

  void testFormatPQAllowanceText_PcsAndWeight()
  {
    std::string allowance;
    std::string expected = "20K";

    allowance = _processor->formatPQAllowanceText(
        S7Builder(&_memHandle).withBaggagePcs(3).withBaggageWeight(20, 'K').build(), false);

    CPPUNIT_ASSERT_EQUAL(expected, allowance);
  }

  void testFormatPQAllowanceText_PcsAndWeight_UsDot()
  {
    std::string allowance;
    std::string expected = "03";
    expected += BaggageTextFormatter::PIECES_UNIT_CODE;

    allowance = _processor->formatPQAllowanceText(
        S7Builder(&_memHandle).withBaggagePcs(3).withBaggageWeight(20, 'K').build(), true);

    CPPUNIT_ASSERT_EQUAL(expected, allowance);
  }

  void testFormatAllowanceValueText_NoAllowance()
  {
    std::string allowance;

    allowance =
        _processor->formatAllowanceValueText(S7Builder(&_memHandle).withBaggagePcs(0).build());

    CPPUNIT_ASSERT_EQUAL(BaggageTextFormatter::NO_ALLOWANCE_DATA, allowance);
  }

  void testFormatAllowanceValueText_PcsOnly()
  {
    std::string allowance;

    allowance =
        _processor->formatAllowanceValueText(S7Builder(&_memHandle).withBaggagePcs(10).build());

    CPPUNIT_ASSERT_EQUAL(std::string("10P"), allowance);
  }

  void testFormatAllowanceValueText_WeightOnly()
  {
    std::string allowance;

    allowance = _processor->formatAllowanceValueText(
        S7Builder(&_memHandle).withBaggagePcs(0).withBaggageWeight(5, 'K').build());

    CPPUNIT_ASSERT_EQUAL(std::string("05KG"), allowance);
  }

  void testFormatAllowanceValueText_PcsAndWeight()
  {
    std::string allowance;

    allowance = _processor->formatAllowanceValueText(
        S7Builder(&_memHandle).withBaggagePcs(10).withBaggageWeight(5, 'K').build());

    CPPUNIT_ASSERT_EQUAL(std::string("10P/05KG"), allowance);
  }

  void testPopulatePQWeight_Kilos()
  {
    int32_t quantity(0);
    Indicator unit;

    CPPUNIT_ASSERT(_processor->populatePQWeight(
        S7Builder(&_memHandle).withBaggageWeight(123, 'K').build(), quantity, unit));
    CPPUNIT_ASSERT_EQUAL(123, quantity);
    CPPUNIT_ASSERT_EQUAL('K', unit);
  }

  void testPopulatePQWeight_Pounds()
  {
    int32_t quantity(0);
    Indicator unit;

    CPPUNIT_ASSERT(_processor->populatePQWeight(
        S7Builder(&_memHandle).withBaggageWeight(123, 'P').build(), quantity, unit));
    CPPUNIT_ASSERT_EQUAL(123, quantity);
    CPPUNIT_ASSERT_EQUAL('L', unit);
  }

  void testPopulatePQWeight_Empty()
  {
    int32_t quantity(-1);
    Indicator unit('X');

    CPPUNIT_ASSERT(!_processor->populatePQWeight(
        S7Builder(&_memHandle).withBaggageWeight(0, ' ').build(), quantity, unit));
    CPPUNIT_ASSERT_EQUAL(-1, quantity);
    CPPUNIT_ASSERT_EQUAL('X', unit);
  }

  void testPopulateWeight_Kilos()
  {
    int32_t quantity(0);
    std::string unit;

    CPPUNIT_ASSERT(_processor->populateWeight(
        S7Builder(&_memHandle).withBaggageWeight(123).build(), quantity, unit));
    CPPUNIT_ASSERT_EQUAL(123, quantity);
    CPPUNIT_ASSERT_EQUAL(std::string("KG"), unit);
  }

  void testPopulateWeight_Pounds()
  {
    int32_t quantity(0);
    std::string unit;

    CPPUNIT_ASSERT(_processor->populateWeight(
        S7Builder(&_memHandle).withBaggageWeight(123, 'P').build(), quantity, unit));
    CPPUNIT_ASSERT_EQUAL(123, quantity);
    CPPUNIT_ASSERT_EQUAL(std::string("LB"), unit);
  }

  void testPopulateWeight_Empty()
  {
    int32_t quantity(-1);
    std::string unit("X");

    CPPUNIT_ASSERT(!_processor->populateWeight(
        S7Builder(&_memHandle).withBaggageWeight(0, ' ').build(), quantity, unit));
    CPPUNIT_ASSERT_EQUAL(-1, quantity);
    CPPUNIT_ASSERT_EQUAL(std::string("X"), unit);
  }

  void testPopulatePQPieces()
  {
    int32_t quantity(0);
    Indicator unit;

    CPPUNIT_ASSERT(_processor->populatePQPieces(
        S7Builder(&_memHandle).withBaggagePcs(123).build(), quantity, unit));
    CPPUNIT_ASSERT_EQUAL(123, quantity);
    CPPUNIT_ASSERT_EQUAL('P', unit);
  }

  void testPopulatePQPieces_Empty()
  {
    int32_t quantity(-1);
    Indicator unit('X');

    CPPUNIT_ASSERT(!_processor->populatePQPieces(
        S7Builder(&_memHandle).withBaggagePcs(0).build(), quantity, unit));
    CPPUNIT_ASSERT_EQUAL(-1, quantity);
    CPPUNIT_ASSERT_EQUAL('X', unit);
  }

  void testPopulatePieces()
  {
    int32_t quantity(0);
    std::string unit;

    CPPUNIT_ASSERT(_processor->populatePieces(
        S7Builder(&_memHandle).withBaggagePcs(123).build(), quantity, unit));
    CPPUNIT_ASSERT_EQUAL(123, quantity);
    CPPUNIT_ASSERT_EQUAL(std::string("P"), unit);
  }

  void testGetS5Record_NonExist()
  {
    std::vector<ServiceSubTypeCode> subCodes;

    subCodes.push_back("0G2");

    const SubCodeInfo* result = _processor->getS5Record("ATP", "LO", subCodes);

    CPPUNIT_ASSERT(!result);
  }

  void testGetS5Record_Exist()
  {
    std::vector<ServiceSubTypeCode> subCodes;

    subCodes.push_back("0F2");

    const SubCodeInfo* result = _processor->getS5Record("ATP", "LO", subCodes);

    CPPUNIT_ASSERT(result);
    CPPUNIT_ASSERT_EQUAL(std::string("12"), std::string(result->description1()));
  }

  void testGetServiceSubCodes_Pcs_Matched()
  {
    std::vector<std::string> txtMsgs;
    std::vector<ServiceSubTypeCode> subcodes;

    txtMsgs.push_back(std::string("//10/0F2"));
    txtMsgs.push_back(std::string("//10/0G2"));

    _processor->getServiceSubCodes(10, txtMsgs, subcodes);

    CPPUNIT_ASSERT_EQUAL(2, (int)subcodes.size());
    CPPUNIT_ASSERT_EQUAL(std::string("0F2"), std::string(subcodes[0]));
    CPPUNIT_ASSERT_EQUAL(std::string("0G2"), std::string(subcodes[1]));
  }

  void testGetServiceSubCodes_Pcs_Not_Matched()
  {
    std::vector<std::string> txtMsgs;
    std::vector<ServiceSubTypeCode> subcodes;

    txtMsgs.push_back(std::string("//10/0F2"));
    txtMsgs.push_back(std::string("//10/0G2"));

    _processor->getServiceSubCodes(5, txtMsgs, subcodes);

    CPPUNIT_ASSERT_EQUAL(0, (int)subcodes.size());
  }

  void testGetServiceSubCodes_Pcs_Invalid_Format()
  {
    std::vector<std::string> txtMsgs;
    std::vector<ServiceSubTypeCode> subcodes;

    txtMsgs.push_back(std::string("//10/02"));
    txtMsgs.push_back(std::string("//1/0G2"));

    _processor->getServiceSubCodes(10, txtMsgs, subcodes);

    CPPUNIT_ASSERT_EQUAL(0, (int)subcodes.size());
  }

  void testShouldAddApplicabilityIndicator_Pass()
  {
    CPPUNIT_ASSERT(_processor->shouldAddApplicabilityIndicator(
        S7Builder(&_memHandle).withApplication('3').build()));
  }

  void testShouldAddApplicabilityIndicator_Fail_Null()
  {
    const OptionalServicesInfo* s7 = 0;
    CPPUNIT_ASSERT(!_processor->shouldAddApplicabilityIndicator(s7));
  }

  void testShouldAddApplicabilityIndicator_Fail_Per_Baggage_Travel()
  {
    CPPUNIT_ASSERT(!_processor->shouldAddApplicabilityIndicator(
        S7Builder(&_memHandle).withApplication('4').build()));
  }

  void testFormatChargeText_Asterisks_Present_NonUsDot()
  {
    bool addFeesApplyAtEachCheckInText(false);
    bool temp(false);

    std::string result = _processor->formatChargeText(
        createBaggageCharge(S7Builder(&_memHandle).withApplication('3').withCarrier("LO").build()),
        "",
        addFeesApplyAtEachCheckInText,
        temp);

    CPPUNIT_ASSERT_EQUAL(false, addFeesApplyAtEachCheckInText);
    CPPUNIT_ASSERT(result.find(BaggageTextFormatter::TWO_ASTERISKS) == std::string::npos);
  }

  void testFormatChargeText_Asterisks_Present_UsDot()
  {
    bool addFeesApplyAtEachCheckInText(false);
    bool temp(false);

    _itin->setBaggageTripType(BaggageTripType::TO_FROM_US);

    std::string result = _processor->formatChargeText(
        createBaggageCharge(S7Builder(&_memHandle).withApplication('3').withCarrier("LO").build()),
        "",
        addFeesApplyAtEachCheckInText,
        temp);

    CPPUNIT_ASSERT_EQUAL(true, addFeesApplyAtEachCheckInText);
    CPPUNIT_ASSERT(result.find(BaggageTextFormatter::TWO_ASTERISKS) != std::string::npos);
  }

  void testFormatChargeText_Asterisks_Not_Present()
  {
    bool addFeesApplyAtEachCheckInText(false);
    bool temp(false);

    std::string result = _processor->formatChargeText(
        createBaggageCharge(S7Builder(&_memHandle).withApplication('4').withCarrier("LO").build()),
        "",
        addFeesApplyAtEachCheckInText,
        temp);

    CPPUNIT_ASSERT_EQUAL(false, addFeesApplyAtEachCheckInText);
    CPPUNIT_ASSERT(result.find(BaggageTextFormatter::TWO_ASTERISKS) == std::string::npos);
  }

  void testFormatChargeText_Asterisks_Not_Present_S7_Null()
  {
    bool addFeesApplyAtEachCheckInText(false);
    bool temp(false);

    std::string result = _processor->formatChargeText(
        createBaggageCharge(), "", addFeesApplyAtEachCheckInText, temp);

    CPPUNIT_ASSERT(!addFeesApplyAtEachCheckInText);
    CPPUNIT_ASSERT(result.find(BaggageTextFormatter::TWO_ASTERISKS) == std::string::npos);
  }

  void testFormatChargeText_NoCharge()
  {
    bool addFeesApplyAtEachCheckInText = false;
    bool addAdditionalAllowancesMayApplyText = false;

    std::string carrierText = "LO";

    std::string expected = BaggageTextFormatter::UNKNOWN_INDICATOR + "/" + carrierText + "\n" +
                           BaggageTextFormatter::UNKNOWN_INDICATOR +
                           BaggageTextFormatter::UNKNOWN_FEE_MSG + carrierText;
    std::string result = _processor->formatChargeText(
        createBaggageCharge(S7Builder(&_memHandle).withCarrier(carrierText).build()),
        carrierText,
        addFeesApplyAtEachCheckInText,
        addAdditionalAllowancesMayApplyText);

    CPPUNIT_ASSERT(!addFeesApplyAtEachCheckInText);
    CPPUNIT_ASSERT(!addAdditionalAllowancesMayApplyText);
    CPPUNIT_ASSERT_EQUAL(expected, result);
  }

  void testFormatChargeText()
  {
    //    TIME_BOMB(2013, 4, 12);
    std::string carrierText = "LO";

    BaggageCharge* charge =
        createBaggageCharge(S7Builder(&_memHandle).withCarrier(carrierText).build());
    charge->feeAmount() = 100.0;
    charge->feeCurrency() = "PLN";

    bool addFeesApplyAtEachCheckInText = false;
    bool addAdditionalAllowancesMayApplyText = false;

    std::string expected = "PLN100.00/" + carrierText;
    std::string result = _processor->formatChargeText(
        charge, carrierText, addFeesApplyAtEachCheckInText, addAdditionalAllowancesMayApplyText);

    CPPUNIT_ASSERT(!addFeesApplyAtEachCheckInText);
    CPPUNIT_ASSERT(addAdditionalAllowancesMayApplyText);
    CPPUNIT_ASSERT_EQUAL(expected, result);
  }

  void testFormatChargeText_NoAvailNoCharges_X()
  {
    bool addFeesApplyAtEachCheckInText = false;
    bool addAdditionalAllowancesMayApplyText = false;

    std::string carrierText = "LO";

    std::string expected = "NOT PERMITTED/" + carrierText;
    std::string result = _processor->formatChargeText(
        createBaggageCharge(
            S7Builder(&_memHandle).withCarrier(carrierText).withNotAvailNoCharge('X').build()),
        carrierText,
        addFeesApplyAtEachCheckInText,
        addAdditionalAllowancesMayApplyText);

    CPPUNIT_ASSERT(!addFeesApplyAtEachCheckInText);
    CPPUNIT_ASSERT(addAdditionalAllowancesMayApplyText);
    CPPUNIT_ASSERT_EQUAL(expected, result);
  }

  void testFormatAllowanceText_NoS7()
  {
    std::string carrierText = "LO";

    std::string expected = BaggageTextFormatter::UNKNOWN_INDICATOR + "/" + carrierText;
    std::string result = _processor->formatAllowanceText(0, carrierText);

    CPPUNIT_ASSERT_EQUAL(expected, result);
  }

  void testFormatAllowanceText_S7_NoAllowanceData()
  {
    std::string carrierText = "LO";

    std::string expected = BaggageTextFormatter::NO_ALLOWANCE_DATA + "/" + carrierText;
    std::string result = _processor->formatAllowanceText(
        S7Builder(&_memHandle).withCarrier(carrierText).build(), carrierText);

    CPPUNIT_ASSERT_EQUAL(expected, result);
  }

  void testFormatAllowanceText()
  {
    std::string carrierText = "LO";
    std::string expected = "02P/" + carrierText;

    std::string result = _processor->formatAllowanceText(
        S7Builder(&_memHandle).withCarrier(carrierText).withBaggagePcs(2).build(), carrierText);

    CPPUNIT_ASSERT_EQUAL(expected, result);
  }

  void testGetChargeDescription_NoS7()
  {
    std::string result = _processor->getChargeDescription(0);

    CPPUNIT_ASSERT(result.empty());
  }

  void testGetChargeDescription()
  {
    std::string expected = "TEST12";
    std::string result = _processor->getChargeDescription(
        S7Builder(&_memHandle).withVendor("ATP").withCarrier("LO").withSubTypeCode("0F2").build());

    CPPUNIT_ASSERT_EQUAL(expected, result);
  }

  void testGetTravelText()
  {
    std::vector<TravelSeg*> travelSegs;
    travelSegs += AirSegBuilder(&_memHandle).withOrigin("KRK").build();
    travelSegs += AirSegBuilder(&_memHandle).withDestination("MUC").build();

    std::string expected = "KRKMUC";
    std::string result = _processor->getTravelText(
        BaggageTravelBuilder(&_memHandle).withTravelSegMore(travelSegs).build());

    CPPUNIT_ASSERT_EQUAL(expected, result);
  }

  void testGetCarrierText_CxrTravelSeg_usDot()
  {
    std::string expected = "VA";
    std::string result = _processor->getCarrierText(
        *_trx,
        BaggageTravelBuilder(&_memHandle)
            .withCarrierTravelSeg(AirSegBuilder(&_memHandle).withMarketingCarrier("VA").build())
            .build(),
        true);

    CPPUNIT_ASSERT_EQUAL(expected, result);
  }

  void testGetCarrierText_CxrTravelSeg_nonUsDot_open()
  {
    std::string marketingCarrier = "VA";
    std::string result = _processor->getCarrierText(
        *_trx,
        BaggageTravelBuilder(&_memHandle)
            .withCarrierTravelSeg(AirSegBuilder(&_memHandle)
                                      .withMarketingCarrier(marketingCarrier)
                                      .withOperatingCarrier("XX")
                                      .setType(Open)
                                      .build())
            .build(),
        false);

    CPPUNIT_ASSERT_EQUAL(marketingCarrier, result);
  }

  void testGetCarrierText_CxrTravelSeg_nonUsDot_notOpen()
  {
    std::string marketing = "VA";
    std::string result = _processor->getCarrierText(
        *_trx,
        BaggageTravelBuilder(&_memHandle)
            .withCarrierTravelSeg(AirSegBuilder(&_memHandle)
                                      .withMarketingCarrier(marketing)
                                      .withOperatingCarrier("XX")
                                      .setType(Air)
                                      .build())
            .build(),
        false);

    CPPUNIT_ASSERT_EQUAL(marketing, result);
  }

  void testGetCarrierText_CxrTravelSeg_nonUsDot_notOpen_defer()
  {
    std::string operating = "VA";
    std::string result =
        _processor->getCarrierText(*_trx,
                                   BaggageTravelBuilder(&_memHandle)
                                       .withCarrierTravelSeg(AirSegBuilder(&_memHandle)
                                                                 .withMarketingCarrier("XX")
                                                                 .withOperatingCarrier(operating)
                                                                 .setType(Air)
                                                                 .build())
                                       .withDefer()
                                       .build(),
                                   false);

    CPPUNIT_ASSERT_EQUAL(operating, result);
  }

  void testGetCarrierText_1stTravelSeg_usDot()
  {
    std::vector<TravelSeg*> travelSegs;
    travelSegs += AirSegBuilder(&_memHandle).withMarketingCarrier("LH").build();

    std::string expected = "LH";
    std::string result = _processor->getCarrierText(
        *_trx, BaggageTravelBuilder(&_memHandle).withTravelSeg(travelSegs).build(), true);

    CPPUNIT_ASSERT_EQUAL(expected, result);
  }

  void testGetCarrierText_1stTravelSeg_nonUsDot_open()
  {
    std::string marketingCarrier = "LH";

    std::vector<TravelSeg*> travelSegs;
    travelSegs += AirSegBuilder(&_memHandle)
                      .withMarketingCarrier(marketingCarrier)
                      .withOperatingCarrier("XX")
                      .setType(Open)
                      .build();

    std::string result = _processor->getCarrierText(
        *_trx, BaggageTravelBuilder(&_memHandle).withTravelSeg(travelSegs).build(), true);

    CPPUNIT_ASSERT_EQUAL(marketingCarrier, result);
  }

  void testGetCarrierText_1stTravelSeg_nonUsDot_notOpen()
  {
    std::string marketing = "LH";

    std::vector<TravelSeg*> travelSegs;
    travelSegs += AirSegBuilder(&_memHandle)
                      .withMarketingCarrier(marketing)
                      .withOperatingCarrier("XX")
                      .setType(Air)
                      .build();

    std::string result = _processor->getCarrierText(
        *_trx, BaggageTravelBuilder(&_memHandle).withTravelSeg(travelSegs).build(), false);

    CPPUNIT_ASSERT_EQUAL(marketing, result);
  }

  void testGetDescriptionText_NoS5() { CPPUNIT_ASSERT(_processor->getDescriptionText(0).empty()); }

  void testGetDescriptionText_NoDesc1()
  {
    CPPUNIT_ASSERT(
        _processor->getDescriptionText(S5Builder(&_memHandle).withDesc("").build()).empty());
  }

  void testGetDescriptionText_Desc1()
  {
    std::string expected = "TEST12";
    std::string result =
        _processor->getDescriptionText(S5Builder(&_memHandle).withDesc("12").build());

    CPPUNIT_ASSERT_EQUAL(expected, result);
  }

  void testGetDescriptionText_Desc1And2()
  {
    std::string expected = "TEST12 AND TEST10";
    std::string result =
        _processor->getDescriptionText(S5Builder(&_memHandle).withDesc("12", "10").build());

    CPPUNIT_ASSERT_EQUAL(expected, result);
  }

  void testGetChargeAmountAndCurrencyText_NoCharge()
  {
    std::string result = _processor->getChargeAmountAndCurrencyText(0);

    CPPUNIT_ASSERT_EQUAL(BaggageTextFormatter::UNKNOWN_INDICATOR, result);
  }

  void testGetChargeAmountAndCurrencyText_NotPermitted()
  {
    BaggageCharge charge;
    charge.feeCurrency() = "USD";
    charge.feeAmount() = 123.45;
    charge.optFee() = S7Builder(&_memHandle)
                          .withNotAvailNoCharge(BaggageTextFormatter::NOT_PERMITTED_IND)
                          .build();

    std::string result = _processor->getChargeAmountAndCurrencyText(&charge);

    CPPUNIT_ASSERT_EQUAL(BaggageTextFormatter::NOT_PERMITTED, result);
  }

  void testGetChargeAmountAndCurrencyText_CurrencyAndAmount()
  {
    //    TIME_BOMB(2013, 4, 12);

    BaggageCharge charge;
    charge.feeCurrency() = "USD";
    charge.feeAmount() = 123.45;
    charge.optFee() = S7Builder(&_memHandle).build();

    std::string expected = "USD123.45";
    std::string result = _processor->getChargeAmountAndCurrencyText(&charge);

    CPPUNIT_ASSERT_EQUAL(expected, result);
  }

  void testGetChargeAmountAndCurrencyText_CurrencyAndAmount_noS7()
  {
    //    TIME_BOMB(2013, 4, 12);

    BaggageCharge charge;
    charge.feeCurrency() = "USD";
    charge.feeAmount() = 123.45;
    charge.optFee() = 0;

    std::string expected = "USD123.45";
    std::string result = _processor->getChargeAmountAndCurrencyText(&charge);

    CPPUNIT_ASSERT_EQUAL(expected, result);
  }

  void testGetChargeAmountAndCurrencyText_Unknown()
  {
    BaggageCharge charge;
    charge.feeCurrency() = "";
    charge.feeAmount() = 123.45;

    std::string result = _processor->getChargeAmountAndCurrencyText(&charge);

    CPPUNIT_ASSERT_EQUAL(BaggageTextFormatter::UNKNOWN_INDICATOR, result);
  }

  void testGetAllowanceDescription_NoS7()
  {
    CPPUNIT_ASSERT(_processor->getAllowanceDescription(0).empty());
  }

  void testGetAllowanceDescription_NoTaxTbl()
  {
    CPPUNIT_ASSERT(_processor->getAllowanceDescription(S7Builder(&_memHandle)
                                                           .withBaggagePcs(1)
                                                           .withTaxTblItemNo(0)
                                                           .withVendCarr("ATP", "LO")
                                                           .build()).empty());
  }

  void testGetAllowanceDescription_NoFreePcs()
  {
    CPPUNIT_ASSERT(_processor->getAllowanceDescription(S7Builder(&_memHandle)
                                                           .withBaggagePcs(0)
                                                           .withTaxTblItemNo(12)
                                                           .withVendCarr("ATP", "LO")
                                                           .build()).empty());
  }

  void testGetAllowanceDescription_BlankFreePcs()
  {
    int taxTextCounter = _mdh->getTaxTextCounter();
    CPPUNIT_ASSERT(_processor->getAllowanceDescription(S7Builder(&_memHandle)
                                                           .withBaggagePcs(-1)
                                                           .withTaxTblItemNo(12)
                                                           .withVendCarr("ATP", "LO")
                                                           .build()).empty());

    CPPUNIT_ASSERT_EQUAL(taxTextCounter, _mdh->getTaxTextCounter());
  }

  void testGetAllowanceDescription_NoTaxText()
  {
    CPPUNIT_ASSERT(_processor->getAllowanceDescription(S7Builder(&_memHandle)
                                                           .withBaggagePcs(1)
                                                           .withTaxTblItemNo(99)
                                                           .withVendCarr("ATP", "LO")
                                                           .build()).empty());
  }

  void testGetAllowanceDescription_NoSubCodes()
  {
    CPPUNIT_ASSERT(_processor->getAllowanceDescription(S7Builder(&_memHandle)
                                                           .withBaggagePcs(2)
                                                           .withTaxTblItemNo(12)
                                                           .withVendCarr("ATP", "LO")
                                                           .build()).empty());
  }

  void testGetAllowanceDescription()
  {
    std::string expected = "TEST12";
    std::string result = _processor->getAllowanceDescription(S7Builder(&_memHandle)
                                                                 .withBaggagePcs(1)
                                                                 .withTaxTblItemNo(12)
                                                                 .withVendCarr("ATP", "LO")
                                                                 .build());

    CPPUNIT_ASSERT_EQUAL(expected, result);
  }

private:
  class MyDataHandle : public DataHandleMock
  {
    TestMemHandle _memHandle;
    int _taxTextCounter;

  public:
    void initialize() { _taxTextCounter = 0; }

    int getTaxTextCounter() { return _taxTextCounter; }

    const std::vector<SubCodeInfo*>& getSubCode(const VendorCode& vendor,
                                                const CarrierCode& carrier,
                                                const ServiceTypeCode& serviceTypeCode,
                                                const ServiceGroup& serviceGroup,
                                                const DateTime& date)
    {
      std::vector<SubCodeInfo*>* ret = _memHandle.create<std::vector<SubCodeInfo*> >();
      if (serviceTypeCode == "OC" && serviceGroup == "BG" && carrier == "LO")
      {
        if (vendor == "ATP")
        {
          ret->push_back(S5Builder(&_memHandle)
                             .withSubCode("0F2")
                             .withDesc("12")
                             .withFltTktMerchInd(BAGGAGE_CHARGE)
                             .build());
          return *ret;
        }
        else if (vendor == "MMGR")
        {
          return *ret;
        }
      }
      return DataHandleMock::getSubCode(vendor, carrier, serviceTypeCode, serviceGroup, date);
    }

    const ServicesDescription* getServicesDescription(const ServiceGroupDescription& value)
    {
      ServicesDescription* ret = _memHandle.create<ServicesDescription>();

      if (value == "10")
      {
        ret->description() = "TEST10";
        return ret;
      }
      else if (value == "12")
      {
        ret->description() = "TEST12";
        return ret;
      }
      return DataHandleMock::getServicesDescription(value);
    }

    const TaxText* getTaxText(const VendorCode& vendor, int itemNo)
    {
      _taxTextCounter++;
      if (itemNo == 12)
      {
        TaxText* ret(0);
        ret = _memHandle.create<TaxText>();
        ret->txtMsgs() += "//01/0F2";
        return ret;
      }
      else if (itemNo == 99)
        return 0;

      return DataHandleMock::getTaxText(vendor, itemNo);
    }
  };

  MyDataHandle* _mdh;
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  BaggageTextFormatter* _processor;
  Itin* _itin;
  FarePath* _farePath;
};

CPPUNIT_TEST_SUITE_REGISTRATION(BaggageTextFormatterTest);

} // tse
