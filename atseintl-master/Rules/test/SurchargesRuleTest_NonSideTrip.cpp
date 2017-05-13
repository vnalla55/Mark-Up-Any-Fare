//----------------------------------------------------------------------------
//	File: SurchargesRuleTest_NonSideTrip.cpp
//
//	Author: Svetlana Tsarkova
//  	Created:      02/01/2007
//  	Description:  This is a unit test class for SurchargesRule:: validateNonSideTrip()
//
//  Copyright Sabre 2005
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include <gtest/gtest.h>
#include "test/include/GtestHelperMacros.h"
#include "DataModel/PaxType.h"
#include "Common/TSEException.h"
#include "DBAccess/SurchargesInfo.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingRequest.h"
#include "Rules/RuleUtil.h"
#include "Diagnostic/Diag312Collector.h"
#include "Rules/RuleConst.h"
#include "Rules/SurchargesRule.h"
#include "DataModel/SurchargeData.h"
#include "test/include/TestMemHandle.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestConfigInitializer.h"

namespace tse
{

class FakeSurchargesRule : public SurchargesRule
{
public:
  FakeSurchargesRule()
  : SurchargesRule()
  , _addSurchargeCalled(false)
  , _useBaseCheckDateRange(false)
  , _passDOWandTime(false)
  {}

  Record3ReturnTypes
  validateNonSideTrip(std::vector<SurchargesRule::SurchargeSeg>& surchargeSegments,
                      PricingTrx& trx,
                      FarePath* farePath,
                      FareUsage* fareUsage,
                      const PaxTypeFare& fare,
                      const SurchargesInfo& surInfo,
                      RuleUtil::TravelSegWrapperVector& trvVector,
                      Diag312Collector* diag)
  {
    _addSurchargeCalled = false;
    return SurchargesRule::validateNonSideTrip(
        surchargeSegments, trx, farePath, fareUsage, fare, surInfo, trvVector, diag);
  }

  virtual bool checkDateRange(PricingTrx& trx,
                              const SurchargesInfo& surchInfo,
                              TravelSeg& tSeg,
                              bool origin,
                              NoPNRPricingTrx* updt = 0)
  {
    if (_useBaseCheckDateRange)
      return SurchargesRule::checkDateRange(trx, surchInfo, tSeg, origin);
    return true;
  }
  // Remove TRX parameter from this function when removing this fallback flag
  virtual bool checkDOWandTime(PricingTrx& trx,
                               const SurchargesInfo&,
                               TravelSeg& tSeg,
                               bool,
                               bool unimportant = true,
                               NoPNRPricingTrx* updt = 0)
  {
    if (_passDOWandTime)
      return true;

    if (tSeg.departureDT() == DateTime())
    {
      return true;
    }

    DateTime checkedDT = DateTime(2010, 4, 30);
    if (tSeg.departureDT() == checkedDT)
    {
      return true;
    }
    return false;
  }

  virtual SurchargeData* addSurcharge(PricingTrx&,
                                      const FarePath*,
                                      const PaxTypeFare& fare,
                                      FareUsage*,
                                      const SurchargesInfo&,
                                      SurchargeData* surchargeData,
                                      TravelSeg& tSeg,
                                      const LocCode& l1,
                                      const LocCode& l2,
                                      bool s)
  {
    surchargeData->brdAirport() = tSeg.origAirport();
    surchargeData->offAirport() = tSeg.destAirport();

    _addSurchargeCalled = true;
    return surchargeData;
  }

  void addT986Segment(TravelSeg* seg) { _matched986Segs.push_back(seg); };
  void clearT986() { _matched986Segs.clear(); };

  bool _addSurchargeCalled;
  bool _useBaseCheckDateRange;
  bool _passDOWandTime;
};

class SurchargesRuleTest_NonSideTrip : public ::testing::Test
{

public:
  void SetUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _trx = _memHandle.create<PricingTrx>();
    _fareUsage = _memHandle.create<FareUsage>();
    _paxTypeFare = _memHandle.create<PaxTypeFare>();
    _fm = _memHandle.create<FareMarket>();
    _dg = false;
    _diag = 0;
    _paxTypeFare->fareMarket() = _fm;
    _fareUsage->paxTypeFare() = _paxTypeFare;
    _request = _memHandle.create<PricingRequest>();
    _request->ticketingDT() = DateTime(2050, 1, 1);
    _trx->setRequest(_request);
    PricingOptions* pricingOptions = _memHandle.create<PricingOptions>();
    _trx->setOptions(pricingOptions);
    TestConfigInitializer::setValue("FVO_SURCHARGES_ENABLED", "N", "RULES_OPTIONS", true);
    _surInfo = _memHandle.create<SurchargesInfo>();
    _surRule = _memHandle.create<FakeSurchargesRule>();
    _farePath = _memHandle.create<FarePath>();
    _applTravelSegment = _memHandle.create<RuleUtil::TravelSegWrapperVector>();
    _surchargeSegments = _memHandle.create<std::vector<SurchargesRule::SurchargeSeg> >();
  }

  void TearDown() { _memHandle.clear(); }

  void makeOneSeg()
  {
    AirSeg* seg = _memHandle.create<AirSeg>();
    _fm->travelSeg().push_back(seg);
    RuleUtil::TravelSegWrapper* tsw1 = _memHandle.create<RuleUtil::TravelSegWrapper>();
    tsw1->travelSeg() = seg;
    _applTravelSegment->push_back(tsw1);
  }

  void makePaperTicketSurcharge()
  {
    _fareUsage->setPaperTktSurchargeMayApply();
    _surInfo->surchargeType() = RuleConst::OTHER;
  }

  void makeSimpleTravel(LocCode orig, DateTime departureDT, LocCode dest, DateTime arrivalDT,
                        bool origMatch = false, bool destMatch = false)
  {
    AirSeg* seg = _memHandle.create<AirSeg>();
    seg->departureDT() = departureDT;
    seg->arrivalDT() = arrivalDT;
    seg->origAirport() = orig;
    seg->destAirport() = dest;
    seg->bookingDT() = DateTime(2010, 3, 17, 8, 15, 0);

    _fm->travelSeg().push_back(seg);
    RuleUtil::TravelSegWrapper* tsw1 = _memHandle.create<RuleUtil::TravelSegWrapper>();
    tsw1->origMatch() = origMatch;
    tsw1->destMatch() = destMatch;
    tsw1->travelSeg() = seg;
    _applTravelSegment->push_back(tsw1);
  }

  SurchargesInfo* _surInfo;
  FakeSurchargesRule* _surRule;
  PricingTrx* _trx;
  FarePath* _farePath;
  PricingRequest* _request;
  RuleUtil::TravelSegWrapperVector* _applTravelSegment;

  FareUsage* _fareUsage;
  PaxTypeFare* _paxTypeFare;
  FareMarket* _fm;
  bool _dg;
  Diag312Collector* _diag;
  TestMemHandle _memHandle;
  std::vector<SurchargesRule::SurchargeSeg>* _surchargeSegments;
};

  TEST_F(SurchargesRuleTest_NonSideTrip, testFailWhenNoSeg)
  {
    Record3ReturnTypes ret = _surRule->validateNonSideTrip(*_surchargeSegments,
                                                          *_trx,
                                                          _farePath,
                                                          _fareUsage,
                                                          *_fareUsage->paxTypeFare(),
                                                          *_surInfo,
                                                          *_applTravelSegment,
                                                          _diag);

    EXPECT_EQ(ret, SKIP);
  }

  TEST_F(SurchargesRuleTest_NonSideTrip, testPassAndAddWhenOneSeg)
  {
    makeOneSeg();

    Record3ReturnTypes ret = _surRule->validateNonSideTrip(*_surchargeSegments,
                                                          *_trx,
                                                          _farePath,
                                                          _fareUsage,
                                                          *_fareUsage->paxTypeFare(),
                                                          *_surInfo,
                                                          *_applTravelSegment,
                                                          _diag);

    EXPECT_EQ(ret, PASS);
    EXPECT_TRUE(_surRule->_addSurchargeCalled);
  }

  TEST_F(SurchargesRuleTest_NonSideTrip, testPassAndAddWhenNoPaperSurOverride)
  {
    makeOneSeg();
    makePaperTicketSurcharge();

    Record3ReturnTypes ret = _surRule->validateNonSideTrip(*_surchargeSegments,
                                                          *_trx,
                                                          _farePath,
                                                          _fareUsage,
                                                          *_fareUsage->paxTypeFare(),
                                                          *_surInfo,
                                                          *_applTravelSegment,
                                                          _diag);
    EXPECT_EQ(ret, PASS);
    EXPECT_TRUE(_surRule->_addSurchargeCalled);
  }

  TEST_F(SurchargesRuleTest_NonSideTrip, testPassAndNoAddWhenPaperSurOverride)
  {
    makeOneSeg();
    makePaperTicketSurcharge();
    _request->ptsOverride() = 'T';

    Record3ReturnTypes ret = _surRule->validateNonSideTrip(*_surchargeSegments,
                                                          *_trx,
                                                          _farePath,
                                                          _fareUsage,
                                                          *_fareUsage->paxTypeFare(),
                                                          *_surInfo,
                                                          *_applTravelSegment,
                                                          _diag);
    EXPECT_EQ(ret, PASS);
    EXPECT_FALSE(_surRule->_addSurchargeCalled);
  }

  TEST_F(SurchargesRuleTest_NonSideTrip, testValidateNonSideTripEmpty)
  {
    makeSimpleTravel("ORD", DateTime(2010, 4, 29), "IAH", DateTime(2010, 4, 29));

    makeSimpleTravel("IAH", DateTime(2010, 4, 30), "ORD", DateTime(2010, 4, 30));

    _surInfo->tvlPortion() = '1';

    Record3ReturnTypes ret = _surRule->validateNonSideTrip(*_surchargeSegments,
                                                          *_trx,
                                                          _farePath,
                                                          _fareUsage,
                                                          *_fareUsage->paxTypeFare(),
                                                          *_surInfo,
                                                          *_applTravelSegment,
                                                          _diag);

    EXPECT_EQ(ret, SKIP);
    EXPECT_TRUE(_fareUsage->surchargeData().empty());
  }

  TEST_F(SurchargesRuleTest_NonSideTrip, testValidateNonSideTripFoundFirst)
  {
    makeSimpleTravel("ORD", DateTime(2010, 4, 30), "IAH", DateTime(2010, 4, 30));

    makeSimpleTravel("IAH", DateTime(2010, 4, 30), "ORD", DateTime(2010, 4, 30));

    _surInfo->tvlPortion() = '1';

    Record3ReturnTypes ret = _surRule->validateNonSideTrip(*_surchargeSegments,
                                                          *_trx,
                                                          _farePath,
                                                          _fareUsage,
                                                          *_fareUsage->paxTypeFare(),
                                                          *_surInfo,
                                                          *_applTravelSegment,
                                                          _diag);

    EXPECT_EQ(ret, PASS);
    EXPECT_EQ(1, (int)_fareUsage->surchargeData().size());

    SurchargeData* sd = _fareUsage->surchargeData().front();
    EXPECT_EQ(LocCode("ORD"), sd->brdAirport());
    EXPECT_EQ(LocCode("IAH"), sd->offAirport());
  }

  TEST_F(SurchargesRuleTest_NonSideTrip, testValidateNonSideTripRTW)
  {
    makeSimpleTravel("ORD", DateTime(2010, 4, 30), "IAH", DateTime(2010, 4, 30));

    makeSimpleTravel("IAH", DateTime(2010, 4, 30), "ORD", DateTime(2010, 4, 30));

    _surInfo->tvlPortion() = RuleConst::ONEWAY;

    _trx->getOptions()->setRtw(true);

    Record3ReturnTypes ret1 = _surRule->validateNonSideTrip(*_surchargeSegments,
                                                           *_trx,
                                                           _farePath,
                                                           _fareUsage,
                                                           *_fareUsage->paxTypeFare(),
                                                           *_surInfo,
                                                           *_applTravelSegment,
                                                           _diag);

    size_t size = _fareUsage->surchargeData().size();


    TearDown();
    SetUp();
    makeSimpleTravel("ORD", DateTime(2010, 4, 30), "IAH", DateTime(2010, 4, 30));

    makeSimpleTravel("IAH", DateTime(2010, 4, 30), "ORD", DateTime(2010, 4, 30));

    _surInfo->tvlPortion() = RuleConst::PERDIRECTION;

    _trx->getOptions()->setRtw(true);

    Record3ReturnTypes ret2 = _surRule->validateNonSideTrip(*_surchargeSegments,
                                                           *_trx,
                                                           _farePath,
                                                           _fareUsage,
                                                           *_fareUsage->paxTypeFare(),
                                                           *_surInfo,
                                                           *_applTravelSegment,
                                                           _diag);

    EXPECT_EQ(ret1, ret2);
    EXPECT_EQ(size, _fareUsage->surchargeData().size());
  }

  TEST_F(SurchargesRuleTest_NonSideTrip, testDateAndGeoLocFail)
  {
    _surRule->_passDOWandTime = true;
    _surRule->_useBaseCheckDateRange = true;

    _surInfo->startYear() = 2010;
    _surInfo->startMonth() = 12;
    _surInfo->startDay() = 17;
    _surInfo->stopYear() = 2010;
    _surInfo->stopMonth() = 12;
    _surInfo->stopDay() = 17;
    _surInfo->geoTblItemNo() = 1;//FromToVia ATL

    //ATL matched as dest
    makeSimpleTravel("BWI", DateTime(2010, 12, 18), "ATL", DateTime(2010, 12, 18), false, true);
    //ATL matched as orig
    makeSimpleTravel("ATL", DateTime(2010, 12, 19), "CUN", DateTime(2010, 12, 19), true, false);

    Record3ReturnTypes ret = _surRule->validateNonSideTrip(*_surchargeSegments,
                                                          *_trx,
                                                          _farePath,
                                                          _fareUsage,
                                                          *_fareUsage->paxTypeFare(),
                                                          *_surInfo,
                                                          *_applTravelSegment,
                                                          _diag);

    EXPECT_EQ(ret, SKIP);

    _surInfo->startDay() = 19;
    _surInfo->stopDay() = 19;

    ret = _surRule->validateNonSideTrip(*_surchargeSegments,
                                        *_trx,
                                        _farePath,
                                        _fareUsage,
                                        *_fareUsage->paxTypeFare(),
                                        *_surInfo,
                                        *_applTravelSegment,
                                        _diag);

    EXPECT_EQ(ret, PASS);
  }

  TEST_F(SurchargesRuleTest_NonSideTrip, testDOWAndT986)
  {
    //Test validation of DOW will pass when date departure is DateTime(2010, 4, 30)
    //ATL matched as dest
    makeSimpleTravel("BWI", DateTime(2010, 4, 29), "ATL", DateTime(2010, 4, 29));
    //ATL matched as orig
    makeSimpleTravel("ATL", DateTime(2010, 4, 30), "CUN", DateTime(2010, 4, 30));

    _surRule->addT986Segment(_fm->travelSeg().front());

    Record3ReturnTypes ret = _surRule->validateNonSideTrip(*_surchargeSegments,
                                                           *_trx,
                                                           _farePath,
                                                           _fareUsage,
                                                           *_fareUsage->paxTypeFare(),
                                                           *_surInfo,
                                                           *_applTravelSegment,
                                                           _diag);
    //T986 match to second DOW to first segment
    EXPECT_EQ(ret, SKIP);

    _surRule->clearT986();
    _surRule->addT986Segment(_fm->travelSeg().back());

    ret = _surRule->validateNonSideTrip(*_surchargeSegments,
                                        *_trx,
                                        _farePath,
                                        _fareUsage,
                                        *_fareUsage->paxTypeFare(),
                                        *_surInfo,
                                        *_applTravelSegment,
                                        _diag);

    //T986 and DOW match to same second segment
    EXPECT_EQ(ret, PASS);
  }
}
