//-------------------------------------------------------------------
//
//  File:        RepriceSolutionValidatorTest.cpp
//  Created:     October 01, 2007
//  Authors:     Daniel Rolka
//
//  Updates:
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

#include "Common/DateTime.h"
#include "Common/ErrorResponseException.h"
#include "Common/Money.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TrxUtil.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/ProcessTagPermutation.h"
#include "DataModel/ReissueCharges.h"
#include "DataModel/RexBaseRequest.h"
#include "DataModel/RexPricingOptions.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CarrierApplicationInfo.h"
#include "DBAccess/Customer.h"
#include "DBAccess/DateOverrideRuleItem.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/SamePoint.h"
#include "RexPricing/RepriceSolutionValidator.h"
#include "RexPricing/SequenceStopByteByTag.h"
#include "Rules/RuleConst.h"
#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/CppUnitHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestLogger.h"
#include "test/include/TestMemHandle.h"

using namespace std;

namespace tse
{
enum
{ CXRAPPLITEMNO_CARRIER_INCLUDED = 1,
  CXRAPPLITEMNO_CARRIER_EXCLUDED,
  CXRAPPLITEMNO_CARRIER_DOLLAR };

static const CarrierCode CARRIER_AA = "AA";
static const CarrierCode CARRIER_LH = "LH";

class FarePathMock : public FarePath
{
  MoneyAmount _amount;
  Itin _itin;

public:
  FarePathMock(MoneyAmount amount) : _amount(amount) { itin() = &_itin; }

  MoneyAmount getNonrefundableAmountInNUC(const PricingTrx& trx) const { return _amount; }
};

class MyDataHandle : public DataHandleMock
{
  std::vector<DateOverrideRuleItem*>& _doriVector;

public:
  MyDataHandle(std::vector<DateOverrideRuleItem*>& doriVector) : _doriVector(doriVector) {}

  const std::vector<DateOverrideRuleItem*>&
  getDateOverrideRuleItem(const VendorCode&, int, const DateTime&)
  {
    return _doriVector;
  }
};

class RepriceSolutionValidatorOverride : public RepriceSolutionValidator
{
public:
  RepriceSolutionValidatorOverride(RexPricingTrx& trx, FarePath& fp)
    : RepriceSolutionValidator(trx, fp), _dataHandle(_doriVector)
  {
    string date = "2007-01-10 11:11";
    _latestBookingDate = DateTime(date);
  }

  vector<const SamePoint*> _samePointVector;
  vector<DateOverrideRuleItem*> _doriVector;
  DateTime _latestBookingDate;

  const vector<const SamePoint*>&
  getSamePoint(const VendorCode& vendor, int itemNo, const DateTime& date)
  {
    return _samePointVector;
  }

  bool convertCurrency(Money& target, const Money& source)
  {
    target.value() = source.value();
    return true;
  }

protected:
  virtual bool
  zeroT988ExcFare(const ProcessTagPermutation& permutation, const PaxTypeFare& ptf) const
  {
    return false;
  }

  MyDataHandle _dataHandle;
};

class PaxTypeFareMock : public PaxTypeFare
{
public:
  PaxTypeFareMock() { _vendor = ATPCO_VENDOR_CODE; }
  PaxTypeFareMock(VendorCode vendor) : _vendor(vendor) {}
  const VendorCode& vendor() const { return _vendor; }
  const CarrierCode& carrier() const { return _carrier; }
  VendorCode _vendor;
  CarrierCode _carrier;
};

class PermutationMock : public ProcessTagPermutation
{
public:
  PermutationMock(Indicator ind) : _ind(ind) {}
  PermutationMock() : _ind(NEW_TICKET_EQUAL_OR_HIGHER_BLANK) {}
  virtual Indicator checkTable988Byte156() const { return _ind; }

private:
  Indicator _ind;
};

class RepriceSolutionValidatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(RepriceSolutionValidatorTest);

  CPPUNIT_SKIP_TEST(testMatchJourneyRepricingPermitted1);
  CPPUNIT_SKIP_TEST(testMatchJourneyRepricingPermitted2);
  CPPUNIT_SKIP_TEST(testMatchJourneyRepricingDenied);
  CPPUNIT_SKIP_TEST(testCheckSamePointTableForTermBytePass);
  CPPUNIT_SKIP_TEST(testCheckSamePointTableForTermByteFail);
  CPPUNIT_SKIP_TEST(testCheckSamePointTableForFirstFareBytePass);
  CPPUNIT_SKIP_TEST(testCheckSamePointTableForFirstFareByteFail);
  CPPUNIT_TEST(testAnalyseExcFarePath);

  CPPUNIT_TEST(testFullyFlownNoRestrictionPass);
  CPPUNIT_TEST(testFullyFlownOnePoint_OneExcToOneNewPass);
  CPPUNIT_TEST(testFullyFlownOnePoint_OneExcToTwoNewPass);
  CPPUNIT_TEST(testFullyFlownOnePoint_OneExcToThreeNewFail);
  CPPUNIT_TEST(testFullyFlownOnePoint_TwoExcToOneNewPass);
  CPPUNIT_TEST(testFullyFlownOnePoint_ThreeExcToOneNewPass);
  CPPUNIT_SKIP_TEST(testFullyFlownTheSameOldAndNewFCsPass);
  CPPUNIT_TEST(testFullyFlownAtLeastOnePoint_OneToMany);
  CPPUNIT_TEST(testFullyFlownAtLeastOnePoint_ManyToOne);
  CPPUNIT_TEST(testFullyFlownDestOnly_OneExcToOneNewPass);
  CPPUNIT_TEST(testFullyFlownDestOnly_OneExcToTwoNewPass);
  CPPUNIT_TEST(testFullyFlownDestOnly_OneExcToTwoNewFail);
  CPPUNIT_TEST(testFullyFlownDestOnly_OneExcToThreeNewPass);
  CPPUNIT_TEST(testFullyFlownDestOnly_OneExcToThreeNewFail);
  CPPUNIT_TEST(testFullyFlownDestOnly_TwoExcToOneNewPass);
  CPPUNIT_TEST(testFullyFlownDestOnly_ThreeExcToOneNewPass);
  CPPUNIT_TEST(testFullyFlownDestOnly_ThreeExcToTwoNewFail);
  CPPUNIT_TEST(testFullyFlownNoChange_OneExcToOneNewPass);
  CPPUNIT_TEST(testFullyFlownNoChange_TwoExcToTwoNewPass);
  CPPUNIT_TEST(testFullyFlownNoChange_OneExcToTwoNewFail);
  CPPUNIT_TEST(testFullyFlownNoChange_OneExcToThreeNewFail);
  CPPUNIT_TEST(testFullyFlownNoChange_TwoExcToOneNewFail);
  CPPUNIT_TEST(testFullyFlownNoChange_ThreeExcToOneNewFail);

  CPPUNIT_TEST(testCheckTermBlankSameBreak);
  CPPUNIT_TEST(testCheckTermBlankDiffBreak);
  CPPUNIT_TEST(testCheckTermAllowSameBreak);
  CPPUNIT_TEST(testCheckTermAllowDiffBreak);
  CPPUNIT_TEST(testCheckTermMultipleFC);

  CPPUNIT_TEST(testCheckFirstBreakByte25Blank);
  CPPUNIT_TEST(testCheckFirstBreakAllSame);
  CPPUNIT_TEST(testCheckFirstBreakNoFcMapping);
  CPPUNIT_TEST(testCheckFirstBreakDiffBreaks);
  CPPUNIT_TEST(testCheckFirstBreakDiffFareBasis);
  CPPUNIT_TEST(testCheckFirstBreakDiffFareAmount);
  CPPUNIT_TEST(testCheckFirstBreakNoVCTR);
  CPPUNIT_TEST(testCheckFirstBreakDiffVendor);
  CPPUNIT_TEST(testCheckFirstBreakDiffCarrier);
  CPPUNIT_TEST(testCheckFirstBreakDiffTariff);
  CPPUNIT_TEST(testCheckFirstBreakDiffRule);
  CPPUNIT_TEST(testCheckFirstBreakMultipleFCByte25Blank);
  CPPUNIT_TEST(testCheckFirstBreakMultipleFCByte25Y);

  CPPUNIT_TEST(testFindFareRetrievalFlagCurrent);
  CPPUNIT_TEST(testFindFareRetrievalFlagHistorical);
  CPPUNIT_TEST(testFindFareRetrievalFlagTvlCommence);
  CPPUNIT_TEST(testFindFareRetrievalFlagMixed);
  CPPUNIT_TEST(testFindFareRetrievalFlagCombined1);
  CPPUNIT_TEST(testFindFareRetrievalFlagCombined2);

  CPPUNIT_TEST(testCheckTagsTag2Only);
  CPPUNIT_TEST(testCheckTagsTag5Only);
  CPPUNIT_TEST(testCheckTagsTag6OnlyTravelCommenced);
  CPPUNIT_TEST(testCheckTagsTag6OnlyTravelNotCommenced);
  CPPUNIT_TEST(testCheckTagsTag7Only);
  CPPUNIT_TEST(testCheckTagsMixedTags_ExcKCC133_NewCC);
  CPPUNIT_TEST(testCheckTagsMixedTags_ExcKCC133_NewKCC);
  CPPUNIT_TEST(testCheckTagsMixedTags_ExcKCC133_NewKKC);
  CPPUNIT_TEST(testCheckTagsMixedTags_ExcKCC133_NewKCCC);
  CPPUNIT_TEST(testCheckTagsMixedTags_ExcKCC333_NewCC);
  CPPUNIT_TEST(testCheckTagsMixedTags_ExcKCC333_NewKCC);
  CPPUNIT_TEST(testCheckTagsMixedTags_ExcKCC333_NewKKC);
  CPPUNIT_TEST(testCheckTagsMixedTags_ExcKCC333_NewCCCC);
  CPPUNIT_TEST(testCheckTagsMixedTags_ExcKCC333_NewKCCC);
  CPPUNIT_TEST(testCheckTagsMixedTags_ExcKKK111_NewCC);
  CPPUNIT_TEST(testCheckTagsMixedTags_ExcKKK111_NewKCC);
  CPPUNIT_TEST(testCheckTagsMixedTags_ExcKKK111_NewKKC);
  CPPUNIT_TEST(testCheckTagsMixedTags_ExcKKK111_NewKCCK);
  CPPUNIT_TEST(testCheckTagsMixedTags_ExcKKK111_NewKKK);
  CPPUNIT_TEST(testCheckTagsMixedTags_ExcCCC555_NewCH);
  CPPUNIT_TEST(testCheckTagsMixedTags_ExcCCC555_NewKKK);
  CPPUNIT_TEST(testCheckTagsMixedTags_ExcCCC555_NewCC);
  CPPUNIT_SKIP_TEST(testCheckTagsMixedTags_ExcKKK111_NewKKK_FirstFCChanged);
  CPPUNIT_TEST(testCheckTagsMixedTags_ExcKKK111_NewKKK_FirstFCNotChanged);
  CPPUNIT_TEST(testCheckTagsMixedTags_ExcKKK111_NewKKKC);
  CPPUNIT_TEST(testCheckTagsMixedTags_ExcKKK111_NewKKKK);

  CPPUNIT_TEST(testCheckOWRT1);
  CPPUNIT_TEST(testCheckOWRT2);
  CPPUNIT_TEST(testCheckOWRT3);
  CPPUNIT_TEST(testCheckOWRT4);
  CPPUNIT_TEST(testCheckOWRT5);

  CPPUNIT_TEST(testvec3Cache);
  CPPUNIT_TEST(testLatestBookingDate);
  CPPUNIT_TEST(testLatestBookingDateSimple);
  CPPUNIT_TEST(testValidateLatestBookingDateEffFalse);
  CPPUNIT_TEST(testValidateLatestBookingDateDiscFalse);
  CPPUNIT_TEST(testValidateLatestBookingDateTrue);
  CPPUNIT_TEST(testFCBookingDateValidationFalse);
  CPPUNIT_TEST(testFCBookingDateValidationTrue);
  CPPUNIT_TEST(testJourneyBookingDateValidationFalse);
  CPPUNIT_TEST(testJourneyBookingDateValidationTrue);

  CPPUNIT_TEST(testOverrideReservationDatesNoMapping);
  CPPUNIT_TEST(testOverrideReservationDatesWithMapping);

  CPPUNIT_TEST(testIsBaseFareAmountPlusChangeFeeHigherPass);
  CPPUNIT_TEST(testIsBaseFareAmountPlusChangeFeeHigherFail);

  // Tag2 & Tag4
  CPPUNIT_TEST(testCheckTagsMixedTags_ExcHHH222_NewHHH_1FL2UC3UN);
  CPPUNIT_TEST(testCheckTagsMixedTags_ExcHHH222_NewHHH_1UU2UU3UU);
  CPPUNIT_TEST(testCheckTagsMixedTags_ExcKKK444_NewKCC_1FL2UC3UN);
  CPPUNIT_TEST(testCheckTagsMixedTags_ExcKKK444_NewKKK_1FL2UU3UU);

  CPPUNIT_TEST(testMatchJourney);
  CPPUNIT_TEST(testIsPUPartiallyFlown);
  CPPUNIT_TEST(testIsSameCpaCxrFareInExc);
  CPPUNIT_TEST(testCollectFlownOWFares);
  CPPUNIT_TEST(testAnalysePricingUnits);
  CPPUNIT_TEST(testHasStopByte);
  CPPUNIT_TEST(testIsSequenceGreater);
  CPPUNIT_TEST(testStopByteSave);
  CPPUNIT_TEST(testStopByteSkip);
  CPPUNIT_TEST(testStopByteSkipBypassed);
  CPPUNIT_TEST(testStopByteSkipBypassedHasLowestChangeFee);

  CPPUNIT_TEST(testCheckNewTicketEqualOrHigherPassOnBlank);
  CPPUNIT_TEST(testCheckNewTicketEqualOrHigherPassOnB);
  CPPUNIT_TEST(testCheckNewTicketEqualOrHigherPassOnN);
  CPPUNIT_TEST(testCheckNewTicketEqualOrHigherFailOnB);
  CPPUNIT_TEST(testCheckNewTicketEqualOrHigherFailOnN);
  CPPUNIT_TEST(testCheckNewTicketEqualOrHigherFailOnBWhenValidatingBN);
  CPPUNIT_TEST(testCheckNewTicketEqualOrHigherFailOnNWhenValidatingBN);
  CPPUNIT_TEST(testCheckNewTicketEqualOrHigherPassWhenValidatingBN);
  CPPUNIT_TEST(testCheckNewTicketEqualOrHigherPassOnTotal);
  CPPUNIT_TEST(testCheckNewTicketEqualOrHigherFailOnTotal);
  CPPUNIT_TEST(testCheckNewTicketEqualOrHigherPassOnNonrefundable);
  CPPUNIT_TEST(testCheckNewTicketEqualOrHigherFailOnNonrefundable);

  CPPUNIT_TEST(testIsChangedWhenStatusIsChanged);
  CPPUNIT_TEST(testIsChangedWhenStatusIsCabinChangedAndNotMappedTravelSegment);
  CPPUNIT_TEST(testIsChangedWhenStatusIsInventoryChangedAndEmptyRebookBookingCode);
  CPPUNIT_TEST(testIsChangedWhenStatusIsCabinChangedAndEmptyRebookBookingCode);
  CPPUNIT_TEST(testIsChangedWhenStatusIsUnchangedAndSameRebookBookingCode);
  CPPUNIT_TEST(testIsChangedWhenStatusIsUnchangedAndDifferentRebookBookingCode);
  CPPUNIT_TEST(testIsChangedWhenStatusIsUnchangedAndEmptyRebookBookingCode);

  CPPUNIT_TEST(testGetFareApplicationWhenRebookSolution);
  CPPUNIT_TEST(testGetFareApplicationWhenAsbookSolution);

  CPPUNIT_TEST(testRevalidateRules_PassCombFailWhenKeepFareB62XB72Set);
  CPPUNIT_TEST(testRevalidateRules_FailCombFailWhenKeepFareB62YB72Set);
  CPPUNIT_TEST(testRevalidateRules_FailCombFailWhenKeepFareB62XB72Blank);
  CPPUNIT_TEST(testRevalidateRules_FailCombFailWhenKeepFareB62Blank);

  CPPUNIT_TEST(testApplyChangeFeeToFarePath);

  CPPUNIT_TEST(testAnalyzeFUforAlreadyProcessedAndPassesdCat8Cat9_NoCat8Cat9ToRevalidate);
  CPPUNIT_TEST(testAnalyzeFUforAlreadyProcessedAndPassesdCat8Cat9_Cat8Cat9NotProcessedBefore);
  CPPUNIT_TEST(testAnalyzeFUforAlreadyProcessedAndPassesdCat8Cat9_Cat8IsProcessedBefore);
  CPPUNIT_TEST(testAnalyzeFUforAlreadyProcessedAndPassesdCat8Cat9_Cat9IsProcessedBefore);
  CPPUNIT_TEST(testAnalyzeFUforAlreadyProcessedAndPassesdCat8Cat9_Cat8And9AreProcessedBefore);

  CPPUNIT_TEST(testGetNewTicketAmountInNUCWhenNoExchangeReissue);
  CPPUNIT_TEST(testGetNewTicketAmountInNUCWhenExchangeReissueAndBaseCurrEqualCaclulationCurr);
  CPPUNIT_TEST(testGetExchTotalAmountNoReissueExchange);
  CPPUNIT_TEST(testGetExchTotalAmountIsReissueExchangeAndC5ANotEmpty);
  CPPUNIT_TEST(testGetExchTotalAmountIsReissueExchangeAndC5IsEmpty);
  CPPUNIT_TEST(testGetCurrencyNonExchReissie);
  CPPUNIT_TEST(testGetCurrencyForExchReissie);

  CPPUNIT_TEST(testAnyFailAtZeroT988NotFail);
  CPPUNIT_TEST(testAnyFailAtZeroT988PuRuleFail);
  CPPUNIT_TEST(testAnyFailAtZeroT988PuCombFail);
  CPPUNIT_TEST(testAnyFailAtZeroT988FuRuleFail);
  CPPUNIT_TEST(testAnyFailAtZeroT988FuCombFail);

  CPPUNIT_TEST(testMatchNewFcWithFa);

  CPPUNIT_TEST(testOutboundOfTravelSegmentsBlank);
  CPPUNIT_TEST(testOutboundOfTravelSegmentsFirstChanged);
  CPPUNIT_TEST(testOutboundOfTravelSegmentsOrigChanged);
  CPPUNIT_TEST(testOutboundOfTravelSegmentsFirstUnchanged);
  CPPUNIT_TEST(testOutboundOfTravelSegmentsOrigUnchanged);

  CPPUNIT_TEST_SUITE_END();

  void initValidator()
  {
    _validator = _memHandle(new RepriceSolutionValidatorOverride(*_trx, *_farePath));
    std::vector<const PaxTypeFare*>* ptfVec = create<std::vector<const PaxTypeFare*>>();
    ptfVec->push_back(create<PaxTypeFare>());
    _validator->_fcMapping.push_back(*ptfVec);
  }

public:
  void setUp()
  {
    create<TestConfigInitializer>();
    create<RootLoggerGetOff>();

    _trx = create<MockRexPricingTrx>();
    _exchangeItin = create<ExcItin>();
    _exchangeItin->rexTrx() = _trx;

    _trx->fareCalcConfig() = create<FareCalcConfig>();
    _trx->exchangeItin().push_back(_exchangeItin);
    _firstExcTravelSeg = create<AirSeg>();
    _exchangeItin->travelSeg().push_back(_firstExcTravelSeg);
    _farePath = create<FarePath>();
    _farePath->itin() = create<Itin>();
    _excFarePath = create<FarePath>();
    _excFarePath->itin() = _exchangeItin;

    _permutation = create<ProcessTagPermutation>();
    _ffExchangeItin = create<ExcItin>();
    _ffExchangeItin->farePath().push_back(create<FarePath>());
    _ffExchangeItin->farePath().front()->itin() = _ffExchangeItin;
    _ffExchangeItin->rexTrx() = _trx;
    _ffNewItin = create<Itin>();
    addAirSegment(_ffExchangeItin->travelSeg(), "NYC", "LON", "NYC", "LON", false);
    addAirSegment(_ffExchangeItin->travelSeg(), "LON", "PAR", "LON", "PAR", false);
    addAirSegment(_ffExchangeItin->travelSeg(), "PAR", "LON", "PAR", "LON", false);
    addAirSegment(_ffExchangeItin->travelSeg(), "LON", "NYC", "LON", "NYC", false);
    addAirSegment(_ffExchangeItin->travelSeg(), "NYC", "LON", "NYC", "LON", false);

    addAirSegment(_ffNewItin->travelSeg(), "NYC", "LON", "NYC", "LON", false);
    addAirSegment(_ffNewItin->travelSeg(), "LON", "PAR", "LON", "PAR", false);
    addAirSegment(_ffNewItin->travelSeg(), "PAR", "LON", "PAR", "LON", false);
    addAirSegment(_ffNewItin->travelSeg(), "LON", "NYC", "LON", "NYC", false);
    addAirSegment(_ffNewItin->travelSeg(), "NYC", "LON", "NYC", "LON", false);

    _trx->itin().push_back(create<Itin>());
    _arExchangeItin = create<ExcItin>();
    _arNewItin = create<Itin>();
    addAirSegment(_arExchangeItin->travelSeg(), "NYC", "CHI");
    addAirSegment(_arExchangeItin->travelSeg(), "CHI", "NYC");

    addAirSegment(_arNewItin->travelSeg(), "NYC", "CHI");
    addAirSegment(_arNewItin->travelSeg(), "CHI", "NYC");

    _exchangeItin->farePath().push_back(_excFarePath);

    _ts2ss = create<map<TravelSeg*, PaxTypeFare::SegmentStatus*>>();
    _excItinSegs = create<vector<TravelSeg*>>();
    _fm = create<FareMarket>();

    _ptfKeep = create<PaxTypeFare>();
    _ptfKeep->fareMarket() = addExcFcToNewItinKeepFares(_ptfKeep);
    _fuKeep = create<FareUsage>();
    _fuKeep->paxTypeFare() = _ptfKeep;
    _fuKeep->isKeepFare() = true;

    _rexBaseRequest = create<RexBaseRequest>();
    ;
    _agent = create<Agent>();
    ;
    _cus = create<Customer>();
    ;
    _cus->crsCarrier() = "1B";
    _cus->hostName() = "ABAC";
    _agent->agentTJR() = _cus;

    _rexBaseRequest->ticketingAgent() = _agent;
    TrxUtil::enableAbacus();

    _trx->setRequest(_rexBaseRequest);
    _trx->setExcTktNonRefundable(false);
    _trx->totalFareCalcAmount() = 5.00;
    RexPricingOptions* rexOptions = create<RexPricingOptions>();
    rexOptions->excBaseFareCurrency() = NUC;
    _trx->setOptions(rexOptions);
    initValidator();
  }

  void tearDown() { _memHandle.clear(); }

protected:
  // helper functions
  template <typename T>
  T* create()
  {
    return _memHandle.create<T>();
  }

  void createT988() { _t988 = create<ReissueSequence>(); }

  void createPtiWithT988()
  {
    createT988();
    _pti = create<ProcessTagInfo>();
    _pti->reissueSequence()->orig() = _t988;
  }

  void createPaxTypeFareWithFareAndFareInfo()
  {
    _ptf = create<PaxTypeFare>();
    _fare = create<Fare>();
    _fareInfo = create<FareInfo>();
    _fare->setFareInfo(_fareInfo);
    _ptf->setFare(_fare);
  }

  void addFareCompInfoToPti()
  {
    FareCompInfo* fci = create<FareCompInfo>();
    fci->fareMarket() = _fm;
    _pti->fareCompInfo() = fci;
  }

  void addPermutationData(ProcessTagPermutation& perm,
                          ExcItin& itin,
                          ProcessTag pt,
                          int itemNo,
                          int seqNo,
                          char journeyInd,
                          int samePointTblItemNo,
                          VendorCode vendor,
                          LocCode board,
                          LocCode off)
  {
    ProcessTagInfo* pInfo = create<ProcessTagInfo>();

    // ReissueSequence
    ReissueSequence* reissueSeq = create<ReissueSequence>();
    reissueSeq->processingInd() = (int)pt;
    reissueSeq->itemNo() = itemNo;
    reissueSeq->seqNo() = seqNo;
    reissueSeq->journeyInd() = journeyInd;
    reissueSeq->samePointTblItemNo() = samePointTblItemNo;
    pInfo->reissueSequence()->orig() = reissueSeq;

    // PaxTypeFare
    PaxTypeFare* paxType = create<PaxTypeFare>();
    FareInfo* fareInfo = create<FareInfo>();
    fareInfo->vendor() = vendor;
    Fare* fare = create<Fare>();
    fare->setFareInfo(fareInfo);
    paxType->setFare(fare);
    pInfo->paxTypeFare() = paxType;

    // FareMarket
    FareMarket* fareMarket = create<FareMarket>();
    fareMarket->boardMultiCity() = board;
    fareMarket->offMultiCity() = off;
    paxType->fareMarket() = fareMarket;

    // FareCompInfo
    FareCompInfo* fareCompInfo = create<FareCompInfo>();
    fareCompInfo->fareMarket() = fareMarket;
    fareCompInfo->fareCompNumber() = perm.processTags().size() + 1;
    pInfo->fareCompInfo() = fareCompInfo;

    itin.fareComponent().push_back(fareCompInfo);

    perm.processTags().push_back(pInfo);
  }

  void addFareUsage(PricingUnit& pu, LocCode board, LocCode off, bool unflown)
  {
    FareUsage* fu = create<FareUsage>();
    PaxTypeFare* pax = create<PaxTypeFare>();
    FareMarket* fm = create<FareMarket>();

    pu.fareUsage().push_back(fu);
    fu->paxTypeFare() = pax;
    pax->fareMarket() = fm;
    fm->boardMultiCity() = board;
    fm->offMultiCity() = off;

    addAirSegment(fu->travelSeg(), board, off, board, off, unflown);
    fm->travelSeg().push_back(fu->travelSeg().front());
  }

  void initPaxTypeFare(PaxTypeFare& ptf, const TariffCategory& tariff)
  {
    TariffCrossRefInfo* crossRefInf = create<TariffCrossRefInfo>();
    crossRefInf->tariffCat() = tariff;
    Fare* fare = create<Fare>();
    fare->setTariffCrossRefInfo(crossRefInf);
    ptf.initialize(fare, NULL, NULL);
  }

  PaxTypeFare* PTF(FareMarket* fm)
  {
    createPaxTypeFareWithFareAndFareInfo();
    _ptf->fareMarket() = fm;

    return _ptf;
  }

  FareMarket* FM(TravelSeg* p1, TravelSeg* p2 = NULL, TravelSeg* p3 = NULL)
  {
    FareMarket* fm = create<FareMarket>();
    if (p1)
      fm->travelSeg().push_back(p1);
    if (p2)
      fm->travelSeg().push_back(p2);
    if (p3)
      fm->travelSeg().push_back(p3);
    fm->setFCChangeStatus(-1);

    return fm;
  }

  FareCompInfo* FC(FareMarket* fm, uint16_t fareCompNumber = 0)
  {
    FareCompInfo* fc = create<FareCompInfo>();
    fc->fareCompNumber() = fareCompNumber;
    fc->fareMarket() = fm;

    return fc;
  }

  FareUsage* FU(TravelSeg* p1, TravelSeg* p2 = NULL, TravelSeg* p3 = NULL)
  {
    FareUsage* fu = create<FareUsage>();
    if (p1)
      fu->travelSeg().push_back(p1);
    if (p2)
      fu->travelSeg().push_back(p2);
    if (p3)
      fu->travelSeg().push_back(p3);

    return fu;
  }

  FareUsage* FU(PaxTypeFare* p1, TravelSeg* p2 = NULL, TravelSeg* p3 = NULL, TravelSeg* p4 = NULL)
  {
    FareUsage* fu = create<FareUsage>();
    fu->paxTypeFare() = p1;
    if (p2)
      fu->travelSeg().push_back(p2);
    if (p3)
      fu->travelSeg().push_back(p3);
    if (p4)
      fu->travelSeg().push_back(p4);

    return fu;
  }

  PricingUnit* PU(FareUsage* p1, FareUsage* p2 = NULL)
  {
    PricingUnit* pu = create<PricingUnit>();
    if (p1)
      pu->fareUsage().push_back(p1);
    if (p2)
      pu->fareUsage().push_back(p2);

    return pu;
  }

  FarePath* FP(FarePath* fp,
               PricingUnit* p1,
               PricingUnit* p2 = NULL,
               PricingUnit* p3 = NULL,
               PricingUnit* p4 = NULL)
  {
    if (p1)
      fp->pricingUnit().push_back(p1);
    if (p2)
      fp->pricingUnit().push_back(p2);
    if (p3)
      fp->pricingUnit().push_back(p3);
    if (p4)
      fp->pricingUnit().push_back(p4);

    return fp;
  }

  Fare* FA(const FareClassCode& fc,
           const CurrencyCode& cc,
           const MoneyAmount& ma,
           const VendorCode& vc,
           const CarrierCode& cr,
           const TariffNumber& ft,
           const RuleNumber& rn,
           const Directionality& dr)
  {
    FareInfo* fi = create<FareInfo>();
    fi->fareClass() = fc;
    fi->currency() = cc;
    fi->originalFareAmount() = ma;
    fi->vendor() = vc;
    fi->carrier() = cr;
    fi->fareTariff() = ft;
    fi->ruleNumber() = rn;
    fi->directionality() = dr;

    Fare* f = create<Fare>();
    f->setFareInfo(fi);
    return f;
  }

  FareMarket::RetrievalInfo* RI(const DateTime& date, const FareMarket::FareRetrievalFlags& flag)
  {
    FareMarket::RetrievalInfo* ri = create<FareMarket::RetrievalInfo>();
    ri->_date = date;
    ri->_flag = flag;
    return ri;
  }

  ReissueSequence* RS(const Indicator& rtl)
  {
    ReissueSequence* rs = create<ReissueSequence>();
    rs->reissueToLower() = rtl;
    return rs;
  }

  void addAirSegment(vector<TravelSeg*>& tvlSeg,
                     const LocCode& board,
                     const LocCode& off,
                     const LocCode& boardMCity = "",
                     const LocCode& offMCity = "",
                     bool unflown = false)
  {
    AirSeg* seg = create<AirSeg>();
    seg->unflown() = unflown;
    seg->origAirport() = board;
    seg->destAirport() = off;
    seg->segmentType() = Air;
    if (boardMCity == "")
    {
      seg->boardMultiCity() = board;
      seg->offMultiCity() = off;
    }
    else
    {
      seg->boardMultiCity() = boardMCity;
      seg->offMultiCity() = offMCity;
    }
    seg->segmentOrder() = tvlSeg.size() + 1;
    seg->pnrSegment() = tvlSeg.size() + 1;
    tvlSeg.push_back(seg);
  }

  void addSegment(vector<TravelSeg*>* tvlSeg,
                  const LocCode& board,
                  const LocCode& off,
                  bool stopOver = false,
                  int flightNo = 123,
                  const CarrierCode& cxr = CARRIER_AA,
                  const LocCode& boardMCity = "",
                  const LocCode& offMCity = "",
                  const TravelSeg::ChangeStatus changeStatus = TravelSeg::CHANGED,
                  bool unflown = true,
                  const string& depDateTime = "2007-07-07 07:07",
                  const TravelSegType segType = Air)
  {
    AirSeg* seg = create<AirSeg>();

    seg->origAirport() = board;
    seg->destAirport() = off;
    seg->segmentType() = segType;
    seg->stopOver() = stopOver;
    seg->carrier() = cxr;
    if (segType == Air)
      seg->flightNumber() = flightNo;
    seg->unflown() = unflown;
    if (!depDateTime.empty())
      seg->departureDT() = DateTime(const_cast<string&>(depDateTime));

    if (boardMCity == "")
    {
      seg->boardMultiCity() = board;
      seg->offMultiCity() = off;
    }
    else
    {
      seg->boardMultiCity() = boardMCity;
      seg->offMultiCity() = offMCity;
    }
    seg->changeStatus() = changeStatus;
    seg->pnrSegment() = tvlSeg->size() + 1;
    tvlSeg->push_back(seg);
  }

  ProcessTagInfo*
  createProcTagInfo(FareCompInfo* fc,
                    const Indicator extendInd = RepriceSolutionValidator::EXTEND_NO_RESTRICTIONS)
  {
    ReissueSequence* seq = create<ReissueSequence>();
    seq->extendInd() = extendInd;

    ProcessTagInfo* tag = create<ProcessTagInfo>();
    tag->reissueSequence()->orig() = seq;

    tag->fareCompInfo() = fc;

    return tag;
  }

  FarePath* FP(PaxTypeFare* ptf1, PaxTypeFare* ptf2, PaxTypeFare* ptf3, PaxTypeFare* ptf4)
  {
    FarePath* fp = create<FarePath>();
    Itin* itin = create<Itin>();
    fp->itin() = itin;

    return FP(fp, PU(FU(ptf1), FU(ptf2)), PU(FU(ptf3), FU(ptf4)));
  }

  PaxTypeFare* PaxTypeFareWithOWRT(Indicator owrt)
  {
    PaxTypeFare* ptf = PTF(NULL);
    Fare* fare = create<Fare>();
    FareInfo* fareInfo = create<FareInfo>();
    fareInfo->owrt() = owrt;

    fare->setFareInfo(fareInfo);
    ptf->setFare(fare);

    return ptf;
  }

  std::vector<ProcessTagInfo*>* createProcessTagInfoVec()
  {
    std::vector<ProcessTagInfo*>* processTags;
    processTags = create<std::vector<ProcessTagInfo*>>();
    processTags->push_back(create<ProcessTagInfo>());
    return processTags;
  }

  // end helpers

  // tests------------------------------------------------------------------------------------------------------

  void testFullyFlownNoRestrictionPass()
  {
    _permutation->processTags().push_back(createProcTagInfo(
        FC(FM(_ffExchangeItin->travelSeg()[0])), RepriceSolutionValidator::EXTEND_NO_RESTRICTIONS));

    CPPUNIT_ASSERT(_validator->checkFullyFlown(*_permutation));
  }

  void testFullyFlownOnePoint_OneExcToOneNewPass()
  {
    const FareMarket* fm = FM(_ffNewItin->travelSeg()[0]);

    ProcessTagInfo* tag = createProcTagInfo(FC(FM(_ffExchangeItin->travelSeg()[0])),
                                            RepriceSolutionValidator::EXTEND_AT_LEAST_ONE_POINT);

    CPPUNIT_ASSERT(_validator->checkFullyFlownOnePoint(*tag, *fm));
  }

  void testFullyFlownOnePoint_OneExcToTwoNewPass()
  {
    const FareMarket* fm1 = FM(_ffNewItin->travelSeg()[0]);
    const FareMarket* fm2 = FM(_ffNewItin->travelSeg()[1]);

    ProcessTagInfo* tag =
        createProcTagInfo(FC(FM(_ffExchangeItin->travelSeg()[0], _ffExchangeItin->travelSeg()[1])),
                          RepriceSolutionValidator::EXTEND_AT_LEAST_ONE_POINT);

    CPPUNIT_ASSERT(_validator->checkFullyFlownOnePoint(*tag, *fm1));
    CPPUNIT_ASSERT(_validator->checkFullyFlownOnePoint(*tag, *fm2));
  }

  void testFullyFlownOnePoint_OneExcToThreeNewFail()
  {
    const FareMarket* fm1 = FM(_ffNewItin->travelSeg()[0]);
    const FareMarket* fm2 = FM(_ffNewItin->travelSeg()[1]);
    const FareMarket* fm3 = FM(_ffNewItin->travelSeg()[2]);

    ProcessTagInfo* tag = createProcTagInfo(FC(FM(_ffExchangeItin->travelSeg()[0],
                                                  _ffExchangeItin->travelSeg()[1],
                                                  _ffExchangeItin->travelSeg()[2])),
                                            RepriceSolutionValidator::EXTEND_AT_LEAST_ONE_POINT);

    CPPUNIT_ASSERT(_validator->checkFullyFlownOnePoint(*tag, *fm1));
    CPPUNIT_ASSERT(!_validator->checkFullyFlownOnePoint(*tag, *fm2));
    CPPUNIT_ASSERT(_validator->checkFullyFlownOnePoint(*tag, *fm3));
  }

  void testFullyFlownOnePoint_TwoExcToOneNewPass()
  {
    const FareMarket* fm = FM(_ffNewItin->travelSeg()[0], _ffNewItin->travelSeg()[1]);

    ProcessTagInfo* tag = createProcTagInfo(FC(FM(_ffExchangeItin->travelSeg()[0])),
                                            RepriceSolutionValidator::EXTEND_AT_LEAST_ONE_POINT);

    CPPUNIT_ASSERT(_validator->checkFullyFlownOnePoint(*tag, *fm));
  }

  void testFullyFlownOnePoint_ThreeExcToOneNewPass()
  {
    const FareMarket* fm =
        FM(_ffNewItin->travelSeg()[0], _ffNewItin->travelSeg()[1], _ffNewItin->travelSeg()[2]);

    ProcessTagInfo* tag = createProcTagInfo(FC(FM(_ffExchangeItin->travelSeg()[1])),
                                            RepriceSolutionValidator::EXTEND_AT_LEAST_ONE_POINT);

    CPPUNIT_ASSERT(_validator->checkFullyFlownOnePoint(*tag, *fm));
  }

  void testFullyFlownTheSameOldAndNewFCsPass()
  {
    uint16_t fareCompNumber = 1;
    _trx->exchangeItin().clear();
    _trx->exchangeItin().push_back(_ffExchangeItin);

    Itin newItin;
    addAirSegment(newItin.travelSeg(), "KRK", "LON", "KRK", "LON", false);
    _trx->newItin().push_back(&newItin);

    FareCompInfo* fc1 = FC(FM(_ffExchangeItin->travelSeg()[0]), fareCompNumber++);
    FareCompInfo* fc2 = FC(FM(_ffExchangeItin->travelSeg()[1]), fareCompNumber++);
    FareCompInfo* fc3 =
        FC(FM(_ffExchangeItin->travelSeg()[2], _ffExchangeItin->travelSeg()[3]), fareCompNumber++);
    FareCompInfo* fc4 = FC(FM(_ffExchangeItin->travelSeg()[4]), fareCompNumber++);

    _permutation->processTags().push_back(
        createProcTagInfo(fc1, RepriceSolutionValidator::EXTEND_AT_LEAST_ONE_POINT));
    _ffExchangeItin->fareComponent().push_back(fc1);
    _permutation->processTags().push_back(
        createProcTagInfo(fc2, RepriceSolutionValidator::EXTEND_AT_LEAST_ONE_POINT));
    _ffExchangeItin->fareComponent().push_back(fc2);
    _permutation->processTags().push_back(
        createProcTagInfo(fc3, RepriceSolutionValidator::EXTEND_AT_LEAST_ONE_POINT));
    _ffExchangeItin->fareComponent().push_back(fc3);
    _permutation->processTags().push_back(
        createProcTagInfo(fc4, RepriceSolutionValidator::EXTEND_AT_LEAST_ONE_POINT));
    _ffExchangeItin->fareComponent().push_back(fc4);
    _validator->_fcMapping.resize(_trx->exchangeItin().front()->fareComponent().size());
    _validator->_fmMapping.resize(_trx->exchangeItin().front()->fareComponent().size());

    FareMarket* fm3 = FM(_ffNewItin->travelSeg()[2]);
    FareMarket* fm4 = FM(_ffNewItin->travelSeg()[3]);

    FarePath* fp =
        FP(_farePath,
           PU(FU(PTF(FM(_ffNewItin->travelSeg()[0]))), FU(PTF(fm4), _ffNewItin->travelSeg()[3])),
           PU(FU(PTF(FM(_ffNewItin->travelSeg()[1]))), FU(PTF(fm3), _ffNewItin->travelSeg()[2])),
           PU(FU(PTF(FM(_ffNewItin->travelSeg()[4])))));
    fp->itin() = _ffNewItin;

    _validator->analyseFarePath();

    CPPUNIT_ASSERT(_validator->_fmMapping[2].size() == 2);
    CPPUNIT_ASSERT(_validator->_fmMapping[2][0] == fm4);
    CPPUNIT_ASSERT(_validator->_fmMapping[2][1] == fm3);

    CPPUNIT_ASSERT(_validator->checkFullyFlown(*_permutation));
  }

  void testFullyFlownAtLeastOnePoint_OneToMany()
  {
    uint16_t fareCompNumber = 1;
    _trx->exchangeItin().clear();
    _trx->exchangeItin().push_back(_ffExchangeItin);

    Itin newItin;
    addAirSegment(newItin.travelSeg(), "KRK", "LON", "KRK", "LON", false);
    _trx->newItin().clear();
    _trx->newItin().push_back(&newItin);

    _ffExchangeItin->fareComponent().push_back(
        FC(FM(_ffExchangeItin->travelSeg()[0]), fareCompNumber++));
    _ffExchangeItin->fareComponent().push_back(
        FC(FM(_ffExchangeItin->travelSeg()[1]), fareCompNumber++));
    _ffExchangeItin->fareComponent().push_back(
        FC(FM(_ffExchangeItin->travelSeg()[2]), fareCompNumber++));
    _validator->_fcMapping.resize(_trx->exchangeItin().front()->fareComponent().size());
    _validator->_fmMapping.resize(_trx->exchangeItin().front()->fareComponent().size());

    FareMarket* fm1 = FM(_ffNewItin->travelSeg()[0], _ffNewItin->travelSeg()[1]);
    FareMarket* fm2 = FM(_ffNewItin->travelSeg()[2], _ffNewItin->travelSeg()[3]);

    FarePath* fp = FP(_farePath, PU(FU(PTF(fm1))), PU(FU(PTF(fm2))));
    fp->itin() = _ffNewItin;

    initValidator();

    _validator->analyseFarePath();

    CPPUNIT_ASSERT(_validator->_fmMapping[0].size() == 1);
    CPPUNIT_ASSERT(_validator->_fmMapping[0][0] == fm1);

    CPPUNIT_ASSERT(_validator->_fmMapping[1].size() == 1);
    CPPUNIT_ASSERT(_validator->_fmMapping[1][0] == fm1);

    CPPUNIT_ASSERT(_validator->_fmMapping[2].size() == 1);
    CPPUNIT_ASSERT(_validator->_fmMapping[2][0] == fm2);
  }

  void testFullyFlownAtLeastOnePoint_ManyToOne()
  {
    uint16_t fareCompNumber = 1;
    _trx->exchangeItin().clear();
    _trx->exchangeItin().push_back(_ffExchangeItin);

    Itin newItin;
    addAirSegment(newItin.travelSeg(), "KRK", "LON", "KRK", "LON", false);
    _trx->newItin().clear();
    _trx->newItin().push_back(&newItin);

    _ffExchangeItin->fareComponent().push_back(
        FC(FM(_ffExchangeItin->travelSeg()[0]), fareCompNumber++));
    _ffExchangeItin->fareComponent().push_back(FC(FM(_ffExchangeItin->travelSeg()[1],
                                                     _ffExchangeItin->travelSeg()[2],
                                                     _ffExchangeItin->travelSeg()[3]),
                                                  fareCompNumber++));
    _ffExchangeItin->fareComponent().push_back(
        FC(FM(_ffExchangeItin->travelSeg()[4]), fareCompNumber++));
    _validator->_fcMapping.resize(_trx->exchangeItin().front()->fareComponent().size());
    _validator->_fmMapping.resize(_trx->exchangeItin().front()->fareComponent().size());

    FareMarket* fm1 = FM(_ffNewItin->travelSeg()[0], _ffNewItin->travelSeg()[1]);
    FareMarket* fm2 = FM(_ffNewItin->travelSeg()[2]);
    FareMarket* fm3 = FM(_ffNewItin->travelSeg()[3], _ffNewItin->travelSeg()[4]);

    FarePath* fp = FP(_farePath, PU(FU(PTF(fm1))), PU(FU(PTF(fm2))), PU(FU(PTF(fm3))));
    fp->itin() = _ffNewItin;

    initValidator();

    _validator->analyseFarePath();

    CPPUNIT_ASSERT(_validator->_fmMapping[0].size() == 1);
    CPPUNIT_ASSERT(_validator->_fmMapping[0][0] == fm1);

    CPPUNIT_ASSERT(_validator->_fmMapping[1].size() == 3);
    CPPUNIT_ASSERT(_validator->_fmMapping[1][0] == fm1);
    CPPUNIT_ASSERT(_validator->_fmMapping[1][1] == fm2);
    CPPUNIT_ASSERT(_validator->_fmMapping[1][2] == fm3);

    CPPUNIT_ASSERT(_validator->_fmMapping[2].size() == 1);
    CPPUNIT_ASSERT(_validator->_fmMapping[2][0] == fm3);
  }

  void testFullyFlownDestOnly_OneExcToOneNewPass()
  {
    _trx->exchangeItin().clear();
    _trx->exchangeItin().push_back(_ffExchangeItin);
    _farePath->itin() = _ffNewItin;

    const FareMarket* fm = FM(_ffNewItin->travelSeg()[0], _ffNewItin->travelSeg()[1]);

    ProcessTagInfo* tag = createProcTagInfo(FC(FM(_ffExchangeItin->travelSeg()[0])),
                                            RepriceSolutionValidator::EXTEND_BEYOND_DEST_ONLY);

    CPPUNIT_ASSERT(_validator->checkFullyFlownDestOnly(*tag, *fm));
  }

  void testFullyFlownDestOnly_OneExcToTwoNewPass()
  {
    _trx->exchangeItin().clear();
    _trx->exchangeItin().push_back(_ffExchangeItin);
    _farePath->itin() = _ffNewItin;

    const FareMarket* fm1 = FM(_ffNewItin->travelSeg()[0]);
    const FareMarket* fm2 = FM(_ffNewItin->travelSeg()[1], _ffNewItin->travelSeg()[2]);

    ProcessTagInfo* tag =
        createProcTagInfo(FC(FM(_ffExchangeItin->travelSeg()[0], _ffExchangeItin->travelSeg()[1])),
                          RepriceSolutionValidator::EXTEND_BEYOND_DEST_ONLY);

    CPPUNIT_ASSERT(_validator->checkFullyFlownDestOnly(*tag, *fm1));
    CPPUNIT_ASSERT(_validator->checkFullyFlownDestOnly(*tag, *fm2));
  }

  void testFullyFlownDestOnly_OneExcToTwoNewFail()
  {
    _trx->exchangeItin().clear();
    _trx->exchangeItin().push_back(_ffExchangeItin);
    _farePath->itin() = _ffNewItin;

    const FareMarket* fm =
        FM(_ffNewItin->travelSeg()[0], _ffNewItin->travelSeg()[1], _ffNewItin->travelSeg()[2]);

    ProcessTagInfo* tag =
        createProcTagInfo(FC(FM(_ffExchangeItin->travelSeg()[1], _ffExchangeItin->travelSeg()[2])),
                          RepriceSolutionValidator::EXTEND_BEYOND_DEST_ONLY);

    CPPUNIT_ASSERT(!_validator->checkFullyFlownDestOnly(*tag, *fm));
  }

  void testFullyFlownDestOnly_OneExcToThreeNewPass()
  {
    _trx->exchangeItin().clear();
    _trx->exchangeItin().push_back(_ffExchangeItin);
    _farePath->itin() = _ffNewItin;

    const FareMarket* fm1 = FM(_ffNewItin->travelSeg()[0]);
    const FareMarket* fm2 =
        FM(_ffNewItin->travelSeg()[1], _ffNewItin->travelSeg()[2], _ffNewItin->travelSeg()[3]);

    ProcessTagInfo* tag = createProcTagInfo(FC(FM(_ffExchangeItin->travelSeg()[0],
                                                  _ffExchangeItin->travelSeg()[1],
                                                  _ffExchangeItin->travelSeg()[2])),
                                            RepriceSolutionValidator::EXTEND_BEYOND_DEST_ONLY);

    CPPUNIT_ASSERT(_validator->checkFullyFlownDestOnly(*tag, *fm1));
    CPPUNIT_ASSERT(_validator->checkFullyFlownDestOnly(*tag, *fm2));
  }

  void testFullyFlownDestOnly_OneExcToThreeNewFail()
  {
    _trx->exchangeItin().clear();
    _trx->exchangeItin().push_back(_ffExchangeItin);
    _farePath->itin() = _ffNewItin;

    const FareMarket* fm1 = FM(_ffNewItin->travelSeg()[0], _ffNewItin->travelSeg()[1]);
    const FareMarket* fm2 = FM(_ffNewItin->travelSeg()[2], _ffNewItin->travelSeg()[3]);

    ProcessTagInfo* tag = createProcTagInfo(FC(FM(_ffExchangeItin->travelSeg()[1],
                                                  _ffExchangeItin->travelSeg()[2],
                                                  _ffExchangeItin->travelSeg()[3])),
                                            RepriceSolutionValidator::EXTEND_BEYOND_DEST_ONLY);

    CPPUNIT_ASSERT(!_validator->checkFullyFlownDestOnly(*tag, *fm1));
    CPPUNIT_ASSERT(_validator->checkFullyFlownDestOnly(*tag, *fm2));
  }

  void testFullyFlownDestOnly_TwoExcToOneNewPass()
  {
    _trx->exchangeItin().clear();
    _trx->exchangeItin().push_back(_ffExchangeItin);
    _farePath->itin() = _ffNewItin;

    const FareMarket* fm = FM(_ffNewItin->travelSeg()[0], _ffNewItin->travelSeg()[1]);

    ProcessTagInfo* tag = createProcTagInfo(FC(FM(_ffExchangeItin->travelSeg()[0])),
                                            RepriceSolutionValidator::EXTEND_BEYOND_DEST_ONLY);

    CPPUNIT_ASSERT(_validator->checkFullyFlownDestOnly(*tag, *fm));
  }

  void testFullyFlownDestOnly_ThreeExcToOneNewPass()
  {
    _trx->exchangeItin().clear();
    _trx->exchangeItin().push_back(_ffExchangeItin);
    _farePath->itin() = _ffNewItin;

    const FareMarket* fm1 = FM(_ffNewItin->travelSeg()[0]);
    const FareMarket* fm2 =
        FM(_ffNewItin->travelSeg()[1], _ffNewItin->travelSeg()[2], _ffNewItin->travelSeg()[3]);

    ProcessTagInfo* tag =
        createProcTagInfo(FC(FM(_ffExchangeItin->travelSeg()[0], _ffExchangeItin->travelSeg()[1])),
                          RepriceSolutionValidator::EXTEND_BEYOND_DEST_ONLY);

    CPPUNIT_ASSERT(_validator->checkFullyFlownDestOnly(*tag, *fm1));
    CPPUNIT_ASSERT(_validator->checkFullyFlownDestOnly(*tag, *fm2));
  }

  void testFullyFlownDestOnly_ThreeExcToTwoNewFail()
  {
    _trx->exchangeItin().clear();
    _trx->exchangeItin().push_back(_ffExchangeItin);
    _farePath->itin() = _ffNewItin;

    const FareMarket* fm =
        FM(_ffNewItin->travelSeg()[0], _ffNewItin->travelSeg()[1], _ffNewItin->travelSeg()[2]);

    ProcessTagInfo* tag = createProcTagInfo(FC(FM(_ffExchangeItin->travelSeg()[1])),
                                            RepriceSolutionValidator::EXTEND_BEYOND_DEST_ONLY);

    CPPUNIT_ASSERT(!_validator->checkFullyFlownDestOnly(*tag, *fm));
  }

  void testFullyFlownNoChange_OneExcToOneNewPass()
  {
    _trx->exchangeItin().clear();
    _trx->exchangeItin().push_back(_ffExchangeItin);
    _farePath->itin() = _ffNewItin;

    const FareMarket* fm = FM(_ffNewItin->travelSeg()[0]);

    ProcessTagInfo* tag = createProcTagInfo(FC(FM(_ffExchangeItin->travelSeg()[0])),
                                            RepriceSolutionValidator::EXTEND_NO_CHANGE);

    CPPUNIT_ASSERT(_validator->checkFullyFlownNoChange(*tag, *fm));
  }

  void testFullyFlownNoChange_TwoExcToTwoNewPass()
  {
    _trx->exchangeItin().clear();
    _trx->exchangeItin().push_back(_ffExchangeItin);
    _farePath->itin() = _ffNewItin;

    const FareMarket* fm = FM(_ffNewItin->travelSeg()[0], _ffNewItin->travelSeg()[1]);

    ProcessTagInfo* tag =
        createProcTagInfo(FC(FM(_ffExchangeItin->travelSeg()[0], _ffExchangeItin->travelSeg()[1])),
                          RepriceSolutionValidator::EXTEND_NO_CHANGE);

    CPPUNIT_ASSERT(_validator->checkFullyFlownNoChange(*tag, *fm));
  }

  void testFullyFlownNoChange_OneExcToTwoNewFail()
  {
    _trx->exchangeItin().clear();
    _trx->exchangeItin().push_back(_ffExchangeItin);
    _farePath->itin() = _ffNewItin;

    const FareMarket* fm = FM(_ffNewItin->travelSeg()[1]);

    ProcessTagInfo* tag =
        createProcTagInfo(FC(FM(_ffExchangeItin->travelSeg()[0], _ffExchangeItin->travelSeg()[1])),
                          RepriceSolutionValidator::EXTEND_NO_CHANGE);

    CPPUNIT_ASSERT(!_validator->checkFullyFlownNoChange(*tag, *fm));
  }

  void testFullyFlownNoChange_OneExcToThreeNewFail()
  {
    _trx->exchangeItin().clear();
    _trx->exchangeItin().push_back(_ffExchangeItin);
    _farePath->itin() = _ffNewItin;

    const FareMarket* fm1 = FM(_ffNewItin->travelSeg()[0]);
    const FareMarket* fm2 = FM(_ffNewItin->travelSeg()[1], _ffNewItin->travelSeg()[2]);

    ProcessTagInfo* tag = createProcTagInfo(FC(FM(_ffExchangeItin->travelSeg()[0],
                                                  _ffExchangeItin->travelSeg()[1],
                                                  _ffExchangeItin->travelSeg()[2])),
                                            RepriceSolutionValidator::EXTEND_NO_CHANGE);

    CPPUNIT_ASSERT(!_validator->checkFullyFlownNoChange(*tag, *fm1));
    CPPUNIT_ASSERT(!_validator->checkFullyFlownNoChange(*tag, *fm2));
  }

  void testFullyFlownNoChange_TwoExcToOneNewFail()
  {
    _trx->exchangeItin().clear();
    _trx->exchangeItin().push_back(_ffExchangeItin);
    _farePath->itin() = _ffNewItin;

    const FareMarket* fm = FM(_ffNewItin->travelSeg()[0], _ffNewItin->travelSeg()[1]);

    ProcessTagInfo* tag = createProcTagInfo(FC(FM(_ffExchangeItin->travelSeg()[0])),
                                            RepriceSolutionValidator::EXTEND_NO_CHANGE);

    CPPUNIT_ASSERT(!_validator->checkFullyFlownNoChange(*tag, *fm));
  }

  void testFullyFlownNoChange_ThreeExcToOneNewFail()
  {
    _trx->exchangeItin().clear();
    _trx->exchangeItin().push_back(_ffExchangeItin);
    _farePath->itin() = _ffNewItin;

    const FareMarket* fm =
        FM(_ffNewItin->travelSeg()[0], _ffNewItin->travelSeg()[1], _ffNewItin->travelSeg()[2]);

    ProcessTagInfo* tag = createProcTagInfo(FC(FM(_ffExchangeItin->travelSeg()[1])),
                                            RepriceSolutionValidator::EXTEND_NO_CHANGE);

    CPPUNIT_ASSERT(!_validator->checkFullyFlownNoChange(*tag, *fm));
  }

  void testCheckSamePointTableForTermBytePass()
  {
    string date = "2007-12-20 08:25";
    _trx->setOriginalTktIssueDT() = DateTime(date);
    _trx->exchangeItin().push_back(_exchangeItin);

    Itin newItin;
    addAirSegment(newItin.travelSeg(), "KRK", "LON", "KRK", "LON", false);
    addAirSegment(newItin.travelSeg(), "LON", "WAW", "LON", "WAW", false);
    addAirSegment(newItin.travelSeg(), "FRA", "GLA", "FRA", "GLA", false);
    _trx->newItin().push_back(&newItin);

    // permutation and old FarePath
    addPermutationData(
        *_permutation, *_exchangeItin, KEEP_THE_FARES, 222, 222, ' ', 222, "", "FRA", "PAR");
    //

    // new FarePath
    PricingUnit pu;
    _farePath->pricingUnit().push_back(&pu);
    addFareUsage(pu, "KRK", "LON", false);
    addFareUsage(pu, "LON", "WAW", false);
    addFareUsage(pu, "FRA", "GLA", false);
    //

    // pu.fareUsage()[0].paxTypeFare()[0].fareMarket()->travelSeg().push_back(newItin->travelSeg()[0]);

    RepriceSolutionValidatorOverride* rvo =
        dynamic_cast<RepriceSolutionValidatorOverride*>(_validator);
    rvo->_samePointVector.clear();
    rvo->analyseFarePath();

    // Same point table data
    SamePoint sp, sp2;
    sp.mkt1() = "PAR";
    sp.mkt2() = "GLA";
    rvo->_samePointVector.push_back(&sp);

    sp2.mkt1() = "WAW";
    sp2.mkt2() = "MOW";
    rvo->_samePointVector.push_back(&sp2);
    //

    CPPUNIT_ASSERT(rvo->checkSamePointTable(*(_permutation->processTags().front())));
  }

  void testCheckSamePointTableForTermByteFail()
  {
    string date = "2007-12-20 08:25";
    _trx->setOriginalTktIssueDT() = DateTime(date);

    Itin newItin;
    addAirSegment(newItin.travelSeg(), "KRK", "LON", "KRK", "LON", false);
    addAirSegment(newItin.travelSeg(), "LON", "WAW", "LON", "WAW", false);
    addAirSegment(newItin.travelSeg(), "FRA", "GLA", "FRA", "GLA", false);
    _trx->newItin().push_back(&newItin);

    // permutation and old FarePath
    addPermutationData(
        *_permutation, *_exchangeItin, KEEP_THE_FARES, 222, 222, ' ', 222, "", "FRA", "PAR");
    //

    // new FarePath
    PricingUnit pu;
    _farePath->pricingUnit().push_back(&pu);

    addFareUsage(pu, "KRK", "LON", false);
    addFareUsage(pu, "LON", "WAW", false);
    addFareUsage(pu, "FRA", "GLA", false);
    //

    RepriceSolutionValidatorOverride* rvo =
        dynamic_cast<RepriceSolutionValidatorOverride*>(_validator);
    rvo->_samePointVector.clear();
    rvo->analyseFarePath();

    // Same point table data
    SamePoint sp, sp2;
    sp.mkt1() = "PAR";
    sp.mkt2() = "KRK";
    rvo->_samePointVector.push_back(&sp);

    sp2.mkt1() = "WAW";
    sp2.mkt2() = "MOW";
    rvo->_samePointVector.push_back(&sp2);
    //

    CPPUNIT_ASSERT(!rvo->checkSamePointTable(*(_permutation->processTags().front())));
  }

  void testCheckSamePointTableForFirstFareBytePass()
  {
    string date = "2007-12-20 08:25";
    _trx->setOriginalTktIssueDT() = DateTime(date);

    addPermutationData(
        *_permutation, *_exchangeItin, KEEP_THE_FARES, 222, 222, ' ', 222, "", "FRA", "PAR");

    RepriceSolutionValidatorOverride* rvo =
        dynamic_cast<RepriceSolutionValidatorOverride*>(_validator);

    // first _fare
    PaxTypeFare paxType;
    FareMarket fareMarket;
    fareMarket.boardMultiCity() = "FRA";
    fareMarket.offMultiCity() = "WAW";
    paxType.fareMarket() = &fareMarket;
    rvo->_firstFare = &paxType;
    //

    // Same point table data
    SamePoint sp;
    sp.mkt1() = "PAR";
    sp.mkt2() = "WAW";
    rvo->_samePointVector.push_back(&sp);
    //

    CPPUNIT_ASSERT(rvo->checkSamePointTable(*(_permutation->processTags().front()), true));
  }

  void testCheckSamePointTableForFirstFareByteFail()
  {
    string date = "2007-12-20 08:25";
    _trx->setOriginalTktIssueDT() = DateTime(date);

    addPermutationData(
        *_permutation, *_exchangeItin, KEEP_THE_FARES, 222, 222, ' ', 222, "", "FRA", "PAR");

    RepriceSolutionValidatorOverride* rvo =
        dynamic_cast<RepriceSolutionValidatorOverride*>(_validator);

    // first _fare
    PaxTypeFare paxType;
    FareMarket fareMarket;
    fareMarket.boardMultiCity() = "FRA";
    fareMarket.offMultiCity() = "WAW";
    paxType.fareMarket() = &fareMarket;
    rvo->_firstFare = &paxType;
    //

    // Same point table data
    SamePoint sp;
    sp.mkt1() = "PAR";
    sp.mkt2() = "MOW";
    rvo->_samePointVector.push_back(&sp);
    //

    CPPUNIT_ASSERT(!rvo->checkSamePointTable(*(_permutation->processTags().front()), true));
  }
  /*
    void testSetJourneyCheckingFlagPass()
    {
    PricingUnit pu1, pu2;
    FareUsage fu1, fu2, fu3, fu4, fu5, fu6;

    //old OW fully flown FC's
    addAirSegment(fu1.travelSeg(),"KTW", "WAW","KTW", "WAW", false);
    addAirSegment(fu1.travelSeg(),"MOW", "KRK","MOW", "KRK", false);
    addAirSegment(fu1.travelSeg(),"KRK", "FRA","KRK", "FRA", false);

    _exchangeItin->flownOneWayFares().push_back(&fu1);

    addAirSegment(fu2.travelSeg(),"NYC", "DFW","NYC", "DFW", false);
    addAirSegment(fu2.travelSeg(),"DFW", "NYC","DFW", "NYC", false);
    addAirSegment(fu2.travelSeg(),"NYC", "DFW","NYC", "DFW", false);

    _exchangeItin->flownOneWayFares().push_back(&fu2);
    //

    //new
    _farePath->pricingUnit().push_back(&pu1);

    pu1.puType() = PricingUnit::Type::ONEWAY;
    pu1.fareUsage().push_back(&fu3);
    addAirSegment(fu3.travelSeg(),"KTW", "WAW","KTW", "WAW", false);
    addAirSegment(fu3.travelSeg(),"MOW", "KRK","MOW", "KRK", false);
    addAirSegment(fu3.travelSeg(),"KRK", "FRA","KRK", "FRA", false);

    pu1.fareUsage().push_back(&fu4);
    addAirSegment(fu4.travelSeg(),"MOW", "KRK","MOW", "KRK", false);
    addAirSegment(fu4.travelSeg(),"GLA", "LON","GLA", "LON", false);
    addAirSegment(fu4.travelSeg(),"LON", "KRK","LON", "KRK", true);

    _farePath->pricingUnit().push_back(&pu2);

    pu2.puType() = PricingUnit::Type::ROUNDTRIP;
    pu2.fareUsage().push_back(&fu5);
    addAirSegment(fu5.travelSeg(),"LON", "GLA","LON", "GLA", false);
    addAirSegment(fu5.travelSeg(),"GLA", "KRK","GLA", "KRK", false);
    addAirSegment(fu5.travelSeg(),"KRK", "WAW","KRK", "WAW", true);

    pu2.fareUsage().push_back(&fu6);
    addAirSegment(fu6.travelSeg(),"NYC", "DFW","NYC", "DFW", false);
    addAirSegment(fu6.travelSeg(),"DFW", "NYC","DFW", "NYC", false);
    addAirSegment(fu6.travelSeg(),"NYC", "DFW","NYC", "DFW", false);
    //

    CPPUNIT_ASSERT(_validator->setJourneyCheckingFlag(*_farePath));
    }

    void testSetJourneyCheckingFlagFail()
    {
    PricingUnit pu1, pu2;
    FareUsage fu1, fu2, fu3, fu4, fu5;

    //old OW fully flown FC
    addAirSegment(fu1.travelSeg(),"KTW", "WAW","KTW", "WAW", false);
    addAirSegment(fu1.travelSeg(),"MOW", "KRK","MOW", "KRK", false);
    addAirSegment(fu1.travelSeg(),"KRK", "FRA","KRK", "FRA", false);

    _exchangeItin->flownOneWayFares().push_back(&fu1);
    //

    //new
    _farePath->pricingUnit().push_back(&pu1);

    pu1.puType() = PricingUnit::Type::ONEWAY;
    pu1.fareUsage().push_back(&fu2);
    addAirSegment(fu2.travelSeg(),"KTW", "WAW","KTW", "WAW", false);
    addAirSegment(fu2.travelSeg(),"MOW", "KRK","MOW", "KRK", false);
    addAirSegment(fu2.travelSeg(),"KRK", "FRA","KRK", "FRA", false);

    pu1.fareUsage().push_back(&fu3);
    addAirSegment(fu3.travelSeg(),"MOW", "KRK","MOW", "KRK", false);
    addAirSegment(fu3.travelSeg(),"GLA", "LON","GLA", "LON", false);
    addAirSegment(fu3.travelSeg(),"LON", "KRK","LON", "KRK", true);

    _farePath->pricingUnit().push_back(&pu2);

    pu2.puType() = PricingUnit::Type::ROUNDTRIP;
    pu2.fareUsage().push_back(&fu5);
    addAirSegment(fu4.travelSeg(),"LON", "GLA","LON", "GLA", false);
    addAirSegment(fu4.travelSeg(),"GLA", "KRK","GLA", "KRK", false);
    addAirSegment(fu4.travelSeg(),"KRK", "WAW","KRK", "WAW", true);

    pu2.fareUsage().push_back(&fu5);
    addAirSegment(fu5.travelSeg(),"NYC", "DFW","NYC", "DFW", false);
    addAirSegment(fu5.travelSeg(),"DFW", "NYC","DFW", "NYC", false);
    addAirSegment(fu5.travelSeg(),"NYC", "DFW","NYC", "DFW", false);
    //

    CPPUNIT_ASSERT(!_validator->setJourneyCheckingFlag(*_farePath));

    }
  */
  void testMatchJourneyRepricingPermitted1()
  {
    // permutation and old FarePath
    addPermutationData(
        *_permutation, *_exchangeItin, KEEP_THE_FARES, 111, 111, ' ', 0, "", "KRK", "KTW");
    addPermutationData(
        *_permutation, *_exchangeItin, KEEP_THE_FARES, 222, 222, ' ', 0, "", "KTW", "FRA");
    addPermutationData(
        *_permutation, *_exchangeItin, KEEP_THE_FARES, 333, 333, ' ', 0, "", "FRA", "DFW");
    //

    // new FarePath
    PricingUnit pu;
    _farePath->pricingUnit().push_back(&pu);
    addFareUsage(pu, "KRK", "KTW", false);
    addFareUsage(pu, "KTW", "FRA", false);
    addFareUsage(pu, "FRA", "DFW", false);
    //
    CPPUNIT_ASSERT(_validator->matchJourney(*_permutation));
  }

  void testMatchJourneyRepricingPermitted2()
  {
    // permutation and old FarePath
    addPermutationData(
        *_permutation, *_exchangeItin, KEEP_THE_FARES, 111, 111, ' ', 0, "", "KRK", "KTW");
    addPermutationData(
        *_permutation, *_exchangeItin, KEEP_THE_FARES, 222, 222, ' ', 0, "", "KTW", "FRA");
    addPermutationData(
        *_permutation, *_exchangeItin, KEEP_THE_FARES, 333, 333, 'X', 0, "", "FRA", "DFW");
    //

    // new FarePath
    PricingUnit pu;
    _farePath->pricingUnit().push_back(&pu);

    addFareUsage(pu, "KRK", "KTW", false);
    addFareUsage(pu, "KTW", "FRA", false);
    addFareUsage(pu, "FRA", "DFW", false);
    //
    _validator->_journeyCheckingFlag = false;

    CPPUNIT_ASSERT(_validator->matchJourney(*_permutation));
  }

  void testMatchJourneyRepricingDenied()
  {
    // permutation and old FarePath
    addPermutationData(
        *_permutation, *_exchangeItin, KEEP_THE_FARES, 111, 111, ' ', 0, "", "KRK", "KTW");
    addPermutationData(
        *_permutation, *_exchangeItin, KEEP_THE_FARES, 222, 222, ' ', 0, "", "KTW", "FRA");
    addPermutationData(
        *_permutation, *_exchangeItin, KEEP_THE_FARES, 333, 333, 'X', 0, "", "FRA", "DFW");
    //

    // new FarePath
    PricingUnit pu;
    _farePath->pricingUnit().push_back(&pu);

    addFareUsage(pu, "KRK", "KTW", false);
    addFareUsage(pu, "KTW", "FRA", false);
    addFareUsage(pu, "FRA", "DFW", false);
    //

    _validator->_journeyCheckingFlag = true;

    CPPUNIT_ASSERT(!_validator->matchJourney(*_permutation));
  }

  void testAnalyseExcFarePath()
  {
    /*
    //first PU OW
    PricingUnit pu1;
    FareUsage fu1;
    AirSeg seg11,seg12,seg13;

    _excFarePath->pricingUnit().push_back(&pu1);
    pu1.puType() = PricingUnit::Type::ONEWAY;
    pu1.fareUsage().push_back(&fu1);

    seg11.unflown() = true;
    fu1.travelSeg().push_back(&seg11);
    seg12.unflown() = true;
    fu1.travelSeg().push_back(&seg12);
    seg13.unflown() = true;
    fu1.travelSeg().push_back(&seg13);

    _validator->analyseExcFarePath();
    CPPUNIT_ASSERT(_exchangeItin->flownOneWayFares().size() == 0);

    seg11.unflown() = true;
    seg12.unflown() = false;
    seg13.unflown() = true;

    _exchangeItin->flownOneWayFaresProcessed() = false;
    _validator->analyseExcFarePath();
    CPPUNIT_ASSERT(_exchangeItin->flownOneWayFares().size() == 0);

    seg11.unflown() = false;
    seg12.unflown() = false;
    seg13.unflown() = false;

    _exchangeItin->flownOneWayFaresProcessed() = false;
    _validator->analyseExcFarePath();
    CPPUNIT_ASSERT(_exchangeItin->flownOneWayFares().size() == 1);
    _exchangeItin->flownOneWayFares().clear();
    //

    //second PU RT
    PricingUnit pu2;
    FareUsage fu2;
    AirSeg seg21,seg22,seg23;

    _excFarePath->pricingUnit().push_back(&pu2);
    pu2.puType() = PricingUnit::Type::ROUNDTRIP;
    pu2.fareUsage().push_back(&fu2);

    seg21.unflown() = true;
    fu2.travelSeg().push_back(&seg21);
    seg22.unflown() = true;
    fu2.travelSeg().push_back(&seg22);
    seg23.unflown() = true;
    fu2.travelSeg().push_back(&seg23);

    _exchangeItin->flownOneWayFaresProcessed() = false;
    _validator->analyseExcFarePath();
    CPPUNIT_ASSERT(_exchangeItin->flownOneWayFares().size() == 1);
    _exchangeItin->flownOneWayFares().clear();

    seg21.unflown() = true;
    seg22.unflown() = false;
    seg23.unflown() = true;

    _exchangeItin->flownOneWayFaresProcessed() = false;
    _validator->analyseExcFarePath();
    CPPUNIT_ASSERT(_exchangeItin->flownOneWayFares().size() == 1);
    _exchangeItin->flownOneWayFares().clear();

    seg21.unflown() = false;
    seg22.unflown() = false;
    seg23.unflown() = false;

    _exchangeItin->flownOneWayFaresProcessed() = false;
    _validator->analyseExcFarePath();
    CPPUNIT_ASSERT(_exchangeItin->flownOneWayFares().size() == 1);
    _exchangeItin->flownOneWayFares().clear();
    //

    //third PU OW
    PricingUnit pu3;
    FareUsage fu3;
    AirSeg seg31,seg32,seg33;

    _excFarePath->pricingUnit().push_back(&pu3);
    pu3.puType() = PricingUnit::Type::ONEWAY;
    pu3.fareUsage().push_back(&fu3);

    seg31.unflown() = true;
    fu3.travelSeg().push_back(&seg31);
    seg32.unflown() = true;
    fu3.travelSeg().push_back(&seg32);
    seg33.unflown() = true;
    fu3.travelSeg().push_back(&seg33);

    _exchangeItin->flownOneWayFaresProcessed() = false;
    _validator->analyseExcFarePath();
    CPPUNIT_ASSERT(_exchangeItin->flownOneWayFares().size() == 1);
    _exchangeItin->flownOneWayFares().clear();

    seg31.unflown() = true;
    seg32.unflown() = false;
    seg33.unflown() = true;

    _exchangeItin->flownOneWayFaresProcessed() = false;
    _validator->analyseExcFarePath();
    CPPUNIT_ASSERT(_exchangeItin->flownOneWayFares().size() == 1);
    _exchangeItin->flownOneWayFares().clear();

    seg31.unflown() = false;
    seg32.unflown() = false;
    seg33.unflown() = false;

    _exchangeItin->flownOneWayFaresProcessed() = false;
    _validator->analyseExcFarePath();
    CPPUNIT_ASSERT(_exchangeItin->flownOneWayFares().size() == 2);
    _exchangeItin->flownOneWayFares().clear();
    //        */
  }

  void testCheckTermBlankSameBreak()
  {
    static const Indicator BLANK = ' ';

    ReissueSequence seq;
    seq.terminalPointInd() = BLANK;

    ProcessTagInfo tag;
    tag.reissueSequence()->orig() = &seq;

    ProcessTagPermutation perm;
    perm.processTags().push_back(&tag);

    _validator->_fcMapping.resize(1);
    PaxTypeFare paxTypeFare;
    _validator->_fcMapping[0].push_back(&paxTypeFare);

    CPPUNIT_ASSERT(_validator->checkTerm(perm));
  }

  void testCheckTermBlankDiffBreak()
  {
    static const Indicator BLANK = ' ';

    ReissueSequence seq;
    seq.terminalPointInd() = BLANK;
    seq.samePointTblItemNo() = 0;

    ProcessTagInfo tag;
    tag.reissueSequence()->orig() = &seq;

    ProcessTagPermutation perm;
    perm.processTags().push_back(&tag);

    _validator->_fcMapping.clear();
    _validator->_fcMapping.resize(1);

    CPPUNIT_ASSERT(!_validator->checkTerm(perm));
  }

  void testCheckTermAllowSameBreak()
  {
    const Indicator ALLOW = 'Y';

    ReissueSequence seq;
    seq.terminalPointInd() = ALLOW;

    ProcessTagInfo tag;
    tag.reissueSequence()->orig() = &seq;

    ProcessTagPermutation perm;
    perm.processTags().push_back(&tag);

    _validator->_fcMapping.resize(1);
    PaxTypeFare paxTypeFare;
    _validator->_fcMapping[0].push_back(&paxTypeFare);

    CPPUNIT_ASSERT(_validator->checkTerm(perm));
  }

  void testCheckTermAllowDiffBreak()
  {
    const Indicator ALLOW = 'Y';

    ReissueSequence seq;
    seq.terminalPointInd() = ALLOW;
    seq.samePointTblItemNo() = 0;

    ProcessTagInfo tag;
    tag.reissueSequence()->orig() = &seq;

    ProcessTagPermutation perm;
    perm.processTags().push_back(&tag);

    _validator->_fcMapping.clear();
    _validator->_fcMapping.resize(1);

    CPPUNIT_ASSERT(_validator->checkTerm(perm));
  }

  void
  testCheckTermCombination(ProcessTagPermutation& perm, bool exp1, bool exp2, bool exp3, bool exp4)
  {
    PaxTypeFare paxTypeFare;

    _validator->_fcMapping.clear();
    _validator->_fcMapping.resize(2);
    _validator->_fcMapping[0].push_back(&paxTypeFare);
    _validator->_fcMapping[1].push_back(&paxTypeFare);
    bool result = _validator->checkTerm(perm);
    CPPUNIT_ASSERT(result == exp1);

    _validator->_fcMapping[1].clear();
    result = _validator->checkTerm(perm);
    CPPUNIT_ASSERT(result == exp2);

    _validator->_fcMapping[0].clear();
    _validator->_fcMapping[1].push_back(&paxTypeFare);
    result = _validator->checkTerm(perm);
    CPPUNIT_ASSERT(result == exp3);

    _validator->_fcMapping[1].clear();
    result = _validator->checkTerm(perm);
    CPPUNIT_ASSERT(result == exp4);
  }

  void testCheckTermMultipleFC()
  {
    const Indicator ALLOW = 'Y';
    const Indicator BLANK = ' ';

    ReissueSequence seq1, seq2;
    ProcessTagInfo tag1, tag2;
    tag1.reissueSequence()->orig() = &seq1;
    tag2.reissueSequence()->orig() = &seq2;

    ProcessTagPermutation perm;
    perm.processTags().push_back(&tag1);
    perm.processTags().push_back(&tag2);

    seq1.terminalPointInd() = BLANK;
    seq1.samePointTblItemNo() = 0;
    seq2.terminalPointInd() = BLANK;
    seq2.samePointTblItemNo() = 0;
    testCheckTermCombination(perm, true, false, false, false);

    seq1.terminalPointInd() = BLANK;
    seq1.samePointTblItemNo() = 0;
    seq2.terminalPointInd() = ALLOW;
    seq2.samePointTblItemNo() = 0;
    testCheckTermCombination(perm, true, true, false, false);

    seq1.terminalPointInd() = ALLOW;
    seq1.samePointTblItemNo() = 0;
    seq2.terminalPointInd() = BLANK;
    seq2.samePointTblItemNo() = 0;
    testCheckTermCombination(perm, true, false, true, false);

    seq1.terminalPointInd() = ALLOW;
    seq1.samePointTblItemNo() = 0;
    seq2.terminalPointInd() = ALLOW;
    seq2.samePointTblItemNo() = 0;
    testCheckTermCombination(perm, true, true, true, true);
  }

  void testCheckFirstBreak(const Indicator ind,
                           bool sameBreaks,
                           bool sameFareBasis,
                           bool sameFareAmount,
                           const VCTR* vctr,
                           bool exp)
  {
    ReissueSequence seq;
    seq.firstBreakInd() = ind;
    seq.samePointTblItemNo() = 0;

    FareCompInfo fci;
    fci.fareBasisCode() = "AAA";
    if (vctr)
    {
      fci.VCTR() = *vctr;
      fci.hasVCTR() = true;
    }

    ProcessTagInfo tag;
    tag.reissueSequence()->orig() = &seq;
    tag.fareCompInfo() = &fci;

    ProcessTagPermutation perm;
    perm.processTags().push_back(&tag);

    FareInfo fareInfoOld, fareInfoNew;
    fareInfoOld.fareAmount() = 1;
    fareInfoNew.fareAmount() = sameFareAmount ? 1 : 2;
    fareInfoNew.fareClass() = sameFareBasis ? "AAA" : "BBB";

    Fare fareOld, fareNew;
    fareOld.setFareInfo(&fareInfoOld);
    fareNew.setFareInfo(&fareInfoNew);

    FareMarket fareMarketOld, fareMarketNew;
    fareMarketOld.boardMultiCity() = "LON";
    fareMarketOld.offMultiCity() = "DFW";
    fareMarketNew.boardMultiCity() = sameBreaks ? "LON" : "KRK";
    fareMarketNew.offMultiCity() = sameBreaks ? "DFW" : "LON";

    PaxTypeFare paxTypeFareOld, paxTypeFareNew;
    paxTypeFareOld.setFare(&fareOld);
    paxTypeFareNew.setFare(&fareNew);
    paxTypeFareOld.fareMarket() = &fareMarketOld;
    paxTypeFareNew.fareMarket() = &fareMarketNew;

    tag.paxTypeFare() = &paxTypeFareOld;
    _validator->_firstFare = &paxTypeFareNew;

    bool result = _validator->checkFirstBreak(perm);
    CPPUNIT_ASSERT(result == exp);
  }

  void testCheckFirstBreakByte25Blank() { testCheckFirstBreak(' ', false, false, false, 0, true); }

  void testCheckFirstBreakAllSame()
  {
    VCTR vctr;
    testCheckFirstBreak('Y', true, true, true, &vctr, true);
  }

  void testCheckFirstBreakNoFcMapping()
  {
    _validator->_fcMapping.front().clear();
    VCTR vctr;
    testCheckFirstBreak('Y', true, true, true, &vctr, true);
  }

  void testCheckFirstBreakDiffBreaks()
  {
    VCTR vctr;
    testCheckFirstBreak('Y', false, true, true, &vctr, false);
  }

  void testCheckFirstBreakDiffFareBasis()
  {
    VCTR vctr;
    testCheckFirstBreak('Y', true, false, true, &vctr, false);
  }

  void testCheckFirstBreakDiffFareAmount()
  {
    VCTR vctr;
    testCheckFirstBreak('Y', true, true, false, &vctr, false);
  }

  void testCheckFirstBreakNoVCTR() { testCheckFirstBreak('Y', true, true, true, 0, true); }

  void testCheckFirstBreakDiffVendor()
  {
    VCTR vctr;
    vctr.vendor() = "DIF";
    testCheckFirstBreak('Y', true, true, true, &vctr, false);
  }

  void testCheckFirstBreakDiffCarrier()
  {
    VCTR vctr;
    vctr.carrier() = "DIF";
    testCheckFirstBreak('Y', true, true, true, &vctr, false);
  }

  void testCheckFirstBreakDiffTariff()
  {
    VCTR vctr;
    vctr.tariff() = 1;
    testCheckFirstBreak('Y', true, true, true, &vctr, false);
  }

  void testCheckFirstBreakDiffRule()
  {
    VCTR vctr;
    vctr.rule() = 1;
    testCheckFirstBreak('Y', true, true, true, &vctr, false);
  }

  void testCheckFirstBreakMultipleFCByte25Blank()
  {
    ReissueSequence seq1, seq2;
    seq1.firstBreakInd() = ' ';
    seq2.firstBreakInd() = ' ';

    ProcessTagInfo tag1, tag2;
    tag1.reissueSequence()->orig() = &seq1;
    tag2.reissueSequence()->orig() = &seq2;

    ProcessTagPermutation perm;
    perm.processTags().push_back(&tag1);
    perm.processTags().push_back(&tag2);

    CPPUNIT_ASSERT(_validator->checkFirstBreak(perm));
  }

  void testCheckFirstBreakMultipleFCByte25Y()
  {
    ReissueSequence seq1, seq2;
    seq1.firstBreakInd() = ' ';
    seq2.firstBreakInd() = 'Y';
    seq1.samePointTblItemNo() = 0;

    ProcessTagInfo tag1, tag2;
    tag1.reissueSequence()->orig() = &seq1;
    tag2.reissueSequence()->orig() = &seq2;

    ProcessTagPermutation perm;
    perm.processTags().push_back(&tag1);
    perm.processTags().push_back(&tag2);

    // fail at same breaks test
    FareMarket fareMarketOld, fareMarketNew;
    fareMarketOld.boardMultiCity() = "LON";
    fareMarketNew.boardMultiCity() = "KRK";
    fareMarketOld.offMultiCity() = "DFW";
    fareMarketNew.offMultiCity() = "LON";

    PaxTypeFare paxTypeFareOld, paxTypeFareNew;
    paxTypeFareOld.fareMarket() = &fareMarketOld;
    paxTypeFareNew.fareMarket() = &fareMarketNew;

    tag1.paxTypeFare() = &paxTypeFareOld;
    _validator->_firstFare = &paxTypeFareNew;

    CPPUNIT_ASSERT(!_validator->checkFirstBreak(perm));
  }

  FareMarket::FareRetrievalFlags testFindFareRetrievalFlag(FareMarket::FareRetrievalFlags flag1,
                                                           FareMarket::FareRetrievalFlags flag2,
                                                           FareMarket::FareRetrievalFlags flag3)
  {
    PaxTypeFare fare1, fare2, fare3;
    FareMarket::RetrievalInfo* info;
    info = create<FareMarket::RetrievalInfo>();
    fare1.retrievalInfo() = info;
    fare1.retrievalInfo()->_flag = flag1;

    info = create<FareMarket::RetrievalInfo>();
    fare2.retrievalInfo() = info;
    fare2.retrievalInfo()->_flag = flag2;

    info = create<FareMarket::RetrievalInfo>();
    fare3.retrievalInfo() = info;
    fare3.retrievalInfo()->_flag = flag3;

    _validator->_allRepricePTFs.push_back(&fare1);
    _validator->_allRepricePTFs.push_back(&fare2);
    _validator->_allRepricePTFs.push_back(&fare3);

    _validator->findFareRetrievalFlag();
    return _validator->_retrievalFlag;
  }

  void testFindFareRetrievalFlagCurrent()
  {
    FareMarket::FareRetrievalFlags rf = FareMarket::RetrievCurrent;
    FareMarket::FareRetrievalFlags flag = testFindFareRetrievalFlag(rf, rf, rf);
    CPPUNIT_ASSERT(flag == rf);
  }

  void testFindFareRetrievalFlagHistorical()
  {
    FareMarket::FareRetrievalFlags rf = FareMarket::RetrievHistorical;
    FareMarket::FareRetrievalFlags flag = testFindFareRetrievalFlag(rf, rf, rf);
    CPPUNIT_ASSERT(flag == rf);
  }

  void testFindFareRetrievalFlagTvlCommence()
  {
    FareMarket::FareRetrievalFlags rf = FareMarket::RetrievTvlCommence;
    FareMarket::FareRetrievalFlags flag = testFindFareRetrievalFlag(rf, rf, rf);
    CPPUNIT_ASSERT(flag == rf);
  }

  void testFindFareRetrievalFlagMixed()
  {
    FareMarket::FareRetrievalFlags flag = testFindFareRetrievalFlag(
        FareMarket::RetrievCurrent, FareMarket::RetrievHistorical, FareMarket::RetrievTvlCommence);
    CPPUNIT_ASSERT(flag == FareMarket::RetrievNone);
  }

  void testFindFareRetrievalFlagCombined1()
  {
    FareMarket::FareRetrievalFlags rf = (FareMarket::FareRetrievalFlags)(
        FareMarket::RetrievCurrent | FareMarket::RetrievTvlCommence);
    FareMarket::FareRetrievalFlags flag = testFindFareRetrievalFlag(rf, rf, rf);
    CPPUNIT_ASSERT(flag == rf);
  }

  void testFindFareRetrievalFlagCombined2()
  {
    FareMarket::FareRetrievalFlags flg1 = (FareMarket::FareRetrievalFlags)(
        FareMarket::RetrievCurrent | FareMarket::RetrievTvlCommence);
    FareMarket::FareRetrievalFlags flg2 = FareMarket::RetrievTvlCommence;
    FareMarket::FareRetrievalFlags flag = testFindFareRetrievalFlag(flg1, flg2, flg1);
    CPPUNIT_ASSERT(flag == flg2);
  }

  void addPermutationTag(ProcessTag tag)
  {
    ProcessTagInfo* info = create<ProcessTagInfo>();
    ReissueSequence* seq = create<ReissueSequence>();

    seq->processingInd() = tag;
    info->reissueSequence()->orig() = seq;
    _permutation->processTags().push_back(info);
  }

  void testCheckTagsTag2Only()
  {
    addPermutationTag(GUARANTEED_AIR_FARE);
    addPermutationTag(GUARANTEED_AIR_FARE);
    addPermutationTag(GUARANTEED_AIR_FARE);

    _validator->_retrievalFlag = FareMarket::RetrievHistorical;
    CPPUNIT_ASSERT(_validator->checkTags(*_permutation, *_msg));

    _validator->_retrievalFlag = FareMarket::RetrievCurrent;
    CPPUNIT_ASSERT(!_validator->checkTags(*_permutation, *_msg));

    _validator->_retrievalFlag = FareMarket::RetrievTvlCommence;
    CPPUNIT_ASSERT(!_validator->checkTags(*_permutation, *_msg));

    _validator->_retrievalFlag = (FareMarket::FareRetrievalFlags)(FareMarket::RetrievCurrent |
                                                                  FareMarket::RetrievTvlCommence);
    CPPUNIT_ASSERT(!_validator->checkTags(*_permutation, *_msg));

    _validator->_retrievalFlag = FareMarket::RetrievKeep;
    CPPUNIT_ASSERT(!_validator->checkTags(*_permutation, *_msg));

    _validator->_retrievalFlag = FareMarket::RetrievNone;
    CPPUNIT_ASSERT(!_validator->checkTags(*_permutation, *_msg));
  }

  void testCheckTagsTag5Only()
  {
    addPermutationTag(NO_GUARANTEED_FARES);
    addPermutationTag(NO_GUARANTEED_FARES);
    addPermutationTag(NO_GUARANTEED_FARES);

    _validator->_retrievalFlag = FareMarket::RetrievHistorical;
    CPPUNIT_ASSERT(!_validator->checkTags(*_permutation, *_msg));

    _validator->_retrievalFlag = FareMarket::RetrievCurrent;
    CPPUNIT_ASSERT(_validator->checkTags(*_permutation, *_msg));

    _validator->_retrievalFlag = FareMarket::RetrievTvlCommence;
    CPPUNIT_ASSERT(!_validator->checkTags(*_permutation, *_msg));

    _validator->_retrievalFlag = (FareMarket::FareRetrievalFlags)(FareMarket::RetrievCurrent |
                                                                  FareMarket::RetrievTvlCommence);
    CPPUNIT_ASSERT(_validator->checkTags(*_permutation, *_msg));

    _validator->_retrievalFlag = FareMarket::RetrievKeep;
    CPPUNIT_ASSERT(!_validator->checkTags(*_permutation, *_msg));

    _validator->_retrievalFlag = FareMarket::RetrievNone;
    CPPUNIT_ASSERT(!_validator->checkTags(*_permutation, *_msg));
  }

  void testCheckTagsTag6OnlyTravelCommenced()
  {
    addPermutationTag(TRAVEL_COMENCEMENT_AIR_FARES);
    addPermutationTag(TRAVEL_COMENCEMENT_AIR_FARES);
    addPermutationTag(TRAVEL_COMENCEMENT_AIR_FARES);

    _validator->_travelCommenced = true;

    _validator->_retrievalFlag = FareMarket::RetrievHistorical;
    CPPUNIT_ASSERT(!_validator->checkTags(*_permutation, *_msg));

    _validator->_retrievalFlag = FareMarket::RetrievCurrent;
    CPPUNIT_ASSERT(!_validator->checkTags(*_permutation, *_msg));

    _validator->_retrievalFlag = FareMarket::RetrievTvlCommence;
    CPPUNIT_ASSERT(_validator->checkTags(*_permutation, *_msg));

    _validator->_retrievalFlag = (FareMarket::FareRetrievalFlags)(FareMarket::RetrievCurrent |
                                                                  FareMarket::RetrievTvlCommence);
    CPPUNIT_ASSERT(_validator->checkTags(*_permutation, *_msg));

    _validator->_retrievalFlag = FareMarket::RetrievKeep;
    CPPUNIT_ASSERT(!_validator->checkTags(*_permutation, *_msg));

    _validator->_retrievalFlag = FareMarket::RetrievNone;
    CPPUNIT_ASSERT(!_validator->checkTags(*_permutation, *_msg));
  }

  void testCheckTagsTag6OnlyTravelNotCommenced()
  {
    addPermutationTag(TRAVEL_COMENCEMENT_AIR_FARES);
    addPermutationTag(TRAVEL_COMENCEMENT_AIR_FARES);
    addPermutationTag(TRAVEL_COMENCEMENT_AIR_FARES);

    _validator->_travelCommenced = false;

    _validator->_retrievalFlag = FareMarket::RetrievHistorical;
    CPPUNIT_ASSERT(!_validator->checkTags(*_permutation, *_msg));

    _validator->_retrievalFlag = FareMarket::RetrievCurrent;
    CPPUNIT_ASSERT(_validator->checkTags(*_permutation, *_msg));

    _validator->_retrievalFlag = (FareMarket::FareRetrievalFlags)(FareMarket::RetrievCurrent |
                                                                  FareMarket::RetrievHistorical);
    CPPUNIT_ASSERT(_validator->checkTags(*_permutation, *_msg));

    _validator->_retrievalFlag = FareMarket::RetrievKeep;
    CPPUNIT_ASSERT(!_validator->checkTags(*_permutation, *_msg));

    _validator->_retrievalFlag = FareMarket::RetrievNone;
    CPPUNIT_ASSERT(!_validator->checkTags(*_permutation, *_msg));
  }

  void testCheckTagsTag7Only()
  {
    addPermutationTag(REISSUE_DOWN_TO_LOWER_FARE);
    addPermutationTag(REISSUE_DOWN_TO_LOWER_FARE);
    addPermutationTag(REISSUE_DOWN_TO_LOWER_FARE);

    _validator->_retrievalFlag = FareMarket::RetrievHistorical;
    CPPUNIT_ASSERT(!_validator->checkTags(*_permutation, *_msg));

    _validator->_retrievalFlag = FareMarket::RetrievCurrent;
    CPPUNIT_ASSERT(_validator->checkTags(*_permutation, *_msg));

    _validator->_retrievalFlag = FareMarket::RetrievTvlCommence;
    CPPUNIT_ASSERT(!_validator->checkTags(*_permutation, *_msg));

    _validator->_retrievalFlag = (FareMarket::FareRetrievalFlags)(FareMarket::RetrievCurrent |
                                                                  FareMarket::RetrievTvlCommence);
    CPPUNIT_ASSERT(_validator->checkTags(*_permutation, *_msg));

    _validator->_retrievalFlag = FareMarket::RetrievKeep;
    CPPUNIT_ASSERT(!_validator->checkTags(*_permutation, *_msg));

    _validator->_retrievalFlag = FareMarket::RetrievNone;
    CPPUNIT_ASSERT(!_validator->checkTags(*_permutation, *_msg));
  }

  PaxTypeFare* addWarResultToMaps(FareApplication fa, int tag)
  {
    PaxTypeFare* paxType = create<PaxTypeFare>();
    FareInfo* _fareInfo = create<FareInfo>();
    Fare* _fare = create<Fare>();
    _fare->setFareInfo(_fareInfo);
    paxType->setFare(_fare);

    FareMarket* fareMarket = create<FareMarket>();
    paxType->fareMarket() = fareMarket;

    ProcessTagInfo* _pti = create<ProcessTagInfo>();
    ReissueSequence* rs = create<ReissueSequence>();
    rs->processingInd() = tag;
    _pti->reissueSequence()->orig() = rs;

    _permutation->fareApplMap()[paxType] = fa;
    _permutation->fareApplWinnerTags()[fa] = _pti;

    return paxType;
  }

  FareMarket* addExcFcToNewItinKeepFares(const PaxTypeFare* paxType)
  {
    FareMarket* fareMarket = create<FareMarket>();
    _trx->newItinKeepFares()[paxType] = fareMarket;

    return fareMarket;
  }

  void addNewFcToAllRepricePTF(FareMarket::FareRetrievalFlags flags, FareMarket* keepFm = 0)
  {
    PaxTypeFare* paxType = create<PaxTypeFare>();
    FareInfo* _fareInfo = create<FareInfo>();
    Fare* _fare = create<Fare>();
    _fare->setFareInfo(_fareInfo);
    paxType->setFare(_fare);
    FareMarket::RetrievalInfo* ri = create<FareMarket::RetrievalInfo>();
    ri->_flag = flags;
    paxType->retrievalInfo() = ri;

    FareMarket* fareMarket = !keepFm ? create<FareMarket>() : keepFm;
    paxType->fareMarket() = fareMarket;

    _validator->_allRepricePTFs.push_back(paxType);
  }

  void setFirstFmForCheckMixedTags(FareMarket* fm, FCChangeStatus status)
  {
    FareCompInfo* fci = create<FareCompInfo>();
    fci->fareMarket() = fm;
    fm->changeStatus() = status;
    fm->asBookChangeStatus() = status;
    _exchangeItin->fareComponent().push_back(fci);
  }

  void testCheckTagsMixedTags_ExcKCC133_NewCC()
  {
    PaxTypeFare* paxType1 = addWarResultToMaps(KEEP, KEEP_THE_FARES);
    addWarResultToMaps(CURRENT, KEEP_FARES_FOR_TRAVELED_FC);
    addWarResultToMaps(CURRENT, KEEP_FARES_FOR_TRAVELED_FC);

    addExcFcToNewItinKeepFares(paxType1);

    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);
    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);

    CPPUNIT_ASSERT(!_validator->checkTagsForMixedTags(*_permutation, *_msg));
  }

  void testCheckTagsMixedTags_ExcKCC133_NewKCC()
  {
    PaxTypeFare* paxType1 = addWarResultToMaps(KEEP, KEEP_THE_FARES);
    addWarResultToMaps(CURRENT, KEEP_FARES_FOR_TRAVELED_FC);
    addWarResultToMaps(CURRENT, KEEP_FARES_FOR_TRAVELED_FC);

    setFirstFmForCheckMixedTags(paxType1->fareMarket(), tse::UU);

    FareMarket* fm1 = addExcFcToNewItinKeepFares(paxType1);

    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm1);
    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);
    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);

    CPPUNIT_ASSERT(_validator->checkTagsForMixedTags(*_permutation, *_msg));
  }

  void testCheckTagsMixedTags_ExcKCC133_NewKKC()
  {
    PaxTypeFare* paxType1 = addWarResultToMaps(KEEP, KEEP_THE_FARES);
    addWarResultToMaps(CURRENT, KEEP_FARES_FOR_TRAVELED_FC);
    addWarResultToMaps(CURRENT, KEEP_FARES_FOR_TRAVELED_FC);

    setFirstFmForCheckMixedTags(paxType1->fareMarket(), tse::UU);

    FareMarket* fm1 = addExcFcToNewItinKeepFares(paxType1);

    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm1);
    addNewFcToAllRepricePTF(FareMarket::RetrievKeep);
    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);

    CPPUNIT_ASSERT(!_validator->checkTagsForMixedTags(*_permutation, *_msg));
  }

  void testCheckTagsMixedTags_ExcKCC133_NewKCCC()
  {
    PaxTypeFare* paxType1 = addWarResultToMaps(KEEP, KEEP_THE_FARES);
    addWarResultToMaps(CURRENT, KEEP_FARES_FOR_TRAVELED_FC);
    addWarResultToMaps(CURRENT, KEEP_FARES_FOR_TRAVELED_FC);

    setFirstFmForCheckMixedTags(paxType1->fareMarket(), tse::UU);

    FareMarket* fm1 = addExcFcToNewItinKeepFares(paxType1);

    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm1);
    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);
    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);
    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);

    CPPUNIT_ASSERT(_validator->checkTagsForMixedTags(*_permutation, *_msg));
  }

  void testCheckTagsMixedTags_ExcKCC333_NewCC()
  {
    PaxTypeFare* paxType1 = addWarResultToMaps(KEEP, KEEP_FARES_FOR_TRAVELED_FC);
    addWarResultToMaps(CURRENT, KEEP_FARES_FOR_TRAVELED_FC);
    addWarResultToMaps(CURRENT, KEEP_FARES_FOR_TRAVELED_FC);

    setFirstFmForCheckMixedTags(paxType1->fareMarket(), tse::UU);

    addExcFcToNewItinKeepFares(paxType1);

    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);
    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);

    CPPUNIT_ASSERT(!_validator->checkTagsForMixedTags(*_permutation, *_msg));
  }

  void testCheckTagsMixedTags_ExcKCC333_NewKCC()
  {
    PaxTypeFare* paxType1 = addWarResultToMaps(KEEP, KEEP_FARES_FOR_TRAVELED_FC);
    addWarResultToMaps(CURRENT, KEEP_FARES_FOR_TRAVELED_FC);
    addWarResultToMaps(CURRENT, KEEP_FARES_FOR_TRAVELED_FC);

    setFirstFmForCheckMixedTags(paxType1->fareMarket(), tse::UU);

    FareMarket* fm1 = addExcFcToNewItinKeepFares(paxType1);

    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm1);
    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);
    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);

    CPPUNIT_ASSERT(_validator->checkTagsForMixedTags(*_permutation, *_msg));
  }

  void testCheckTagsMixedTags_ExcKCC333_NewKKC()
  {
    PaxTypeFare* paxType1 = addWarResultToMaps(KEEP, KEEP_FARES_FOR_TRAVELED_FC);
    addWarResultToMaps(CURRENT, KEEP_FARES_FOR_TRAVELED_FC);
    addWarResultToMaps(CURRENT, KEEP_FARES_FOR_TRAVELED_FC);

    setFirstFmForCheckMixedTags(paxType1->fareMarket(), tse::UU);

    FareMarket* fm1 = addExcFcToNewItinKeepFares(paxType1);

    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm1);
    addNewFcToAllRepricePTF(FareMarket::RetrievKeep);
    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);

    CPPUNIT_ASSERT(!_validator->checkTagsForMixedTags(*_permutation, *_msg));
  }

  void testCheckTagsMixedTags_ExcKCC333_NewCCCC()
  {
    PaxTypeFare* paxType1 = addWarResultToMaps(KEEP, KEEP_FARES_FOR_TRAVELED_FC);
    addWarResultToMaps(CURRENT, KEEP_FARES_FOR_TRAVELED_FC);
    addWarResultToMaps(CURRENT, KEEP_FARES_FOR_TRAVELED_FC);

    setFirstFmForCheckMixedTags(paxType1->fareMarket(), tse::UU);

    addExcFcToNewItinKeepFares(paxType1);

    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);
    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);
    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);
    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);

    CPPUNIT_ASSERT(!_validator->checkTagsForMixedTags(*_permutation, *_msg));
  }

  void testCheckTagsMixedTags_ExcKCC333_NewKCCC()
  {
    PaxTypeFare* paxType1 = addWarResultToMaps(KEEP, KEEP_FARES_FOR_TRAVELED_FC);
    addWarResultToMaps(CURRENT, KEEP_FARES_FOR_TRAVELED_FC);
    addWarResultToMaps(CURRENT, KEEP_FARES_FOR_TRAVELED_FC);

    setFirstFmForCheckMixedTags(paxType1->fareMarket(), tse::UU);

    FareMarket* fm1 = addExcFcToNewItinKeepFares(paxType1);

    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm1);
    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);
    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);
    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);

    CPPUNIT_ASSERT(_validator->checkTagsForMixedTags(*_permutation, *_msg));
  }

  void testCheckTagsMixedTags_ExcKKK111_NewCC()
  {
    PaxTypeFare* paxType1 = addWarResultToMaps(KEEP, KEEP_THE_FARES);
    addWarResultToMaps(KEEP, KEEP_THE_FARES);
    addWarResultToMaps(KEEP, KEEP_THE_FARES);

    setFirstFmForCheckMixedTags(paxType1->fareMarket(), tse::UU);

    addExcFcToNewItinKeepFares(paxType1);

    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);
    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);

    CPPUNIT_ASSERT(!_validator->checkTagsForMixedTags(*_permutation, *_msg));
  }

  void testCheckTagsMixedTags_ExcKKK111_NewKCC()
  {
    PaxTypeFare* paxType1 = addWarResultToMaps(KEEP, KEEP_THE_FARES);
    addWarResultToMaps(KEEP, KEEP_THE_FARES);
    addWarResultToMaps(KEEP, KEEP_THE_FARES);

    setFirstFmForCheckMixedTags(paxType1->fareMarket(), tse::UU);

    FareMarket* fm1 = addExcFcToNewItinKeepFares(paxType1);

    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm1);
    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);
    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);

    CPPUNIT_ASSERT(!_validator->checkTagsForMixedTags(*_permutation, *_msg));
  }

  void testCheckTagsMixedTags_ExcKKK111_NewKKC()
  {
    PaxTypeFare* paxType1 = addWarResultToMaps(KEEP, KEEP_THE_FARES);
    PaxTypeFare* paxType2 = addWarResultToMaps(KEEP, KEEP_THE_FARES);
    addWarResultToMaps(KEEP, KEEP_THE_FARES);

    setFirstFmForCheckMixedTags(paxType1->fareMarket(), tse::UU);

    FareMarket* fm1 = addExcFcToNewItinKeepFares(paxType1);
    FareMarket* fm2 = addExcFcToNewItinKeepFares(paxType2);

    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm1);
    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm2);
    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);

    CPPUNIT_ASSERT(!_validator->checkTagsForMixedTags(*_permutation, *_msg));
  }

  void testCheckTagsMixedTags_ExcKKK111_NewKCCK()
  {
    PaxTypeFare* paxType1 = addWarResultToMaps(KEEP, KEEP_THE_FARES);
    PaxTypeFare* paxType2 = addWarResultToMaps(KEEP, KEEP_THE_FARES);
    addWarResultToMaps(KEEP, KEEP_THE_FARES);

    setFirstFmForCheckMixedTags(paxType1->fareMarket(), tse::UU);

    FareMarket* fm1 = addExcFcToNewItinKeepFares(paxType1);
    FareMarket* fm2 = addExcFcToNewItinKeepFares(paxType2);

    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm1);
    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);
    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);
    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm2);

    CPPUNIT_ASSERT(!_validator->checkTagsForMixedTags(*_permutation, *_msg));
  }

  void testCheckTagsMixedTags_ExcKKK111_NewKKK()
  {
    PaxTypeFare* paxType1 = addWarResultToMaps(KEEP, KEEP_THE_FARES);
    PaxTypeFare* paxType2 = addWarResultToMaps(KEEP, KEEP_THE_FARES);
    PaxTypeFare* paxType3 = addWarResultToMaps(KEEP, KEEP_THE_FARES);

    setFirstFmForCheckMixedTags(paxType1->fareMarket(), tse::UU);

    FareMarket* fm1 = addExcFcToNewItinKeepFares(paxType1);
    FareMarket* fm2 = addExcFcToNewItinKeepFares(paxType2);
    FareMarket* fm3 = addExcFcToNewItinKeepFares(paxType3);

    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm1);
    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm2);
    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm3);

    CPPUNIT_ASSERT(_validator->checkTagsForMixedTags(*_permutation, *_msg));
  }

  void testCheckTagsMixedTags_ExcCCC555_NewKKK()
  {
    PaxTypeFare* paxType1 = addWarResultToMaps(CURRENT, NO_GUARANTEED_FARES);
    PaxTypeFare* paxType2 = addWarResultToMaps(CURRENT, NO_GUARANTEED_FARES);
    PaxTypeFare* paxType3 = addWarResultToMaps(CURRENT, NO_GUARANTEED_FARES);

    FareMarket* fm1 = addExcFcToNewItinKeepFares(paxType1);
    FareMarket* fm2 = addExcFcToNewItinKeepFares(paxType2);
    FareMarket* fm3 = addExcFcToNewItinKeepFares(paxType3);

    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm1);
    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm2);
    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm3);

    CPPUNIT_ASSERT(!_validator->checkTagsForMixedTags(*_permutation, *_msg));
  }

  void testCheckTagsMixedTags_ExcCCC555_NewCH()
  {
    addWarResultToMaps(CURRENT, NO_GUARANTEED_FARES);
    addWarResultToMaps(CURRENT, NO_GUARANTEED_FARES);
    addWarResultToMaps(CURRENT, NO_GUARANTEED_FARES);

    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);
    addNewFcToAllRepricePTF(FareMarket::RetrievHistorical);

    CPPUNIT_ASSERT(!_validator->checkTagsForMixedTags(*_permutation, *_msg));
  }

  void testCheckTagsMixedTags_ExcCCC555_NewCC()
  {
    addWarResultToMaps(CURRENT, NO_GUARANTEED_FARES);
    addWarResultToMaps(CURRENT, NO_GUARANTEED_FARES);
    addWarResultToMaps(CURRENT, NO_GUARANTEED_FARES);

    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);
    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);

    CPPUNIT_ASSERT(_validator->checkTagsForMixedTags(*_permutation, *_msg));
  }

  void testCheckTagsMixedTagsSetup(FareApplication fa[3],
                                   ProcessTag tag[3],
                                   FCChangeStatus changeStati[3],
                                   FareMarket::FareRetrievalFlags flags[3])
  {
    PaxTypeFare* paxType1 = addWarResultToMaps(fa[0], tag[0]);
    PaxTypeFare* paxType2 = addWarResultToMaps(fa[1], tag[1]);
    PaxTypeFare* paxType3 = addWarResultToMaps(fa[2], tag[2]);

    setFirstFmForCheckMixedTags(paxType1->fareMarket(), changeStati[0]);
    setFirstFmForCheckMixedTags(paxType2->fareMarket(), changeStati[1]);
    setFirstFmForCheckMixedTags(paxType3->fareMarket(), changeStati[2]);

    FareMarket* fm1 = addExcFcToNewItinKeepFares(paxType1);
    FareMarket* fm2 = addExcFcToNewItinKeepFares(paxType2);
    FareMarket* fm3 = addExcFcToNewItinKeepFares(paxType3);

    addNewFcToAllRepricePTF(flags[0], fm1);
    addNewFcToAllRepricePTF(flags[1], fm2);
    addNewFcToAllRepricePTF(flags[2], fm3);
  }

  void testCheckTagsMixedTags_ExcHHH222_NewHHH_1FL2UC3UN()
  {
    FareApplication fa[3] = {HISTORICAL, HISTORICAL, HISTORICAL};
    ProcessTag tag[3] = {GUARANTEED_AIR_FARE, GUARANTEED_AIR_FARE, GUARANTEED_AIR_FARE};
    FCChangeStatus changeStati[3] = {tse::FL, tse::UC, tse::UN};
    FareMarket::FareRetrievalFlags flags[3] = {
        FareMarket::RetrievKeep, FareMarket::RetrievCurrent, FareMarket::RetrievCurrent};
    testCheckTagsMixedTagsSetup(fa, tag, changeStati, flags);
    CPPUNIT_ASSERT(!_validator->checkTagsForMixedTags(*_permutation, *_msg));
  }

  void testCheckTagsMixedTags_ExcHHH222_NewHHH_1UU2UU3UU()
  {
    FareApplication fa[3] = {HISTORICAL, HISTORICAL, HISTORICAL};
    ProcessTag tag[3] = {GUARANTEED_AIR_FARE, GUARANTEED_AIR_FARE, GUARANTEED_AIR_FARE};
    FCChangeStatus changeStati[3] = {tse::UU, tse::UU, tse::UU};
    FareMarket::FareRetrievalFlags flags[3] = {FareMarket::RetrievHistorical,
                                               FareMarket::RetrievHistorical,
                                               FareMarket::RetrievHistorical};
    testCheckTagsMixedTagsSetup(fa, tag, changeStati, flags);
    CPPUNIT_ASSERT(_validator->checkTagsForMixedTags(*_permutation, *_msg));
  }

  void testCheckTagsMixedTags_ExcKKK444_NewKCC_1FL2UC3UN()
  {
    FareApplication fa[3] = {KEEP, CURRENT, CURRENT};
    ProcessTag tag[3] = {
        KEEP_FARES_FOR_UNCHANGED_FC, KEEP_FARES_FOR_UNCHANGED_FC, KEEP_FARES_FOR_UNCHANGED_FC};
    FCChangeStatus changeStati[3] = {tse::FL, tse::UC, tse::UN};
    FareMarket::FareRetrievalFlags flags[3] = {
        FareMarket::RetrievKeep, FareMarket::RetrievCurrent, FareMarket::RetrievCurrent};
    testCheckTagsMixedTagsSetup(fa, tag, changeStati, flags);
    CPPUNIT_ASSERT(_validator->checkTagsForMixedTags(*_permutation, *_msg));
  }

  void testCheckTagsMixedTags_ExcKKK444_NewKKK_1FL2UU3UU()
  {
    FareApplication fa[3] = {KEEP, KEEP, KEEP};
    ProcessTag tag[3] = {
        KEEP_FARES_FOR_UNCHANGED_FC, KEEP_FARES_FOR_UNCHANGED_FC, KEEP_FARES_FOR_UNCHANGED_FC};
    FCChangeStatus changeStati[3] = {tse::FL, tse::UU, tse::UU};
    FareMarket::FareRetrievalFlags flags[3] = {
        FareMarket::RetrievKeep, FareMarket::RetrievKeep, FareMarket::RetrievKeep};
    testCheckTagsMixedTagsSetup(fa, tag, changeStati, flags);
    CPPUNIT_ASSERT(_validator->checkTagsForMixedTags(*_permutation, *_msg));
  }

  void testCheckTagsMixedTags_ExcKKK111_NewKKK_FirstFCChanged()
  {
    PaxTypeFare* paxType1 = addWarResultToMaps(KEEP, KEEP_THE_FARES);
    PaxTypeFare* paxType2 = addWarResultToMaps(KEEP, KEEP_THE_FARES);
    PaxTypeFare* paxType3 = addWarResultToMaps(KEEP, KEEP_THE_FARES);

    setFirstFmForCheckMixedTags(paxType1->fareMarket(), tse::UC);

    FareMarket* fm1 = addExcFcToNewItinKeepFares(paxType1);
    FareMarket* fm2 = addExcFcToNewItinKeepFares(paxType2);
    FareMarket* fm3 = addExcFcToNewItinKeepFares(paxType3);

    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm1);
    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm2);
    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm3);

    CPPUNIT_ASSERT(!_validator->checkTagsForMixedTags(*_permutation, *_msg));
  }

  void testCheckTagsMixedTags_ExcKKK111_NewKKK_FirstFCNotChanged()
  {
    PaxTypeFare* paxType1 = addWarResultToMaps(KEEP, KEEP_THE_FARES);
    PaxTypeFare* paxType2 = addWarResultToMaps(KEEP, KEEP_THE_FARES);
    PaxTypeFare* paxType3 = addWarResultToMaps(KEEP, KEEP_THE_FARES);

    setFirstFmForCheckMixedTags(paxType1->fareMarket(), tse::UU);

    FareMarket* fm1 = addExcFcToNewItinKeepFares(paxType1);
    FareMarket* fm2 = addExcFcToNewItinKeepFares(paxType2);
    FareMarket* fm3 = addExcFcToNewItinKeepFares(paxType3);

    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm1);
    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm2);
    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm3);

    CPPUNIT_ASSERT(_validator->checkTagsForMixedTags(*_permutation, *_msg));
  }

  void testCheckTagsMixedTags_ExcKKK111_NewKKKK()
  {
    PaxTypeFare* paxType1 = addWarResultToMaps(KEEP, KEEP_THE_FARES);
    PaxTypeFare* paxType2 = addWarResultToMaps(KEEP, KEEP_THE_FARES);
    PaxTypeFare* paxType3 = addWarResultToMaps(KEEP, KEEP_THE_FARES);

    setFirstFmForCheckMixedTags(paxType1->fareMarket(), tse::UU);

    FareMarket* fm1 = addExcFcToNewItinKeepFares(paxType1);
    FareMarket* fm2 = addExcFcToNewItinKeepFares(paxType2);
    FareMarket* fm3 = addExcFcToNewItinKeepFares(paxType3);

    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm1);
    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm2);
    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm3);
    addNewFcToAllRepricePTF(FareMarket::RetrievKeep);

    CPPUNIT_ASSERT(!_validator->checkTagsForMixedTags(*_permutation, *_msg));
  }

  void testCheckOWRT1()
  {
    std::cout << "start\n";
    PaxTypeFare* ptf1 = PaxTypeFareWithOWRT(ONE_WAY_MAY_BE_DOUBLED);
    PaxTypeFare* ptf2 = PaxTypeFareWithOWRT(ONE_WAY_MAYNOT_BE_DOUBLED);
    PaxTypeFare* ptf3 = PaxTypeFareWithOWRT(ONE_WAY_MAYNOT_BE_DOUBLED);
    PaxTypeFare* ptf4 = PaxTypeFareWithOWRT(ONE_WAY_MAY_BE_DOUBLED);
    FarePath* fp = FP(ptf1, ptf2, ptf3, ptf4);
    std::vector<ProcessTagInfo*>* processTags = createProcessTagInfoVec();
    std::string errorMsg;
    ReissueSequence rs;
    rs.owrt() = RepriceSolutionValidator::OWRT_ONLY_OW_FARES;
    (*processTags)[0]->reissueSequence()->orig() = &rs;

    RepriceSolutionValidatorOverride rsv(*_trx, *fp);
    CPPUNIT_ASSERT(rsv.checkOWRT(*processTags, errorMsg));
  }

  void testCheckOWRT2()
  {
    PaxTypeFare* ptf1 = PaxTypeFareWithOWRT(ROUND_TRIP_MAYNOT_BE_HALVED);
    PaxTypeFare* ptf2 = PaxTypeFareWithOWRT(ROUND_TRIP_MAYNOT_BE_HALVED);
    PaxTypeFare* ptf3 = PaxTypeFareWithOWRT(ROUND_TRIP_MAYNOT_BE_HALVED);
    PaxTypeFare* ptf4 = PaxTypeFareWithOWRT(ROUND_TRIP_MAYNOT_BE_HALVED);
    FarePath* fp = FP(ptf1, ptf2, ptf3, ptf4);

    std::vector<ProcessTagInfo*>* processTags = createProcessTagInfoVec();
    std::string errorMsg;
    ReissueSequence rs;
    rs.owrt() = RepriceSolutionValidator::OWRT_ONLY_RT_FARES;
    (*processTags)[0]->reissueSequence()->orig() = &rs;

    RepriceSolutionValidatorOverride rsv(*_trx, *fp);
    CPPUNIT_ASSERT(rsv.checkOWRT(*processTags, errorMsg));
  }

  void testCheckOWRT3()
  {
    PaxTypeFare* ptf1 = PaxTypeFareWithOWRT(ONE_WAY_MAY_BE_DOUBLED);
    PaxTypeFare* ptf2 = PaxTypeFareWithOWRT(ONE_WAY_MAY_BE_DOUBLED);
    PaxTypeFare* ptf3 = PaxTypeFareWithOWRT(ONE_WAY_MAY_BE_DOUBLED);
    PaxTypeFare* ptf4 = PaxTypeFareWithOWRT(ROUND_TRIP_MAYNOT_BE_HALVED);
    FarePath* fp = FP(ptf1, ptf2, ptf3, ptf4);

    std::vector<ProcessTagInfo*>* processTags = createProcessTagInfoVec();
    std::string errorMsg;
    ReissueSequence rs;
    rs.owrt() = RepriceSolutionValidator::OWRT_ONLY_OW_FARES;
    (*processTags)[0]->reissueSequence()->orig() = &rs;

    RepriceSolutionValidatorOverride rsv(*_trx, *fp);
    CPPUNIT_ASSERT(!rsv.checkOWRT(*processTags, errorMsg));
  }

  void testCheckOWRT4()
  {
    PaxTypeFare* ptf1 = PaxTypeFareWithOWRT(ROUND_TRIP_MAYNOT_BE_HALVED);
    PaxTypeFare* ptf2 = PaxTypeFareWithOWRT(ONE_WAY_MAYNOT_BE_DOUBLED);
    PaxTypeFare* ptf3 = PaxTypeFareWithOWRT(ROUND_TRIP_MAYNOT_BE_HALVED);
    PaxTypeFare* ptf4 = PaxTypeFareWithOWRT(ROUND_TRIP_MAYNOT_BE_HALVED);
    FarePath* fp = FP(ptf1, ptf2, ptf3, ptf4);

    std::vector<ProcessTagInfo*>* processTags = createProcessTagInfoVec();
    std::string errorMsg;
    ReissueSequence rs;
    rs.owrt() = RepriceSolutionValidator::OWRT_ONLY_RT_FARES;
    (*processTags)[0]->reissueSequence()->orig() = &rs;

    RepriceSolutionValidatorOverride rsv(*_trx, *fp);
    CPPUNIT_ASSERT(!rsv.checkOWRT(*processTags, errorMsg));
  }

  void testCheckOWRT5()
  {
    PaxTypeFare* ptf1 = PaxTypeFareWithOWRT(ROUND_TRIP_MAYNOT_BE_HALVED);
    PaxTypeFare* ptf2 = PaxTypeFareWithOWRT(ONE_WAY_MAYNOT_BE_DOUBLED);
    PaxTypeFare* ptf3 = PaxTypeFareWithOWRT(ROUND_TRIP_MAYNOT_BE_HALVED);
    PaxTypeFare* ptf4 = PaxTypeFareWithOWRT(ONE_WAY_MAY_BE_DOUBLED);
    FarePath* fp = FP(ptf1, ptf2, ptf3, ptf4);

    std::vector<ProcessTagInfo*>* processTags = createProcessTagInfoVec();
    std::string errorMsg;
    ReissueSequence rs;
    rs.owrt() = RepriceSolutionValidator::OWRT_ALL_FARES_AVAILABLE;
    (*processTags)[0]->reissueSequence()->orig() = &rs;

    RepriceSolutionValidatorOverride rsv(*_trx, *fp);
    CPPUNIT_ASSERT(rsv.checkOWRT(*processTags, errorMsg));
  }

  void testCheckTagsMixedTags_ExcKKK111_NewKKKC()
  {
    PaxTypeFare* paxType1 = addWarResultToMaps(KEEP, KEEP_THE_FARES);
    PaxTypeFare* paxType2 = addWarResultToMaps(KEEP, KEEP_THE_FARES);
    PaxTypeFare* paxType3 = addWarResultToMaps(KEEP, KEEP_THE_FARES);

    setFirstFmForCheckMixedTags(paxType1->fareMarket(), tse::UU);

    FareMarket* fm1 = addExcFcToNewItinKeepFares(paxType1);
    FareMarket* fm2 = addExcFcToNewItinKeepFares(paxType2);
    FareMarket* fm3 = addExcFcToNewItinKeepFares(paxType3);

    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm1);
    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm2);
    addNewFcToAllRepricePTF(FareMarket::RetrievKeep, fm3);
    addNewFcToAllRepricePTF(FareMarket::RetrievCurrent);

    CPPUNIT_ASSERT(_validator->checkTagsForMixedTags(*_permutation, *_msg));
  }

  void testvec3Cache()
  {
    const uint16_t fareCompNumber = 11;
    const uint32_t rec3ItemNo = 22;

    const pair<int, int> key = make_pair(fareCompNumber, rec3ItemNo);

    _validator->_record3Cache.insert(make_pair(key, true));

    map<pair<int, int>, bool>::const_iterator rec3CacheI = _validator->_record3Cache.find(key);

    CPPUNIT_ASSERT(_validator->_record3Cache.size() == 1);
    CPPUNIT_ASSERT(rec3CacheI->second);
  }

  void testLatestBookingDate()
  {
    string date = "2007-01-01 11:11";
    AirSeg ts1;
    ts1.bookingDT() = DateTime(date);

    date = "2007-01-02 11:11";
    AirSeg ts2;
    ts2.bookingDT() = DateTime(date);

    date = "2007-01-05 11:11";
    AirSeg ts3; // this one have latest booking date
    ts3.bookingDT() = DateTime(date);

    date = "2007-01-03 11:11";
    AirSeg ts4;
    ts4.bookingDT() = DateTime(date);

    date = "2007-01-05 01:01";
    AirSeg ts5;
    ts5.bookingDT() = DateTime(date);

    PricingUnit pu1;
    pu1.travelSeg().push_back(&ts1);

    PricingUnit pu2;
    pu2.travelSeg().push_back(&ts2);
    pu2.travelSeg().push_back(&ts3);
    pu2.travelSeg().push_back(&ts4);

    PricingUnit pu3;
    pu3.travelSeg().push_back(&ts5);

    vector<PricingUnit*> puVec;
    puVec.push_back(&pu1);
    puVec.push_back(&pu2);
    puVec.push_back(&pu3);

    const DateTime* dt = _validator->latestBookingDate(puVec);

    CPPUNIT_ASSERT((*dt) == ts3.bookingDT());
  }

  void testValidateLatestBookingDateEffFalse()
  {
    DateOverrideRuleItem dori;

    string date = "2001-02-01 01:01";
    dori.resEffDate() = DateTime(date);

    date = "2001-01-20 01:01";
    dori.resDiscDate() = DateTime(date);

    RepriceSolutionValidatorOverride* rsvo =
        dynamic_cast<RepriceSolutionValidatorOverride*>(_validator);

    rsvo->_doriVector.push_back(&dori);

    date = "2001-01-01 01:01";
    DateTime latestBD = DateTime(date);

    VoluntaryChangesInfo vcRec3;
    VoluntaryChangesInfoW vcRec3W;
    vcRec3W.orig() = &vcRec3;

    vcRec3.vendor() = CARRIER_LH;
    vcRec3.overrideDateTblItemNo() = 1;
    vcRec3.itemNo() = 619;

    CPPUNIT_ASSERT(!_validator->validateLastBookingDate(&latestBD, vcRec3W));
  }

  void testValidateLatestBookingDateDiscFalse()
  {
    DateOverrideRuleItem dori;

    string date = "2000-01-01 01:01";
    dori.resEffDate() = DateTime(date);

    date = "2000-01-20 01:01";
    dori.resDiscDate() = DateTime(date);

    RepriceSolutionValidatorOverride* rsvo =
        dynamic_cast<RepriceSolutionValidatorOverride*>(_validator);

    rsvo->_doriVector.push_back(&dori);

    date = "2001-01-01 01:01";
    DateTime latestBD = DateTime(date);

    VoluntaryChangesInfo vcRec3;
    VoluntaryChangesInfoW vcRec3W;
    vcRec3W.orig() = &vcRec3;

    vcRec3.vendor() = CARRIER_LH;
    vcRec3.overrideDateTblItemNo() = 1;
    vcRec3.itemNo() = 619;

    CPPUNIT_ASSERT(!_validator->validateLastBookingDate(&latestBD, vcRec3W));
  }

  void testValidateLatestBookingDateTrue()
  {
    DateOverrideRuleItem dori;

    string date = "2000-01-01 01:01";
    dori.resEffDate() = DateTime(date);

    date = "2001-01-20 01:01";
    dori.resDiscDate() = DateTime(date);

    RepriceSolutionValidatorOverride* rsvo =
        dynamic_cast<RepriceSolutionValidatorOverride*>(_validator);

    rsvo->_doriVector.push_back(&dori);

    date = "2001-01-01 01:01";
    DateTime latestBD = DateTime(date);

    VoluntaryChangesInfo vcRec3;
    VoluntaryChangesInfoW vcRec3W;
    vcRec3W.orig() = &vcRec3;

    vcRec3.vendor() = CARRIER_LH;
    vcRec3.overrideDateTblItemNo() = 1;
    vcRec3.itemNo() = 619;

    CPPUNIT_ASSERT(_validator->validateLastBookingDate(&latestBD, vcRec3W));
  }

  void testLatestBookingDateSimple()
  {
    string date = "2007-01-01 11:11";
    AirSeg ts1;
    ts1.bookingDT() = DateTime(date);

    date = "2007-01-05 11:11";
    AirSeg ts3; // this one have latest booking date
    ts3.bookingDT() = DateTime(date);

    date = "2007-01-05 01:01";
    AirSeg ts5;
    ts5.bookingDT() = DateTime(date);

    vector<TravelSeg*> travelSegVec;

    travelSegVec.push_back(&ts1);
    travelSegVec.push_back(&ts3);
    travelSegVec.push_back(&ts5);

    const DateTime* dt = _validator->latestBookingDate(travelSegVec);

    CPPUNIT_ASSERT((*dt) == ts3.bookingDT());
  }
  void testFCBookingDateValidationFalse()
  {
    DateOverrideRuleItem dori;

    string date = "2000-01-01 01:01";
    dori.resEffDate() = DateTime(date);

    date = "2000-01-20 01:01";
    dori.resDiscDate() = DateTime(date);

    RepriceSolutionValidatorOverride* rsvo =
        dynamic_cast<RepriceSolutionValidatorOverride*>(_validator);

    rsvo->_doriVector.push_back(&dori);

    VoluntaryChangesInfo vcRec3;
    VoluntaryChangesInfoW vcRec3W;
    vcRec3W.orig() = &vcRec3;

    vcRec3.vendor() = CARRIER_LH;
    vcRec3.overrideDateTblItemNo() = 1;
    vcRec3.itemNo() = 619;

    date = "2001-01-01 01:01";
    AirSeg ts5;
    ts5.bookingDT() = DateTime(date);

    FareMarket fm;
    fm.travelSeg().push_back(&ts5);

    createPaxTypeFareWithFareAndFareInfo();
    _ptf->fareMarket() = &fm;

    vector<const PaxTypeFare*> mappedFCvec;
    mappedFCvec.push_back(_ptf);

    CPPUNIT_ASSERT(!_validator->fareComponentBookingDateValidation(mappedFCvec, vcRec3W));
  }
  void testFCBookingDateValidationTrue()
  {
    DateOverrideRuleItem dori;

    string date = "2000-01-01 01:01";
    dori.resEffDate() = DateTime(date);

    date = "2003-01-20 01:01";
    dori.resDiscDate() = DateTime(date);

    RepriceSolutionValidatorOverride* rsvo =
        dynamic_cast<RepriceSolutionValidatorOverride*>(_validator);

    rsvo->_doriVector.push_back(&dori);

    VoluntaryChangesInfo vcRec3;
    VoluntaryChangesInfoW vcRec3W;
    vcRec3W.orig() = &vcRec3;

    vcRec3.vendor() = CARRIER_LH;
    vcRec3.overrideDateTblItemNo() = 1;
    vcRec3.itemNo() = 619;

    date = "2001-01-01 01:01";
    AirSeg ts5;
    ts5.bookingDT() = DateTime(date);

    FareMarket fm;
    fm.travelSeg().push_back(&ts5);

    createPaxTypeFareWithFareAndFareInfo();
    _ptf->fareMarket() = &fm;

    vector<const PaxTypeFare*> mappedFCvec;
    mappedFCvec.push_back(_ptf);

    CPPUNIT_ASSERT(_validator->fareComponentBookingDateValidation(mappedFCvec, vcRec3W));
  }
  void testJourneyBookingDateValidationFalse()
  {
    DateOverrideRuleItem dori;

    string date = "2000-01-01 01:01";
    dori.resEffDate() = DateTime(date);

    date = "2000-01-20 01:01";
    dori.resDiscDate() = DateTime(date);

    RepriceSolutionValidatorOverride* rsvo =
        dynamic_cast<RepriceSolutionValidatorOverride*>(_validator);

    rsvo->_doriVector.push_back(&dori);

    VoluntaryChangesInfo vcRec3;
    VoluntaryChangesInfoW vcRec3W;
    vcRec3W.orig() = &vcRec3;

    vcRec3.vendor() = CARRIER_LH;
    vcRec3.overrideDateTblItemNo() = 1;
    vcRec3.itemNo() = 619;

    date = "2001-01-05 01:01";
    AirSeg ts5;
    ts5.bookingDT() = DateTime(date);

    PricingUnit pu;
    pu.travelSeg().push_back(&ts5);
    _farePath->pricingUnit().push_back(&pu);

    CPPUNIT_ASSERT(!_validator->journeyBookingDateValidation(vcRec3W));
  }
  void testJourneyBookingDateValidationTrue()
  {
    DateOverrideRuleItem dori;

    string date = "2000-01-01 01:01";
    dori.resEffDate() = DateTime(date);

    date = "2003-01-20 01:01";
    dori.resDiscDate() = DateTime(date);

    RepriceSolutionValidatorOverride* rsvo =
        dynamic_cast<RepriceSolutionValidatorOverride*>(_validator);

    rsvo->_doriVector.push_back(&dori);

    VoluntaryChangesInfo vcRec3;
    VoluntaryChangesInfoW vcRec3W;
    vcRec3W.orig() = &vcRec3;

    vcRec3.vendor() = CARRIER_LH;
    vcRec3.overrideDateTblItemNo() = 1;
    vcRec3.itemNo() = 619;

    date = "2001-01-01 01:01";
    AirSeg ts5;
    ts5.bookingDT() = DateTime(date);

    PricingUnit pu;
    pu.travelSeg().push_back(&ts5);
    _farePath->pricingUnit().push_back(&pu);

    CPPUNIT_ASSERT(_validator->journeyBookingDateValidation(vcRec3W));
  }

  void testOverrideReservationDatesNoMapping()
  {
    createPtiWithT988();
    DateOverrideRuleItem dori;

    string date = "2000-01-01 01:01";
    dori.resEffDate() = DateTime(date);

    date = "2003-01-20 01:01";
    dori.resDiscDate() = DateTime(date);

    RepriceSolutionValidatorOverride* rsvo =
        dynamic_cast<RepriceSolutionValidatorOverride*>(_validator);

    rsvo->_doriVector.push_back(&dori);

    VoluntaryChangesInfo vcRec3;
    VoluntaryChangesInfoW vcRec3W;
    vcRec3W.orig() = &vcRec3;

    vcRec3.vendor() = CARRIER_LH;
    vcRec3.overrideDateTblItemNo() = 1;
    vcRec3.itemNo() = 619;

    date = "2005-01-01 01:01";
    AirSeg ts5;
    ts5.bookingDT() = DateTime(date);

    PricingUnit pu;
    pu.travelSeg().push_back(&ts5);
    _farePath->pricingUnit().push_back(&pu);

    _pti->record3()->orig() = &vcRec3;

    FareCompInfo fc;
    fc.fareCompNumber() = 1;
    _pti->fareCompInfo() = &fc;

    _trx->exchangeItin().front()->fareComponent().push_back(&fc);

    _permutation->processTags().push_back(_pti);

    rsvo->_fcMapping.resize(1);
    rsvo->_fcMapping[0].clear();

    CPPUNIT_ASSERT(!rsvo->checkOverrideReservationDates(*_permutation));
  }

  void testOverrideReservationDatesWithMapping()
  {
    createPtiWithT988();
    DateOverrideRuleItem dori;

    string date = "2000-01-01 01:01";
    dori.resEffDate() = DateTime(date);

    date = "2003-01-20 01:01";
    dori.resDiscDate() = DateTime(date);

    RepriceSolutionValidatorOverride* rsvo =
        dynamic_cast<RepriceSolutionValidatorOverride*>(_validator);

    rsvo->_doriVector.push_back(&dori);

    VoluntaryChangesInfo vcRec3;
    VoluntaryChangesInfoW vcRec3W;
    vcRec3W.orig() = &vcRec3;

    vcRec3.vendor() = CARRIER_LH;
    vcRec3.overrideDateTblItemNo() = 1;
    vcRec3.itemNo() = 619;

    date = "2005-01-01 01:01";
    AirSeg ts5;
    ts5.bookingDT() = DateTime(date);

    PricingUnit pu;
    pu.travelSeg().push_back(&ts5);
    _farePath->pricingUnit().push_back(&pu);

    _pti->record3()->orig() = &vcRec3;

    FareCompInfo fc;
    fc.fareCompNumber() = 1;
    _pti->fareCompInfo() = &fc;

    _trx->exchangeItin().front()->fareComponent().push_back(&fc);

    _permutation->processTags().push_back(_pti);

    vector<PricingUnit*> puVec;

    _farePath->pricingUnit() = puVec;

    createPaxTypeFareWithFareAndFareInfo();
    FareMarket fm;
    AirSeg ts;
    fm.travelSeg().push_back(&ts);
    _ptf->fareMarket() = &fm;

    rsvo->_fcMapping.resize(1);
    rsvo->_fcMapping[0].clear();
    rsvo->_fcMapping[0].push_back(_ptf);

    CPPUNIT_ASSERT(!rsvo->checkOverrideReservationDates(*_permutation));
  }

  void testIsBaseFareAmountPlusChangeFeeHigherPass()
  {
    _farePath->setTotalNUCAmount(50);
    MoneyAmount changeFee = 55;
    _excFarePath->setTotalNUCAmount(100);
    // 50+55 > 100
    CPPUNIT_ASSERT(_validator->isBaseFareAmountPlusChangeFeeHigher(changeFee));
  }

  void testIsBaseFareAmountPlusChangeFeeHigherFail()
  {
    _farePath->setTotalNUCAmount(50);
    MoneyAmount changeFee = 25;
    _excFarePath->setTotalNUCAmount(100);
    // 50+25 !> 100
    CPPUNIT_ASSERT(!_validator->isBaseFareAmountPlusChangeFeeHigher(changeFee));

    changeFee = 50;
    // 50+50 !> 100
    CPPUNIT_ASSERT(!_validator->isBaseFareAmountPlusChangeFeeHigher(changeFee));
  }

  void testMatchJourney()
  {
    createPtiWithT988();
    _t988->journeyInd() = 'X';
    _permutation->processTags().push_back(_pti);
    _validator->_journeyCheckingFlag = true;

    CPPUNIT_ASSERT(!_validator->matchJourney(*_permutation));
  }

  void testIsPUPartiallyFlown()
  {
    AirSeg as1, as2;
    as1.unflown() = true;
    as2.unflown() = false;

    PricingUnit pu;
    pu.travelSeg().push_back(&as1);
    pu.travelSeg().push_back(&as2);

    CPPUNIT_ASSERT(_validator->RepriceSolutionValidator::isPUPartiallyFlown(&pu));
  }

  void testIsSameCpaCxrFareInExc()
  {
    FareMarket fm;
    fm.governingCarrier() = CARRIER_AA;
    fm.boardMultiCity() = "NYC";
    fm.offMultiCity() = "DFW";

    createPaxTypeFareWithFareAndFareInfo();
    _ptf->fareMarket() = &fm;

    FareUsage fu;
    fu.paxTypeFare() = _ptf;

    _trx->exchangeItin().front()->farePath().front()->flownOWFares().push_back(&fu);

    CPPUNIT_ASSERT(_validator->RepriceSolutionValidator::isSameCpaCxrFareInExc(&fu));
  }

  void testCollectFlownOWFares()
  {
    // this is little tricky way to set FM flown
    AirSeg as;
    as.unflown() = false;

    FareMarket fm;
    fm.travelSeg().push_back(&as);
    fm.setFCChangeStatus(5);
    // now we have fm flown

    createPaxTypeFareWithFareAndFareInfo();
    FareInfo fi;
    fi.owrt() = '1';
    _fare->setFareInfo(&fi);
    _ptf->fareMarket() = &fm;

    FareUsage fu;
    fu.paxTypeFare() = _ptf;

    PricingUnit pu;
    pu.fareUsage().push_back(&fu);

    vector<PricingUnit*> puVec;
    puVec.push_back(&pu);

    vector<FareUsage*> fuVec;
    _validator->collectFlownOWFares(puVec, fuVec);

    CPPUNIT_ASSERT(!fuVec.empty());
  }

  void testAnalysePricingUnits()
  {
    // this is little tricky way to set FM flown
    // and sets IsPUPartiallyFlown
    AirSeg as1, as2;
    as1.unflown() = true;
    as2.unflown() = false;

    FareMarket fm;
    fm.travelSeg().push_back(&as2);
    fm.setFCChangeStatus(5);
    // now we have fm flown

    createPaxTypeFareWithFareAndFareInfo();
    FareInfo fi;
    fi.owrt() = '2';
    _fare->setFareInfo(&fi);
    _ptf->fareMarket() = &fm;

    FareUsage fu;
    fu.paxTypeFare() = _ptf;

    // this sets IsSameCpaCxrFareInExc method
    fm.governingCarrier() = CARRIER_AA;
    fm.boardMultiCity() = "NYC";
    fm.offMultiCity() = "DFW";
    _trx->exchangeItin().front()->farePath().front()->flownOWFares().push_back(&fu);

    PricingUnit pu;
    pu.travelSeg().push_back(&as1);
    pu.travelSeg().push_back(&as2);
    pu.fareUsage().push_back(&fu);

    _farePath->pricingUnit().push_back(&pu);

    _validator->analysePricingUnits();

    CPPUNIT_ASSERT(_validator->_journeyCheckingFlag);
  }

  void testHasStopByte()
  {
    SequenceStopByteByTag sequenceStopByteByTag;
    ProcessTagInfo* processTagInfo(create<ProcessTagInfo>());
    ReissueSequence* reissueSequence(create<ReissueSequence>());
    processTagInfo->reissueSequence()->orig() = reissueSequence;
    reissueSequence->stopInd() = 'X';
    CPPUNIT_ASSERT(SequenceStopByteByTag::hasStopByte(processTagInfo));
    reissueSequence->stopInd() = ' ';
    CPPUNIT_ASSERT(!SequenceStopByteByTag::hasStopByte(processTagInfo));
  }

  void testIsSequenceGreater()
  {
    SequenceStopByteByTag sequenceStopByteByTag;

    set<pair<int, int>> sequences;
    sequences.insert(make_pair(1100, 1));
    sequences.insert(make_pair(1300, 1));

    ProcessTagInfo* processTagInfo(create<ProcessTagInfo>());
    processTagInfo->reissueSequence()->orig() = create<ReissueSequence>();
    CPPUNIT_ASSERT(sequenceStopByteByTag.isSequenceGreater(sequences, 1200));
    CPPUNIT_ASSERT(!sequenceStopByteByTag.isSequenceGreater(sequences, 1000));
  }

  void testStopByteSave()
  {
    SequenceStopByteByTag sequenceStopByteByTag;
    ProcessTagInfo* processTagInfo(create<ProcessTagInfo>());
    ReissueSequence* reissueSequence(create<ReissueSequence>());
    processTagInfo->reissueSequence()->orig() = reissueSequence;
    processTagInfo->fareCompInfo() = 0;
    reissueSequence->processingInd() = 1;
    reissueSequence->stopInd() = 'X';
    reissueSequence->seqNo() = 1200;
    ProcessTagPermutation permutation;
    permutation.processTags().push_back(processTagInfo);
    sequenceStopByteByTag.saveStopByteInfo(permutation);
    CPPUNIT_ASSERT(SequenceStopByteByTag::hasStopByte(processTagInfo));
    CPPUNIT_ASSERT(!sequenceStopByteByTag.skipByStopByte(permutation));
    pair<ProcessTag, FareCompInfo*> key((ProcessTag)1, (FareCompInfo*)0);
    int sequence(1200);
    CPPUNIT_ASSERT(sequenceStopByteByTag.hasSequence(
        sequenceStopByteByTag.tagFcWithStopBytes()[key], sequence));
  }

  void testStopByteSkip()
  {
    SequenceStopByteByTag sequenceStopByteByTag;
    ProcessTagInfo* processTagInfo(create<ProcessTagInfo>());
    ReissueSequence* reissueSequence(create<ReissueSequence>());

    processTagInfo->reissueSequence()->orig() = reissueSequence;
    processTagInfo->fareCompInfo() = 0;
    reissueSequence->processingInd() = 1;
    reissueSequence->stopInd() = 'X';
    reissueSequence->seqNo() = 1200;
    ProcessTagPermutation permutation;
    permutation.processTags().push_back(processTagInfo);
    sequenceStopByteByTag.saveStopByteInfo(permutation);
    CPPUNIT_ASSERT(SequenceStopByteByTag::hasStopByte(processTagInfo));
    CPPUNIT_ASSERT(!sequenceStopByteByTag.skipByStopByte(permutation));
    pair<ProcessTag, FareCompInfo*> key((ProcessTag)1, (FareCompInfo*)0);
    CPPUNIT_ASSERT(
        sequenceStopByteByTag.hasSequence(sequenceStopByteByTag.tagFcWithStopBytes()[key], 1200));

    reissueSequence->seqNo() = 1300;
    int sequence(1300);
    CPPUNIT_ASSERT(!sequenceStopByteByTag.hasSequence(
        sequenceStopByteByTag.tagFcWithStopBytes()[key], sequence));
    CPPUNIT_ASSERT(sequenceStopByteByTag.skipByStopByte(permutation));
  }

  void testStopByteSkipBypassed()
  {
    SequenceStopByteByTag sequenceStopByteByTag;
    ProcessTagInfo* processTagInfo(create<ProcessTagInfo>());
    ReissueSequence* reissueSequence(create<ReissueSequence>());

    processTagInfo->reissueSequence()->orig() = reissueSequence;
    processTagInfo->fareCompInfo() = 0;
    reissueSequence->processingInd() = 1;
    reissueSequence->stopInd() = 'X';
    reissueSequence->seqNo() = 1200;
    ProcessTagPermutation permutation;
    permutation.processTags().push_back(processTagInfo);
    sequenceStopByteByTag.saveStopByteInfo(permutation);
    CPPUNIT_ASSERT(SequenceStopByteByTag::hasStopByte(processTagInfo));
    CPPUNIT_ASSERT(!sequenceStopByteByTag.skipByStopByte(permutation));
    pair<ProcessTag, FareCompInfo*> key((ProcessTag)1, (FareCompInfo*)0);
    CPPUNIT_ASSERT(
        sequenceStopByteByTag.hasSequence(sequenceStopByteByTag.tagFcWithStopBytes()[key], 1200));

    reissueSequence->seqNo() = 1300;
    int sequence(1300);
    CPPUNIT_ASSERT(!sequenceStopByteByTag.hasSequence(
        sequenceStopByteByTag.tagFcWithStopBytes()[key], sequence));
    CPPUNIT_ASSERT(sequenceStopByteByTag.skipByStopByte(permutation));
  }

  void testStopByteSkipBypassedHasLowestChangeFee()
  {
    SequenceStopByteByTag sequenceStopByteByTag;
    _validator->_sequenceStopByteByTag = &sequenceStopByteByTag;
    ProcessTagPermutation permutation1;
    ProcessTagInfo processTagInfo1;
    ReissueSequence reissueSequence1;

    reissueSequence1.processingInd() = 1;
    reissueSequence1.stopInd() = 'X';
    reissueSequence1.seqNo() = 1200;

    processTagInfo1.reissueSequence()->orig() = &reissueSequence1;
    processTagInfo1.fareCompInfo() = 0;

    permutation1.processTags().push_back(&processTagInfo1);
    sequenceStopByteByTag.saveStopByteInfo(permutation1);
    CPPUNIT_ASSERT(SequenceStopByteByTag::hasStopByte(&processTagInfo1));
    CPPUNIT_ASSERT(!sequenceStopByteByTag.skipByStopByte(permutation1));
    pair<ProcessTag, FareCompInfo*> key((ProcessTag)1, (FareCompInfo*)0);
    CPPUNIT_ASSERT(
        sequenceStopByteByTag.hasSequence(sequenceStopByteByTag.tagFcWithStopBytes()[key], 1200));

    ProcessTagPermutation permutation2;
    ProcessTagInfo processTagInfo2;
    ReissueSequence reissueSequence2;
    reissueSequence2.processingInd() = 1;
    reissueSequence2.stopInd() = 'X';
    reissueSequence2.seqNo() = 1300;

    processTagInfo2.reissueSequence()->orig() = &reissueSequence2;
    processTagInfo2.fareCompInfo() = 0;

    permutation2.processTags().push_back(&processTagInfo2);
    pair<ProcessTag, FareCompInfo*> key2((ProcessTag)1, (FareCompInfo*)0);
    int sequence(1300);
    sequenceStopByteByTag.saveStopByteInfo(permutation2);
    CPPUNIT_ASSERT(sequenceStopByteByTag.hasSequence(
        sequenceStopByteByTag.tagFcWithStopBytes()[key2], sequence));
    CPPUNIT_ASSERT(sequenceStopByteByTag.skipByStopByte(permutation2));

    _validator->setLowestChangeFee(2);
    _validator->_trx.processTagPermutations().clear();
    _validator->_trx.processTagPermutations().push_back(&permutation1);
    _validator->_trx.processTagPermutations().push_back(&permutation2);
    CPPUNIT_ASSERT(1 == _validator->setLowestChangeFee(2));
    CPPUNIT_ASSERT(!abs(_validator->_lowestChangeFee - 19.99) < EPSILON);
    CPPUNIT_ASSERT(!(_validator->_permWithLowestChangeFee == &permutation2));
    // CPPUNIT_ASSERT( abs( _validator->_lowestChangeFee - 1999.99 ) < EPSILON );
    CPPUNIT_ASSERT(_validator->_permWithLowestChangeFee == &permutation1);
    _validator->_trx.processTagPermutations().clear();
    _validator->_sequenceStopByteByTag = 0;
  }

  void testCheckNewTicketEqualOrHigherPassOnBlank()
  {
    /*_permutation->processTags().push_back(
      createProcTagInfo(FC(FM(_ffExchangeItin->travelSeg()[0])),
      RepriceSolutionValidator::EXTEND_NO_RESTRICTIONS));

      _permutation->processTags().push_back(createProcTagInfo(fc1,
      RepriceSolutionValidator::EXTEND_AT_LEAST_ONE_POINT));
    */
    PermutationMock perm;
    CPPUNIT_ASSERT(_validator->checkNewTicketEqualOrHigher(perm));
  }

  void testCheckNewTicketEqualOrHigherPassOnB()
  {
    PermutationMock perm(ProcessTagPermutation::NEW_TICKET_EQUAL_OR_HIGHER_B);
    _validator->_byte156ValueB = NEW_TICKET_EQUAL_OR_HIGHER_PASS;
    _farePath->minFareCheckDone() = true;
    CPPUNIT_ASSERT(_validator->checkNewTicketEqualOrHigher(perm));
  }

  void testCheckNewTicketEqualOrHigherPassOnN()
  {
    PermutationMock perm(ProcessTagPermutation::NEW_TICKET_EQUAL_OR_HIGHER_N);
    _validator->_byte156ValueN = NEW_TICKET_EQUAL_OR_HIGHER_PASS;
    CPPUNIT_ASSERT(_validator->checkNewTicketEqualOrHigher(perm));
  }

  void testCheckNewTicketEqualOrHigherFailOnB()
  {
    PermutationMock perm(ProcessTagPermutation::NEW_TICKET_EQUAL_OR_HIGHER_B);
    _validator->_byte156ValueB = NEW_TICKET_EQUAL_OR_HIGHER_FAIL;
    _farePath->minFareCheckDone() = true;
    CPPUNIT_ASSERT(!_validator->checkNewTicketEqualOrHigher(perm));
  }

  void testCheckNewTicketEqualOrHigherFailOnN()
  {
    PermutationMock perm(ProcessTagPermutation::NEW_TICKET_EQUAL_OR_HIGHER_N);
    _validator->_byte156ValueN = NEW_TICKET_EQUAL_OR_HIGHER_FAIL;
    CPPUNIT_ASSERT(!_validator->checkNewTicketEqualOrHigher(perm));
  }

  void testCheckNewTicketEqualOrHigherFailOnBWhenValidatingBN()
  {
    PermutationMock perm(ProcessTagPermutation::NEW_TICKET_EQUAL_OR_HIGHER_BN);
    _validator->_byte156ValueB = NEW_TICKET_EQUAL_OR_HIGHER_FAIL;
    _validator->_byte156ValueN = NEW_TICKET_EQUAL_OR_HIGHER_PASS;
    _farePath->minFareCheckDone() = true;
    CPPUNIT_ASSERT(!_validator->checkNewTicketEqualOrHigher(perm));
  }

  void testCheckNewTicketEqualOrHigherFailOnNWhenValidatingBN()
  {
    PermutationMock perm(ProcessTagPermutation::NEW_TICKET_EQUAL_OR_HIGHER_BN);
    _validator->_byte156ValueB = NEW_TICKET_EQUAL_OR_HIGHER_PASS;
    _validator->_byte156ValueN = NEW_TICKET_EQUAL_OR_HIGHER_FAIL;
    CPPUNIT_ASSERT(!_validator->checkNewTicketEqualOrHigher(perm));
  }

  void testCheckNewTicketEqualOrHigherPassWhenValidatingBN()
  {
    PermutationMock perm(ProcessTagPermutation::NEW_TICKET_EQUAL_OR_HIGHER_BN);
    _validator->_byte156ValueB = NEW_TICKET_EQUAL_OR_HIGHER_PASS;
    _validator->_byte156ValueN = NEW_TICKET_EQUAL_OR_HIGHER_PASS;
    _farePath->minFareCheckDone() = true;
    CPPUNIT_ASSERT(_validator->checkNewTicketEqualOrHigher(perm));
  }

  void testCheckNewTicketEqualOrHigherPassOnTotal()
  {
    _farePath->setTotalNUCAmount(300);
    FarePath FP;
    (_validator->_trx.exchangeItin().front())->farePath().push_back(&FP);
    _trx->exchangeItin().front()->farePath().front()->setTotalNUCAmount(200);
    PermutationMock perm(ProcessTagPermutation::NEW_TICKET_EQUAL_OR_HIGHER_B);
    _validator->_byte156ValueB = NEW_TICKET_EQUAL_OR_HIGHER_NOT_PROCESSED;
    _farePath->minFareCheckDone() = true;
    CPPUNIT_ASSERT(_validator->checkNewTicketEqualOrHigher(perm));
  }

  void testCheckNewTicketEqualOrHigherFailOnTotal()
  {
    _farePath->setTotalNUCAmount(200);
    _trx->totalFareCalcAmount() = 500.00;
    FarePath FP;
    (_validator->_trx.exchangeItin().front())->farePath().push_back(&FP);
    _trx->exchangeItin().front()->farePath().front()->setTotalNUCAmount(200.01);
    PermutationMock perm(ProcessTagPermutation::NEW_TICKET_EQUAL_OR_HIGHER_B);
    _validator->_byte156ValueB = NEW_TICKET_EQUAL_OR_HIGHER_NOT_PROCESSED;
    _farePath->minFareCheckDone() = true;
    CPPUNIT_ASSERT(!_validator->checkNewTicketEqualOrHigher(perm));
  }

  void testCheckNewTicketEqualOrHigherPassOnNonrefundable()
  {
    FarePathMock fpm(300);
    FarePathMock FP(200);
    RepriceSolutionValidator* validator =
        _memHandle.create<RepriceSolutionValidatorOverride>(*_trx, fpm);
    (validator->_trx.exchangeItin().front())->farePath().clear();
    (validator->_trx.exchangeItin().front())->farePath().push_back(&FP);
    PermutationMock perm(ProcessTagPermutation::NEW_TICKET_EQUAL_OR_HIGHER_N);
    validator->_byte156ValueN = NEW_TICKET_EQUAL_OR_HIGHER_NOT_PROCESSED;
    fpm.minFareCheckDone() = true;
    CPPUNIT_ASSERT(validator->checkNewTicketEqualOrHigher(perm));
  }

  void testCheckNewTicketEqualOrHigherFailOnNonrefundable()
  {
    _trx->setExcTktNonRefundable(true);
    _trx->totalFareCalcAmount() = 201.00;
    FarePathMock fpm(200);
    RepriceSolutionValidator* validator =
        _memHandle.insert(new RepriceSolutionValidatorOverride(*_trx, fpm));
    FarePathMock FP(201);
    (validator->_trx.exchangeItin().front())->farePath().clear();
    (validator->_trx.exchangeItin().front())->farePath().push_back(&FP);
    PermutationMock perm(ProcessTagPermutation::NEW_TICKET_EQUAL_OR_HIGHER_N);
    validator->_byte156ValueN = NEW_TICKET_EQUAL_OR_HIGHER_NOT_PROCESSED;
    fpm.minFareCheckDone() = true;
    CPPUNIT_ASSERT(!validator->checkNewTicketEqualOrHigher(perm));
  }

  void testIsChangedWhenStatusIsChanged()
  {
    AirSeg excSeg;
    excSeg.changeStatus() = TravelSeg::CHANGED;
    excSeg.isCabinChanged() = false;

    CPPUNIT_ASSERT(_validator->isTravelSegmentStatusChanged(*_ts2ss, excSeg));
  }

  void testIsChangedWhenStatusIsCabinChangedAndNotMappedTravelSegment()
  {
    AirSeg excSeg;
    excSeg.changeStatus() = TravelSeg::CHANGED;
    excSeg.isCabinChanged() = true;

    CPPUNIT_ASSERT(_validator->isTravelSegmentStatusChanged(*_ts2ss, excSeg));
  }

  void testIsChangedWhenStatusIsInventoryChangedAndEmptyRebookBookingCode()
  {
    AirSeg excSeg;

    PaxTypeFare::SegmentStatus ss;
    AirSeg newSeg;
    _ts2ss->insert(make_pair(&newSeg, &ss));
    excSeg.newTravelUsedToSetChangeStatus().push_back(&newSeg);

    excSeg.changeStatus() = TravelSeg::INVENTORYCHANGED;

    CPPUNIT_ASSERT(_validator->isTravelSegmentStatusChanged(*_ts2ss, excSeg));
  }

  void testIsChangedWhenStatusIsCabinChangedAndEmptyRebookBookingCode()
  {
    AirSeg excSeg;

    PaxTypeFare::SegmentStatus ss;
    AirSeg newSeg;
    _ts2ss->insert(make_pair(&newSeg, &ss));
    excSeg.newTravelUsedToSetChangeStatus().push_back(&newSeg);

    excSeg.changeStatus() = TravelSeg::CHANGED;
    excSeg.isCabinChanged() = true;

    CPPUNIT_ASSERT(_validator->isTravelSegmentStatusChanged(*_ts2ss, excSeg));
  }

  void testIsChangedWhenStatusIsUnchangedAndSameRebookBookingCode()
  {
    AirSeg excSeg;

    PaxTypeFare::SegmentStatus ss;
    AirSeg newSeg;
    _ts2ss->insert(make_pair(&newSeg, &ss));
    excSeg.newTravelUsedToSetChangeStatus().push_back(&newSeg);
    ss._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED);
    ss._bkgCodeReBook = "V";
    excSeg.setBookingCode("V");

    excSeg.changeStatus() = TravelSeg::UNCHANGED;

    CPPUNIT_ASSERT(!_validator->isTravelSegmentStatusChanged(*_ts2ss, excSeg));
  }

  void testIsChangedWhenStatusIsUnchangedAndDifferentRebookBookingCode()
  {
    AirSeg excSeg;

    PaxTypeFare::SegmentStatus ss;
    AirSeg newSeg;
    _ts2ss->insert(make_pair(&newSeg, &ss));
    excSeg.newTravelUsedToSetChangeStatus().push_back(&newSeg);
    ss._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED);
    ss._bkgCodeReBook = "V";
    excSeg.setBookingCode("Y");

    excSeg.changeStatus() = TravelSeg::UNCHANGED;

    CPPUNIT_ASSERT(_validator->isTravelSegmentStatusChanged(*_ts2ss, excSeg));
  }

  void testIsChangedWhenStatusIsUnchangedAndEmptyRebookBookingCode()
  {
    AirSeg excSeg;

    PaxTypeFare::SegmentStatus ss;
    AirSeg newSeg;
    _ts2ss->insert(make_pair(&newSeg, &ss));
    excSeg.newTravelUsedToSetChangeStatus().push_back(&newSeg);

    excSeg.changeStatus() = TravelSeg::UNCHANGED;

    CPPUNIT_ASSERT(!_validator->isTravelSegmentStatusChanged(*_ts2ss, excSeg));
  }

  void testGetFareApplicationWhenRebookSolution()
  {
    _validator->_isRebookSolution = true;
    PaxTypeFare excFare;
    FareMarket fareMarket;
    excFare.fareMarket() = &fareMarket;
    fareMarket.changeStatus() = UU;
    _permutation->setRebookFareTypeSelection(fareMarket.changeStatus(), KEEP);

    CPPUNIT_ASSERT(_validator->getFareApplication(*_permutation, excFare) == KEEP);
  }

  void testGetFareApplicationWhenAsbookSolution()
  {
    _validator->_isRebookSolution = false;
    PaxTypeFare excFare;
    _permutation->fareApplMap()[&excFare] = HISTORICAL;

    CPPUNIT_ASSERT(_validator->getFareApplication(*_permutation, excFare) == HISTORICAL);
  }

  void testRevalidateRules_PassCombFailWhenKeepFareB62XB72Set()
  {
    FareUsage fuNotKeep;
    _fuKeep->combinationFailedButSoftPassForKeepFare() = true;

    PricingUnit pu;
    pu.fareUsage().push_back(_fuKeep);
    pu.fareUsage().push_back(&fuNotKeep);

    map<const PaxTypeFare*, ProcessTagInfo*> keepFareProcessTags;
    ReissueSequence seq;
    seq.revalidationInd() = 'X'; // Byte 62 set
    setAllPrivision(seq, 'X');
    ProcessTagInfo tag;
    tag.reissueSequence()->orig() = &seq;
    keepFareProcessTags[_ptfKeep] = &tag;

    CPPUNIT_ASSERT(_validator->revalidateRules(pu, keepFareProcessTags));
  }

  void testRevalidateRules_FailCombFailWhenKeepFareB62YB72Set()
  {
    FareUsage fuNotKeep;
    _fuKeep->combinationFailedButSoftPassForKeepFare() = true;

    PricingUnit pu;
    pu.fareUsage().push_back(_fuKeep);
    pu.fareUsage().push_back(&fuNotKeep);

    map<const PaxTypeFare*, ProcessTagInfo*> keepFareProcessTags;
    ReissueSequence seq;
    seq.revalidationInd() = 'Y'; // Byte 62 set
    setAllPrivision(seq, ' ');
    seq.provision10() = 'X'; // Byte 72 set
    ProcessTagInfo tag;
    tag.reissueSequence()->orig() = &seq;
    keepFareProcessTags[_ptfKeep] = &tag;

    CPPUNIT_ASSERT(false == _validator->revalidateRules(pu, keepFareProcessTags));
  }

  void testRevalidateRules_FailCombFailWhenKeepFareB62XB72Blank()
  {
    FareUsage fuNotKeep;
    _fuKeep->combinationFailedButSoftPassForKeepFare() = true;

    PricingUnit pu;
    pu.fareUsage().push_back(_fuKeep);
    pu.fareUsage().push_back(&fuNotKeep);

    map<const PaxTypeFare*, ProcessTagInfo*> keepFareProcessTags;
    ReissueSequence seq;
    seq.revalidationInd() = 'X'; // Byte 62 set
    setAllPrivision(seq, 'X');
    seq.provision10() = ' '; // Byte 72 not set
    ProcessTagInfo tag;
    tag.reissueSequence()->orig() = &seq;
    keepFareProcessTags[_ptfKeep] = &tag;

    CPPUNIT_ASSERT(false == _validator->revalidateRules(pu, keepFareProcessTags));
  }

  void testRevalidateRules_FailCombFailWhenKeepFareB62Blank()
  {
    FareUsage fuNotKeep;
    _fuKeep->combinationFailedButSoftPassForKeepFare() = true;

    PricingUnit pu;
    pu.fareUsage().push_back(_fuKeep);
    pu.fareUsage().push_back(&fuNotKeep);

    map<const PaxTypeFare*, ProcessTagInfo*> keepFareProcessTags;
    ReissueSequence seq;
    seq.revalidationInd() = ' '; // Byte 62 blank
    setAllPrivision(seq, ' ');
    ProcessTagInfo tag;
    tag.reissueSequence()->orig() = &seq;
    keepFareProcessTags[_ptfKeep] = &tag;

    CPPUNIT_ASSERT(false == _validator->revalidateRules(pu, keepFareProcessTags));
  }

  void setAllPrivision(ReissueSequence& seq, const Indicator provision)
  {
    seq.provision1() = provision;
    seq.provision2() = provision;
    seq.provision3() = provision;
    seq.provision4() = provision;
    seq.provision5() = provision;
    seq.provision6() = provision;
    seq.provision7() = provision;
    seq.provision8() = provision;
    seq.provision9() = provision;
    seq.provision10() = provision;
    seq.provision11() = provision;
    seq.provision12() = provision;
    seq.provision13() = provision;
    seq.provision14() = provision;
    seq.provision15() = provision;
    seq.provision17() = provision;
    seq.provision18() = provision;
    seq.provision50() = provision;
  }

  void testApplyChangeFeeToFarePath()
  {
    ReissueCharges* reissueCharges = create<ReissueCharges>();
    _validator->_permutationReissueCharges[_validator->_permWithLowestChangeFee] = reissueCharges;
    _validator->_lowestChangeFee = MoneyAmount(100);
    _validator->_itinIndex = 0;

    _validator->applyChangeFeeToFarePath();

    CPPUNIT_ASSERT(_farePath->reissueCharges() == reissueCharges);
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(100), _farePath->rexChangeFee());
  }

  void testAnalyzeFUforAlreadyProcessedAndPassesdCat8Cat9_NoCat8Cat9ToRevalidate()
  {
    FareUsage fu;
    std::vector<uint16_t> vec;
    vec.push_back(2);
    vec.push_back(3);
    std::vector<uint16_t>::iterator cat = find(vec.begin(), vec.end(), 8);
    CPPUNIT_ASSERT(cat == vec.end());

    cat = find(vec.begin(), vec.end(), 9);
    CPPUNIT_ASSERT(cat == vec.end());

    _validator->analyzeFUforAlreadyProcessedAndPassesdCat8Cat9(fu, vec);
    CPPUNIT_ASSERT_EQUAL(size_t(2), vec.size());
  }

  void testAnalyzeFUforAlreadyProcessedAndPassesdCat8Cat9_Cat8Cat9NotProcessedBefore()
  {
    FareUsage fu;
    std::vector<uint16_t> vec;
    vec.push_back(2);
    vec.push_back(3);
    vec.push_back(8);
    vec.push_back(9);

    CPPUNIT_ASSERT_EQUAL(size_t(0), fu.stopoverSurcharges().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), fu.transferSurcharges().size());
    std::vector<uint16_t>::iterator cat = find(vec.begin(), vec.end(), 8);
    CPPUNIT_ASSERT(cat != vec.end());

    cat = find(vec.begin(), vec.end(), 9);
    CPPUNIT_ASSERT(cat != vec.end());

    _validator->analyzeFUforAlreadyProcessedAndPassesdCat8Cat9(fu, vec);
    CPPUNIT_ASSERT_EQUAL(size_t(4), vec.size());

    cat = find(vec.begin(), vec.end(), 8);
    CPPUNIT_ASSERT(cat != vec.end());

    cat = find(vec.begin(), vec.end(), 9);
    CPPUNIT_ASSERT(cat != vec.end());
  }

  void testAnalyzeFUforAlreadyProcessedAndPassesdCat8Cat9_Cat8IsProcessedBefore()
  {
    FareUsage fu;
    FareUsage::StopoverSurcharge* sur = new FareUsage::StopoverSurcharge();
    FareUsage::StopoverSurchargeMultiMap::value_type value(
        static_cast<TravelSeg*>(_firstExcTravelSeg), sur);
    fu.stopoverSurcharges().insert(value);

    std::vector<uint16_t> vec;
    vec.push_back(2);
    vec.push_back(3);
    vec.push_back(8);
    vec.push_back(9);

    CPPUNIT_ASSERT_EQUAL(size_t(1), fu.stopoverSurcharges().size());
    std::vector<uint16_t>::iterator cat = find(vec.begin(), vec.end(), 8);
    CPPUNIT_ASSERT(cat != vec.end());

    cat = find(vec.begin(), vec.end(), 9);
    CPPUNIT_ASSERT(cat != vec.end());

    _validator->analyzeFUforAlreadyProcessedAndPassesdCat8Cat9(fu, vec);
    CPPUNIT_ASSERT_EQUAL(size_t(3), vec.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu.stopoverSurcharges().size());

    cat = find(vec.begin(), vec.end(), 8);
    CPPUNIT_ASSERT(cat == vec.end());

    cat = find(vec.begin(), vec.end(), 9);
    CPPUNIT_ASSERT(cat != vec.end());
  }

  void testAnalyzeFUforAlreadyProcessedAndPassesdCat8Cat9_Cat9IsProcessedBefore()
  {
    FareUsage fu;
    FareUsage::TransferSurcharge* sur = new FareUsage::TransferSurcharge();
    FareUsage::TransferSurchargeMultiMap::value_type value(
        static_cast<TravelSeg*>(_firstExcTravelSeg), sur);
    fu.transferSurcharges().insert(value);

    std::vector<uint16_t> vec;
    vec.push_back(2);
    vec.push_back(3);
    vec.push_back(8);
    vec.push_back(9);

    CPPUNIT_ASSERT_EQUAL(size_t(1), fu.transferSurcharges().size());
    CPPUNIT_ASSERT_EQUAL(size_t(0), fu.stopoverSurcharges().size());
    std::vector<uint16_t>::iterator cat = find(vec.begin(), vec.end(), 8);
    CPPUNIT_ASSERT(cat != vec.end());

    cat = find(vec.begin(), vec.end(), 9);
    CPPUNIT_ASSERT(cat != vec.end());

    _validator->analyzeFUforAlreadyProcessedAndPassesdCat8Cat9(fu, vec);
    CPPUNIT_ASSERT_EQUAL(size_t(3), vec.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu.transferSurcharges().size());

    cat = find(vec.begin(), vec.end(), 8);
    CPPUNIT_ASSERT(cat != vec.end());
    cat = find(vec.begin(), vec.end(), 9);
    CPPUNIT_ASSERT(cat == vec.end());
  }

  void testAnalyzeFUforAlreadyProcessedAndPassesdCat8Cat9_Cat8And9AreProcessedBefore()
  {
    FareUsage fu;
    FareUsage::StopoverSurcharge* surs = new FareUsage::StopoverSurcharge();
    FareUsage::StopoverSurchargeMultiMap::value_type values(
        static_cast<TravelSeg*>(_firstExcTravelSeg), surs);
    fu.stopoverSurcharges().insert(values);
    FareUsage::TransferSurcharge* surf = new FareUsage::TransferSurcharge();
    FareUsage::TransferSurchargeMultiMap::value_type valuef(
        static_cast<TravelSeg*>(_firstExcTravelSeg), surf);
    fu.transferSurcharges().insert(valuef);

    CPPUNIT_ASSERT_EQUAL(size_t(1), fu.stopoverSurcharges().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu.transferSurcharges().size());
    std::vector<uint16_t> vec;
    vec.push_back(2);
    vec.push_back(3);
    vec.push_back(8);
    vec.push_back(9);

    std::vector<uint16_t>::iterator cat = find(vec.begin(), vec.end(), 8);
    CPPUNIT_ASSERT(cat != vec.end());

    cat = find(vec.begin(), vec.end(), 9);
    CPPUNIT_ASSERT(cat != vec.end());

    _validator->analyzeFUforAlreadyProcessedAndPassesdCat8Cat9(fu, vec);
    CPPUNIT_ASSERT_EQUAL(size_t(2), vec.size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu.stopoverSurcharges().size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), fu.transferSurcharges().size());

    cat = find(vec.begin(), vec.end(), 8);
    CPPUNIT_ASSERT(cat == vec.end());

    cat = find(vec.begin(), vec.end(), 9);
    CPPUNIT_ASSERT(cat == vec.end());
  }

  void testGetNewTicketAmountInNUCWhenNoExchangeReissue()
  {
    _farePath->itin() = _ffNewItin;
    _ffNewItin->originationCurrency() = "USD";
    _ffNewItin->calculationCurrency() = "USD";
    _farePath->increaseTotalNUCAmount(200);
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(200), _validator->getNewTicketAmount());
  }

  void testGetNewTicketAmountInNUCWhenExchangeReissueAndBaseCurrEqualCaclulationCurr()
  {
    _trx->setRexPrimaryProcessType('A');
    _farePath->itin() = _ffNewItin;
    _ffNewItin->originationCurrency() = "USD";
    _ffNewItin->calculationCurrency() = "USD";
    _farePath->increaseTotalNUCAmount(200);
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(200), _validator->getNewTicketAmount());
  }

  void testGetExchTotalAmountNoReissueExchange()
  {
    _farePath->increaseTotalNUCAmount(205);
    (_validator->_trx.exchangeItin().front())->farePath().clear();
    (_validator->_trx.exchangeItin().front())->farePath().push_back(_farePath);
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(205), _validator->getExchTotalAmount());
  }

  void testGetExchTotalAmountIsReissueExchangeAndC5ANotEmpty()
  {
    _trx->setRexPrimaryProcessType('A');
    _farePath->increaseTotalNUCAmount(210);
    (_validator->_trx.exchangeItin().front())->farePath().clear();
    (_validator->_trx.exchangeItin().front())->farePath().push_back(_farePath);
    RexPricingOptions rexOptions;
    rexOptions.excTotalFareAmt() = "205.00";
    _trx->setOptions(&rexOptions);
    CPPUNIT_ASSERT_EQUAL(MoneyAmount(205), _validator->getExchTotalAmount());
  }

  void testGetExchTotalAmountIsReissueExchangeAndC5IsEmpty()
  {
    _trx->setRexPrimaryProcessType('A');
    _farePath->increaseTotalNUCAmount(210);
    (_validator->_trx.exchangeItin().front())->farePath().clear();
    (_validator->_trx.exchangeItin().front())->farePath().push_back(_farePath);
    RexPricingOptions rexOptions;
    rexOptions.excTotalFareAmt() = "";
    _trx->setOptions(&rexOptions);

    ErrorResponseException err(ErrorResponseException::NO_ERROR);
    try
    {
      _validator->getExchTotalAmount();
    }
    catch (const ErrorResponseException& catched)
    {
      err = catched.code();
    }

    CPPUNIT_ASSERT_EQUAL(ErrorResponseException::REISSUE_RULES_FAIL, err.code());
  }

  void testGetCurrencyNonExchReissie()
  {
    _validator->calcCurrCode() = "GBP";
    CPPUNIT_ASSERT_EQUAL(CurrencyCode("GBP"), _validator->getCurrency());
  }

  void testGetCurrencyForExchReissie()
  {
    _farePath->itin() = _ffNewItin;
    _trx->setRexPrimaryProcessType('A');
    _validator->calcCurrCode() = "GBP";
    _ffNewItin->originationCurrency() = "NZD";

    CPPUNIT_ASSERT_EQUAL(CurrencyCode("NZD"), _validator->getCurrency());
  }

  FareMarket* get()
  {
    FareMarket* fm = _memHandle(new FareMarket);
    for (int i = 0; i != 3; ++i)
    {
      fm->travelSeg().push_back(_memHandle(new AirSeg));
      fm->travelSeg().back()->setBookingCode("PL");
    }
    return fm;
  }

  FareMarket* getNew(bool rebook)
  {
    _farePath->pricingUnit().push_back(create<PricingUnit>());
    _farePath->pricingUnit().front()->fareUsage().push_back(create<FareUsage>());
    _farePath->pricingUnit().front()->fareUsage().front()->paxTypeFare() = create<PaxTypeFare>();

    for (int i = 0; i != 3; ++i)
      _farePath->pricingUnit().front()->fareUsage().front()->segmentStatus().push_back(
          *create<PaxTypeFare::SegmentStatus>());

    _farePath->pricingUnit().front()->fareUsage().front()->segmentStatus()[1]._bkgCodeReBook = "DL";

    if (rebook)
      _farePath->pricingUnit().front()->fareUsage().front()->segmentStatus()
          [1]._bkgCodeSegStatus.set(PaxTypeFare::BKSS_REBOOKED);

    FareMarket* fm = get();
    _farePath->pricingUnit().front()->fareUsage().front()->paxTypeFare()->fareMarket() = fm;

    return fm;
  }

  void setUpAnyFailAtZeroT988()
  {
    _pti = create<ProcessTagInfo>();
    _pu = create<PricingUnit>();
  }

  void testAnyFailAtZeroT988NotFail()
  {
    setUpAnyFailAtZeroT988();
    CPPUNIT_ASSERT(!_validator->anyFailAtZeroT988(_pti, *_fuKeep, *_pu));
  }
  void testAnyFailAtZeroT988PuRuleFail()
  {
    setUpAnyFailAtZeroT988();
    _pu->ruleFailedButSoftPassForKeepFare() = true;
    CPPUNIT_ASSERT(_validator->anyFailAtZeroT988(_pti, *_fuKeep, *_pu));
  }
  void testAnyFailAtZeroT988PuCombFail()
  {
    setUpAnyFailAtZeroT988();
    _pu->combinationFailedButSoftPassForKeepFare() = true;
    CPPUNIT_ASSERT(_validator->anyFailAtZeroT988(_pti, *_fuKeep, *_pu));
  }
  void testAnyFailAtZeroT988FuRuleFail()
  {
    setUpAnyFailAtZeroT988();
    _fuKeep->ruleFailed() = true;
    _fuKeep->isKeepFare() = true;
    CPPUNIT_ASSERT(_validator->anyFailAtZeroT988(_pti, *_fuKeep, *_pu));
  }
  void testAnyFailAtZeroT988FuCombFail()
  {
    setUpAnyFailAtZeroT988();
    _fuKeep->combinationFailedButSoftPassForKeepFare() = true;
    CPPUNIT_ASSERT(_validator->anyFailAtZeroT988(_pti, *_fuKeep, *_pu));
  }

  void testMatchNewFcWithFa()
  {
    std::vector<bool> matchedNewFc(3, true);
    CPPUNIT_ASSERT(_validator->matchNewFcWithFa(matchedNewFc, FareMarket::RetrievKeep));
  }

  void setUpOutboundPortionOfTravel(Indicator outInd)
  {
    createPtiWithT988();
    _t988->outboundInd() = outInd;
    _pti->paxTypeFare() = _ptfKeep;
    _pu = create<PricingUnit>();
    _excFarePath->pricingUnit().push_back(_pu);
    _pu->fareUsage().push_back(_fuKeep);
  }

  void setUnchangedSegment()
  {
    _firstExcTravelSeg->changeStatusVec()[0] = TravelSeg::UNCHANGED;
    _firstExcTravelSeg->unflown() = false;
  }

  void testOutboundOfTravelSegmentsBlank()
  {
    setUpOutboundPortionOfTravel(ProcessTagInfo::NO_RESTRICTION);
    CPPUNIT_ASSERT(_validator->matchOutboundPortionOfTvl(*_pti, 0, false, 0));
  }

  void testOutboundOfTravelSegmentsFirstChanged()
  {
    setUpOutboundPortionOfTravel(ProcessTagInfo::FIRST_FC);
    _fuKeep->travelSeg().push_back(_firstExcTravelSeg);
    CPPUNIT_ASSERT(!_validator->matchOutboundPortionOfTvl(*_pti, 0, false, 0));
  }

  void testOutboundOfTravelSegmentsOrigChanged()
  {
    setUpOutboundPortionOfTravel(ProcessTagInfo::ORIG_TO_STOPOVER);
    _pu->travelSeg().push_back(_firstExcTravelSeg);
    CPPUNIT_ASSERT(!_validator->matchOutboundPortionOfTvl(*_pti, 0, false, 0));
  }

  void testOutboundOfTravelSegmentsFirstUnchanged()
  {
    setUpOutboundPortionOfTravel(ProcessTagInfo::FIRST_FC);
    setUnchangedSegment();
    _fuKeep->travelSeg().push_back(_firstExcTravelSeg);
    CPPUNIT_ASSERT(_validator->matchOutboundPortionOfTvl(*_pti, 0, false, 0));
  }

  void testOutboundOfTravelSegmentsOrigUnchanged()
  {
    setUpOutboundPortionOfTravel(ProcessTagInfo::ORIG_TO_STOPOVER);
    setUnchangedSegment();
    _pu->travelSeg().push_back(_firstExcTravelSeg);
    CPPUNIT_ASSERT(_validator->matchOutboundPortionOfTvl(*_pti, 0, false, 0));
  }

private:
  class MockRexPricingTrx : public RexPricingTrx
  {
  public:
    virtual Money
    convertCurrency(const Money& source, const CurrencyCode& targetCurr, bool rounding) const
    {
      if ((source.code() == NUC && targetCurr == USD) ||
          (source.code() == USD && targetCurr == NUC))
        return Money(source.value(), targetCurr);

      return Money(source.value() * 2, targetCurr);
    }
  };

  RexPricingTrx* _trx;
  RepriceSolutionValidator* _validator;
  FarePath* _farePath;
  FarePath* _excFarePath;
  ExcItin* _exchangeItin;
  ProcessTagPermutation* _permutation;
  ExcItin* _ffExchangeItin;
  Itin* _ffNewItin;
  ExcItin* _arExchangeItin;
  Itin* _arNewItin;
  AirSeg* _firstExcTravelSeg;
  std::map<TravelSeg*, PaxTypeFare::SegmentStatus*>* _ts2ss;
  vector<TravelSeg*>* _excItinSegs;
  ProcessTagInfo* _pti;
  ReissueSequence* _t988;
  FareInfo* _fareInfo;
  FareInfo* _excFareInfo;
  Fare* _fare;
  Fare* _excFare;
  PaxTypeFare* _ptf;
  PaxTypeFare* _excPtf;
  FareMarket* _fm;
  PaxTypeFare* _ptfKeep;
  FareUsage* _fuKeep;
  TestMemHandle _memHandle;
  string* _msg;
  PricingUnit* _pu;
  RexBaseRequest* _rexBaseRequest;
  Agent* _agent;
  Customer* _cus;
};

CPPUNIT_TEST_SUITE_REGISTRATION(RepriceSolutionValidatorTest);
} //  tse
