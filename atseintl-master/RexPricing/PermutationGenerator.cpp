//-------------------------------------------------------------------
//
//  File:        PermutationGenerator.cpp
//  Created:     August 9, 2007
//  Authors:     Grzegorz Cholewiak
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
#include "Common/Config/ConfigurableValue.h"
#include "Common/Logger.h"
#include "Common/Memory/Config.h"
#include "Common/Memory/GlobalManager.h"
#include "Common/Memory/Monitor.h"
#include "Common/MemoryUsage.h"
#include "Common/TrxUtil.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/ProcessTagPermutation.h"
#include "DataModel/ReissueOptionsMap.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TrxAborter.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag688Collector.h"
#include "RexPricing/PenaltyEstimator.h"
#include "RexPricing/PermutationGenerator.h"
#include "RexPricing/TagWarEngine.h"
#include "RexPricing/OptimizationMapper.h"
#include "Rules/ReissueTable.h"
#include "Rules/RuleUtil.h"

#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <functional>
#include <vector>

namespace tse
{
namespace
{
ConfigurableValue<uint32_t>
maxPermNumber("REX_FARE_SELECTOR_SVC", "PERM_MAX_NUMBER", 0);
ConfigurableValue<uint32_t>
maxMemoryGrow("REX_FARE_SELECTOR_SVC", "PERM_MAX_MEMORY_GROW", 0);
ConfigurableValue<uint32_t>
maxVM("TO_MAN", "MAX_VIRTUAL_MEMORY", 0);
ConfigurableValue<uint32_t>
minAvailMem("TO_MAN", "MIN_AVAILABLE_MEMORY", 0);
}

FALLBACK_DECL(reworkTrxAborter)
FALLBACK_DECL(carrierApplicationOpt)
FALLBACK_DECL(excPrivatePublicOpt)

Logger
PermutationGenerator::_logger("atseintl.RexPricing.PermutationGenerator");

boost::mutex PermutationGenerator::_mutex;
volatile bool PermutationGenerator::_healthCheckConfigured = false;
uint32_t PermutationGenerator::_maxPermNumber = 0;
uint32_t PermutationGenerator::_maxMemoryGrow = 0;
uint32_t PermutationGenerator::_maxVM = 0;
uint32_t PermutationGenerator::_minAvailMem = 0;

namespace
{
struct CmpReissueSeqs
{
  bool operator()(const ProcessTagInfo* t1, const ProcessTagInfo* t2) const
  {
    if (t1 == t2)
      return false;
    if (t1->record3()->itemNo() != t2->record3()->itemNo())
      return t1->record3()->itemNo() < t2->record3()->itemNo();

    if (t1->record3()->reissueTblItemNo() == 0 && t2->record3()->reissueTblItemNo() == 0)
      return false;

    if (t1->record3()->reissueTblItemNo() != t2->record3()->reissueTblItemNo())
      return t1->record3()->reissueTblItemNo() < t2->record3()->reissueTblItemNo();

    if (t1->seqNo() != t2->seqNo())
      return t1->seqNo() < t2->seqNo();
    return t1 < t2;
  }
};

struct isTagNumber : public std::binary_function<const ProcessTagInfo*, const int, bool>
{
  bool operator()(const ProcessTagInfo* tag, const int num) const
  {
    return tag->reissueSequence()->orig() ? tag->reissueSequence()->orig()->processingInd() == num
                                          : false;
  }
};

struct DifferentTagNumber
    : public std::binary_function<const ProcessTagInfo*, const ProcessTagInfo*, bool>
{
  bool operator()(const ProcessTagInfo* tag, const ProcessTagInfo* tag2) const
  {
    if (tag->reissueSequence()->orig())
      return !isTagNumber()(tag2, tag->reissueSequence()->processingInd());

    return tag2->reissueSequence()->orig();
  }
};

class PenaltyComparer
{
public:
  bool operator()(const ProcessTagPermutation* lhs, const ProcessTagPermutation* rhs) const
  {
    return lhs->getEstimatedChangeFee() < rhs->getEstimatedChangeFee();
  }
};

} // namespace

PermutationGenerator::PermutationGenerator(RexPricingTrx& trx)
  : _trx(trx), _dc(nullptr), _dc331(nullptr)
{
  setHealthCheckConfig();
  if (_trx.diagnostic().diagnosticType() == Diagnostic688)
  {
    std::string diagRangeMin = _trx.diagnostic().diagParamMapItem("RS");
    std::string diagRangeMax = _trx.diagnostic().diagParamMapItem("RE");
    std::string diagPermIndex = _trx.diagnostic().diagParamMapItem("ID");

    DCFactory* factory = DCFactory::instance();
    _dc = dynamic_cast<Diag688Collector*>(factory->create(_trx));
    if (_dc != nullptr)
    {
      _dc->enable(Diagnostic688);
      int permIndex = std::atoi(diagPermIndex.c_str());

      _dc->minRange() = permIndex > 0 ? permIndex : std::atoi(diagRangeMin.c_str());
      if (_dc->minRange() < 1)
        _dc->minRange() = 1;

      _dc->maxRange() = permIndex > 0 ? permIndex : std::atoi(diagRangeMax.c_str());
      if (_dc->maxRange() < 1)
        _dc->maxRange() = 0;

      if (!_dc->isActive())
      {
        _dc = nullptr;
      }

      if (_dc)
      {
        _dc331 = dynamic_cast<DiagCollector*>(factory->create(_trx));
        if (_dc331)
          _dc331->activate();
      }
    }
  }

  _travelCommenced = !_trx.exchangeItin().front()->travelSeg().front()->unflown();
}

PermutationGenerator::~PermutationGenerator()
{
  if (_dc != nullptr)
  {
    _dc->flushMsg();

    _dc = nullptr;

    if (_dc331)
    {
      _dc331->deActivate();
    }
  }
}

void
PermutationGenerator::process()
{
  const ExcItin* excItin = _trx.exchangeItin().front();

  matchExcToNewFareMarket(excItin);

  std::vector<const PaxTypeFare*> ptfv;
  RuleUtil::getAllPTFs(ptfv, *_trx.exchangeItin().front()->farePath().front());
  std::vector<std::vector<ProcessTagInfo*>> seqsByFC(excItin->fareComponent().size());
  if (!excItin->travelSeg().front()->unflown() && ptfv.front()->fareMarket()->changeStatus() != FL)
  {
    _trx.allPermutationsRequireCurrentForFlown() = false;
    _trx.allPermutationsRequireNotCurrentForFlown() = false;
  }

  for (const PaxTypeFare* ptf : ptfv)
  {
    std::vector<ReissueOptions::R3WithDateRange> r3v;
    _trx.reissueOptions().getRec3s(ptf, r3v);

    if (r3v.empty() && ptf->isFareByRule())
    {
      ptf = ptf->baseFare();
      _trx.reissueOptions().getRec3s(ptf, r3v);
    }

    FareCompInfo* fci = excItin->findFareCompInfo(ptf->fareMarket());

    for (const auto& r3Pair : r3v)
    {
      std::vector<ReissueOptions::ReissueSeqWithDateRange> t988v;
      _trx.reissueOptions().getT988s(ptf, r3Pair.first, t988v);
      for (const auto& t988Pair : t988v)
      {
        ProcessTagInfo* pti = nullptr;
        _trx.dataHandle().get(pti);
        pti->reissueSequence()->orig() = t988Pair.first;
        pti->record3()->orig() = r3Pair.first;
        pti->paxTypeFare() = ptf;
        pti->fareCompInfo() = fci;
        seqsByFC[pti->fareCompNumber() - 1].push_back(pti);
      }
    }
  }

  CartesianProduct<std::vector<ProcessTagInfo*>> cp;
  for (std::vector<ProcessTagInfo*>& ptiV : seqsByFC)
  {
    std::sort(ptiV.begin(), ptiV.end(), CmpReissueSeqs());
    checkStopByte(ptiV);
    cp.addSet(ptiV);
  }

  CartesianProduct<std::vector<ProcessTagInfo*>>::ProductType permSelector = cp.getNext();
  uint16_t permNumber(0), maxRange(_dc ? _dc->maxRange() : 0), validPermCounter(0);
  bool permPassed;
  std::string validationOut;

  if (_dc)
    _dc->printLine();

  LOG4CXX_DEBUG(_logger, "Start building permutations");

  size_t startVM(0);
  size_t startRSS(0);
  if (!Memory::changesFallback)
  {
    if (_maxMemoryGrow > 0)
    {
      if (_minAvailMem > 0)
        startRSS =
            Memory::MemoryMonitor::instance()->getUpdatedResidentMemorySize() / (1024 * 1024);
      else
        startVM = Memory::MemoryMonitor::instance()->getUpdatedVirtualMemorySize() / (1024 * 1024);
    }
  }
  else
  {
    if (_minAvailMem > 0)
    {
      startRSS = _maxMemoryGrow > 0 ? MemoryUsage::getResidentMemorySize() / (1024 * 1024) : 0;
    }
    else
    {
      startVM = _maxMemoryGrow > 0 ? MemoryUsage::getVirtualMemorySize() / (1024 * 1024) : 0;
    }
  }
  const size_t numFC = ptfv.size();

  bool connectionChanged = false, stopoverChanged = false;
  std::tie(connectionChanged, stopoverChanged) =
      stopoversOrConnectionChanged(*excItin->farePath().front(), _trx.curNewItin()->travelSeg());

  while (!permSelector.empty())
  {
    ++permNumber;

    checkHealth(validPermCounter, startVM, startRSS, numFC);

    ProcessTagPermutation* p = nullptr;

    if (_dc)
    {
      _dc->maxRange() = maxRange ? std::min(maxRange, permNumber) : permNumber;
      _dc->permNumber() = permNumber;
      validationOut = "";
    }

    std::pair<bool, bool> validSeq_Mixed = permCharacteristic(permSelector, validationOut);
    permPassed = validSeq_Mixed.first && overrideProcessTagInfoData(permSelector, validationOut);

    if (permPassed)
    {
      _trx.dataHandle().get(p);
      std::copy(permSelector.begin(), permSelector.end(), std::back_inserter(p->processTags()));

      p->number() = permNumber;

      p->mixedTags() = validSeq_Mixed.second;

      markWithFareTypes(*p);
      mapKeepFares(*p);

      if ((permPassed = checkElectronicTicket(*p, validationOut)) &&
          (permPassed = checkStopoverConnection(*p, connectionChanged,
                                                stopoverChanged, validationOut)))
      {
        saveFareBytesData(*p);
        checkFareRetrieval(*p);
        _trx.insert(*p);
        ++validPermCounter;
      }
      else
      {
        p = nullptr;
      }
    }

    if (_dc)
    {
      _dc->permStatus() = permPassed;
      *_dc << std::make_pair(&permSelector, validSeq_Mixed.first) << validationOut << p;
      _dc->printPermStatus();
    }

    LOG4CXX_DEBUG(_logger, "built permutation #" << permNumber);

    permSelector = cp.getNext();
  }

  mapExpndKeepFares();

  LOG4CXX_DEBUG(_logger, "Finish building " << permNumber << " permuations");
  LOG4CXX_INFO(_logger, "EXC_FC " << numFC << " PERMUTATIONS " << permNumber);
  setReissueExchangeROEConversionDate();
  if (_dc)
  {
    _dc->printKeepFareMapping();
    _dc->printSummary(_trx, permNumber);
  }
  if (_trx.allPermutationsRequireCurrentForFlown() &&
      _trx.allPermutationsRequireNotCurrentForFlown())
  {
    _trx.allPermutationsRequireCurrentForFlown() = false;
    _trx.allPermutationsRequireNotCurrentForFlown() = false;
  }

  CurrencyCode paymentCurrency = _trx.getOptions()->currencyOverride();
  std::for_each(_trx.processTagPermutations().begin(),
                _trx.processTagPermutations().end(),
                PenaltyEstimator(_unmatchedExcFCs, paymentCurrency, _trx));

  std::sort(_trx.processTagPermutations().begin(),
            _trx.processTagPermutations().end(),
            PenaltyComparer());

  if (!fallback::carrierApplicationOpt(&_trx))
  {
    _trx.optimizationMapper().processMapping(*excItin, *_trx.curNewItin(), _dc);

    std::vector<std::vector<const ProcessTagInfo*>> validPTIs(excItin->fareComponent().size());
    for (const ProcessTagPermutation* permutation : _trx.processTagPermutations())
    {
      uint32_t counter = 0;
      for (const ProcessTagInfo* pti : permutation->processTags())
      {
        validPTIs[counter++].push_back(pti);
      }
    }

    uint32_t counter = 0;
    auto cxrApplTblChecker = [](const ProcessTagInfo* pti)
                             { return pti->record3()->carrierApplTblItemNo() == 0; };

    auto privPubChecker = [](const ProcessTagInfo* pti)
                          { return pti->reissueSequence()->excludePrivate() == 'X' &&
                                   pti->reissueSequence()->ruleTariffNo() == 0; };

    for (const std::vector<const ProcessTagInfo*>& ptis : validPTIs)
    {
      if (std::all_of(ptis.begin(), ptis.end(), cxrApplTblChecker))
      {
        const FareCompInfo* fc = excItin->fareComponent()[counter];
        _trx.optimizationMapper().addCarrierRestriction(fc, { fc->fareMarket()->governingCarrier() });
      }

      if (fallback::excPrivatePublicOpt(&_trx) &&
          std::all_of(ptis.begin(), ptis.end(), privPubChecker))
      {
        const FareCompInfo* fc = excItin->fareComponent()[counter];
        _trx.optimizationMapper().addTariffRestriction(fc, ptfv[counter]);
      }
      ++counter;
    }
  }
}

void
PermutationGenerator::matchExcToNewFareMarket(const ExcItin* excItin)
{
  const std::vector<FareMarket*>& newFareMarkets = _trx.curNewItin()->fareMarket();

  for (const FareMarket* exc : excItin->fareMarket())
  {
    const FareMarket* singleNewHit = nullptr;
    int count = 0;

    for (const FareMarket* newFM : newFareMarkets)
    {
      if (newFM->boardMultiCity() == exc->boardMultiCity() &&
          newFM->offMultiCity() == exc->offMultiCity())
      {
        singleNewHit = newFM;
        if (++count > 1)
          break;
      }
    }

    if (count == 1)
    {
      FareCompInfo* fcInfo = excItin->findFareCompInfo(exc);
      if (fcInfo)
      {
        _matchedExcFCs.push_back(static_cast<uint16_t>(fcInfo->fareCompNumber() - 1));
        fcInfo->partialFareBreakLimitationValidation().setNewSegments(singleNewHit->travelSeg(),
                                                                      *_trx.curNewItin());
      }
    }
    else if (count == 0)
    {
      FareCompInfo* fcInfo = excItin->findFareCompInfo(exc);
      if (fcInfo)
      {
        _unmatchedExcFCs.push_back(static_cast<uint16_t>(fcInfo->fareCompNumber() - 1));
      }
    }
  }
}

void
PermutationGenerator::saveFareBytesData(ProcessTagPermutation& permutation)
{
  for (uint32_t i : _matchedExcFCs)
  {
    ProcessTagInfo* pti = permutation.processTags()[i];
    const FareMarket* fm = pti->paxTypeFare()->fareMarket();

    RexPricingTrx::BoardOffToFareBytesData::iterator result =
        _trx.fareBytesData().find(std::make_pair(fm->boardMultiCity(), fm->offMultiCity()));

    if (result != _trx.fareBytesData().end())
      ((*result).second).add(pti);
    else
      _trx.fareBytesData()[std::make_pair(fm->boardMultiCity(), fm->offMultiCity())] =
          FareBytesData(pti);
  }

  std::multimap<CarrierCode, FareBytesData>& unmatchedFCs = _trx.unmappedFareBytesData();
  for (uint16_t i : _unmatchedExcFCs)
  {
    ProcessTagInfo* pti = permutation.processTags()[i];
    const FareMarket* fm = pti->paxTypeFare()->fareMarket();
    unmatchedFCs.insert(
        std::pair<CarrierCode, FareBytesData>(fm->governingCarrier(), FareBytesData(pti)));
  }
}

bool
PermutationGenerator::isProperTagsSequence(
    CartesianProduct<std::vector<ProcessTagInfo*>>::ProductType& permSelector,
    std::string& validationOut)
{
  bool valid = true;
  CartesianProduct<std::vector<ProcessTagInfo*>>::ProductType::iterator iter =
      find_if(permSelector.begin(),
              permSelector.end(),
              std::bind2nd(isTagNumber(), REISSUE_DOWN_TO_LOWER_FARE));
  if (iter != permSelector.end())
  {
    if (iter != permSelector.begin())
      valid = false;
    else
    {
      if (find_if(iter + 1,
                  permSelector.end(),
                  std::not1(std::bind2nd(isTagNumber(), REISSUE_DOWN_TO_LOWER_FARE))) !=
          permSelector.end())
        valid = false;
    }
  }

  if (!valid)
    validationOut = " \nTAG 7 CHECK: FAILED";

  return valid;
}

const std::pair<bool, bool>
PermutationGenerator::permCharacteristic(
    const CartesianProduct<std::vector<ProcessTagInfo*>>::ProductType& permSelector,
    std::string& validationOut) const
{
  std::pair<bool, bool> validSeq_Mixed(true, true);

  if (std::adjacent_find(permSelector.begin(), permSelector.end(), DifferentTagNumber()) !=
      permSelector.end())
  {
    if (std::find_if(permSelector.begin(),
                     permSelector.end(),
                     std::bind2nd(isTagNumber(), REISSUE_DOWN_TO_LOWER_FARE)) != permSelector.end())
      validSeq_Mixed.first = false;
  }
  else
    validSeq_Mixed.second = false;

  if (_dc)
  {
    if (!validSeq_Mixed.first)
      validationOut = " \nTAG 7 CHECK: FAILED";
    else if (validSeq_Mixed.second)
      validationOut = " \nMIXED TAG PERMUTATION";
    else
    {
      if (!permSelector.empty())
      {
        if (permSelector.front()->reissueSequence()->orig())
          validationOut = " \nSINGLE TAG " +
                          boost::lexical_cast<std::string>(
                              permSelector.front()->reissueSequence()->processingInd()) +
                          " PERMUTATION";
        else
          validationOut = " \nWHOLE ZERO T988 PERMUTATION";
      }
    }
  }

  return validSeq_Mixed;
}

void
PermutationGenerator::checkFareRetrieval(const ProcessTagPermutation& perm)
{
  typedef std::map<FareApplication, ProcessTagInfo*> FareApplWinnerTagsType;
  for (const FareApplWinnerTagsType::value_type& pair : perm.fareApplWinnerTags())
  {
    FareMarket::FareRetrievalFlags flag = FareMarket::RetrievNone;
    switch (pair.first)
    {
    case HISTORICAL:
      if (!_trx.needRetrieveHistoricalFare())
        flag = FareMarket::RetrievHistorical;
      _trx.fareRetrievalFlags().set(FareMarket::MergeHistorical);
      break;

    case CURRENT:
      if (!_trx.needRetrieveCurrentFare())
        flag = FareMarket::RetrievCurrent;
      break;

    case TRAVEL_COMMENCEMENT:
      if (!_trx.needRetrieveTvlCommenceFare())
        flag = FareMarket::RetrievTvlCommence;
      break;

    case KEEP:
      if (!_trx.needRetrieveKeepFare())
        flag = FareMarket::RetrievKeep;
      break;

    default:
      break;
    }

    if (flag != FareMarket::RetrievNone)
    {
      _trx.fareRetrievalFlags().set(flag);
    }
  }

  if (perm.hasTag7only())
  {
    if (perm.getReissueToLowerByte() != ProcessTagPermutation::REISSUE_TO_LOWER_BLANK)
      _trx.fareRetrievalFlags().set(_trx.lastTktReIssueDT() != DateTime::emptyDate()
                                        ? FareMarket::RetrievLastReissue
                                        : FareMarket::RetrievHistorical);
  }

  if (!_trx.repriceWithDiffDates() && perm.fareApplWinnerTags().size() > 1)
    _trx.repriceWithDiffDates() = true;

  if (!_trx.needRetrieveKeepFare() && perm.hasZeroT988())
  {
    _trx.fareRetrievalFlags().set(FareMarket::RetrievKeep);

    if (!perm.fareApplWinnerTags().empty() && perm.fareApplWinnerTags().begin()->first != KEEP)
      _trx.repriceWithDiffDates() = true;
  }
}

bool
PermutationGenerator::checkElectronicTicket(ProcessTagPermutation& permutation,
                                            std::string& validationOut)
{
  if (permutation.checkTable988Byte123() != ProcessTagPermutation::ELECTRONIC_TICKET_MIXED)
    return true;
  validationOut += " \nELECTRONIC TICKET: FAILED";
  return false;
}

void
PermutationGenerator::markWithFareTypes(ProcessTagPermutation& perm)
{
  if (_dc)
    _dc->fcStatuses().clear();

  bool haveUnchanged(false);
  bool haveChanged(false);
  bool haveUnchangedAfter(false);
  bool notAllZeroT988 = false;
  FareApplication fareAppl(UNKNOWN_FA);

  for (const ProcessTagInfo* pti : perm.processTags())
  {
    const PaxTypeFare* paxTypeFare = pti->paxTypeFare();
    if (paxTypeFare != nullptr)
    {
      const FCChangeStatus& changeStatus = paxTypeFare->fareMarket()->changeStatus();

      if (!pti->reissueSequence()->orig())
      {
        updatePermuationInfoForFareMarkets(KEEP, changeStatus);
        markKeepForZeroT988(perm, *paxTypeFare);
        continue;
      }

      notAllZeroT988 = true;

      if (!haveUnchangedAfter && changeStatus == UN)
        haveUnchangedAfter = true;
      if (!haveUnchanged && changeStatus == UU)
        haveUnchanged = true;
      if (!haveChanged && changeStatus == UC)
        haveChanged = true;

      fareAppl = perm.fareTypeSelection(changeStatus);
      if (fareAppl == UNKNOWN_FA) // Not processed yet
      {
        fareAppl = TagWarEngine::getFA(perm, changeStatus, _travelCommenced);
        if (changeStatus == FL)
        {
          saveFlownFcFareAppl(paxTypeFare->fareMarket(), fareAppl);
        }
        perm.setFareTypeSelection(changeStatus, fareAppl);
        perm.setRebookFareTypeSelection(changeStatus, fareAppl);
      }
      updatePermuationInfoForFareMarkets(fareAppl, changeStatus);
      perm.fareApplMap().insert(std::make_pair(paxTypeFare, fareAppl));
      if (_dc)
        _dc->fcStatuses().insert(changeStatus);
    }
  }

  if (notAllZeroT988)
  {
    if (haveChanged)
    {
      if (_dc)
        _dc->fcStatuses().insert(UU);
      fareAppl = TagWarEngine::getFA(perm, UU, _travelCommenced);
      perm.setFareTypeSelection(UU, fareAppl);
      updatePermuationInfoForFareMarkets(fareAppl, UU);

      if (_dc)
        _dc->fcStatuses().insert(UN);
      fareAppl = TagWarEngine::getFA(perm, UN, _travelCommenced);
      perm.setFareTypeSelection(UN, fareAppl);
      updatePermuationInfoForFareMarkets(fareAppl, UN);
    }
    else
    {
      if (_dc)
        _dc->fcStatuses().insert(UC);
      fareAppl = TagWarEngine::getFA(perm, UC, _travelCommenced);
      perm.setFareTypeSelection(UC, fareAppl);
      updatePermuationInfoForFareMarkets(fareAppl, UC);
      if (_dc)
        _dc->fcStatuses().insert(UN);
      fareAppl = TagWarEngine::getFA(perm, UN, _travelCommenced);
      perm.setFareTypeSelection(UN, fareAppl);
      updatePermuationInfoForFareMarkets(fareAppl, UN);
    }
    fareAppl = TagWarEngine::getFA(perm, UU, _travelCommenced);
    perm.setRebookFareTypeSelection(UU, fareAppl);
    fareAppl = TagWarEngine::getFA(perm, UN, _travelCommenced);
    perm.setRebookFareTypeSelection(UN, fareAppl);
    fareAppl = TagWarEngine::getFA(perm, UC, _travelCommenced);
    perm.setRebookFareTypeSelection(UC, fareAppl);
  }
}

void
PermutationGenerator::markKeepForZeroT988(ProcessTagPermutation& perm,
                                          const PaxTypeFare& paxTypeFare)
{
  perm.setFareTypeSelection(paxTypeFare.fareMarket()->changeStatus(), KEEP);
  perm.setRebookFareTypeSelection(paxTypeFare.fareMarket()->changeStatus(), KEEP);
  perm.fareApplMap().insert(std::make_pair(&paxTypeFare, KEEP));

  if (paxTypeFare.fareMarket()->changeStatus() == FL)
    saveFlownFcFareAppl(paxTypeFare.fareMarket(), KEEP);

  if (_trx.newItinKeepFares().find(&paxTypeFare) == _trx.newItinKeepFares().end())
  {
    FareMarket* matchedNewItinFm = matchToNewItinFmForKeepFare(paxTypeFare.fareMarket());

    if (matchedNewItinFm)
      _trx.newItinKeepFares().insert(std::make_pair(&paxTypeFare, matchedNewItinFm));
  }
}

void
PermutationGenerator::saveFlownFcFareAppl(const FareMarket* fareMarket,
                                          const FareApplication& fareAppl)
{
  FareMarket::FareRetrievalFlags flag = getRetrievalFlag(fareAppl);

  std::map<const FareMarket*, FareMarket::FareRetrievalFlags>::iterator iter =
      _trx.excFlownFcFareAppl().find(fareMarket);
  if (iter != _trx.excFlownFcFareAppl().end())
    iter->second = (FareMarket::FareRetrievalFlags)(iter->second | flag);
  else
    _trx.excFlownFcFareAppl().insert(std::make_pair(fareMarket, flag));
}

FareMarket::FareRetrievalFlags
PermutationGenerator::getRetrievalFlag(const FareApplication& fareAppl)
{
  switch (fareAppl)
  {
  case HISTORICAL:
    return FareMarket::RetrievHistorical;
  case CURRENT:
    return FareMarket::RetrievCurrent;
  case TRAVEL_COMMENCEMENT:
    return FareMarket::RetrievTvlCommence;
  case KEEP:
    return FareMarket::RetrievKeep;
  default:
    return FareMarket::RetrievNone;
  }
}

bool
isNotCOSChanged(const TravelSeg* ts)
{
  return (ts->changeStatus() == TravelSeg::CHANGED && !ts->isCabinChanged());
}

bool
PermutationGenerator::mapKeepForComponent(const ProcessTagPermutation& perm,
                                          const PaxTypeFare& excPtf,
                                          FareApplication fa) const
{
  return !(fa != KEEP &&
           !(excPtf.fareMarket()->changeStatus() == UC &&
             (perm.rebookFareTypeSelection(UU) == KEEP ||
              perm.rebookFareTypeSelection(UN) == KEEP) &&
             std::find_if(excPtf.fareMarket()->travelSeg().begin(),
                          excPtf.fareMarket()->travelSeg().end(),
                          isNotCOSChanged) == excPtf.fareMarket()->travelSeg().end()));
}

void
PermutationGenerator::mapKeepFares(ProcessTagPermutation& perm)
{
  if (perm.needKeepFare())
  {
    for (const ProcessTagPermutation::PaxTypeFareApplication::value_type& fareApplPair :
         perm.fareApplMap())
    {
      const PaxTypeFare& excPtf = *fareApplPair.first;

      if (mapKeepForComponent(perm, excPtf, fareApplPair.second) &&
          _trx.newItinKeepFares().find(&excPtf) == _trx.newItinKeepFares().end())
      {
        FareMarket* newFm = matchToNewItinFmForKeepFare(excPtf.fareMarket());
        if (newFm)
          _trx.newItinKeepFares().insert(std::make_pair(&excPtf, newFm));
      }
    }
  }
}

void
PermutationGenerator::mapExpndKeepFares()
{
  for (const ProcessTagPermutation* perm : _trx.processTagPermutations())
  {
    if (perm->needKeepFare())
    {
      for (const ProcessTagPermutation::PaxTypeFareApplication::value_type& fareApplPair :
           perm->fareApplMap())
      {
        const PaxTypeFare& excPtf = *fareApplPair.first;

        if (mapKeepForComponent(*perm, excPtf, fareApplPair.second) &&
            perm->needExpndKeepFare(*excPtf.fareMarket()))
          matchToNewItinFmForExpndKeepFare(excPtf);
      }
    }
  }
}

namespace
{
class NotIntlExpndCandidate : public std::unary_function<const FareMarket*, bool>
{
  const FareMarket& _excFm;

public:
  NotIntlExpndCandidate(const FareMarket& excFm) : _excFm(excFm) {}

  bool operator()(FareMarket* newFm)
  {
    return _excFm.governingCarrier() != newFm->governingCarrier() ||
           _excFm.geoTravelType() != newFm->geoTravelType() ||
           !RexPricingTrx::expndKeepSameNation(*_excFm.origin(), *newFm->origin()) ||
           !RexPricingTrx::expndKeepSameNation(*_excFm.destination(), *newFm->destination());
  }
};

class NotDomesticExpndCandidate : public std::unary_function<const FareMarket*, bool>
{
  const FareMarket& _excFm;

public:
  NotDomesticExpndCandidate(const FareMarket& excFm) : _excFm(excFm) {}

  bool operator()(FareMarket* newFm)
  {
    return _excFm.governingCarrier() != newFm->governingCarrier() ||
           _excFm.boardMultiCity() != newFm->boardMultiCity() ||
           _excFm.offMultiCity() != newFm->offMultiCity();
  }
};

class MakeExpndMapItem
    : public std::unary_function<const FareMarket*, RexPricingTrx::ExpndKeepMap::value_type>
{
  const PaxTypeFare& _excPtf;

public:
  MakeExpndMapItem(const PaxTypeFare& excPtf) : _excPtf(excPtf) {}

  RexPricingTrx::ExpndKeepMap::value_type operator()(FareMarket* newFm)
  {
    return std::make_pair(newFm, &_excPtf);
  }
};

class AlreadyProcessed
    : public std::unary_function<const RexPricingTrx::ExpndKeepMap::value_type&, bool>
{
  const PaxTypeFare& _excPtf;

public:
  AlreadyProcessed(const PaxTypeFare& excPtf) : _excPtf(excPtf) {}

  bool operator()(const RexPricingTrx::ExpndKeepMap::value_type& mapItem)
  {
    return mapItem.second == &_excPtf;
  }
};

} // namespace

void
PermutationGenerator::matchToNewItinFmForExpndKeepFare(const PaxTypeFare& excPtf)
{
  if (std::find_if(_trx.expndKeepMap().begin(),
                   _trx.expndKeepMap().end(),
                   AlreadyProcessed(excPtf)) != _trx.expndKeepMap().end())
    return;

  std::vector<FareMarket*> sortedNewFMs;

  if (_trx.exchangeItin().front()->geoTravelType() != GeoTravelType::International)
    std::remove_copy_if(_trx.curNewItin()->fareMarket().begin(),
                        _trx.curNewItin()->fareMarket().end(),
                        std::back_inserter(sortedNewFMs),
                        NotDomesticExpndCandidate(*excPtf.fareMarket()));
  else
    std::remove_copy_if(_trx.curNewItin()->fareMarket().begin(),
                        _trx.curNewItin()->fareMarket().end(),
                        std::back_inserter(sortedNewFMs),
                        NotIntlExpndCandidate(*excPtf.fareMarket()));

  std::transform(sortedNewFMs.begin(),
                 sortedNewFMs.end(),
                 std::inserter(_trx.expndKeepMap(), _trx.expndKeepMap().end()),
                 MakeExpndMapItem(excPtf));
}

FareMarket*
PermutationGenerator::matchToNewItinFmForKeepFare(const FareMarket* excItinFm)
{
  if (excItinFm == nullptr)
    return nullptr;

  const bool matchSameChangeStatus = (excItinFm->changeStatus() != UC); // when all tag 1
  for (FareMarket* newItinFm : _trx.curNewItin()->fareMarket())
  {
    RexPricingTrx::NewToExcItinFareMarketMapForKeep::iterator mappedIter =
        _trx.newToExcItinFareMarketMapForKeep().find(newItinFm);
    if (mappedIter != _trx.newToExcItinFareMarketMapForKeep().end())
    {
      if (mappedIter->second == excItinFm)
        return mappedIter->first;
      else
        continue; // It has been mapped to other exc fare component
    }

    if ((!matchSameChangeStatus || (newItinFm->changeStatus() == excItinFm->changeStatus())) &&
        (newItinFm->boardMultiCity() == excItinFm->boardMultiCity()) &&
        (newItinFm->offMultiCity() == excItinFm->offMultiCity()) &&
        (newItinFm->governingCarrier() == excItinFm->governingCarrier()) &&
        (newItinFm->geoTravelType() == excItinFm->geoTravelType()) &&
        (newItinFm->sideTripTravelSeg().size() == excItinFm->sideTripTravelSeg().size()))
    {
      _trx.newToExcItinFareMarketMapForKeep().insert(std::make_pair(newItinFm, excItinFm));
      return newItinFm;
    }
  }

  return nullptr;
}

void
PermutationGenerator::checkStopByte(std::vector<ProcessTagInfo*>& FCtags)
{ // to be implemented
}

bool
PermutationGenerator::overrideProcessTagInfoData(
    CartesianProduct<std::vector<ProcessTagInfo*>>::ProductType& permSelector,
    std::string& postRuleValidationOut)
{
  bool result(true);

  for (ProcessTagInfo*& pt : permSelector)
  {
    FareCompInfo* fc = pt->fareCompInfo();
    const VoluntaryChangesInfo* r3 = pt->record3()->orig();
    FareCompInfo::OverridingIntlFcData* od = fc->findOverridingData(r3);
    if (!od)
      continue;

    ProcessTagInfo* overPti = permSelector.at(fc->getOverridingFc(od) - 1);

    if (!overPti->reissueSequence()->orig())
      continue;

    FareCompInfo::SkippedValidationsSet* svs = fc->getSkippedValidationsSet(od);

    /*
      Normally (whithout diagnostic 688) here when false we break and return result.
      We don't need to processs all Process Tags since permutation will killed anyway.

      When diagnostic 688 is enabled we go through all Process Tags to prepare proper permutation
      with overriden data like in case of valid permutations and display one in diagnostic.
      This is to display whole overriden permutation in 688 and not one partially processed -
      partially overriden and partially not
    */
    if (result)
    {
      const FareMarket& fm = *pt->paxTypeFare()->fareMarket();
      /*
        When diagnostic 688 is enabled we do once again validation of domestic FC with domestic T988
        record
        to display the reason why we do revalidation.
      */
      if (_dc331)
      {
        _dc331->str("");
        *_dc331 << " \nVALIDATION PRIOR TO INTL OVERRIDE (FC" << fc->fareCompNumber() << " "
                << fm.boardMultiCity() << fm.offMultiCity() << ")\n";

        const size_t count = _dc331->str().length();

        postRuleValidation(fm, *pt->reissueSequence()->orig(), *svs, true);
        if (count != _dc331->str().length())
          *_dc331 << " \nVALIDATION AFTER INTL OVERRIDE\n";
        else
          _dc331->str("");
      }

      result = postRuleValidation(fm, *overPti->reissueSequence()->orig(), *svs);
      if (!result && !_dc)
        break;

      if (_dc331)
        postRuleValidationOut += _dc331->str();
    }

    if (!pt->record3()->overriding())
    {
      pt->record3()->overriding() = overPti->record3()->orig();
      pt->reissueSequence()->overriding() = overPti->reissueSequence()->orig();
      pt->overridingPTF() = overPti->paxTypeFare();
      pt->record3()->setConditionallyOverridenBytes(VoluntaryChangesInfoW::coByte47_75,
                                                    svs->isSet(svOriginallyScheduledFlight));

      addPtiToCache(pt, overPti, pt);
    }
    else
    {
      ProcessTagInfo* pti = findPtiInCache(pt, overPti);
      if (!pti)
      {
        _trx.dataHandle().get(pti);
        pti->paxTypeFare() = pt->paxTypeFare();
        pti->fareCompInfo() = fc;
        pti->reissueSequence()->orig() = pt->reissueSequence()->orig();
        pti->record3()->orig() = r3;

        pti->record3()->overriding() = overPti->record3()->orig();
        pti->reissueSequence()->overriding() = overPti->reissueSequence()->orig();
        pti->overridingPTF() = overPti->paxTypeFare();
        pti->record3()->setConditionallyOverridenBytes(VoluntaryChangesInfoW::coByte47_75,
                                                       svs->isSet(svOriginallyScheduledFlight));

        addPtiToCache(pt, overPti, pti);
      }
      pt = pti;
    }
  }

  return result;
}

bool
PermutationGenerator::postRuleValidation(const FareMarket& fm,
                                         const ReissueSequence& reissueSeq,
                                         FareCompInfo::SkippedValidationsSet& svs,
                                         bool runForDomestic)
{
  ReissueTable t988Val(_trx, getExcItin(), _dc331);
  if (svs.isSet(svAgencyRestrictions) && !t988Val.matchAgencyRestrictions(reissueSeq) &&
      !runForDomestic)
  {
    return false;
  }

  if (svs.isSet(svOriginallyScheduledFlight) &&
      !t988Val.matchOriginallyScheduledFlight(fm, reissueSeq) && !runForDomestic)
  {
    return false;
  }

  return true;
}

const Itin*
PermutationGenerator::getExcItin()
{
  return _trx.exchangeItin().front();
}

void
PermutationGenerator::setReissueExchangeROEConversionDate()
{
  if (!_trx.applyReissueExchange())
    return;
  if (!_trx.newItin().empty())
  {
    bool useCurrentDate = false; // Historical
    if (_trx.curNewItin()->exchangeReissue() == EXCHANGE)
    {
      if (_trx.needRetrieveCurrentFare())
        useCurrentDate = true; // current
    }
    else // Reissue
    {
      if (_trx.needRetrieveCurrentFare() && !_trx.needRetrieveHistoricalFare() &&
          !_trx.needRetrieveTvlCommenceFare() && !_trx.needRetrieveKeepFare())
        useCurrentDate = true; // current
    }
    if (useCurrentDate)
      _trx.newItinROEConversionDate() = _trx.currentTicketingDT();
    else
      _trx.newItinROEConversionDate() = getPreviousDate();

    if (_trx.needRetrieveCurrentFare() &&
        (_trx.needRetrieveHistoricalFare() || _trx.needRetrieveTvlCommenceFare() ||
         _trx.needRetrieveKeepFare()))
    {
      if (useCurrentDate)
        _trx.newItinSecondROEConversionDate() = getPreviousDate();
      else
        _trx.newItinSecondROEConversionDate() = _trx.currentTicketingDT();
    }
  }
}

void
PermutationGenerator::setHealthCheckConfig()
{
  if (!_healthCheckConfigured)
  {
    boost::lock_guard<boost::mutex> guard(_mutex);
    if (_healthCheckConfigured)
      return;
    _maxPermNumber = maxPermNumber.getValue();
    _maxMemoryGrow = maxMemoryGrow.getValue();
    _maxVM = maxVM.getValue();
    _minAvailMem = minAvailMem.getValue();
    _healthCheckConfigured = true;
  }
}

void
PermutationGenerator::checkHealth(const uint32_t& permNumber,
                                  const size_t& startVM,
                                  size_t startRSS,
                                  const size_t& numFC)
{
  if (_maxPermNumber > 0 && permNumber > _maxPermNumber)
  {
    LOG4CXX_ERROR(_logger,
                  "EXC_FC " << numFC << " PERMUTATIONS " << (permNumber - 1) << " MAX_REACHED");
    throw ErrorResponseException(ErrorResponseException::MAX_NUMBER_COMBOS_EXCEEDED);
  }

  if (permNumber % HEALTH_CHECK_INTERVAL == 0)
  {
    // Check memory grow
    if (_maxMemoryGrow > 0)
    {
      bool willThrow(false);
      if (_minAvailMem > 0)
      {
        size_t curRSS;
        size_t curAvail;
        if (!Memory::changesFallback)
        {
          curRSS =
              Memory::MemoryMonitor::instance()->getUpdatedResidentMemorySize() / (1024 * 1024);
          curAvail =
              Memory::GlobalManager::instance()->getUpdatedAvailableMemorySize() / (1024 * 1024);
        }
        else
        {
          curRSS = MemoryUsage::getResidentMemorySize() / (1024 * 1024);
          curAvail = MemoryUsage::getAvailableMemory() / 1024;
        }
        LOG4CXX_DEBUG(_logger, "startRSS=" << startRSS << " curAvail=" << curAvail);
        willThrow = (curAvail < _minAvailMem + 1024) && (curRSS > _maxMemoryGrow + startRSS);
      }
      else if (_maxVM > 0)
      {
        size_t curVM;
        if (!Memory::changesFallback)
        {
          curVM = Memory::MemoryMonitor::instance()->getUpdatedVirtualMemorySize() / (1024 * 1024);
        }
        else
        {
          curVM = MemoryUsage::getVirtualMemorySize() / (1024 * 1024);
        }
        LOG4CXX_DEBUG(_logger, "StartVM=" << startVM << " curVM=" << curVM);
        willThrow = (curVM >= _maxVM - 1024) && (curVM - startVM > _maxMemoryGrow);
      }
      if (willThrow)
      {
        LOG4CXX_ERROR(_logger,
                      "EXC_FC " << numFC << " PERMUTATIONS " << (permNumber - 1)
                                << " MEMORY_REACHED");
        throw ErrorResponseException(ErrorResponseException::MAX_NUMBER_COMBOS_EXCEEDED);
      }
    }

    // Check time out
    try
    {
      if (fallback::reworkTrxAborter(&_trx))
        checkTrxAborted(_trx);
      else
        _trx.checkTrxAborted();
    }
    catch (ErrorResponseException& ex)
    {
      LOG4CXX_ERROR(_logger,
                    "EXC_FC " << numFC << " PERMUTATIONS " << (permNumber - 1)
                              << " TIMEOUT_REACHED");
      throw ErrorResponseException(ErrorResponseException::MAX_NUMBER_COMBOS_EXCEEDED);
    }
  }
}

const DateTime&
PermutationGenerator::getPreviousDate()
{
  if (!_trx.previousExchangeDT().isEmptyDate())
    return _trx.previousExchangeDT();
  return _trx.originalTktIssueDT();
}

void
PermutationGenerator::updatePermuationInfoForFareMarkets(const FareApplication& fareAppl,
                                                         const FCChangeStatus& changeStatus)
{
  if (changeStatus == FL)
  {
    _trx.allPermutationsRequireCurrentForFlown() &= fareAppl == CURRENT;
    _trx.allPermutationsRequireNotCurrentForFlown() &=
        (fareAppl == KEEP || fareAppl == HISTORICAL || fareAppl == TRAVEL_COMMENCEMENT);
  }
  else
  {
    _trx.allPermutationsRequireCurrentForUnflown() &= fareAppl == CURRENT;
    _trx.allPermutationsRequireNotCurrentForUnflown() &=
        (fareAppl == KEEP || fareAppl == HISTORICAL || fareAppl == TRAVEL_COMMENCEMENT);
  }
}

std::tuple<bool, bool>
PermutationGenerator::stopoversOrConnectionChanged(const FarePath& fp,
                                                   const std::vector<TravelSeg*>& curTvlSegs) const
{
  bool connectionChanged = false;
  bool stopoverChanged = false;
  std::vector<TravelSeg*> segs;

  for (auto pu : fp.pricingUnit())
  {
    for (auto fu : pu->fareUsage())
    {
      std::copy(fu->travelSeg().begin(), fu->travelSeg().end() - 1, std::back_inserter(segs));
    }
  }

  for (TravelSeg* seg : segs)
  {
    TravelSeg* foundSegment = nullptr;
    for (TravelSeg* newSeg : curTvlSegs)
    {
      if (seg->origin()->loc() == newSeg->origin()->loc() &&
          seg->destination()->loc() == newSeg->destination()->loc())
      {
        foundSegment = newSeg;
      }
    }

    if (seg->isForcedStopOver() || seg->stopOver())
    {
      if (!foundSegment || !(foundSegment->isForcedStopOver() || foundSegment->stopOver()))
      {
        stopoverChanged = true;
      }
    }
    else if (seg->isForcedConx() || !seg->stopOver())
    {
      if (!foundSegment || !(foundSegment->isForcedConx() || !foundSegment->stopOver()))
      {
        connectionChanged = true;
      }
    }
  }

  return std::make_tuple(connectionChanged, stopoverChanged);
}

bool
PermutationGenerator::checkStopoverConnection(ProcessTagPermutation& permutation,
                                              bool connectionChanged,
                                              bool stopoverChanged,
                                              std::string& validationOut) const
{
  switch (permutation.getStopoverConnectionByte())
  {
  case ProcessTagPermutation::STOPCONN_BLANK:
    return true;

  case ProcessTagPermutation::STOPCONN_B:
  {
    if (stopoverChanged || connectionChanged)
    {
      validationOut = "CONNECTION AND STOPOVER CHANGE IS NOT ALLOWED - FAILED\n";
      return false;
    }
    return true;
  }

  case ProcessTagPermutation::STOPCONN_C:
  {
    if (connectionChanged)
    {
      validationOut = "CONNECTION CHANGE IS NOT ALLOWED - FAILED\n";
      return false;
    }
    return true;
  }

  case ProcessTagPermutation::STOPCONN_S:
  {
    if (stopoverChanged)
    {
      validationOut = "STOPOVER CHANGE IS NOT ALLOWED - FAILED\n";
      return false;
    }
    return true;
  }
  }
  return true;
}

} // tse
