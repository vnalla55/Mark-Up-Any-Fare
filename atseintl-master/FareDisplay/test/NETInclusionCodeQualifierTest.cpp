
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"
#include "FareDisplay/NETInclusionCodeQualifier.h"

#include "Common/TseStringTypes.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "Rules/RuleConst.h"

namespace tse
{
class NETInclusionCodeQualifierTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(NETInclusionCodeQualifierTest);
  CPPUNIT_TEST(testQualifyNETInclusionCode_Pass_Net);
  CPPUNIT_TEST(testQualifyNETInclusionCode_Fail_NetWithSpecified);
  CPPUNIT_TEST(testQualifyNETInclusionCode_Fail_NetNotAllowed);
  CPPUNIT_TEST(testQualifyNETInclusionCode_Pass_NotNet);
  CPPUNIT_TEST(testQualifyNETInclusionCode_Fail_NotNet);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    _memHandle.create<TestConfigInitializer>();

    _fdTrx = _memHandle.create<FareDisplayTrx>();
    _ptFare = _memHandle.create<PaxTypeFare>();
    _fca = _memHandle.create<FareClassAppInfo>();
    _request = _memHandle.create<FareDisplayRequest>();
    _ncq = _memHandle.create<NETInclusionCodeQualifier>();
    FareInfo* fareInfo = _memHandle.create<FareInfo>();
    Fare* fare = _memHandle.create<Fare>();

    _fdTrx->setRequest(_request);
    fare->setFareInfo(fareInfo);
    _ptFare->setFare(fare);
    _ptFare->fareClassAppInfo() = _fca;

    _ptFare->fareDisplayCat35Type() = RuleConst::NET_FARE;
  }

  void tearDown() { _memHandle.clear(); }

  void testQualifyNETInclusionCode_Pass_Net()
  {
    _fca->_displayCatType = RuleConst::NF_ADD;
    _request->requestedInclusionCode() = FD_NET;

    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::FD_Valid, _ncq->qualify(*_fdTrx, *_ptFare));
  }

  void testQualifyNETInclusionCode_Fail_NetWithSpecified()
  {
    _fca->_displayCatType = RuleConst::NF_SPECIFIED;
    _request->requestedInclusionCode() = FD_NET;

    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::FD_Inclusion_Code, _ncq->qualify(*_fdTrx, *_ptFare));
  }

  void testQualifyNETInclusionCode_Fail_NetNotAllowed()
  {
    _fca->_displayCatType = RuleConst::NF_ADD;
    _request->requestedInclusionCode() = FD_NET;
    makeCat35FareNotAllowed();

    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::FD_Inclusion_Code, _ncq->qualify(*_fdTrx, *_ptFare));
  }
  void testQualifyNETInclusionCode_Pass_NotNet()
  {
    _fca->_displayCatType = RuleConst::NF_SPECIFIED;
    _request->requestedInclusionCode() = FD_ADDON;

    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::FD_Inclusion_Code, _ncq->qualify(*_fdTrx, *_ptFare));
  }

  void testQualifyNETInclusionCode_Fail_NotNet()
  {
    _fca->_displayCatType = RuleConst::NF_SPECIFIED;
    _request->requestedInclusionCode() = FD_ADDON;
    _ptFare->fareDisplayCat35Type() = RuleConst::SELLING_CARRIER_FARE;

    CPPUNIT_ASSERT_EQUAL(PaxTypeFare::FD_Valid, _ncq->qualify(*_fdTrx, *_ptFare));
  }

protected:
  void makeCat35FareNotAllowed()
  {
    _ptFare->status().set(PaxTypeFare::PTF_Negotiated);
    NegPaxTypeFareRuleData* ruleData = _memHandle.create<NegPaxTypeFareRuleData>();
    PaxTypeFare::PaxTypeFareAllRuleData* allRules =
        _memHandle.create<PaxTypeFare::PaxTypeFareAllRuleData>();

    allRules->chkedRuleData = true;
    allRules->chkedGfrData = false;
    allRules->fareRuleData = ruleData;
    allRules->gfrRuleData = 0;

    (*_ptFare->paxTypeFareRuleDataMap())[RuleConst::NEGOTIATED_RULE] = allRules;

    ruleData->cat35Level() = NETInclusionCodeQualifier::NET_LEVEL_NOT_ALLOWED;
  }

protected:
  FareDisplayTrx* _fdTrx;
  PaxTypeFare* _ptFare;
  FareClassAppInfo* _fca;
  FareDisplayRequest* _request;
  NETInclusionCodeQualifier* _ncq;
  TestMemHandle _memHandle;
};
CPPUNIT_TEST_SUITE_REGISTRATION(NETInclusionCodeQualifierTest);
}
