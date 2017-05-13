
//----------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-----------------------------------------------------------------

#include "DataModel/AirSeg.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/BookingCodeExceptionSegment.h"
#include "BookingCode/BookingCodeExceptionValidator.h"

#include <gtest/gtest.h>
#include "test/include/GtestHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class BookingCodeExceptionValidatorRtwTest : public ::testing::Test
{
public:
  void SetUp()
  {
    _memHandle.create<TestConfigInitializer>();
    _pro = _memHandle.create<PricingOptions>();
    _req = _memHandle.create<PricingRequest>();
    _trx = _memHandle.create<PricingTrx>();
    _trx->setOptions(_pro);
    _trx->setRequest(_req);
    _fm = _memHandle.create<FareMarket>();
    _ptf = _memHandle.create<PaxTypeFare>();
    _ptf->fareMarket() = _fm;
    _fu = _memHandle.create<FareUsage>();
    _validator = _memHandle(new BookingCodeExceptionValidator(_itin, false, false, false));
    _validator->_fu = _fu;
    _economyCabin.setEconomyClass();
    _businessCabin.setBusinessClass();
  }

protected:
  typedef BookingCodeExceptionValidator::RtwPreferredCabinContext RtwPreferredCabinContext;

  void setupInitRtwPreferredCabin()
  {
    _pro->setRtw(true);
    _req->lowFareRequested() = 'T';
    _trx->altTrxType() = PricingTrx::WP;
    _ptf->cabin().setEconomyClass();
    _fm->travelSeg().resize(2u);
    _validator->_rtwPreferredCabin.assign(10u, _businessCabin);
  }

  void setupUpdateRtwPreferredCabin()
  {
    _fm->travelSeg().assign(1u, _memHandle.create<AirSeg>());
    _fm->travelSeg().front()->bookedCabin().setEconomyClass();
    _fu->segmentStatus().resize(1u);
    _ptf->cabin().setEconomyClass();
    PaxTypeFare::SegmentStatus& stat = _fu->segmentStatus().front();
    stat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_PASS);
    stat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED);
    stat._reBookCabin.setPremiumEconomyClass();
    _validator->_rtwPreferredCabin.assign(1u, CabinType());
    _validator->_statusType = BookingCodeExceptionValidator::STATUS_RULE1;
  }

  void setupPrepareSequenceValidationRtw()
  {
    PaxTypeFare::SegmentStatus segStat;
    segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_PASS);
    segStat._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED);
    segStat._bkgCodeReBook = "D";
    segStat._reBookCabin.setBusinessClass();
    _fu->segmentStatus().resize(2u, segStat);
    _validator->_fltResultVector.resize(2u, TravelSegResult(1,1));
    _validator->_rtwPreferredCabin.assign(2u, CabinType());
  }

  TravelSegmentResultVector& getTsResultVector() { return _validator->_fltResultVector; }
  std::vector<CabinType>& getRtwPreferredCabin() { return _validator->_rtwPreferredCabin; }
  void callInitRtwPreferredCabin() { _validator->initRtwPreferredCabin(*_trx, *_ptf); }
  void callUpdateRtwPreferredCabin(const BookingCodeExceptionSegment& bceSeg, uint32_t fltI)
  {
    _validator->updateRtwPreferredCabin(bceSeg, *_ptf, fltI);
  }
  void callPrepareSequenceValidationRtw(RtwPreferredCabinContext& cxt)
  {
    _validator->prepareSequenceValidationRtw(cxt, *_ptf);
  }

  TestMemHandle _memHandle;
  PricingTrx* _trx;
  PricingRequest* _req;
  PricingOptions* _pro;

  FareMarket* _fm;
  PaxTypeFare* _ptf;
  FareUsage* _fu;
  Itin _itin;
  BookingCodeExceptionValidator* _validator;
  CabinType _economyCabin;
  CabinType _businessCabin;
};

static bool operator==(const PaxTypeFare::SegmentStatus& s1, const PaxTypeFare::SegmentStatus& s2)
{
  return s1._bkgCodeReBook == s2._bkgCodeReBook &&
         s1._bkgCodeSegStatus.value() == s2._bkgCodeSegStatus.value() &&
         s1._reBookCabin == s2._reBookCabin;
}

TEST_F(BookingCodeExceptionValidatorRtwTest, testInitRtwPreferredCabin_Pass)
{
  setupInitRtwPreferredCabin();
  callInitRtwPreferredCabin();
  ASSERT_EQ(size_t(2), getRtwPreferredCabin().size());
  EXPECT_EQ(CabinType(), getRtwPreferredCabin()[0]);
  EXPECT_EQ(CabinType(), getRtwPreferredCabin()[1]);
}

TEST_F(BookingCodeExceptionValidatorRtwTest, testInitRtwPreferredCabin_PassBusiness)
{
  setupInitRtwPreferredCabin();
  _ptf->cabin().setBusinessClass();
  callInitRtwPreferredCabin();
  ASSERT_EQ(size_t(2), getRtwPreferredCabin().size());
  EXPECT_EQ(CabinType(), getRtwPreferredCabin()[0]);
  EXPECT_EQ(CabinType(), getRtwPreferredCabin()[1]);
}

TEST_F(BookingCodeExceptionValidatorRtwTest, testInitRtwPreferredCabin_PassFirstClass)
{
  setupInitRtwPreferredCabin();
  _ptf->cabin().setFirstClass();
  callInitRtwPreferredCabin();
  ASSERT_EQ(size_t(2), getRtwPreferredCabin().size());
  EXPECT_EQ(CabinType(), getRtwPreferredCabin()[0]);
  EXPECT_EQ(CabinType(), getRtwPreferredCabin()[1]);
}

TEST_F(BookingCodeExceptionValidatorRtwTest, testInitRtwPreferredCabin_NoRtw)
{
  setupInitRtwPreferredCabin();
  _pro->setRtw(false);
  callInitRtwPreferredCabin();
  ASSERT_TRUE(getRtwPreferredCabin().empty());
}

TEST_F(BookingCodeExceptionValidatorRtwTest, testInitRtwPreferredCabin_NoLowFareReq)
{
  setupInitRtwPreferredCabin();
  _req->lowFareRequested() = 'N';
  callInitRtwPreferredCabin();
  ASSERT_TRUE(getRtwPreferredCabin().empty());
}

TEST_F(BookingCodeExceptionValidatorRtwTest, testInitRtwPreferredCabin_NoWp)
{
  setupInitRtwPreferredCabin();
  _trx->altTrxType() = PricingTrx::WPA;
  callInitRtwPreferredCabin();
  ASSERT_TRUE(getRtwPreferredCabin().empty());
}

TEST_F(BookingCodeExceptionValidatorRtwTest, testInitRtwPreferredCabin_PremiumEconomyFare)
{
  setupInitRtwPreferredCabin();
  _ptf->cabin().setPremiumEconomyClass();
  callInitRtwPreferredCabin();
  ASSERT_TRUE(getRtwPreferredCabin().empty());
}

TEST_F(BookingCodeExceptionValidatorRtwTest, testUpdateRtwPreferredCabin_True)
{
  setupUpdateRtwPreferredCabin();
  BookingCodeExceptionSegment bces;
  bces.restrictionTag() = BookingCodeExceptionValidator::BCE_PERMITTED;
  callUpdateRtwPreferredCabin(bces, 0);
  ASSERT_EQ(_economyCabin, getRtwPreferredCabin().front());
}

TEST_F(BookingCodeExceptionValidatorRtwTest, testUpdateRtwPreferredCabin_TrueBusinessFare)
{
  setupUpdateRtwPreferredCabin();
  _ptf->cabin().setBusinessClass();
  _fm->travelSeg().front()->bookedCabin().setBusinessClass();
  _fu->segmentStatus().front()._reBookCabin.setPremiumBusinessClass();
  BookingCodeExceptionSegment bces;
  bces.restrictionTag() = BookingCodeExceptionValidator::BCE_PERMITTED;
  callUpdateRtwPreferredCabin(bces, 0);
  ASSERT_EQ(_businessCabin, getRtwPreferredCabin().front());
}


TEST_F(BookingCodeExceptionValidatorRtwTest, testUpdateRtwPreferredCabin_Required)
{
  setupUpdateRtwPreferredCabin();
  BookingCodeExceptionSegment bces;
  bces.restrictionTag() = BookingCodeExceptionValidator::BCE_REQUIRED;
  callUpdateRtwPreferredCabin(bces, 0);
  ASSERT_EQ(CabinType(), getRtwPreferredCabin().front());
}

TEST_F(BookingCodeExceptionValidatorRtwTest, testUpdateRtwPreferredCabin_BookedBusiness)
{
  setupUpdateRtwPreferredCabin();
  _fm->travelSeg().front()->bookedCabin().setBusinessClass();
  BookingCodeExceptionSegment bces;
  bces.restrictionTag() = BookingCodeExceptionValidator::BCE_PERMITTED;
  callUpdateRtwPreferredCabin(bces, 0);
  ASSERT_EQ(CabinType(), getRtwPreferredCabin().front());
}

TEST_F(BookingCodeExceptionValidatorRtwTest, testUpdateRtwPreferredCabin_RebookEconomy)
{
  setupUpdateRtwPreferredCabin();
  _fu->segmentStatus().front()._reBookCabin.setEconomyClass();
  BookingCodeExceptionSegment bces;
  bces.restrictionTag() = BookingCodeExceptionValidator::BCE_PERMITTED;
  callUpdateRtwPreferredCabin(bces, 0);
  ASSERT_EQ(CabinType(), getRtwPreferredCabin().front());
}

TEST_F(BookingCodeExceptionValidatorRtwTest, testUpdateRtwPreferredCabin_NoRebook)
{
  setupUpdateRtwPreferredCabin();
  _fu->segmentStatus().front()._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED, false);
  BookingCodeExceptionSegment bces;
  bces.restrictionTag() = BookingCodeExceptionValidator::BCE_PERMITTED;
  callUpdateRtwPreferredCabin(bces, 0);
  ASSERT_EQ(CabinType(), getRtwPreferredCabin().front());
}

TEST_F(BookingCodeExceptionValidatorRtwTest, testPrepareSequenceValidationRtw_NonApplicable)
{
  setupPrepareSequenceValidationRtw();
  getRtwPreferredCabin().clear();
  PaxTypeFare::SegmentStatusVec oldStatus = _fu->segmentStatus();
  RtwPreferredCabinContext cxt;
  callPrepareSequenceValidationRtw(cxt);
  EXPECT_TRUE(cxt.prevFltResults.empty());
  EXPECT_TRUE(cxt.prevSegStats.empty());
  EXPECT_TRUE(cxt.prevRtwPC.empty());
  EXPECT_EQ(oldStatus, _fu->segmentStatus());
}

TEST_F(BookingCodeExceptionValidatorRtwTest, testPrepareSequenceValidationRtw_AllUndefined)
{
  setupPrepareSequenceValidationRtw();
  TravelSegmentResultVector oldFltResults = getTsResultVector();
  PaxTypeFare::SegmentStatusVec oldStatus = _fu->segmentStatus();
  std::vector<CabinType> oldRtwPC = getRtwPreferredCabin();

  RtwPreferredCabinContext cxt;
  callPrepareSequenceValidationRtw(cxt);

  EXPECT_EQ(oldFltResults, cxt.prevFltResults);
  EXPECT_EQ(oldStatus, cxt.prevSegStats);
  EXPECT_EQ(oldRtwPC, cxt.prevRtwPC);
  EXPECT_EQ(oldFltResults, getTsResultVector());
  EXPECT_EQ(oldStatus, _fu->segmentStatus());
}

TEST_F(BookingCodeExceptionValidatorRtwTest, testPrepareSequenceValidationRtw_Economy)
{
  setupPrepareSequenceValidationRtw();
  getRtwPreferredCabin()[0].setEconomyClass();
  TravelSegmentResultVector oldFltResults = getTsResultVector();
  PaxTypeFare::SegmentStatusVec oldStatus = _fu->segmentStatus();
  std::vector<CabinType> oldRtwPC = getRtwPreferredCabin();

  RtwPreferredCabinContext cxt;
  callPrepareSequenceValidationRtw(cxt);

  EXPECT_EQ(oldFltResults, cxt.prevFltResults);
  EXPECT_EQ(oldStatus, cxt.prevSegStats);
  EXPECT_EQ(oldRtwPC, cxt.prevRtwPC);
  EXPECT_EQ(oldFltResults[1], getTsResultVector()[1]);
  EXPECT_EQ(oldStatus[1], _fu->segmentStatus()[1]);
  TravelSegResult emptyTSR(BookingCodeExceptionValidator::BCE_FLT_NOMATCH,
                           BookingCodeExceptionValidator::BCE_FLT_NOMATCH);
  EXPECT_EQ(emptyTSR, getTsResultVector()[0]);
  EXPECT_FALSE(_fu->segmentStatus()[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_PASS));
  EXPECT_FALSE(_fu->segmentStatus()[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_FAIL));
  EXPECT_FALSE(_fu->segmentStatus()[0]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED));
  EXPECT_TRUE(_fu->segmentStatus()[0]._bkgCodeReBook.empty());
  EXPECT_TRUE(_fu->segmentStatus()[0]._reBookCabin.isUndefinedClass());
}
}
