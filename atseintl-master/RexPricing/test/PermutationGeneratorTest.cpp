#include "test/include/CppUnitHelperMacros.h"
#include <boost/assign/list_of.hpp>
#include "RexPricing/PermutationGenerator.h"
#include "DataModel/ExcItin.h"
#include "DataModel/ProcessTagPermutation.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/FareUsage.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/AirSeg.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/Loc.h"
#include "DBAccess/ReissueSequence.h"
#include <iostream>
#include "test/include/TestMemHandle.h"
#include "test/include/TestFallbackUtil.h"

namespace tse
{
using boost::assign::list_of;

class PermutationGeneratorOverride : public PermutationGenerator
{
public:
  PermutationGeneratorOverride(RexPricingTrx& trx) : PermutationGenerator(trx) {}
  std::pair<bool, bool> _validSeqMixed;

  bool
  isProperTagsSequence(CartesianProduct<std::vector<ProcessTagInfo*> >::ProductType& permSelector)
  {
    std::string s;
    _validSeqMixed = permCharacteristic(permSelector, s);
    return PermutationGenerator::isProperTagsSequence(permSelector, s);
  }
};

class PermutationGeneratorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(PermutationGeneratorTest);

  CPPUNIT_TEST(testIsProperTagsSequenceValidSequenceAllTags7);
  CPPUNIT_TEST(testIsProperTagsSequenceValidSequenceNoTags7);
  CPPUNIT_TEST(testIsProperTagsSequenceNotValidSequenceOnlyFirstTagIs7);
  CPPUNIT_TEST(testIsProperTagsSequenceNotValidSequenceOnlyLastTagIs7);
  CPPUNIT_TEST(testIsProperTagsSequenceNotValidSequenceTagInTheMiddleIs7);

  CPPUNIT_TEST(testMarkFareRetrievalForNeededFareApplication);
  CPPUNIT_TEST(testNotMarkFareRetrievalForUnneededFareApplication);
  CPPUNIT_TEST(testTagWarResultFareApplicationSaved);
  CPPUNIT_TEST(testNewFmMatchToExcFm);
  CPPUNIT_TEST(testNewFmNotMatchToExcFmDueToCity);
  CPPUNIT_TEST(testNewFmNotMatchToExcFmDueToChangeStatus);
  CPPUNIT_TEST(testNewFmNotMatchToExcFmDueToCxr);

  CPPUNIT_TEST(testElectronicTicketPassOnBlank);
  CPPUNIT_TEST(testElectronicTicketPassOnR);
  CPPUNIT_TEST(testElectronicTicketPassOnN);
  CPPUNIT_TEST(testElectronicTicketFailOnMix);

  CPPUNIT_TEST(testMapKeepFareWhenKeepFareAndSameFareBreaks);
  CPPUNIT_TEST(testMapKeepFareWhenKeepFareAndDifferentFareBreaks);
  CPPUNIT_TEST(testMapKeepFareWhenHistFareAndSameFareBreaksInvChanged);
  CPPUNIT_TEST(testMapKeepFareWhenHistFareAndDifferentFareBreaksInvChanged);
  CPPUNIT_TEST(testMapKeepFareWhenHistFareAndSameFareBreaksFlightChanged);

  CPPUNIT_TEST(testSetReissueExchangeROEConversionDateWhenApplyReissueExchNotExists);
  CPPUNIT_TEST(testSetReissueExchangeROEConversionDateWhenExchangeForCurrentDatePass);
  CPPUNIT_TEST(testSetReissueExchangeROEConversionDateWhenExchangeForNonCurrentDateNoD95PassOrigin);
  CPPUNIT_TEST(testSetReissueExchangeROEConversionDateWhenReissueForCurrentDateOnlyPass);
  CPPUNIT_TEST(testSetReissueExchangeROEConversionDateWhenReissueForCurrentAndKeepDatesFail);
  CPPUNIT_TEST(
      testSetReissueExchangeROEConversionDateWhenReissueForCurrentAndKeepDatesD95PresentPass);
  CPPUNIT_TEST(
      testSetReissueExchangeROEConversionDateWhenReissueForCurrentAndKeepDatesNoD95PassOrigin);
  CPPUNIT_TEST(
      testSetReissueExchangeROEConversionDateSetsSecondROEDateToEmptyWhenOnlyCurrentFaresNeeded);
  CPPUNIT_TEST(
      testSetReissueExchangeROEConversionDateSetsSecondROEDateToPreviousDateWhenExchangeForCurrentFares);
  CPPUNIT_TEST(testSetReissueExchangeROEConversionDateSetsSecondROEDateToCurrentDateWhenMixedFares);

  CPPUNIT_TEST(testGetPreviousDateReturnOriginalIssueDateWhenPreviousDateEmpty);
  CPPUNIT_TEST(testGetPreviousDateReturnPreviousExchangeDateWhenNotEmpty);

  CPPUNIT_TEST(testMatchForEKeepAlreadyProcessed);
  CPPUNIT_TEST(testMatchForEKeepSize);
  CPPUNIT_TEST(testMatchForEKeepConstruction);
  CPPUNIT_TEST(testMatchForEKeepCxrDiff);
  CPPUNIT_TEST(testMatchForEKeepOrginNationDiff);
  CPPUNIT_TEST(testMatchForEKeepDestNationDiff);
  CPPUNIT_TEST(testMatchForEKeepGDDiff);

  CPPUNIT_TEST(testUpdatePermutationInfoForFareMarketsSetFlagWhenCurrentFlown);
  CPPUNIT_TEST(testUpdatePermutationInfoForFareMarketsSetFlagForKeepFlown);
  CPPUNIT_TEST(testUpdatePermutationInfoForFareMarketsSetFlagForHistoricalFlown);
  CPPUNIT_TEST(testUpdatePermutationInfoForFareMarketsSetFlagForTravelCommencementFlown);
  CPPUNIT_TEST(testUpdatePermutationInfoForFareMarketsSetFlagWhenCurrentUnflown);
  CPPUNIT_TEST(testUpdatePermutationInfoForFareMarketsSetFlagForKeepUnflown);
  CPPUNIT_TEST(testUpdatePermutationInfoForFareMarketsSetFlagForHistoricalUnflown);
  CPPUNIT_TEST(testUpdatePermutationInfoForFareMarketsSetFlagForTravelCommencementUnflown);

  CPPUNIT_TEST(testConnectionChanged);
  CPPUNIT_TEST(testStopOverChanged);
  CPPUNIT_TEST_SUITE_END();

protected:
  TestMemHandle _memH;
  PermutationGeneratorOverride* _permGenerator;
  RexPricingTrx* _trx;
  ExcItin* _exchangeItin;
  Itin* _newItin;
  AirSeg* _firstExcTravelSeg;

public:
  void testIsProperTagsSequenceValidSequenceAllTags7()
  {
    CartesianProduct<std::vector<ProcessTagInfo*> >::ProductType permSelector;

    AddProcessTag(permSelector, REISSUE_DOWN_TO_LOWER_FARE);
    AddProcessTag(permSelector, REISSUE_DOWN_TO_LOWER_FARE);
    AddProcessTag(permSelector, REISSUE_DOWN_TO_LOWER_FARE);
    AddProcessTag(permSelector, REISSUE_DOWN_TO_LOWER_FARE);

    CPPUNIT_ASSERT(_permGenerator->isProperTagsSequence(permSelector) == true);

    CPPUNIT_ASSERT(_permGenerator->_validSeqMixed.first);
    CPPUNIT_ASSERT(!_permGenerator->_validSeqMixed.second);

    ClearProcessTagPermutationData(permSelector);
  }

  void testIsProperTagsSequenceValidSequenceNoTags7()
  {
    CartesianProduct<std::vector<ProcessTagInfo*> >::ProductType permSelector;

    AddProcessTag(permSelector, KEEP_THE_FARES);
    AddProcessTag(permSelector, GUARANTEED_AIR_FARE);
    AddProcessTag(permSelector, NO_GUARANTEED_FARES);
    AddProcessTag(permSelector, KEEP_FARES_FOR_TRAVELED_FC);

    CPPUNIT_ASSERT(_permGenerator->isProperTagsSequence(permSelector) == true);

    CPPUNIT_ASSERT(_permGenerator->_validSeqMixed.first);
    CPPUNIT_ASSERT(_permGenerator->_validSeqMixed.second);

    ClearProcessTagPermutationData(permSelector);
  }

  void testIsProperTagsSequenceNotValidSequenceOnlyFirstTagIs7()
  {
    CartesianProduct<std::vector<ProcessTagInfo*> >::ProductType permSelector;

    AddProcessTag(permSelector, REISSUE_DOWN_TO_LOWER_FARE);
    AddProcessTag(permSelector, GUARANTEED_AIR_FARE);
    AddProcessTag(permSelector, NO_GUARANTEED_FARES);
    AddProcessTag(permSelector, KEEP_FARES_FOR_TRAVELED_FC);

    CPPUNIT_ASSERT(_permGenerator->isProperTagsSequence(permSelector) == false);

    CPPUNIT_ASSERT(!_permGenerator->_validSeqMixed.first);
    CPPUNIT_ASSERT(_permGenerator->_validSeqMixed.second);

    ClearProcessTagPermutationData(permSelector);
  }

  void testIsProperTagsSequenceNotValidSequenceOnlyLastTagIs7()
  {
    CartesianProduct<std::vector<ProcessTagInfo*> >::ProductType permSelector;

    AddProcessTag(permSelector, GUARANTEED_AIR_FARE);
    AddProcessTag(permSelector, NO_GUARANTEED_FARES);
    AddProcessTag(permSelector, KEEP_FARES_FOR_TRAVELED_FC);
    AddProcessTag(permSelector, REISSUE_DOWN_TO_LOWER_FARE);

    CPPUNIT_ASSERT(_permGenerator->isProperTagsSequence(permSelector) == false);

    CPPUNIT_ASSERT(!_permGenerator->_validSeqMixed.first);
    CPPUNIT_ASSERT(_permGenerator->_validSeqMixed.second);

    ClearProcessTagPermutationData(permSelector);
  }

  void testIsProperTagsSequenceNotValidSequenceTagInTheMiddleIs7()
  {
    CartesianProduct<std::vector<ProcessTagInfo*> >::ProductType permSelector;

    AddProcessTag(permSelector, GUARANTEED_AIR_FARE);
    AddProcessTag(permSelector, REISSUE_DOWN_TO_LOWER_FARE);
    AddProcessTag(permSelector, NO_GUARANTEED_FARES);
    AddProcessTag(permSelector, REISSUE_DOWN_TO_LOWER_FARE);
    AddProcessTag(permSelector, KEEP_FARES_FOR_TRAVELED_FC);

    CPPUNIT_ASSERT(_permGenerator->isProperTagsSequence(permSelector) == false);

    CPPUNIT_ASSERT(!_permGenerator->_validSeqMixed.first);
    CPPUNIT_ASSERT(_permGenerator->_validSeqMixed.second);

    ClearProcessTagPermutationData(permSelector);
  }

  void AddProcessTag(ProcessTagPermutation& perm, ProcessTag tag)
  {
    ProcessTagInfo* pInfo = new ProcessTagInfo();
    ReissueSequence* seq = new ReissueSequence();
    pInfo->reissueSequence()->orig() = seq;
    seq->processingInd() = tag;
    perm.processTags().push_back(pInfo);
  }

  void ClearProcessTags(ProcessTagPermutation& perm)
  {
    std::vector<ProcessTagInfo*>::iterator iter = perm.processTags().begin();
    while (iter != perm.processTags().end())
    {
      delete (*iter)->reissueSequence()->orig();
      delete *iter;
      ++iter;
    }
    perm.processTags().clear();
  }

  void AddProcessTag(CartesianProduct<std::vector<ProcessTagInfo*> >::ProductType& permSelector,
                     ProcessTag pt)
  {
    ProcessTagInfo* pInfo = new ProcessTagInfo();
    ReissueSequence* p = new ReissueSequence();
    pInfo->reissueSequence()->orig() = p;
    p->processingInd() = (int)pt;
    permSelector.push_back(pInfo);
  }

  void ClearProcessTagPermutationData(
      CartesianProduct<std::vector<ProcessTagInfo*> >::ProductType& permSelector)
  {
    CartesianProduct<std::vector<ProcessTagInfo*> >::ProductType::iterator i = permSelector.begin();

    for (; i != permSelector.end(); i++)
    {
      delete (*i)->reissueSequence()->orig();
      delete *i;
    }
  }

  void setUp()
  {
    CppUnit::TestFixture::setUp();
    _trx = _memH(new RexPricingTrx);
    _exchangeItin = _memH(new ExcItin);
    _trx->exchangeItin().push_back(_exchangeItin);
    _exchangeItin->geoTravelType() = GeoTravelType::International;
    _newItin = _memH(new Itin);
    _trx->newItin().push_back(_newItin);
    _firstExcTravelSeg = _memH(new AirSeg);
    _exchangeItin->travelSeg().push_back(_firstExcTravelSeg);
    PermutationGenerator::_healthCheckConfigured = true;
    _permGenerator = _memH(new PermutationGeneratorOverride(*_trx));
  }

  void tearDown()
  {
    _memH.clear();
  }

  TravelSeg* createTravelSeg(LocCode city1, LocCode city2)
  {
    TravelSeg* seg = _memH(new AirSeg);
    Loc* loc1 = _memH(new Loc);
    loc1->loc() = city1;
    seg->origin() = loc1;

    Loc* loc2 = _memH(new Loc);
    loc2->loc() = city2;
    seg->destination() = loc2;

    return seg;
  }

  void testMarkFareRetrievalForNeededFareApplication()
  {
    ProcessTagPermutation perm;
    ProcessTagInfo tag;
    ReissueSequence rs;
    rs.processingInd() = 0;
    tag.reissueSequence()->orig() = &rs;

    perm.processTags().push_back(&tag);
    perm.fareApplWinnerTags().insert(std::make_pair(KEEP, &tag));

    _permGenerator->checkFareRetrieval(perm);

    CPPUNIT_ASSERT(_trx->needRetrieveKeepFare());
  }

  void testNotMarkFareRetrievalForUnneededFareApplication()
  {
    ProcessTagPermutation perm;
    ProcessTagInfo tag;
    ReissueSequence rs;
    rs.processingInd() = 0;
    tag.reissueSequence()->orig() = &rs;

    perm.processTags().push_back(&tag);
    perm.fareApplWinnerTags().insert(std::make_pair(HISTORICAL, &tag));

    _permGenerator->checkFareRetrieval(perm);

    CPPUNIT_ASSERT(!_trx->needRetrieveKeepFare());
  }

  void testTagWarResultFareApplicationSaved()
  {
    ProcessTagInfo* tag3 = createProcessTagInfoWithFare(KEEP_FARES_FOR_TRAVELED_FC, FL);
    ProcessTagInfo* tag9 = createProcessTagInfoWithFare(HISTORICAL_FARES_FOR_TRAVELED_FC, FL);

    ProcessTagPermutation perm;
    perm.processTags().push_back(tag3);
    perm.processTags().push_back(tag9);

    _permGenerator->markWithFareTypes(perm);
    CPPUNIT_ASSERT(perm.fareApplMap().find(tag3->paxTypeFare())->second == KEEP);
    CPPUNIT_ASSERT(perm.fareApplMap().find(tag9->paxTypeFare())->second == KEEP);
  }

  ProcessTagInfo* createProcessTagInfoWithFare(int tagNum, FCChangeStatus changeStatus)
  {
    FareMarket* fm = new FareMarket();
    fm->changeStatus() = changeStatus;
    PaxTypeFare* fare = new PaxTypeFare();
    fare->fareMarket() = fm;

    ReissueSequence* item988 = new ReissueSequence();
    item988->processingInd() = tagNum;

    FareCompInfo* fci = new FareCompInfo();
    fci->fareMarket() = fm;

    ProcessTagInfo* processTag = new ProcessTagInfo();
    processTag->reissueSequence()->orig() = item988;
    processTag->paxTypeFare() = fare;
    processTag->fareCompInfo() = fci;

    return processTag;
  }

  void testNewFmMatchToExcFm()
  {
    Itin newItin;
    FareMarket* newFm = createFareMarket("DFW", "NRT", "JL", GeoTravelType::International, UU);
    newItin.fareMarket().push_back(newFm);
    _trx->newItin().clear();
    _trx->newItin().push_back(&newItin);

    FareMarket* excFm = createFareMarket("DFW", "NRT", "JL", GeoTravelType::International, UU);

    _permGenerator->matchToNewItinFmForKeepFare(excFm);
    CPPUNIT_ASSERT(_trx->newToExcItinFareMarketMapForKeep().find(newFm)->second == excFm);
  }

  void testNewFmNotMatchToExcFmDueToCity()
  {
    Itin newItin;
    FareMarket* newFm = createFareMarket("DFW", "ICN", "AA", GeoTravelType::International, UU);
    newItin.fareMarket().push_back(newFm);
    _trx->newItin().clear();
    _trx->newItin().push_back(&newItin);

    FareMarket* excFm = createFareMarket("DFW", "NRT", "AA", GeoTravelType::International, UU);

    _permGenerator->matchToNewItinFmForKeepFare(excFm);
    CPPUNIT_ASSERT(_trx->newToExcItinFareMarketMapForKeep().find(newFm) ==
                   _trx->newToExcItinFareMarketMapForKeep().end());
  }

  void testNewFmNotMatchToExcFmDueToCxr()
  {
    Itin newItin;
    FareMarket* newFm = createFareMarket("DFW", "NRT", "AA", GeoTravelType::International, UU);
    newItin.fareMarket().push_back(newFm);
    _trx->newItin().clear();
    _trx->newItin().push_back(&newItin);

    FareMarket* excFm = createFareMarket("DFW", "NRT", "JL", GeoTravelType::International, UU);

    _permGenerator->matchToNewItinFmForKeepFare(excFm);
    CPPUNIT_ASSERT(_trx->newToExcItinFareMarketMapForKeep().find(newFm) ==
                   _trx->newToExcItinFareMarketMapForKeep().end());
  }

  void testNewFmNotMatchToExcFmDueToChangeStatus()
  {
    Itin newItin;
    FareMarket* newFm = createFareMarket("DFW", "NRT", "JL", GeoTravelType::International, UC);
    newItin.fareMarket().push_back(newFm);
    _trx->newItin().clear();
    _trx->newItin().push_back(&newItin);

    FareMarket* excFm = createFareMarket("DFW", "NRT", "JL", GeoTravelType::International, UU);

    _permGenerator->matchToNewItinFmForKeepFare(excFm);
    CPPUNIT_ASSERT(_trx->newToExcItinFareMarketMapForKeep().find(newFm) ==
                   _trx->newToExcItinFareMarketMapForKeep().end());
  }

  FareMarket* createFareMarket(tse::LocCode boardCity,
                               tse::LocCode offCity,
                               CarrierCode govCxr,
                               GeoTravelType geoTravelType,
                               FCChangeStatus changeStatus)
  {
    FareMarket* fm = new FareMarket();
    fm->changeStatus() = changeStatus;
    fm->boardMultiCity() = boardCity;
    fm->offMultiCity() = offCity;
    fm->governingCarrier() = govCxr;
    fm->geoTravelType() = geoTravelType;

    return fm;
  }

  class ProcessTagPermutationMock : public ProcessTagPermutation
  {
  public:
    ProcessTagPermutationMock(Indicator byte123Value) : _byte123Value(byte123Value) {}
    Indicator _byte123Value;
    Indicator checkTable988Byte123() { return _byte123Value; }
  };

  void testElectronicTicketPassOnBlank()
  {
    ProcessTagPermutationMock ptpMock(ProcessTagPermutation::ELECTRONIC_TICKET_BLANK);
    std::string validationOut;
    CPPUNIT_ASSERT(_permGenerator->checkElectronicTicket(ptpMock, validationOut));
    CPPUNIT_ASSERT(validationOut.empty());
  }
  void testElectronicTicketPassOnR()
  {
    ProcessTagPermutationMock ptpMock(ProcessTagPermutation::ELECTRONIC_TICKET_REQUIRED);
    std::string validationOut;
    CPPUNIT_ASSERT(_permGenerator->checkElectronicTicket(ptpMock, validationOut));
    CPPUNIT_ASSERT(validationOut.empty());
  }
  void testElectronicTicketPassOnN()
  {
    ProcessTagPermutationMock ptpMock(ProcessTagPermutation::ELECTRONIC_TICKET_NOT_ALLOWED);
    std::string validationOut;
    CPPUNIT_ASSERT(_permGenerator->checkElectronicTicket(ptpMock, validationOut));
    CPPUNIT_ASSERT(validationOut.empty());
  }
  void testElectronicTicketFailOnMix()
  {
    ProcessTagPermutationMock ptpMock(ProcessTagPermutation::ELECTRONIC_TICKET_MIXED);
    std::string validationOut;
    CPPUNIT_ASSERT(!_permGenerator->checkElectronicTicket(ptpMock, validationOut));
    std::string str = " \nELECTRONIC TICKET: FAILED";
    CPPUNIT_ASSERT_EQUAL(validationOut, str);
  }

  void addTag10ToApplMapAndToWinnerTags(ProcessTagPermutation& perm,
                                        FareApplication fa,
                                        FCChangeStatus changeStatus)
  {
    ProcessTagInfo* tag =
        createProcessTagInfoWithFare(KEEP_FOR_UNCH_CURRENT_FOR_CHNG, changeStatus);
    perm.fareApplWinnerTags().insert(std::make_pair(fa, tag));
    perm.fareApplWinnerTags().insert(std::make_pair(KEEP, tag));
    perm.fareApplMap().insert(std::make_pair(tag->paxTypeFare(), fa));
    perm.processTags().push_back(tag);
  }

  void testMapKeepFareWhenKeepFareAndSameFareBreaks()
  {
    ProcessTagPermutation perm;
    addTag10ToApplMapAndToWinnerTags(perm, KEEP, UU);

    _newItin->fareMarket().push_back(
        const_cast<FareMarket*>(perm.fareApplWinnerTags()[KEEP]->paxTypeFare()->fareMarket()));

    _permGenerator->mapKeepFares(perm);
    CPPUNIT_ASSERT(_trx->newItinKeepFares().size() == 1);
  }

  void testMapKeepFareWhenKeepFareAndDifferentFareBreaks()
  {
    ProcessTagPermutation perm;
    addTag10ToApplMapAndToWinnerTags(perm, KEEP, UU);

    FareMarket fareMarket;
    fareMarket.boardMultiCity() = "CHI";
    _newItin->fareMarket().push_back(&fareMarket);

    _permGenerator->mapKeepFares(perm);
    CPPUNIT_ASSERT(_trx->newItinKeepFares().size() == 0);
  }

  void testMapKeepFareWhenHistFareAndSameFareBreaksInvChanged()
  {
    ProcessTagPermutation perm;
    addTag10ToApplMapAndToWinnerTags(perm, HISTORICAL, UC);

    FareMarket* fareMarket =
        const_cast<FareMarket*>(perm.fareApplWinnerTags()[HISTORICAL]->paxTypeFare()->fareMarket());
    perm.setRebookFareTypeSelection(UU, KEEP);
    AirSeg as1;
    AirSeg as2;
    fareMarket->travelSeg().push_back(&as1);
    fareMarket->travelSeg().push_back(&as2);
    as1.changeStatus() = TravelSeg::INVENTORYCHANGED;
    as2.changeStatus() = TravelSeg::INVENTORYCHANGED;
    _newItin->fareMarket().push_back(fareMarket);

    _permGenerator->mapKeepFares(perm);
    CPPUNIT_ASSERT(_trx->newItinKeepFares().size() == 1);
  }

  void testMapKeepFareWhenHistFareAndDifferentFareBreaksInvChanged()
  {
    ProcessTagPermutation perm;
    addTag10ToApplMapAndToWinnerTags(perm, HISTORICAL, UC);

    FareMarket fareMarket;
    fareMarket.boardMultiCity() = "CHI";
    perm.setRebookFareTypeSelection(UU, KEEP);
    AirSeg as1;
    AirSeg as2;
    fareMarket.travelSeg().push_back(&as1);
    fareMarket.travelSeg().push_back(&as2);
    as1.changeStatus() = TravelSeg::INVENTORYCHANGED;
    as2.changeStatus() = TravelSeg::INVENTORYCHANGED;
    _newItin->fareMarket().push_back(&fareMarket);

    _permGenerator->mapKeepFares(perm);
    CPPUNIT_ASSERT(_trx->newItinKeepFares().size() == 0);
  }

  void testMapKeepFareWhenHistFareAndSameFareBreaksFlightChanged()
  {
    ProcessTagPermutation perm;
    addTag10ToApplMapAndToWinnerTags(perm, HISTORICAL, UC);

    FareMarket* fareMarket =
        const_cast<FareMarket*>(perm.fareApplWinnerTags()[HISTORICAL]->paxTypeFare()->fareMarket());
    perm.setRebookFareTypeSelection(UU, KEEP);
    AirSeg as1;
    AirSeg as2;
    fareMarket->travelSeg().push_back(&as1);
    fareMarket->travelSeg().push_back(&as2);
    as1.changeStatus() = TravelSeg::INVENTORYCHANGED;
    as2.changeStatus() = TravelSeg::CHANGED;
    _newItin->fareMarket().push_back(fareMarket);

    _permGenerator->mapKeepFares(perm);
    CPPUNIT_ASSERT(_trx->newItinKeepFares().size() == 0);
  }

  void testSetReissueExchangeROEConversionDateWhenApplyReissueExchNotExists()
  {
    _trx->newItinROEConversionDate() = DateTime(2009, 1, 2);
    _trx->currentTicketingDT() = DateTime(2008, 8, 8);
    _permGenerator->setReissueExchangeROEConversionDate();
    CPPUNIT_ASSERT(_trx->newItinROEConversionDate() == DateTime(2009, 1, 2));
  }

  void testSetReissueExchangeROEConversionDateWhenExchangeForCurrentDatePass()
  {
    _trx->setRexPrimaryProcessType('A');
    _trx->newItinROEConversionDate() = DateTime(2009, 1, 2);
    _trx->setItinIndex(0);
    _trx->newItin().front()->exchangeReissue() = EXCHANGE;
    _trx->markFareRetrievalMethodCurrent(true);
    _trx->currentTicketingDT() = DateTime(2010, 8, 8);
    _permGenerator->setReissueExchangeROEConversionDate();
    CPPUNIT_ASSERT(_trx->newItinROEConversionDate() == DateTime(2010, 8, 8));
  }

  void testSetReissueExchangeROEConversionDateWhenExchangeForNonCurrentDateNoD95PassOrigin()
  {
    _trx->setRexPrimaryProcessType('A');
    _trx->newItinROEConversionDate() = DateTime(2009, 1, 2);
    _trx->setItinIndex(0);
    _trx->newItin().front()->exchangeReissue() = EXCHANGE;
    _trx->markFareRetrievalMethodKeep(true);
    _trx->currentTicketingDT() = DateTime(2010, 8, 8);
    _trx->setOriginalTktIssueDT() = DateTime(2009, 1, 2);
    _permGenerator->setReissueExchangeROEConversionDate();
    CPPUNIT_ASSERT(_trx->newItinROEConversionDate() == DateTime(2009, 1, 2));
  }

  void testSetReissueExchangeROEConversionDateWhenReissueForCurrentDateOnlyPass()
  {
    _trx->setRexPrimaryProcessType('A');
    _trx->newItinROEConversionDate() = DateTime(2009, 1, 2);
    _trx->setItinIndex(0);
    _trx->newItin().front()->exchangeReissue() = REISSUE;
    _trx->markFareRetrievalMethodCurrent(true);
    _trx->currentTicketingDT() = DateTime(2010, 8, 8);
    _permGenerator->setReissueExchangeROEConversionDate();
    CPPUNIT_ASSERT(_trx->newItinROEConversionDate() == DateTime(2010, 8, 8));
  }

  void testSetReissueExchangeROEConversionDateWhenReissueForCurrentAndKeepDatesFail()
  {
    _trx->setRexPrimaryProcessType('A');
    _trx->newItinROEConversionDate() = DateTime(2009, 1, 2);
    _trx->setItinIndex(0);
    _trx->newItin().front()->exchangeReissue() = REISSUE;
    _trx->markFareRetrievalMethodCurrent(true);
    _trx->markFareRetrievalMethodKeep(true);
    _trx->currentTicketingDT() = DateTime(2010, 8, 8);
    _permGenerator->setReissueExchangeROEConversionDate();
    CPPUNIT_ASSERT(_trx->newItinROEConversionDate() != DateTime(2010, 8, 8));
  }

  void testSetReissueExchangeROEConversionDateWhenReissueForCurrentAndKeepDatesD95PresentPass()
  {
    _trx->setRexPrimaryProcessType('A');
    _trx->newItinROEConversionDate() = DateTime(2009, 1, 2);
    _trx->setItinIndex(0);
    _trx->newItin().front()->exchangeReissue() = REISSUE;
    _trx->markFareRetrievalMethodCurrent(true);
    _trx->markFareRetrievalMethodKeep(true);
    _trx->previousExchangeDT() = DateTime(2009, 1, 10);
    _trx->currentTicketingDT() = DateTime(2010, 8, 8);
    _permGenerator->setReissueExchangeROEConversionDate();
    CPPUNIT_ASSERT(_trx->newItinROEConversionDate() == DateTime(2009, 1, 10));
  }

  void testSetReissueExchangeROEConversionDateWhenReissueForCurrentAndKeepDatesNoD95PassOrigin()
  {
    _trx->setRexPrimaryProcessType('A');
    _trx->newItinROEConversionDate() = DateTime(2009, 1, 2);
    _trx->setItinIndex(0);
    _trx->newItin().front()->exchangeReissue() = REISSUE;
    _trx->markFareRetrievalMethodCurrent(true);
    _trx->markFareRetrievalMethodKeep(true);
    _trx->setOriginalTktIssueDT() = DateTime(2009, 1, 2);
    _trx->currentTicketingDT() = DateTime(2010, 8, 8);
    _permGenerator->setReissueExchangeROEConversionDate();
    CPPUNIT_ASSERT(_trx->newItinROEConversionDate() == DateTime(2009, 1, 2));
  }

  void testSetReissueExchangeROEConversionDateSetsSecondROEDateToEmptyWhenOnlyCurrentFaresNeeded()
  {
    _trx->setRexPrimaryProcessType('A');
    _trx->setItinIndex(0);
    _trx->newItin().front()->exchangeReissue() = EXCHANGE;
    _trx->markFareRetrievalMethodCurrent(true);
    _trx->markFareRetrievalMethodKeep(false);
    _trx->currentTicketingDT() = DateTime(2010, 8, 8);
    _permGenerator->setReissueExchangeROEConversionDate();
    CPPUNIT_ASSERT(_trx->newItinSecondROEConversionDate().isEmptyDate());
  }

  void
  testSetReissueExchangeROEConversionDateSetsSecondROEDateToPreviousDateWhenExchangeForCurrentFares()
  {
    _trx->setOriginalTktIssueDT() = DateTime(2009, 1, 2);
    _trx->previousExchangeDT() = DateTime(2009, 1, 3);

    _trx->setRexPrimaryProcessType('A');
    _trx->newItinROEConversionDate() = DateTime(2009, 1, 2);
    _trx->setItinIndex(0);
    _trx->newItin().front()->exchangeReissue() = EXCHANGE;
    _trx->markFareRetrievalMethodCurrent(true);
    _trx->markFareRetrievalMethodKeep(true);
    _trx->currentTicketingDT() = DateTime(2010, 8, 8);
    _permGenerator->setReissueExchangeROEConversionDate();
    CPPUNIT_ASSERT_EQUAL(_trx->previousExchangeDT(), _trx->newItinSecondROEConversionDate());
  }

  void testSetReissueExchangeROEConversionDateSetsSecondROEDateToCurrentDateWhenMixedFares()
  {
    _trx->setRexPrimaryProcessType('A');
    _trx->newItinROEConversionDate() = DateTime(2009, 1, 2);
    _trx->setItinIndex(0);
    _trx->newItin().front()->exchangeReissue() = REISSUE;
    _trx->markFareRetrievalMethodCurrent(true);
    _trx->markFareRetrievalMethodKeep(true);
    _trx->currentTicketingDT() = DateTime(2010, 8, 8);
    _trx->setOriginalTktIssueDT() = DateTime(2009, 1, 2);
    _permGenerator->setReissueExchangeROEConversionDate();
    CPPUNIT_ASSERT_EQUAL(_trx->currentTicketingDT(), _trx->newItinSecondROEConversionDate());
  }

  void testGetPreviousDateReturnOriginalIssueDateWhenPreviousDateEmpty()
  {
    _trx->setOriginalTktIssueDT() = DateTime(2009, 1, 2);
    _trx->previousExchangeDT() = DateTime(DateTime::emptyDate());
    CPPUNIT_ASSERT_EQUAL(_trx->originalTktIssueDT(), _permGenerator->getPreviousDate());
  }

  void testGetPreviousDateReturnPreviousExchangeDateWhenNotEmpty()
  {
    _trx->setOriginalTktIssueDT() = DateTime(2009, 1, 1);
    _trx->previousExchangeDT() = DateTime(2009, 1, 2);
    CPPUNIT_ASSERT_EQUAL(_trx->previousExchangeDT(), _permGenerator->getPreviousDate());
  }

  void addFareMarkets(const std::vector<CarrierCode>& cxrCodes,
                      const std::vector<GeoTravelType>& gd,
                      const std::vector<NationCode>& origNations,
                      const std::vector<NationCode>& destNations)
  {
    std::vector<CarrierCode>::const_iterator cci = cxrCodes.begin();
    std::vector<CarrierCode>::const_iterator ccie = cxrCodes.end();
    for (unsigned i = 0; cci != ccie; ++cci, ++i)
      _newItin->fareMarket().push_back(getFm(cxrCodes[i], gd[i], origNations[i], destNations[i]));
  }

  FareMarket* getFm(CarrierCode cxr, GeoTravelType gd, NationCode origN, NationCode destN)
  {
    FareMarket* fm = _memH(new FareMarket);
    fm->governingCarrier() = cxr;
    fm->geoTravelType() = gd;
    Loc* orig = _memH(new Loc);
    orig->nation() = origN;
    Loc* dest = _memH(new Loc);
    dest->nation() = destN;
    fm->origin() = orig;
    fm->destination() = dest;
    return fm;
  }

  PaxTypeFare* getExcPtf(CarrierCode cxr, GeoTravelType gd, NationCode origN, NationCode destN)
  {
    PaxTypeFare* excPtf = _memH(new PaxTypeFare);
    excPtf->fareMarket() = getFm(cxr, gd, origN, destN);
    return excPtf;
  }

  void testMatchForEKeepAlreadyProcessed()
  {
    addFareMarkets(list_of("AA")("AA")("AA"),
                   list_of(GeoTravelType::International)(GeoTravelType::International)(GeoTravelType::International),
                   list_of("US")("US")("US"),
                   list_of("US")("US")("US"));

    PaxTypeFare* excPtf = getExcPtf("AA", GeoTravelType::International, "US", "US");

    _trx->expndKeepMap().insert(std::make_pair(static_cast<FareMarket*>(0), excPtf));

    _permGenerator->matchToNewItinFmForExpndKeepFare(*excPtf);

    CPPUNIT_ASSERT_EQUAL(static_cast<RexPricingTrx::ExpndKeepMap::size_type>(1),
                         _trx->expndKeepMap().size());
  }

  void testMatchForEKeepSize()
  {
    addFareMarkets(list_of("AA")("AA")("AA"),
                   list_of(GeoTravelType::International)(GeoTravelType::International)(GeoTravelType::International),
                   list_of("US")("US")("US"),
                   list_of("US")("US")("US"));

    PaxTypeFare* excPtf = getExcPtf("AA", GeoTravelType::International, "US", "US");

    _permGenerator->matchToNewItinFmForExpndKeepFare(*excPtf);
    CPPUNIT_ASSERT_EQUAL(static_cast<RexPricingTrx::ExpndKeepMap::size_type>(3),
                         _trx->expndKeepMap().size());
  }

  void testMatchForEKeepConstruction()
  {
    addFareMarkets(list_of("AA")("AA")("AA"),
                   list_of(GeoTravelType::International)(GeoTravelType::International)(GeoTravelType::International),
                   list_of("US")("US")("US"),
                   list_of("US")("US")("US"));

    const PaxTypeFare* excPtf = getExcPtf("AA", GeoTravelType::International, "US", "US");

    _permGenerator->matchToNewItinFmForExpndKeepFare(*excPtf);
    CPPUNIT_ASSERT_EQUAL(_trx->expndKeepMap().lower_bound(_newItin->fareMarket()[0])->second,
                         excPtf);
    CPPUNIT_ASSERT_EQUAL(_trx->expndKeepMap().lower_bound(_newItin->fareMarket()[1])->second,
                         excPtf);
    CPPUNIT_ASSERT_EQUAL(_trx->expndKeepMap().lower_bound(_newItin->fareMarket()[2])->second,
                         excPtf);
  }

  void testMatchForEKeepCxrDiff()
  {
    addFareMarkets(list_of("AA")("UA")("AA"),
                   list_of(GeoTravelType::International)(GeoTravelType::International)(GeoTravelType::International),
                   list_of("US")("US")("US"),
                   list_of("US")("US")("US"));

    PaxTypeFare* excPtf = getExcPtf("AA", GeoTravelType::International, "US", "US");

    _permGenerator->matchToNewItinFmForExpndKeepFare(*excPtf);
    CPPUNIT_ASSERT_EQUAL(static_cast<RexPricingTrx::ExpndKeepMap::size_type>(2),
                         _trx->expndKeepMap().size());
  }

  void testMatchForEKeepOrginNationDiff()
  {
    addFareMarkets(list_of("AA")("AA")("AA"),
                   list_of(GeoTravelType::International)(GeoTravelType::International)(GeoTravelType::International),
                   list_of("RU")("US")("US"),
                   list_of("US")("US")("US"));

    PaxTypeFare* excPtf = getExcPtf("AA", GeoTravelType::International, "US", "US");

    _permGenerator->matchToNewItinFmForExpndKeepFare(*excPtf);
    CPPUNIT_ASSERT_EQUAL(static_cast<RexPricingTrx::ExpndKeepMap::size_type>(2),
                         _trx->expndKeepMap().size());
  }

  void testMatchForEKeepDestNationDiff()
  {
    addFareMarkets(list_of("AA")("AA")("AA"),
                   list_of(GeoTravelType::International)(GeoTravelType::International)(GeoTravelType::International),
                   list_of("US")("US")("US"),
                   list_of("US")("US")("RU"));

    PaxTypeFare* excPtf = getExcPtf("AA", GeoTravelType::International, "US", "US");

    _permGenerator->matchToNewItinFmForExpndKeepFare(*excPtf);
    CPPUNIT_ASSERT_EQUAL(static_cast<RexPricingTrx::ExpndKeepMap::size_type>(2),
                         _trx->expndKeepMap().size());
  }

  void testMatchForEKeepGDDiff()
  {
    addFareMarkets(list_of("AA")("AA")("AA"),
                   list_of(GeoTravelType::International)(GeoTravelType::Domestic)(GeoTravelType::International),
                   list_of("US")("US")("US"),
                   list_of("US")("US")("US"));

    PaxTypeFare* excPtf = getExcPtf("AA", GeoTravelType::International, "US", "US");

    _permGenerator->matchToNewItinFmForExpndKeepFare(*excPtf);
    CPPUNIT_ASSERT_EQUAL(static_cast<RexPricingTrx::ExpndKeepMap::size_type>(2),
                         _trx->expndKeepMap().size());
  }

  void testUpdatePermutationInfoForFareMarketsSetFlagWhenCurrentFlown()
  {
    FareApplication fareAppl = CURRENT;
    FCChangeStatus changeStatus = FL;
    _permGenerator->updatePermuationInfoForFareMarkets(fareAppl, changeStatus);
    CPPUNIT_ASSERT(_trx->allPermutationsRequireCurrentForFlown());
    CPPUNIT_ASSERT(!_trx->allPermutationsRequireNotCurrentForFlown());
  }
  void testUpdatePermutationInfoForFareMarketsSetFlagForKeepFlown()
  {
    FareApplication fareAppl = KEEP;
    FCChangeStatus changeStatus = FL;
    _permGenerator->updatePermuationInfoForFareMarkets(fareAppl, changeStatus);
    CPPUNIT_ASSERT(!_trx->allPermutationsRequireCurrentForFlown());
    CPPUNIT_ASSERT(_trx->allPermutationsRequireNotCurrentForFlown());
  }
  void testUpdatePermutationInfoForFareMarketsSetFlagForHistoricalFlown()
  {
    FareApplication fareAppl = HISTORICAL;
    FCChangeStatus changeStatus = FL;
    _permGenerator->updatePermuationInfoForFareMarkets(fareAppl, changeStatus);
    CPPUNIT_ASSERT(!_trx->allPermutationsRequireCurrentForFlown());
    CPPUNIT_ASSERT(_trx->allPermutationsRequireNotCurrentForFlown());
  }
  void testUpdatePermutationInfoForFareMarketsSetFlagForTravelCommencementFlown()
  {
    FareApplication fareAppl = TRAVEL_COMMENCEMENT;
    FCChangeStatus changeStatus = FL;
    _permGenerator->updatePermuationInfoForFareMarkets(fareAppl, changeStatus);
    CPPUNIT_ASSERT(!_trx->allPermutationsRequireCurrentForFlown());
    CPPUNIT_ASSERT(_trx->allPermutationsRequireNotCurrentForFlown());
  }
  void testUpdatePermutationInfoForFareMarketsSetFlagWhenCurrentUnflown()
  {
    FareApplication fareAppl = CURRENT;
    FCChangeStatus changeStatus = UU;
    _permGenerator->updatePermuationInfoForFareMarkets(fareAppl, changeStatus);
    CPPUNIT_ASSERT(_trx->allPermutationsRequireCurrentForUnflown());
    CPPUNIT_ASSERT(!_trx->allPermutationsRequireNotCurrentForUnflown());
  }
  void testUpdatePermutationInfoForFareMarketsSetFlagForKeepUnflown()
  {
    FareApplication fareAppl = KEEP;
    FCChangeStatus changeStatus = UU;
    _permGenerator->updatePermuationInfoForFareMarkets(fareAppl, changeStatus);
    CPPUNIT_ASSERT(!_trx->allPermutationsRequireCurrentForUnflown());
    CPPUNIT_ASSERT(_trx->allPermutationsRequireNotCurrentForUnflown());
  }
  void testUpdatePermutationInfoForFareMarketsSetFlagForHistoricalUnflown()
  {
    FareApplication fareAppl = HISTORICAL;
    FCChangeStatus changeStatus = UU;
    _permGenerator->updatePermuationInfoForFareMarkets(fareAppl, changeStatus);
    CPPUNIT_ASSERT(!_trx->allPermutationsRequireCurrentForUnflown());
    CPPUNIT_ASSERT(_trx->allPermutationsRequireNotCurrentForUnflown());
  }
  void testUpdatePermutationInfoForFareMarketsSetFlagForTravelCommencementUnflown()
  {
    FareApplication fareAppl = TRAVEL_COMMENCEMENT;
    FCChangeStatus changeStatus = UU;
    _permGenerator->updatePermuationInfoForFareMarkets(fareAppl, changeStatus);
    CPPUNIT_ASSERT(!_trx->allPermutationsRequireCurrentForUnflown());
    CPPUNIT_ASSERT(_trx->allPermutationsRequireNotCurrentForUnflown());
  }

  void testConnectionChanged()
  {
    FareUsage* fu = _memH(new FareUsage);
    fu->travelSeg().push_back(createTravelSeg("KRK", "LON"));
    fu->travelSeg().push_back(createTravelSeg("LON", "DFW"));

    PricingUnit* pu = _memH(new PricingUnit);
    pu->fareUsage().push_back(fu);

    FarePath* fp = _memH(new FarePath);
    fp->pricingUnit().push_back(pu);

    std::vector<TravelSeg*> curNewTvlSegs = {createTravelSeg("KRK", "PAR"),
                                             createTravelSeg("PAR", "DFW")};

    bool connectionChanged = false,
         stopoverChanged = false;

    std::tie(connectionChanged,
             stopoverChanged) = _permGenerator->stopoversOrConnectionChanged(*fp, curNewTvlSegs);
    CPPUNIT_ASSERT(connectionChanged);
  }

  void testStopOverChanged()
  {
    FareUsage* fu = _memH(new FareUsage);
    fu->travelSeg().push_back(createTravelSeg("KRK", "LON"));
    fu->travelSeg().back()->stopOver() = true;
    fu->travelSeg().push_back(createTravelSeg("LON", "DFW"));

    PricingUnit* pu = _memH(new PricingUnit);
    pu->fareUsage().push_back(fu);

    FarePath* fp = _memH(new FarePath);
    fp->pricingUnit().push_back(pu);

    std::vector<TravelSeg*> curNewTvlSegs = {createTravelSeg("KRK", "PAR"),
                                             createTravelSeg("PAR", "DFW")};

    bool connectionChanged = false,
         stopoverChanged = false;

    std::tie(connectionChanged,
             stopoverChanged) = _permGenerator->stopoversOrConnectionChanged(*fp, curNewTvlSegs);
    CPPUNIT_ASSERT(stopoverChanged);
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(PermutationGeneratorTest);
}
