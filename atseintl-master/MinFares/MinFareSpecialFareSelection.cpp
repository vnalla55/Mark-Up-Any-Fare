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
#include "MinFares/MinFareSpecialFareSelection.h"

#include "Common/FareMarketUtil.h"
#include "Common/Global.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/GoverningCarrier.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/TseUtil.h"
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
#include "DBAccess/MinFareFareTypeGrp.h"
#include "DBAccess/MinFareFareTypeGrpSeg.h"
#include "DBAccess/MinFareRuleLevelExcl.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "MinFares/MinimumFare.h"

#include <algorithm>
#include <iomanip>

namespace tse
{
static Logger
logger("atseintl.MinFares.MinFareSpecialFareSelection");

MinFareSpecialFareSelection::MinFareSpecialFareSelection(
    MinimumFareModule module,
    EligibleFare eligibleFare,
    FareDirectionChoice fareDirection,
    const FareType& fareType,
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
    _fareType(fareType)
{
  if (thruFare != nullptr)
  {
    _cabin = thruFare->cabin();
    _isProm = thruFare->fareTypeDesignator().isFTDPromotional();
  }
  _curFareTypeVec.push_back(fareType);
}

const PaxTypeFare*
MinFareSpecialFareSelection::selectFare(
    PaxTypeStatus selPaxTypeStatus /*= PAX_TYPE_STATUS_UNKNOWN*/, bool selNormalFare /*= false*/)
{
  return selectFare(_travelSegs, selPaxTypeStatus, selNormalFare);
}

const PaxTypeFare*
MinFareSpecialFareSelection::selectFare(
    const std::vector<TravelSeg*>& travelSegs,
    PaxTypeStatus selPaxTypeStatus /*= PAX_TYPE_STATUS_UNKNOWN*/,
    bool selNormalFare /*= false*/)
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
    (*_diag) << " \n****************" << MinFareModuleName[_module]
             << (_isProm ? " PROMOTIONAL " : " SPECIAL ") << "FARE SELECTION*******************\n";

    displayFareSelectReq(travelSegs);
    displayMinFareApplLogic();
  }

  // Check GovCxr override
  _govCxrOverrides.clear();
  FareMarketUtil::getGovCxrOverride(_trx, travelSegs, _govCxrOverrides);

  // Look fare in specified GovCxr fare market first
  if (!_yyOverride && !_govCxrOverrides.empty())
  {
    std::vector<CarrierCode>::iterator govCxrOverrideIter = _govCxrOverrides.begin();
    for (; (selectedFare == nullptr) && (govCxrOverrideIter != _govCxrOverrides.end());
         ++govCxrOverrideIter)
    {
      _govCxr = *govCxrOverrideIter;

      // Find fare with next higher special fare type by Promotional Group
      // Designator Process.
      _curFareTypeVec.assign(1, _fareType);

      selectedFare = processHigherFareType(travelSegs);
    }
  }

  // Try fare markets with normal GovCxr logic
  if (selectedFare == nullptr)
  {
    _govCxr = "";

    // Find fare with next higher special fare type by Promotional Group
    // Designator Process.
    _curFareTypeVec.assign(1, _fareType);

    selectedFare = processHigherFareType(travelSegs);
  }

  if (_diag != nullptr)
  {
    if (selectedFare == nullptr)
    {
      (*_diag) << " \nNO FARE SELECTED\n";
    }
    else
    {
      (*_diag) << " \nFARE SELECTED:\n";
      displayFare(*selectedFare);
      (*_diag) << " \n";
    }
    _diag->flushMsg();
  }

  LOG4CXX_DEBUG(logger, "Exit selectFare");

  return selectedFare;
}

const PaxTypeFare*
MinFareSpecialFareSelection::selectNormalFare(
    const std::vector<TravelSeg*>& travelSegs,
    PaxTypeStatus selPaxTypeStatus /*= PAX_TYPE_STATUS_UNKNOWN*/)
{
  LOG4CXX_DEBUG(logger, "Enter selectNormalFare");

  _selPaxTypeStatus =
      (selPaxTypeStatus == PAX_TYPE_STATUS_UNKNOWN) ? _paxTypeStatus : selPaxTypeStatus;

  const PaxTypeFare* selectedFare = nullptr;

  if (_diag != nullptr)
  {
    (*_diag) << " \n****************" << MinFareModuleName[_module]
             << (_isProm ? " PROMOTIONAL " : " SPECIAL ") << "FARE SELECTION*******************\n";

    displayFareSelectReq(travelSegs);
  }

  // Select lowest validated Normal fare within the same cabin.
  selectedFare = selectNormalCabinFare(travelSegs);

  LOG4CXX_DEBUG(logger, "Exit selectNormalFare");

  return selectedFare;
}

void
MinFareSpecialFareSelection::displayFareSelectReq(const std::vector<TravelSeg*>& travelSegs)
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

  (*_diag) << " FT/" << _fareType << " DIR/" << FareDirStr[_fareDirection] << " RULESCOPE/";

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
MinFareSpecialFareSelection::displayMinFareApplLogic()
{
  if (_minFareAppl == nullptr)
    return;

  (*_diag) << " \nMIN FARE APPL SEQ# " << _minFareAppl->seqNo() << " \n  APPLY DEFAULT - "
           << _minFareAppl->applyDefaultLogic();

  if (_minFareAppl->applyDefaultLogic() == MinimumFare::YES)
  {
    if (_minFareDefaultLogic == nullptr)
      return;

    (*_diag) << "  DEFAULT LOGIC SEQ# " << _minFareDefaultLogic->seqNo();

    switch (_module)
    {
    case HIP:
    case BHC:
      (*_diag) << " \n  SAME TRF CAT - "
               << (_minFareDefaultLogic->spclHipTariffCatInd() == MinimumFare::YES ? "Y" : "N")
               << " \n  SAME RULE AND TRF - "
               << (_minFareDefaultLogic->spclHipRuleTrfInd() == MinimumFare::YES ? "Y" : "N")
               << " \n  SAME FARE CLS - "
               << (_minFareDefaultLogic->spclHipFareClassInd() == MinimumFare::YES ? "Y" : "N")
               << " \n  SAME 1ST CHAR OF FARE CLS - "
               << (_minFareDefaultLogic->spclHip1stCharInd() == MinimumFare::YES ? "Y" : "N")
               << " \n  STOP OVER COMP REQUIRED - "
               << (_minFareDefaultLogic->spclHipStopCompInd() == MinimumFare::YES ? "Y" : "N");
      break;

    case CTM:
      (*_diag) << " \n  SAME TRF CAT - "
               << (_minFareDefaultLogic->spclCtmTariffCatInd() == MinimumFare::YES ? "Y" : "N")
               << " \n  SAME RULE AND TRF - "
               << (_minFareDefaultLogic->spclCtmRuleTrfInd() == MinimumFare::YES ? "Y" : "N")
               << " \n  SAME FARE CLS - "
               << (_minFareDefaultLogic->spclCtmFareClassInd() == MinimumFare::YES ? "Y" : "N")
               << " \n  SAME 1ST CHAR OF FARE CLS - "
               << (_minFareDefaultLogic->spclSame1stCharFBInd2() == MinimumFare::YES ? "Y" : "N")
               << " \n  STOP OVER COMP REQUIRED - "
               << (_minFareDefaultLogic->spclCtmStopCompInd() == MinimumFare::YES ? "Y" : "N");
      break;

    default:
      break;
    }

    if (_isProm)
      (*_diag) << " \n  PROMOTIONAL PROCESS NAME - " << _minFareDefaultLogic->specialProcessName();

    (*_diag) << " \n\n";
  }
  else
  {
    switch (_module)
    {
    case HIP:
    case BHC:
      (*_diag) << " \n  SAME TRF CAT - "
               << (_minFareAppl->spclHipTariffCatInd() == MinimumFare::YES ? "Y" : "N")
               << " \n  SAME RULE AND TRF - "
               << (_minFareAppl->spclHipRuleTrfInd() == MinimumFare::YES ? "Y" : "N")
               << " \n  SAME FARE CLS - "
               << (_minFareAppl->spclHipFareClassInd() == MinimumFare::YES ? "Y" : "N")
               << " \n  SAME 1ST CHAR OF FARE CLS - "
               << (_minFareAppl->spclHip1stCharInd() == MinimumFare::YES ? "Y" : "N")
               << " \n  STOP OVER COMP REQUIRED - "
               << (_minFareAppl->spclHipStopCompInd() == MinimumFare::YES ? "Y" : "N");
      break;

    case CTM:
      (*_diag) << " \n  SAME TRF CAT - "
               << (_minFareAppl->spclCtmTariffCatInd() == MinimumFare::YES ? "Y" : "N")
               << " \n  SAME RULE AND TRF - "
               << (_minFareAppl->spclCtmRuleTrfInd() == MinimumFare::YES ? "Y" : "N")
               << " \n  SAME FARE CLS - "
               << (_minFareAppl->spclCtmFareClassInd() == MinimumFare::YES ? "Y" : "N")
               << " \n  SAME 1ST CHAR OF FARE CLS - "
               << (_minFareAppl->spclSame1stCharFBInd2() == MinimumFare::YES ? "Y" : "N")
               << " \n  STOP OVER COMP REQUIRED - "
               << (_minFareAppl->spclCtmStopCompInd() == MinimumFare::YES ? "Y" : "N");
      break;

    default:
      break;
    }

    if (_isProm)
      (*_diag) << " \n  PROMOTIONAL PROCESS NAME - " << _minFareAppl->specialProcessName();

    (*_diag) << " \n \n";
  }
}

const PaxTypeFare*
MinFareSpecialFareSelection::selectSpecialFare(const std::vector<TravelSeg*>& travelSegs)
{
  const PaxTypeFare* selectedFare = nullptr;

  bool matchedApprec = matchedApplRecord();
  getFareMarket(travelSegs, true);

  // Select validated GovCxr fare
  selectedFare = selectGovCxrValidatedFare(matchedApprec);
  if (selectedFare != nullptr)
    return selectedFare;

  // Select validated YY fare
  return selectValidatedFare(_yyValidatedFares, matchedApprec);
}

const PaxTypeFare*
MinFareSpecialFareSelection::selectNormalCabinFare(const std::vector<TravelSeg*>& travelSegs)
{
  const PaxTypeFare* selectedFare = nullptr;

  getFareMarket(travelSegs, false);

  // Select validated GovCxr fare
  selectedFare = selectGovCxrValidatedFare(false);
  if (selectedFare != nullptr)
    return selectedFare;

  // Select validated YY fare
  selectedFare = selectValidatedFare(_yyValidatedFares, false);

  return selectedFare;
}

void
MinFareSpecialFareSelection::getFareMarket(const std::vector<TravelSeg*>& travelSegs, bool forSpcl)
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
    (*_diag) << " \n" << travelSegs.front()->boardMultiCity() << "-"
             << travelSegs.back()->offMultiCity() << " PAX/";

    if (!_actualPaxType.empty())
      (*_diag) << _actualPaxType;
    else
      (*_diag) << ((_paxType == nullptr) ? "   " : _paxType->paxType());

    if (forSpcl)
    {
      (*_diag) << " FARETYPE";
      std::vector<FareType>::iterator it = _curFareTypeVec.begin();
      std::vector<FareType>::iterator ie = _curFareTypeVec.end();
      for (; it != ie; it++)
        (*_diag) << "/" << *it;
    }
    else
      (*_diag) << " CABIN/" << _cabin;

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

    // Special code for CTM to get fare market without matching all travel segments
    // as long as start and end segments are same.
    if ((_module == CTM) &&
        ((fareMarkets.size() < 1) ||
         (fareMarkets.front()->paxTypeCortege(_paxType) == nullptr))) // No market or market has no fares
    {
      fareMarkets.clear(); // Remove the one with no fares
      getAlternateFareMarket(travelSegs, fareMarkets);
    }

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

  if (!_govCxr.empty()) // Look fare in specified GovCxr fare market first
  {
    std::vector<FareMarket*>::iterator fmIter = fareMarkets.begin();
    for (; fmIter != fareMarkets.end(); ++fmIter)
    {
      FareMarket& fareMarket = **fmIter;
      if (fareMarket.governingCarrier() == _govCxr)
      {
        processFareMarket(fareMarket, forSpcl);
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
          processFareMarket(fareMarket, forSpcl);
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
          processFareMarket(fareMarket, forSpcl);
        }
      }
    }
  }
}

void
MinFareSpecialFareSelection::processFareMarket(const FareMarket& fareMarket, bool forSpcl)
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
    identifyValidatedFares(fares, yyAllowed, forSpcl);
  }
  else // Only HIP uses _actualPaxType for selecting fare for Double discounted child fare
  {
    const std::vector<PaxTypeFare*>& fares = fareMarket.allPaxTypeFare();
    identifyValidatedFares(fares, yyAllowed, forSpcl);
  }
}

void
MinFareSpecialFareSelection::identifyValidatedFares(const std::vector<PaxTypeFare*>& fares,
                                                    bool yyAllowed,
                                                    bool forSpcl)
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

    if (UNLIKELY(!curFare.isGlobalDirectionValid()))
      continue;

    if ((forSpcl && matchSpecial(curFare) && matchFareType(curFare.fcaFareType())) ||
        (!forSpcl && curFare.isNormal() && (curFare.cabin() == _cabin)))
    {
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

          if (UNLIKELY(_diag != nullptr))
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
MinFareSpecialFareSelection::selectGovCxrValidatedFare(bool matchApplRec)
{
  if (_govCxrValidatedFares.size() < 1) // If there is no validated fare
  {
    return nullptr;
  }
  else if (_govCxrValidatedFares.size() == 1) // If there is only one validated fare
  {
    const PaxTypeFare* fare = *_govCxrValidatedFares.begin();
    if (passRuleLevelExclusion(*fare))
      return fare;
    else
      return nullptr;
  }
  else // If there is multiple validated normal fare
  {
    return selectValidatedFare(_govCxrValidatedFares, matchApplRec);
  }

  return nullptr;
}

const PaxTypeFare*
MinFareSpecialFareSelection::selectValidatedFare(const PaxTypeFareSet& validatedFares,
                                                 bool matchApplRec)
{
  int32_t numFares = validatedFares.size();

  if (numFares < 1)
    return nullptr;

  if (numFares == 1 || !matchApplRec)
    return selectFareByRuleLevelExcl(validatedFares);

  if (!_sameTariffCat && !_sameRuleTariff && !_sameFareClass && !_same1charFareBasis &&
      !_stopOverComp)
    return selectFareByRuleLevelExcl(validatedFares);

  return selectFareByLogicPref(validatedFares);
}

const PaxTypeFare*
MinFareSpecialFareSelection::selectFareByLogicPref(const PaxTypeFareSet& validatedFares)
{
  ApplLogicOrder applLogicOrder(_thruFare, _same1charFareBasis, _stopOverComp);
  FareApplLogicSet fareApplLogicSet(applLogicOrder);

  PaxTypeFareSet::const_iterator i = validatedFares.begin();
  for (; i != validatedFares.end(); i++)
  {
    const PaxTypeFare* curFare = *i;

    // Public vs. Private
    if ((_sameTariffCat && curFare->tcrTariffCat() != _thruFare->tcrTariffCat()) ||
        (_sameRuleTariff && (curFare->tcrRuleTariff() != _thruFare->tcrRuleTariff() ||
                             curFare->ruleNumber() != _thruFare->ruleNumber())) ||
        (_sameFareClass && curFare->fareClass() != _thruFare->fareClass()))
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

bool
MinFareSpecialFareSelection::matchSpecial(const PaxTypeFare& paxTypeFare)
{
  return paxTypeFare.isSpecial();
}

void
MinFareSpecialFareSelection::getDefaultLogic()
{
  if (_minFareDefaultLogic == nullptr)
    throw ErrorResponseException(ErrorResponseException::MIN_FARE_MISSING_DATA,
                                 MinimumFare::MISSING_DATA_ERROR_MSG.c_str());

  switch (_module)
  {
  case HIP:
  case BHC:
    if (_minFareDefaultLogic->spclHipTariffCatInd() == MinimumFare::YES)
      _sameTariffCat = true;
    if (_minFareDefaultLogic->spclHipRuleTrfInd() == MinimumFare::YES)
      _sameRuleTariff = true;
    if (_minFareDefaultLogic->spclHipFareClassInd() == MinimumFare::YES)
      _sameFareClass = true;
    if (_minFareDefaultLogic->spclHip1stCharInd() == MinimumFare::YES)
      _same1charFareBasis = true;
    if (_minFareDefaultLogic->spclHipStopCompInd() == MinimumFare::YES)
      _stopOverComp = true;
    break;

  case CTM:
    if (_minFareDefaultLogic->spclCtmTariffCatInd() == MinimumFare::YES)
      _sameTariffCat = true;
    if (_minFareDefaultLogic->spclCtmRuleTrfInd() == MinimumFare::YES)
      _sameRuleTariff = true;
    if (_minFareDefaultLogic->spclCtmFareClassInd() == MinimumFare::YES)
      _sameFareClass = true;
    if (_minFareDefaultLogic->spclSame1stCharFBInd2() == MinimumFare::YES)
      _same1charFareBasis = true;
    if (_minFareDefaultLogic->spclCtmStopCompInd() == MinimumFare::YES)
      _stopOverComp = true;
    break;

  default:
    break;
  }

  _spclProcName = _minFareDefaultLogic->specialProcessName();
}

void
MinFareSpecialFareSelection::getOverrideLogic()
{
  if (!_minFareAppl)
    return;

  switch (_module)
  {
  case HIP:
    if (_minFareAppl->spclHipTariffCatInd() == MinimumFare::YES)
      _sameTariffCat = true;
    if (_minFareAppl->spclHipRuleTrfInd() == MinimumFare::YES)
      _sameRuleTariff = true;
    if (_minFareAppl->spclHipFareClassInd() == MinimumFare::YES)
      _sameFareClass = true;
    if (_minFareAppl->spclHip1stCharInd() == MinimumFare::YES)
      _same1charFareBasis = true;
    if (_minFareAppl->spclHipStopCompInd() == MinimumFare::YES)
      _stopOverComp = true;
    break;

  case CTM:
    if (_minFareAppl->spclCtmTariffCatInd() == MinimumFare::YES)
      _sameTariffCat = true;
    if (_minFareAppl->spclCtmRuleTrfInd() == MinimumFare::YES)
      _sameRuleTariff = true;
    if (_minFareAppl->spclCtmFareClassInd() == MinimumFare::YES)
      _sameFareClass = true;
    if (_minFareAppl->spclSame1stCharFBInd2() == MinimumFare::YES)
      _same1charFareBasis = true;
    if (_minFareAppl->spclCtmStopCompInd() == MinimumFare::YES)
      _stopOverComp = true;
    break;

  default:
    break;
  }

  _spclProcName = _minFareAppl->specialProcessName();
}

PtfPair
MinFareSpecialFareSelection::newSelectFare(PaxTypeStatus selPaxTypeStatus)
{
  PtfPair ptfPair;
  ptfPair.first = ptfPair.second = nullptr;

  for (;; selPaxTypeStatus = PaxTypeUtil::nextPaxTypeStatus(selPaxTypeStatus))
  {
    ptfPair.first = selectFare(selPaxTypeStatus);
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
MinFareSpecialFareSelection::selectNormalFareForConstruction(
    const std::vector<TravelSeg*>& travelSegs, PaxTypeStatus selPaxTypeStatus, bool selectNextCabin)
{
  // ignore selectNextCabin for special fare
  return selectNormalFare(travelSegs, selPaxTypeStatus);
}

// ----------------------------------------------------------------------------
//
// @function const PaxTypeFare* MinFareSpecialFareSelection::processHigherFareType
//
// Description:
//
//  Function is used to get list of fare types for special fare from
//    new MINFARESPECIALLFARTYPE table, and using these fare types trye to find
//    lowest valid fare which can be used.
//
// @param  travelSegs         - vector of travel segments for which we try to find valid
//                              PaxTypeFare
// @return lowest valid PaxTypeFare if match is found, otherwise 0
//
// ----------------------------------------------------------------------------
const PaxTypeFare*
MinFareSpecialFareSelection::processHigherFareType(const std::vector<TravelSeg*>& travelSegs)
{
  const PaxTypeFare* selectedFare = nullptr;
  // initial selected setNo set to -1, this will be changed by call to higherFareType
  int32_t setNo = -1;

  // first try to match on current fare type
  _curFareTypeVec.assign(1, _fareType);
  selectedFare = selectSpecialFare(travelSegs);

  // if no match on current fare type, then in loop get the vectors of FareTypes
  while ((selectedFare == nullptr) && higherFareType(_fareType, setNo))
  {
    if (!_curFareTypeVec.empty())
      selectedFare = selectSpecialFare(travelSegs);
  }
  return selectedFare;
}
const MinFareFareTypeGrp*
MinFareSpecialFareSelection::getMinFareFareTypeGrp()
{
  if (_spclProcName.empty())
    return nullptr;

  // taken out, so it can be override by test class
  return _trx.dataHandle().getMinFareFareTypeGrp(_spclProcName, _travelDate);
}
const FareTypeMatrix*
MinFareSpecialFareSelection::getFareTypeMatrix(const FareType& fareType, const DateTime& date)
{
  return _trx.dataHandle().getFareTypeMatrix(fareType, date);
}
// ----------------------------------------------------------------------------
//
// @function bool MinFareSpecialFareSelection::higherFareType
//
// Description:
//
//  Function is used to get vector of fare types from new MINFARESPECIALLFARTYPE table
//    logic is to:
//    - when called first time:
//     1) find fare type, and save grpSetNo and setNo
//     2) if far type is found, return all fare types from matched grpsetno/setno
//    - when called next time (workingSet is different then -1)
//     1) for matched grpsetno/setno for passed fare type, try to find next setno
//        within founded grpsetno
//     2) if next setno is found, return all fare types from this grpsetno/setno
//
// @param  fareType           - fare type being validated
// @param  workingSet         - setno, on which fare type was found, (or next setno,
//                              if no match was done on returned fare types)
//
// @return true if foun fare types
//
// ----------------------------------------------------------------------------
bool
MinFareSpecialFareSelection::higherFareType(const FareType& fareType, int32_t& workingSet)
{
  // prepare returning data
  _curFareTypeVec.clear();
  bool ret = false;

  // get minfaretype group
  const MinFareFareTypeGrp* fareTypeGroup = getMinFareFareTypeGrp();

  // if succedded get segmenst
  if (fareTypeGroup != nullptr)
  {
    // should display details
    bool printDiag = (_diag != nullptr) && (_trx.diagnostic().diagnosticType() == Diagnostic710) &&
                     (_trx.diagnostic().diagParamMapItem(Diagnostic::MISCELLANEOUS) == "ALL");
    const std::vector<MinFareFareTypeGrpSeg*>& fareTypeHierarchy = fareTypeGroup->segs();

    std::vector<MinFareFareTypeGrpSeg*>::const_iterator i = fareTypeHierarchy.begin();
    std::vector<MinFareFareTypeGrpSeg*>::const_iterator ie = fareTypeHierarchy.end();

    // will be set when fare type is found
    int32_t setNo = -1;
    int32_t grpSetNo = -1;

    // look for next, higher set
    if (workingSet >= 0)
      workingSet++;

    for (; i != ie; i++)
    {
      // check if still in correct group setno
      if ((grpSetNo != -1) && grpSetNo != (*i)->grpSetNo())
        break;

      // look for fare type and get setno and groupsetno
      if ((grpSetNo == -1) && fareType == (*i)->fareType())
      {
        setNo = (*i)->setNo();
        grpSetNo = (*i)->grpSetNo();
        if (printDiag && workingSet == -1)
        {
          (*_diag) << "MATCHED SPCL FARE TYPE " << (*i)->fareType()
                   << " ON GRPSETNO: " << (*i)->grpSetNo() << " SETNO: " << (*i)->setNo()
                   << std::endl;
        }
      }
      // if found match on fare type, then get correct setNo (current or next)
      if (setNo != -1)
      {
        // if first passing, save setNo
        if (workingSet < 0)
        {
          workingSet = setNo;
          break;
        }
        // try to find next setNo
        else
        {
          if (workingSet <= (*i)->setNo())
          {
            setNo = (*i)->setNo();
            workingSet = setNo;
            break;
          }
        }
      }
    }
    // if found setno
    if ((setNo != -1) && (workingSet == setNo))
    {
      for (i = fareTypeHierarchy.begin(); i != ie; i++)
      {
        // if finished with tghis group set no then done
        if ((*i)->grpSetNo() > grpSetNo)
          break;

        // is within the same setno/grpsetno, then get fare type
        if (((*i)->setNo() == setNo) && ((*i)->grpSetNo() == grpSetNo))
        {
          // marjk that some far types are found
          ret = true;
          if (_fareType != (*i)->fareType())
            _curFareTypeVec.push_back((*i)->fareType());
        }
      }
    }
    if (ret && printDiag)
      (*_diag) << " USING SPCL FARE TYPES FROM GRPSETNO: " << grpSetNo << " SETNO: " << setNo
               << std::endl;
  }
  return ret;
}
bool
MinFareSpecialFareSelection::matchedApplRecord()
{
  return (_curFareTypeVec.size() == 1) && (_curFareTypeVec.front() == _fareType);
}
bool
MinFareSpecialFareSelection::matchFareType(const FareType& fareType)
{
  return std::find(_curFareTypeVec.begin(), _curFareTypeVec.end(), fareType) !=
         _curFareTypeVec.end();
}
} // tse
