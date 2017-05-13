#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/std/vector.hpp>
#include "Common/VCTR.h"
#include "DataModel/FareMarket.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/StopoversInfo.h"
#include "Rules/StopoversInfoWrapper.h"
#include "Rules/StopoversTravelSegWrapper.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/RuleItemInfo.h"
#include "DBAccess/StopoversInfoSeg.h"
#include "Rules/Stopovers.h"
#include "Common/ChargeSODirect.h"
#include "test/include/TestMemHandle.h"

using namespace boost::assign;
using namespace std;

namespace tse
{

class StopoversInfoWrapperStub : public StopoversInfoWrapper
{
public:
  StopoversInfoWrapperStub() : StopoversInfoWrapper() {}

  ~StopoversInfoWrapperStub() {}

  void soInfo(const StopoversInfo* soInfo) { return StopoversInfoWrapper::soInfo(soInfo); }

  bool checkIfChargeIsApplicable(const StopoversInfoSeg* soInfoSeg,
                                 const int16_t chargeNum,
                                 const FMDirection dir,
                                 const bool forceAddAmt) const
  {
    return StopoversInfoWrapper::checkIfChargeIsApplicable(soInfoSeg, chargeNum, dir, forceAddAmt);
  }

  void updateDirectionalityChargeIfApplicable(PricingTrx& trx,
                                              const StopoversInfoSeg* soInfoSeg,
                                              const int16_t chargeNum,
                                              const FMDirection dir,
                                              const bool forceAddAmt) const
  {
    return StopoversInfoWrapper::updateDirectionalityChargeIfApplicable(
        trx, soInfoSeg, chargeNum, dir, forceAddAmt);
  }

  bool numberOfChargesOneOrTwoApplied(const int16_t chargeNum) const
  {
    return StopoversInfoWrapper::numberOfChargesOneOrTwoApplied(chargeNum);
  }
};

class StopoversInfoWrapperTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(StopoversInfoWrapperTest);

  CPPUNIT_TEST(testCheckIfChargeIsApplicableWhenWrongChargeNum);
  CPPUNIT_TEST(testCheckIfChargeIsApplicableWhenMapIsEmpty);

  CPPUNIT_TEST(testCheckIfChargeIsApplicableWhenMapIsNotEmptyAndSoInfoSegHasNoDirectionality);
  CPPUNIT_TEST(testCheckIfChargeIsApplicableWhenMapIsNotEmptyAndSoInfoSegNotMatch);
  CPPUNIT_TEST(
      testCheckIfChargeIsApplicableWhenMapIsNotEmptyAndSoInfoSegMatchAndDirectSEG_INOUT_EITHER1);
  CPPUNIT_TEST(
      testCheckIfChargeIsApplicableWhenMapIsNotEmptyAndSoInfoSegMatchAndDirectSEG_INOUT_EITHER2);
  CPPUNIT_TEST(
      testCheckIfChargeIsApplicableWhenMapIsNotEmptyAndSoInfoSegMatchAndChangeONEForcedAddAmtReachMax);
  CPPUNIT_TEST(
      testCheckIfChargeIsApplicableWhenMapIsNotEmptyAndSoInfoSegMatchAndChangeONEForcedAddAmtNotReachMax);
  CPPUNIT_TEST(
      testCheckIfChargeIsApplicableWhenMapIsNotEmptyAndSoInfoSegMatchAndChangeONECHARGE1AmtReachMax);
  CPPUNIT_TEST(
      testCheckIfChargeIsApplicableWhenMapIsNotEmptyAndSoInfoSegMatchAndChangeONECHARGE1AmtNotReachMax);

  CPPUNIT_TEST(
      testCheckIfChargeIsApplicableWhenMapIsNotEmptyAndSoInfoSegMatchAndChangeTWOForcedAddAmtReachMax);
  CPPUNIT_TEST(
      testCheckIfChargeIsApplicableWhenMapIsNotEmptyAndSoInfoSegMatchAndChangeTWOForcedAddAmtNotReachMax);
  CPPUNIT_TEST(
      testCheckIfChargeIsApplicableWhenMapIsNotEmptyAndSoInfoSegMatchAndChangeTWOCHARGE1AmtReachMax);
  CPPUNIT_TEST(
      testCheckIfChargeIsApplicableWhenMapIsNotEmptyAndSoInfoSegMatchAndChangeTWOCHARGE1AmtNotReachMax);

  CPPUNIT_TEST(testUpdateDirectionalityChargeIfApplicableWhenSoInfoSegZero);
  CPPUNIT_TEST(testUpdateDirectionalityChargeIfApplicableWhenWrongChargeNum);
  CPPUNIT_TEST(testUpdateDirectionalityChargeIfApplicableWhenSoInfoSegDirIsBlank);
  CPPUNIT_TEST(testUpdateDirectionalityChargeIfApplicableWhenSoInfoSegDirIsNotBlank);
  CPPUNIT_TEST(testUpdateDirectionalityChargeIfApplicableWhenSameSoInfoSegUsed);
  CPPUNIT_TEST(testUpdateDirectionalityChargeIfApplicableWhenDifferentSoInfoSegUsed);
  CPPUNIT_TEST(testUpdateDirectionalityChargeIfApplicableWhenSoInfoSegDirIsNotBlankAndChangeNumTwo);
  CPPUNIT_TEST(testUpdateDirectionalityChargeIfApplicableWhenSameSoInfoSegUsedAndChangeNumTwo);
  CPPUNIT_TEST(testUpdateDirectionalityChargeIfApplicableWhenDifferentSoInfoSegUsedAndChangeNumTwo);
  CPPUNIT_TEST(
      testUpdateDirectionalityChargeIfApplicableWhenSameSoInfoSegUsedAndChangeNumOneAndTwo);
  CPPUNIT_TEST(
      testUpdateDirectionalityChargeIfApplicableWhenDifferentSoInfoSegUsedAndChangeNumOneAndTwo);
  CPPUNIT_TEST(
      testUpdateDirectionalityChargeIfApplicableWhenSameSoInfoSegUsedAndChangeNumOneAndForceADD);
  CPPUNIT_TEST(
      testUpdateDirectionalityChargeIfApplicableWhenDifferentSoInfoSegUsedAndChangeNumOneAndTwoAndForceADD);
  CPPUNIT_TEST(testNumberOfChargesOneOrTwoAppliedAndChangeNumOneAndRuleItemNumberNotMatch);
  CPPUNIT_TEST(testNumberOfChargesOneOrTwoAppliedAndChangeNumTwoAndRuleItemNumberNotMatch);
  CPPUNIT_TEST(
      testNumberOfChargesOneOrTwoAppliedAndChangeNumOneAndRuleItemNumberMatchForFirstSoInfoSeg);
  CPPUNIT_TEST(
      testNumberOfChargesOneOrTwoAppliedAndChangeNumOneAndRuleItemNumberMatchForFirstSoInfoSegReachMax);
  CPPUNIT_TEST(
      testNumberOfChargesOneOrTwoAppliedAndChangeNumTwoAndRuleItemNumberMatchForSecondSoInfoSeg);
  CPPUNIT_TEST(
      testNumberOfChargesOneOrTwoAppliedAndChangeNumTwoAndRuleItemNumberMatchForSecondSoInfoSegReachMax);

  CPPUNIT_TEST_SUITE_END();

private:
  PricingTrx* _trx;
  PricingRequest* _request;
  FareMarket* _fareMarket;
  StopoversInfoWrapper* _siw;
  StopoversInfoWrapperStub* _siws;
  RuleItemInfo* _ruleItemInfo;
  StopoversTravelSegWrapper* _stsw;
  CategoryRuleInfo* _cri;
  TestMemHandle _memHandle;
  FMDirection _dir;
  StopoversInfoSeg* _soInfoSeg;

public:
  void setUp()
  {
    _trx = _memHandle.create<PricingTrx>();
    _request = _memHandle.create<PricingRequest>();
    _request->ticketingDT() = DateTime::localTime();
    _trx->setRequest(_request);
    _fareMarket = _memHandle.create<FareMarket>();
    _ruleItemInfo = _memHandle.create<RuleItemInfo>();
    _cri = _memHandle.create<CategoryRuleInfo>();
    _stsw = _memHandle.create<StopoversTravelSegWrapper>();
    _soInfoSeg = _memHandle.create<StopoversInfoSeg>();

    _memHandle.insert(_siws = new StopoversInfoWrapperStub());
    _memHandle.insert(_siw = new StopoversInfoWrapper());

    createBaseCategoryRuleInfo(*_cri);
    _siw->crInfo(_cri);
    _stsw->ruleVCTRMatch() = _siw->getVCTR();

    _dir = FMDirection::OUTBOUND;
  }

  void tearDown() { _memHandle.clear(); }

  void createBaseStopoversInfo(tse::StopoversInfo& soInfo)
  {
    soInfo.vendor() = "ATP";
    soInfo.itemNo() = 1234;
    soInfo.unavailTag() = ' ';
    soInfo.geoTblItemNoBtw() = 0;
    soInfo.geoTblItemNoAnd() = 0;
    soInfo.geoTblItemNoGateway() = 0;
    soInfo.gtwyInd() = ' ';
    soInfo.samePointsTblItemNo() = 0;
    soInfo.samePntStops() = 0;
    soInfo.samePntTransit() = 0;
    soInfo.samePntConnections() = 0;
    soInfo.noStopsMin() = "00";
    soInfo.noStopsMax() = "XX";
    soInfo.noStopsOutbound() = "";
    soInfo.noStopsInbound() = "";
    soInfo.minStayTime() = 0;
    soInfo.maxStayTime() = 0;
    soInfo.minStayTimeUnit() = ' ';
    soInfo.maxStayTimeUnit() = ' ';
    soInfo.outOrReturnInd() = ' ';
    soInfo.sameCarrierInd() = ' ';
    soInfo.ojStopoverInd() = ' ';
    soInfo.ct2StopoverInd() = ' ';
    soInfo.ct2PlusStopoverInd() = ' ';

    soInfo.charge1FirstAmt() = 0;
    soInfo.charge1AddAmt() = 0;
    soInfo.charge1NoDec() = 0;
    soInfo.charge1Appl() = ' ';
    soInfo.charge1Total() = ' ';
    soInfo.charge1First() = "";
    soInfo.charge1AddNo() = "";
    soInfo.charge1Cur() = "";

    soInfo.charge2FirstAmt() = 0;
    soInfo.charge2AddAmt() = 0;
    soInfo.charge2NoDec() = 0;
    soInfo.charge2Appl() = ' ';
    soInfo.charge2Total() = ' ';
    soInfo.charge2First() = "";
    soInfo.charge2AddNo() = "";
    soInfo.charge2Cur() = "";
  }

  void createBaseCategoryRuleInfo(CategoryRuleInfo& crInfo)
  {
    crInfo.categoryNumber() = RuleConst::STOPOVER_RULE;
    crInfo.vendorCode() = "ATP";
    crInfo.tariffNumber() = 1;
    crInfo.carrierCode() = "NW";
    crInfo.ruleNumber() = "0001";
    crInfo.sequenceNumber() = 1;
  }

  void testCheckIfChargeIsApplicableWhenWrongChargeNum()
  {
    CPPUNIT_ASSERT_EQUAL(false, _siws->checkIfChargeIsApplicable(_soInfoSeg, 3, _dir, false));
  }

  void testCheckIfChargeIsApplicableWhenMapIsEmpty()
  {
    CPPUNIT_ASSERT_EQUAL(false, _siws->checkIfChargeIsApplicable(_soInfoSeg, 1, _dir, false));
  }

  void testCheckIfChargeIsApplicableWhenMapIsNotEmptyAndSoInfoSegHasNoDirectionality()
  {
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_BLANK;
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 1, _dir, false);
    CPPUNIT_ASSERT_EQUAL(false, _siws->checkIfChargeIsApplicable(_soInfoSeg, 1, _dir, false));
  }

  void testCheckIfChargeIsApplicableWhenMapIsNotEmptyAndSoInfoSegNotMatch()
  {
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_IN;
    StopoversInfoSeg* soInfoSegI = new StopoversInfoSeg();
    soInfoSegI->stopoverInOutInd() = Stopovers::SEG_INOUT_IN;
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 1, _dir, false);
    CPPUNIT_ASSERT_EQUAL(true, _siws->checkIfChargeIsApplicable(soInfoSegI, 1, _dir, false));
  }

  void testCheckIfChargeIsApplicableWhenMapIsNotEmptyAndSoInfoSegMatchAndDirectSEG_INOUT_EITHER1()
  {
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_EITHER;
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 1, _dir, false);
    CPPUNIT_ASSERT_EQUAL(
        false, _siws->checkIfChargeIsApplicable(_soInfoSeg, 1, FMDirection::INBOUND, false));
  }

  void testCheckIfChargeIsApplicableWhenMapIsNotEmptyAndSoInfoSegMatchAndDirectSEG_INOUT_EITHER2()
  {
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_EITHER;
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 1, FMDirection::INBOUND, false);
    CPPUNIT_ASSERT_EQUAL(false, _siws->checkIfChargeIsApplicable(_soInfoSeg, 1, _dir, false));
  }

  void
  testCheckIfChargeIsApplicableWhenMapIsNotEmptyAndSoInfoSegMatchAndChangeONEForcedAddAmtReachMax()
  {
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_OUT;
    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    soInfo.charge1AddNo() = "01";
    _siws->soInfo(&soInfo);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 1, _dir, true);
    CPPUNIT_ASSERT_EQUAL(false, _siws->checkIfChargeIsApplicable(_soInfoSeg, 1, _dir, true));
  }
  void
  testCheckIfChargeIsApplicableWhenMapIsNotEmptyAndSoInfoSegMatchAndChangeONEForcedAddAmtNotReachMax()
  {
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_OUT;
    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    soInfo.charge1AddNo() = "02";
    _siws->soInfo(&soInfo);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 1, _dir, true);
    CPPUNIT_ASSERT_EQUAL(true, _siws->checkIfChargeIsApplicable(_soInfoSeg, 1, _dir, true));
  }

  void
  testCheckIfChargeIsApplicableWhenMapIsNotEmptyAndSoInfoSegMatchAndChangeONECHARGE1AmtReachMax()
  {
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_OUT;
    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    soInfo.charge1First() = "01";
    _siws->soInfo(&soInfo);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 1, _dir, false);
    CPPUNIT_ASSERT_EQUAL(false, _siws->checkIfChargeIsApplicable(_soInfoSeg, 1, _dir, false));
  }
  void
  testCheckIfChargeIsApplicableWhenMapIsNotEmptyAndSoInfoSegMatchAndChangeONECHARGE1AmtNotReachMax()
  {
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_OUT;
    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    soInfo.charge1First() = "02";
    _siws->soInfo(&soInfo);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 1, _dir, false);
    CPPUNIT_ASSERT_EQUAL(true, _siws->checkIfChargeIsApplicable(_soInfoSeg, 1, _dir, false));
  }

  void
  testCheckIfChargeIsApplicableWhenMapIsNotEmptyAndSoInfoSegMatchAndChangeTWOForcedAddAmtReachMax()
  {
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_OUT;
    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    soInfo.charge2AddNo() = "01";
    _siws->soInfo(&soInfo);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 2, _dir, true);
    CPPUNIT_ASSERT_EQUAL(false, _siws->checkIfChargeIsApplicable(_soInfoSeg, 2, _dir, true));
  }
  void
  testCheckIfChargeIsApplicableWhenMapIsNotEmptyAndSoInfoSegMatchAndChangeTWOForcedAddAmtNotReachMax()
  {
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_OUT;
    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    //soInfo.charge2AddNo() = "02";
    soInfo.charge1AddNo() = "02";
    _siws->soInfo(&soInfo);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 2, _dir, true);
    CPPUNIT_ASSERT_EQUAL(true, _siws->checkIfChargeIsApplicable(_soInfoSeg, 2, _dir, true));
  }

  void
  testCheckIfChargeIsApplicableWhenMapIsNotEmptyAndSoInfoSegMatchAndChangeTWOCHARGE1AmtReachMax()
  {
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_OUT;
    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    soInfo.charge2First() = "01";
    _siws->soInfo(&soInfo);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 2, _dir, false);
    CPPUNIT_ASSERT_EQUAL(false, _siws->checkIfChargeIsApplicable(_soInfoSeg, 2, _dir, false));
  }
  void
  testCheckIfChargeIsApplicableWhenMapIsNotEmptyAndSoInfoSegMatchAndChangeTWOCHARGE1AmtNotReachMax()
  {
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_OUT;
    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    soInfo.charge1First() = "02";
    _siws->soInfo(&soInfo);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 2, _dir, false);
    CPPUNIT_ASSERT_EQUAL(true, _siws->checkIfChargeIsApplicable(_soInfoSeg, 2, _dir, false));
  }

  void testUpdateDirectionalityChargeIfApplicableWhenSoInfoSegZero()
  {
    StopoversInfoSeg* soInfoSeg = 0;
    _siws->updateDirectionalityChargeIfApplicable(*_trx, soInfoSeg, 1, _dir, false);
    CPPUNIT_ASSERT(_siws->_infoSegsDirectionalityCharge.empty());
  }

  void testUpdateDirectionalityChargeIfApplicableWhenWrongChargeNum()
  {
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 3, _dir, false);
    CPPUNIT_ASSERT(_siws->_infoSegsDirectionalityCharge.empty());
  }

  void testUpdateDirectionalityChargeIfApplicableWhenSoInfoSegDirIsBlank()
  {
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_BLANK;
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 1, _dir, false);
    CPPUNIT_ASSERT(_siws->_infoSegsDirectionalityCharge.empty());
  }

  //  Change Num = 1

  void testUpdateDirectionalityChargeIfApplicableWhenSoInfoSegDirIsNotBlank()
  {
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_OUT;
    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    _siws->soInfo(&soInfo);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 1, _dir, false);
    CPPUNIT_ASSERT(!_siws->_infoSegsDirectionalityCharge.empty());
  }

  void testUpdateDirectionalityChargeIfApplicableWhenSameSoInfoSegUsed()
  {
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_OUT;
    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    _siws->soInfo(&soInfo);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 1, _dir, false);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 1, _dir, false);
    size_t sizeOne = 1;
    CPPUNIT_ASSERT_EQUAL(sizeOne, _siws->_infoSegsDirectionalityCharge.size());
  }

  void testUpdateDirectionalityChargeIfApplicableWhenDifferentSoInfoSegUsed()
  {
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_OUT;
    StopoversInfoSeg* soInfoSegI = new StopoversInfoSeg();
    soInfoSegI->stopoverInOutInd() = Stopovers::SEG_INOUT_IN;
    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    _siws->soInfo(&soInfo);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 1, _dir, false);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, soInfoSegI, 1, _dir, false);
    size_t sizeTwo = 2;
    CPPUNIT_ASSERT_EQUAL(sizeTwo, _siws->_infoSegsDirectionalityCharge.size());
  }

  //  Change Num = 2

  void testUpdateDirectionalityChargeIfApplicableWhenSoInfoSegDirIsNotBlankAndChangeNumTwo()
  {
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_OUT;
    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    _siws->soInfo(&soInfo);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 2, _dir, false);
    CPPUNIT_ASSERT(!_siws->_infoSegsDirectionalityCharge.empty());
  }

  void testUpdateDirectionalityChargeIfApplicableWhenSameSoInfoSegUsedAndChangeNumTwo()
  {
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_OUT;
    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    _siws->soInfo(&soInfo);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 2, _dir, false);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 2, _dir, false);
    size_t sizeOne = 1;
    CPPUNIT_ASSERT_EQUAL(sizeOne, _siws->_infoSegsDirectionalityCharge.size());
  }

  void testUpdateDirectionalityChargeIfApplicableWhenDifferentSoInfoSegUsedAndChangeNumTwo()
  {
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_OUT;
    StopoversInfoSeg* soInfoSegI = new StopoversInfoSeg();
    soInfoSegI->stopoverInOutInd() = Stopovers::SEG_INOUT_IN;
    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    _siws->soInfo(&soInfo);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 2, _dir, false);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, soInfoSegI, 2, _dir, false);
    size_t sizeTwo = 2;
    CPPUNIT_ASSERT_EQUAL(sizeTwo, _siws->_infoSegsDirectionalityCharge.size());
  }

  //  Change Num = 1 & 2

  void testUpdateDirectionalityChargeIfApplicableWhenSameSoInfoSegUsedAndChangeNumOneAndTwo()
  {
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_OUT;
    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    _siws->soInfo(&soInfo);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 1, _dir, false);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 2, _dir, false);
    size_t sizeOne = 1;
    CPPUNIT_ASSERT_EQUAL(sizeOne, _siws->_infoSegsDirectionalityCharge.size());
  }

  void testUpdateDirectionalityChargeIfApplicableWhenDifferentSoInfoSegUsedAndChangeNumOneAndTwo()
  {
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_OUT;
    StopoversInfoSeg* soInfoSegI = new StopoversInfoSeg();
    soInfoSegI->stopoverInOutInd() = Stopovers::SEG_INOUT_IN;
    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    _siws->soInfo(&soInfo);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 1, _dir, false);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, soInfoSegI, 2, _dir, false);
    size_t sizeTwo = 2;
    CPPUNIT_ASSERT_EQUAL(sizeTwo, _siws->_infoSegsDirectionalityCharge.size());
  }

  //  Change Num = 1 and ForceAddAmount On

  void testUpdateDirectionalityChargeIfApplicableWhenSameSoInfoSegUsedAndChangeNumOneAndForceADD()
  {
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_OUT;
    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    _siws->soInfo(&soInfo);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 1, _dir, true);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 1, _dir, false);
    size_t sizeOne = 1;
    CPPUNIT_ASSERT_EQUAL(sizeOne, _siws->_infoSegsDirectionalityCharge.size());
  }

  //  Change Num = 1 & 2 and ForceAddAmount On

  void
  testUpdateDirectionalityChargeIfApplicableWhenDifferentSoInfoSegUsedAndChangeNumOneAndTwoAndForceADD()
  {
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_OUT;
    StopoversInfoSeg* soInfoSegI = new StopoversInfoSeg();
    soInfoSegI->stopoverInOutInd() = Stopovers::SEG_INOUT_IN;
    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    _siws->soInfo(&soInfo);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 1, _dir, true);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, soInfoSegI, 2, _dir, true);
    size_t sizeTwo = 2;
    CPPUNIT_ASSERT_EQUAL(sizeTwo, _siws->_infoSegsDirectionalityCharge.size());
  }

  //  Change Num = 1

  void testNumberOfChargesOneOrTwoAppliedAndChangeNumOneAndRuleItemNumberNotMatch()
  {
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_OUT;
    StopoversInfoSeg* soInfoSegI = new StopoversInfoSeg();
    soInfoSegI->stopoverInOutInd() = Stopovers::SEG_INOUT_IN;
    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    _siws->soInfo(&soInfo);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 1, _dir, false);
    _siws->itemNo() = 1233;
    _siws->updateDirectionalityChargeIfApplicable(*_trx, soInfoSegI, 1, _dir, false);
    _siws->itemNo() = 1232;
    int16_t chargeNum = 1;

    CPPUNIT_ASSERT_EQUAL(false, _siws->numberOfChargesOneOrTwoApplied(chargeNum));
  }

  void testNumberOfChargesOneOrTwoAppliedAndChangeNumTwoAndRuleItemNumberNotMatch()
  {
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_OUT;
    StopoversInfoSeg* soInfoSegI = new StopoversInfoSeg();
    soInfoSegI->stopoverInOutInd() = Stopovers::SEG_INOUT_IN;
    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    _siws->soInfo(&soInfo);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, 1, _dir, false);
    _siws->itemNo() = 1233;
    _siws->updateDirectionalityChargeIfApplicable(*_trx, soInfoSegI, 1, _dir, false);
    _siws->itemNo() = 1232;
    int16_t chargeNum = 2;

    CPPUNIT_ASSERT_EQUAL(false, _siws->numberOfChargesOneOrTwoApplied(chargeNum));
  }

  void testNumberOfChargesOneOrTwoAppliedAndChangeNumOneAndRuleItemNumberMatchForFirstSoInfoSeg()
  {
    int16_t chargeNum = 1;
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_OUT;
    StopoversInfoSeg* soInfoSegI = new StopoversInfoSeg();
    soInfoSegI->stopoverInOutInd() = Stopovers::SEG_INOUT_IN;
    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    soInfo.charge1First() = "02";
    _siws->soInfo(&soInfo);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, chargeNum, _dir, false);
    _siws->itemNo() = 1233;
    _siws->updateDirectionalityChargeIfApplicable(*_trx, soInfoSegI, chargeNum, _dir, false);
    _siws->itemNo() = 1234;

    CPPUNIT_ASSERT_EQUAL(true, _siws->numberOfChargesOneOrTwoApplied(chargeNum));
  }

  void
  testNumberOfChargesOneOrTwoAppliedAndChangeNumOneAndRuleItemNumberMatchForFirstSoInfoSegReachMax()
  {
    int16_t chargeNum = 1;
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_OUT;
    StopoversInfoSeg* soInfoSegI = new StopoversInfoSeg();
    soInfoSegI->stopoverInOutInd() = Stopovers::SEG_INOUT_IN;
    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    soInfo.charge1First() = "01";
    _siws->soInfo(&soInfo);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, chargeNum, _dir, false);
    _siws->itemNo() = 1233;
    _siws->updateDirectionalityChargeIfApplicable(*_trx, soInfoSegI, chargeNum, _dir, false);
    _siws->itemNo() = 1234;

    CPPUNIT_ASSERT_EQUAL(false, _siws->numberOfChargesOneOrTwoApplied(chargeNum));
  }

  void testNumberOfChargesOneOrTwoAppliedAndChangeNumTwoAndRuleItemNumberMatchForSecondSoInfoSeg()
  {
    int16_t chargeNum = 2;
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_OUT;
    StopoversInfoSeg* soInfoSegI = new StopoversInfoSeg();
    soInfoSegI->stopoverInOutInd() = Stopovers::SEG_INOUT_IN;
    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    soInfo.charge1First() = "02";
    _siws->soInfo(&soInfo);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, chargeNum, _dir, false);
    _siws->itemNo() = 1233;
    _siws->updateDirectionalityChargeIfApplicable(*_trx, soInfoSegI, chargeNum, _dir, false);

    CPPUNIT_ASSERT_EQUAL(true, _siws->numberOfChargesOneOrTwoApplied(chargeNum));
  }

  void
  testNumberOfChargesOneOrTwoAppliedAndChangeNumTwoAndRuleItemNumberMatchForSecondSoInfoSegReachMax()
  {
    int16_t chargeNum = 2;
    _soInfoSeg->stopoverInOutInd() = Stopovers::SEG_INOUT_OUT;
    StopoversInfoSeg* soInfoSegI = new StopoversInfoSeg();
    soInfoSegI->stopoverInOutInd() = Stopovers::SEG_INOUT_IN;
    StopoversInfo soInfo;
    createBaseStopoversInfo(soInfo);
    soInfo.charge2First() = "01";
    _siws->soInfo(&soInfo);
    _siws->updateDirectionalityChargeIfApplicable(*_trx, _soInfoSeg, chargeNum, _dir, false);
    _siws->itemNo() = 1233;
    _siws->updateDirectionalityChargeIfApplicable(*_trx, soInfoSegI, chargeNum, _dir, false);

    CPPUNIT_ASSERT_EQUAL(false, _siws->numberOfChargesOneOrTwoApplied(chargeNum));
  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(StopoversInfoWrapperTest);
}
