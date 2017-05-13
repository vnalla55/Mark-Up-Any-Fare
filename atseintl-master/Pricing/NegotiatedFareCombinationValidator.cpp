//----------------------------------------------------------------------------
//
//  File:  NegotiatedFareCombinationValidator.cpp
//  Created:  July 10, 2009
//  Authors:  Nakamon Thamsiriboon/Konrad Koch
//
//  Description: Validate Negotiated Fares combination
//
//  Copyright Sabre 2009
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

#include "Pricing/NegotiatedFareCombinationValidator.h"

#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/NegFareRestExt.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag535Collector.h"
#include "Pricing/PricingUtil.h"
#include "Rules/NegotiatedFareRuleUtil.h"
#include "Rules/NetRemitFareSelection.h"
#include "Rules/NetRemitPscMatchUtil.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "Rules/ValueCodeUtil.h"

namespace tse
{
static Logger
logger("atseintl.Pricing.NegotiatedFareCombinationValidator");

namespace
{
ConfigurableValue<bool>
skipTourCodesValidation("SOLO_CARNIVAL_OPT", "SKIP_TOUR_CODES_VALIDATION", false);
}
NegotiatedFareCombinationValidator::NegotiatedFareCombinationValidator(PricingTrx& trx)
  : _trx(trx),
    _dc(nullptr),
    _warningCode(NO_WARNING),
    _farePathScope(false),
    _abacusUser(false),
    _infiniUser(false),
    _shouldDisplayTfdpsc(false)
{
  if (UNLIKELY(_trx.diagnostic().diagnosticType() == Diagnostic535))
  {
    _dc = dynamic_cast<Diag535Collector*>(DCFactory::instance()->create(_trx));

    if (_dc)
      _dc->enable(Diagnostic535);
  }
  _abacusUser = _trx.getRequest()->ticketingAgent()->abacusUser();
  _infiniUser = _trx.getRequest()->ticketingAgent()->infiniUser();
}

NegotiatedFareCombinationValidator::~NegotiatedFareCombinationValidator()
{
  if (UNLIKELY(_dc))
  {
    _dc->flushMsg();
  }
}

bool
NegotiatedFareCombinationValidator::validate(const PricingUnit& pricingUnit)
{
  bool result = true;
  _farePathScope = false;

  std::vector<const PaxTypeFare*> paxTypeFares;

  getAllPaxTypeFares(pricingUnit, paxTypeFares);

  result = validate(paxTypeFares);

  if (UNLIKELY(result && _infiniUser && !(validateNetRemitMethod1(paxTypeFares))))
  {
    result = false;
  }

  if (UNLIKELY(result && TrxUtil::optimusNetRemitEnabled(_trx) &&
               !validatePrintOptionCombination(paxTypeFares)))
  {
    result = false;
  }

  if (result && !validateTourCodeCombination(paxTypeFares))
  {
    result = false;
  }

  if (UNLIKELY(result && TrxUtil::tfdNetRemitFareCombEnabled(_trx) &&
               !validateTFDCombination(paxTypeFares)))
  {
    result = false;
  }

  if (result && (TrxUtil::optimusNetRemitEnabled(_trx) || _infiniUser) && !paxTypeFares.empty() &&
      isNetRemitFareCombination(*paxTypeFares.begin()))
  {
    NetRemitPscMatchUtil nrMatch(pricingUnit);
    if (!nrMatch.process())
    {
      _shouldDisplayTfdpsc = true;
      _warningCode = TFDPSC_MATCH_FAIL;
      result = false;
    }

    if (result && !validateFareBoxCombination(paxTypeFares))
    {
      result = false;
    }
  }

  if (UNLIKELY(_dc && paxTypeFares.size() > 0))
    doDiagnostic(paxTypeFares, result);

  return result;
}

bool
NegotiatedFareCombinationValidator::validate(const FarePath& farePath)
{
  bool result = true;
  _farePathScope = true;
  std::vector<const PaxTypeFare*> paxTypeFares;
  std::vector<const PaxTypeFare*> allPaxTypeFares;

  std::vector<PricingUnit*>::const_iterator puIt = farePath.pricingUnit().begin();
  const std::vector<PricingUnit*>::const_iterator puItEnd = farePath.pricingUnit().end();

  if (farePath.pricingUnit().size() > 1)
  {
    for (; puIt != puItEnd; puIt++)
    {
      paxTypeFares.push_back((*puIt)->fareUsage().front()->paxTypeFare());
    }
    result = validate(paxTypeFares);
    if (UNLIKELY(result && TrxUtil::optimusNetRemitEnabled(_trx) &&
                 !validatePrintOptionCombination(paxTypeFares)))
    {
      result = false;
    }
  }
  if (result && farePath.pricingUnit().size() > 1)
  {
    getAllPaxTypeFares(farePath, allPaxTypeFares);
    if (result && !validateTourCodeCombination(allPaxTypeFares))
    {
      result = false;
      if (_dc && allPaxTypeFares.size() > 0)
      {
        doDiagnostic(allPaxTypeFares, result, &farePath);
      }
      return result;
    }
  }

  if (UNLIKELY(result && TrxUtil::optimusNetRemitEnabled(_trx) && !validateValueCode(farePath)))
  {
    result = false;
    if (allPaxTypeFares.empty())
      getAllPaxTypeFares(farePath, allPaxTypeFares);
    if (_dc && allPaxTypeFares.size() > 0)
    {
      doDiagnostic(allPaxTypeFares, result, &farePath);
    }
    return result;
  }

  if (UNLIKELY(result && TrxUtil::optimusNetRemitEnabled(_trx) && !paxTypeFares.empty() &&
               isNetRemitFareCombination(*paxTypeFares.begin()) &&
               !validateFareBoxCombination(paxTypeFares)))
  {
    result = false;
  }

  if (UNLIKELY(result && TrxUtil::tfdNetRemitFareCombEnabled(_trx) &&
               !validateAllTFDData(farePath, paxTypeFares)))
  {
    result = false;
  }

  if (UNLIKELY(_dc && paxTypeFares.size() > 0))
  {
    doDiagnostic(paxTypeFares, result, &farePath);
  }
  if (!paxTypeFares.empty())
    paxTypeFares.clear();
  if (!allPaxTypeFares.empty())
    allPaxTypeFares.clear();

  return result;
}

bool
NegotiatedFareCombinationValidator::validate(const std::vector<const PaxTypeFare*>& paxTypeFares)
{
  bool result = true;
  if (UNLIKELY(paxTypeFares.empty()))
    return result;

  LOG4CXX_DEBUG(logger, "Validating combination of negotiated fares");

  result = validateNegFareCombination(paxTypeFares);

  if (result)
  {
    if (!validateCommissionCombination(paxTypeFares))
    {
      _warningCode = MIXED_COMMISSION;
      result = false;
    }
    else
    {
      NegotiatedFareRuleUtil nfru;
      if (UNLIKELY(!_farePathScope && !nfru.checkForTourCodesConflict(_trx, paxTypeFares)))
      {
        _warningCode = CONFLICTING_TOUR_CODES;
        result = false;
      }
    }
  }

  return result;
}

bool
NegotiatedFareCombinationValidator::validateNegFareCombination(
    const std::vector<const PaxTypeFare*>& paxTypeFares)
{
  if (LIKELY(paxTypeFares.size() > 0))
  {
    std::vector<const PaxTypeFare*>::const_iterator ptfIt = paxTypeFares.begin(),
                                                    ptfItEnd = paxTypeFares.end();

    const PaxTypeFare& firstPTF = **ptfIt;
    const NegFareRest* firstFareC35TktData = getCat35TktData(firstPTF);
    bool isFirstFareNetRemit =
        NegotiatedFareRuleUtil::isNetRemitFare(firstPTF.fcaDisplayCatType(), firstFareC35TktData);
    const Indicator& firstFareMethodType = getBspMethodType(firstFareC35TktData);

    for (ptfIt++; ptfIt != ptfItEnd; ptfIt++)
    {
      const PaxTypeFare& currentPTF = **ptfIt;
      const NegFareRest* currentFareC35TktData = getCat35TktData(currentPTF);

      if (UNLIKELY(!validateNetRemitCombination(
                       isFirstFareNetRemit,
                       NegotiatedFareRuleUtil::isNetRemitFare(currentPTF.fcaDisplayCatType(),
                                                              currentFareC35TktData))))
      {
        _warningCode = MIXED_FARES;
        return false;
      }

      if (UNLIKELY(!validateMethodTypeCombination(firstFareMethodType,
                                                  getBspMethodType(currentFareC35TktData))))
      {
        _warningCode = MULTIPLE_BSP;
        return false;
      }
    }
  }
  return true;
}

bool
NegotiatedFareCombinationValidator::validateNetRemitCombination(bool isPTF1NetRemitFare,
                                                                bool isPTF2NetRemitFare) const
{
  return (isPTF1NetRemitFare || isPTF2NetRemitFare) ? (isPTF1NetRemitFare && isPTF2NetRemitFare)
                                                    : true;
}

bool
NegotiatedFareCombinationValidator::validateMethodTypeCombination(const Indicator& method1,
                                                                  const Indicator& method2) const
{
  return (method1 == method2);
}

const NegFareRest*
NegotiatedFareCombinationValidator::getCat35Record3(const PaxTypeFare& paxTypeFare) const
{
  const NegPaxTypeFareRuleData* negPaxTypeFare = paxTypeFare.getNegRuleData();

  if (LIKELY(negPaxTypeFare))
    return dynamic_cast<const NegFareRest*>(negPaxTypeFare->ruleItemInfo());

  return nullptr;
}

const NegFareRest*
NegotiatedFareCombinationValidator::getCat35TktData(const PaxTypeFare& paxTypeFare) const
{
  return paxTypeFare.isNegotiated() ? getCat35Record3(paxTypeFare) : nullptr;
}

Indicator
NegotiatedFareCombinationValidator::getBspMethodType(const NegFareRest* negFareRest) const
{
  return negFareRest ? negFareRest->netRemitMethod() : RuleConst::NRR_METHOD_BLANK;
}

const char*
NegotiatedFareCombinationValidator::getWarningMessage() const
{
  return getWarningMessage(_warningCode);
}

const char*
NegotiatedFareCombinationValidator::getWarningMessage(const WarningCode warningCode) const
{
  switch (warningCode)
  {
  case MIXED_FARES:
    return "MIXED FARES INCL NET REMIT";

  case MULTIPLE_BSP:
    return "MULTIPLE BSP METHOD TYPES";

  case MIXED_COMMISSION:
    return "MIXED COMMISSION";

  case MULTIPLE_TOUR_CODES:
    return "MULTIPLE TOUR CODES";

  case CONFLICTING_TOUR_CODES:
    return "CAT35/CAT27 TOUR CODE CONFLICT";

  case CONFLICTING_TFD_BYTE101:
    return "NET REMIT/FARE IND BYTE 101 CONFLICT";

  case TFD_RETRIEVE_FAIL:
    return "NET REMIT/UNA TO VAL TKT FARE DATA";

  case TFDPSC_MATCH_FAIL:
    return "NET REMIT/UNABLE TO MATCH TFDPSC";

  case MULTIPLE_VALUE_CODES:
    return "MULTIPLE VALUE CODES";

  case MULTIPLE_PRINT_OPTIONS:
    return "MULTIPLE PRINT OPTIONS";

  case MIXED_FARE_BOX_AMT:
    return "MIXED FARE BOX CUR/AMT";

  case METHOD_TYPE_1_REQ:
    return "METHOD TYPE 1 REQ COMM G\n"
           "                           "
           "AND VALUE CODE B/V";

  case NO_WARNING:
  default:
    return "";
  }
}

namespace
{
bool
isRealFare(const PaxTypeFare* ptFare)
{
  return !ptFare->fareMarket()->useDummyFare();
}

const PaxTypeFare&
getRealFare(const PaxTypeFare* ptFare, const PaxTypeFare* realPtFare)
{
  if (UNLIKELY(realPtFare && !isRealFare(ptFare)))
    return *realPtFare;
  return *ptFare;
}
}

bool
NegotiatedFareCombinationValidator::validateCommissionCombination(
    const std::vector<const PaxTypeFare*>& paxTypeFares)
{
  const PaxTypeFare* realPtFare = nullptr;
  std::vector<const PaxTypeFare*>::const_iterator it =
      std::find_if(paxTypeFares.begin(), paxTypeFares.end(), isRealFare);
  if (LIKELY(it != paxTypeFares.end()))
    realPtFare = *it;

  std::vector<const PaxTypeFare*>::const_iterator ptfIt = paxTypeFares.begin(),
                                                  ptfItEnd = paxTypeFares.end();

  std::vector<const PaxTypeFare*>::const_iterator ptfItS = paxTypeFares.begin() + 1;

  const NegFareRest* negFareRest1 = nullptr;
  const NegFareRest* negFareRest2 = nullptr;

  for (; ptfIt != ptfItEnd - 1; ptfIt++)
  {
    const PaxTypeFare& ptf = getRealFare(*ptfIt, realPtFare);
    if (ptf.isNegotiated())
      negFareRest1 = getCat35Record3(ptf);

    const PaxTypeFare& ptf2 = getRealFare(*ptfItS, realPtFare);
    if (ptf2.isNegotiated())
      negFareRest2 = getCat35Record3(ptf2);

    if (negFareRest1 && negFareRest2)
    {
      if (negFareRest1 != negFareRest2)
      {
        if (negFareRest1->netGrossInd() != negFareRest2->netGrossInd())
          return false;
        if (negFareRest1->commPercent() != negFareRest2->commPercent())
          return false;
      }
    }
    else
    {
      if (negFareRest1)
      {
        if (negFareRest1->commPercent() != RuleConst::PERCENT_NO_APPL ||
            negFareRest1->netGrossInd() != BLANK)
          return false;
        if (UNLIKELY((negFareRest1->netRemitMethod() == RuleConst::NRR_METHOD_BLANK) &&
                     (negFareRest1->commAmt1() != 0 || negFareRest1->commAmt2() != 0)))
          return false;
      }
      else if (negFareRest2)
      {
        if (negFareRest2->commPercent() != RuleConst::PERCENT_NO_APPL ||
            negFareRest2->netGrossInd() != BLANK)
          return false;
        if ((negFareRest2->netRemitMethod() == RuleConst::NRR_METHOD_BLANK) &&
            (negFareRest2->commAmt1() != 0 || negFareRest2->commAmt2() != 0))
          return false;
      }
    }
    negFareRest1 = nullptr;
    negFareRest2 = nullptr;
    ptfItS++;
  }
  return true;
}

bool
NegotiatedFareCombinationValidator::validateTFDCombination(
    const std::vector<const PaxTypeFare*>& paxTypeFares)
{
  NegotiatedFareRuleUtil nfru;
  if (UNLIKELY(!nfru.validateTFDCombination(_trx, paxTypeFares)))
  {
    _warningCode = CONFLICTING_TFD_BYTE101;
    return false;
  }
  return true;
}

bool
NegotiatedFareCombinationValidator::validateAllTFDData(
    const FarePath& farePath, std::vector<const PaxTypeFare*>& paxTypeFares)
{
  if (TrxUtil::optimusNetRemitEnabled(_trx) &&
      !isNetRemitFareCombination(farePath.pricingUnit()[0]->fareUsage()[0]->paxTypeFare()))
    return true;

  if (!paxTypeFares.empty() && !validateTFDCombination(paxTypeFares))
    return false;

  std::vector<PricingUnit*>::const_iterator puIt = farePath.pricingUnit().begin();
  std::vector<PricingUnit*>::const_iterator puItEnd = farePath.pricingUnit().end();

  if (TrxUtil::optimusNetRemitEnabled(_trx))
  {
    _shouldDisplayTfdpsc = true;
    for (; puIt != puItEnd; ++puIt)
    {
      PricingUnit& pricingUnit = **puIt;
      std::vector<FareUsage*>::iterator fareUsageI = pricingUnit.fareUsage().begin();
      std::vector<FareUsage*>::iterator fareUsageEnd = pricingUnit.fareUsage().end();

      for (; fareUsageI != fareUsageEnd; ++fareUsageI)
      {
        FareUsage& fu = **fareUsageI;

        // NegPaxTypeFareRuleData* negPaxTypeFare = 0;
        const NegFareRest* negFareRest = nullptr;

        PaxTypeFare* ptf = fu.paxTypeFare();
        paxTypeFares.push_back(ptf);

        if (ptf->isNegotiated())
        {
          negFareRest = getCat35Record3(*ptf);
          if (negFareRest)
          {
            const NegPaxTypeFareRuleData* ruleData = nullptr;
            const NegFareRestExt* negFareRestExt = nullptr;
            Indicator tktFareDataInd1 = negFareRest->tktFareDataInd1();

            if (tktFareDataInd1 == RuleConst::BLANK &&
                _trx.dataHandle().getVendorType(ptf->vendor()) == RuleConst::SMF_VENDOR)
            {
              // We have rec3 so we know type is NegPaxTypeFareRuleData
              ruleData =
                  ptf->paxTypeFareRuleData(RuleConst::NEGOTIATED_RULE)->toNegPaxTypeFareRuleData();
              TSE_ASSERT(ruleData);
              negFareRestExt = ruleData->negFareRestExt();
              if (negFareRestExt)
                tktFareDataInd1 = negFareRestExt->fareBasisAmtInd();
            }

            if (tktFareDataInd1 == RuleConst::BLANK || tktFareDataInd1 == RuleConst::NR_VALUE_N)
              continue;

            if (!processNetRemitFareSelection(_trx, farePath, pricingUnit, fu, *negFareRest))
            {
              _warningCode = TFD_RETRIEVE_FAIL;
              return false;
            }
          } // if negFareRest
        } // if isNegotiated
      } // for - fareusage
    } // for PU
  }
  else
  {
    for (; puIt != puItEnd; ++puIt)
    {
      PricingUnit& pricingUnit = **puIt;
      std::vector<FareUsage*>::iterator fareUsageI = pricingUnit.fareUsage().begin();
      std::vector<FareUsage*>::iterator fareUsageEnd = pricingUnit.fareUsage().end();

      for (; fareUsageI != fareUsageEnd; ++fareUsageI)
      {
        FareUsage& fu = **fareUsageI;
        const NegFareRest* negFareRest = nullptr;
        PaxTypeFare* ptf = fu.paxTypeFare();
        paxTypeFares.push_back(ptf);
        if (ptf->isNegotiated())
        {
          negFareRest = getCat35Record3(*ptf);
          if (negFareRest &&
              NegotiatedFareRuleUtil::isNetRemitFare(ptf->fcaDisplayCatType(), negFareRest) &&
              negFareRest->tktFareDataInd1() != ' ')
          {
            if (!processNetRemitFareSelection(_trx, farePath, pricingUnit, fu, *negFareRest))
            {
              _warningCode = TFD_RETRIEVE_FAIL;
              return false;
            }
          } // if negFareRest
        } // if isNegotiated
      } // for - fareusage
    } // for PU
  }
  return true;
}

bool
NegotiatedFareCombinationValidator::validateNetRemitMethod1(
    std::vector<const PaxTypeFare*>& paxTypeFares)
{
  if (UNLIKELY(!NegotiatedFareRuleUtil::validateNetRemitMethod1(paxTypeFares)))
  {
    _warningCode = METHOD_TYPE_1_REQ;
    return false;
  }
  return true;
}

bool
NegotiatedFareCombinationValidator::processNetRemitFareSelection(PricingTrx& trx,
                                                                 const FarePath& farePath,
                                                                 PricingUnit& pricingUnit,
                                                                 FareUsage& fareUsage,
                                                                 const NegFareRest& negFareRest)
    const
{
  return NetRemitFareSelection::processNetRemitFareSelection(
      trx, farePath, pricingUnit, fareUsage, negFareRest);
}

void
NegotiatedFareCombinationValidator::getAllPaxTypeFares(
    const FarePath& fp, std::vector<const PaxTypeFare*>& paxTypeFares)
{
  std::vector<PricingUnit*>::const_iterator puIt = fp.pricingUnit().begin();
  const std::vector<PricingUnit*>::const_iterator puItEnd = fp.pricingUnit().end();
  for (; puIt != puItEnd; puIt++)
  {
    const PricingUnit& pu = *(*puIt);
    getAllPaxTypeFares(pu, paxTypeFares);
  }
}

void
NegotiatedFareCombinationValidator::getAllPaxTypeFares(
    const PricingUnit& pu, std::vector<const PaxTypeFare*>& paxTypeFares)
{
  std::vector<FareUsage*>::const_iterator fuIt = pu.fareUsage().begin();
  const std::vector<FareUsage*>::const_iterator fuItEnd = pu.fareUsage().end();

  for (; fuIt != fuItEnd; fuIt++)
  {
    if (LIKELY(!(*fuIt)->paxTypeFare()->isDummyFare()))
      paxTypeFares.push_back((*fuIt)->paxTypeFare());
  }
}

bool
NegotiatedFareCombinationValidator::validatePrintOptionCombination(
    std::vector<const PaxTypeFare*>& paxTypeFares)
{
  if (UNLIKELY(!NegotiatedFareRuleUtil::validatePrintOptionCombination(paxTypeFares)))
  {
    _warningCode = MULTIPLE_PRINT_OPTIONS;
    return false;
  }
  return true;
}

bool
NegotiatedFareCombinationValidator::validateTourCodeCombination(
    std::vector<const PaxTypeFare*>& paxTypeFares)
{
  if (UNLIKELY(_trx.getTrxType() == PricingTrx::IS_TRX &&
               _trx.getOptions()->isCarnivalSumOfLocal()))
  {
    if (skipTourCodesValidation.getValue())
      return true;
  }

  NegotiatedFareRuleUtil nfru;
  if (!nfru.validateTourCodeCombination(_trx, paxTypeFares))
  {
    _warningCode = MULTIPLE_TOUR_CODES;
    return false;
  }
  return true;
}

void
NegotiatedFareCombinationValidator::doDiagnostic(std::vector<const PaxTypeFare*>& paxTypeFares,
                                                 bool result,
                                                 const FarePath* fp)
{
  if (!isNegotiatedFareCombination(paxTypeFares))
    return;

  _dc->displayHeader(_farePathScope);

  std::vector<const PaxTypeFare*>::const_iterator ptfIt = paxTypeFares.begin(),
                                                  ptfItEnd = paxTypeFares.end();
  for (; ptfIt != ptfItEnd; ptfIt++)
  {
    const PaxTypeFare& currentPTF = **ptfIt;
    _dc->displayPaxTypeFare(
        currentPTF, getCat35TktData(currentPTF), _shouldDisplayTfdpsc, _farePathScope, _trx, fp);
  }
  _dc->displayValidationResult(result, getWarningMessage());
}

bool
NegotiatedFareCombinationValidator::isNetRemitFareCombination(const PaxTypeFare* paxTypeFare) const
{
  if (paxTypeFare && paxTypeFare->isNegotiated())
  {
    const NegFareRest* negFareRest = getCat35Record3(*paxTypeFare);
    if (negFareRest &&
        NegotiatedFareRuleUtil::isNetRemitFare(paxTypeFare->fcaDisplayCatType(), negFareRest))
      return true;
  }
  return false;
}

bool
NegotiatedFareCombinationValidator::isNegotiatedFareCombination(
    const std::vector<const PaxTypeFare*>& paxTypeFares) const
{
  std::vector<const PaxTypeFare*>::const_iterator ptfIt = paxTypeFares.begin(),
                                                  ptfItEnd = paxTypeFares.end();
  for (; ptfIt != ptfItEnd; ptfIt++)
  {
    const PaxTypeFare& currentPTF = **ptfIt;
    if (currentPTF.isNegotiated())
      return true;
  }
  return false;
}

bool
NegotiatedFareCombinationValidator::validateValueCode(const FarePath& farePath)
{
  bool result = true;
  bool hasDynamicValueCodeFare = ValueCodeUtil::hasDynamicValueCodeFare(farePath);
  bool hasStaticValueCodeFare = ValueCodeUtil::hasCat35ValueCode(farePath);
  if (UNLIKELY(hasDynamicValueCodeFare || hasStaticValueCodeFare))
  {
    if (!farePath.endorsementsCollected())
    {
      PricingUtil::collectEndorsements(_trx, const_cast<FarePath&>(farePath));
      const_cast<FarePath&>(farePath).endorsementsCollected() = true;
    }
    if (hasDynamicValueCodeFare)
      result = ValueCodeUtil::validateDynamicValueCodeCombination(_trx, farePath);
    else if (hasStaticValueCodeFare)
      result = ValueCodeUtil::validateStaticValueCodeCombination(_trx, farePath);
    if (!result)
      _warningCode = MULTIPLE_VALUE_CODES;
  }
  return result;
}

bool
NegotiatedFareCombinationValidator::validateFareBoxCombination(
    const std::vector<const PaxTypeFare*>& paxTypeFares)
{
  NegotiatedFareRuleUtil nfru;
  if (!nfru.validateFareBoxCombination(paxTypeFares))
  {
    _warningCode = MIXED_FARE_BOX_AMT;
    return false;
  }
  return true;
}
}
