//-------------------------------------------------------------------
//
//  File:        RepriceSolutionValidator.cpp
//  Created:     September 10, 2007
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

#include "RexPricing/RepriceSolutionValidator.h"

#include "BookingCode/FareBookingCodeValidator.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/FlownStatusCheck.h"
#include "Common/Logger.h"
#include "Common/MemoryUsage.h"
#include "Common/Money.h"
#include "Common/TrxUtil.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TSEException.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Billing.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FarePath.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/ProcessTagPermutation.h"
#include "DataModel/ReissueCharges.h"
#include "DataModel/RexPricingOptions.h"
#include "DataModel/RexPricingRequest.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/CarrierApplicationInfo.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/DateOverrideRuleItem.h"
#include "DBAccess/SamePoint.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag602Collector.h"
#include "Diagnostic/Diag689Collector.h"
#include "MinFares/MinFareChecker.h"
#include "Pricing/Combinations.h"
#include "Pricing/FarePathUtils.h"
#include "RexPricing/CommonSolutionValidator.h"
#include "RexPricing/FarePathChangeDetermination.h"
#include "RexPricing/ReissuePenaltyCalculator.h"
#include "RexPricing/ReissueToLowerValidator.h"
#include "RexPricing/RexAdvResTktValidator.h"
#include "RexPricing/SequenceStopByteByTag.h"
#include "RexPricing/TagWarEngine.h"
#include "Rules/CategoryRuleItemSet.h"
#include "Rules/ReissueTable.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Rules/RuleUtil.h"

#include <boost/bind.hpp>

#include <algorithm>
#include <limits>
#include <utility>

namespace tse
{
FALLBACK_DECL(reworkTrxAborter);
namespace
{
ConfigurableValue<uint32_t>
maxDiagSize("OUTPUT_LIMITS", "MAX_PSS_OUTPUT");

Logger
logger("atseintl.RexPricing.RepriceSolutionValidator");
} // namespace

RepriceSolutionValidator::RepriceSolutionValidator(RexPricingTrx& trx, FarePath& fp)
  : _trx(trx),
    _farePath(fp),
    _dc(nullptr),
    _journeyLatestBD(nullptr),
    _firstFare(nullptr),
    _retrievalFlag(FareMarket::RetrievNone),
    _journeyCheckingFlag(false),
    _travelCommenced(false),
    _lowestChangeFee(std::numeric_limits<double>::max()),
    _lowestChangeFeeInQueue(0.0),
    _permWithLowestChangeFee(nullptr),
    _sequenceStopByteByTag(nullptr),
    _byte156ValueB(NEW_TICKET_EQUAL_OR_HIGHER_NOT_PROCESSED),
    _byte156ValueN(NEW_TICKET_EQUAL_OR_HIGHER_NOT_PROCESSED),
    _isRebookSolution(_farePath.rebookClassesExists()),
    _maxDiagSize(0),
    _genericRexMapper(trx, &_allRepricePTFs),
    _farePathChangeDetermination(nullptr),
    _expndKeepValidator(trx),
    _fareBytesValidator(trx,
                        _allRepricePTFs,
                        _itinIndex,
                        &_genericRexMapper,
                        trx.exchangeItin().front()->farePath().front()),
    _newTicketEqualOrHigherValidator(trx, fp, nullptr),
    _stopoverConnectionValidator(trx, fp, nullptr)
{
  LOG4CXX_DEBUG(logger, "RepriceSolutionValidator constructor");

  if (_trx.diagnostic().diagnosticType() == Diagnostic689)
  {
    _maxDiagSize = maxDiagSize.getValue();
    if (_maxDiagSize == 0)
      _maxDiagSize = DIAG_OUTPUT_LIMIT;
    LOG4CXX_DEBUG(logger, "maxDiagSize=" << _maxDiagSize);

    DCFactory* factory = DCFactory::instance();
    _dc = dynamic_cast<Diag689Collector*>(factory->create(_trx));
    if (_dc != nullptr)
    {
      _dc->enable(Diagnostic689);

      if (!_dc->isActive())
      {
        _dc = nullptr;
      }
      else
      {
        _dc->initialize();
        _dc->filterByFarePath(_farePath);
        if (!_dc->filterPassed())
        {
          _dc = nullptr;
        }
      }
    }
  }

  _newTicketEqualOrHigherValidator.setDiagnostic(_dc);
  _stopoverConnectionValidator.setDiagnostic(_dc);

  _fcMapping.resize(_trx.exchangeItin().front()->fareComponent().size());
  _fmMapping.resize(_trx.exchangeItin().front()->fareComponent().size());
  _calcCurrCode = _farePath.itin() ? _farePath.itin()->calculationCurrency() : "";
  _travelCommenced = !_trx.exchangeItin().front()->travelSeg().front()->unflown();
  if (_trx.getTrxType() == PricingTrx::MIP_TRX)
    _itinIndex = _trx.getItinPos(_farePath.itin());
  else
    _itinIndex = 0;

  _permCheckInterval = TrxUtil::permCheckInterval(_trx);
  _solutionCountLimit = TrxUtil::maxCat31SolutionCount(_trx);
  _memCheckTrxInterval = TrxUtil::getMemCheckTrxInterval(_trx);

  if (_solutionCountLimit > 0 && _trx.nbrOfSolutionsBuilt() > _solutionCountLimit)
  {
    if (_dc != nullptr)
    {
      *((DiagCollector*)_dc) << _trx.nbrOfSolutionsBuilt()
                             << " NUMBER OF SOLUTIONS BUILT EXCEEDED MAX " << _solutionCountLimit
                             << " ATTEMPTS" << std::endl;
      _dc->flushMsg();
      _dc = nullptr;
    }
    LOG4CXX_ERROR(logger,
                  "TXN:" << _trx.transactionId() << " - MAX NBR OF ATTEMPTED SOLUTIONS EXCEEDED");
    throw ErrorResponseException(ErrorResponseException::MAX_NUMBER_COMBOS_EXCEEDED);
  }

  _genericRexMapper.setDiag(_dc);
  _fareBytesValidator.setDiag(_dc);
}

RepriceSolutionValidator::~RepriceSolutionValidator()
{
  LOG4CXX_DEBUG(logger, "RepriceSolutionValidator destructor");

  if (_dc != nullptr)
  {
    _dc->flushMsg();
    _dc = nullptr;
  }
}

bool
RepriceSolutionValidator::skipByStopByte(const ProcessTagPermutation& permutation)
{
  return _sequenceStopByteByTag->skipByStopByte(permutation);
}

void
RepriceSolutionValidator::saveStopByteInfo(const ProcessTagPermutation& permutation)
{
  _sequenceStopByteByTag->saveStopByteInfo(permutation);
}

bool
RepriceSolutionValidator::process()
{
  uint32_t validPermutations = 0;
  SequenceStopByteByTag sequenceStopByteByTag;
  _sequenceStopByteByTag = &sequenceStopByteByTag;

  analyseExcFarePath();
  analyseFarePath();

  _farePathChangeDetermination = _trx.dataHandle().create<FarePathChangeDetermination>();
  _farePathChangeDetermination->determineChanges(_farePath, _trx);

  int permNumber = 0;

  RexAdvResTktValidator rexAdvResTktValidator(_trx,
                                              *_trx.newItin()[_itinIndex],
                                              _farePath,
                                              *_trx.exchangeItin().front()->farePath().front(),
                                              _advResOverrideCache,
                                              _dc,
                                              _genericRexMapper);
  rexAdvResTktValidator.permutationIndependentSetUp();

  if (_dc)
  {
    _dc->isRebookSolution() = _isRebookSolution;
    if (_trx.getTrxType() == PricingTrx::MIP_TRX && _trx.billing()->actionCode() == "WFR")
      *_dc << "NEW ITIN INDEX " << _itinIndex << std::endl;
    *_dc << _farePath;
  }

  _lowestChangeFeeInQueue =
      _trx.processTagPermutations(_itinIndex).front()->getEstimatedChangeFee();
  std::vector<ProcessTagPermutation*>::iterator ptIter =
      _trx.processTagPermutations(_itinIndex).begin();

  do
  {
    ++permNumber;
    checkTimeout(permNumber);
    checkMemory(permNumber);

    ProcessTagPermutation& permutation(**ptIter);
    if (_dc)
      _dc->permutationFilterPassed() = true;
    if (hasDiagAndFilterPassed())
    {
      if (_trx.getOptions()->AdvancePurchaseOption() == 'N')
        _dc->setCAT31OptionN() = true;
      *_dc << permutation;
    }

    if (skipByStopByte(permutation))
    {
      if (hasDiagAndFilterPassed())
      {
        for (const ProcessTagInfo* pt : permutation.processTags())
        {
          const FarePath& excFp = *_trx.exchangeItin().front()->farePath().front();
          _dc->printPermutationInfo(*pt, false, excFp);
        }
        printStopSequences(permutation);
        *_dc << "PERMUTATION " << permutation.number() << ": "
             << "SKIPPED\n";
        _dc->lineSkip(0);
      }
    }
    else
    {
      if (isFarePathValidForPermutation(permutation, rexAdvResTktValidator))
      {
        saveStopByteInfo(permutation);
        validPermutations++;

        if (permutationReissueChargesExists(&permutation))
        {
          if (_permutationReissueCharges[&permutation]->changeFee <
              permutation.getEstimatedChangeFee())
          {
            checkDiag();
            break;
          }
        }
      }
    }

    checkDiag();

  } while (++ptIter != _trx.processTagPermutations(_itinIndex).end());

  bool rc = setLowestChangeFee(validPermutations);
  _trx.incrementRSVCounter(rc);

  return rc;
}

void
RepriceSolutionValidator::printStopSequences(const ProcessTagPermutation& permutation)
{
  DiagCollector& dc(*_dc);
  for (const ProcessTagInfo* processTagInfo : permutation.processTags())
  {
    if (!processTagInfo->reissueSequence()->orig())
      continue;

    FareCompInfo* fareCompInfo(processTagInfo->fareCompInfo());
    ProcessTag tag((ProcessTag)processTagInfo->processTag());
    int sequence((ProcessTag)processTagInfo->seqNo());
    std::pair<ProcessTag, FareCompInfo*> key(tag, fareCompInfo);

    TagFcWithStopBytes::iterator item(_sequenceStopByteByTag->tagFcWithStopBytes().find(key));
    TagFcWithStopBytes::iterator NOT_FOUND(_sequenceStopByteByTag->tagFcWithStopBytes().end());
    if (item != NOT_FOUND)
    {
      std::pair<int, int> sequencePermutation;
      if (_sequenceStopByteByTag->findSequenceGreater(item->second, sequence, sequencePermutation))
      {
        int seqFound(sequencePermutation.first);
        int permutationNumber(sequencePermutation.second);
        dc << "SKIPPED BY PERM " << permutationNumber << " FC " << fareCompInfo->fareCompNumber()
           << " SEQ " << seqFound << std::endl;
      }
    }
  }
}

uint32_t
RepriceSolutionValidator::setLowestChangeFee(uint32_t validPermutations)
{
  MoneyAmount changeFee(0.0);
  ProcessTagPermutation* currentLowestPermutation(nullptr);
  typedef std::map<int, ProcessTagPermutation*> ByNumberType;
  ByNumberType byNumber;
  for (const ProcessTagPermutation* pt : _sequenceStopByteByTag->maybeSkipPermutations())
  {
    LOG4CXX_DEBUG(logger,
                  __FUNCTION__ << " check for out of order items to skip, fixup lowestChangeFee");
    ProcessTagPermutation& permutation(*const_cast<ProcessTagPermutation*>(pt));
    MoneyAmount currentFee((getTotalChangeFee(permutation)));
    if (skipByStopByte(permutation))
    {
      if (hasDiagAndFilterPassed())
      {
        DiagCollector& dc(*_dc);

        printStopSequences(permutation);
        dc << "PERMUTATION " << permutation.number() << ": PREVIOUSLY PASSED - SKIPPED\n";
        dc.lineSkip(0);
      }

      validPermutations--;
    }
    else
    {
      if (!currentLowestPermutation || changeFee + EPSILON > currentFee)
      {
        changeFee = getTotalChangeFee(permutation);
        currentLowestPermutation = &permutation;
        byNumber[permutation.number()] = &permutation;
      }
    }
  }

  for (const ByNumberType::value_type& it : byNumber)
  {
    MoneyAmount checkChangeFee(getTotalChangeFee(*(it.second)));
    if (changeFee + EPSILON >= checkChangeFee)
    {
      currentLowestPermutation = it.second;
      changeFee = checkChangeFee;
      break;
    }
  }

  if (validPermutations && currentLowestPermutation)
  {
    if (!_permWithLowestChangeFee || _lowestChangeFee > changeFee + EPSILON)
    {
      _lowestChangeFee = changeFee;
      _permWithLowestChangeFee = currentLowestPermutation;
      applyChangeFeeToFarePath();
    }
  }
  return validPermutations;
}

void
RepriceSolutionValidator::findFirstFare()
{
  const int16_t firstPNRSegment = _trx.newItin()[_itinIndex]->travelSeg().front()->pnrSegment();
  for (const PaxTypeFare* fare : _allRepricePTFs)
  {
    if (fare->fareMarket()->travelSeg().front()->pnrSegment() == firstPNRSegment)
    {
      _firstFare = fare;
      break;
    }
  }
}

void
RepriceSolutionValidator::findFareRetrievalFlag()
{
  _retrievalFlag = _allRepricePTFs.front()->retrievalFlag();
  std::vector<const PaxTypeFare*>::const_iterator iter = _allRepricePTFs.begin() + 1;
  while (iter != _allRepricePTFs.end())
  {
    FareMarket::FareRetrievalFlags paxFlag = (*iter)->retrievalFlag();
    _retrievalFlag = (FareMarket::FareRetrievalFlags)(_retrievalFlag & paxFlag);
    ++iter;
  }
}

struct RepriceSolutionValidator::SameFareBreaks
{
  SameFareBreaks(const FareCompInfo* fc)
    : _boardMultiCity(fc->fareMarket()->boardMultiCity()),
      _offMultiCity(fc->fareMarket()->offMultiCity())
  {
  }

  SameFareBreaks(const PaxTypeFare* ptf)
    : _boardMultiCity(ptf->fareMarket()->boardMultiCity()),
      _offMultiCity(ptf->fareMarket()->offMultiCity())
  {
  }

  SameFareBreaks(const LocCode boardMultiCity, const LocCode offMultiCity)
    : _boardMultiCity(boardMultiCity), _offMultiCity(offMultiCity)
  {
  }

  bool operator()(const PaxTypeFare* ptf) const
  {
    const FareMarket* repriceFM = ptf->fareMarket();
    return (repriceFM->boardMultiCity() == _boardMultiCity &&
            repriceFM->offMultiCity() == _offMultiCity);
  }
  const LocCode& _boardMultiCity;
  const LocCode& _offMultiCity;
};

bool
RepriceSolutionValidator::isPUPartiallyFlown(const PricingUnit* pu) const
{
  bool flownFind = false;
  bool unflownFind = false;

  for (const TravelSeg* ts : pu->travelSeg())
  {
    if (ts->unflown())
      unflownFind = true;
    else
      flownFind = true;

    if (unflownFind && flownFind)
    {
      return true;
    }
  }

  return false;
}

bool
RepriceSolutionValidator::isSameCpaCxrFareInExc(const FareUsage* fu) const
{
  const FareMarket& fm = *fu->paxTypeFare()->fareMarket();
  for (const FareUsage* fpfu : _trx.exchangeItin().front()->farePath().front()->flownOWFares())
  {
    const FareMarket& fpfm = *fpfu->paxTypeFare()->fareMarket();
    if (fpfm.boardMultiCity() == fm.boardMultiCity() && fpfm.offMultiCity() == fm.offMultiCity() &&
        fpfm.governingCarrier() == fm.governingCarrier())
    {
      LOG4CXX_DEBUG(logger, "same fare in exc");
      return true;
    }
  }

  return false;
}

void
RepriceSolutionValidator::analysePricingUnits()
{
  LOG4CXX_DEBUG(logger, "RepriceSolutionValidator::analysePricingUnits");

  for (const PricingUnit* pu : _farePath.pricingUnit())
  {
    const bool puIsPartiallyFlown = isPUPartiallyFlown(pu);

    for (FareUsage* fu : pu->fareUsage())
    {
      LOG4CXX_DEBUG(logger, "_journeyCheckingFlag 0: |" << _journeyCheckingFlag << "|");
      LOG4CXX_DEBUG(logger, "puIsPartiallyFlown 1: |" << puIsPartiallyFlown << "|");
      LOG4CXX_DEBUG(logger, "owrt 2: |" << fu->paxTypeFare()->owrt() << "|");
      LOG4CXX_DEBUG(
          logger, "new fu flown from FM 1:|" << fu->paxTypeFare()->fareMarket()->isFlown() << "|");

      if (!_journeyCheckingFlag && puIsPartiallyFlown &&
          fu->paxTypeFare()->fareMarket()->isFlown() &&
          fu->paxTypeFare()->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
      {
        _journeyCheckingFlag = isSameCpaCxrFareInExc(fu);
      }
      _allRepricePTFs.push_back(fu->paxTypeFare());

      if (fu->paxTypeFare()->retrievalFlag() & FareMarket::RetrievExpndKeep)
        _expndKeepValidator.newExpndPtfs().push_back(fu->paxTypeFare());

      if (_isRebookSolution)
      {
        size_t idx = 0;
        for (TravelSeg* ts : fu->travelSeg())
        {
          _ts2ss[ts] = &fu->segmentStatus()[idx++];
        }
      }
    }
  }
  if (_isRebookSolution)
    setFareMarketChangeStatus(_trx.exchangeItin().front(), _trx.newItin()[_itinIndex]);

  _genericRexMapper.map();
}

void
RepriceSolutionValidator::analyseFarePath()
{
  analysePricingUnits();

  for (const FareCompInfo* fc : _trx.exchangeItin().front()->fareComponent())
  {
    std::vector<const PaxTypeFare*>::iterator foundFC = _allRepricePTFs.begin();
    while ((foundFC = std::find_if(foundFC, _allRepricePTFs.end(), SameFareBreaks(fc))) !=
           _allRepricePTFs.end())
      _fcMapping[fc->fareCompNumber() - 1].push_back(*foundFC++);

    if (fc->fareMarket()->isFlown())
    {
      const int16_t excStart = fc->fareMarket()->travelSeg().front()->segmentOrder();
      const int16_t excEnd = fc->fareMarket()->travelSeg().back()->segmentOrder();
      for (const PaxTypeFare* ptf : _allRepricePTFs)
      {
        const int16_t newStart = ptf->fareMarket()->travelSeg().front()->segmentOrder();
        const int16_t newEnd = ptf->fareMarket()->travelSeg().back()->segmentOrder();
        if (excStart <= newEnd && excEnd >= newStart)
          _fmMapping[fc->fareCompNumber() - 1].push_back(ptf->fareMarket());
      }
    }
  }

  findFirstFare();
  findFareRetrievalFlag();
}

namespace
{
struct isFmInNewFp : public std::binary_function<const PaxTypeFare*, const FareMarket*, bool>
{
  bool operator()(const PaxTypeFare* curFc, const FareMarket* searchFm) const
  {
    return curFc->fareMarket() == searchFm;
  }
};

} // namespace

bool
RepriceSolutionValidator::checkTags(ProcessTagPermutation& permutation, std::string& errorMsg)
{
  if (hasDiagAndFilterPassed())
  {
    _dc->printLine();
    *_dc << "TAG DEFINITION VALIDATION\n";
  }

  if (!permutation.mixedTags() && permutation.processTags().front()->reissueSequence()->orig())
  {
    FareMarket::FareRetrievalFlags flag;

    ProcessTag tag = (ProcessTag)99;

    tag = (ProcessTag)permutation.processTags().front()->processTag();

    switch (tag)
    {
    case GUARANTEED_AIR_FARE: // 2
      flag = FareMarket::RetrievHistorical;
      break;
    case NO_GUARANTEED_FARES: // 5
      flag = FareMarket::RetrievCurrent;
      break;
    case TRAVEL_COMENCEMENT_AIR_FARES: // 6
      flag = _travelCommenced ? FareMarket::RetrievTvlCommence : FareMarket::RetrievCurrent;
      break;
    case REISSUE_DOWN_TO_LOWER_FARE: // 7
      flag = FareMarket::RetrievCurrent;
      break;
    default:
      flag = FareMarket::RetrievNone;
      break;
    }

    if (hasDiagAndFilterPassed())
    {
      *_dc << " PERMUTATION CONSISTS OF SAME TAGS: " << tag << "\n";
    }

    if (flag != FareMarket::RetrievNone)
    {
      if (hasDiagAndFilterPassed())
      {
        *_dc << " FARE RETRIEVAL FLAG: " << flag << "\n";
        *_dc << " REPRICE SOLUTION RETRIEVAL: " << _retrievalFlag << "\n";
      }

      return flag & _retrievalFlag;
    }
  }

  return checkTagsForMixedTags(permutation, errorMsg);
}

struct BoardOffIdenticalFareRetrievalAndFlownStatusSame
    : std::unary_function<const PaxTypeFare*, bool>
{
  const LocCode& _board;
  const LocCode& _off;
  FareMarket::FareRetrievalFlags _flag;
  const static FareMarket::FareRetrievalFlags _anyRetrievalFlag =
      static_cast<FareMarket::FareRetrievalFlags>(0xff);
  bool _flown;

  BoardOffIdenticalFareRetrievalAndFlownStatusSame(
      const PaxTypeFare* ptf, FareMarket::FareRetrievalFlags flag = _anyRetrievalFlag)
    : _board(ptf->fareMarket()->boardMultiCity()),
      _off(ptf->fareMarket()->offMultiCity()),
      _flag(flag),
      _flown(ptf->fareMarket()->isFlown())
  {
  }

  bool operator()(const PaxTypeFare* ptf)
  {
    bool rc(false);
    if (ptf->fareMarket()->boardMultiCity() == _board && ptf->fareMarket()->offMultiCity() == _off)
      if (ptf->retrievalFlag() & _flag && _flown == ptf->fareMarket()->isFlown())
        rc = true;
    return rc;
  }
};

bool
RepriceSolutionValidator::zeroT988ExcFare(const ProcessTagPermutation& permutation,
                                          const PaxTypeFare& ptf) const
{
  return !permutation.processTags()[ptf.fareMarket()->fareCompInfo()->fareCompNumber() - 1]
              ->reissueSequence()
              ->orig();
}

bool
RepriceSolutionValidator::determineNewKeepPTF(
    const PaxTypeFare& excFare, std::vector<const PaxTypeFare*>::const_iterator& newPTFIter)
{
  RexPricingTrx::NewItinKeepFareMap::iterator newFmIter =
      _trx.newItinKeepFares(_itinIndex).find(&excFare);

  if (newFmIter != _trx.newItinKeepFares(_itinIndex).end())
  {
    newPTFIter = find_if(_allRepricePTFs.begin(),
                         _allRepricePTFs.end(),
                         std::bind2nd(isFmInNewFp(), newFmIter->second));

    if (newPTFIter != _allRepricePTFs.end() &&
        ((**newPTFIter).retrievalFlag() & FareMarket::RetrievKeep))
    {
      if ((**newPTFIter).retrievalFlag() & FareMarket::RetrievExpndKeep)
        _expndKeepValidator.removeFromSearchScope(**newPTFIter);

      return true;
    }
  }

  const PaxTypeFare* newPtf = _expndKeepValidator.matchTagDefinition(excFare);
  if (newPtf)
  {
    newPTFIter = find(_allRepricePTFs.begin(), _allRepricePTFs.end(), newPtf);
    return true;
  }

  return false;
}

bool
RepriceSolutionValidator::checkTagsForMixedTags(ProcessTagPermutation& permutation,
                                                std::string& errorMsg)
{
  FareMarket::FareRetrievalFlags flag = FareMarket::RetrievNone;
  std::vector<bool> matchedNewFc(_allRepricePTFs.size(), false);
  std::vector<const PaxTypeFare*>::const_iterator newPTFIter;
  std::map<FareApplication, ProcessTagInfo*>::iterator iWinnerTag;
  bool checkWithoutKeep = false;

  _expndKeepValidator.setSearchScope();

  if (hasDiagAndFilterPassed())
    *_dc << "CHECK TAGS FOR MIXED RETRIEVAL\n";

  int i = 0;

  for (const ProcessTagPermutation::PaxTypeFareApplication::value_type& fareApp :
       permutation.fareApplMap())
  {
    ++i;

    const PaxTypeFare& excFare = *fareApp.first;
    FareApplication fa = fareApp.second;

    if (hasDiagAndFilterPassed())
    {
      *_dc << " EXC FC" << i << ":" << excFare.fareClass() << "\n";
      *_dc << "  FARE APPLCATION: " << fa << "\n";
    }

    bool zeroT988Fare = zeroT988ExcFare(permutation, excFare);

    if (_isRebookSolution && !zeroT988Fare)
    {
      fa = permutation.rebookFareTypeSelection(excFare.fareMarket()->changeStatus());
      if (hasDiagAndFilterPassed())
        *_dc << "  REBOOK FARE APPL:" << fa << "\n";
    }

    flag = decodeFareRetrievalFlag(fa);
    ProcessTag tag((ProcessTag)0); // invalid tag

    if ((iWinnerTag = permutation.fareApplWinnerTags().find(fa)) !=
        permutation.fareApplWinnerTags().end())
      tag = (ProcessTag)((*iWinnerTag).second)->processTag();

    if (hasDiagAndFilterPassed())
    {
      *_dc << "  FARE RETRIEVAL FLAG: " << flag << "\n";
      *_dc << "  WINNER PROCESS TAG FOR FARE APPLICATION: " << tag << "\n";
    }

    MixedTagsCacheKey key(&excFare, fa, tag);
    MixedTagsCache::iterator cache = _mixedTagsCache.find(key);
    if (cache != _mixedTagsCache.end() && (fa != KEEP || tag != 0))
    {
      MixedTagsCacheResult res = cache->second;
      if (!res._ret)
      {
        if (hasDiagAndFilterPassed())
          errorMsg = "PREVIOUS FAILED ON SAME FARE WITH SAME APPLICATION AND TAG\n";
        return false;
      }

      if ((fa == HISTORICAL && tag == HISTORICAL_FARES_FOR_TRAVELED_FC) || fa == KEEP)
      {
        matchedNewFc[res._newPTFIter - _allRepricePTFs.begin()] = true;
      }
      else if (!matchNewFcWithFa(matchedNewFc, flag))
      {
        if (hasDiagAndFilterPassed())
          errorMsg = "NO NEW FC MATCHED RETRIEVEL FLAG\n";
        return false;
      }
      continue;
    }

    if (fa == HISTORICAL && tag == HISTORICAL_FARES_FOR_TRAVELED_FC)
    {
      if (!excFare.fareMarket()->isFlown())
      {
        if (hasDiagAndFilterPassed())
          errorMsg = "HISTORICAL FARE APPL ON NOT FLOWN FARE MARKET\n";
        return false;
      }

      BoardOffIdenticalFareRetrievalAndFlownStatusSame status(&excFare, flag);
      if ((newPTFIter = std::find_if(_allRepricePTFs.begin(), _allRepricePTFs.end(), status)) ==
          _allRepricePTFs.end())
      {
        if (hasDiagAndFilterPassed())
          errorMsg = "NO FARE IN COMB WITH SUCH BOARD/OFF POINT/RETRIEVAL FLAG/FLOWN STAT\n";
        return false;
      }

      matchedNewFc[newPTFIter - _allRepricePTFs.begin()] = true;
    }
    else if (fa == KEEP)
    {
      if (!determineNewKeepPTF(excFare, newPTFIter))
      {
        if (hasDiagAndFilterPassed())
          errorMsg = "KEEP FARE NOT FOUND IN NEW FARE PATH\n";
        return false;
      }

      matchedNewFc[newPTFIter - _allRepricePTFs.begin()] = true;

      if (zeroT988Fare)
        continue;

      if (iWinnerTag == permutation.fareApplWinnerTags().end())
      {
        errorMsg = "NO WINNER TAG";
        return false;
      }

      if (tag == KEEP_FARES_FOR_TRAVELED_FC)
        checkWithoutKeep = true;
    }
    else if (!matchNewFcWithFa(matchedNewFc, flag))
    {
      if (hasDiagAndFilterPassed())
        errorMsg = "NO NEW FC MATCHED RETRIEVEL FLAG\n";
      return false;
    }
    else
    {
      newPTFIter = _allRepricePTFs.end();
    }

    _mixedTagsCache.insert(MixedTagsCache::value_type(key, MixedTagsCacheResult(newPTFIter, true)));
  }

  // We artificially match Current PTFs which aren't similar to any PTFs from the exchange itin,
  // because without
  // it it would be impossible to reprice an itinerary where the only change is adding new
  // segments.
  std::vector<bool>::iterator matchedNewFCsIt = matchedNewFc.begin();
  std::vector<const PaxTypeFare*>::iterator repricePTFsIt = _allRepricePTFs.begin();

  for (; matchedNewFCsIt != matchedNewFc.end(); ++matchedNewFCsIt, ++repricePTFsIt)
  {
    if (*matchedNewFCsIt || !((*repricePTFsIt)->retrievalFlag() & FareMarket::RetrievCurrent))
      continue;

    BoardOffIdenticalFareRetrievalAndFlownStatusSame status(*repricePTFsIt);
    const ProcessTagPermutation::PaxTypeFareApplication::const_iterator
    fareApplBegin = permutation.fareApplMap().begin(),
    fareApplEnd = permutation.fareApplMap().end();
    ProcessTagPermutation::PaxTypeFareApplication::const_iterator match = std::find_if(
        fareApplBegin,
        fareApplEnd,
        boost::bind(
            status,
            boost::bind(&ProcessTagPermutation::PaxTypeFareApplication::value_type::first, _1)));

    if (match == fareApplEnd)
    {
      *matchedNewFCsIt = true;
    }
  }

  std::vector<bool>::iterator failedNewFcIter =
      std::find(matchedNewFc.begin(), matchedNewFc.end(), false);
  if (failedNewFcIter != matchedNewFc.end())
  {
    if (hasDiagAndFilterPassed())
    {
      *_dc << " EXC FC" << (std::distance(matchedNewFc.begin(), failedNewFcIter) + 1)
           << " NO RETRIEVAL MATCHING FARE IN NEW FARE PATH\n";
      errorMsg = "NO EXC FC REQUIRED FARE IN NEW FARE PATH\n";
    }

    return false;
  }

  // set info about pt3 to skip bytes 24-31 validation
  if (checkWithoutKeep)
  {
    for (ProcessTagInfo* PT : permutation.processTags())
    {
      if (getFareApplication(permutation, *PT->paxTypeFare()) == KEEP)
        PT->isValidTag3() = true;
    }
  }

  return true;
}

bool
RepriceSolutionValidator::matchNewFcWithFa(std::vector<bool>& matchedNewFc,
                                           FareMarket::FareRetrievalFlags flag)
{
  if (std::find(matchedNewFc.begin(), matchedNewFc.end(), false) == matchedNewFc.end())
    return true;

  bool found = false;
  for (size_t i = 0, S = _allRepricePTFs.size(); i < S; ++i)
  {
    FareMarket::FareRetrievalFlags paxFlag = _allRepricePTFs[i]->retrievalFlag();
    if (paxFlag & flag)
    {
      matchedNewFc[i] = true;
      found = true;
    }
  }

  return found;
}

FareMarket::FareRetrievalFlags
RepriceSolutionValidator::decodeFareRetrievalFlag(FareApplication fa)
{
  switch (fa)
  {
  case HISTORICAL:
    return FareMarket::RetrievHistorical;
  case KEEP:
    return FareMarket::RetrievKeep;
  case TRAVEL_COMMENCEMENT:
    return FareMarket::RetrievTvlCommence;
  case CURRENT:
    return FareMarket::RetrievCurrent;
  default:
    return FareMarket::RetrievNone;
  }
}

bool
RepriceSolutionValidator::matchOutboundPortionOfTravel(const ProcessTagPermutation& permutation)
    const
{
  const bool diagAndIfFilteredFilterPassed(hasDiagAndFilterPassed());

  uint16_t fcNo = 0;
  for (const ProcessTagInfo* pt : permutation.processTags())
  {
    ++fcNo;

    if (!pt->reissueSequence()->orig())
      continue;

    if (!matchOutboundPortionOfTvl(*pt, fcNo, diagAndIfFilteredFilterPassed, _dc))
      return false;
  }
  return true;
}

bool
RepriceSolutionValidator::matchExpndKeepSeasonalityDOW(const ProcessTagPermutation& permutation)
{
  _expndKeepValidator.dc() = hasDiagAndFilterPassed() ? _dc : nullptr;
  return _expndKeepValidator.matchSeasonalityDOW(permutation);
}

bool
RepriceSolutionValidator::isFarePathValidForPermutation(
    ProcessTagPermutation& permutation, RexAdvResTktValidator& rexAdvResTktValidator)
{
  std::string errorMsg;
  bool result = checkTags(permutation, errorMsg);

  if (hasDiagAndFilterPassed())
  {
    uint32_t fc = 0;
    for (const ProcessTagInfo* pt : permutation.processTags())
    {
      ++fc;
      const ProcessTagInfo& pti(*pt);

      if (_isRebookSolution)
      {
        const FareMarket* fareMarket = pti.paxTypeFare()->fareMarket();
        FCChangeStatus changeStatus = fareMarket->changeStatus();
        FCChangeStatus asBookChangeStatus = fareMarket->asBookChangeStatus();
        if (asBookChangeStatus != changeStatus)
        {
          *_dc << "FC" << fc << " STATUS CHANGED TO ";
          switch (changeStatus)
          {
          case UU:
            *_dc << "UU\n";
            break;
          case UN:
            *_dc << "UN\n";
            break;
          case FL:
            *_dc << "FL\n";
            break;
          case UC:
            *_dc << "UC\n";
            break;
          default:
            *_dc << "-\n";
          }
        }
      }
      const FarePath& excFp = *_trx.exchangeItin().front()->farePath().front();
      _dc->printPermutationInfo(pti, result, excFp);
    }
    if (!result)
    {
      *_dc << "TAGS CHECK: ";

      if (errorMsg.empty())
        *_dc << "FAILED\n";
      else
        *_dc << errorMsg;
    }
  }

  if (result)
    result = _stopoverConnectionValidator.match(permutation);

  if (result)
  {
    result =
        (matchExpndKeepSeasonalityDOW(permutation) && matchOutboundPortionOfTravel(permutation));
  }

  if (result)
  {
    bool permWithTag7 = permutation.hasTag7only();
    result =
        (checkFareBreakLimitations(permutation, permWithTag7) &&
         _fareBytesValidator.validate(permutation) && revalidateRulesForKeepFares(permutation) &&
         validateBkgRevalInd(permutation) && rexAdvResTktValidator.validate(permutation) &&
         checkOverrideReservationDates(permutation) &&
         _newTicketEqualOrHigherValidator.match(permutation));

    if (result)
      result = savePermutationChangeFee(permutation, permWithTag7);
  }

  if (hasDiagAndFilterPassed())
  {
    _dc->printPermutationValidationResult(permutation, result ? "PASSED" : "FAILED");
  }

  return result;
}

bool
RepriceSolutionValidator::checkOWRT(const std::vector<ProcessTagInfo*>& processTags,
                                    std::string& errorMsg)
{
  for (const ProcessTagInfo* pt : processTags)
  {
    Indicator owrtByte = pt->reissueSequence()->owrt();

    if (owrtByte != OWRT_ALL_FARES_AVAILABLE)
    {
      for (const PricingUnit* pu : _farePath.pricingUnit())
      {
        for (FareUsage* fu : pu->fareUsage())
        {
          if ((fu->paxTypeFare()->owrt() == ONE_WAY_MAY_BE_DOUBLED ||
               fu->paxTypeFare()->owrt() == ONE_WAY_MAYNOT_BE_DOUBLED) &&
              owrtByte == OWRT_ONLY_RT_FARES)
          {
            errorMsg = "RT FARES ONLY";
            return false;
          }

          if (fu->paxTypeFare()->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED &&
              owrtByte == OWRT_ONLY_OW_FARES)
          {
            errorMsg = "OW FARES ONLY";
            return false;
          }
        }
      }
    }
  }
  return true;
}

bool
RepriceSolutionValidator::savePermutationChangeFee(ProcessTagPermutation& permutation,
                                                   bool permWithTag7)
{
  MoneyAmount changeFee = 0.0;
  ReissueCharges* reissueCharge = calculateReissueCharges(permutation);

  if (!_farePath.ignoreReissueCharges() || _dc)
  {
    // how we can get changeFee we just computed and not stored???
    // below line will always set changeFee to 0.0
    changeFee = getTotalChangeFee(permutation);
  }

  bool status(true);
  if (permWithTag7)
    status =
        isBaseFareAmountPlusChangeFeeHigher(changeFee) ? false : checkReissueToLower(permutation);

  if (status)
  {
    _permutationReissueCharges[&permutation] = reissueCharge;
  }
  return status;
}

ReissueCharges*
RepriceSolutionValidator::calculateReissueCharges(ProcessTagPermutation& permutation)
{
  if (_farePath.ignoreReissueCharges())
  {
    if (hasDiagAndFilterPassed())
      *_dc << "PENALTY FEE: NOT ASSESSED OPEN/UNCHGED\n";

    return nullptr;
  }

  const RexPricingRequest& request = static_cast<const RexPricingRequest&>(*_trx.getRequest());
  const CarrierCode& validatingCarrier = request.newValidatingCarrier();

  ReissuePenaltyCalculator calculator;
  calculator.initialize(_trx,
                        _calcCurrCode,
                        *_farePathChangeDetermination,
                        validatingCarrier,
                        _trx.getExchangePaxType().paxTypeInfo(),
                        &permutation,
                        _dc);

  return calculator.process();
}

bool
RepriceSolutionValidator::checkOverrideReservationDates(const ProcessTagPermutation& permutation)
{
  for (const ProcessTagInfo* pt : permutation.processTags())
  {
    if (pt->record3()->overrideDateTblItemNo())
    {
      const uint16_t fareCompNumber = pt->fareCompNumber();
      LOG4CXX_DEBUG(logger, "fareCompNumber |" << fareCompNumber << "|");

      const uint32_t rec3ItemNo = pt->record3()->itemNo();
      LOG4CXX_DEBUG(logger, "rec3 itemNo |" << rec3ItemNo << "|");

      LOG4CXX_DEBUG(logger, "cache size |" << _record3Cache.size() << "|");

      const std::pair<int, int> key = std::make_pair(fareCompNumber, rec3ItemNo);
      std::map<std::pair<int, int>, bool>::const_iterator rec3CacheI = _record3Cache.find(key);
      std::map<std::pair<int, int>, bool>::const_iterator rec3CacheIend = _record3Cache.end();

      if (rec3CacheI != rec3CacheIend)
      {
        if (rec3CacheI->second)
          continue;
        else
          return false;
      }
      else
      {
        LOG4CXX_DEBUG(logger, "record 3 not in cache, validating...");
        const VoluntaryChangesInfoW& vcRec3 = (*pt->record3());
        bool dateValidationResult;

        if (_fcMapping[fareCompNumber - 1].empty())
          dateValidationResult = journeyBookingDateValidation(vcRec3);

        else
          dateValidationResult =
              fareComponentBookingDateValidation(_fcMapping[fareCompNumber - 1], vcRec3);

        _record3Cache.insert(std::make_pair(key, dateValidationResult));
        return dateValidationResult;
      }
    }
  }
  return true;
}

bool
RepriceSolutionValidator::journeyBookingDateValidation(const VoluntaryChangesInfoW& vcRec3)
{
  LOG4CXX_DEBUG(logger, "no mapping, journey booking date validation");

  if (!_journeyLatestBD)
    _journeyLatestBD = latestBookingDate(_farePath.pricingUnit());

  LOG4CXX_DEBUG(logger, "last booking date of JOURNEY set to |" << (*_journeyLatestBD) << "|");

  return validateLastBookingDate(_journeyLatestBD, vcRec3);
}

bool
RepriceSolutionValidator::fareComponentBookingDateValidation(
    std::vector<const PaxTypeFare*>& mappedFCvec, const VoluntaryChangesInfoW& vcRec3)
{
  LOG4CXX_DEBUG(logger, "mapped to |" << mappedFCvec.size() << "| pax type fare/s");

  for (const PaxTypeFare* ptf : mappedFCvec)
  {
    LOG4CXX_DEBUG(logger, "      validating |" << ptf << "|");

    const DateTime* latestBD = latestBookingDate(ptf->fareMarket()->travelSeg());

    LOG4CXX_DEBUG(logger, "last booking date of FC set to |" << (*latestBD) << "|");

    if (!validateLastBookingDate(latestBD, vcRec3))
      return false;
  }
  return true;
}

const std::vector<DateOverrideRuleItem*>&
RepriceSolutionValidator::getDateOverrideRuleItem(const VendorCode& vendor, int itemNo)
{
  return _trx.dataHandle().getDateOverrideRuleItem(vendor, itemNo);
}

bool
RepriceSolutionValidator::validateLastBookingDate(const DateTime* latestBD,
                                                  const VoluntaryChangesInfoW& vcRec3)
{
  const std::vector<DateOverrideRuleItem*>& dorItemVec =
      getDateOverrideRuleItem(vcRec3.vendor(), vcRec3.overrideDateTblItemNo());

  for (const DateOverrideRuleItem* dori : dorItemVec)
  {
    LOG4CXX_DEBUG(logger, "resEffDate |" << dori->resEffDate() << "| <=");
    LOG4CXX_DEBUG(logger, "latestBookingDate |" << (*latestBD) << "| >=");
    LOG4CXX_DEBUG(logger, "resDiscDate |" << dori->resDiscDate() << "|");

    if ((*latestBD) < dori->resEffDate() || (*latestBD) > dori->resDiscDate())
    {
      if (hasDiagAndFilterPassed())
      {
        *_dc << "NEW ITIN LATEST BOOKING DATE: " << (*latestBD).toIsoExtendedString() << std::endl;
        *_dc << "  RES EFF DTE - " << dori->resEffDate().toIsoExtendedString();
        *_dc << "    RES DISC DTE - " << dori->resDiscDate().toIsoExtendedString() << std::endl;
        *_dc << "  FAILED ITEM " << vcRec3.itemNo() << " - OVERRIDE DATES NOT MET" << std::endl;
      }
      return false;
    }
  }
  return true;
}

const DateTime*
RepriceSolutionValidator::latestBookingDate(const std::vector<TravelSeg*>& travelSegVec) const
{
  const DateTime* latestBD = &DateTime::emptyDate();

  std::vector<TravelSeg*>::const_iterator latestSegIter =
      max_element(travelSegVec.begin(), travelSegVec.end(), CompareBookingDate());
  if (latestSegIter != travelSegVec.end())
    latestBD = &(*latestSegIter)->bookingDT();
  return latestBD;
}

const DateTime*
RepriceSolutionValidator::latestBookingDate(const std::vector<PricingUnit*>& pricingUnitVec) const
{
  const DateTime* journeyLatestBD = latestBookingDate(pricingUnitVec.front()->travelSeg());

  std::vector<PricingUnit*>::const_iterator puIter = pricingUnitVec.begin();
  ++puIter;

  LOG4CXX_DEBUG(logger, "BD in front PU  = |" << (*journeyLatestBD) << "|");

  for (; puIter != pricingUnitVec.end(); ++puIter)
  {
    const DateTime* currentPuLatestBD = latestBookingDate((*puIter)->travelSeg());

    LOG4CXX_DEBUG(logger, "BD in PU |" << (*currentPuLatestBD) << "|");

    if ((*currentPuLatestBD) > (*journeyLatestBD))
    {
      journeyLatestBD = currentPuLatestBD;
    }
  }

  return journeyLatestBD;
}

void
RepriceSolutionValidator::diagFullyFlown(const ProcessTagInfo& pti,
                                         const Indicator& matched,
                                         const FareMarket* failedOnMarket,
                                         int fcNumber)
{
  *_dc << "FULLY FLOWN CHECK: FAILED(" << fcNumber << ")" << std::endl;
  *_dc << "  EXCHANGE FC: " << pti.fareMarket()->travelSeg().front()->boardMultiCity() << "-"
       << pti.fareMarket()->travelSeg().back()->offMultiCity() << "\n";
  if (failedOnMarket)
  {
    LocCode dest;
    std::vector<TravelSeg*>::const_iterator fmIter = failedOnMarket->travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator fmIterEnd = failedOnMarket->travelSeg().end();
    *_dc << "  FAILED ON NEW FC: ";
    if (fmIter != fmIterEnd)
    {
      dest = (*fmIter)->offMultiCity();
      *_dc << (*fmIter)->boardMultiCity() << "-" << dest;
      ++fmIter;
    }
    for (; fmIter != fmIterEnd; ++fmIter)
    {
      const TravelSeg* seg = *fmIter;
      if (seg->boardMultiCity() != dest)
        *_dc << "//-" << seg->boardMultiCity();
      dest = seg->offMultiCity();
      *_dc << "-" << dest;
    }
    *_dc << "\n";
  }
  switch (matched)
  {
  case EXTEND_NO_CHANGE:
    *_dc << "  FC MAY NOT BE RE-PRICED A FUTHER POINT\n";
    break;
  case EXTEND_AT_LEAST_ONE_POINT:
    *_dc << "  FC MAY EXTEND AS LONG AS ONE TERMINAL POINT ON EXCHANGE FC\n"
         << "  IS USED AS TICKETING POINT ON THE NEW FC\n";
    break;
  case EXTEND_BEYOND_DEST_ONLY:
    *_dc << "  FC MAY ONLY EXTEND BEYOND DEST TERMINAL POINT OF EXCHANGE FC\n";
    break;
  }
}

bool
RepriceSolutionValidator::checkFullyFlown(const ProcessTagPermutation& permutation)
{
  const FareMarket* failedOnMarket = nullptr;
  std::vector<ProcessTagInfo*>::const_iterator ptiIter = permutation.processTags().begin();
  std::vector<ProcessTagInfo*>::const_iterator ptiIterEnd = permutation.processTags().end();

  Indicator matched = EXTEND_NO_RESTRICTIONS;
  for (; ptiIter != ptiIterEnd; ++ptiIter)
  {
    ProcessTagInfo* pti = *ptiIter;

    if (!pti->fareMarket()->isFlown())
      break;

    if (!pti->reissueSequence()->orig() ||
        pti->reissueSequence()->extendInd() == EXTEND_NO_RESTRICTIONS || (*ptiIter)->isValidTag3())
    {
      continue;
    }
    else
    {
      matched = validateFullyFlownForFC(pti, failedOnMarket);
      if (matched != EXTEND_NO_RESTRICTIONS)
        break;
    }
  }

  if (matched != EXTEND_NO_RESTRICTIONS && hasDiagAndFilterPassed())
    diagFullyFlown(
        **ptiIter, matched, failedOnMarket, (ptiIter - permutation.processTags().begin()) + 1);

  return (matched == EXTEND_NO_RESTRICTIONS);
}

Indicator
RepriceSolutionValidator::validateFullyFlownForFC(ProcessTagInfo* pti,
                                                  const FareMarket*& failedOnMarket)
{
  Indicator matched = EXTEND_NO_RESTRICTIONS;
  const Indicator& extendInd = pti->reissueSequence()->extendInd();

  FareCompInfo::RepriceFullyFlownValidationResultCache& cache =
      pti->fareCompInfo()->repriceFFValidationCache(_itinIndex);
  for (const FareMarket* fm : _fmMapping[pti->fareCompNumber() - 1])
  {
    FareCompInfo::RepriceFullyFlownValidationResultCacheKey key(
        pti->reissueSequence()->overridingWhenExists(), fm);
    FareCompInfo::RepriceFullyFlownValidationResultCache::iterator foundCacheItem = cache.find(key);

    if (foundCacheItem != cache.end())
    {
      matched = foundCacheItem->second;
    }
    else
    {
      switch (extendInd)
      {
      case EXTEND_AT_LEAST_ONE_POINT:
      {
        if (!checkFullyFlownOnePoint(*pti, *fm))
          matched = EXTEND_AT_LEAST_ONE_POINT;
      }
      break;
      case EXTEND_BEYOND_DEST_ONLY:
      {
        if (!checkFullyFlownDestOnly(*pti, *fm))
          matched = EXTEND_BEYOND_DEST_ONLY;
      }
      break;
      case EXTEND_NO_CHANGE:
      {
        if (!checkFullyFlownNoChange(*pti, *fm))
          matched = EXTEND_NO_CHANGE;
      }
      break;
      default:
        matched = EXTEND_NO_CHANGE;
        break;
      }
      cache.insert(make_pair(key, matched));
    }
    if (matched != EXTEND_NO_RESTRICTIONS)
    {
      failedOnMarket = fm;
      return matched;
    }
  }
  return matched;
}

bool
RepriceSolutionValidator::checkFullyFlownOnePoint(const ProcessTagInfo& pti,
                                                  const FareMarket& fareMarket)
{
  const LocCode& orig = pti.fareMarket()->travelSeg().front()->boardMultiCity();
  const LocCode& dest = pti.fareMarket()->travelSeg().back()->offMultiCity();

  for (const TravelSeg* ts : fareMarket.travelSeg())
  {
    if (ts->boardMultiCity() == orig || ts->offMultiCity() == dest)
      return true;
  }

  return false;
}

bool
RepriceSolutionValidator::checkFullyFlownDestOnly(const ProcessTagInfo& pti,
                                                  const FareMarket& fareMarket)
{
  return (pti.fareMarket()->travelSeg().front()->segmentOrder() <=
          fareMarket.travelSeg().front()->segmentOrder());
}

bool
RepriceSolutionValidator::checkFullyFlownNoChange(const ProcessTagInfo& pti,
                                                  const FareMarket& fareMarket)
{
  const std::vector<TravelSeg*>& travelSeg = pti.fareMarket()->travelSeg();
  return (travelSeg.front()->segmentOrder() == fareMarket.travelSeg().front()->segmentOrder() &&
          travelSeg.back()->segmentOrder() == fareMarket.travelSeg().back()->segmentOrder());
}

bool
RepriceSolutionValidator::checkFareBreakLimitations(const ProcessTagPermutation& permutation,
                                                    bool& permWithTag7)
{
  if (hasDiagAndFilterPassed())
    *_dc << "VALIDATING FARE BREAK LIMITATIONS\n";
  if (permWithTag7)
  {
    return checkTerm(permutation) && matchJourney(permutation);
  }

  return checkTerm(permutation) && checkFirstBreak(permutation) && checkFullyFlown(permutation) &&
         matchJourney(permutation);
}

bool
RepriceSolutionValidator::checkTerm(const ProcessTagPermutation& permutation)
{
  bool valid = true;

  std::vector<std::vector<const PaxTypeFare*>>::const_iterator iterFC = _fcMapping.begin();

  const std::vector<ProcessTagInfo*>& processTags = permutation.processTags();
  std::vector<ProcessTagInfo*>::const_iterator iterPT = processTags.begin();

  while (iterPT != processTags.end())
  {
    const ReissueSequenceW* seq = (*iterPT)->reissueSequence();
    if (seq->orig() && !(*iterPT)->isValidTag3() && BLANK == seq->terminalPointInd() &&
        (*iterFC).empty() && !checkSamePointTable(**iterPT))
    {
      valid = false;
      break;
    }
    ++iterPT;
    ++iterFC;
  }

  if (!valid && hasDiagAndFilterPassed())
  {
    *_dc << "TERM CHECK: FAILED FC" << (int)(iterFC - _fcMapping.begin()) + 1 << "\n";
  }
  return valid;
}

namespace
{
struct findFirstBreakRestriction : std::unary_function<const ProcessTagInfo*, bool>
{
  bool operator()(const ProcessTagInfo* tag) const
  {
    if (!tag->reissueSequence()->orig() || tag->isValidTag3())
      return false;

    static const Indicator SAME_FARE = 'Y';

    const ReissueSequenceW* seq = tag->reissueSequence();
    return SAME_FARE == seq->firstBreakInd();
  }
};
} // namespace

bool
RepriceSolutionValidator::checkFirstBreak(const ProcessTagPermutation& permutation)
{
  const std::vector<ProcessTagInfo*>& processTags = permutation.processTags();

  if (find_if(processTags.begin(), processTags.end(), findFirstBreakRestriction()) ==
      processTags.end())
  {
    return true;
  }

  if (!_fcMapping.front().empty() || checkSamePointTable(*(processTags.front())))
  {
    const PaxTypeFare* previousFirstFare = processTags.front()->paxTypeFare();
    const FareCompInfo* fareCompInfo = processTags.front()->fareCompInfo();
    const VCTR& vctr = fareCompInfo->VCTR();

    if ((SameFareBreaks(previousFirstFare)(_firstFare) ||
         checkSamePointTable(*(processTags.front()), true)) &&
        (_firstFare->createFareBasis(_trx) == fareCompInfo->fareBasisCode()) &&
        (std::fabs(_firstFare->fareAmount() - previousFirstFare->fareAmount()) < EPSILON) &&
        (!fareCompInfo->hasVCTR() ||
         (vctr.vendor() == _firstFare->vendor() && vctr.carrier() == _firstFare->carrier() &&
          vctr.tariff() == _firstFare->fareTariff() && vctr.rule() == _firstFare->ruleNumber())))
    {
      return true;
    }
  }
  else
  {
    return true;
  }

  if (hasDiagAndFilterPassed())
  {
    *_dc << "FIRST BREAK CHECK: FAILED" << std::endl;
  }
  return false;
}

namespace
{
class MatchSamePoints
{
public:
  MatchSamePoints() {}

  const LocCode& newBoardMultiCity() const { return _boardMultiCity; }
  const LocCode& newOffMultiCity() const { return _offMultiCity; }

  bool matchSamePoint(const SamePoint* sp, const FareMarket* oldFM)
  {
    if (oldFM->boardMultiCity() == sp->mkt1())
    {
      _boardMultiCity = sp->mkt2();
      _offMultiCity = oldFM->offMultiCity();
      return true;
    }
    else if (oldFM->boardMultiCity() == sp->mkt2())
    {
      _boardMultiCity = sp->mkt1();
      _offMultiCity = oldFM->offMultiCity();
      return true;
    }
    else if (oldFM->offMultiCity() == sp->mkt1())
    {
      _boardMultiCity = oldFM->boardMultiCity();
      _offMultiCity = sp->mkt2();
      return true;
    }
    else if (oldFM->offMultiCity() == sp->mkt2())
    {
      _boardMultiCity = oldFM->boardMultiCity();
      _offMultiCity = sp->mkt1();
      return true;
    }

    return false;
  }

private:
  LocCode _boardMultiCity;
  LocCode _offMultiCity;
};

} // namespace

const std::vector<const SamePoint*>&
RepriceSolutionValidator::getSamePoint(const VendorCode& vendor, int itemNo, const DateTime& date)
{
  return _trx.dataHandle().getSamePoint(vendor, itemNo, date);
}

bool
RepriceSolutionValidator::checkSamePointTable(const ProcessTagInfo& tag, bool runForFirstBreak)
{
  int samePointItemNo = tag.reissueSequence()->samePointTblItemNo();
  if (!samePointItemNo)
    return false;

  int fcNo = tag.fareCompInfo()->fareCompNumber();

  FareBreakMapKey key = std::make_pair(fcNo, samePointItemNo);
  FareBreakMap* fbMapping;

  if (runForFirstBreak)
    fbMapping = &_fbMappingFB;
  else
    fbMapping = &_fbMappingTB;

  FareBreakMap::const_iterator iter = fbMapping->find(key);
  if (iter != fbMapping->end())
    return iter->second;

  MatchSamePoints mSP;
  const std::vector<const SamePoint*>& spV =
      getSamePoint(tag.paxTypeFare()->vendor(),
                   samePointItemNo,
                   _trx.originalTktIssueDT()); // should be this data PaxTypeFare::retrievalDate()

  bool result = false;

  for (const SamePoint* sp : spV)
  {
    if (mSP.matchSamePoint(sp, tag.paxTypeFare()->fareMarket()))
    {
      std::vector<const PaxTypeFare*>::iterator foundFC = _allRepricePTFs.begin();

      if (runForFirstBreak)
      {
        if (SameFareBreaks(mSP.newBoardMultiCity(), mSP.newOffMultiCity())(_firstFare))
        {
          result = true;
          break;
        }
      }
      else if (std::find_if(foundFC,
                            _allRepricePTFs.end(),
                            SameFareBreaks(mSP.newBoardMultiCity(), mSP.newOffMultiCity())) !=
               _allRepricePTFs.end())
      {
        result = true;
        break;
      }
    }
  }

  (*fbMapping)[key] = result;

  return result;
}

bool
RepriceSolutionValidator::matchJourney(const ProcessTagPermutation& permutation)
{
  if (!_journeyCheckingFlag)
    return true;

  for (const ProcessTagInfo* tag : permutation.processTags())
  {
    if (tag->reissueSequence()->orig() && !tag->isValidTag3() &&
        tag->reissueSequence()->journeyInd() == 'X')
    {
      if (hasDiagAndFilterPassed())
      {
        *_dc << "JOURNEY CHECK: FAILED" << std::endl;
      }
      return false;
    }
  }
  return true;
}

void
RepriceSolutionValidator::analyseExcFarePath()
{
  ExcItin* itin = _trx.exchangeItin().front();
  if (itin == nullptr || itin->farePath().empty())
    return;

  FarePath* farePath = itin->farePath().front();

  if (!farePath->flownOWFaresCollected())
  {
    collectFlownOWFares(farePath->pricingUnit(), farePath->flownOWFares());
    farePath->flownOWFaresCollected() = true;
  }
}

void
RepriceSolutionValidator::collectFlownOWFares(const std::vector<PricingUnit*>& puVec,
                                              std::vector<FareUsage*>& collectedFU)
{
  for (const PricingUnit* pu : puVec)
  {
    for (FareUsage* fu : pu->fareUsage())
    {
      LOG4CXX_DEBUG(logger, "fare tag: |" << fu->paxTypeFare()->owrt() << "|");
      LOG4CXX_DEBUG(logger, "flown fare : |" << fu->paxTypeFare()->fareMarket()->isFlown() << "|");

      if (fu->paxTypeFare()->fareMarket()->isFlown() &&
          (fu->paxTypeFare()->owrt() == ONE_WAY_MAY_BE_DOUBLED ||
           fu->paxTypeFare()->owrt() == ONE_WAY_MAYNOT_BE_DOUBLED))
      {
        LOG4CXX_DEBUG(logger, "flown fare from exc collected");
        collectedFU.push_back(fu);
      }
    }
  }
}

void
RepriceSolutionValidator::applyChangeFeeToFarePath()
{
  _farePath.setLowestFee31Perm(_permWithLowestChangeFee);
  _farePath.reissueCharges() = _permutationReissueCharges[_permWithLowestChangeFee];

  if (hasDiagAndFilterPassed())
    *_dc << "LOWEST CHANGE FEE:  PERMUTATION " << _permWithLowestChangeFee->number() << std::endl;
  _farePath.setRexChangeFee(_lowestChangeFee);
}

bool
RepriceSolutionValidator::checkReissueToLower(const ProcessTagPermutation& permutation)
{
  Combinations combinationsController;
  RuleControllerWithChancelor<PricingUnitRuleController> puRuleController;
  ReissueToLowerValidator val(
      _trx, _farePath, combinationsController, puRuleController, _FM2PrevReissueFMCache);

  bool result = val.process(permutation);

  if (hasDiagAndFilterPassed())
    *_dc << "REISSUE TO LOWER: " << (result ? "PASSED\n" : "FAILED\n");

  return result;
}

bool
RepriceSolutionValidator::checkNewTicketEqualOrHigher(const ProcessTagPermutation& permutation)
{
  bool result = false;
  Indicator ind = permutation.checkTable988Byte156();
  switch (ind)
  {
  case ProcessTagPermutation::NEW_TICKET_EQUAL_OR_HIGHER_BLANK:
    result = true;
    break;
  case ProcessTagPermutation::NEW_TICKET_EQUAL_OR_HIGHER_B:
    result = compareTotalFareCalculationAmounts();
    break;
  case ProcessTagPermutation::NEW_TICKET_EQUAL_OR_HIGHER_N:
    result = compareSumOfNonrefundableAmountOfExchangeTicketAndNewTicket(permutation);
    break;
  case ProcessTagPermutation::NEW_TICKET_EQUAL_OR_HIGHER_BN:
    result = (compareSumOfNonrefundableAmountOfExchangeTicketAndNewTicket(permutation) &&
              compareTotalFareCalculationAmounts());
    break;
  }

  if (hasDiagAndFilterPassed())
  {
    if (_trx.isPlusUpCalculationNeeded())
    {
      getMinFarePlusUps();

      NonRefundableUtil nru(_trx);

      // below diag output - move to 689
      const CurrencyCode& currency = getCurrency();

      *_dc << "NONREFUNDABLE AMOUNT FOR EXCHANGE TICKET: NUC" << nru.excNonRefundableNucAmt()
           << "\n";
      *_dc << "NONREFUNDABLE AMOUNT FOR NEW TICKET: NUC"
           << _farePath.getNonrefundableAmountInNUC(_trx) << "\n";

      Money excNonRefAmt(nru.excNonRefundableBaseCurrAmt(), currency),
          newNonRefAmt(getNonRefAmountInBaseCurrencyOfNewTicket(), currency);

      *_dc << "NONREFUNDABLE AMOUNT FOR EXCHANGE TICKET: "
           << excNonRefAmt.toString(_trx.originalTktIssueDT()) << "\n";
      *_dc << "NONREFUNDABLE AMOUNT FOR NEW TICKET: " << newNonRefAmt.toString() << "\n";

      *_dc << "TOTAL AMOUNT FOR EXCHANGE TICKET: NUC" << nru.excTotalNucAmt() << "\n";
      *_dc << "TOTAL AMOUNT FOR NEW TICKET: NUC" << _farePath.getTotalNUCAmount() << "\n";

      Money excTotalAmt(nru.excTotalBaseCurrAmt(), currency),
          newTotalAmt(getNewTicketAmount(), currency);

      *_dc << "TOTAL AMOUNT FOR EXCHANGE TICKET: "
           << excTotalAmt.toString(_trx.originalTktIssueDT()) << "\n";
      *_dc << "TOTAL AMOUNT FOR NEW TICKET: " << newTotalAmt.toString() << "\n";
    }

    if (ind == ProcessTagPermutation::NEW_TICKET_EQUAL_OR_HIGHER_BN)
      *_dc << "NEW TICKET EQUAL OR HIGHER: BN\n";
    else
      *_dc << "NEW TICKET EQUAL OR HIGHER: " << ind << "\n";
    *_dc << "NEW TICKET EQUAL OR HIGHER: " << (result ? "PASSED\n" : "FAILED\n");
  }

  return result;
}

bool
RepriceSolutionValidator::compareTotalFareCalculationAmounts()
{
  getMinFarePlusUps();
  NonRefundableUtil nru(_trx);

  const MoneyAmount& valueOfNewTicket = getNewTicketAmount();
  const MoneyAmount& valueOfExchangeTicket =
      (_trx.isPlusUpCalculationNeeded() ? nru.excTotalBaseCurrAmt() : getExchTotalAmount());

  const CurrencyCode& currency = getCurrency();
  if (hasDiagAndFilterPassed())
  {
    if (!_trx.isPlusUpCalculationNeeded())
    {
      *_dc << "TOTAL AMOUNT FOR EXCHANGE TICKET: " << valueOfExchangeTicket << currency << "\n";
      *_dc << "TOTAL AMOUNT FOR NEW TICKET: " << valueOfNewTicket << currency << "\n";
    }
  }

  if (_byte156ValueB == NEW_TICKET_EQUAL_OR_HIGHER_PASS)
    return true;
  if (_byte156ValueB == NEW_TICKET_EQUAL_OR_HIGHER_FAIL)
    return false;

  if (valueOfExchangeTicket - valueOfNewTicket > EPSILON)
  {
    const MoneyAmount& nucValueOfExchangeTicket =
        _trx.isPlusUpCalculationNeeded() ? nru.excTotalNucAmt() : getExchNUCAmount();

    if (nucValueOfExchangeTicket - _farePath.getTotalNUCAmount() > EPSILON)
    {
      _byte156ValueB = NEW_TICKET_EQUAL_OR_HIGHER_FAIL;
      return false;
    }
  }
  _byte156ValueB = NEW_TICKET_EQUAL_OR_HIGHER_PASS;
  return true;
}

bool
RepriceSolutionValidator::compareSumOfNonrefundableAmountOfExchangeTicketAndNewTicket(
    const ProcessTagPermutation& perm)
{
  if (_byte156ValueN == NEW_TICKET_EQUAL_OR_HIGHER_PASS ||
      (perm.isNMostRestrictiveResidualPenaltyInd()))
    return true;
  if (_byte156ValueN == NEW_TICKET_EQUAL_OR_HIGHER_FAIL)
    return false;

  MoneyAmount sumOfNonrefundableAmountsOfExchangeTicket;
  if (_trx.isPlusUpCalculationNeeded())
  {
    getMinFarePlusUps();
    NonRefundableUtil nru(_trx);
    sumOfNonrefundableAmountsOfExchangeTicket = nru.excNonRefundableBaseCurrAmt();
  }
  else
  {
    sumOfNonrefundableAmountsOfExchangeTicket = getNonRefAmountInBaseCurrencyOfExchangeTicket();
  }

  if (!(sumOfNonrefundableAmountsOfExchangeTicket > EPSILON))
  {
    _byte156ValueN = NEW_TICKET_EQUAL_OR_HIGHER_PASS;
    return true;
  }

  MoneyAmount sumOfNonrefundableAmountsOfNewTicket = getNonRefAmountInBaseCurrencyOfNewTicket();
  const CurrencyCode& currency = getCurrency();
  if (hasDiagAndFilterPassed())
  {
    if (!_trx.isPlusUpCalculationNeeded())
    {
      *_dc << "NONREFUNDABLE AMOUNT FOR EXCHANGE TICKET: "
           << sumOfNonrefundableAmountsOfExchangeTicket << currency << "\n";
      *_dc << "NONREFUNDABLE AMOUNT FOR NEW TICKET: " << sumOfNonrefundableAmountsOfNewTicket
           << currency << "\n";
    }
  }

  if (sumOfNonrefundableAmountsOfExchangeTicket - sumOfNonrefundableAmountsOfNewTicket > EPSILON)
  {
    _byte156ValueN = NEW_TICKET_EQUAL_OR_HIGHER_FAIL;
    return false;
  }
  _byte156ValueN = NEW_TICKET_EQUAL_OR_HIGHER_PASS;
  return true;
}

bool
RepriceSolutionValidator::validateBkgRevalInd(const ProcessTagPermutation& permutation)
{
  if (hasDiagAndFilterPassed())
    *_dc << "VALIDATING BOOKING CODE REVALIDATE IND \n";

  if (!_farePath.bookingCodeFailButSoftPassForKeepFare() && _isRebookSolution)
  {
    if (hasDiagAndFilterPassed())
      *_dc << "NO KEEP FARES FAILED BKG: PASS \n";

    return true;
  }

  if (!farePathHasKeepFares())
  {
    if (hasDiagAndFilterPassed())
      *_dc << "NO KEEP FARES: PASS \n";
    return true;
  }

  if (!permutation.hasZeroT988() && !permutation.needKeepFare() &&
      _farePath.bookingCodeFailButSoftPassForKeepFare())
  {
    if (hasDiagAndFilterPassed())
      *_dc << "PERM NEED NO KEEP FARES BUT FARES FAILED BKG: FAIL \n";
    return false; // Permuation does not need Keep Fares, but there is rule failed
  }
  std::map<const PaxTypeFare*, ProcessTagInfo*> keepFareProcessTags;
  getKeepFareProcessTags(permutation, keepFareProcessTags);
  return validateFarePathBkgRevalInd(keepFareProcessTags);
}

bool
RepriceSolutionValidator::farePathHasKeepFares()
{
  for (const PricingUnit* pu : _farePath.pricingUnit())
  {
    if (pu->hasKeepFare())
      return true;
  }
  return false;
}

bool
RepriceSolutionValidator::validateFarePathBkgRevalInd(
    const std::map<const PaxTypeFare*, ProcessTagInfo*>& keepFareProcessTags)
{
  for (const PricingUnit* pu : _farePath.pricingUnit())
  {
    if (!pu->hasKeepFare())
      continue;

    for (FareUsage* pfu : pu->fareUsage())
    {
      FareUsage& fu = *pfu;
      if (!fu.isKeepFare() || !fu.bookingCodeStatus().isSet(PaxTypeFare::BKS_FAIL_CAT31_KEEP_FARE))
        continue;
      if (!validateFareUsageBkgRevalInd(fu, keepFareProcessTags))
        return false;
    }
  }
  return true;
}

bool
RepriceSolutionValidator::validateFareUsageBkgRevalInd(
    FareUsage& fu, const std::map<const PaxTypeFare*, ProcessTagInfo*>& keepFareProcessTags)
{
  PaxTypeFare* keepFare = fu.paxTypeFare();
  if (hasDiagAndFilterPassed())
    *_dc << "KEEP FARE: " << keepFare->fareClass() << " \n";
  const PaxTypeFare* matchedExcItinFare = matchKeepFareInExcItin(keepFare);
  if (matchedExcItinFare == nullptr)
  {
    if (hasDiagAndFilterPassed())
      *_dc << "MATCHING FARE NOT FOUND: FAIL \n";
    return false;
  }

  std::map<const PaxTypeFare*, ProcessTagInfo*>::const_iterator usedAsKeepFareIter =
      keepFareProcessTags.find(matchedExcItinFare);
  if (usedAsKeepFareIter == keepFareProcessTags.end())
  {
    // This fare is not used as keep fare in this permutation, all rules should be validated.
    // Fail this reprice solution.
    if (hasDiagAndFilterPassed())
      *_dc << "MATCHING TAG NOT FOUND: FAIL \n";
    return false;
  }

  ProcessTagInfo* tag = usedAsKeepFareIter->second;
  if (tag == nullptr || tag->reissueSequence() == nullptr) // this is wrapper, will never be zero!
  {
    if (hasDiagAndFilterPassed())
      *_dc << "PROCESS TAG INFO ZERO: FAIL \n";
    return false;
  }

  if (!tag->reissueSequence()->orig())
  {
    if (hasDiagAndFilterPassed())
    {
      *_dc << "NO T988 BOOKING CODE REVALIDATION\n";
      *_dc << "IND TREATED AS BLANK: FAIL\n";
    }

    return false;
  }

  if (tag->reissueSequence()->bkgCdRevalInd() != 'X' &&
      tag->reissueSequence()->bkgCdRevalInd() != 'C')
  {
    if (hasDiagAndFilterPassed())
    {
      *_dc << "BOOKING CODE REVALIDATE IND : BLANK \n";
      *_dc << "KEEP FARE FAILED BOOKING CODE: FAIL \n";
    }
    return false;
  }

  if (hasDiagAndFilterPassed())
  {
    *_dc << "BOOKING CODE REVALIDATE IND : X/C \n";
    *_dc << "IGNORE BOOKING CODE FAIL: PASS"
         << " \n";
  }
  return true;
}

bool
RepriceSolutionValidator::revalidateRulesForKeepFares(const ProcessTagPermutation& permutation)
{
  std::set<PricingUnit*> puNeedRevalidations;
  bool puSoftPassed = false;

  for (PricingUnit* pu : _farePath.pricingUnit())
  {
    if (pu->hasKeepFare())
      puNeedRevalidations.insert(pu);

    if (pu->ruleFailedButSoftPassForKeepFare() || pu->combinationFailedButSoftPassForKeepFare())
      puSoftPassed = true;
  }

  if (!puNeedRevalidations.empty())
  {
    if (!permutation.hasZeroT988() && !permutation.needKeepFare() && puSoftPassed)
    {
      if (hasDiagAndFilterPassed())
        *_dc << "REVALIDATE RULES FOR KEEP FARE: FAILED\n";
      return false; // Permuation does not need Keep Fares, but there is rule failed
    }
    std::map<const PaxTypeFare*, ProcessTagInfo*> keepFareProcessTags;

    getKeepFareProcessTags(permutation, keepFareProcessTags);
    for (PricingUnit* pu : puNeedRevalidations)
    {
      if (!revalidateRules(*pu, keepFareProcessTags))
      {
        if (hasDiagAndFilterPassed())
          *_dc << "REVALIDATE RULES FOR KEEP FARE: FAILED\n";
        return false;
      }
    }
  }

  if (hasDiagAndFilterPassed() && !puNeedRevalidations.empty())
    *_dc << "REVALIDATE RULES FOR KEEP FARE: PASSED\n";

  return true;
}

FareApplication
RepriceSolutionValidator::getFareApplication(const ProcessTagPermutation& permutation,
                                             const PaxTypeFare& excFare)
{
  FareApplication fa = UNKNOWN_FA;
  if (_isRebookSolution)
  {
    fa = permutation.rebookFareTypeSelection(excFare.fareMarket()->changeStatus());
  }
  else
  {
    ProcessTagPermutation::PaxTypeFareApplication::const_iterator faIter =
        permutation.fareApplMap().find(&excFare);
    if (faIter != permutation.fareApplMap().end())
    {
      fa = faIter->second;
    }
  }

  return fa;
}

void
RepriceSolutionValidator::getKeepFareProcessTags(
    const ProcessTagPermutation& permutation,
    std::map<const PaxTypeFare*, ProcessTagInfo*>& keepFareProcessTags)
{
  for (ProcessTagInfo* pt : permutation.processTags())
  {
    const PaxTypeFare* excItinFare = pt->paxTypeFare();

    if (!pt->reissueSequence()->orig() || getFareApplication(permutation, *excItinFare) == KEEP)
    {
      keepFareProcessTags.insert(std::make_pair(excItinFare, pt));
    }
  }
}

bool
RepriceSolutionValidator::revalidateRules(
    PricingUnit& pu, const std::map<const PaxTypeFare*, ProcessTagInfo*>& keepFareProcessTags)
{
  for (FareUsage* pfu : pu.fareUsage())
  {
    FareUsage& fu = *pfu;
    fu.categoryIgnoredForKeepFare().clear(); // It maybe populated from another permutation.

    if (fu.isKeepFare())
    {
      PaxTypeFare* keepFare = fu.paxTypeFare();
      const PaxTypeFare* matchedExcItinFare = matchKeepFareInExcItin(keepFare);
      if (matchedExcItinFare == nullptr)
        return false;

      std::map<const PaxTypeFare*, ProcessTagInfo*>::const_iterator usedAsKeepFareIter =
          keepFareProcessTags.find(matchedExcItinFare);
      if (usedAsKeepFareIter == keepFareProcessTags.end())
      {
        if (fu.ruleFailedButSoftPassForKeepFare() || fu.combinationFailedButSoftPassForKeepFare())
        {
          // This fare is not used as keep fare in this permutation, all rules should be validated.
          // Fail this reprice solution.
          return false;
        }
        else
          continue;
      }

      // revalidate rule categories
      ProcessTagInfo* tag = usedAsKeepFareIter->second;

      if (anyFailAtZeroT988(tag, fu, pu))
        return false;

      if (tag != nullptr && tag->reissueSequence()->orig() &&
          tag->reissueSequence()->revalidationInd() != BLANK)
      {
        if (!validateCat35Security(fu, tag))
          return false;
        getCategoryIgnored(*tag, fu.categoryIgnoredForKeepFare());
        if (fu.categoryIgnoredForKeepFare().empty())
        {
          if (fu.ruleFailedButSoftPassForKeepFare() || fu.combinationFailedButSoftPassForKeepFare())
            return false;
          else
            continue;
        }

        if (fu.combinationFailedButSoftPassForKeepFare() &&
            fu.categoryIgnoredForKeepFare().find(10) == fu.categoryIgnoredForKeepFare().end())
          return false;

        // Find fare market scope rules that could not be ignored
        std::vector<uint16_t> fmCategoriesNeedReval;
        std::vector<uint16_t> puCategoriesNeedReval;
        if (isRequiredRuleFailed(fu, 1, fmCategoriesNeedReval) ||
            isRequiredRuleFailed(fu, 2, puCategoriesNeedReval) ||
            isRequiredRuleFailed(fu, 3, puCategoriesNeedReval) ||
            isRequiredRuleFailed(fu, 4, fmCategoriesNeedReval) ||
            isRequiredRuleFailed(fu, 6, puCategoriesNeedReval) ||
            isRequiredRuleFailed(fu, 7, puCategoriesNeedReval) ||
            isRequiredRuleFailed(fu, 8, puCategoriesNeedReval) ||
            isRequiredRuleFailed(fu, 9, puCategoriesNeedReval) ||
            isRequiredRuleFailed(fu, 11, fmCategoriesNeedReval) ||
            isRequiredRuleFailed(fu, 13, puCategoriesNeedReval) ||
            isRequiredRuleFailed(fu, 14, puCategoriesNeedReval) ||
            isRequiredRuleFailed(fu, 15, fmCategoriesNeedReval) ||
            isRequiredRuleFailed(fu, 23, fmCategoriesNeedReval))
          return false;

        if (!fmCategoriesNeedReval.empty() &&
            !revalidateRuleInFareMarketScope(*keepFare, fmCategoriesNeedReval))
          return false;

        if (!puCategoriesNeedReval.empty() &&
            !revalidateRuleInPuScope(pu, fu, puCategoriesNeedReval))
          return false;
      }
      else
      {
        if (fu.ruleFailedButSoftPassForKeepFare() || fu.combinationFailedButSoftPassForKeepFare())
          return false;
      }
    }
  }

  return true;
}

bool
RepriceSolutionValidator::anyFailAtZeroT988(const ProcessTagInfo* tag,
                                            const FareUsage& fu,
                                            const PricingUnit& pu) const
{
  return tag != nullptr && !tag->reissueSequence()->orig() &&
         (pu.ruleFailedButSoftPassForKeepFare() || pu.combinationFailedButSoftPassForKeepFare() ||
          fu.ruleFailedButSoftPassForKeepFare() || fu.combinationFailedButSoftPassForKeepFare());
}

bool
RepriceSolutionValidator::isRequiredRuleFailed(FareUsage& fu,
                                               const uint16_t& category,
                                               std::vector<uint16_t>& categoryNeedReval)
{
  if (fu.categoryIgnoredForKeepFare().find(category) == fu.categoryIgnoredForKeepFare().end())
  {
    PaxTypeFare* fare = fu.paxTypeFare();
    if (fare != nullptr && fare->isCategoryProcessed(category) && !fare->isCategoryValid(category))
      return true;

    categoryNeedReval.push_back(category);
  }

  return false;
}

bool
RepriceSolutionValidator::revalidateRuleInFareMarketScope(
    PaxTypeFare& keepFare, const std::vector<uint16_t>& fmCategoriesNeedReval)
{
  _trx.setFareApplicationDT(keepFare.retrievalInfo()->_date);

  RuleControllerWithChancelor<FareMarketRuleController> ruleController(PURuleValidation,
                                                                       fmCategoriesNeedReval);
  bool fmRevalResult = ruleController.validate(
      *(static_cast<PricingTrx*>(&_trx)), *(const_cast<Itin*>(_farePath.itin())), keepFare);

  _trx.setFareApplicationDT(_trx.currentTicketingDT());

  return fmRevalResult;
}

bool
RepriceSolutionValidator::revalidateRuleInPuScope(PricingUnit& pu,
                                                  FareUsage& fu,
                                                  std::vector<uint16_t>& categoriesNeedReval)
{
  _trx.setFareApplicationDT(fu.paxTypeFare()->retrievalInfo()->_date);

  analyzeFUforAlreadyProcessedAndPassesdCat8Cat9(fu, categoriesNeedReval);
  RuleControllerWithChancelor<PricingUnitRuleController> ruleController(DynamicValidation,
                                                                        categoriesNeedReval);
  bool puRevalResult = ruleController.validate(
      *(static_cast<PricingTrx*>(&_trx)), *(const_cast<FarePath*>(&_farePath)), pu, fu);

  _trx.setFareApplicationDT(_trx.currentTicketingDT());

  return puRevalResult;
}

const PaxTypeFare*
RepriceSolutionValidator::matchKeepFareInExcItin(const PaxTypeFare* keepFare)
{
  for (const RexPricingTrx::NewItinKeepFareMap::value_type& excItinFareToNewFm :
       _trx.newItinKeepFares(_itinIndex))
  {
    if (excItinFareToNewFm.second == keepFare->fareMarket())
      return excItinFareToNewFm.first;
  }

  return _expndKeepValidator.getExcPtf(*keepFare);
}

void
RepriceSolutionValidator::getCategoryIgnored(const ProcessTagInfo& tag,
                                             std::set<uint16_t>& catgoryIgnored)
{
  const ReissueSequenceW& tbl988 = *(tag.reissueSequence());
  bool markedAsIgnored = (tbl988.revalidationInd() == 'X');
  if ((markedAsIgnored && tbl988.provision1() == 'X') ||
      (!markedAsIgnored && tbl988.provision1() == BLANK))
  {
    catgoryIgnored.insert(1);
  }
  if ((markedAsIgnored && tbl988.provision2() == 'X') ||
      (!markedAsIgnored && tbl988.provision2() == BLANK))
  {
    catgoryIgnored.insert(2);
  }
  if ((markedAsIgnored && tbl988.provision3() == 'X') ||
      (!markedAsIgnored && tbl988.provision3() == BLANK))
  {
    catgoryIgnored.insert(3);
  }
  if ((markedAsIgnored && tbl988.provision4() == 'X') ||
      (!markedAsIgnored && tbl988.provision4() == BLANK))
  {
    catgoryIgnored.insert(4);
  }
  if ((markedAsIgnored && tbl988.provision5() == 'X') ||
      (!markedAsIgnored && tbl988.provision5() == BLANK))
  {
    if (isTktResvBytesBlank(tbl988))
      catgoryIgnored.insert(5);
  }
  if ((markedAsIgnored && tbl988.provision6() == 'X') ||
      (!markedAsIgnored && tbl988.provision6() == BLANK))
  {
    catgoryIgnored.insert(6);
  }
  if ((markedAsIgnored && tbl988.provision7() == 'X') ||
      (!markedAsIgnored && tbl988.provision7() == BLANK))
  {
    catgoryIgnored.insert(7);
  }
  if ((markedAsIgnored && tbl988.provision8() == 'X') ||
      (!markedAsIgnored && tbl988.provision8() == BLANK))
  {
    catgoryIgnored.insert(8);
  }
  if ((markedAsIgnored && tbl988.provision9() == 'X') ||
      (!markedAsIgnored && tbl988.provision9() == BLANK))
  {
    catgoryIgnored.insert(9);
  }
  if ((markedAsIgnored && tbl988.provision10() == 'X') ||
      (!markedAsIgnored && tbl988.provision10() == BLANK))
  {
    catgoryIgnored.insert(10);
  }
  if ((markedAsIgnored && tbl988.provision11() == 'X') ||
      (!markedAsIgnored && tbl988.provision11() == BLANK))
  {
    catgoryIgnored.insert(11);
  }
  if ((markedAsIgnored && tbl988.provision13() == 'X') ||
      (!markedAsIgnored && tbl988.provision13() == BLANK))
  {
    catgoryIgnored.insert(13);
  }
  if ((markedAsIgnored && tbl988.provision14() == 'X') ||
      (!markedAsIgnored && tbl988.provision14() == BLANK))
  {
    catgoryIgnored.insert(14);
  }
  if ((markedAsIgnored && tbl988.provision15() == 'X') ||
      (!markedAsIgnored && tbl988.provision15() == BLANK))
  {
    catgoryIgnored.insert(15);
  }
  if ((markedAsIgnored && tbl988.provision18() == 'X') ||
      (!markedAsIgnored && tbl988.provision18() == BLANK))
  {
    catgoryIgnored.insert(23);
  }
}

bool
RepriceSolutionValidator::isTktResvBytesBlank(const ReissueSequenceW& tbl988)
{
  return (tbl988.ticketResvInd() == BLANK && tbl988.departureInd() == BLANK &&
          tbl988.reissueTOD() == -1 && tbl988.reissuePeriod().empty() &&
          tbl988.reissueUnit().empty() && tbl988.optionInd() == BLANK && tbl988.departure() == 0 &&
          tbl988.departureUnit() == BLANK);
}

bool
RepriceSolutionValidator::isBaseFareAmountPlusChangeFeeHigher(const MoneyAmount& changeFee)
{
  const FarePath& excFarePath = *_trx.exchangeItin().front()->farePath().front();
  bool result =
      _farePath.getTotalNUCAmount() + changeFee > excFarePath.getTotalNUCAmount() + EPSILON;

  if (result && hasDiagAndFilterPassed())
    *_dc << "TAG CHECK FAILED: REPRICE AMT HIGHER THAN EXCHANGE\n";
  return result;
}

bool
RepriceSolutionValidator::hasDiagAndFilterPassed() const
{
  return _dc && _dc->filterPassed();
}

namespace
{
class IsChanged
{
  const std::map<TravelSeg*, PaxTypeFare::SegmentStatus*>& _ts2ss;

public:
  IsChanged(const std::map<TravelSeg*, PaxTypeFare::SegmentStatus*>& ts2ss) : _ts2ss(ts2ss) {}

  bool operator()(TravelSeg* ts) const
  {
    bool result = ts->changeStatus() == TravelSeg::CHANGED ||
                  ts->changeStatus() == TravelSeg::INVENTORYCHANGED;
    if (ts->changeStatus() == TravelSeg::CHANGED && !ts->isCabinChanged())
    {
      result = true;
    }
    else if (ts->newTravelUsedToSetChangeStatus().empty())
    {
      if (ts->unflown())
      {
        LOG4CXX_DEBUG(logger,
                      "RepriceSolutionValidator::IsChanged"
                      " - empty newTravelUsedToSetChangeStatus vector");
      }
    }
    else if (ts->segmentType() != Arunk && ts->unflown())
    {
      for (TravelSeg* newTs : ts->newTravelUsedToSetChangeStatus())
      {
        bool isNewTS = false;
        std::map<TravelSeg*, PaxTypeFare::SegmentStatus*>::const_iterator segmentStatusIter =
            _ts2ss.find(newTs);
        if (segmentStatusIter == _ts2ss.end())
        {
          isNewTS = true;
          segmentStatusIter = _ts2ss.find(ts);
          if (segmentStatusIter == _ts2ss.end())
          {
            LOG4CXX_DEBUG(logger,
                          "RepriceSolutionValidator::IsChanged"
                          " - wrong segment status vector");
            break;
          }
        }
        PaxTypeFare::SegmentStatus* segmentStatus = segmentStatusIter->second;
        if (!segmentStatus)
        {
          LOG4CXX_DEBUG(logger, "RepriceSolutionValidator::IsChanged - empty segment status");
          break;
        }
        if (!segmentStatus->_bkgCodeReBook.empty() &&
            segmentStatus->_bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED))
        {
          result = false;
          if (segmentStatus->_bkgCodeReBook != (isNewTS ? newTs : ts)->getBookingCode())
            result = true;
          break;
        }
      }
    }
    return result;
  }
};

} // namespace

bool
RepriceSolutionValidator::isTravelSegmentStatusChanged(
    const std::map<TravelSeg*, PaxTypeFare::SegmentStatus*>& ts2ss, TravelSeg& ts) const
{
  IsChanged isChanged(ts2ss);
  return isChanged(&ts);
}

void
RepriceSolutionValidator::setFareMarketChangeStatus(ExcItin* itinFirst, Itin* itinSecond)
{
  TravelSeg* pointOfChangeTravelSeg = nullptr;
  std::vector<TravelSeg*>::const_iterator tvlSegI = itinFirst->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlSegIEnd = itinFirst->travelSeg().end();

  if ((tvlSegI = std::find_if(tvlSegI, tvlSegIEnd, IsChanged(_ts2ss))) != tvlSegIEnd)
    pointOfChangeTravelSeg = *tvlSegI;

  int16_t pointOfChangeFirst =
      (pointOfChangeTravelSeg != nullptr ? pointOfChangeTravelSeg->pnrSegment() : -1);

  pointOfChangeTravelSeg = nullptr;
  tvlSegI = itinSecond->travelSeg().begin();
  tvlSegIEnd = itinSecond->travelSeg().end();

  if ((tvlSegI = std::find_if(tvlSegI, tvlSegIEnd, IsChanged(_ts2ss))) != tvlSegIEnd)
    pointOfChangeTravelSeg = *tvlSegI;

  int16_t pointOfChangeSecond =
      (pointOfChangeTravelSeg != nullptr ? pointOfChangeTravelSeg->pnrSegment() : -1);

  for (FareMarket* pfm : itinFirst->fareMarket())
  {
    FareMarket& fareMarket = *pfm;
    fareMarket.changeStatus() =
        getChangeStatus(fareMarket.travelSeg(), pointOfChangeFirst, pointOfChangeSecond);
  }
}

struct RepriceSolutionValidator::IsUnflown
{
  bool operator()(TravelSeg* seg) { return seg->unflown(); }
};

FCChangeStatus
RepriceSolutionValidator::getChangeStatus(const std::vector<TravelSeg*>& tvlSegs,
                                          const int16_t& pointOfChgFirst,
                                          const int16_t& pointOfChgSecond)
{
  std::vector<TravelSeg*>::const_iterator tvlSegI = tvlSegs.begin();
  const std::vector<TravelSeg*>::const_iterator tvlSegIEnd = tvlSegs.end();

  if ((tvlSegI = std::find_if(tvlSegI, tvlSegIEnd, IsUnflown())) == tvlSegIEnd)
    return tse::FL;
  if (std::find_if(tvlSegI, tvlSegIEnd, IsChanged(_ts2ss)) != tvlSegIEnd)
    return tse::UC;

  int16_t pointOfChg = pointOfChgFirst;
  TravelSeg* lastSeg = tvlSegs.back();

  if (pointOfChgFirst == -1 && !tvlSegs.back()->newTravelUsedToSetChangeStatus().empty())
  {
    pointOfChg = pointOfChgSecond;
    lastSeg = tvlSegs.back()->newTravelUsedToSetChangeStatus().back();
  }

  if (lastSeg->pnrSegment() > pointOfChg && pointOfChg > 0)
    return tse::UN;
  else
    return tse::UU;
}

struct RepriceSolutionValidator::IsStopOver
{
  bool operator()(TravelSeg* seg) { return (seg->isForcedStopOver() || seg->stopOver()); }
};

const PricingUnit*
RepriceSolutionValidator::determineExcPrU(const ProcessTagInfo& pti) const
{
  for (const PricingUnit* prU : _trx.exchangeItin().front()->farePath().front()->pricingUnit())
  {
    if (tse::farepathutils::failedFareExistsInPU(pti.paxTypeFare(), *prU))
      return prU;
  }

  LOG4CXX_FATAL(logger, "Not found proper pricing unit");

  return nullptr;
}

bool
RepriceSolutionValidator::matchOutboundPortionOfTvl(const ProcessTagInfo& pti,
                                                    uint16_t fcNumber,
                                                    bool diagAndIfFilteredFilterPassed,
                                                    DiagCollector* _dc) const
{
  const ReissueSequence& t988Seq = *pti.reissueSequence()->orig();

  if (t988Seq.outboundInd() == ProcessTagInfo::NO_RESTRICTION)
    return true;

  const PricingUnit* prU = determineExcPrU(pti);

  if (!prU)
    return false;

  std::vector<TravelSeg*>::const_iterator segRangeBegin, segRangeEnd;

  switch (t988Seq.outboundInd())
  {
  case ProcessTagInfo::FIRST_FC:
    segRangeBegin = prU->fareUsage().front()->travelSeg().begin();
    segRangeEnd = prU->fareUsage().front()->travelSeg().end();
    break;

  case ProcessTagInfo::ORIG_TO_STOPOVER:
    segRangeBegin = prU->travelSeg().begin();
    segRangeEnd = std::find_if(prU->travelSeg().begin(), prU->travelSeg().end(), IsStopOver());
    if (segRangeEnd != prU->travelSeg().end())
      ++segRangeEnd;
    break;

  default:
    LOG4CXX_ERROR(logger,
                  "T988 item No. " << t988Seq.itemNo() << " seq No. " << t988Seq.seqNo()
                                   << " incorrect byte 22");
    if (diagAndIfFilteredFilterPassed)
      *_dc << "\nERROR: TABLE 988 SEQ NO " << t988Seq.seqNo() << " INCORRECT BYTE 22\n";
    return false;
  }

  std::vector<TravelSeg*>::const_iterator firstChanged =
      std::find_if(segRangeBegin, segRangeEnd, IsChanged(_ts2ss));

  if (firstChanged == segRangeEnd)
    return true;

  if (diagAndIfFilteredFilterPassed)
  {
    *_dc << "BYTE 22 CHECK FAILED(" << fcNumber << ")\n"
         << "  SEQ: " << t988Seq.seqNo() << "  OUTBOUND IND: " << t988Seq.outboundInd()
         << "  CHANGE TO " << (*firstChanged)->origAirport() << "-"
         << (*firstChanged)->destAirport() << " SEGMENT NOT ALLOWED\n";
  }

  return false;
}

bool
RepriceSolutionValidator::validateCat35Security(const FareUsage& fu, const ProcessTagInfo* tag)
{
  if (!fu.paxTypeFare()->isNegotiated())
    return true;

  if (hasDiagAndFilterPassed())
    *_dc << "VALIDATING CAT35 SECURITY FARE: " << fu.paxTypeFare()->fareClass() << " \n";

  const NegPaxTypeFareRuleData* negPaxTypeFareRuleData = fu.paxTypeFare()->getNegRuleData();
  if (negPaxTypeFareRuleData == nullptr)
  {
    if (hasDiagAndFilterPassed())
      *_dc << "NEG PAX TYPE FARE RULE DATA MISSING: FAIL \n";
    return false;
  }

  if (!negPaxTypeFareRuleData->rexCat35FareUsingPrevAgent())
  {
    if (hasDiagAndFilterPassed())
      *_dc << "CAT35 SECURITY USING NEW AGENT: PASS \n";
    return true;
  }

  if (hasDiagAndFilterPassed())
    *_dc << "CAT35 SECURITY USING PREVIOUS AGENT \n";

  const ReissueSequenceW& tbl988 = *((*tag).reissueSequence());
  bool markedAsIgnored = (tbl988.revalidationInd() == 'X');
  if ((markedAsIgnored && tbl988.provision15() == 'X') ||
      (!markedAsIgnored && tbl988.provision15() == BLANK))
  {
    if (hasDiagAndFilterPassed())
      *_dc << "IGNORE CAT 35 SECURITY: PASS \n";
    return true;
  }

  if (hasDiagAndFilterPassed())
    *_dc << "DO NOT IGNORE CAT 35 SECURITY: FAIL \n";

  return false;
}

MoneyAmount
RepriceSolutionValidator::getTotalChangeFee(ProcessTagPermutation& permutation)
{
  ReissueCharges* reissueCharge = _permutationReissueCharges[&permutation];
  if (!reissueCharge || reissueCharge->changeFee < EPSILON)
    return 0.0;

  return reissueCharge->changeFeeInCalculationCurrency;
}

void
RepriceSolutionValidator::getMinFarePlusUps()
{
  if (!_farePath.minFareCheckDone())
  {
    MinFareChecker minFareChecker;
    minFareChecker.process(_trx, _farePath);
  }
}

void
RepriceSolutionValidator::analyzeFUforAlreadyProcessedAndPassesdCat8Cat9(
    FareUsage& fu, std::vector<uint16_t>& categoriesNeedReval)
{
  std::vector<uint16_t>::iterator cat =
      find(categoriesNeedReval.begin(), categoriesNeedReval.end(), 8);
  if (cat != categoriesNeedReval.end() && !fu.stopoverSurcharges().empty())
    categoriesNeedReval.erase(cat);

  cat = find(categoriesNeedReval.begin(), categoriesNeedReval.end(), 9);
  if (cat != categoriesNeedReval.end() && !fu.transferSurcharges().empty())
    categoriesNeedReval.erase(cat);
}

const MoneyAmount
RepriceSolutionValidator::getNewTicketAmount()
{
  MoneyAmount newValueOfNewTicket = _farePath.getTotalNUCAmount();
  if (hasDiagAndFilterPassed())
  {
    if (!_trx.isPlusUpCalculationNeeded())
    {
      *_dc << "TOTAL AMOUNT FOR NEW TICKET: " << newValueOfNewTicket << "NUC\n";
    }
  }
  if (_trx.applyReissueExchange() &&
      (_farePath.itin()->originationCurrency() != _farePath.itin()->calculationCurrency()))
    newValueOfNewTicket = _farePath.convertToBaseCurrency(_trx, newValueOfNewTicket, NUC);

  return newValueOfNewTicket;
}

const MoneyAmount
RepriceSolutionValidator::getExchTotalAmount()
{
  if (_trx.applyReissueExchange())
  {
    RexPricingOptions* rexOptions = dynamic_cast<RexPricingOptions*>(_trx.getOptions());
    if (rexOptions->excTotalFareAmt().empty())
      throw ErrorResponseException(ErrorResponseException::REISSUE_RULES_FAIL);
    return (double)atof(rexOptions->excTotalFareAmt().c_str()); // C5A
  }

  return getExchNUCAmount();
}

const MoneyAmount
RepriceSolutionValidator::getExchNUCAmount()
{
  const FarePath& excFarePath = *_trx.exchangeItin().front()->farePath().front();
  if (hasDiagAndFilterPassed())
  {
    if (!_trx.isPlusUpCalculationNeeded())
    {
      *_dc << "TOTAL AMOUNT FOR EXCHANGE TICKET: " << excFarePath.getTotalNUCAmount() << "NUC\n";
    }
  }

  return excFarePath.getTotalNUCAmount();
}

const CurrencyCode
RepriceSolutionValidator::getCurrency()
{
  if (_trx.applyReissueExchange())
    return _farePath.itin()->originationCurrency();

  return _calcCurrCode;
}

const MoneyAmount
RepriceSolutionValidator::getNonRefAmountInBaseCurrencyOfExchangeTicket()
{
  const FarePath& excFarePath = *_trx.exchangeItin().front()->farePath().front();
  MoneyAmount nonRefundableAmountsOfExchangeTicket = excFarePath.getNonrefundableAmountInNUC(_trx);
  if (hasDiagAndFilterPassed())
  {
    if (!_trx.isPlusUpCalculationNeeded())
    {
      *_dc << "NONREFUNDABLE AMOUNT FOR EXCHANGE TICKET: " << nonRefundableAmountsOfExchangeTicket
           << "NUC\n";
    }
  }

  if (_trx.applyReissueExchange())
  {
    nonRefundableAmountsOfExchangeTicket =
        _farePath.convertToBaseCurrency(_trx, nonRefundableAmountsOfExchangeTicket, NUC);
  }
  return nonRefundableAmountsOfExchangeTicket;
}

const MoneyAmount
RepriceSolutionValidator::getNonRefAmountInBaseCurrencyOfNewTicket()
{
  MoneyAmount nonRefundableAmountsOfNewTicket = _farePath.getNonrefundableAmountInNUC(_trx);
  if (hasDiagAndFilterPassed())
  {
    if (!_trx.isPlusUpCalculationNeeded())
    {
      *_dc << "NONREFUNDABLE AMOUNT FOR NEW TICKET: " << nonRefundableAmountsOfNewTicket << "NUC\n";
    }
  }

  if (_trx.applyReissueExchange() && nonRefundableAmountsOfNewTicket > 0)
    nonRefundableAmountsOfNewTicket =
        _farePath.convertToBaseCurrency(_trx, nonRefundableAmountsOfNewTicket, NUC);

  return nonRefundableAmountsOfNewTicket;
}

void
RepriceSolutionValidator::checkTimeout(const uint32_t& permNumber)
{
  if (_permCheckInterval > 0 && permNumber % _permCheckInterval == 0)
  {
    try
    {
      if (fallback::reworkTrxAborter(&_trx))
        checkTrxAborted(_trx);
      else
        _trx.checkTrxAborted();
    }
    catch (ErrorResponseException& ex)
    {
      LOG4CXX_ERROR(logger,
                    "REPRICE SOLUTION VALIDATED PERMUTATIONS " << (permNumber - 1)
                                                               << " TIMEOUT_REACHED");
      throw ErrorResponseException(ErrorResponseException::MAX_NUMBER_COMBOS_EXCEEDED);
    }
  }
}

void
RepriceSolutionValidator::checkMemory(uint32_t permNumber)
{
  TrxUtil::checkTrxMemoryAborted(_trx, permNumber, _permCheckInterval, _memCheckTrxInterval);
}

void
RepriceSolutionValidator::checkDiag()
{
  if (_dc)
  {
    LOG4CXX_DEBUG(logger, "curDiagSize=" << _dc->str().size());
    if (_dc->str().size() >= _maxDiagSize)
    {
      LOG4CXX_INFO(logger, "diag689 size " << _dc->str().size() << " reach limit " << _maxDiagSize);
      _dc->flushMsg();
      _dc = nullptr;
    }
  }
}

} // tse
