#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/DataHandle.h"
#include "Fares/FareTypeMatcher.cpp"
#include "Rules/RuleConst.h"

#include "test/include/CppUnitHelperMacros.h"

#include <iostream>

using namespace tse;
using namespace std;

namespace tse
{

class FareTypeMatcherTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(FareTypeMatcherTest);
  CPPUNIT_TEST(matchFareType_Cat19CNNFarePassWithBaseFarePTCInList);
  CPPUNIT_TEST(matchFareType_Cat19CNNFareFailWhenNoCNNOrBaseFarePTCInList);
  CPPUNIT_TEST(matchFareType_NotCat19CNNDiscFareFailWhenNoCNNInPTCList);
  CPPUNIT_TEST(matchFareType_NotCat19CNNFareFailWhenNoCNNInPTCList);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    // TestFixture::setUp();
    _itin.geoTravelType() = GeoTravelType::Domestic;
    _trx.itin().push_back(&_itin);
    _trx.setOptions(&_options);

    DataHandle& dh = _trx.dataHandle();

    _fareTypeIT = "IT";
    FareTypeQualMsg* qualMsg;
    dh.get(qualMsg);
    qualMsg->fareTypeReqInd() = 'Y';
    dh.get(_ftq_IT_reqITX);
    _ftq_IT_reqITX->addQualMsg(_fareTypeIT, *qualMsg);
    _ftq_IT_reqITX->addPsgType("ITX");

    dh.get(_ruleData);
    _ruleData->baseFare() = &_itxPTF;
    DiscountInfo* di;
    dh.get(di);
    di->category() = RuleConst::CHILDREN_DISCOUNT_RULE;
    _ruleData->ruleItemInfo() = di;

    FareClassAppSegInfo* cnnFcAppSegInfo, *itxFcAppSegInfo, *gvtFcAppSegInfo;
    dh.get(cnnFcAppSegInfo);
    dh.get(itxFcAppSegInfo);
    dh.get(gvtFcAppSegInfo);
    cnnFcAppSegInfo->_paxType = "CNN";
    itxFcAppSegInfo->_paxType = "ITX";
    gvtFcAppSegInfo->_paxType = "GVT";
    _cnnDiscPTF.fareClassAppSegInfo() = cnnFcAppSegInfo;
    _itxPTF.fareClassAppSegInfo() = itxFcAppSegInfo;
    _gvtPTF.fareClassAppSegInfo() = gvtFcAppSegInfo;

    _cnnDiscPTF.status().set(PaxTypeFare::PTF_Discounted);
    _cnnDiscPTF.setRuleData(RuleConst::CHILDREN_DISCOUNT_RULE, _trx.dataHandle(), _ruleData);

    FarePath* fp = 0;
    _ftm = new FareTypeMatcher(_trx, fp);
  }

  void tearDown()
  {
    // TestFixture::tearDown();
    delete _ftm;
  }

  void matchFareType_Cat19CNNFarePassWithBaseFarePTCInList()
  {
    _ruleData->baseFare() = &_itxPTF;

    CPPUNIT_ASSERT_EQUAL(true, _ftm->matchFareTypeQualifier(_ftq_IT_reqITX, _fareTypeIT, &_itxPTF));
    CPPUNIT_ASSERT_EQUAL(false,
                         _ftm->matchFareTypeQualifier(_ftq_IT_reqITX, _fareTypeIT, &_gvtPTF));
    CPPUNIT_ASSERT_EQUAL(true,
                         _ftm->matchFareTypeQualifier(_ftq_IT_reqITX, _fareTypeIT, &_cnnDiscPTF));
  }

  void matchFareType_Cat19CNNFareFailWhenNoCNNOrBaseFarePTCInList()
  {
    _ruleData->baseFare() = &_gvtPTF;

    CPPUNIT_ASSERT_EQUAL(true, _ftm->matchFareTypeQualifier(_ftq_IT_reqITX, _fareTypeIT, &_itxPTF));
    CPPUNIT_ASSERT_EQUAL(false,
                         _ftm->matchFareTypeQualifier(_ftq_IT_reqITX, _fareTypeIT, &_gvtPTF));
    CPPUNIT_ASSERT_EQUAL(false,
                         _ftm->matchFareTypeQualifier(_ftq_IT_reqITX, _fareTypeIT, &_cnnDiscPTF));
  }

  void matchFareType_NotCat19CNNDiscFareFailWhenNoCNNInPTCList()
  {
    _ruleData->baseFare() = &_itxPTF;
    DiscountInfo* di = (DiscountInfo*)(_ruleData->ruleItemInfo());
    di->category() = RuleConst::OTHER_DISCOUNT_RULE;

    CPPUNIT_ASSERT_EQUAL(true, _ftm->matchFareTypeQualifier(_ftq_IT_reqITX, _fareTypeIT, &_itxPTF));
    CPPUNIT_ASSERT_EQUAL(false,
                         _ftm->matchFareTypeQualifier(_ftq_IT_reqITX, _fareTypeIT, &_gvtPTF));
    CPPUNIT_ASSERT_EQUAL(false,
                         _ftm->matchFareTypeQualifier(_ftq_IT_reqITX, _fareTypeIT, &_cnnDiscPTF));
  }

  void matchFareType_NotCat19CNNFareFailWhenNoCNNInPTCList()
  {
    _cnnDiscPTF.status().set(PaxTypeFare::PTF_Discounted, false);
    _cnnDiscPTF.setRuleData(RuleConst::CHILDREN_DISCOUNT_RULE, _trx.dataHandle(), 0);

    CPPUNIT_ASSERT_EQUAL(true, _ftm->matchFareTypeQualifier(_ftq_IT_reqITX, _fareTypeIT, &_itxPTF));
    CPPUNIT_ASSERT_EQUAL(false,
                         _ftm->matchFareTypeQualifier(_ftq_IT_reqITX, _fareTypeIT, &_gvtPTF));
    CPPUNIT_ASSERT_EQUAL(false,
                         _ftm->matchFareTypeQualifier(_ftq_IT_reqITX, _fareTypeIT, &_cnnDiscPTF));
  }

private:
  PricingTrx _trx;
  PricingOptions _options;
  Itin _itin;
  PaxTypeFare _cnnDiscPTF;
  PaxTypeFare _itxPTF;
  PaxTypeFare _gvtPTF;

  PaxTypeFareRuleData* _ruleData;
  FareType _fareTypeIT;
  FareTypeQualifier* _ftq_IT_reqITX;

  FareTypeMatcher* _ftm;
};
}

CPPUNIT_TEST_SUITE_REGISTRATION(FareTypeMatcherTest);
