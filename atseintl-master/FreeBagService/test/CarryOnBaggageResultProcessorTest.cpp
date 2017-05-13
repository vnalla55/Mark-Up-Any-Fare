// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"

#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/ServicesDescription.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/TaxText.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/AirSeg.h"
#include "DataModel/BaggageCharge.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "FreeBagService/CarryOnBaggageResultProcessor.h"
#include "FreeBagService/test/AirSegBuilder.h"
#include "FreeBagService/test/BaggageTravelBuilder.h"
#include "FreeBagService/test/S5Builder.h"
#include "FreeBagService/test/S7Builder.h"
#include "ServiceFees/OCFees.h"

namespace tse
{

class ItinBuilder
{
  Itin* _itin;
  TestMemHandle* _memHandle;

public:
  ItinBuilder(TestMemHandle* memHandle)
  {
    _memHandle = memHandle;
    _itin = _memHandle->create<Itin>();
  }

  ItinBuilder&
  addAirSegment(CarrierCode carrier, const LocCode& origAirport, const LocCode& destAirport)
  {
    _itin->travelSeg().push_back(AirSegBuilder(_memHandle)
                                     .withOperatingCarrier(carrier)
                                     .withLocations(origAirport, destAirport)
                                     .build());
    return *this;
  }

  Itin* build() { return _itin; }
};

class BaggageChargeBuilder
{
  BaggageCharge* _charge;
  TestMemHandle* _memHandle;

public:
  BaggageChargeBuilder(TestMemHandle* memHandle)
  {
    _memHandle = memHandle;
    _charge = _memHandle->create<BaggageCharge>();
  }

  ~BaggageChargeBuilder() {}

  BaggageChargeBuilder& withAmountAndCurrency(MoneyAmount amount, const CurrencyCode& currency)
  {
    _charge->feeAmount() = amount;
    _charge->feeCurrency() = currency;
    return *this;
  }

  BaggageChargeBuilder& withS5(const SubCodeInfo* s5)
  {
    _charge->subCodeInfo() = s5;
    return *this;
  }

  BaggageChargeBuilder& withS7(const OptionalServicesInfo* s7)
  {
    _charge->optFee() = s7;
    return *this;
  }

  BaggageCharge* build() { return _charge; }
};

class CarryOnBaggageResultProcessorTest : public CppUnit::TestFixture
{
  class CarryOnBaggageResultProcessoDataHandleMock : public DataHandleMock
  {
    TestMemHandle _memHandle;

  public:
    const TaxText* getTaxText(const VendorCode& vendor, int itemNo)
    {
      TaxText* taxText = _memHandle.create<TaxText>();

      static const std::string TAXTEXT_1 = "BAGGAGE IS NOT ALLOWED";
      static const std::string TAXTEXT_2 = "//";
      static const std::string TAXTEXT_3 = "//03/0F3";
      static const std::string TAXTEXT_4 = "//03/0F7";
      static const std::string TAXTEXT_5 = "//01/0F3";
      static const std::string TAXTEXT_6 = "//01/0F3";
      static const std::string TAXTEXT_9 = "//01/0FX";

      switch (itemNo)
      {
      case 0:
        return 0;
      case 1:
        taxText->txtMsgs().push_back(TAXTEXT_1);
        break;
      case 2:
        taxText->txtMsgs().push_back(TAXTEXT_1);
        taxText->txtMsgs().push_back(TAXTEXT_2);
        break;
      case 3:
        taxText->txtMsgs().push_back(TAXTEXT_3);
        break;
      case 4:
        taxText->txtMsgs().push_back(TAXTEXT_4);
        break;
      case 5:
        taxText->txtMsgs().push_back(TAXTEXT_5);
        break;
      case 6:
        taxText->txtMsgs().push_back(TAXTEXT_6);
        break;
      case 9:
        taxText->txtMsgs().push_back(TAXTEXT_9);
        break;
      case 10:
        return 0;
      default:
        taxText->txtMsgs().push_back(TAXTEXT_4);
      }

      return taxText;
    }

    SubCodeInfo* getS5(const ServiceSubTypeCode& serviceSubTypeCode,
                       const ServiceGroupDescription& desc1,
                       const ServiceGroupDescription& desc2,
                       const std::string& commercialName)
    {
      return S5Builder(&_memHandle)
          .withFltTktMerchInd(BAGGAGE_CHARGE)
          .withSubCode(serviceSubTypeCode)
          .withDesc(desc1, desc2)
          .withCommercialName(commercialName)
          .build();
    }

    const std::vector<SubCodeInfo*>& getSubCode(const VendorCode& vendor,
                                                const CarrierCode& carrier,
                                                const ServiceTypeCode& serviceTypeCode,
                                                const ServiceGroup& serviceGroup,
                                                const DateTime& date)
    {
      std::vector<SubCodeInfo*>* subCodes = _memHandle.create<std::vector<SubCodeInfo*> >();
      subCodes->push_back(getS5("0F3", "", "", "EXAMPLE CHARGE NAME"));
      subCodes->push_back(getS5("0F7", "SK", "", "EXAMPLE CHARGE NAME"));
      subCodes->push_back(getS5("0F3", "FA", "32", "EXAMPLE CHARGE NAME"));
      return *subCodes;
    }

    const ServicesDescription* getServicesDescription(const ServiceGroupDescription& value)
    {
      ServicesDescription* ret = _memHandle.create<ServicesDescription>();
      ret->description() = "UP TO 10K";
      return ret;
    }
  };

  CPPUNIT_TEST_SUITE(CarryOnBaggageResultProcessorTest);
  CPPUNIT_TEST(test_formatAllowanceText);
  CPPUNIT_TEST(test_formatAllowanceText_noBT);
  CPPUNIT_TEST(test_formatAllowanceText_singleBT);
  CPPUNIT_TEST(test_formatAllowanceText_treeBT_same_allowance);
  CPPUNIT_TEST(test_formatAllowanceText_treeBT_One_taxTblItemNo_0);
  CPPUNIT_TEST(test_formatAllowanceText_treeBT_taxTblItemNo_0_TaxTextNotFound);
  CPPUNIT_TEST(test_formatAllowanceText_treeBT_taxTblItemNo_0_S5_NotFound);
  CPPUNIT_TEST(test_formatAllowanceText_treeBT_TaxTextNotFound_S5_NotFound);
  CPPUNIT_TEST(test_formatAllowanceText_treeBT_different_taxTblItemNo);
  CPPUNIT_TEST(test_formatAllowanceText_treeBT_different_taxTblItemNo_same_TaxText);
  CPPUNIT_TEST(test_formatAllowanceText_treeBT_taxTblItemNo_0_S5_NotFound);
  CPPUNIT_TEST(test_formatAllowanceText_treeBT_S5_NotFound);
  CPPUNIT_TEST(test_formatAllowanceText_treeBT_S5_Found_S5_NotFound);

  CPPUNIT_TEST(test_formatChargesText_noBT);
  CPPUNIT_TEST(test_formatChargesText_singleBT_singleCharge);
  CPPUNIT_TEST(test_formatChargesText_singleBT_multipleCharge);
  CPPUNIT_TEST(test_formatChargesText_singleBT_multipleCharge_notPermitted);
  CPPUNIT_TEST(test_formatChargesText_singleBT_multipleCharge_additionalNotPermitted);
  CPPUNIT_TEST(test_formatChargesText_mergedBT_singleCharge);
  CPPUNIT_TEST(test_formatChargesText_multipleBT_singleCharge);
  CPPUNIT_TEST(test_formatChargesText_multipleBT_multipleCharge);
  CPPUNIT_TEST(test_formatChargesText_multipleBT_mergedCharges);
  CPPUNIT_TEST(test_formatChargesText_singleBT_doNotProcess);
  CPPUNIT_TEST(test_formatChargesText_singleBT_noCharges);
  CPPUNIT_TEST(test_formatChargesText_multipleBT_noCharges_differentCxr);
  CPPUNIT_TEST(test_formatChargesText_multipleBT_noCharges_sameCxr);
  CPPUNIT_TEST(test_formatChargesText_singleBT_singleCharge_noS7);
  CPPUNIT_TEST(test_formatChargesText_multipleBT_singleCharge_noS7);
  CPPUNIT_TEST(test_formatChargesText_mixedCase);

  CPPUNIT_TEST(testGatherSubCodeQuantities);

  CPPUNIT_TEST_SUITE_END();

  TestMemHandle _memHandle;
  PricingTrx* _trx;
  CarryOnBaggageResultProcessor* _resultProcessor;
  BaggageTravel* _baggageTravel;
  OptionalServicesInfo* _s7;
  Itin* _itin;
  CarryOnBaggageResultProcessoDataHandleMock* _dataHandleMock;

  static const CurrencyCode _defaultCurrency;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    _trx->billing() = _memHandle.create<Billing>();

    _trx->ticketingDate() = DateTime(2013, 1, 1);
    PricingOptions* opts = _memHandle.create<PricingOptions>();
    opts->currencyOverride() = _defaultCurrency;
    _trx->setOptions(opts);
    _resultProcessor = _memHandle.create<CarryOnBaggageResultProcessor>(*_trx);
    _baggageTravel = _memHandle.create<BaggageTravel>();
    _dataHandleMock = _memHandle.create<CarryOnBaggageResultProcessoDataHandleMock>();

    _itin = ItinBuilder(&_memHandle)
                .addAirSegment("LH", "KRK", "WAW")
                .addAirSegment("LH", "WAW", "ARN")
                .build();
  }

  void tearDown() { _memHandle.clear(); }

  SubCodeInfo* getS5(const std::string& commercialName)
  {
    return S5Builder(&_memHandle)
        .withFltTktMerchInd(BAGGAGE_CHARGE)
        .withCommercialName(commercialName)
        .build();
  }

  void test_formatAllowanceText()
  {
    _itin = ItinBuilder(&_memHandle)
                .addAirSegment("AA", "KRK", "WAW")
                .addAirSegment("LH", "WAW", "ARN")
                .addAirSegment("AA", "ARN", "NYO")
                .build();

    std::vector<TravelSeg*>::const_iterator travelSeg1 = _itin->travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator travelSeg2 = _itin->travelSeg().begin() + 1;
    std::vector<TravelSeg*>::const_iterator travelSeg3 = _itin->travelSeg().begin() + 2;

    std::vector<const BaggageTravel*> baggageTravels;

    baggageTravels.push_back(
        BaggageTravelBuilder(&_memHandle).withMSSTravelSeg(travelSeg1).withTrx(_trx).build());

    baggageTravels.push_back(
        BaggageTravelBuilder(&_memHandle).withMSSTravelSeg(travelSeg2).withTrx(_trx).build());

    baggageTravels.push_back(
        BaggageTravelBuilder(&_memHandle).withMSSTravelSeg(travelSeg3).withTrx(_trx).build());

    std::string expected = "CARRY ON ALLOWANCE\n"
                           "KRKWAW ARNNYO-AA-CARRY ON ALLOWANCE UNKNOWN-CONTACT CARRIER\n"
                           "WAWARN-LH-CARRY ON ALLOWANCE UNKNOWN-CONTACT CARRIER\n";

    CPPUNIT_ASSERT_EQUAL(expected, _resultProcessor->formatAllowanceText(baggageTravels));
  }

  void test_formatAllowanceText_noBT()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    std::string expected = EMPTY_STRING();

    CPPUNIT_ASSERT_EQUAL(expected, _resultProcessor->formatAllowanceText(baggageTravels));
  }

  void test_formatAllowanceText_singleBT()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    std::vector<TravelSeg*>::const_iterator travelSeg = _itin->travelSeg().begin();

    OptionalServicesInfo* s7 = S7Builder(&_memHandle)
                                   .withBaggagePcs(2)
                                   .withBaggageWeight(15, 'K')
                                   .withTaxTblItemNo(123)
                                   .build();

    baggageTravels.push_back(BaggageTravelBuilder(&_memHandle)
                                 .withMSSTravelSeg(travelSeg)
                                 .withAllowanceS7(s7)
                                 .withTrx(_trx)
                                 .build());

    std::string expected = "CARRY ON ALLOWANCE\n"
                           "KRKWAW-02P/15KG/LH\n"
                           "03/EACH PIECE UP TO 10K\n";

    CPPUNIT_ASSERT_EQUAL(expected, _resultProcessor->formatAllowanceText(baggageTravels));
  }

  void test_formatAllowanceText_treeBT_same_allowance()
  {
    _itin = ItinBuilder(&_memHandle)
                .addAirSegment("AA", "KRK", "WAW")
                .addAirSegment("AA", "WAW", "ARN")
                .addAirSegment("AA", "ARN", "NYO")
                .build();

    std::vector<TravelSeg*>::const_iterator travelSeg1 = _itin->travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator travelSeg2 = _itin->travelSeg().begin() + 1;
    std::vector<TravelSeg*>::const_iterator travelSeg3 = _itin->travelSeg().begin() + 2;

    OptionalServicesInfo* s7first = S7Builder(&_memHandle)
                                        .withBaggagePcs(2)
                                        .withBaggageWeight(15, 'K')
                                        .withTaxTblItemNo(3)
                                        .build();

    OptionalServicesInfo* s7second = S7Builder(&_memHandle)
                                         .withBaggagePcs(2)
                                         .withBaggageWeight(15, 'K')
                                         .withTaxTblItemNo(3)
                                         .build();

    OptionalServicesInfo* s7third = S7Builder(&_memHandle)
                                        .withBaggagePcs(2)
                                        .withBaggageWeight(15, 'K')
                                        .withTaxTblItemNo(3)
                                        .build();

    std::vector<const BaggageTravel*> baggageTravels;

    baggageTravels.push_back(BaggageTravelBuilder(&_memHandle)
                                 .withMSSTravelSeg(travelSeg1)
                                 .withAllowanceS7(s7first)
                                 .withTrx(_trx)
                                 .build());

    baggageTravels.push_back(BaggageTravelBuilder(&_memHandle)
                                 .withMSSTravelSeg(travelSeg2)
                                 .withAllowanceS7(s7second)
                                 .withTrx(_trx)
                                 .build());

    baggageTravels.push_back(BaggageTravelBuilder(&_memHandle)
                                 .withMSSTravelSeg(travelSeg3)
                                 .withAllowanceS7(s7third)
                                 .withTrx(_trx)
                                 .build());

    std::string expected = "CARRY ON ALLOWANCE\n"
                           "KRKWAW WAWARN ARNNYO-02P/15KG/AA\n"
                           "03/EACH PIECE EXAMPLE CHARGE NAME\n";

    CPPUNIT_ASSERT_EQUAL(expected, _resultProcessor->formatAllowanceText(baggageTravels));
  }

  void test_formatAllowanceText_treeBT_One_taxTblItemNo_0()
  {
    _itin = ItinBuilder(&_memHandle)
                .addAirSegment("AA", "KRK", "WAW")
                .addAirSegment("AA", "WAW", "ARN")
                .addAirSegment("AA", "ARN", "NYO")
                .build();

    std::vector<TravelSeg*>::const_iterator travelSeg1 = _itin->travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator travelSeg2 = _itin->travelSeg().begin() + 1;
    std::vector<TravelSeg*>::const_iterator travelSeg3 = _itin->travelSeg().begin() + 2;

    OptionalServicesInfo* s7first = S7Builder(&_memHandle)
                                        .withBaggagePcs(2)
                                        .withBaggageWeight(15, 'K')
                                        .withTaxTblItemNo(3)
                                        .build();

    OptionalServicesInfo* s7second = S7Builder(&_memHandle)
                                         .withBaggagePcs(2)
                                         .withBaggageWeight(15, 'K')
                                         .withTaxTblItemNo(0)
                                         .build();

    OptionalServicesInfo* s7third = S7Builder(&_memHandle)
                                        .withBaggagePcs(2)
                                        .withBaggageWeight(15, 'K')
                                        .withTaxTblItemNo(3)
                                        .build();

    std::vector<const BaggageTravel*> baggageTravels;

    baggageTravels.push_back(BaggageTravelBuilder(&_memHandle)
                                 .withMSSTravelSeg(travelSeg1)
                                 .withAllowanceS7(s7first)
                                 .withTrx(_trx)
                                 .build());

    baggageTravels.push_back(BaggageTravelBuilder(&_memHandle)
                                 .withMSSTravelSeg(travelSeg2)
                                 .withAllowanceS7(s7second)
                                 .withTrx(_trx)
                                 .build());

    baggageTravels.push_back(BaggageTravelBuilder(&_memHandle)
                                 .withMSSTravelSeg(travelSeg3)
                                 .withAllowanceS7(s7third)
                                 .withTrx(_trx)
                                 .build());

    std::string expected = "CARRY ON ALLOWANCE\n"
                           "KRKWAW ARNNYO-02P/15KG/AA\n"
                           "03/EACH PIECE EXAMPLE CHARGE NAME\n"
                           "WAWARN-02P/15KG/AA\n";

    CPPUNIT_ASSERT_EQUAL(expected, _resultProcessor->formatAllowanceText(baggageTravels));
  }

  void test_formatAllowanceText_treeBT_taxTblItemNo_0_TaxTextNotFound()
  {
    _itin = ItinBuilder(&_memHandle)
                .addAirSegment("AA", "KRK", "WAW")
                .addAirSegment("AA", "WAW", "ARN")
                .addAirSegment("AA", "ARN", "NYO")
                .build();

    std::vector<TravelSeg*>::const_iterator travelSeg1 = _itin->travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator travelSeg2 = _itin->travelSeg().begin() + 1;
    std::vector<TravelSeg*>::const_iterator travelSeg3 = _itin->travelSeg().begin() + 2;

    OptionalServicesInfo* s7first = S7Builder(&_memHandle)
                                        .withBaggagePcs(2)
                                        .withBaggageWeight(15, 'K')
                                        .withTaxTblItemNo(3)
                                        .build();

    OptionalServicesInfo* s7second = S7Builder(&_memHandle)
                                         .withBaggagePcs(2)
                                         .withBaggageWeight(15, 'K')
                                         .withTaxTblItemNo(0)
                                         .build();

    OptionalServicesInfo* s7third = S7Builder(&_memHandle)
                                        .withBaggagePcs(2)
                                        .withBaggageWeight(15, 'K')
                                        .withTaxTblItemNo(10)
                                        .build();

    std::vector<const BaggageTravel*> baggageTravels;

    baggageTravels.push_back(BaggageTravelBuilder(&_memHandle)
                                 .withMSSTravelSeg(travelSeg1)
                                 .withAllowanceS7(s7first)
                                 .withTrx(_trx)
                                 .build());

    baggageTravels.push_back(BaggageTravelBuilder(&_memHandle)
                                 .withMSSTravelSeg(travelSeg2)
                                 .withAllowanceS7(s7second)
                                 .withTrx(_trx)
                                 .build());

    baggageTravels.push_back(BaggageTravelBuilder(&_memHandle)
                                 .withMSSTravelSeg(travelSeg3)
                                 .withAllowanceS7(s7third)
                                 .withTrx(_trx)
                                 .build());

    std::string expected = "CARRY ON ALLOWANCE\n"
                           "KRKWAW-02P/15KG/AA\n"
                           "03/EACH PIECE EXAMPLE CHARGE NAME\n"
                           "WAWARN ARNNYO-02P/15KG/AA\n";

    CPPUNIT_ASSERT_EQUAL(expected, _resultProcessor->formatAllowanceText(baggageTravels));
  }

  void test_formatAllowanceText_treeBT_taxTblItemNo_0_S5_NotFound()
  {
    _itin = ItinBuilder(&_memHandle)
                .addAirSegment("AA", "KRK", "WAW")
                .addAirSegment("AA", "WAW", "ARN")
                .addAirSegment("AA", "ARN", "NYO")
                .build();

    std::vector<TravelSeg*>::const_iterator travelSeg1 = _itin->travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator travelSeg2 = _itin->travelSeg().begin() + 1;
    std::vector<TravelSeg*>::const_iterator travelSeg3 = _itin->travelSeg().begin() + 2;

    OptionalServicesInfo* s7first = S7Builder(&_memHandle)
                                        .withBaggagePcs(2)
                                        .withBaggageWeight(15, 'K')
                                        .withTaxTblItemNo(3)
                                        .build();

    OptionalServicesInfo* s7second = S7Builder(&_memHandle)
                                         .withBaggagePcs(2)
                                         .withBaggageWeight(15, 'K')
                                         .withTaxTblItemNo(0)
                                         .build();

    OptionalServicesInfo* s7third = S7Builder(&_memHandle)
                                        .withBaggagePcs(2)
                                        .withBaggageWeight(15, 'K')
                                        .withTaxTblItemNo(9)
                                        .build();

    std::vector<const BaggageTravel*> baggageTravels;

    baggageTravels.push_back(BaggageTravelBuilder(&_memHandle)
                                 .withMSSTravelSeg(travelSeg1)
                                 .withAllowanceS7(s7first)
                                 .withTrx(_trx)
                                 .build());

    baggageTravels.push_back(BaggageTravelBuilder(&_memHandle)
                                 .withMSSTravelSeg(travelSeg2)
                                 .withAllowanceS7(s7second)
                                 .withTrx(_trx)
                                 .build());

    baggageTravels.push_back(BaggageTravelBuilder(&_memHandle)
                                 .withMSSTravelSeg(travelSeg3)
                                 .withAllowanceS7(s7third)
                                 .withTrx(_trx)
                                 .build());

    std::string expected = "CARRY ON ALLOWANCE\n"
                           "KRKWAW-02P/15KG/AA\n"
                           "03/EACH PIECE EXAMPLE CHARGE NAME\n"
                           "WAWARN ARNNYO-02P/15KG/AA\n";

    CPPUNIT_ASSERT_EQUAL(expected, _resultProcessor->formatAllowanceText(baggageTravels));
  }

  void test_formatAllowanceText_treeBT_S5_NotFound()
  {
    _itin = ItinBuilder(&_memHandle)
                .addAirSegment("AA", "KRK", "WAW")
                .addAirSegment("AA", "WAW", "ARN")
                .addAirSegment("AA", "ARN", "NYO")
                .build();

    std::vector<TravelSeg*>::const_iterator travelSeg1 = _itin->travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator travelSeg2 = _itin->travelSeg().begin() + 1;
    std::vector<TravelSeg*>::const_iterator travelSeg3 = _itin->travelSeg().begin() + 2;

    std::vector<const BaggageTravel*> baggageTravels;

    baggageTravels.push_back(
        BaggageTravelBuilder(&_memHandle).withMSSTravelSeg(travelSeg1).withTrx(_trx).build());

    baggageTravels.push_back(
        BaggageTravelBuilder(&_memHandle).withMSSTravelSeg(travelSeg2).withTrx(_trx).build());

    baggageTravels.push_back(
        BaggageTravelBuilder(&_memHandle).withMSSTravelSeg(travelSeg3).withTrx(_trx).build());

    std::string expected = "CARRY ON ALLOWANCE\n"
                           "KRKWAW WAWARN ARNNYO-AA-CARRY ON ALLOWANCE UNKNOWN-CONTACT CARRIER\n";

    CPPUNIT_ASSERT_EQUAL(expected, _resultProcessor->formatAllowanceText(baggageTravels));
  }

  void test_formatAllowanceText_treeBT_S5_Found_S5_NotFound()
  {
    _itin = ItinBuilder(&_memHandle)
                .addAirSegment("AA", "KRK", "WAW")
                .addAirSegment("AA", "WAW", "ARN")
                .addAirSegment("AA", "ARN", "NYO")
                .build();

    std::vector<TravelSeg*>::const_iterator travelSeg1 = _itin->travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator travelSeg2 = _itin->travelSeg().begin() + 1;
    std::vector<TravelSeg*>::const_iterator travelSeg3 = _itin->travelSeg().begin() + 2;

    std::vector<const BaggageTravel*> baggageTravels;

    baggageTravels.push_back(
        BaggageTravelBuilder(&_memHandle).withMSSTravelSeg(travelSeg1).withTrx(_trx).build());

    baggageTravels.push_back(BaggageTravelBuilder(&_memHandle)
                                 .withMSSTravelSeg(travelSeg2)
                                 .withAllowanceS5()
                                 .withTrx(_trx)
                                 .build());

    baggageTravels.push_back(
        BaggageTravelBuilder(&_memHandle).withMSSTravelSeg(travelSeg3).withTrx(_trx).build());

    std::string expected = "CARRY ON ALLOWANCE\n"
                           "KRKWAW WAWARN ARNNYO-AA-CARRY ON ALLOWANCE UNKNOWN-CONTACT CARRIER\n";

    CPPUNIT_ASSERT_EQUAL(expected, _resultProcessor->formatAllowanceText(baggageTravels));
  }

  void test_formatAllowanceText_treeBT_TaxTextNotFound_S5_NotFound()
  {
    _itin = ItinBuilder(&_memHandle)
                .addAirSegment("AA", "KRK", "WAW")
                .addAirSegment("AA", "WAW", "ARN")
                .addAirSegment("AA", "ARN", "NYO")
                .build();

    std::vector<TravelSeg*>::const_iterator travelSeg1 = _itin->travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator travelSeg2 = _itin->travelSeg().begin() + 1;
    std::vector<TravelSeg*>::const_iterator travelSeg3 = _itin->travelSeg().begin() + 2;

    OptionalServicesInfo* s7first = S7Builder(&_memHandle)
                                        .withBaggagePcs(2)
                                        .withBaggageWeight(15, 'K')
                                        .withTaxTblItemNo(3)
                                        .build();

    OptionalServicesInfo* s7second = S7Builder(&_memHandle)
                                         .withBaggagePcs(2)
                                         .withBaggageWeight(15, 'K')
                                         .withTaxTblItemNo(9)
                                         .build();

    OptionalServicesInfo* s7third = S7Builder(&_memHandle)
                                        .withBaggagePcs(2)
                                        .withBaggageWeight(15, 'K')
                                        .withTaxTblItemNo(10)
                                        .build();

    std::vector<const BaggageTravel*> baggageTravels;

    baggageTravels.push_back(BaggageTravelBuilder(&_memHandle)
                                 .withMSSTravelSeg(travelSeg1)
                                 .withAllowanceS7(s7first)
                                 .withTrx(_trx)
                                 .build());

    baggageTravels.push_back(BaggageTravelBuilder(&_memHandle)
                                 .withMSSTravelSeg(travelSeg2)
                                 .withAllowanceS7(s7second)
                                 .withTrx(_trx)
                                 .build());

    baggageTravels.push_back(BaggageTravelBuilder(&_memHandle)
                                 .withMSSTravelSeg(travelSeg3)
                                 .withAllowanceS7(s7third)
                                 .withTrx(_trx)
                                 .build());

    std::string expected = "CARRY ON ALLOWANCE\n"
                           "KRKWAW-02P/15KG/AA\n"
                           "03/EACH PIECE EXAMPLE CHARGE NAME\n"
                           "WAWARN ARNNYO-02P/15KG/AA\n";

    CPPUNIT_ASSERT_EQUAL(expected, _resultProcessor->formatAllowanceText(baggageTravels));
  }

  void test_formatAllowanceText_treeBT_different_taxTblItemNo()
  {
    _itin = ItinBuilder(&_memHandle)
                .addAirSegment("AA", "KRK", "WAW")
                .addAirSegment("AA", "WAW", "ARN")
                .addAirSegment("AA", "ARN", "NYO")
                .build();

    std::vector<TravelSeg*>::const_iterator travelSeg1 = _itin->travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator travelSeg2 = _itin->travelSeg().begin() + 1;
    std::vector<TravelSeg*>::const_iterator travelSeg3 = _itin->travelSeg().begin() + 2;

    OptionalServicesInfo* s7first = S7Builder(&_memHandle)
                                        .withBaggagePcs(2)
                                        .withBaggageWeight(15, 'K')
                                        .withTaxTblItemNo(3)
                                        .build();

    OptionalServicesInfo* s7second = S7Builder(&_memHandle)
                                         .withBaggagePcs(2)
                                         .withBaggageWeight(15, 'K')
                                         .withTaxTblItemNo(4)
                                         .build();

    OptionalServicesInfo* s7third = S7Builder(&_memHandle)
                                        .withBaggagePcs(2)
                                        .withBaggageWeight(15, 'K')
                                        .withTaxTblItemNo(3)
                                        .build();

    std::vector<const BaggageTravel*> baggageTravels;

    baggageTravels.push_back(BaggageTravelBuilder(&_memHandle)
                                 .withMSSTravelSeg(travelSeg1)
                                 .withAllowanceS7(s7first)
                                 .withTrx(_trx)
                                 .build());

    baggageTravels.push_back(BaggageTravelBuilder(&_memHandle)
                                 .withMSSTravelSeg(travelSeg2)
                                 .withAllowanceS7(s7second)
                                 .withTrx(_trx)
                                 .build());

    baggageTravels.push_back(BaggageTravelBuilder(&_memHandle)
                                 .withMSSTravelSeg(travelSeg3)
                                 .withAllowanceS7(s7third)
                                 .withTrx(_trx)
                                 .build());

    std::string expected = "CARRY ON ALLOWANCE\n"
                           "KRKWAW ARNNYO-02P/15KG/AA\n"
                           "03/EACH PIECE EXAMPLE CHARGE NAME\n"
                           "WAWARN-02P/15KG/AA\n"
                           "03/EACH PIECE UP TO 10K\n";

    CPPUNIT_ASSERT_EQUAL(expected, _resultProcessor->formatAllowanceText(baggageTravels));
  }

  void test_formatAllowanceText_treeBT_different_taxTblItemNo_same_TaxText()
  {
    _itin = ItinBuilder(&_memHandle)
                .addAirSegment("AA", "KRK", "WAW")
                .addAirSegment("AA", "WAW", "ARN")
                .addAirSegment("AA", "ARN", "NYO")
                .build();

    std::vector<TravelSeg*>::const_iterator travelSeg1 = _itin->travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator travelSeg2 = _itin->travelSeg().begin() + 1;
    std::vector<TravelSeg*>::const_iterator travelSeg3 = _itin->travelSeg().begin() + 2;

    OptionalServicesInfo* s7first = S7Builder(&_memHandle)
                                        .withBaggagePcs(2)
                                        .withBaggageWeight(15, 'K')
                                        .withTaxTblItemNo(13)
                                        .build();

    OptionalServicesInfo* s7second = S7Builder(&_memHandle)
                                         .withBaggagePcs(2)
                                         .withBaggageWeight(15, 'K')
                                         .withTaxTblItemNo(4)
                                         .build();

    OptionalServicesInfo* s7third = S7Builder(&_memHandle)
                                        .withBaggagePcs(2)
                                        .withBaggageWeight(15, 'K')
                                        .withTaxTblItemNo(3)
                                        .build();

    std::vector<const BaggageTravel*> baggageTravels;

    baggageTravels.push_back(BaggageTravelBuilder(&_memHandle)
                                 .withMSSTravelSeg(travelSeg1)
                                 .withAllowanceS7(s7first)
                                 .withTrx(_trx)
                                 .build());

    baggageTravels.push_back(BaggageTravelBuilder(&_memHandle)
                                 .withMSSTravelSeg(travelSeg2)
                                 .withAllowanceS7(s7second)
                                 .withTrx(_trx)
                                 .build());

    baggageTravels.push_back(BaggageTravelBuilder(&_memHandle)
                                 .withMSSTravelSeg(travelSeg3)
                                 .withAllowanceS7(s7third)
                                 .withTrx(_trx)
                                 .build());

    std::string expected = "CARRY ON ALLOWANCE\n"
                           "KRKWAW WAWARN-02P/15KG/AA\n"
                           "03/EACH PIECE UP TO 10K\n"
                           "ARNNYO-02P/15KG/AA\n"
                           "03/EACH PIECE EXAMPLE CHARGE NAME\n";

    CPPUNIT_ASSERT_EQUAL(expected, _resultProcessor->formatAllowanceText(baggageTravels));
  }

  void test_formatChargesText_noBT()
  {
    std::vector<const BaggageTravel*> baggageTravels;
    std::string expected = EMPTY_STRING();

    CPPUNIT_ASSERT_EQUAL(expected, _resultProcessor->formatChargesText(baggageTravels));
  }

  void test_formatChargesText_singleBT_singleCharge()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    // 1st charge
    SubCodeInfo* s5 = getS5("EXAMPLE CHARGE NAME");
    OptionalServicesInfo* s7 = S7Builder(&_memHandle)
                                   .withBaggageOccurrence(1, 2)
                                   .withBaggageWeight(10, 'K')
                                   .withApplication('H')
                                   .build();
    BaggageCharge* charge = BaggageChargeBuilder(&_memHandle)
                                .withAmountAndCurrency(123, _defaultCurrency)
                                .withS5(s5)
                                .withS7(s7)
                                .build();
    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("KRK")
                          .withDestination("DFW")
                          .withOperatingCarrier("AA")
                          .withBaggageCharge(charge)
                          .withTrx(_trx)
                          .build();

    std::string expected = "CARRY ON CHARGES\n"
                           "KRKDFW-AA\n"
                           "1ST-2ND OVER 10KG EXAMPLE CHARGE NAME-USD123.00 PER KILO\n";

    CPPUNIT_ASSERT_EQUAL(expected, _resultProcessor->formatChargesText(baggageTravels));
  }

  void test_formatChargesText_singleBT_multipleCharge()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    // 1st charge
    SubCodeInfo* s5 = getS5("EXAMPLE FIRST CHARGE NAME");
    OptionalServicesInfo* s7 = S7Builder(&_memHandle)
                                   .withBaggageOccurrence(1, 2)
                                   .withBaggageWeight(10, 'K')
                                   .withApplication('H')
                                   .build();
    BaggageCharge* firstCharge = BaggageChargeBuilder(&_memHandle)
                                     .withAmountAndCurrency(123, _defaultCurrency)
                                     .withS5(s5)
                                     .withS7(s7)
                                     .build();

    // 2nd charge
    s5 = getS5("EXAMPLE SECOND CHARGE NAME");
    s7 = S7Builder(&_memHandle)
             .withBaggageOccurrence(3, 3)
             .withBaggageWeight(20, 'L')
             .withApplication('H')
             .build();
    BaggageCharge* secondCharge = BaggageChargeBuilder(&_memHandle)
                                      .withAmountAndCurrency(456, _defaultCurrency)
                                      .withS5(s5)
                                      .withS7(s7)
                                      .build();

    // 3rd charge
    s5 = getS5("EXAMPLE THIRD CHARGE NAME");
    s7 = S7Builder(&_memHandle)
             .withBaggageOccurrence(4, 0)
             .withBaggageWeight(30, 'K')
             .withApplication('F')
             .build();
    BaggageCharge* thirdCharge = BaggageChargeBuilder(&_memHandle)
                                     .withAmountAndCurrency(789, _defaultCurrency)
                                     .withS5(s5)
                                     .withS7(s7)
                                     .build();

    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("KRK")
                          .withDestination("DFW")
                          .withOperatingCarrier("AA")
                          .withBaggageCharge(firstCharge)
                          .withBaggageCharge(secondCharge)
                          .withBaggageCharge(thirdCharge)
                          .withTrx(_trx)
                          .build();

    std::string expected =
        "CARRY ON CHARGES\n"
        "KRKDFW-AA\n"
        "1ST-2ND OVER 10KG EXAMPLE FIRST CHARGE NAME-USD123.00 PER KILO\n"
        "3RD OVER 20LB EXAMPLE SECOND CHARGE NAME-USD456.00 PER KILO\n"
        "4TH AND EACH ABOVE 4TH OVER 30KG EXAMPLE THIRD CHARGE NAME-USD789.00 PER 5 KILOS\n";

    CPPUNIT_ASSERT_EQUAL(expected, _resultProcessor->formatChargesText(baggageTravels));
  }

  void test_formatChargesText_singleBT_multipleCharge_notPermitted()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    // 1st charge
    SubCodeInfo* s5 = getS5("EXAMPLE FIRST CHARGE NAME");
    OptionalServicesInfo* s7 = S7Builder(&_memHandle)
                                   .withBaggageOccurrence(0, 0)
                                   .withBaggageWeight(10, 'K')
                                   .withApplication('C')
                                   .build();
    BaggageCharge* firstCharge = BaggageChargeBuilder(&_memHandle)
                                     .withAmountAndCurrency(123, _defaultCurrency)
                                     .withS5(s5)
                                     .withS7(s7)
                                     .build();

    // 2nd charge
    s5 = getS5("EXAMPLE SECOND CHARGE NAME");
    s7 = S7Builder(&_memHandle).withBaggageOccurrence(3, 4).withNotAvailNoCharge('X').build();
    BaggageCharge* secondCharge = BaggageChargeBuilder(&_memHandle).withS5(s5).withS7(s7).build();

    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("NYC")
                          .withDestination("LAX")
                          .withOperatingCarrier("VA")
                          .withBaggageCharge(firstCharge)
                          .withBaggageCharge(secondCharge)
                          .withTrx(_trx)
                          .build();

    std::string expected = "CARRY ON CHARGES\n"
                           "NYCLAX-VA\n"
                           "OVER 10KG EXAMPLE FIRST CHARGE NAME-USD123.00 PER KILO\n"
                           "EXAMPLE SECOND CHARGE NAME-NOT PERMITTED\n";

    CPPUNIT_ASSERT_EQUAL(expected, _resultProcessor->formatChargesText(baggageTravels));
  }

  void test_formatChargesText_singleBT_multipleCharge_additionalNotPermitted()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    // 1st charge
    SubCodeInfo* s5 = getS5("EXAMPLE FIRST CHARGE NAME");
    OptionalServicesInfo* s7 = S7Builder(&_memHandle)
                                   .withBaggageOccurrence(0, 0)
                                   .withBaggageWeight(10, 'K')
                                   .withApplication('C')
                                   .build();
    BaggageCharge* firstCharge = BaggageChargeBuilder(&_memHandle)
                                     .withAmountAndCurrency(123, _defaultCurrency)
                                     .withS5(s5)
                                     .withS7(s7)
                                     .build();

    // 2nd charge
    // use same s5 as the previous charge for "ADDITIONAL" text
    s7 = S7Builder(&_memHandle).withBaggageOccurrence(3, 4).withNotAvailNoCharge('X').build();
    BaggageCharge* secondCharge = BaggageChargeBuilder(&_memHandle).withS5(s5).withS7(s7).build();

    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("NYC")
                          .withDestination("LAX")
                          .withOperatingCarrier("VA")
                          .withBaggageCharge(firstCharge)
                          .withBaggageCharge(secondCharge)
                          .withTrx(_trx)
                          .build();

    std::string expected = "CARRY ON CHARGES\n"
                           "NYCLAX-VA\n"
                           "OVER 10KG EXAMPLE FIRST CHARGE NAME-USD123.00 PER KILO\n"
                           "ADDITIONAL EXAMPLE FIRST CHARGE NAME-NOT PERMITTED\n";

    CPPUNIT_ASSERT_EQUAL(expected, _resultProcessor->formatChargesText(baggageTravels));
  }

  void test_formatChargesText_mergedBT_singleCharge()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    // 1st charge
    SubCodeInfo* s5 = getS5("EXAMPLE CHARGE NAME");
    OptionalServicesInfo* s7 = S7Builder(&_memHandle)
                                   .withBaggageOccurrence(1, 2)
                                   .withBaggageWeight(10, 'K')
                                   .withApplication('H')
                                   .build();
    BaggageCharge* charge = BaggageChargeBuilder(&_memHandle)
                                .withAmountAndCurrency(123, _defaultCurrency)
                                .withS5(s5)
                                .withS7(s7)
                                .build();

    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("KRK")
                          .withDestination("FRA")
                          .withOperatingCarrier("AA")
                          .withBaggageCharge(charge)
                          .withTrx(_trx)
                          .build();
    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("FRA")
                          .withDestination("DFW")
                          .withOperatingCarrier("AA")
                          .withBaggageCharge(charge)
                          .withTrx(_trx)
                          .build();

    std::string expected = "CARRY ON CHARGES\n"
                           "KRKFRA FRADFW-AA\n"
                           "1ST-2ND OVER 10KG EXAMPLE CHARGE NAME-USD123.00 PER KILO\n";

    CPPUNIT_ASSERT_EQUAL(expected, _resultProcessor->formatChargesText(baggageTravels));
  }

  void test_formatChargesText_multipleBT_singleCharge()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    // 1st baggage travel
    SubCodeInfo* s5 = getS5("EXAMPLE CHARGE NAME");
    OptionalServicesInfo* s7 = S7Builder(&_memHandle)
                                   .withBaggageOccurrence(1, 2)
                                   .withBaggageWeight(10, 'K')
                                   .withApplication('H')
                                   .build();
    BaggageCharge* charge = BaggageChargeBuilder(&_memHandle)
                                .withAmountAndCurrency(123, _defaultCurrency)
                                .withS5(s5)
                                .withS7(s7)
                                .build();
    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("KRK")
                          .withDestination("FRA")
                          .withOperatingCarrier("AA")
                          .withBaggageCharge(charge)
                          .withTrx(_trx)
                          .build();

    // 2nd baggage travel
    s5 = getS5("OTHER EXAMPLE CHARGE NAME");
    s7 = S7Builder(&_memHandle)
             .withBaggageOccurrence(3, 0)
             .withBaggageWeight(20, 'L')
             .withNotAvailNoCharge('X')
             .build();
    charge = BaggageChargeBuilder(&_memHandle).withS5(s5).withS7(s7).build();
    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("FRA")
                          .withDestination("DFW")
                          .withOperatingCarrier("LH")
                          .withBaggageCharge(charge)
                          .withTrx(_trx)
                          .build();

    std::string expected = "CARRY ON CHARGES\n"
                           "KRKFRA-AA\n"
                           "1ST-2ND OVER 10KG EXAMPLE CHARGE NAME-USD123.00 PER KILO\n"
                           "FRADFW-LH\n"
                           "OVER 20LB OTHER EXAMPLE CHARGE NAME-NOT PERMITTED\n";

    CPPUNIT_ASSERT_EQUAL(expected, _resultProcessor->formatChargesText(baggageTravels));
  }

  void test_formatChargesText_multipleBT_multipleCharge()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    // 1st baggage travel
    SubCodeInfo* s5 = getS5("EXAMPLE FIRST CHARGE NAME");
    OptionalServicesInfo* s7 = S7Builder(&_memHandle)
                                   .withBaggageOccurrence(1, 2)
                                   .withBaggageWeight(10, 'K')
                                   .withApplication('H')
                                   .build();
    BaggageCharge* firstCharge = BaggageChargeBuilder(&_memHandle)
                                     .withAmountAndCurrency(123, _defaultCurrency)
                                     .withS5(s5)
                                     .withS7(s7)
                                     .build();

    s5 = getS5("EXAMPLE SECOND CHARGE NAME");
    s7 = S7Builder(&_memHandle)
             .withBaggageOccurrence(3, 3)
             .withBaggageWeight(20, 'L')
             .withApplication('H')
             .build();
    BaggageCharge* secondCharge = BaggageChargeBuilder(&_memHandle)
                                      .withAmountAndCurrency(456, _defaultCurrency)
                                      .withS5(s5)
                                      .withS7(s7)
                                      .build();

    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("KRK")
                          .withDestination("DFW")
                          .withOperatingCarrier("AA")
                          .withBaggageCharge(firstCharge)
                          .withBaggageCharge(secondCharge)
                          .withTrx(_trx)
                          .build();

    // 2nd baggage travel
    s5 = getS5("2ND EXAMPLE FIRST CHARGE NAME");
    s7 = S7Builder(&_memHandle)
             .withBaggageOccurrence(0, 0)
             .withBaggageWeight(10, 'K')
             .withApplication('H')
             .build();
    firstCharge = BaggageChargeBuilder(&_memHandle)
                      .withAmountAndCurrency(123, _defaultCurrency)
                      .withS5(s5)
                      .withS7(s7)
                      .build();

    s5 = getS5("2ND EXAMPLE SECOND CHARGE NAME");
    s7 = S7Builder(&_memHandle).withBaggageOccurrence(3, 4).build();
    secondCharge = BaggageChargeBuilder(&_memHandle)
                       .withAmountAndCurrency(456, _defaultCurrency)
                       .withS5(s5)
                       .withS7(s7)
                       .build();

    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("DFW")
                          .withDestination("NYC")
                          .withOperatingCarrier("UA")
                          .withBaggageCharge(firstCharge)
                          .withBaggageCharge(secondCharge)
                          .withTrx(_trx)
                          .build();

    // 3rd baggage travel
    s5 = getS5("3RD EXAMPLE FIRST CHARGE NAME");
    s7 = S7Builder(&_memHandle)
             .withBaggageOccurrence(0, 0)
             .withBaggageWeight(10, 'K')
             .withApplication('C')
             .build();
    firstCharge = BaggageChargeBuilder(&_memHandle)
                      .withAmountAndCurrency(123, _defaultCurrency)
                      .withS5(s5)
                      .withS7(s7)
                      .build();

    // use same s5 as the previous charge for "ADDITIONAL" text
    s7 = S7Builder(&_memHandle).withBaggageOccurrence(3, 4).withNotAvailNoCharge('X').build();
    secondCharge = BaggageChargeBuilder(&_memHandle).withS5(s5).withS7(s7).build();

    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("NYC")
                          .withDestination("LAX")
                          .withOperatingCarrier("VA")
                          .withBaggageCharge(firstCharge)
                          .withBaggageCharge(secondCharge)
                          .withTrx(_trx)
                          .build();

    std::string expected = "CARRY ON CHARGES\n"
                           "KRKDFW-AA\n"
                           "1ST-2ND OVER 10KG EXAMPLE FIRST CHARGE NAME-USD123.00 PER KILO\n"
                           "3RD OVER 20LB EXAMPLE SECOND CHARGE NAME-USD456.00 PER KILO\n"
                           "DFWNYC-UA\n"
                           "OVER 10KG 2ND EXAMPLE FIRST CHARGE NAME-USD123.00 PER KILO\n"
                           "3RD-4TH 2ND EXAMPLE SECOND CHARGE NAME-USD456.00\n"
                           "NYCLAX-VA\n"
                           "OVER 10KG 3RD EXAMPLE FIRST CHARGE NAME-USD123.00 PER KILO\n"
                           "ADDITIONAL 3RD EXAMPLE FIRST CHARGE NAME-NOT PERMITTED\n";

    CPPUNIT_ASSERT_EQUAL(expected, _resultProcessor->formatChargesText(baggageTravels));
  }

  void test_formatChargesText_multipleBT_mergedCharges()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    // 1st baggage travel
    SubCodeInfo* s5 = getS5("EXAMPLE FIRST CHARGE NAME");
    OptionalServicesInfo* s7 = S7Builder(&_memHandle)
                                   .withBaggageOccurrence(1, 2)
                                   .withBaggageWeight(10, 'K')
                                   .withApplication('H')
                                   .build();
    BaggageCharge* firstChargeToMerge = BaggageChargeBuilder(&_memHandle)
                                            .withAmountAndCurrency(123, _defaultCurrency)
                                            .withS5(s5)
                                            .withS7(s7)
                                            .build();

    s5 = getS5("EXAMPLE SECOND CHARGE NAME");
    s7 = S7Builder(&_memHandle)
             .withBaggageOccurrence(3, 3)
             .withBaggageWeight(20, 'L')
             .withApplication('H')
             .build();
    BaggageCharge* secondChargeToMerge = BaggageChargeBuilder(&_memHandle)
                                             .withAmountAndCurrency(456, _defaultCurrency)
                                             .withS5(s5)
                                             .withS7(s7)
                                             .build();

    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("KRK")
                          .withDestination("DFW")
                          .withOperatingCarrier("AA")
                          .withBaggageCharge(firstChargeToMerge)
                          .withBaggageCharge(secondChargeToMerge)
                          .withTrx(_trx)
                          .build();

    // 2nd baggage travel
    s5 = getS5("2ND EXAMPLE FIRST CHARGE NAME");
    s7 = S7Builder(&_memHandle)
             .withBaggageOccurrence(0, 0)
             .withBaggageWeight(10, 'K')
             .withApplication('H')
             .build();
    BaggageCharge* secondCharge = BaggageChargeBuilder(&_memHandle)
                                      .withAmountAndCurrency(123, _defaultCurrency)
                                      .withS5(s5)
                                      .withS7(s7)
                                      .build();

    s5 = getS5("2ND EXAMPLE SECOND CHARGE NAME");
    s7 = S7Builder(&_memHandle).withBaggageOccurrence(3, 4).build();
    BaggageCharge* thirdCharge = BaggageChargeBuilder(&_memHandle)
                                     .withAmountAndCurrency(456, _defaultCurrency)
                                     .withS5(s5)
                                     .withS7(s7)
                                     .build();

    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("DFW")
                          .withDestination("NYC")
                          .withOperatingCarrier("UA")
                          .withBaggageCharge(secondCharge)
                          .withBaggageCharge(thirdCharge)
                          .withTrx(_trx)
                          .build();

    // 3rd baggage travel
    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("NYC")
                          .withDestination("LAX")
                          .withOperatingCarrier("VA")
                          .withBaggageCharge(firstChargeToMerge)
                          .withBaggageCharge(secondChargeToMerge)
                          .withTrx(_trx)
                          .build();

    std::string expected = "CARRY ON CHARGES\n"
                           "KRKDFW NYCLAX-AA\n"
                           "1ST-2ND OVER 10KG EXAMPLE FIRST CHARGE NAME-USD123.00 PER KILO\n"
                           "3RD OVER 20LB EXAMPLE SECOND CHARGE NAME-USD456.00 PER KILO\n"
                           "DFWNYC-UA\n"
                           "OVER 10KG 2ND EXAMPLE FIRST CHARGE NAME-USD123.00 PER KILO\n"
                           "3RD-4TH 2ND EXAMPLE SECOND CHARGE NAME-USD456.00\n";

    CPPUNIT_ASSERT_EQUAL(expected, _resultProcessor->formatChargesText(baggageTravels));
  }

  void test_formatChargesText_singleBT_doNotProcess()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    // 1st charge
    SubCodeInfo* s5 = getS5("EXAMPLE CHARGE NAME");
    OptionalServicesInfo* s7 = S7Builder(&_memHandle)
                                   .withBaggageOccurrence(1, 2)
                                   .withBaggageWeight(10, 'K')
                                   .withApplication('H')
                                   .build();
    BaggageCharge* charge = BaggageChargeBuilder(&_memHandle)
                                .withAmountAndCurrency(123, _defaultCurrency)
                                .withS5(s5)
                                .withS7(s7)
                                .build();
    BaggageTravel* baggageTravel = BaggageTravelBuilder(&_memHandle)
                                       .withOrigin("KRK")
                                       .withDestination("DFW")
                                       .withOperatingCarrier("AA")
                                       .withBaggageCharge(charge)
                                       .withTrx(_trx)
                                       .build();
    baggageTravel->_processCharges = false;

    baggageTravels += baggageTravel;

    CPPUNIT_ASSERT(_resultProcessor->formatChargesText(baggageTravels).empty());
  }

  void test_formatChargesText_singleBT_noCharges()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    // 1st baggage travel
    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("KRK")
                          .withDestination("DFW")
                          .withOperatingCarrier("AA")
                          .withTrx(_trx)
                          .build();

    std::string expected = "CARRY ON CHARGES\n"
                           "KRKDFW-AA-CARRY ON FEES UNKNOWN-CONTACT CARRIER\n";

    CPPUNIT_ASSERT_EQUAL(expected, _resultProcessor->formatChargesText(baggageTravels));
  }

  void test_formatChargesText_multipleBT_noCharges_differentCxr()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    // 1st baggage travel
    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("KRK")
                          .withDestination("DFW")
                          .withOperatingCarrier("AA")
                          .withTrx(_trx)
                          .build();

    // 2nd baggage travel
    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("DFW")
                          .withDestination("NYC")
                          .withOperatingCarrier("UA")
                          .withTrx(_trx)
                          .build();

    std::string expected = "CARRY ON CHARGES\n"
                           "KRKDFW-AA-CARRY ON FEES UNKNOWN-CONTACT CARRIER\n"
                           "DFWNYC-UA-CARRY ON FEES UNKNOWN-CONTACT CARRIER\n";

    CPPUNIT_ASSERT_EQUAL(expected, _resultProcessor->formatChargesText(baggageTravels));
  }

  void test_formatChargesText_multipleBT_noCharges_sameCxr()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    // 1st baggage travel
    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("KRK")
                          .withDestination("DFW")
                          .withOperatingCarrier("AA")
                          .withTrx(_trx)
                          .build();

    // 2nd baggage travel
    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("DFW")
                          .withDestination("NYC")
                          .withOperatingCarrier("UA")
                          .withTrx(_trx)
                          .build();

    // 3rd baggage travel
    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("NYC")
                          .withDestination("LAX")
                          .withOperatingCarrier("AA")
                          .withTrx(_trx)
                          .build();

    std::string expected = "CARRY ON CHARGES\n"
                           "KRKDFW NYCLAX-AA-CARRY ON FEES UNKNOWN-CONTACT CARRIER\n"
                           "DFWNYC-UA-CARRY ON FEES UNKNOWN-CONTACT CARRIER\n";

    CPPUNIT_ASSERT_EQUAL(expected, _resultProcessor->formatChargesText(baggageTravels));
  }

  void test_formatChargesText_singleBT_singleCharge_noS7()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    // 1st charge
    SubCodeInfo* s5 = getS5("EXAMPLE CHARGE NAME");
    BaggageCharge* charge = BaggageChargeBuilder(&_memHandle)
                                .withAmountAndCurrency(123, _defaultCurrency)
                                .withS5(s5)
                                .withS7(0)
                                .build();
    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("KRK")
                          .withDestination("DFW")
                          .withOperatingCarrier("AA")
                          .withBaggageCharge(charge)
                          .withTrx(_trx)
                          .build();

    std::string expected = "CARRY ON CHARGES\n"
                           "KRKDFW-AA\n"
                           "EXAMPLE CHARGE NAME-CARRY ON FEES UNKNOWN-CONTACT CARRIER\n";

    CPPUNIT_ASSERT_EQUAL(expected, _resultProcessor->formatChargesText(baggageTravels));
  }

  void test_formatChargesText_multipleBT_singleCharge_noS7()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    // 1st baggage travel
    SubCodeInfo* s5 = getS5("EXAMPLE CHARGE NAME");
    BaggageCharge* chargeToMerge = BaggageChargeBuilder(&_memHandle)
                                       .withAmountAndCurrency(123, _defaultCurrency)
                                       .withS5(s5)
                                       .withS7(0)
                                       .build();
    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("KRK")
                          .withDestination("DFW")
                          .withOperatingCarrier("AA")
                          .withBaggageCharge(chargeToMerge)
                          .withTrx(_trx)
                          .build();

    // 2nd baggage travel
    s5 = getS5("OTHER EXAMPLE CHARGE NAME");
    BaggageCharge* secondCharge = BaggageChargeBuilder(&_memHandle)
                                      .withAmountAndCurrency(123, _defaultCurrency)
                                      .withS5(s5)
                                      .withS7(0)
                                      .build();
    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("DFW")
                          .withDestination("NYC")
                          .withOperatingCarrier("VA")
                          .withBaggageCharge(secondCharge)
                          .withTrx(_trx)
                          .build();

    // 3rd baggage travel
    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("NYC")
                          .withDestination("LAX")
                          .withOperatingCarrier("AA")
                          .withBaggageCharge(chargeToMerge)
                          .withTrx(_trx)
                          .build();

    std::string expected = "CARRY ON CHARGES\n"
                           "KRKDFW NYCLAX-AA\n"
                           "EXAMPLE CHARGE NAME-CARRY ON FEES UNKNOWN-CONTACT CARRIER\n"
                           "DFWNYC-VA\n"
                           "OTHER EXAMPLE CHARGE NAME-CARRY ON FEES UNKNOWN-CONTACT CARRIER\n";

    CPPUNIT_ASSERT_EQUAL(expected, _resultProcessor->formatChargesText(baggageTravels));
  }

  void test_formatChargesText_mixedCase()
  {
    std::vector<const BaggageTravel*> baggageTravels;

    // 1st baggage travel (merge)
    SubCodeInfo* s5 = getS5("EXAMPLE FIRST CHARGE NAME");
    OptionalServicesInfo* s7 = S7Builder(&_memHandle)
                                   .withBaggageOccurrence(1, 2)
                                   .withBaggageWeight(10, 'K')
                                   .withApplication('H')
                                   .build();
    BaggageCharge* firstChargeToMerge = BaggageChargeBuilder(&_memHandle)
                                            .withAmountAndCurrency(123, _defaultCurrency)
                                            .withS5(s5)
                                            .withS7(s7)
                                            .build();

    s5 = getS5("EXAMPLE SECOND CHARGE NAME");
    s7 = S7Builder(&_memHandle)
             .withBaggageOccurrence(3, 3)
             .withBaggageWeight(20, 'L')
             .withApplication('H')
             .build();
    BaggageCharge* secondChargeToMerge = BaggageChargeBuilder(&_memHandle)
                                             .withAmountAndCurrency(456, _defaultCurrency)
                                             .withS5(s5)
                                             .withS7(s7)
                                             .build();

    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("KRK")
                          .withDestination("MUC")
                          .withOperatingCarrier("AA")
                          .withBaggageCharge(firstChargeToMerge)
                          .withBaggageCharge(secondChargeToMerge)
                          .withTrx(_trx)
                          .build();

    // 2nd baggage travel (do not process)
    s5 = getS5("EXAMPLE CHARGE NAME");
    s7 = S7Builder(&_memHandle)
             .withBaggageOccurrence(1, 2)
             .withBaggageWeight(10, 'K')
             .withApplication('H')
             .build();
    BaggageCharge* charge = BaggageChargeBuilder(&_memHandle)
                                .withAmountAndCurrency(123, _defaultCurrency)
                                .withS5(s5)
                                .withS7(s7)
                                .build();
    BaggageTravel* baggageTravel = BaggageTravelBuilder(&_memHandle)
                                       .withOrigin("MUC")
                                       .withDestination("DFW")
                                       .withOperatingCarrier("LH")
                                       .withBaggageCharge(charge)
                                       .withTrx(_trx)
                                       .build();
    baggageTravel->_processCharges = false;

    baggageTravels += baggageTravel;

    // 3rd baggage travel
    s5 = getS5("2ND EXAMPLE FIRST CHARGE NAME");
    s7 = S7Builder(&_memHandle)
             .withBaggageOccurrence(0, 0)
             .withBaggageWeight(10, 'K')
             .withApplication('H')
             .build();
    charge = BaggageChargeBuilder(&_memHandle)
                 .withAmountAndCurrency(123, _defaultCurrency)
                 .withS5(s5)
                 .withS7(s7)
                 .build();

    s5 = getS5("2ND EXAMPLE SECOND CHARGE NAME");
    s7 = S7Builder(&_memHandle).withBaggageOccurrence(3, 4).build();
    BaggageCharge* secondCharge = BaggageChargeBuilder(&_memHandle)
                                      .withAmountAndCurrency(456, _defaultCurrency)
                                      .withS5(s5)
                                      .withS7(s7)
                                      .build();

    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("DFW")
                          .withDestination("NYC")
                          .withOperatingCarrier("UA")
                          .withBaggageCharge(charge)
                          .withBaggageCharge(secondCharge)
                          .withTrx(_trx)
                          .build();

    // 4th baggage travel (no charges, merge)
    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("NYC")
                          .withDestination("LAX")
                          .withOperatingCarrier("AA")
                          .withTrx(_trx)
                          .build();

    // 5th baggage travel (no charges)
    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("LAX")
                          .withDestination("SEA")
                          .withOperatingCarrier("UA")
                          .withTrx(_trx)
                          .build();

    // 6th baggage travel (no s7, merge)
    s5 = getS5("EXAMPLE CHARGE NAME");
    BaggageCharge* chargeToMerge = BaggageChargeBuilder(&_memHandle)
                                       .withAmountAndCurrency(123, _defaultCurrency)
                                       .withS5(s5)
                                       .withS7(0)
                                       .build();
    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("SEA")
                          .withDestination("LHR")
                          .withOperatingCarrier("AA")
                          .withBaggageCharge(chargeToMerge)
                          .withTrx(_trx)
                          .build();

    // 7th baggage travel (no s7)
    s5 = getS5("OTHER EXAMPLE CHARGE NAME");
    charge = BaggageChargeBuilder(&_memHandle)
                 .withAmountAndCurrency(123, _defaultCurrency)
                 .withS5(s5)
                 .withS7(0)
                 .build();
    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("LHR")
                          .withDestination("WAW")
                          .withOperatingCarrier("VA")
                          .withBaggageCharge(charge)
                          .withTrx(_trx)
                          .build();

    // 8th baggage travel (no charges, merge)
    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("WAW")
                          .withDestination("HEL")
                          .withOperatingCarrier("AA")
                          .withTrx(_trx)
                          .build();

    // 9th baggage travel (no s7, merge)
    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("HEL")
                          .withDestination("CDG")
                          .withOperatingCarrier("AA")
                          .withBaggageCharge(chargeToMerge)
                          .withTrx(_trx)
                          .build();

    // 10th baggage travel (merge)
    baggageTravels += BaggageTravelBuilder(&_memHandle)
                          .withOrigin("CGD")
                          .withDestination("KRK")
                          .withOperatingCarrier("AA")
                          .withBaggageCharge(firstChargeToMerge)
                          .withBaggageCharge(secondChargeToMerge)
                          .withTrx(_trx)
                          .build();

    std::string expected = "CARRY ON CHARGES\n"
                           "KRKMUC CGDKRK-AA\n"
                           "1ST-2ND OVER 10KG EXAMPLE FIRST CHARGE NAME-USD123.00 PER KILO\n"
                           "3RD OVER 20LB EXAMPLE SECOND CHARGE NAME-USD456.00 PER KILO\n"
                           "DFWNYC-UA\n"
                           "OVER 10KG 2ND EXAMPLE FIRST CHARGE NAME-USD123.00 PER KILO\n"
                           "3RD-4TH 2ND EXAMPLE SECOND CHARGE NAME-USD456.00\n"
                           "NYCLAX WAWHEL-AA-CARRY ON FEES UNKNOWN-CONTACT CARRIER\n"
                           "LAXSEA-UA-CARRY ON FEES UNKNOWN-CONTACT CARRIER\n"
                           "SEALHR HELCDG-AA\n"
                           "EXAMPLE CHARGE NAME-CARRY ON FEES UNKNOWN-CONTACT CARRIER\n"
                           "LHRWAW-VA\n"
                           "OTHER EXAMPLE CHARGE NAME-CARRY ON FEES UNKNOWN-CONTACT CARRIER\n";

    CPPUNIT_ASSERT_EQUAL(expected, _resultProcessor->formatChargesText(baggageTravels));
  }

  void testGatherSubCodeQuantities()
  {
    SubCodeInfo s5_1;
    SubCodeInfo s5_2;
    SubCodeInfo s5_3;

    BaggageCharge s7_1;
    s7_1.subCodeInfo() = &s5_1;

    BaggageCharge s7_2;
    s7_2.subCodeInfo() = &s5_2;
    BaggageCharge s7_3;
    s7_3.subCodeInfo() = &s5_2;
    BaggageCharge s7_4;
    s7_4.subCodeInfo() = &s5_2;

    BaggageCharge s7_5;
    s7_5.subCodeInfo() = &s5_3;
    BaggageCharge s7_6;
    s7_6.subCodeInfo() = &s5_3;

    ChargeVector charges;
    charges += &s7_1;
    charges += &s7_2;
    charges += &s7_3;
    charges += &s7_4;
    charges += &s7_5;
    charges += &s7_6;

    std::map<const SubCodeInfo*, size_t> quantities;
    _resultProcessor->gatherSubCodeQuantities(charges, quantities);

    CPPUNIT_ASSERT_EQUAL((size_t)1, quantities[&s5_1]);
    CPPUNIT_ASSERT_EQUAL((size_t)3, quantities[&s5_2]);
    CPPUNIT_ASSERT_EQUAL((size_t)2, quantities[&s5_3]);
  }
};

const CurrencyCode CarryOnBaggageResultProcessorTest::_defaultCurrency = "USD";
CPPUNIT_TEST_SUITE_REGISTRATION(CarryOnBaggageResultProcessorTest);
} // tse
