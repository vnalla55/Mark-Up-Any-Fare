#include "test/include/CppUnitHelperMacros.h"
#include "Rules/TourCodeUtil.h"
#include "test/include/TestMemHandle.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "Rules/RuleConst.h"
#include "Common/TrxUtil.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/NegFareRestExt.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DBAccess/Customer.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{
using namespace std;

class TourCodeUtilTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(TourCodeUtilTest);

  CPPUNIT_TEST(testValidate_Pass_OneFuWith1st);
  CPPUNIT_TEST(testValidate_Pass_OneFuWith2nd);
  CPPUNIT_TEST(testValidate_Fail_1stOn2ndFare);
  CPPUNIT_TEST(testValidate_Fail_2ndOn3rdFare);
  CPPUNIT_TEST(testValidate_Fail_Vendor);
  CPPUNIT_TEST(testValidate_Pass_Match);
  CPPUNIT_TEST(testValidate_Fail_NoCat35);

  CPPUNIT_TEST(testMatchTourCodeCombination_Pass_Equal);
  CPPUNIT_TEST(testMatchTourCodeCombination_Pass_1st);
  CPPUNIT_TEST(testMatchTourCodeCombination_Pass_2nd);
  CPPUNIT_TEST(testMatchTourCodeCombination_Pass_All);
  CPPUNIT_TEST(testMatchTourCodeCombination_Fail_All_WithDifferentType);
  CPPUNIT_TEST(testMatchTourCodeCombination_Pass_BlankEqual);
  CPPUNIT_TEST(testMatchTourCodeCombination_Fail_BlankDifferent);
  CPPUNIT_TEST(testMatchTourCodeCombination_Fail_2ndWith1st);
  CPPUNIT_TEST(testMatchTourCodeCombination_Fail_1stWith2nd);
  CPPUNIT_TEST(testMatchTourCodeCombination_Fail_BlankWith1st);
  CPPUNIT_TEST(testMatchTourCodeCombination_Fail_AllWith1st);
  CPPUNIT_TEST(testMatchTourCodeCombination_Fail_EmptyTcWith1st);
  CPPUNIT_TEST(testMatchTourCodeCombination_Pass_EmptyTcWith2nd);
  CPPUNIT_TEST(testMatchTourCodeCombination_Fail_EmptyTcWithAll);
  CPPUNIT_TEST(testMatchTourCodeCombination_Pass_EmptyTcWithBlank);
  CPPUNIT_TEST(testMatchTourCodeCombination_Pass_1stWithEmptyTc);
  CPPUNIT_TEST(testMatchTourCodeCombination_Fail_2ndWithEmptyTc);
  CPPUNIT_TEST(testMatchTourCodeCombination_Fail_AllWithEmptyTc);
  CPPUNIT_TEST(testMatchTourCodeCombination_Pass_BlankWithEmptyTc);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _trx = _memHandle.create<PricingTrx>();
    _memHandle.create<MyDataHandle>();
    _tcUtil = _memHandle.insert(new TourCodeUtil(_paxTypeFares));
    enableOptimusNetRemit();
  }

  void tearDown()
  {
    _paxTypeFares.clear();
    _memHandle.clear();
  }

  void testValidate_Pass_OneFuWith1st()
  {
    addPtfToCollection("TC", RuleConst::DISPLAY_OPTION_1ST);
    CPPUNIT_ASSERT(_tcUtil->validate(*_trx));
  }

  void testValidate_Pass_OneFuWith2nd()
  {
    addPtfToCollection("TC", RuleConst::DISPLAY_OPTION_2ND);
    CPPUNIT_ASSERT(_tcUtil->validate(*_trx));
  }

  void testValidate_Fail_1stOn2ndFare()
  {
    addPtfToCollection("", RuleConst::DISPLAY_OPTION_BLANK);
    addPtfToCollection("TC", RuleConst::DISPLAY_OPTION_1ST);

    CPPUNIT_ASSERT(!_tcUtil->validate(*_trx));
  }

  void testValidate_Fail_2ndOn3rdFare()
  {
    addPtfToCollection("", RuleConst::DISPLAY_OPTION_BLANK);
    addPtfToCollection("", RuleConst::DISPLAY_OPTION_BLANK);
    addPtfToCollection("TC", RuleConst::DISPLAY_OPTION_2ND);

    CPPUNIT_ASSERT(!_tcUtil->validate(*_trx));
  }

  void testValidate_Fail_Vendor()
  {
    addPtfToCollection("TC", RuleConst::DISPLAY_OPTION_1ST);
    addPtfToCollection("TC", RuleConst::DISPLAY_OPTION_1ST, ' ', "ATP");

    CPPUNIT_ASSERT(!_tcUtil->validate(*_trx));
  }

  void testValidate_Pass_Match()
  {
    addPtfToCollection("TC", RuleConst::DISPLAY_OPTION_1ST);
    addPtfToCollection("TC", RuleConst::DISPLAY_OPTION_1ST);

    CPPUNIT_ASSERT(_tcUtil->validate(*_trx));
  }

  void testValidate_Fail_NoCat35()
  {
    addPtfToCollection("", RuleConst::DISPLAY_OPTION_BLANK);
    addPtfToCollection("", RuleConst::DISPLAY_OPTION_BLANK);

    CPPUNIT_ASSERT(!_tcUtil->validate(*_trx));
  }

  void testMatchTourCodeCombination_Pass_Equal()
  {
    CPPUNIT_ASSERT(
        _tcUtil->matchTourCodeCombination(createTcTuple("TC", RuleConst::DISPLAY_OPTION_1ST),
                                          createTcTuple("TC", RuleConst::DISPLAY_OPTION_1ST)));
  }

  void testMatchTourCodeCombination_Pass_1st()
  {
    CPPUNIT_ASSERT(
        _tcUtil->matchTourCodeCombination(createTcTuple("TC1", RuleConst::DISPLAY_OPTION_1ST),
                                          createTcTuple("TC2", RuleConst::DISPLAY_OPTION_1ST)));
  }

  void testMatchTourCodeCombination_Pass_2nd()
  {
    CPPUNIT_ASSERT(
        _tcUtil->matchTourCodeCombination(createTcTuple("TC1", RuleConst::DISPLAY_OPTION_2ND),
                                          createTcTuple("TC2", RuleConst::DISPLAY_OPTION_2ND)));
  }

  void testMatchTourCodeCombination_Pass_All()
  {
    CPPUNIT_ASSERT(
        _tcUtil->matchTourCodeCombination(createTcTuple("TC1", RuleConst::DISPLAY_OPTION_ALL),
                                          createTcTuple("TC2", RuleConst::DISPLAY_OPTION_ALL)));
  }

  void testMatchTourCodeCombination_Fail_All_WithDifferentType()
  {
    CPPUNIT_ASSERT(!_tcUtil->matchTourCodeCombination(
        createTcTuple("TC1", RuleConst::DISPLAY_OPTION_ALL, RuleConst::PRINT_OPTION_1),
        createTcTuple("TC2", RuleConst::DISPLAY_OPTION_ALL, RuleConst::PRINT_OPTION_2)));
  }

  void testMatchTourCodeCombination_Pass_BlankEqual()
  {
    CPPUNIT_ASSERT(
        _tcUtil->matchTourCodeCombination(createTcTuple("TC1", RuleConst::DISPLAY_OPTION_BLANK),
                                          createTcTuple("TC1", RuleConst::DISPLAY_OPTION_BLANK)));
  }

  void testMatchTourCodeCombination_Fail_BlankDifferent()
  {
    CPPUNIT_ASSERT(
        !_tcUtil->matchTourCodeCombination(createTcTuple("TC1", RuleConst::DISPLAY_OPTION_BLANK),
                                           createTcTuple("TC2", RuleConst::DISPLAY_OPTION_BLANK)));
  }

  void testMatchTourCodeCombination_Fail_2ndWith1st()
  {
    CPPUNIT_ASSERT(
        !_tcUtil->matchTourCodeCombination(createTcTuple("TC1", RuleConst::DISPLAY_OPTION_2ND),
                                           createTcTuple("TC2", RuleConst::DISPLAY_OPTION_1ST)));
  }

  void testMatchTourCodeCombination_Fail_1stWith2nd()
  {
    CPPUNIT_ASSERT(
        !_tcUtil->matchTourCodeCombination(createTcTuple("TC1", RuleConst::DISPLAY_OPTION_1ST),
                                           createTcTuple("TC2", RuleConst::DISPLAY_OPTION_2ND)));
  }

  void testMatchTourCodeCombination_Fail_BlankWith1st()
  {
    CPPUNIT_ASSERT(
        !_tcUtil->matchTourCodeCombination(createTcTuple("TC1", RuleConst::DISPLAY_OPTION_BLANK),
                                           createTcTuple("TC2", RuleConst::DISPLAY_OPTION_1ST)));
  }

  void testMatchTourCodeCombination_Fail_AllWith1st()
  {
    CPPUNIT_ASSERT(
        !_tcUtil->matchTourCodeCombination(createTcTuple("TC1", RuleConst::DISPLAY_OPTION_ALL),
                                           createTcTuple("TC2", RuleConst::DISPLAY_OPTION_1ST)));
  }

  void testMatchTourCodeCombination_Fail_EmptyTcWith1st()
  {
    CPPUNIT_ASSERT(
        !_tcUtil->matchTourCodeCombination(createTcTuple("", RuleConst::DISPLAY_OPTION_BLANK),
                                           createTcTuple("TC2", RuleConst::DISPLAY_OPTION_1ST)));
  }

  void testMatchTourCodeCombination_Pass_EmptyTcWith2nd()
  {
    CPPUNIT_ASSERT(
        _tcUtil->matchTourCodeCombination(createTcTuple("", RuleConst::DISPLAY_OPTION_BLANK),
                                          createTcTuple("TC2", RuleConst::DISPLAY_OPTION_2ND)));
  }

  void testMatchTourCodeCombination_Fail_EmptyTcWithAll()
  {
    CPPUNIT_ASSERT(
        !_tcUtil->matchTourCodeCombination(createTcTuple("", RuleConst::DISPLAY_OPTION_BLANK),
                                           createTcTuple("TC2", RuleConst::DISPLAY_OPTION_ALL)));
  }

  void testMatchTourCodeCombination_Pass_EmptyTcWithBlank()
  {
    CPPUNIT_ASSERT(
        _tcUtil->matchTourCodeCombination(createTcTuple("", RuleConst::DISPLAY_OPTION_BLANK),
                                          createTcTuple("TC2", RuleConst::DISPLAY_OPTION_BLANK)));
  }

  void testMatchTourCodeCombination_Pass_1stWithEmptyTc()
  {
    CPPUNIT_ASSERT(
        _tcUtil->matchTourCodeCombination(createTcTuple("TC1", RuleConst::DISPLAY_OPTION_1ST),
                                          createTcTuple("", RuleConst::DISPLAY_OPTION_BLANK)));
  }

  void testMatchTourCodeCombination_Fail_2ndWithEmptyTc()
  {
    CPPUNIT_ASSERT(
        !_tcUtil->matchTourCodeCombination(createTcTuple("TC1", RuleConst::DISPLAY_OPTION_2ND),
                                           createTcTuple("", RuleConst::DISPLAY_OPTION_BLANK)));
  }

  void testMatchTourCodeCombination_Fail_AllWithEmptyTc()
  {
    CPPUNIT_ASSERT(
        !_tcUtil->matchTourCodeCombination(createTcTuple("TC1", RuleConst::DISPLAY_OPTION_ALL),
                                           createTcTuple("", RuleConst::DISPLAY_OPTION_BLANK)));
  }

  void testMatchTourCodeCombination_Pass_BlankWithEmptyTc()
  {
    CPPUNIT_ASSERT(
        _tcUtil->matchTourCodeCombination(createTcTuple("TC1", RuleConst::DISPLAY_OPTION_BLANK),
                                          createTcTuple("", RuleConst::DISPLAY_OPTION_BLANK)));
  }

protected:
  void enableOptimusNetRemit()
  {
    _memHandle.create<TestConfigInitializer>();
    TestConfigInitializer::setValue("ACTIVATE_OPTIMUS_NET_REMIT", "Y", "PRICING_SVC", true);
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _trx->getRequest()->ticketingAgent() = _memHandle.create<Agent>();
    _trx->getRequest()->ticketingAgent()->agentTJR() = _memHandle.create<Customer>();
    TrxUtil::enableAbacus();
    _trx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() = "1B";
    _trx->getRequest()->ticketingAgent()->agentTJR()->hostName() = "ABAC";
  }

  void addPtfToCollection(const std::string& tourCode,
                          Indicator tcCombInd,
                          Indicator tcType = RuleConst::PRINT_OPTION_1,
                          const VendorCode& vendor = "SMFC",
                          const CarrierCode& cxr = "AA")
  {
    MockPaxTypeFare* ptf = _memHandle.insert(new MockPaxTypeFare(vendor, cxr));
    ptf->negFareRest().tourBoxCode1() = tourCode;
    ptf->negFareRestExt().tourCodeCombInd() = tcCombInd;
    ptf->negFareRest().tourBoxCodeType1() = tcType;
    _paxTypeFares.push_back(ptf);
  }

  TourCodeUtil::TourCodeTuple&
  createTcTuple(const string& tc, Indicator tcInd, Indicator tcType = RuleConst::PRINT_OPTION_1)
  {
    return *_memHandle.insert(
        new TourCodeUtil::TourCodeTuple(*_memHandle.insert(new std::string(tc)),
                                        *_memHandle.insert(new Indicator(tcInd)),
                                        *_memHandle.insert(new Indicator(tcType))));
  };

  class MockPaxTypeFare : public PaxTypeFare
  {
  public:
    MockPaxTypeFare(const VendorCode& vendor, const CarrierCode& carrier)
    {
      _fareInfo.vendor() = vendor;
      _fareInfo.carrier() = carrier;
      _fare.initialize(Fare::FS_ForeignDomestic, &_fareInfo, _fareMarket, &_tariffCrossRefInfo);
      setFare(&_fare);
      status().set(PaxTypeFare::PTF_Negotiated);
      _fareClassAppInfo._displayCatType = RuleConst::NET_SUBMIT_FARE;
      fareClassAppInfo() = &_fareClassAppInfo;
      _negFareRest.netRemitMethod() = RuleConst::NRR_METHOD_2;
      _negFareRest.negFareCalcTblItemNo() = 1;
      _negRuleData.ruleItemInfo() = &_negFareRest;
      _negRuleData.negFareRestExt() = &_negFareRestExt;
      _negAllRuleData.fareRuleData = &_negRuleData;
      (*(paxTypeFareRuleDataMap()))[RuleConst::NEGOTIATED_RULE] = &_negAllRuleData;
    }

    NegPaxTypeFareRuleData& negRuleData() { return _negRuleData; }
    const NegPaxTypeFareRuleData& negRuleData() const { return _negRuleData; }

    NegFareRestExt& negFareRestExt() { return _negFareRestExt; }
    const NegFareRestExt& negFareRestExt() const { return _negFareRestExt; }

    NegFareRest& negFareRest() { return _negFareRest; }
    const NegFareRest& negFareRest() const { return _negFareRest; }

  protected:
    FareClassAppInfo _fareClassAppInfo;
    FareInfo _fareInfo;
    Fare _fare;
    TariffCrossRefInfo _tariffCrossRefInfo;
    FareMarket _fareMarket;
    NegFareRest _negFareRest;
    NegFareRestExt _negFareRestExt;
    PaxTypeFare::PaxTypeFareAllRuleData _negAllRuleData;
    NegPaxTypeFareRuleData _negRuleData;
  };

  class MyDataHandle : public DataHandleMock
  {
  public:
    char getVendorType(const VendorCode& vendor)
    {
      if (vendor == "SMFC")
        return 'T';
      return DataHandleMock::getVendorType(vendor);
    }
  };

private:
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  TourCodeUtil* _tcUtil;
  std::vector<const PaxTypeFare*> _paxTypeFares;
};

CPPUNIT_TEST_SUITE_REGISTRATION(TourCodeUtilTest);
};
