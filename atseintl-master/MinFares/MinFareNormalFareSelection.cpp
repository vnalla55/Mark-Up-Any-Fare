//----------------------------------------------------------------------------
//  Copyright Sabre 2004
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------
#include "MinFares/MinFareNormalFareSelection.h"

#include "Common/CabinType.h"
#include "Common/FareMarketUtil.h"
#include "Common/Global.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/GoverningCarrier.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/TseUtil.h"
#include "Common/Vendor.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RepricingTrx.h"
#include "DBAccess/FareTypeMatrix.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MinFareAppl.h"
#include "DBAccess/MinFareDefaultLogic.h"
#include "DBAccess/MinFareRuleLevelExcl.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/DiagManager.h"
#include "MinFares/MinimumFare.h"

#include <functional>
#include <iomanip>

namespace tse
{
static Logger
logger("atseintl.MinFares.MinFareNormalFareSelection");

MinFareNormalFareSelection::MinFareNormalFareSelection(
    MinimumFareModule module,
    EligibleFare eligibleFare,
    FareDirectionChoice fareDirection,
    CabinType cabin,
    PricingTrx& trx,
    const Itin& itin,
    const std::vector<TravelSeg*>& travelSegs,
    const std::vector<PricingUnit*>& pricingUnits,
    const PaxType* paxType,
    const DateTime& travelDate,
    const FarePath* farePath,
    const PaxTypeFare* thruFare,
    const MinFareAppl* minFareAppl,
    const MinFareDefaultLogic* minFareDefaultLogic,
    const RepricingTrx* repricingTrx,
    const PaxTypeCode& actualPaxType)
  : MinFareFareSelection(module,
                         eligibleFare,
                         fareDirection,
                         trx,
                         itin,
                         travelSegs,
                         pricingUnits,
                         paxType,
                         travelDate,
                         farePath,
                         thruFare,
                         minFareAppl,
                         minFareDefaultLogic,
                         repricingTrx,
                         actualPaxType),
    _preferedFareType("")
{
  _cabin = cabin;
}

const PaxTypeFare*
MinFareNormalFareSelection::selectFare(PaxTypeStatus selPaxTypeStatus /*=PAX_TYPE_STATUS_UNKNOWN*/,
                                       bool selectNextCabin /*=false*/)
{
  return selectFare(_travelSegs, selPaxTypeStatus, selectNextCabin);
}

/**
 * This function is used to select a normal fare for a market.
 *
 * @return const Fare* - Fare selected.
 */
const PaxTypeFare*
MinFareNormalFareSelection::selectFare(const std::vector<TravelSeg*>& travelSegs,
                                       PaxTypeStatus selPaxTypeStatus /*=PAX_TYPE_STATUS_UNKNOWN*/,
                                       bool selectNextCabin /*=false*/)
{
  LOG4CXX_DEBUG(logger, "Enter selectFare");

  _selPaxTypeStatus =
      (selPaxTypeStatus == PAX_TYPE_STATUS_UNKNOWN) ? _paxTypeStatus : selPaxTypeStatus;

  // Check fare sel preference
  getFareSelPref();

  const PaxTypeFare* selectedFare = nullptr;

  // Check YY override
  _yyOverride = FareMarketUtil::isYYOverride(_trx, travelSegs);

  if (_diag != nullptr)
  {
    (*_diag) << "\n****************" << MinFareModuleName[_module]
             << " NORMAL FARE SELECTION*******************\n";
    displayFareSelectReq(travelSegs, _cabin);
    displayMinFareApplLogic();
  }

  if (_cabin.isUnknownClass())
  {
    LOG4CXX_DEBUG(logger, "Exit selectFare");
    return selectedFare;
  }

  // Select the same cabin fare
  if (!_preferedFareType.empty())
    _sameFareType = true;

  // Check GovCxr override
  _govCxrOverrides.clear();
  FareMarketUtil::getGovCxrOverride(_trx, travelSegs, _govCxrOverrides);

  // Look fare in specified GovCxr fare market first
  if (!_yyOverride && !_govCxrOverrides.empty())
  {
    std::vector<CarrierCode>::iterator govCxrOverrideIter = _govCxrOverrides.begin();
    for (; govCxrOverrideIter != _govCxrOverrides.end(); ++govCxrOverrideIter)
    {
      _govCxr = *govCxrOverrideIter;

      selectedFare = selectFareForCabin(travelSegs, _cabin, true);

      if (selectedFare == nullptr)
      {
        if (selectNextCabin || isDefaultThruAll())
        {
          if (_cabin.isEconomyClass())
            selectedFare = selectFareForHigherCabin(travelSegs);
          else
            selectedFare = selectFareForLowerCabin(travelSegs);
        }
      }

      if (selectedFare)
        break;
    }
  }

  // Try fare markets with normal GovCxr logic
  if (selectedFare == nullptr)
  {
    _govCxr = "";

    selectedFare = selectFareForCabin(travelSegs, _cabin, true);
    if (selectedFare == nullptr)
    {
      if (selectNextCabin || isDefaultThruAll())
      {
        if (_cabin.isEconomyClass())
          selectedFare = selectFareForHigherCabin(travelSegs);
        else
          selectedFare = selectFareForLowerCabin(travelSegs);
      }
    }
  }

  if (_diag != nullptr)
  {
    if (selectedFare == nullptr)
    {
      (*_diag) << " \nNO FARE SELECTED\n";
    }
    _diag->flushMsg();
  }

  LOG4CXX_DEBUG(logger, "Exit selectFare");

  return selectedFare;
}

void
MinFareNormalFareSelection::getFareMarket(const std::vector<TravelSeg*>& travelSegs,
                                          CabinType cabin)
{
  // Clear old fares
  _govCxrValidatedFares.clear();
  _govCxrNonValidatedFares.clear();
  _yyValidatedFares.clear();
  _yyNonValidatedFares.clear();

  if (travelSegs.empty())
  {
    return;
  }

  if (_diag != nullptr)
  {
    (*_diag) << travelSegs.front()->boardMultiCity() << "-" << travelSegs.back()->offMultiCity()
             << " PAX/";

    if (!_actualPaxType.empty())
      (*_diag) << _actualPaxType;
    else
      (*_diag) << ((_paxType == nullptr) ? "   " : _paxType->paxType());

    (*_diag) << " CABIN/" << cabin;

    if (_sameFareType)
    {
      (*_diag) << " FARETYPE/" << _preferedFareType;
    }

    if (!_govCxr.empty())
      (*_diag) << " GOVCXROVERRIDE/" << _govCxr;

    if (_repricingTrx != nullptr)
      (*_diag) << " REPRICING";

    (*_diag) << " \n";
  }

  // Get all fare markets for the paxType
  std::vector<FareMarket*> fareMarkets;
  if (_repricingTrx != nullptr)
    TrxUtil::getFareMarket(
        *_repricingTrx,
        travelSegs,
        _thruFare
            ? _thruFare->retrievalDate()
            : _farePath->pricingUnit().front()->fareUsage().front()->paxTypeFare()->retrievalDate(),
        fareMarkets,
        &_itin);
  else
  {
    TrxUtil::getFareMarket(
        _trx,
        travelSegs,
        _thruFare
            ? _thruFare->retrievalDate()
            : _farePath->pricingUnit().front()->fareUsage().front()->paxTypeFare()->retrievalDate(),
        fareMarkets,
        &_itin);

    // Check if need reprice for Net Remit
    if ((fareMarkets.size() < 1) && isRepriceNeeded())
    {
      PricingTrx* newTrx = getRepriceTrx(travelSegs);
      if (newTrx != nullptr)
      {
        TrxUtil::getFareMarket(*newTrx,
                               travelSegs,
                               _thruFare ? _thruFare->retrievalDate() : _farePath->pricingUnit()
                                                                            .front()
                                                                            ->fareUsage()
                                                                            .front()
                                                                            ->paxTypeFare()
                                                                            ->retrievalDate(),
                               fareMarkets,
                               &_itin);
      }
    }
  }

  // Special code for RSC, OSC and CTM to get fare market without matching all travel segments
  // as long as start and end segments are same.
  if (((_module == RSC) || (_module == OSC) || (_module == CTM)) &&
      ((fareMarkets.size() < 1) ||
       (fareMarkets.front()->paxTypeCortege(_paxType) == nullptr))) // No market or market has no fares
  {
    fareMarkets.clear(); // Remove the one with no fares
    getAlternateFareMarket(travelSegs, fareMarkets);
  }

  if (!_govCxr.empty()) // Look fare in specified GovCxr fare market first
  {
    std::vector<FareMarket*>::iterator fmIter = fareMarkets.begin();
    for (; fmIter != fareMarkets.end(); ++fmIter)
    {
      FareMarket& fareMarket = **fmIter;
      if (fareMarket.governingCarrier() == _govCxr)
      {
        processFareMarket(fareMarket, cabin);
        break;
      }
    }
  }
  else // Look fare in normal GovCxr logic fare market
  {
    std::vector<FareMarket*>::iterator fmIter = fareMarkets.begin();
    for (; fmIter != fareMarkets.end(); ++fmIter)
    {
      FareMarket& fareMarket = **fmIter;
      if (_yyOverride)
      {
        if (fareMarket.governingCarrier() == INDUSTRY_CARRIER)
        {
          processFareMarket(fareMarket, cabin);
          break;
        }
      }
      else
      {
        if (_govCxrOverrides.empty() ||
            (std::find(_govCxrOverrides.begin(),
                       _govCxrOverrides.end(),
                       fareMarket.governingCarrier()) == _govCxrOverrides.end()))
        {
          processFareMarket(fareMarket, cabin);
        }
      }
    }
  }
}

void
MinFareNormalFareSelection::processFareMarket(const FareMarket& fareMarket, CabinType cabin)
{
  _globalDirection = fareMarket.getGlobalDirection();
  const CarrierCode& govCxr = fareMarket.governingCarrier();
  bool yyAllowed =
      ((govCxr == INDUSTRY_CARRIER) || isCxrAllowYYFare(govCxr, fareMarket.travelSeg()));
  if (_diag != nullptr)
  {
    (*_diag) << FareMarketUtil::getDisplayString(fareMarket) << "\n";

    std::string gd;
    globalDirectionToStr(gd, _globalDirection);
    (*_diag) << "GI/" << gd << " " << govCxr << (yyAllowed ? " ALLOW" : " DISALLOW")
             << " YY FARE\n\n";

    displayFareHeader();
  }

  if (_actualPaxType.empty())
  {
    const PaxTypeBucket* paxTypeCortege = fareMarket.paxTypeCortege(_paxType);
    if (paxTypeCortege == nullptr)
      return;

    const std::vector<PaxTypeFare*>& fares = paxTypeCortege->paxTypeFare();
    identifyValidatedFares(fares, cabin, yyAllowed);
  }
  else // Only HIP uses _actualPaxType for selecting fare for Double discounted child fare
  {
    const std::vector<PaxTypeFare*>& fares = fareMarket.allPaxTypeFare();
    identifyValidatedFares(fares, cabin, yyAllowed);
  }
}

const PaxTypeFare*
MinFareNormalFareSelection::selectFareForCabin(const std::vector<TravelSeg*>& travelSegs,
                                               const CabinType cabin,
                                               bool selectLowest)
{
  const PaxTypeFare* selectedFare = nullptr;

  if (cabin.isUnknownClass())
    return selectedFare;

  getFareMarket(travelSegs, cabin);

  // Select validated GovCxr fare
  selectedFare = selectGovCxrValidatedFare(
      _govCxrValidatedFares, _govCxrNonValidatedFares, selectLowest, (cabin == _cabin));

  if (selectedFare != nullptr)
  {
    if (_diag != nullptr)
    {
      (*_diag) << " \nFARE SELECTED: \n";
      displayFare(*selectedFare);
      (*_diag) << " \n";
    }
    return selectedFare;
  }

  // Select validated YY fare
  selectedFare = selectValidatedFare(_yyValidatedFares, selectLowest, (cabin == _cabin));

  if (selectedFare != nullptr)
  {
    if (_diag != nullptr)
    {
      (*_diag) << " \nFARE SELECTED: \n";
      displayFare(*selectedFare);
      (*_diag) << " \n";
    }

    return selectedFare;
  }

  if (!_repricingTrx && __builtin_expect(_trx.isFootNotePrevalidationAllowed(), true))
    getFareMarket(travelSegs, cabin);

  // Select Non-validated GovCxr fare
  selectedFare = selectNonValidatedFare(_govCxrNonValidatedFares);
  if (selectedFare != nullptr)
  {
    if (_diag != nullptr)
    {
      (*_diag) << " \nFARE SELECTED: \n";
      displayFare(*selectedFare);
      (*_diag) << " \n";
    }

    return selectedFare;
  }

  // Select Non-validated YY fare
  selectedFare = selectNonValidatedFare(_yyNonValidatedFares);
  if (selectedFare != nullptr)
  {
    if (_diag != nullptr)
    {
      (*_diag) << " \nFARE SELECTED: \n";
      displayFare(*selectedFare);
      (*_diag) << " \n";
    }

    return selectedFare;
  }

  return selectedFare;
}

const PaxTypeFare*
MinFareNormalFareSelection::selectFareForHigherCabin(const std::vector<TravelSeg*>& travelSegs)
{
  const PaxTypeFare* selectedFare = nullptr;

  bool selectLowestInPE = _selHighestInHigherCabin; // Select lowest in premium economy
  // Cabin is economy class bump to only premium economy -- add code to bump only to premium economy
  // This DB column name is now changed to lowest in premium economy cabin
  if (selectLowestInPE)
    selectedFare = selectFareForCabin(travelSegs, _cabin.minFaresHigherCabin(), selectLowestInPE);
  return selectedFare;
}

const PaxTypeFare*
MinFareNormalFareSelection::selectFareForLowerCabin(const std::vector<TravelSeg*>& travelSegs)
{
  const PaxTypeFare* selectedFare = nullptr;
  bool selectLowest = true;
  switch (_module)
  {
  case HIP:
  case BHC:
  case CTM:
    // If the fare market is non-economy, select the highest fare from next
    // Lower cabin
    selectLowest = false;
    break;

  case COM:
  case DMC:
  case CPM:
  case OJM:
  case COP:
  case OSC:
  case RSC:

    // If the fare market is non-economy, select the lowest fare from next
    // Lower cabin
    break;

  default:
    break;
  }

  CabinType modifiedCabin;
  for (modifiedCabin = _cabin; selectedFare == nullptr && getLowerCabin(modifiedCabin);)
  {
    selectedFare = selectFareForCabin(travelSegs, modifiedCabin, selectLowest);
  }
  return selectedFare;
}

bool
MinFareNormalFareSelection::getLowerCabin(CabinType& modifiedCabin)
{

  switch (modifiedCabin.getCabinIndicator())
  {
  case CabinType::FIRST_CLASS_PREMIUM:
  case CabinType::FIRST_CLASS:
  case CabinType::BUSINESS_CLASS_PREMIUM:
    break;
  case CabinType::BUSINESS_CLASS:
  case CabinType::ECONOMY_CLASS_PREMIUM:
    // Do not Bump to Premium Economy or Economy if Original is First or Premium First
    if (_cabin.getCabinIndicator() <= CabinType::FIRST_CLASS)
      return false;
    break;
  default:
    return false;
  }

  modifiedCabin = modifiedCabin.minFaresLowerCabin();
  return true;
}

const PaxTypeFare*
MinFareNormalFareSelection::selectGovCxrValidatedFare(const PaxTypeFareSet& govCxrValidatedFares,
                                                      const PaxTypeFareSet& govCxrNonValidatedFares,
                                                      bool selectLowest,
                                                      bool forSameCabin)
{
  if (govCxrValidatedFares.size() < 1) // If there is no validated normal fare
  {
    return nullptr;
  }
  else if (govCxrValidatedFares.size() == 1) // If there is only one validated normal fare
  {
    if (passRuleLevelExclusion(**govCxrValidatedFares.begin()))
      return *govCxrValidatedFares.begin();
  }
  else // If there is multiple validated normal fare
  {
    return selectValidatedFare(govCxrValidatedFares, selectLowest, forSameCabin);
  }

  return nullptr;
}

// Identify validated and nonValidated normal fares
void
MinFareNormalFareSelection::identifyValidatedFares(const std::vector<PaxTypeFare*>& fares,
                                                   const CabinType cabin,
                                                   bool yyAllowed)
{
  std::vector<PaxTypeFare*>::const_iterator iter = fares.begin();
  for (; iter != fares.end(); iter++)
  {
    PaxTypeFare& curFare = **iter;

    if (LIKELY(!_yyOverride))
    {
      // Depending on GovCxr to allow YY or not
      if (!yyAllowed && (curFare.fare()->isIndustry()))
        continue;
    }

    if (curFare.isNormal() && (curFare.cabin() == cabin) && curFare.isGlobalDirectionValid())
    {
      // Only display normal fares in the this cabin,
      if (UNLIKELY(_diag != nullptr))
        displayFare(curFare);

      if (isRightPaxType(curFare) && validateCat25Cat35(curFare) && isEligible(curFare) &&
          isRightDirection(curFare))
      {
        ValidStatus status = validateFare(curFare);
        switch (status)
        {
        case VALIDATED:
        {
          if (curFare.carrier() == INDUSTRY_CARRIER)
            _yyValidatedFares.insert(&curFare);
          else
            _govCxrValidatedFares.insert(&curFare);

          if (_diag != nullptr)
            (*_diag) << " V";
        }
        break;
        case NON_VALIDATED:
        {
          if (curFare.carrier() == INDUSTRY_CARRIER)
            _yyNonValidatedFares.insert(&curFare);
          else
            _govCxrNonValidatedFares.insert(&curFare);

          if (_diag != nullptr)
            (*_diag) << " N";
        }
        break;
        default:
          break;
        }
      }

      if (UNLIKELY(_diag != nullptr))
        (*_diag) << " \n";
    }
  }
}

const PaxTypeFare*
MinFareNormalFareSelection::selectValidatedFare(const PaxTypeFareSet& validatedFares,
                                                bool selectLowest,
                                                bool forSameCabin)
{
  if (!forSameCabin) // Diff cabin, do not check Appl Logic
    return selectWithinAllFare(validatedFares, selectLowest);

  // Use Minimum Fare Application Overwrite Logic or Default Logic
  // to select a fare.
  const PaxTypeFare* retFare = nullptr;

  if (isAllSameType(validatedFares)) // All fares are same fare type.
  {
    // Use selection priority for MPM and Routing fares.(For HIP/BHC/CTM)

    if (_selMpmBeforeRouting)
    {
      retFare = selectMpmFare(validatedFares);
      if (retFare != nullptr)
        return retFare;

      retFare = selectRoutingFare(validatedFares);
    }
    else if (_selRoutingBeforeMpm)
    {
      retFare = selectRoutingFare(validatedFares);
      if (retFare != nullptr)
        return retFare;

      retFare = selectMpmFare(validatedFares);
    }
    else
    {
      retFare = selectWithinAllFare(validatedFares, selectLowest);
    }
  }
  else // Mixed fare types
  {
    if (_selSameFareType || _selSameRbd || _compStopOver ||
        _selTariffCat != MinimumFare::BLANK_TRF_CAT)
    {
      retFare = selectFareByLogicPref(validatedFares);
    }
    else
    {
      retFare = selectWithinAllFare(validatedFares, selectLowest);
    }
  }

  return retFare;
}

const PaxTypeFare*
MinFareNormalFareSelection::selectWithinAllFare(const PaxTypeFareSet& validatedFares,
                                                bool selectLowest)
{
  if (selectLowest) // Select lowest
  {
    PaxTypeFareSet::const_iterator iter = validatedFares.begin();
    for (; iter != validatedFares.end(); iter++)
    {
      const PaxTypeFare* paxTypeFare = *iter;
      if (_sameFareType && // Currently only OSC and COP have this need.
          (paxTypeFare->fcaFareType() != _preferedFareType))
        continue;

      if (passRuleLevelExclusion(*paxTypeFare))
        return paxTypeFare;
    }

    // No prefered fare type available, select again
    iter = validatedFares.begin();
    if (_sameFareType)
    {
      for (; iter != validatedFares.end(); iter++)
      {
        const PaxTypeFare* paxTypeFare = *iter;
        if (passRuleLevelExclusion(*paxTypeFare))
          return paxTypeFare;
      }
    }
  }
  else // Select Highest
  {
    PaxTypeFareSet::const_reverse_iterator iter = validatedFares.rbegin();
    for (; iter != validatedFares.rend(); iter++)
    {
      const PaxTypeFare* paxTypeFare = *iter;
      if (_sameFareType && (paxTypeFare->fcaFareType() != _preferedFareType))
        continue;

      if (passRuleLevelExclusion(*paxTypeFare))
        return paxTypeFare;
    }

    // No prefered fare type available, select again
    iter = validatedFares.rbegin();
    for (; iter != validatedFares.rend(); iter++)
    {
      const PaxTypeFare* paxTypeFare = *iter;
      if (passRuleLevelExclusion(*paxTypeFare))
        return paxTypeFare;
    }
  }
  return nullptr;
}

const PaxTypeFare*
MinFareNormalFareSelection::selectMpmFare(const PaxTypeFareSet& validatedFares)
{
  PaxTypeFareSet::const_iterator iter = validatedFares.begin();
  for (; iter != validatedFares.end(); iter++)
  {
    const PaxTypeFare* paxTypeFare = *iter;
    if (!paxTypeFare->isRouting())
    {
      if (passRuleLevelExclusion(*paxTypeFare))
        return paxTypeFare;
    }
  }

  return nullptr;
}

const PaxTypeFare*
MinFareNormalFareSelection::selectRoutingFare(const PaxTypeFareSet& validatedFares)
{
  PaxTypeFareSet::const_iterator iter = validatedFares.begin();
  for (; iter != validatedFares.end(); iter++)
  {
    const PaxTypeFare* paxTypeFare = *iter;
    if (paxTypeFare->isRouting())
    {
      if (passRuleLevelExclusion(*paxTypeFare))
        return paxTypeFare;
    }
  }
  return nullptr;
}

const PaxTypeFare*
MinFareNormalFareSelection::selectFareByLogicPref(const PaxTypeFareSet& validatedFares)
{
  ApplLogicOrder applLogicOrder(_thruFare, _selSameRbd, _compStopOver);
  FareApplLogicSet fareApplLogicSet(applLogicOrder);

  PaxTypeFareSet::const_iterator i = validatedFares.begin();
  for (; i != validatedFares.end(); i++)
  {
    const PaxTypeFare* curFare = *i;

    // Public vs. Private
    if ((_selTariffCat != MinimumFare::BLANK_TRF_CAT) && (_selTariffCat != curFare->tcrTariffCat()))
      continue;

    // ANY vs Same Fare Type
    if ((_selSameFareType || _selSameRbd) && (curFare->fcaFareType() != _thruFare->fcaFareType()))
      continue;

    // Sort by RBD and StopOver
    fareApplLogicSet.insert(curFare);
  }

  FareApplLogicSet::const_iterator iter = fareApplLogicSet.begin();
  for (; iter != fareApplLogicSet.end(); iter++)
  {
    const PaxTypeFare* paxTypeFare = *iter;
    if (passRuleLevelExclusion(*paxTypeFare))
      return paxTypeFare;
  }

  return nullptr;
}

void
MinFareNormalFareSelection::getDefaultLogic()
{
  if (_minFareDefaultLogic == nullptr)
    throw ErrorResponseException(ErrorResponseException::MIN_FARE_MISSING_DATA,
                                 MinimumFare::MISSING_DATA_ERROR_MSG.c_str());

  // Highest fare in higher cabin
  _selHighestInHigherCabin = (_minFareDefaultLogic->nmlFareCompareInd() == MinimumFare::YES);

  // MPM vs. Routing
  _selMpmBeforeRouting = (_minFareDefaultLogic->nmlMpmBeforeRtgInd() == MinimumFare::YES);
  _selRoutingBeforeMpm = (_minFareDefaultLogic->nmlRtgBeforeMpmInd() == MinimumFare::YES);

  switch (_module)
  {
  case HIP:
  case BHC:
    // Trariff Category
    _selTariffCat = _minFareDefaultLogic->nmlHipTariffCatInd();

    // Restr vs. Unrestr/SAMETYPE vs. ANYTYPE
    if ((_thruFare != nullptr) && _thruFare->isNormal())
    {
      const FareTypeMatrix* fareTypeMatrix =
          _trx.dataHandle().getFareTypeMatrix(_thruFare->fcaFareType(), _travelDate);
      if (fareTypeMatrix != nullptr)
      {
        if (fareTypeMatrix->restrInd() == RESTRICTED)
          _selSameFareType = (_minFareDefaultLogic->nmlHipRestrCompInd() == SAME_TYPE);
        else if (fareTypeMatrix->restrInd() == UNRESTRICTED)
          _selSameFareType = (_minFareDefaultLogic->nmlHipUnrestrCompInd() == SAME_TYPE);
      }

      _selSameRbd = (_minFareDefaultLogic->nmlHipRbdCompInd() == MinimumFare::YES);
    }

    _compStopOver = (_minFareDefaultLogic->nmlHipStopCompInd() == MinimumFare::YES);
    break;

  case CTM:
    // Trariff Category
    _selTariffCat = _minFareDefaultLogic->nmlCtmTariffCatInd();

    // Restr vs. Unrestr/SAMETYPE vs. ANYTYPE
    if (_thruFare != nullptr)
    {
      const FareTypeMatrix* fareTypeMatrix =
          _trx.dataHandle().getFareTypeMatrix(_thruFare->fcaFareType(), _travelDate);
      if (fareTypeMatrix != nullptr)
      {
        if (fareTypeMatrix->restrInd() == RESTRICTED)
          _selSameFareType = (_minFareDefaultLogic->nmlCtmRestrCompInd() == SAME_TYPE);
        else if (fareTypeMatrix->restrInd() == UNRESTRICTED)
          _selSameFareType = (_minFareDefaultLogic->nmlCtmUnrestrCompInd() == SAME_TYPE);
      }
    }

    _selSameRbd = (_minFareDefaultLogic->nmlCtmRbdCompInd() == MinimumFare::YES);
    _compStopOver = (_minFareDefaultLogic->nmlCtmStopCompInd() == MinimumFare::YES);
    break;

  default:
    break;
  }
}

void
MinFareNormalFareSelection::getOverrideLogic()
{
  // Highest fare in higher cabin
  _selHighestInHigherCabin = (_minFareAppl->nmlFareCompareInd() == MinimumFare::YES);

  // MPM vs. Routing
  _selMpmBeforeRouting = (_minFareAppl->nmlMpmBeforeRtgInd() == MinimumFare::YES);
  _selRoutingBeforeMpm = (_minFareAppl->nmlRtgBeforeMpmInd() == MinimumFare::YES);

  switch (_module)
  {
  case HIP:
  case BHC:

    // Trariff Category
    _selTariffCat = _minFareAppl->nmlHipTariffCatInd();

    // Restr vs. Unrestr/SAMETYPE vs. ANYTYPE
    if (_thruFare != nullptr)
    {
      const FareTypeMatrix* fareTypeMatrix =
          _trx.dataHandle().getFareTypeMatrix(_thruFare->fcaFareType(), _travelDate);
      if (fareTypeMatrix != nullptr)
      {
        if (fareTypeMatrix->restrInd() == RESTRICTED)
          _selSameFareType = (_minFareAppl->nmlHipRestrCompInd() == SAME_TYPE);
        else if (fareTypeMatrix->restrInd() == UNRESTRICTED)
          _selSameFareType = (_minFareAppl->nmlHipUnrestrCompInd() == SAME_TYPE);
      }
    }

    _selSameRbd = (_minFareAppl->nmlHipRbdCompInd() == MinimumFare::YES);
    _compStopOver = (_minFareAppl->nmlHipStopCompInd() == MinimumFare::YES);
    break;

  case CTM:
    // Trariff Category
    _selTariffCat = _minFareAppl->nmlCtmTariffCatInd();

    // Restr vs. Unrestr/SAMETYPE vs. ANYTYPE
    if (_thruFare != nullptr)
    {
      const FareTypeMatrix* fareTypeMatrix =
          _trx.dataHandle().getFareTypeMatrix(_thruFare->fcaFareType(), _travelDate);
      if (fareTypeMatrix != nullptr)
      {
        if (fareTypeMatrix->restrInd() == RESTRICTED)
          _selSameFareType = (_minFareAppl->nmlCtmRestrCompInd() == SAME_TYPE);
        else if (fareTypeMatrix->restrInd() == UNRESTRICTED)
          _selSameFareType = (_minFareAppl->nmlCtmUnrestrCompInd() == SAME_TYPE);
      }
    }

    _selSameRbd = (_minFareAppl->nmlCtmRbdCompInd() == MinimumFare::YES);
    _compStopOver = (_minFareAppl->nmlCtmStopCompInd() == MinimumFare::YES);
    break;

  default:
    break;
  }
}

/**
 * Pick the Highest fare amount from the Non-validated Governing Carrier Normal Fares.
 * Check the selected fare with Minimum Fare Rule Level Exclusion.
 * If the fare is not allowed for the fare comparison, select the next lower fare
 * amount until the selected fare passes Minimum Fare Rule Level Exclusion.
 */
const PaxTypeFare*
MinFareNormalFareSelection::selectNonValidatedFare(const PaxTypeFareSet& nonValidatedFares)
{
  if (nonValidatedFares.size() < 1)
    return nullptr;

  PaxTypeFareSet::const_reverse_iterator iter = nonValidatedFares.rbegin();
  for (; iter != nonValidatedFares.rend(); iter++)
  {
    const PaxTypeFare* paxTypeFare = *iter;
    if (_sameFareType && (paxTypeFare->fcaFareType() != _preferedFareType))
      continue;

    if (passRuleLevelExclusion(*paxTypeFare))
      return paxTypeFare;
  }

  // No prefered fare type available, select again
  iter = nonValidatedFares.rbegin();
  for (; iter != nonValidatedFares.rend(); iter++)
  {
    const PaxTypeFare* paxTypeFare = *iter;
    if (passRuleLevelExclusion(*paxTypeFare))
      return paxTypeFare;
  }

  return nullptr;
}

bool
MinFareNormalFareSelection::isAllSameType(const PaxTypeFareSet& fares)
{
  FareType fareType;
  bool fareTypeSet = false;

  PaxTypeFareSet::const_iterator i = fares.begin();
  for (; i != fares.end(); i++)
  {
    if (fareTypeSet)
    {
      if ((*i)->fcaFareType() != fareType)
        return false;
    }
    else
    {
      fareType = (*i)->fcaFareType();
      fareTypeSet = true;
    }
  }

  return true;
}

void
MinFareNormalFareSelection::displayFareSelectReq(const std::vector<TravelSeg*>& travelSegs,
                                                 CabinType cabin)
{
  (*_diag) << travelSegs.front()->boardMultiCity() << "-" << travelSegs.back()->offMultiCity()
           << " REQPAX/" << ((_paxType == nullptr) ? "   " : _paxType->paxType()) << " SELPAX/";

  switch (_selPaxTypeStatus)
  {
  case PAX_TYPE_STATUS_CHILD:
    (*_diag) << "CNN";
    break;
  case PAX_TYPE_STATUS_INFANT:
    (*_diag) << "INF";
    break;
  case PAX_TYPE_STATUS_ADULT:
  default:
    (*_diag) << "ADT";
    break;
  }

  (*_diag) << " CABIN/" << cabin << " DIR/" << FareDirStr[_fareDirection] << " RULESCOPE/";

  if (!_pricingUnits.empty() && (_farePath != nullptr))
    (*_diag) << "PU/"
             << DiagnosticUtil::pricingUnitTypeToShortString(_pricingUnits.front()->puType());
  else
    (*_diag) << "FM";

  if (_yyOverride) // WPC-YY entry
    (*_diag) << " \nC-YY";

  if (_exemptRuleVal) // for Net Remit MinFare check
    (*_diag) << " \nEXEMPT RULE VALIDATION";

  (*_diag) << " \n";

  if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX)
    MinimumFare::printRexDatesAdjustment(*_diag, _trx.dataHandle().ticketDate());

  if (_thruFare != nullptr)
  {
    (*_diag) << "THRU FARE " << _thruFare->fareMarket()->boardMultiCity() << "-"
             << _thruFare->fareMarket()->offMultiCity() << " \n";
    displayFare(*_thruFare);
    (*_diag) << " \n";
  }
}

void
MinFareNormalFareSelection::displayMinFareApplLogic()
{
  if (_minFareAppl == nullptr)
    return;

  (*_diag) << " \nMIN FARE APPL SEQ# " << _minFareAppl->seqNo() << " \n  APPLY DEFAULT - "
           << _minFareAppl->applyDefaultLogic();

  if ((_minFareAppl->applyDefaultLogic() == MinimumFare::YES))
  {
    if (_minFareDefaultLogic == nullptr)
      return;

    (*_diag) << "  DEFAULT LOGIC SEQ# " << _minFareDefaultLogic->seqNo();

    (*_diag) << " \n  COMPARE THE LOWEST FARE IN PREMIUM ECONOMY CABIN - ";

    (*_diag) << (_minFareDefaultLogic->nmlFareCompareInd() == MinimumFare::YES ? "Y" : "N")
             << " \n  APPLY MPM BEFORE ROUTING - "
             << (_minFareDefaultLogic->nmlMpmBeforeRtgInd() == MinimumFare::YES ? "Y" : "N")
             << " \n  APPLY ROUTING BEFORE MPM - "
             << (_minFareDefaultLogic->nmlRtgBeforeMpmInd() == MinimumFare::YES ? "Y" : "N");
    switch (_module)
    {
    case HIP:
    case BHC:
      (*_diag) << " \n  RESTRICTED - "
               << (_minFareDefaultLogic->nmlHipRestrCompInd() == SAME_TYPE ? "SAME TYPE"
                                                                           : "ANY TYPE")
               << " \n  UNRESTRICTED - "
               << (_minFareDefaultLogic->nmlHipUnrestrCompInd() == SAME_TYPE ? "SAME TYPE"
                                                                             : "ANY TYPE")
               << " \n  RBD COMPARE REQUIRED - "
               << (_minFareDefaultLogic->nmlHipRbdCompInd() == MinimumFare::YES ? "Y" : "N")
               << " \n  STOPOVER COMPARE REQUIRED - "
               << (_minFareDefaultLogic->nmlHipStopCompInd() == MinimumFare::YES ? "Y" : "N")
               << " \n  TARIFF CATEGORY - " << _minFareDefaultLogic->nmlHipTariffCatInd();
      break;
    case CTM:
      (*_diag) << " \n  RESTRICTED - "
               << (_minFareDefaultLogic->nmlCtmRestrCompInd() == SAME_TYPE ? "SAME TYPE"
                                                                           : "ANY TYPE")
               << " \n  UNRESTRICTED - "
               << (_minFareDefaultLogic->nmlCtmUnrestrCompInd() == SAME_TYPE ? "SAME TYPE"
                                                                             : "ANY TYPE")
               << " \n  RBD COMPARE REQUIRED - "
               << (_minFareDefaultLogic->nmlCtmRbdCompInd() == MinimumFare::YES ? "Y" : "N")
               << " \n  STOPOVER COMPARE REQUIRED - "
               << (_minFareDefaultLogic->nmlCtmStopCompInd() == MinimumFare::YES ? "Y" : "N")
               << " \n  TARIFF CATEGORY - " << _minFareDefaultLogic->nmlCtmTariffCatInd();
      break;
    default:
      break;
    }

    (*_diag) << " \n\n";
  }
  else
  {
    (*_diag) << " \n  COMPARE THE LOWEST FARE IN PREMIUM ECONOMY CABIN - ";

    (*_diag) << (_minFareAppl->nmlFareCompareInd() == MinimumFare::YES ? "Y" : "N")
             << " \n  APPLY MPM BEFORE ROUTING - "
             << (_minFareAppl->nmlMpmBeforeRtgInd() == MinimumFare::YES ? "Y" : "N")
             << " \n  APPLY ROUTING BEFORE MPM - "
             << (_minFareAppl->nmlRtgBeforeMpmInd() == MinimumFare::YES ? "Y" : "N");

    switch (_module)
    {
    case HIP:
    case BHC:
      (*_diag) << " \n  RESTRICTED - "
               << (_minFareAppl->nmlHipRestrCompInd() == SAME_TYPE ? "SAME TYPE" : "ANY TYPE")
               << " \n  UNRESTRICTED - "
               << (_minFareAppl->nmlHipUnrestrCompInd() == SAME_TYPE ? "SAME TYPE" : "ANY TYPE")
               << " \n  RBD COMPARE REQUIRED - "
               << (_minFareAppl->nmlHipRbdCompInd() == MinimumFare::YES ? "Y" : "N")
               << " \n  STOPOVER COMPARE REQUIRED - "
               << (_minFareAppl->nmlHipStopCompInd() == MinimumFare::YES ? "Y" : "N")
               << " \n  TARIFF CATEGORY - " << _minFareAppl->nmlHipTariffCatInd();
      break;
    case CTM:
      (*_diag) << " \n  RESTRICTED - "
               << (_minFareAppl->nmlCtmRestrCompInd() == SAME_TYPE ? "SAME TYPE" : "ANY TYPE")
               << " \n  UNRESTRICTED - "
               << (_minFareAppl->nmlCtmUnrestrCompInd() == SAME_TYPE ? "SAME TYPE" : "ANY TYPE")
               << " \n  RBD COMPARE REQUIRED - "
               << (_minFareAppl->nmlCtmRbdCompInd() == MinimumFare::YES ? "Y" : "N")
               << " \n  STOPOVER COMPARE REQUIRED - "
               << (_minFareAppl->nmlCtmStopCompInd() == MinimumFare::YES ? "Y" : "N")
               << " \n  TARIFF CATEGORY - " << _minFareAppl->nmlCtmTariffCatInd();
      break;
    default:
      break;
    }

    (*_diag) << " \n \n";
  }
}

class MatchPaxType : public std::binary_function<const PaxTypeFare*, PaxTypeCode, bool>
{
public:
  bool operator()(const PaxTypeFare* ptf, const PaxTypeCode& paxType) const
  {
    return (ptf->fcasPaxType().empty() || ptf->fcasPaxType() == paxType);
  }
}; // lint !e1509

/**
 *
 */
PtfPair
MinFareNormalFareSelection::newSelectFare(PaxTypeStatus selPaxTypeStatus, bool selectNextCabin)
{
  PtfPair ptfPair;
  ptfPair.first = ptfPair.second = nullptr;

  for (;; selPaxTypeStatus = PaxTypeUtil::nextPaxTypeStatus(selPaxTypeStatus))
  {
    ptfPair.first = selectFare(_travelSegs, selPaxTypeStatus, selectNextCabin);
    if (ptfPair.first)
    {
      break;
    }
    else
    {
      GlobalDirection globalDir;

      GlobalDirectionFinderV2Adapter::getGlobalDirection(
          &_trx, _travelDate, _travelSegs, globalDir);

      if (globalDir == GlobalDirection::XX) // Invalid global direction, do not construct the market
      {
        if (_diag != nullptr)
        {
          (*_diag) << " \nINVALID GI - NO FARE CONSTRUCTION\n";
          _diag->flushMsg();
        }
        break; // No need to try next PaxTypeStatus
      }
    }

    if (selPaxTypeStatus == PAX_TYPE_STATUS_UNKNOWN || selPaxTypeStatus == PAX_TYPE_STATUS_ADULT)
    {
      break;
    }
  }

  return ptfPair;
}

const PaxTypeFare*
MinFareNormalFareSelection::selectNormalFareForConstruction(
    const std::vector<TravelSeg*>& travelSegs, PaxTypeStatus selPaxTypeStatus, bool selectNextCabin)
{
  return selectFare(travelSegs, selPaxTypeStatus, selectNextCabin);
}
}
