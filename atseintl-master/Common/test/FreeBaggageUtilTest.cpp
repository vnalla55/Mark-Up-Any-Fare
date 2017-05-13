// ----------------------------------------------------------------
//
//   Copyright Sabre 2010
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

#include "Common/FreeBaggageUtil.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/Billing.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "FreeBagService/test/S5Builder.h"
#include "ServiceFees/OCFees.h"

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{

class FreeBaggageUtilTest : public CppUnit::TestFixture
{

  class MyDataHandle : public DataHandleMock
  {
  public:
    MyDataHandle() : _subCodes(0) {}

    const std::vector<SubCodeInfo*>& getSubCode(const VendorCode& vendor,
                                                const CarrierCode& carrier,
                                                const ServiceTypeCode& /*serviceTypeCode*/,
                                                const ServiceGroup& /*serviceGroup*/,
                                                const DateTime& /*date*/)
    {
      std::vector<SubCodeInfo*>* ret = _memHandle.create<std::vector<SubCodeInfo*> >();
      if (_subCodes)
      {
        for (SubCodeInfo* subCodeInfo : *_subCodes)
        {
          if ((subCodeInfo->vendor() == vendor) && (subCodeInfo->carrier() == carrier))
            ret->push_back(subCodeInfo);
        }
      }

      return *ret;
    }

    void setSubCode(const std::vector<SubCodeInfo*>* subCodes) { _subCodes = subCodes; }

  private:
    TestMemHandle _memHandle;
    const std::vector<SubCodeInfo*>* _subCodes;
  };

  struct EmptyS5Condition : std::unary_function<const SubCodeInfo*, bool>
  {
    bool operator()(const SubCodeInfo* s5) const { return true; }
  };

  struct CheckFltTktMerchIndCondition : std::unary_function<const SubCodeInfo*, bool>
  {
    bool operator()(const SubCodeInfo* s5) const { return BAGGAGE_CHARGE == s5->fltTktMerchInd(); }
  };

  CPPUNIT_TEST_SUITE(FreeBaggageUtilTest);

  CPPUNIT_TEST(test_getServiceSubCodes);
  CPPUNIT_TEST(test_isAlpha_AlphaString);
  CPPUNIT_TEST(test_isAlpha_NonAlphaString);

  CPPUNIT_TEST(test_s5SoftMatcher_firstConditionOk);
  CPPUNIT_TEST(test_s5SoftMatcher_firstConditionNotOk1);
  CPPUNIT_TEST(test_s5SoftMatcher_firstConditionNotOk2);
  CPPUNIT_TEST(test_s5SoftMatcher_secondConditionOk);
  CPPUNIT_TEST(test_s5SoftMatcher_secondConditionNotOk1);
  CPPUNIT_TEST(test_s5SoftMatcher_secondConditionNotOk2);

  CPPUNIT_TEST(test_s5RecordsRetriever_noCondition_theSameSubCodeTypes_from_ATP_and_MERCHMANAGER);
  CPPUNIT_TEST(test_s5RecordsRetriever_sampleS5Condition_fltTktMerchInd_Check);
  CPPUNIT_TEST(test_CarryOnAllowanceS5RecordsForTable196Strategy);

  CPPUNIT_TEST(test_ancillaryPricingRequestBaggageBasedContition);

  CPPUNIT_TEST(testCalcFirstChargedPiece_NoAllowanceMatched);
  CPPUNIT_TEST(testCalcFirstChargedPiece_WeightAllowance);
  CPPUNIT_TEST(testCalcFirstChargedPiece_Allowance1);

  CPPUNIT_TEST(testMatchOccurrence_BlankBlank);
  CPPUNIT_TEST(testMatchOccurrence_LastBlank);
  CPPUNIT_TEST(testMatchOccurrence_BothSpecified);


  CPPUNIT_TEST_SUITE_END();

private:
  MyDataHandle* _mdh;
  TestMemHandle _memHandle;
  PricingTrx* _trx;
  FreeBaggageUtil* _freeBaggageUtil;

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _mdh = _memHandle.create<MyDataHandle>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setRequest(_memHandle.create<PricingRequest>());
    _freeBaggageUtil = _memHandle.create<FreeBaggageUtil>();
  }

  void tearDown() { _memHandle.clear(); }

  void test_getServiceSubCodes()
  {
    std::multimap<ServiceSubTypeCode, int> subCodeMultimap;
    std::vector<std::string> texts;
    texts.push_back("//01/0E3");
    texts.push_back("//02/0E3");
    texts.push_back("//03/0E3");

    FreeBaggageUtil::getServiceSubCodes(texts, subCodeMultimap);

    int size = subCodeMultimap.size();
    int expectedSize = 3;

    CPPUNIT_ASSERT_EQUAL(size, expectedSize);
  }

  void test_isAlpha_AlphaString()
  {
    CPPUNIT_ASSERT_EQUAL(true, FreeBaggageUtil::isAlpha(std::string("TEST")));
  }

  void test_isAlpha_NonAlphaString()
  {
    CPPUNIT_ASSERT_EQUAL(false, FreeBaggageUtil::isAlpha(std::string("12TEST")));
    CPPUNIT_ASSERT_EQUAL(false, FreeBaggageUtil::isAlpha(std::string("TEST12")));
    CPPUNIT_ASSERT_EQUAL(false, FreeBaggageUtil::isAlpha(std::string("12")));
  }

  void test_s5SoftMatcher_firstConditionOk()
  {
    ServiceSubTypeCode matchedSubcodeType = "0F2";
    Indicator validFltTktMrchInd = 'C';

    SubCodeInfo* s5 = _memHandle.create<SubCodeInfo>();
    s5->serviceSubTypeCode() = matchedSubcodeType;
    s5->fltTktMerchInd() = validFltTktMrchInd;

    std::vector<ServiceSubTypeCode> subCodes;
    subCodes.push_back(matchedSubcodeType);

    FreeBaggageUtil::S5MatchLogic matcher(subCodes);

    CPPUNIT_ASSERT_EQUAL(true, matcher.isFirstConditionOk(s5));
  }

  void test_s5SoftMatcher_firstConditionNotOk1()
  {
    ServiceSubTypeCode s5SubcodeType = "0F2";
    Indicator notMatchingFltTktMrchInd = 'A';

    SubCodeInfo* s5 = _memHandle.create<SubCodeInfo>();
    s5->serviceSubTypeCode() = s5SubcodeType;
    s5->fltTktMerchInd() = notMatchingFltTktMrchInd;

    std::vector<ServiceSubTypeCode> subCodes;
    subCodes.push_back(s5SubcodeType);

    FreeBaggageUtil::S5MatchLogic matcher(subCodes);

    CPPUNIT_ASSERT_EQUAL(false, matcher.isFirstConditionOk(s5));
  }

  void test_s5SoftMatcher_firstConditionNotOk2()
  {
    Indicator validFltTktMrchInd = 'C';

    SubCodeInfo* s5 = _memHandle.create<SubCodeInfo>();
    s5->serviceSubTypeCode() = "0F2";
    s5->fltTktMerchInd() = validFltTktMrchInd;

    std::vector<ServiceSubTypeCode> emptySubCodes;

    FreeBaggageUtil::S5MatchLogic matcher(emptySubCodes);

    CPPUNIT_ASSERT_EQUAL(false, matcher.isFirstConditionOk(s5));
  }

  void test_s5SoftMatcher_secondConditionOk()
  {
    SubCodeInfo* s5 = _memHandle.create<SubCodeInfo>();
    s5->description1() = "10";
    s5->serviceSubGroup() = "";

    std::vector<ServiceSubTypeCode> dummySubCodes;
    FreeBaggageUtil::S5MatchLogic matcher(dummySubCodes);

    CPPUNIT_ASSERT_EQUAL(true, matcher.isSecondConditionOk(s5));
  }

  void test_s5SoftMatcher_secondConditionNotOk1()
  {
    ServiceGroupDescription nonNumericDescription = "ss";

    std::vector<ServiceSubTypeCode> dummySubCodes;
    FreeBaggageUtil::S5MatchLogic matcher(dummySubCodes);

    SubCodeInfo* s5 = _memHandle.create<SubCodeInfo>();
    s5->description1() = nonNumericDescription;
    s5->serviceSubGroup() = "";

    CPPUNIT_ASSERT_EQUAL(false, matcher.isSecondConditionOk(s5));
  }

  void test_s5SoftMatcher_secondConditionNotOk2()
  {
    ServiceGroup nonEmptySubgroup = "0F2";

    std::vector<ServiceSubTypeCode> dummySubCodes;
    FreeBaggageUtil::S5MatchLogic matcher(dummySubCodes);

    SubCodeInfo* s5 = _memHandle.create<SubCodeInfo>();
    s5->description1() = "11";
    s5->serviceSubGroup() = nonEmptySubgroup;

    CPPUNIT_ASSERT_EQUAL(false, matcher.isSecondConditionOk(s5));
  }

  void test_s5RecordsRetriever_noCondition_theSameSubCodeTypes_from_ATP_and_MERCHMANAGER()
  {
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn.push_back(createRecordS5('C', "AAA", ATPCO_VENDOR_CODE));
    subCodesIn.push_back(createRecordS5('C', "AAA", MERCH_MANAGER_VENDOR_CODE));
    _mdh->setSubCode(&subCodesIn);

    FreeBaggageUtil::S5RecordsRetriever retriever(EmptyS5Condition(), "AA", *_trx);

    CPPUNIT_ASSERT_EQUAL((size_t)1, retriever._filteredSubCodes.size());
    CPPUNIT_ASSERT_EQUAL((VendorCode)ATPCO_VENDOR_CODE,
                         (*retriever._filteredSubCodes.begin())->vendor());
  }

  void test_s5RecordsRetriever_sampleS5Condition_fltTktMerchInd_Check()
  {
    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn.push_back(createRecordS5('D', "AAA", ATPCO_VENDOR_CODE));
    subCodesIn.push_back(createRecordS5('C', "BBB", MERCH_MANAGER_VENDOR_CODE));
    _mdh->setSubCode(&subCodesIn);

    FreeBaggageUtil::S5RecordsRetriever retrieveAll(EmptyS5Condition(), "AA", *_trx);
    CPPUNIT_ASSERT_EQUAL((size_t)2, retrieveAll._filteredSubCodes.size());

    FreeBaggageUtil::S5RecordsRetriever retrieveOnlyCfltTktMerchInd(
        CheckFltTktMerchIndCondition(), "AA", *_trx);
    CPPUNIT_ASSERT_EQUAL((size_t)1, retrieveOnlyCfltTktMerchInd._filteredSubCodes.size());
    CPPUNIT_ASSERT_EQUAL(
        (Indicator)'C', (*retrieveOnlyCfltTktMerchInd._filteredSubCodes.begin())->fltTktMerchInd());
  }

  void test_CarryOnAllowanceS5RecordsForTable196Strategy()
  {
    Indicator matchingFltTktMerchInd = BAGGAGE_CHARGE;
    ServiceSubTypeCode matchingSubCodeType = "BBB";

    std::vector<SubCodeInfo*> subCodesIn;
    subCodesIn.push_back(createRecordS5('D', "AAA", ATPCO_VENDOR_CODE));
    subCodesIn.push_back(
        createRecordS5(matchingFltTktMerchInd, matchingSubCodeType, MERCH_MANAGER_VENDOR_CODE));
    _mdh->setSubCode(&subCodesIn);

    FreeBaggageUtil::CarryOnAllowanceS5RecordsForTable196Strategy s5ForCarryOnRetriever("AA",
                                                                                        *_trx);

    const SubCodeInfo* matchingS5 = s5ForCarryOnRetriever(matchingSubCodeType);

    CPPUNIT_ASSERT_EQUAL((VendorCode)MERCH_MANAGER_VENDOR_CODE, matchingS5->vendor());
    CPPUNIT_ASSERT_EQUAL((ServiceSubTypeCode)matchingSubCodeType, matchingS5->serviceSubTypeCode());
    CPPUNIT_ASSERT_EQUAL((Indicator)matchingFltTktMerchInd, matchingS5->fltTktMerchInd());
  }

  void test_ancillaryPricingRequestBaggageBasedContition()
  {
    AncillaryPricingTrx* ancTrx = _memHandle.create<AncillaryPricingTrx>();
    AncRequest* ancRequest = _memHandle.create<AncRequest>();
    ancTrx->billing() = _memHandle.create<Billing>();
    ancTrx->setRequest(ancRequest);

    ancTrx->billing()->requestPath() = ACS_PO_ATSE_PATH;
    ancTrx->billing()->actionCode() = "";
    ancRequest->ancRequestType() = AncRequest::M70Request;
    CPPUNIT_ASSERT(FreeBaggageUtil::isItBaggageDataTransaction(ancTrx));

    ancTrx->billing()->requestPath() = "";
    ancTrx->billing()->actionCode() = "MISC6";
    ancRequest->ancRequestType() = AncRequest::M70Request;
    CPPUNIT_ASSERT(FreeBaggageUtil::isItBaggageDataTransaction(ancTrx));

    ancTrx->billing()->requestPath() = "";
    ancTrx->billing()->actionCode() = "";
    ancRequest->ancRequestType() = AncRequest::WPBGRequest;
    CPPUNIT_ASSERT(FreeBaggageUtil::isItBaggageDataTransaction(ancTrx));
  }

  void testCalcFirstChargedPiece_NoAllowanceMatched()
  {
    OCFees* ocFees = _memHandle(new OCFees);
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), FreeBaggageUtil::calcFirstChargedPiece(nullptr));
    CPPUNIT_ASSERT_EQUAL(uint32_t(0), FreeBaggageUtil::calcFirstChargedPiece(ocFees));
  }

  void testCalcFirstChargedPiece_WeightAllowance()
  {
    OptionalServicesInfo* s7 = _memHandle(new OptionalServicesInfo);
    s7->freeBaggagePcs() = -1;
    s7->baggageWeight() = 10;
    s7->baggageWeightUnit() = 'K';
    OCFees* ocFees = _memHandle(new OCFees);
    ocFees->optFee() = s7;

    // Don't process charges if weight-based allowance has been matched
    CPPUNIT_ASSERT_EQUAL(MAX_BAG_PIECES, FreeBaggageUtil::calcFirstChargedPiece(ocFees));
  }

  void testCalcFirstChargedPiece_Allowance1()
  {
    OptionalServicesInfo* s7 = _memHandle(new OptionalServicesInfo);
    s7->freeBaggagePcs() = 1;
    OCFees* ocFees = _memHandle(new OCFees);
    ocFees->optFee() = s7;

    CPPUNIT_ASSERT_EQUAL(uint32_t(1), FreeBaggageUtil::calcFirstChargedPiece(ocFees));
  }

  void testMatchOccurrence_BlankBlank()
  {
    OptionalServicesInfo* s7 = _memHandle(new OptionalServicesInfo);
    s7->baggageOccurrenceFirstPc() = -1;
    s7->baggageOccurrenceLastPc() = -1;
    CPPUNIT_ASSERT(FreeBaggageUtil::matchOccurrence(*s7, 1));
    CPPUNIT_ASSERT(FreeBaggageUtil::matchOccurrence(*s7, 10));
  }

  void testMatchOccurrence_LastBlank()
  {
    OptionalServicesInfo* s7 = _memHandle(new OptionalServicesInfo);
    s7->baggageOccurrenceFirstPc() = 2;
    s7->baggageOccurrenceLastPc() = -1;
    CPPUNIT_ASSERT(!FreeBaggageUtil::matchOccurrence(*s7, 1));
    CPPUNIT_ASSERT(FreeBaggageUtil::matchOccurrence(*s7, 2));
    CPPUNIT_ASSERT(FreeBaggageUtil::matchOccurrence(*s7, 10));
  }

  void testMatchOccurrence_BothSpecified()
  {
    OptionalServicesInfo* s7 = _memHandle(new OptionalServicesInfo);
    s7->baggageOccurrenceFirstPc() = 2;
    s7->baggageOccurrenceLastPc() = 3;
    CPPUNIT_ASSERT(!FreeBaggageUtil::matchOccurrence(*s7, 1));
    CPPUNIT_ASSERT(FreeBaggageUtil::matchOccurrence(*s7, 2));
    CPPUNIT_ASSERT(FreeBaggageUtil::matchOccurrence(*s7, 3));
    CPPUNIT_ASSERT(!FreeBaggageUtil::matchOccurrence(*s7, 4));
  }

  // auxiliary methods
  SubCodeInfo* createRecordS5(const Indicator& srvType,
                              const ServiceSubTypeCode& subType,
                              const VendorCode& vendor)
  {

    return S5Builder(&_memHandle)
        .withVendCarr(vendor, "AA")
        .withFltTktMerchInd(srvType)
        .withSubGroup("")
        .withSubCode(subType)
        .withDesc("99")
        .withIndustryCarrier(' ')
        .build();
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(FreeBaggageUtilTest);
}
