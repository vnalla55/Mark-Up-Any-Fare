#include "test/include/CppUnitHelperMacros.h"

#include "DataModel/Agent.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DBAccess/NUCInfo.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/PricingOptions.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/ServicesDescription.h"
#include "DBAccess/ServicesSubGroup.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/SvcFeesCxrResultingFCLInfo.h"
#include "DBAccess/SvcFeesResBkgDesigInfo.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/OCFeesUsage.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "Xform/AncillaryBaggageResponseFormatter.h"

namespace tse
{
class OCFeesUsageBuilder
{
  TestMemHandle& _memHandle;
  const PricingTrx& _trx;
  OCFeesUsage* _ocFeesUsage;
  OCFees* _ocFees;

public:
  OCFeesUsageBuilder(TestMemHandle& memHandle, const PricingTrx& trx)
    : _memHandle(memHandle), _trx(trx)
  {
    _ocFeesUsage = _memHandle.create<OCFeesUsage>();
    _ocFees = _memHandle.create<OCFees>();
    _ocFeesUsage->oCFees() = _ocFees;
  }

  OCFeesUsageBuilder& setServiceGroup(const ServiceGroup& serviceGroup)
  {
    SubCodeInfo* subCodeInfo = _memHandle.create<SubCodeInfo>();
    subCodeInfo->serviceGroup() = serviceGroup;
    _ocFees->subCodeInfo() = subCodeInfo;

    return *this;
  }

  OCFeesUsageBuilder& setFrequentFlyerMileageAppl(const Indicator& frequentFlyerMileageAppl)
  {
    OptionalServicesInfo* s7 = _memHandle.create<OptionalServicesInfo>();
    s7->frequentFlyerMileageAppl() = frequentFlyerMileageAppl;
    _ocFees->optFee() = s7;
    return *this;
  }

  OCFeesUsageBuilder&
  setFeeAmount(const MoneyAmount& feeAmount, const CurrencyCode& feeCurrency = "USD")
  {
    _ocFees->feeAmount() = feeAmount;
    _ocFees->feeCurrency() = feeCurrency;
    return *this;
  }

  OCFeesUsage* build() { return _ocFeesUsage; }
};

class AncillaryBaggageResponseFormatterTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(AncillaryBaggageResponseFormatterTest);

  CPPUNIT_TEST(test_constructXMLResponse);

  CPPUNIT_TEST(test_buildChargeLine_S7NoCharge_Blank_KG);
  CPPUNIT_TEST(test_buildChargeLine_S7NoCharge_Blank_LB);
  CPPUNIT_TEST(test_buildChargeLine_S7NoCharge_X);

  CPPUNIT_TEST(test_formatSubCodeDescription_NoDescription1);
  CPPUNIT_TEST(test_formatSubCodeDescription_Description1Alphabetic);
  CPPUNIT_TEST(test_formatSubCodeDescription_Description1Alphanumeric);
  CPPUNIT_TEST(test_formatSubCodeDescription_Description1Alphabetic_Description2Alphabetic);
  CPPUNIT_TEST(test_formatSubCodeDescription_Description1Alphanumeric_Description2Alphabetic);
  CPPUNIT_TEST(test_formatSubCodeDescription_Description1Alphabetic_Description2Alphanumeric);
  CPPUNIT_TEST(test_formatSubCodeDescription_Description1Alphanumeric_Description2Alphanumeric);
  CPPUNIT_TEST(test_formatSubCodeDescription_MaxLineLengthExceeded);

  CPPUNIT_TEST(test_formatPaxType_PaxCode_Provided);
  CPPUNIT_TEST(test_formatPaxType_TktNumber_Provided);

  CPPUNIT_TEST(test_getFootnote_FeeAppl_C);
  CPPUNIT_TEST(test_getFootnote_FeeAppl_P);
  CPPUNIT_TEST(test_getFootnote_FeeAppl_H);
  CPPUNIT_TEST(test_getFootnote_FeeAppl_K);
  CPPUNIT_TEST(test_getFootnote_FeeAppl_F);
  CPPUNIT_TEST(test_getFootnote_FeeAppl_3_UsDot);
  CPPUNIT_TEST(test_getFootnote_FeeAppl_3_NonUsDot);
  CPPUNIT_TEST(test_getFootnote_FFStatus_0);
  CPPUNIT_TEST(test_getFootnote_FFStatus_1);
  CPPUNIT_TEST(test_getFootnote_FFStatus_10);
  CPPUNIT_TEST(test_getFootnote_RefundInd_N);
  CPPUNIT_TEST(test_getFootnote_RefundInd_R);
  CPPUNIT_TEST(test_getFootnote_FeeAppl_C_FFStatus_1);
  CPPUNIT_TEST(test_getFootnote_FeeAppl_F_FFStatus_9_RefundInd_R);

  CPPUNIT_TEST(test_validateRequestedBTIndices_InRange);
  CPPUNIT_TEST(test_validateRequestedBTIndices_NotInRange);

  CPPUNIT_TEST(test_split_StringShorterThatMaxLen);
  CPPUNIT_TEST(test_split_StringLongerThatMaxLen);
  CPPUNIT_TEST(test_split_StringWithSpacesLongerThatMaxLen);

  CPPUNIT_TEST(test_getMoneyAmount_SA_schema_1_0_0_C);
  CPPUNIT_TEST(test_getMoneyAmount_BG_schema_1_0_0_C);
  CPPUNIT_TEST(test_getMoneyAmount_PT_schema_1_0_0_C);
  CPPUNIT_TEST(test_getMoneyAmount_SA_schema_2_0_0_C);
  CPPUNIT_TEST(test_getMoneyAmount_BG_schema_2_0_0_C);
  CPPUNIT_TEST(test_getMoneyAmount_PT_schema_2_0_0_C);
  CPPUNIT_TEST(test_getMoneyAmount_SA_schema_1_0_0_K);
  CPPUNIT_TEST(test_getMoneyAmount_BG_schema_1_0_0_K);
  CPPUNIT_TEST(test_getMoneyAmount_PT_schema_1_0_0_K);
  CPPUNIT_TEST(test_getMoneyAmount_SA_schema_2_0_0_K);
  CPPUNIT_TEST(test_getMoneyAmount_BG_schema_2_0_0_K);
  CPPUNIT_TEST(test_getMoneyAmount_PT_schema_2_0_0_K);

  CPPUNIT_TEST_SUITE_END();

private:
  class AncillaryBaggageResponseFormatterDataHandleMock : public DataHandleMock
  {
    TestMemHandle _memH;

  public:
    AncillaryBaggageResponseFormatterDataHandleMock() {}

    ~AncillaryBaggageResponseFormatterDataHandleMock() { _memH.clear(); }

    const ServicesDescription* getServicesDescription(const ServiceGroupDescription& value)
    {
      ServicesDescription* ret = _memH.create<ServicesDescription>();

      if (value == "1")
      {
        ret->description() = "ALPHABETIC";
        return ret;
      }
      else if (value == "2")
      {
        ret->description() = "1ALPHANUMERIC1";
        return ret;
      }
      else if (value == "")
        return 0;

      return DataHandleMock::getServicesDescription(value);
    }

    NUCInfo*
    getNUCFirst(const CurrencyCode& currency, const CarrierCode& carrier, const DateTime& date)
    {
      NUCInfo* nucInfo = _memH.create<NUCInfo>();
      nucInfo->_roundingFactor = 10;
      nucInfo->_nucFactorNodec = 8;
      nucInfo->_roundingRule = UP;
      return nucInfo;
    }
  };

private:
  AncillaryBaggageResponseFormatterDataHandleMock* _dataHandleMock;
  AncillaryBaggageResponseFormatter* _ancFormatter;
  AncillaryPricingTrx* _ancTrx;
  AncRequest* _ancRequest;
  PricingOptions* _pOptions;
  Agent* _agent;
  Itin* _itin;
  TestMemHandle _memH;

public:
  void setUp()
  {
    _memH.create<TestConfigInitializer>();

    _dataHandleMock = _memH.create<AncillaryBaggageResponseFormatterDataHandleMock>();
    _ancTrx = _memH.create<AncillaryPricingTrx>();
    _ancFormatter = _memH.create<AncillaryBaggageResponseFormatter>(*_ancTrx);
    _ancRequest = _memH.create<AncRequest>();
    _pOptions = _memH.create<PricingOptions>();
    _itin = _memH.create<Itin>();
    _agent = _memH.create<Agent>();

    _agent->currencyCodeAgent() = "USD";
    _ancTrx->setRequest(_ancRequest);
    _ancTrx->getRequest()->ticketingAgent() = _agent;
    _ancTrx->setOptions(_pOptions);
    _ancTrx->itin().push_back(_itin);
  }

  void tearDown() { _memH.clear(); }

  void setSchemaVersion(uint16_t major, uint16_t minor, uint16_t revision)
  {
    _ancRequest->majorSchemaVersion() = major;
    _ancRequest->minorSchemaVersion() = minor;
    _ancRequest->revisionSchemaVersion() = revision;
  }

  void setSellingCurrency(const CurrencyCode& currencyCode)
  {
    _ancTrx->getOptions()->currencyOverride() = currencyCode;
  }

  void test_constructXMLResponse()
  {
    XMLConstruct construct;

    _ancFormatter->constructXMLResponse(construct, "TEST\nTEST\nTESTTESTTEST\nTEST");

    CPPUNIT_ASSERT_EQUAL(
        std::string("<MSG N06=\"X\" Q0K=\"1\" S18=\"TEST\"/><MSG N06=\"X\" "
                    "Q0K=\"2\" S18=\"TEST\"/><MSG N06=\"X\" Q0K=\"3\" S18=\"TESTTESTTEST\"/>"
                    "<MSG N06=\"X\" Q0K=\"4\" S18=\"TEST\"/>"),
        construct.getXMLData());
  }

  void test_buildChargeLine_S7NoCharge_Blank_KG()
  {
    std::ostringstream output;
    OCFees ocFees;
    OCFeesUsage ocFeeUsage;
    OptionalServicesInfo s7;
    std::vector<std::string> desciptionLines;

    s7.baggageOccurrenceFirstPc() = 1;
    s7.baggageOccurrenceLastPc() = 2;
    s7.baggageWeight() = 10;
    s7.baggageWeightUnit() = 'K';

    ocFeeUsage.oCFees() = &ocFees;
    ocFees.ocfeeUsage().push_back(&ocFeeUsage);
    ocFees.optFee() = &s7;

    desciptionLines.push_back("DESCRIPTION");

    _ancFormatter->buildChargeLine(output, 1, &ocFeeUsage, "LO", desciptionLines, true);

    CPPUNIT_ASSERT_EQUAL(
        std::string("1   LO  DESCRIPTION            01  02   10KG   USD  0.00   \n"), output.str());
  }

  void test_buildChargeLine_S7NoCharge_Blank_LB()
  {
    std::ostringstream output;
    OCFees ocFees;
    OCFeesUsage ocFeeUsage;
    OptionalServicesInfo s7;
    std::vector<std::string> desciptionLines;

    s7.baggageOccurrenceFirstPc() = 1;
    s7.baggageOccurrenceLastPc() = 2;
    s7.baggageWeight() = 10;
    s7.baggageWeightUnit() = 'P';

    ocFeeUsage.oCFees() = &ocFees;
    ocFees.ocfeeUsage().push_back(&ocFeeUsage);
    ocFees.optFee() = &s7;

    desciptionLines.push_back("DESCRIPTION");

    _ancFormatter->buildChargeLine(output, 1, &ocFeeUsage, "LO", desciptionLines, true);

    CPPUNIT_ASSERT_EQUAL(
        std::string("1   LO  DESCRIPTION            01  02   10LB   USD  0.00   \n"), output.str());
  }

  void test_buildChargeLine_S7NoCharge_X()
  {
    std::ostringstream output;
    OCFees ocFees;
    OCFeesUsage ocFeeUsage;
    OptionalServicesInfo s7;
    std::vector<std::string> desciptionLines;

    s7.notAvailNoChargeInd() = 'X';

    ocFeeUsage.oCFees() = &ocFees;
    ocFees.ocfeeUsage().push_back(&ocFeeUsage);
    ocFees.optFee() = &s7;

    desciptionLines.push_back("DESCRIPTION");

    _ancFormatter->buildChargeLine(output, 1, &ocFeeUsage, "LO", desciptionLines, true);

    CPPUNIT_ASSERT_EQUAL(
        std::string("1   LO  DESCRIPTION            N/A N/A  N/A    USD  NOTAVAIL\n"),
        output.str());
  }

  void test_formatSubCodeDescription_Empty()
  {
    std::vector<std::string> desciptionLines;

    CPPUNIT_ASSERT_EQUAL(false, _ancFormatter->formatSubCodeDescription(0, 10, desciptionLines));
  }

  void test_formatSubCodeDescription_NoDescription1()
  {
    SubCodeInfo subCodeInfo;
    std::vector<std::string> desciptionLines;

    subCodeInfo.description1() = "";
    subCodeInfo.commercialName() = "COMMERCIAL NAME";

    CPPUNIT_ASSERT_EQUAL(
        true, _ancFormatter->formatSubCodeDescription(&subCodeInfo, 100, desciptionLines));
    CPPUNIT_ASSERT_EQUAL((size_t)1, desciptionLines.size());
    CPPUNIT_ASSERT_EQUAL(std::string("COMMERCIAL NAME"), desciptionLines.front());
  }

  void test_formatSubCodeDescription_Description1Alphabetic()
  {
    SubCodeInfo subCodeInfo;
    std::vector<std::string> desciptionLines;

    subCodeInfo.description1() = "1";

    CPPUNIT_ASSERT_EQUAL(
        true, _ancFormatter->formatSubCodeDescription(&subCodeInfo, 100, desciptionLines));
    CPPUNIT_ASSERT_EQUAL((size_t)1, desciptionLines.size());
    CPPUNIT_ASSERT_EQUAL(std::string("ALPHABETIC"), desciptionLines.front());
  }

  void test_formatSubCodeDescription_Description1Alphanumeric()
  {
    SubCodeInfo subCodeInfo;
    std::vector<std::string> desciptionLines;

    subCodeInfo.description1() = "2";

    CPPUNIT_ASSERT_EQUAL(
        true, _ancFormatter->formatSubCodeDescription(&subCodeInfo, 100, desciptionLines));
    CPPUNIT_ASSERT_EQUAL((size_t)1, desciptionLines.size());
    CPPUNIT_ASSERT_EQUAL(std::string("1ALPHANUMERIC1"), desciptionLines.front());
  }

  void test_formatSubCodeDescription_Description1Alphabetic_Description2Alphabetic()
  {
    SubCodeInfo subCodeInfo;
    std::vector<std::string> desciptionLines;

    subCodeInfo.description1() = "1";
    subCodeInfo.description2() = "1";

    CPPUNIT_ASSERT_EQUAL(
        true, _ancFormatter->formatSubCodeDescription(&subCodeInfo, 100, desciptionLines));
    CPPUNIT_ASSERT_EQUAL((size_t)1, desciptionLines.size());
    CPPUNIT_ASSERT_EQUAL(std::string("ALPHABETIC ALPHABETIC"), desciptionLines.front());
  }

  void test_formatSubCodeDescription_Description1Alphanumeric_Description2Alphabetic()
  {
    SubCodeInfo subCodeInfo;
    std::vector<std::string> desciptionLines;

    subCodeInfo.description1() = "2";
    subCodeInfo.description2() = "1";

    CPPUNIT_ASSERT_EQUAL(
        true, _ancFormatter->formatSubCodeDescription(&subCodeInfo, 100, desciptionLines));
    CPPUNIT_ASSERT_EQUAL((size_t)1, desciptionLines.size());
    CPPUNIT_ASSERT_EQUAL(std::string("1ALPHANUMERIC1 ALPHABETIC"), desciptionLines.front());
  }

  void test_formatSubCodeDescription_Description1Alphabetic_Description2Alphanumeric()
  {
    SubCodeInfo subCodeInfo;
    std::vector<std::string> desciptionLines;

    subCodeInfo.description1() = "1";
    subCodeInfo.description2() = "2";

    CPPUNIT_ASSERT_EQUAL(
        true, _ancFormatter->formatSubCodeDescription(&subCodeInfo, 100, desciptionLines));
    CPPUNIT_ASSERT_EQUAL((size_t)1, desciptionLines.size());
    CPPUNIT_ASSERT_EQUAL(std::string("ALPHABETIC 1ALPHANUMERIC1"), desciptionLines.front());
  }

  void test_formatSubCodeDescription_Description1Alphanumeric_Description2Alphanumeric()
  {
    SubCodeInfo subCodeInfo;
    std::vector<std::string> desciptionLines;

    subCodeInfo.description1() = "2";
    subCodeInfo.description2() = "2";

    CPPUNIT_ASSERT_EQUAL(
        true, _ancFormatter->formatSubCodeDescription(&subCodeInfo, 100, desciptionLines));
    CPPUNIT_ASSERT_EQUAL((size_t)1, desciptionLines.size());
    CPPUNIT_ASSERT_EQUAL(std::string("1ALPHANUMERIC1 AND 1ALPHANUMERIC1"), desciptionLines.front());
  }

  void test_formatSubCodeDescription_MaxLineLengthExceeded()
  {
    SubCodeInfo subCodeInfo;
    std::vector<std::string> descriptionLines;

    subCodeInfo.description1() = "2";
    subCodeInfo.description2() = "2";

    CPPUNIT_ASSERT_EQUAL(
        true, _ancFormatter->formatSubCodeDescription(&subCodeInfo, 10, descriptionLines));
    CPPUNIT_ASSERT_EQUAL((size_t)4, descriptionLines.size());
    CPPUNIT_ASSERT_EQUAL(std::string("1ALPHANUME"), descriptionLines[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("RIC1 AND 1"), descriptionLines[1]);
    CPPUNIT_ASSERT_EQUAL(std::string("ALPHANUMER"), descriptionLines[2]);
    CPPUNIT_ASSERT_EQUAL(std::string("IC1"), descriptionLines[3]);
  }

  void test_formatPaxType_PaxCode_Provided()
  {
    PaxType paxType;

    paxType.paxType() = "ADT";

    std::string expected("ADT");

    CPPUNIT_ASSERT_EQUAL(expected, _ancFormatter->formatPaxType(&paxType, false));
    CPPUNIT_ASSERT_EQUAL(expected, _ancFormatter->formatPaxType(&paxType, true));
  }

  void test_formatPaxType_TktNumber_Provided()
  {
    PaxType paxType;
    PaxType::TktInfo tktInfo;

    tktInfo.tktRefNumber() = "5";
    paxType.psgTktInfo().push_back(&tktInfo);

    CPPUNIT_ASSERT_EQUAL(std::string("TR5"), _ancFormatter->formatPaxType(&paxType, true));
  }

  void test_getFootnote_FeeAppl_C()
  {
    OptionalServicesInfo s7;
    s7.frequentFlyerMileageAppl() = 'C';

    std::stringstream expected;
    expected << AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_KILO;

    CPPUNIT_ASSERT_EQUAL(expected.str(), _ancFormatter->formatFootnotes(s7, false));
    CPPUNIT_ASSERT_EQUAL(
        true,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_KILO]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_5KILOS]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_AT_EACH_LOC]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::SERVICE_NONREFUNDABLE]);
    CPPUNIT_ASSERT_EQUAL(
        false, _ancFormatter->_showFootnoteLegend[AncillaryBaggageResponseFormatter::FREQ_FLYER]);
  }

  void test_getFootnote_FeeAppl_P()
  {
    OptionalServicesInfo s7;
    s7.frequentFlyerMileageAppl() = 'P';

    std::stringstream expected;
    expected << AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_KILO;

    CPPUNIT_ASSERT_EQUAL(expected.str(), _ancFormatter->formatFootnotes(s7, false));
    CPPUNIT_ASSERT_EQUAL(
        true,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_KILO]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_5KILOS]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_AT_EACH_LOC]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::SERVICE_NONREFUNDABLE]);
    CPPUNIT_ASSERT_EQUAL(
        false, _ancFormatter->_showFootnoteLegend[AncillaryBaggageResponseFormatter::FREQ_FLYER]);
  }

  void test_getFootnote_FeeAppl_H()
  {
    OptionalServicesInfo s7;
    s7.frequentFlyerMileageAppl() = 'H';

    std::stringstream expected;
    expected << AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_KILO;

    CPPUNIT_ASSERT_EQUAL(expected.str(), _ancFormatter->formatFootnotes(s7, false));
    CPPUNIT_ASSERT_EQUAL(
        true,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_KILO]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_5KILOS]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_AT_EACH_LOC]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::SERVICE_NONREFUNDABLE]);
    CPPUNIT_ASSERT_EQUAL(
        false, _ancFormatter->_showFootnoteLegend[AncillaryBaggageResponseFormatter::FREQ_FLYER]);
  }

  void test_getFootnote_FeeAppl_K()
  {
    OptionalServicesInfo s7;
    s7.frequentFlyerMileageAppl() = 'K';

    std::stringstream expected;
    expected << AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_KILO;

    CPPUNIT_ASSERT_EQUAL(expected.str(), _ancFormatter->formatFootnotes(s7, false));
    CPPUNIT_ASSERT_EQUAL(
        true,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_KILO]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_5KILOS]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_AT_EACH_LOC]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::SERVICE_NONREFUNDABLE]);
    CPPUNIT_ASSERT_EQUAL(
        false, _ancFormatter->_showFootnoteLegend[AncillaryBaggageResponseFormatter::FREQ_FLYER]);
  }

  void test_getFootnote_FeeAppl_F()
  {
    OptionalServicesInfo s7;
    s7.frequentFlyerMileageAppl() = 'F';

    std::stringstream expected;
    expected << AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_5KILOS;

    CPPUNIT_ASSERT_EQUAL(expected.str(), _ancFormatter->formatFootnotes(s7, false));
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_KILO]);
    CPPUNIT_ASSERT_EQUAL(
        true,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_5KILOS]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_AT_EACH_LOC]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::SERVICE_NONREFUNDABLE]);
    CPPUNIT_ASSERT_EQUAL(
        false, _ancFormatter->_showFootnoteLegend[AncillaryBaggageResponseFormatter::FREQ_FLYER]);
  }

  void test_getFootnote_FeeAppl_3_UsDot()
  {
    OptionalServicesInfo s7;
    s7.frequentFlyerMileageAppl() = '3';

    std::stringstream expected;
    expected << AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_AT_EACH_LOC;

    CPPUNIT_ASSERT_EQUAL(expected.str(), _ancFormatter->formatFootnotes(s7, true));
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_KILO]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_5KILOS]);
    CPPUNIT_ASSERT_EQUAL(
        true,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_AT_EACH_LOC]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::SERVICE_NONREFUNDABLE]);
    CPPUNIT_ASSERT_EQUAL(
        false, _ancFormatter->_showFootnoteLegend[AncillaryBaggageResponseFormatter::FREQ_FLYER]);
  }

  void test_getFootnote_FeeAppl_3_NonUsDot()
  {
    OptionalServicesInfo s7;
    s7.frequentFlyerMileageAppl() = '3';

    CPPUNIT_ASSERT_EQUAL(std::string(""), _ancFormatter->formatFootnotes(s7, false));
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_KILO]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_5KILOS]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_AT_EACH_LOC]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::SERVICE_NONREFUNDABLE]);
    CPPUNIT_ASSERT_EQUAL(
        false, _ancFormatter->_showFootnoteLegend[AncillaryBaggageResponseFormatter::FREQ_FLYER]);
  }

  void test_getFootnote_FFStatus_0()
  {
    OptionalServicesInfo s7;
    s7.frequentFlyerStatus() = 0;

    CPPUNIT_ASSERT_EQUAL(std::string(""), _ancFormatter->formatFootnotes(s7, false));
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_KILO]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_5KILOS]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_AT_EACH_LOC]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::SERVICE_NONREFUNDABLE]);
    CPPUNIT_ASSERT_EQUAL(
        false, _ancFormatter->_showFootnoteLegend[AncillaryBaggageResponseFormatter::FREQ_FLYER]);
  }

  void test_getFootnote_FFStatus_1()
  {
    OptionalServicesInfo s7;
    s7.frequentFlyerStatus() = 1;

    CPPUNIT_ASSERT_EQUAL(std::string("1"), _ancFormatter->formatFootnotes(s7, false));
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_KILO]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_5KILOS]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_AT_EACH_LOC]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::SERVICE_NONREFUNDABLE]);
    CPPUNIT_ASSERT_EQUAL(
        true, _ancFormatter->_showFootnoteLegend[AncillaryBaggageResponseFormatter::FREQ_FLYER]);
  }

  void test_getFootnote_FFStatus_10()
  {
    OptionalServicesInfo s7;
    s7.frequentFlyerStatus() = 0;

    CPPUNIT_ASSERT_EQUAL(std::string(""), _ancFormatter->formatFootnotes(s7, false));
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_KILO]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_5KILOS]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_AT_EACH_LOC]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::SERVICE_NONREFUNDABLE]);
    CPPUNIT_ASSERT_EQUAL(
        false, _ancFormatter->_showFootnoteLegend[AncillaryBaggageResponseFormatter::FREQ_FLYER]);
  }

  void test_getFootnote_RefundInd_N()
  {
    OptionalServicesInfo s7;
    s7.refundReissueInd() = 'N';

    std::stringstream expected;
    expected << AncillaryBaggageResponseFormatter::SERVICE_NONREFUNDABLE;

    CPPUNIT_ASSERT_EQUAL(expected.str(), _ancFormatter->formatFootnotes(s7, false));
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_KILO]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_5KILOS]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_AT_EACH_LOC]);
    CPPUNIT_ASSERT_EQUAL(
        true,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::SERVICE_NONREFUNDABLE]);
    CPPUNIT_ASSERT_EQUAL(
        false, _ancFormatter->_showFootnoteLegend[AncillaryBaggageResponseFormatter::FREQ_FLYER]);
  }

  void test_getFootnote_RefundInd_R()
  {
    OptionalServicesInfo s7;
    s7.refundReissueInd() = 'R';

    std::stringstream expected;
    expected << AncillaryBaggageResponseFormatter::SERVICE_NONREFUNDABLE;

    CPPUNIT_ASSERT_EQUAL(expected.str(), _ancFormatter->formatFootnotes(s7, false));
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_KILO]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_5KILOS]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_AT_EACH_LOC]);
    CPPUNIT_ASSERT_EQUAL(
        true,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::SERVICE_NONREFUNDABLE]);
    CPPUNIT_ASSERT_EQUAL(
        false, _ancFormatter->_showFootnoteLegend[AncillaryBaggageResponseFormatter::FREQ_FLYER]);
  }

  void test_getFootnote_FeeAppl_C_FFStatus_1()
  {
    OptionalServicesInfo s7;
    s7.frequentFlyerMileageAppl() = 'C';
    s7.frequentFlyerStatus() = 1;

    std::stringstream expected;
    expected << AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_KILO << "1";

    CPPUNIT_ASSERT_EQUAL(expected.str(), _ancFormatter->formatFootnotes(s7, false));
    CPPUNIT_ASSERT_EQUAL(
        true,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_KILO]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_5KILOS]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_AT_EACH_LOC]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::SERVICE_NONREFUNDABLE]);
    CPPUNIT_ASSERT_EQUAL(
        true, _ancFormatter->_showFootnoteLegend[AncillaryBaggageResponseFormatter::FREQ_FLYER]);
  }

  void test_getFootnote_FeeAppl_F_FFStatus_9_RefundInd_R()
  {
    OptionalServicesInfo s7;
    s7.frequentFlyerMileageAppl() = 'F';
    s7.frequentFlyerStatus() = 9;
    s7.refundReissueInd() = 'R';

    std::stringstream expected;
    expected << AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_5KILOS << "9"
             << AncillaryBaggageResponseFormatter::SERVICE_NONREFUNDABLE;

    CPPUNIT_ASSERT_EQUAL(expected.str(), _ancFormatter->formatFootnotes(s7, false));
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_KILO]);
    CPPUNIT_ASSERT_EQUAL(
        true,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_5KILOS]);
    CPPUNIT_ASSERT_EQUAL(
        false,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_AT_EACH_LOC]);
    CPPUNIT_ASSERT_EQUAL(
        true,
        _ancFormatter
            ->_showFootnoteLegend[AncillaryBaggageResponseFormatter::SERVICE_NONREFUNDABLE]);
    CPPUNIT_ASSERT_EQUAL(
        true, _ancFormatter->_showFootnoteLegend[AncillaryBaggageResponseFormatter::FREQ_FLYER]);
  }

  void test_validateRequestedBTIndices_InRange()
  {
    std::set<uint32_t> reqBTindices;
    BaggageTravel bt1;
    BaggageTravel bt2;

    _ancTrx->baggageTravels()[_itin].push_back(&bt1);
    _ancTrx->baggageTravels()[_itin].push_back(&bt2);

    reqBTindices.insert(1);
    reqBTindices.insert(2);

    CPPUNIT_ASSERT_EQUAL(true, _ancFormatter->validateRequestedBTIndices(reqBTindices));
  }

  void test_validateRequestedBTIndices_NotInRange()
  {
    std::set<uint32_t> reqBTindices;
    BaggageTravel bt1;
    BaggageTravel bt2;

    _ancTrx->baggageTravels()[_itin].push_back(&bt1);
    _ancTrx->baggageTravels()[_itin].push_back(&bt2);

    reqBTindices.insert(0);
    CPPUNIT_ASSERT_EQUAL(false, _ancFormatter->validateRequestedBTIndices(reqBTindices));

    reqBTindices.clear();
    reqBTindices.insert(3);
    CPPUNIT_ASSERT_EQUAL(false, _ancFormatter->validateRequestedBTIndices(reqBTindices));
  }

  void test_split_StringShorterThatMaxLen()
  {
    std::string str("TEST ");
    std::vector<std::string> output;

    _ancFormatter->split(str, 10, output);

    CPPUNIT_ASSERT_EQUAL((size_t)1, output.size());
    CPPUNIT_ASSERT_EQUAL(std::string("TEST "), output[0]);
  }

  void test_split_StringLongerThatMaxLen()
  {
    std::string str("TEST1TEST2TEST3");
    std::vector<std::string> output;

    _ancFormatter->split(str, 10, output);

    CPPUNIT_ASSERT_EQUAL((size_t)2, output.size());
    CPPUNIT_ASSERT_EQUAL(std::string("TEST1TEST2"), output[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("TEST3"), output[1]);
  }

  void test_split_StringWithSpacesLongerThatMaxLen()
  {
    std::string str("TEST1TEST2   TEST3");
    std::vector<std::string> output;

    _ancFormatter->split(str, 10, output);

    CPPUNIT_ASSERT_EQUAL((size_t)2, output.size());
    CPPUNIT_ASSERT_EQUAL(std::string("TEST1TEST2"), output[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("TEST3"), output[1]);
  }

  void test_getMoneyAmount_SA_schema_1_0_0_C()
  {
    setSchemaVersion(1, 0, 0);
    setSellingCurrency("USD");
    OCFeesUsage* usage = OCFeesUsageBuilder(_memH, *_ancTrx)
                             .setServiceGroup("SA")
                             .setFeeAmount(35.73)
                             .setFrequentFlyerMileageAppl('C')
                             .build();

    CPPUNIT_ASSERT_EQUAL(35.73, _ancFormatter->getMoneyAmount(usage));
  }

  void test_getMoneyAmount_BG_schema_1_0_0_C()
  {
    setSchemaVersion(1, 0, 0);
    setSellingCurrency("USD");
    OCFeesUsage* usage = OCFeesUsageBuilder(_memH, *_ancTrx)
                             .setServiceGroup("BG")
                             .setFeeAmount(35.73)
                             .setFrequentFlyerMileageAppl('C')
                             .build();

    CPPUNIT_ASSERT_EQUAL(35.73, _ancFormatter->getMoneyAmount(usage));
  }

  void test_getMoneyAmount_PT_schema_1_0_0_C()
  {
    setSchemaVersion(1, 0, 0);
    setSellingCurrency("USD");
    OCFeesUsage* usage = OCFeesUsageBuilder(_memH, *_ancTrx)
                             .setServiceGroup("PT")
                             .setFeeAmount(35.73)
                             .setFrequentFlyerMileageAppl('C')
                             .build();

    CPPUNIT_ASSERT_EQUAL(35.73, _ancFormatter->getMoneyAmount(usage));
  }

  void test_getMoneyAmount_SA_schema_2_0_0_C()
  {
    setSchemaVersion(2, 0, 0);
    setSellingCurrency("USD");
    OCFeesUsage* usage = OCFeesUsageBuilder(_memH, *_ancTrx)
                             .setServiceGroup("SA")
                             .setFeeAmount(35.73)
                             .setFrequentFlyerMileageAppl('C')
                             .build();

    CPPUNIT_ASSERT_EQUAL(35.73, _ancFormatter->getMoneyAmount(usage));
  }

  void test_getMoneyAmount_BG_schema_2_0_0_C()
  {
    setSchemaVersion(2, 0, 0);
    setSellingCurrency("USD");
    OCFeesUsage* usage = OCFeesUsageBuilder(_memH, *_ancTrx)
                             .setServiceGroup("BG")
                             .setFeeAmount(35.73)
                             .setFrequentFlyerMileageAppl('C')
                             .build();

    CPPUNIT_ASSERT_EQUAL(40.0, _ancFormatter->getMoneyAmount(usage));
  }

  void test_getMoneyAmount_PT_schema_2_0_0_C()
  {
    setSchemaVersion(2, 0, 0);
    setSellingCurrency("USD");
    OCFeesUsage* usage = OCFeesUsageBuilder(_memH, *_ancTrx)
                             .setServiceGroup("PT")
                             .setFeeAmount(35.73)
                             .setFrequentFlyerMileageAppl('C')
                             .build();

    CPPUNIT_ASSERT_EQUAL(40.0, _ancFormatter->getMoneyAmount(usage));
  }

  void test_getMoneyAmount_SA_schema_1_0_0_K()
  {
    setSchemaVersion(1, 0, 0);
    setSellingCurrency("USD");
    OCFeesUsage* usage = OCFeesUsageBuilder(_memH, *_ancTrx)
                             .setServiceGroup("SA")
                             .setFeeAmount(35.73)
                             .setFrequentFlyerMileageAppl('K')
                             .build();

    CPPUNIT_ASSERT_EQUAL(35.73, _ancFormatter->getMoneyAmount(usage));
  }

  void test_getMoneyAmount_BG_schema_1_0_0_K()
  {
    setSchemaVersion(1, 0, 0);
    setSellingCurrency("USD");
    OCFeesUsage* usage = OCFeesUsageBuilder(_memH, *_ancTrx)
                             .setServiceGroup("BG")
                             .setFeeAmount(35.73)
                             .setFrequentFlyerMileageAppl('K')
                             .build();

    CPPUNIT_ASSERT_EQUAL(35.73, _ancFormatter->getMoneyAmount(usage));
  }

  void test_getMoneyAmount_PT_schema_1_0_0_K()
  {
    setSchemaVersion(1, 0, 0);
    setSellingCurrency("USD");
    OCFeesUsage* usage = OCFeesUsageBuilder(_memH, *_ancTrx)
                             .setServiceGroup("PT")
                             .setFeeAmount(35.73)
                             .setFrequentFlyerMileageAppl('K')
                             .build();

    CPPUNIT_ASSERT_EQUAL(35.73, _ancFormatter->getMoneyAmount(usage));
  }

  void test_getMoneyAmount_SA_schema_2_0_0_K()
  {
    setSchemaVersion(2, 0, 0);
    setSellingCurrency("USD");
    OCFeesUsage* usage = OCFeesUsageBuilder(_memH, *_ancTrx)
                             .setServiceGroup("SA")
                             .setFeeAmount(35.73)
                             .setFrequentFlyerMileageAppl('K')
                             .build();

    CPPUNIT_ASSERT_EQUAL(35.73, _ancFormatter->getMoneyAmount(usage));
  }

  void test_getMoneyAmount_BG_schema_2_0_0_K()
  {
    setSchemaVersion(2, 0, 0);
    setSellingCurrency("USD");
    OCFeesUsage* usage = OCFeesUsageBuilder(_memH, *_ancTrx)
                             .setServiceGroup("BG")
                             .setFeeAmount(35.73)
                             .setFrequentFlyerMileageAppl('K')
                             .build();

    CPPUNIT_ASSERT_EQUAL(35.73, _ancFormatter->getMoneyAmount(usage));
  }

  void test_getMoneyAmount_PT_schema_2_0_0_K()
  {
    setSchemaVersion(2, 0, 0);
    setSellingCurrency("USD");
    OCFeesUsage* usage = OCFeesUsageBuilder(_memH, *_ancTrx)
                             .setServiceGroup("PT")
                             .setFeeAmount(35.73)
                             .setFrequentFlyerMileageAppl('K')
                             .build();

    CPPUNIT_ASSERT_EQUAL(35.73, _ancFormatter->getMoneyAmount(usage));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(AncillaryBaggageResponseFormatterTest);

} // tse
