/*----------------------------------------------------------------------------
 *  File:    FareMarketMerger.C
 *  Created: Oct 03, 2004
 *  Authors: Mohammad Hossan
 *
 *  Description:  To build all the MergedFareMarket for a Itin.
 *
 *  Copyright Sabre 2003
 *
 *     The copyright to the computer program(s) herein
 *     is the property of Sabre.
 *     The program(s) may be used and/or copied only with
 *     the written permission of Sabre or in accordance
 *     with the terms and conditions stipulated in the
 *     agreement/contract under which the program(s)
 *     have been supplied.
 *-------------------------------------------------------------------------*/
#include "Pricing/FareMarketMerger.h"

#include "Common/Assert.h"
#include "Common/BrandingUtil.h"
#include "Common/DateTime.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/ShoppingUtil.h"
#include "Common/TNBrands/ItinBranding.h"
#include "Common/TNBrands/TNBrandsFunctions.h"
#include "Common/TseEnums.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TNBrandsTypes.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/IndustryPricingAppl.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/PaxTypeInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag602Collector.h"
#include "Diagnostic/Diag981Collector.h"
#include "Fares/FareUtil.h"
#include "Pricing/MergedFareMarket.h"
#include "RexPricing/RexFareBytesValidator.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DBAccess/FareByRuleApp.h"

#include <map>
#include <set>

// for Debug only
// void debugOnlyDisplayPTF(const std::vector<PaxTypeFare*>& ptfV);
// bool debugOnlyFilter(const FareMarket& fareMarket);

namespace tse
{
FALLBACK_DECL(fallbackAcctCodeCorpIDGroupFilter);
FALLBACK_DECL(fallbackFlexFareGroupNewJumpCabinLogic);
FALLBACK_DECL(fallbackJumpCabinExistingLogic);
FALLBACK_DECL(fallbckFareMarketMergerRefactoring);
FALLBACK_DECL(fallbckFareMarketMergerRefactoring2);

static Logger
logger("atseintl.Pricing.FareMarketMerger");

namespace
{

struct CheckConditionForRexTrx
{
  CheckConditionForRexTrx(const PricingTrx& trx) : _trx(static_cast<const RexPricingTrx&>(trx))
  {
    if (RexPricingTrx::isRexTrxAndNewItin(trx))
    {
      switch (_trx.tag1PricingSvcCallStatus())
      {
      case RexPricingTrx::NONE:
        _fun = &CheckConditionForRexTrx::check;
        break;
      case RexPricingTrx::TAG1PERMUTATION:
        _fun = &CheckConditionForRexTrx::keep;
        break;
      case RexPricingTrx::TAG7PERMUTATION:
        _fun = &CheckConditionForRexTrx::current;
      }
    }
    else
      _fun = &CheckConditionForRexTrx::fail;
  }

  bool operator()(const FareMarket& fm) const { return (this->*_fun)(fm); }

protected:
  bool (CheckConditionForRexTrx::*_fun)(const FareMarket& fm) const;
  const RexPricingTrx& _trx;

  bool check(const FareMarket& fm) const
  {
    return (fm.retrievalFlag() == FareMarket::RetrievLastReissue ||
            (fm.retrievalFlag() & FareMarket::RetrievHistorical &&
             !_trx.fareRetrievalFlags().isSet(FareMarket::MergeHistorical)));
  }

  bool fail(const FareMarket& fm) const { return false; }

  bool keep(const FareMarket& fm) const
  {
    return !(!check(fm) &&
             (fm.retrievalFlag() == FareMarket::RetrievNone || fm.retrievalInfo()->keep()));
  }

  bool current(const FareMarket& fm) const
  {
    return !(!check(fm) && (fm.retrievalFlag() & FareMarket::RetrievCurrent));
  }
};
}

//----------------------------------------------------------------------

FareMarketMerger::CxrPrefFareComparator::CxrPrefFareComparator(PaxTypeBucket& cortege,
                                                               uint16_t paxTypeNum,
                                                               bool compareCabins)
  : _cortege(cortege), _paxTypeNum(paxTypeNum), _cabinComparator(compareCabins)
{
}

bool
FareMarketMerger::CxrPrefFareComparator::
operator()(const PaxTypeFare* lptf, const PaxTypeFare* rptf)
{
  if (rptf == nullptr)
    return true;
  if (lptf == nullptr)
    return false;

  if ((lptf->carrier() == INDUSTRY_CARRIER) != (rptf->carrier() == INDUSTRY_CARRIER))
  {
    return (rptf->carrier() == INDUSTRY_CARRIER);
  }

  MoneyAmount lNucAmount = lptf->totalFareAmount();
  MoneyAmount rNucAmount = rptf->totalFareAmount();

  lNucAmount += lptf->baggageLowerBound();
  rNucAmount += rptf->baggageLowerBound();

  {
    const CxrPrecalculatedTaxes& taxes = _cortege.cxrPrecalculatedTaxes();
    if (!taxes.empty())
    {
      lNucAmount += taxes.getLowerBoundTotalTaxAmount(*lptf);
      rNucAmount += taxes.getLowerBoundTotalTaxAmount(*rptf);
    }
  }

  MoneyAmount diff = fabs(rNucAmount - lNucAmount);
  if (diff > EPSILON)
    return (lNucAmount < rNucAmount);

  if (_cabinComparator.shouldCompareCabins() &&
      !_cabinComparator.areCabinsEqual(lptf->cabin(), rptf->cabin()))
  {
    return _cabinComparator.isLessCabin(lptf->cabin(), rptf->cabin());
  }

  if (lptf->mileage() != rptf->mileage())
    return (lptf->mileage() < rptf->mileage());

  // totalFareAmount equal (or close enough)

  // left < right MUST imply !right < left
  if (lptf->isFareByRule() != rptf->isFareByRule())
  {
    return lptf->isFareByRule();
  }

  // may need to change for fare group number.
  if (lptf->actualPaxTypeItem().size() > _paxTypeNum &&
      rptf->actualPaxTypeItem().size() > _paxTypeNum)
  {
    if (lptf->actualPaxTypeItem()[_paxTypeNum] != rptf->actualPaxTypeItem()[_paxTypeNum])
    {
      return (lptf->actualPaxTypeItem()[_paxTypeNum] < rptf->actualPaxTypeItem()[_paxTypeNum]);
    }
  }

  if (lptf->isWebFare() != rptf->isWebFare())
  {
    return lptf->isWebFare();
  }

  // may need to change for group fare, etc.
  if (lptf->fcasPaxType() != rptf->fcasPaxType())
  {
    const PaxTypeCode paxCode = _cortege.requestedPaxType()->paxType();
    if (paxCode == lptf->fcasPaxType())
      return true;
    else if (paxCode == rptf->fcasPaxType())
      return false;
    // both not requested, so put ADT at end
    if (rptf->fcasPaxType() == ADULT)
      return true;
    else if (lptf->fcasPaxType() == ADULT)
      return false;
  }

  return (lptf->fareClass() < rptf->fareClass());
}

//----------------------------------------------------------------------

bool
FareMarketMerger::ValidPaxTypeFareCopier::isValidForFareBytes(PaxTypeFare* ptf)
{
  typedef std::multimap<CarrierCode, FareBytesData>::iterator Iterator;
  typedef std::pair<Iterator, Iterator> Range;

  RexPricingTrx::BoardOffToFareBytesData::iterator i = _fareBytesData->find(
      std::make_pair(ptf->fareMarket()->boardMultiCity(), ptf->fareMarket()->offMultiCity()));

  RexPricingTrx& rexTrx = static_cast<RexPricingTrx&>(_trx);

  RexFareBytesValidator rfbv(rexTrx,
                             rexTrx.exchangeItin().front()->farePath().front());

  if (i == _fareBytesData->end())
  {
    if (_trx.excTrxType() == PricingTrx::AR_EXC_TRX)
    {
      RexPricingTrx& rexTrx = static_cast<RexPricingTrx&>(_trx);
      if (rexTrx.trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE)
      {
        typedef std::multimap<CarrierCode, FareBytesData> CxrToFareBytesData;
        CxrToFareBytesData& unmappedFareByteData = rexTrx.unmappedFareBytesData();

        if (unmappedFareByteData.count(ptf->fareMarket()->governingCarrier()))
        {
          Range allPTIs = unmappedFareByteData.equal_range(ptf->fareMarket()->governingCarrier());

          Iterator iter = allPTIs.first;
          for (; iter != allPTIs.second; ++iter)
          {
            if (rfbv.validate(iter->second, *ptf))
              return true;
          }
          return false;
        }
      }
    }
    return true;
  }

  return rfbv.validate((*i).second, *ptf);
}

bool
FareMarketMerger::ValidPaxTypeFareCopier::isValidForFlexFares(PaxTypeFare* ptf)
{
  flexFares::GroupsFilter groupsFilter(_trx.getFlexFaresTotalAttrs());
  flexFares::GroupsIds groups;
  groups.insert(_groupId);
  bool rc = false;

  rc = groupsFilter.filterOutInvalidGroups(ptf->getFlexFaresValidationStatus(), _groupId);
  if (rc && !fallback::fallbackAcctCodeCorpIDGroupFilter(&_trx))
    rc = isMatchedAcctCodeCorpID(const_cast<const PaxTypeFare*>(ptf), _groupId);

  if (fallback::fallbackJumpCabinExistingLogic(&_trx))//update the below logic to a new function
  {
    if (rc && !fallback::fallbackFlexFareGroupNewJumpCabinLogic(&_trx) && (_trx.getRequest()->
      getFlexFaresGroupsData().getFFGJumpCabinLogic(_groupId) == flexFares::JumpCabinLogic::DISABLED))
      rc = !ptf->jumpedDownCabinAllowed();
  }
  else
    rc = rc && isJumpCabinLogicValidForFlexFares(ptf, _groupId);

  if ((_diagPtr) && (!rc))
    _diagPtr->displayFailedFlexFaresPTF(ptf);
  return rc;
}

bool
FareMarketMerger::ValidPaxTypeFareCopier::isJumpCabinLogicValidForFlexFares(
                                          const PaxTypeFare* ptf, flexFares::GroupId grpId)
{
  flexFares::JumpCabinLogic ffgJumpCabin =
    _trx.getRequest()->getFlexFaresGroupsData().getFFGJumpCabinLogic(grpId);
  switch (ffgJumpCabin)
  {
    case flexFares::JumpCabinLogic::DISABLED:
      return !ptf->jumpedDownCabinAllowed() && _trx.legPreferredCabinClass()[0]==ptf->cabin();
    case flexFares::JumpCabinLogic::ONLY_MIXED:
      return ptf->cabin() >= _trx.legPreferredCabinClass()[0];
    default:
      return true;
  }
}

//Returns true if any group has matched corpID/accCode OR if it is a normal fare
//without corpID/AccCode. Else returns false
bool
FareMarketMerger::ValidPaxTypeFareCopier::isMatchedAcctCodeCorpID(const PaxTypeFare* ptf, const flexFares::GroupId groupID)
{
  std::string accountCode;
  if (ptf->isFareByRule())
    accountCode = ptf->fbrApp().accountCode().c_str();
  if (accountCode.empty() && ptf->matchedCorpID())
    accountCode = ptf->matchedAccCode();

  if (!accountCode.empty())
  {
    if(_trx.getRequest()->getMutableFlexFaresGroupsData().getAccCodes(groupID).count(accountCode) ||
       _trx.getRequest()->getMutableFlexFaresGroupsData().getCorpIds(groupID).count(accountCode))
      return true;
    else
      return false;
  }
  return true;
}

namespace {
  bool isValidForBrandInDirection(const PricingTrx& trx,
                                  const PaxTypeFare& ptf,
                                  const skipper::CarrierBrandPairs& brands,
                                  Direction direction)
  {
    skipper::CarrierBrandPairs::const_iterator cbp =
      brands.find(skipper::CarrierDirection(ptf.fareMarket()->governingCarrier(), direction));

    if (cbp == brands.end())
    {
      if (direction == Direction::BOTHWAYS)
      {
        cbp = brands.find(skipper::CarrierDirection(ptf.fareMarket()->governingCarrier(), Direction::ORIGINAL));
        if (cbp != brands.end() && ptf.isValidForBrand(trx, &cbp->second))
          return true;
        cbp = brands.find(skipper::CarrierDirection(ptf.fareMarket()->governingCarrier(), Direction::REVERSED));
      }
      else
        cbp = brands.find(skipper::CarrierDirection(ptf.fareMarket()->governingCarrier(), Direction::BOTHWAYS));
    }

    if (cbp != brands.end())
    {
      if (ptf.isValidForBrand(trx, &cbp->second))
        return true;
    }
    return false;
  }
}

bool
FareMarketMerger::ValidPaxTypeFareCopier::isValidForBrandCombination(PaxTypeFare* ptf)
{
  if (_carriersBrands.empty())
    return true;

  return isValidForBrandInDirection(_trx, *ptf, _carriersBrands, ptf->getDirection());
}

void
FareMarketMerger::ValidPaxTypeFareCopier::
operator()(PaxTypeFare* paxTypeFare)
{
  if (!_trx.isValidatingCxrGsaApplicable())
  {
    if (_trx.getTrxType() == PricingTrx::MIP_TRX && _itin && paxTypeFare->isNegotiated() &&
        !FareUtil::isNegFareCarrierValid(paxTypeFare->negotiatedInfo().carrier(),
                                         paxTypeFare->negotiatedInfo().tktAppl(),
                                         *_itin,
                                         false))
      return;
  }

  if (UNLIKELY(paxTypeFare->isMatchFareFocusRule()))
      return;

  if (UNLIKELY(_checkFareRetrievalDate))
  {
    if ((paxTypeFare->retrievalInfo() == nullptr) ||
        (paxTypeFare->retrievalInfo()->_flag ==
         (FareMarket::FareRetrievalFlags)FareMarket::RetrievNone) ||
        !paxTypeFare->isValidAsKeepFare(_trx) ||
        (_isPrivateFares && paxTypeFare->tcrTariffCat() == RuleConst::PUBLIC_TARIFF) || // PV
        (_isPublishedFares && paxTypeFare->tcrTariffCat() == RuleConst::PRIVATE_TARIFF) || // PL
        (_fareGroupRequested && (paxTypeFare->actualPaxTypeItem().empty() ||
                                 !(paxTypeFare->actualPaxTypeItem()[_paxTypeNum]))) ||
        !isValidForFareBytes(paxTypeFare) ||
        (_rexPTFValidator && !_rexPTFValidator->validate(*paxTypeFare)))
    {
      return; // this fare is invalid for rex pricing for new itin
    }

    if (_trx.getRequest()->isBrandedFaresRequest() && _brandCode && !_brandCode->empty())
    {
      if(!isValidForBrand(paxTypeFare))
      {
        return;
      }
    }

    if (!isValidForCarrier(paxTypeFare))
      return;
  }
  else
  {
    bool isFareValid = (_trx.getTrxType() == PricingTrx::IS_TRX || _trx.getOptions()->fareX() ||
                        _trx.fxCnException())
                           ? paxTypeFare->isValidForPricing()
                           : paxTypeFare->isValidForCombination();

    // check brand status
    if (isFareValid)
      isFareValid = !paxTypeFare->fare()->isInvBrand(_brandIndex) &&
                    !paxTypeFare->fare()->isInvBrandCorpID(_brandIndex);

    const bool isAnyBranded = (_trx.isBRAll() ||
                               _trx.activationFlags().isSearchForBrandsPricing() ||
                               (_brandCode && !_brandCode->empty()));
    if (UNLIKELY(isFareValid && isAnyBranded))
    {
      isFareValid = isValidForBrand(paxTypeFare);
    }

    if (UNLIKELY(isFareValid && _trx.isFlexFare()))
      isFareValid = isValidForFlexFares(paxTypeFare);

    if (isFareValid)
      isFareValid = isValidForCarrier(paxTypeFare);

    if ((!isFareValid) ||
        (_isPrivateFares && paxTypeFare->tcrTariffCat() == RuleConst::PUBLIC_TARIFF) || // PV
        (_isPublishedFares && paxTypeFare->tcrTariffCat() == RuleConst::PRIVATE_TARIFF) || // PL
        (_fareGroupRequested && (paxTypeFare->actualPaxTypeItem().empty() ||
                                 !(paxTypeFare->actualPaxTypeItem()[_paxTypeNum]))))

    {
      return; // this fare is invalid for pricing
    }

    if (UNLIKELY(paxTypeFare->isFilterPricing() && !paxTypeFare->validForFilterPricing()))
    {
      return;
    }
  }

  ++_validFaresCount;
  if (paxTypeFare->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
    _tag2FareIndicator = true;
  _paxTypeFareVect.push_back(paxTypeFare);
}

void
FareMarketMerger::getHighestTPMCarriers(std::map< uint16_t, std::set<CarrierCode> >& ret)
{
  if (!_carrier ||
      !_trx.isIataFareSelectionApplicable() ||
      (PricingTrx::IS_TRX != _trx.getTrxType()))
    return;

  ShoppingTrx& shoppingTrx = dynamic_cast<ShoppingTrx&>(_trx);
  for (uint16_t legId = 0; legId < shoppingTrx.legs().size(); ++legId)
  {
    ItinIndex::Key govCxrKey;
    ShoppingUtil::createCxrKey(*_carrier, govCxrKey);
    ItinIndex::ItinCell* curItinCell =
      ShoppingUtil::retrieveDirectItin(shoppingTrx, legId, govCxrKey, ItinIndex::CHECK_NOTHING);

    if (curItinCell)
    {
      Itin* curItin = curItinCell->second;
      if (curItin)
      {
        for (FareMarket* fareMarket : curItin->fareMarket())
        {
          if (fareMarket &&
              (fareMarket->getFmTypeSol() == FareMarket::SOL_FM_THRU) &&
              (fareMarket->governingCarrier() != *_carrier))
            ret[legId].insert(fareMarket->governingCarrier());
        }
      }
    }
  }
}

namespace
{
  class FareMarketValidator
  {
  public:
    FareMarketValidator(PricingTrx& trx,
                        const CarrierCode* carrier,
                        Itin& itin,
                        uint16_t brandIndex)
    : _shoppingTrx(dynamic_cast<ShoppingTrx*>(&trx)), _noPNRTrx(0), _carrier(carrier),
      _itin(itin), _brandIndex(brandIndex),
      _tnMultiBrandPath(trx.isBRAll() || trx.activationFlags().isSearchForBrandsPricing())
    {
      if (_shoppingTrx && _carrier && trx.isIataFareSelectionApplicable())
          getHighestTPMCarriers(_highestTPMCarrierMap);
      else
        _noPNRTrx = dynamic_cast<NoPNRPricingTrx*>(&trx);
    }

    bool isFareMarketValid(FareMarket& fareMarket)
    {
      if (UNLIKELY(_noPNRTrx && _noPNRTrx->isNoMatch() &&
                   !fareMarket.isFareTypeGroupValid(_noPNRTrx->processedFTGroup())))
        return false;

      if (_shoppingTrx)
      {
        if (_carrier)
        {
          size_t legId = fareMarket.legIndex();
          const CarrierCode& governingCarrier = fareMarket.governingCarrier();
          // see if it's specified that we should only use fare markets with
          // a certain governing carrier
          bool isProcessedCarrier = (fareMarket.governingCarrier() == *_carrier);

          bool isHighTPMCarrier = _highestTPMCarrierMap.count(legId) &&
            _highestTPMCarrierMap.at(legId).count(governingCarrier);

          if (!isProcessedCarrier && !isHighTPMCarrier)
            return false;
        }
        else if (UNLIKELY(fareMarket.combineSameCxr()))
        {
          // if carrier is not allowed to combine to other carrier, we should not
          // use its fm if carrier is not specified
          return false;
        }
      }

      if (fareMarket.failCode() != ErrorResponseException::NO_ERROR)
        return false;

      if (fareMarket.breakIndicator())
      {
        // LOG4CXX_INFO(logger, "breakIndicator is true, not for combinability");
        return false;
      }

      if (UNLIKELY(_tnMultiBrandPath))
      {
        // it happens that when building pricing option spaces we exclude a given carrier from some of them
        // ( for example if it doesn't have brands we want it to appear only in one space )
        // therefore we don't want to use that carrier's fare markets in Pricing a given space at all
        const int16_t segmentIndex = _itin.segmentOrder(fareMarket.travelSeg().front()) - 1;
        if (!_itin.getItinBranding().isCarrierValidForSpaceSegment(_brandIndex,
                                                                   segmentIndex,
                                                                   fareMarket.governingCarrier()))
        {
          return false;
        }
      }

      return true;
    }

  private:
    void getHighestTPMCarriers(std::map< uint16_t, std::set<CarrierCode> >& ret)
    {
      for (uint16_t legId = 0; legId < _shoppingTrx->legs().size(); ++legId)
      {
        ItinIndex::Key govCxrKey;
        ShoppingUtil::createCxrKey(*_carrier, govCxrKey);
        ItinIndex::ItinCell* curItinCell =
          ShoppingUtil::retrieveDirectItin(*_shoppingTrx, legId, govCxrKey, ItinIndex::CHECK_NOTHING);

        if (curItinCell)
        {
          Itin* curItin = curItinCell->second;
          if (curItin)
          {
            for (FareMarket* fareMarket : curItin->fareMarket())
            {
              if (fareMarket &&
                  (fareMarket->getFmTypeSol() == FareMarket::SOL_FM_THRU) &&
                  (fareMarket->governingCarrier() != *_carrier))
                ret[legId].insert(fareMarket->governingCarrier());
            }
          }
        }
      }
    }

    ShoppingTrx* _shoppingTrx;
    NoPNRPricingTrx* _noPNRTrx;
    const CarrierCode* _carrier;
    Itin& _itin;
    uint16_t _brandIndex;
    std::map< uint16_t, std::set<CarrierCode> > _highestTPMCarrierMap;
    bool _tnMultiBrandPath;
  };
}

bool
FareMarketMerger::buildAllMergedFareMarket(std::vector<MergedFareMarket*>& mergedFareMarketVect)
{
  if (fallback::fallbckFareMarketMergerRefactoring2(&_trx))
    return buildAllMergedFareMarket_old(mergedFareMarketVect);

  //-------------------------------------------------------------------------------------------

  if(UNLIKELY(_diag993InCbsMode))
    _diagPtr->printMergerHeader();

  CheckConditionForRexTrx checkRexCondition(_trx);
  FareMarketValidator fmValidator(_trx, _carrier, _itin, _brandIndex);

  std::set<FareMarket*> alreadyConsideredFMSet;

  std::vector<FareMarket*>::iterator fmIt = _itin.fareMarket().begin();
  std::vector<FareMarket*>::iterator fmItEnd = _itin.fareMarket().end();
  for (; fmIt != fmItEnd; ++fmIt)
  {
    FareMarket* fareMarket = *fmIt;
    if (alreadyConsideredFMSet.count(fareMarket) || checkRexCondition(*fareMarket))
      continue;

    if (!fmValidator.isFareMarketValid(*fareMarket))
    {
      // LOG4CXX_INFO(logger, "FareMarket is not for combinability");
      continue;
    }

    MergedFareMarket* mfm = nullptr;
    _trx.dataHandle().get(mfm);
    copyFareMarketInfo(*fareMarket, *mfm);

    if (UNLIKELY(_brandCode && !((*_brandCode).empty())))
    {
      mfm->brandCode() = *_brandCode;
    }

    // using a vector instead of set in case order of item matters...
    std::vector<FareMarket*> allGovCxrFM; // of same O&D and ...
    allGovCxrFM.push_back(fareMarket);

    // find all FM that matches all the TravelSegs
    for (auto it = std::next(fmIt); it != fmItEnd; ++it)
    {
      FareMarket* fm = *it;

      if (!fmValidator.isFareMarketValid(*fm) || checkRexCondition(*fm))
      {
        // LOG4CXX_INFO(logger, "FM not valid for combinability");
        alreadyConsideredFMSet.insert(fm);
        continue;
      }

      if (sameMarket(*fareMarket, *fm))
      {
        // LOG4CXX_INFO(logger, "Multi Gov Cxr exists- FM:" <<
        //              fareMarket->boardMultiCity() << "-" << fareMarket->offMultiCity()
        //               << " Cxr:" << fareMarket->governingCarrier() << "  FM:" <<
        //               fm->boardMultiCity() << "-" << fm->offMultiCity() <<
        //              " Cxr:" << fm->governingCarrier())

        allGovCxrFM.push_back(fm);
        alreadyConsideredFMSet.insert(fm);
      }
    } // end for

    mfm->cxrFarePreferred() = isCxrFarePreferred(allGovCxrFM);

    if (mergeFareMarket(allGovCxrFM, *mfm))
    {
      setTag2FareIndicator(*mfm);

      if (_trx.delayXpn())
      {
        mfm->findFirstFare(_trx.paxType());
      }

      if (fallback::fallbckFareMarketMergerRefactoring(&_trx))
      {
        if (UNLIKELY(_trx.getRequest()->isBrandedFaresRequest() || _trx.isBRAll()
                     || _trx.activationFlags().isSearchForBrandsPricing()))
        {
          // Calculate the final status taking statuses
          // from all fare markets that were used to create
          // this merged fare market
          for (size_t i = 0; i < mfm->mergedFareMarket().size(); ++i)
          {
            const FareMarket* fMarket = mfm->mergedFareMarket()[i];
            TSE_ASSERT(fMarket != nullptr);

            if (_trx.getRequest()->isBrandedFaresRequest() && (_brandCode != nullptr))
              mfm->updateIbfErrorMessage(fMarket->getStatusForBrand(*_brandCode, Direction::BOTHWAYS));
            else if (_trx.isBRAll() || _trx.activationFlags().isSearchForBrandsPricing())
            {
              // segOrder enumerates segments starting from 1, not 0
              const int16_t segmentIndex = _itin.segmentOrder(fMarket->travelSeg().front()) - 1;
              // Considering current pricing option space there's only one brand matching gov.carrier
              // of each merged fare market. We accumulate statuses from all merger fare markets
              // choosing the best status of them all ( as this is a 'choice' situation )
              if (!BrandingUtil::isDirectionalityToBeUsed(_trx))
              {
                BrandCode brandCode =
                  _itin.getItinBranding().getBrandCode(
                      _brandIndex, segmentIndex, fMarket->governingCarrier(), Direction::BOTHWAYS);
                mfm->updateIbfErrorMessage(fMarket->getStatusForBrand(brandCode, Direction::BOTHWAYS));
              }
              else
              {
                IbfErrorMessageFmAcc bothwaysStatus;
                //we don't know direction of fare here, so we have to update status for brands
                //in both directions (if only BOTHWAYS is present it will be returned anyway)
                //we want "better" status, so IbfErrorMessageFmAcc will calculate it for us.
                BrandCode brandCode =
                  _itin.getItinBranding().getBrandCodeOrEmpty(
                      _brandIndex, segmentIndex, fMarket->governingCarrier(), Direction::ORIGINAL);
                if (!brandCode.empty())
                  bothwaysStatus.updateStatus(fMarket->getStatusForBrand(brandCode, Direction::ORIGINAL));
                brandCode = _itin.getItinBranding().getBrandCodeOrEmpty(
                    _brandIndex, segmentIndex, fMarket->governingCarrier(), Direction::REVERSED);
                if (!brandCode.empty())
                  bothwaysStatus.updateStatus(fMarket->getStatusForBrand(brandCode, Direction::REVERSED));

                mfm->updateIbfErrorMessage(bothwaysStatus.getStatus());
              }
            }
          }
        }
      }

      mergedFareMarketVect.push_back(mfm);

      if (_diagPtr)
        _diagPtr->printMFM(*mfm);
    }
  }

  if (_trx.getRequest()->originBasedRTPricing())
  {
    setTag2FareIndicatorForDummyFM(mergedFareMarketVect);
  }

  if(UNLIKELY(_diag993InCbsMode))
    _diagPtr->printMergerFooter();

  return true;
}

bool
FareMarketMerger::buildAllMergedFareMarket_old(std::vector<MergedFareMarket*>& mergedFareMarketVect)
{
  if(UNLIKELY(_diag993InCbsMode))
    _diagPtr->printMergerHeader();

  const bool useDirectionality = BrandingUtil::isDirectionalityToBeUsed(_trx);

  CheckConditionForRexTrx checkRexCondition(_trx);
  std::map< uint16_t, std::set<CarrierCode> > highestTPMCarrierMap;
  getHighestTPMCarriers(highestTPMCarrierMap);

  std::set<FareMarket*> alreadyConsideredFMSet;
  std::vector<FareMarket*>::iterator fmIt = _itin.fareMarket().begin();
  std::vector<FareMarket*>::iterator fmItEnd = _itin.fareMarket().end();
  for (; fmIt != fmItEnd; ++fmIt)
  {
    FareMarket* fareMarket = *fmIt;
    const std::set<FareMarket*>::const_iterator i = alreadyConsideredFMSet.find(fareMarket);
    if (i != alreadyConsideredFMSet.end() || checkRexCondition(*fareMarket))
      continue;

    std::set<CarrierCode> highestTPMCarriers;
    if (PricingTrx::IS_TRX == _trx.getTrxType())
    {
      std::map< uint16_t, std::set<CarrierCode> >::const_iterator i =
        highestTPMCarrierMap.find(fareMarket->legIndex());
      if (i != highestTPMCarrierMap.end())
        highestTPMCarriers = i->second;
    }

    if (!isFareMarketValid(*fareMarket, highestTPMCarriers))
    {
      // LOG4CXX_INFO(logger, "FareMarket is not for combinability");
      continue;
    }

    // using a vector instead of set in case order of item matters...
    std::vector<FareMarket*> allGovCxrFM; // of same O&D and ...
    MergedFareMarket* mfm = nullptr;
    _trx.dataHandle().get(mfm);
    copyFareMarketInfo(*fareMarket, *mfm);

    if (UNLIKELY(_brandCode && !((*_brandCode).empty())))
    {
      mfm->brandCode() = *_brandCode;
    }

    allGovCxrFM.push_back(fareMarket);

    // find all FM that matches all the TravelSegs
    std::vector<FareMarket*>::iterator it = fmIt;
    ++it;
    for (; it != fmItEnd; ++it)
    {
      FareMarket* fm = *it;

      if (!isFareMarketValid(*fm, highestTPMCarriers) ||
          checkRexCondition(*fm))
      {
        // LOG4CXX_INFO(logger, "FM not valid for combinability");
        alreadyConsideredFMSet.insert(fm);
        continue;
      }

      if (sameMarket(*fareMarket, *fm))
      {
      // LOG4CXX_INFO(logger, "Multi Gov Cxr exists- FM:" <<
      //              fareMarket->boardMultiCity() << "-" << fareMarket->offMultiCity()
      //               << " Cxr:" << fareMarket->governingCarrier() << "  FM:" <<
      //               fm->boardMultiCity() << "-" << fm->offMultiCity() <<
      //              " Cxr:" << fm->governingCarrier())
        allGovCxrFM.push_back(fm);
        alreadyConsideredFMSet.insert(fm);
      }
    } // end for

    mfm->cxrFarePreferred() = isCxrFarePreferred(allGovCxrFM);

    if (mergeFareMarket(allGovCxrFM, *mfm))
    {
      setTag2FareIndicator(*mfm);

      if (_trx.delayXpn())
      {
        mfm->findFirstFare(_trx.paxType());
      }

      if (fallback::fallbckFareMarketMergerRefactoring(&_trx))
      {
        if (UNLIKELY(_trx.getRequest()->isBrandedFaresRequest() || _trx.isBRAll()
                     || _trx.activationFlags().isSearchForBrandsPricing()))
        {
          // Calculate the final status taking statuses
          // from all fare markets that were used to create
          // this merged fare market
          for (size_t i = 0; i < mfm->mergedFareMarket().size(); ++i)
          {
            const FareMarket* fMarket = mfm->mergedFareMarket()[i];
            TSE_ASSERT(fMarket != nullptr);

            if (_trx.getRequest()->isBrandedFaresRequest() && (_brandCode != nullptr))
              mfm->updateIbfErrorMessage(fMarket->getStatusForBrand(*_brandCode, Direction::BOTHWAYS));
            else if (_trx.isBRAll() || _trx.activationFlags().isSearchForBrandsPricing())
            {
              // segOrder enumerates segments starting from 1, not 0
              const int16_t segmentIndex = _itin.segmentOrder(fMarket->travelSeg().front()) - 1;
              // Considering current pricing option space there's only one brand matching gov.carrier
              // of each merged fare market. We accumulate statuses from all merger fare markets
              // choosing the best status of them all ( as this is a 'choice' situation )
              if (!useDirectionality)
              {
                BrandCode brandCode =
                  _itin.getItinBranding().getBrandCode(
                      _brandIndex, segmentIndex, fMarket->governingCarrier(), Direction::BOTHWAYS);
                mfm->updateIbfErrorMessage(fMarket->getStatusForBrand(brandCode, Direction::BOTHWAYS));
              }
              else
              {
                IbfErrorMessageFmAcc bothwaysStatus;
                //we don't know direction of fare here, so we have to update status for brands
                //in both directions (if only BOTHWAYS is present it will be returned anyway)
                //we want "better" status, so IbfErrorMessageFmAcc will calculate it for us.
                BrandCode brandCode =
                  _itin.getItinBranding().getBrandCodeOrEmpty(
                      _brandIndex, segmentIndex, fMarket->governingCarrier(), Direction::ORIGINAL);
                if (!brandCode.empty())
                  bothwaysStatus.updateStatus(fMarket->getStatusForBrand(brandCode, Direction::ORIGINAL));
                brandCode = _itin.getItinBranding().getBrandCodeOrEmpty(
                    _brandIndex, segmentIndex, fMarket->governingCarrier(), Direction::REVERSED);
                if (!brandCode.empty())
                  bothwaysStatus.updateStatus(fMarket->getStatusForBrand(brandCode, Direction::REVERSED));

                mfm->updateIbfErrorMessage(bothwaysStatus.getStatus());
              }
            }
          }
        }
      }

      mergedFareMarketVect.push_back(mfm);

      if (_diagPtr)
        _diagPtr->printMFM(*mfm);
    }
  }

  if (_trx.getRequest()->originBasedRTPricing())
  {
    setTag2FareIndicatorForDummyFM(mergedFareMarketVect);
  }

  if(UNLIKELY(_diag993InCbsMode))
    _diagPtr->printMergerFooter();

  return true;
}

/*-------------------------------------------------------------------------
 *
 *------------------------------------------------------------------------*/
void
FareMarketMerger::setTag2FareIndicatorForDummyFM(
    std::vector<MergedFareMarket*>& mergedFareMarketVect)
{
  // find dummy fare market
  std::vector<MergedFareMarket*>::iterator mfmIt = mergedFareMarketVect.begin();
  std::vector<MergedFareMarket*>::iterator mfmEnd = mergedFareMarketVect.end();
  for (; mfmIt != mfmEnd; ++mfmIt)
  {
    if ((**mfmIt).mergedFareMarket().empty())
    {
      continue;
    }
    if ((**mfmIt).mergedFareMarket().front()->useDummyFare())
    {
      break;
    }
  }
  if (mfmIt == mfmEnd)
  {
    return; // nothing need to be done
  }
  MergedFareMarket& dummyMergedFM = **mfmIt;

  // find thru fare market opposite direction
  mfmIt = mergedFareMarketVect.begin();
  for (; mfmIt != mfmEnd; ++mfmIt)
  {
    if ((**mfmIt).boardMultiCity() == dummyMergedFM.offMultiCity() &&
        (**mfmIt).offMultiCity() == dummyMergedFM.boardMultiCity())
    {
      break;
    }
  }
  if (mfmIt == mfmEnd)
  {
    dummyMergedFM.tag2FareIndicator() = MergedFareMarket::Tag2FareIndicator::Absent;
    return; // nothing need to be done
  }
  dummyMergedFM.tag2FareIndicator() = (**mfmIt).tag2FareIndicator();

  return;
}

namespace
{
  void setGovCrxAndDirection(MergedFareMarket& mfm, FareMarket& fm)
  {
    if (LIKELY(std::find(mfm.governingCarrier().begin(),
                         mfm.governingCarrier().end(),
                         fm.governingCarrier()) == mfm.governingCarrier().end()))
    {
      mfm.governingCarrier().push_back(fm.governingCarrier());
    }

    if (fm.direction() == FMDirection::OUTBOUND)
    {
      mfm.outboundGovCxr() = fm.governingCarrier();
    }
    else if (fm.direction() == FMDirection::INBOUND)
    {
      mfm.inboundGovCxr() = fm.governingCarrier();
    }
  }

  IbfErrorMessage
  calculateSoldoutStatus(const std::vector<FareMarket*>& fareMarkets,
                         const BrandCode& brandCode)
  {
    IbfErrorMessageChoiceAcc fmStatus;
    for (const FareMarket* fm : fareMarkets)
    {
      fmStatus.updateStatus(fm->getStatusForBrand(brandCode, Direction::BOTHWAYS));
    }
    return fmStatus.getStatus();
  }

  bool
  isMultiBrandTransaction(PricingTrx& trx)
  {
    if (!(trx.getTrxType() == PricingTrx::MIP_TRX || trx.getTrxType() == PricingTrx::PRICING_TRX))
      return false;

    return trx.activationFlags().isSearchForBrandsPricing() ||  // Multiple Brands in Pricing
      trx.getRequest()->isBrandedFaresRequest() ||         // interline Branded Fares
      trx.isBRAll();                                       // TN Shopping Multiple Brands
  }

  void
  setUseAnyBrandOnFareMarket(MergedFareMarket& mfm,
                             Diag993Collector* diagPtr,
                             BrandCode& brandCode)
  {
    mfm.brandCode() = ANY_BRAND;
    if (UNLIKELY(diagPtr))
      diagPtr->writeUseAnyBrand(mfm, brandCode);
  }

}  // end unnamed namespace

void
FareMarketMerger::fillMergedFmDataFromFm(FareMarket& mergedFm, const FareMarket& fm)
{
  mergedFm.origin() = fm.origin();
  mergedFm.destination() = fm.destination();

  mergedFm.boardMultiCity() = fm.boardMultiCity();
  mergedFm.offMultiCity() = fm.offMultiCity();

  mergedFm.setGlobalDirection(fm.getGlobalDirection());
  mergedFm.geoTravelType() = fm.geoTravelType();
  mergedFm.travelBoundary() = fm.travelBoundary();

  mergedFm.travelSeg() = fm.travelSeg();

  mergedFm.sideTripTravelSeg() = fm.sideTripTravelSeg();
  mergedFm.governingCarrier() = fm.governingCarrier();
  mergedFm.governingCarrierPref() = fm.governingCarrierPref();

  mergedFm.fareBasisCode() = fm.fareBasisCode();

  if (fm.isMultiPaxUniqueFareBasisCodes())
  {
    if (!mergedFm.isMultiPaxUniqueFareBasisCodes())
      mergedFm.createMultiPaxUniqueFareBasisCodes();
    mergedFm.getMultiPaxUniqueFareBasisCodes() = fm.getMultiPaxUniqueFareBasisCodes();
  }

  mergedFm.fbcUsage() = fm.fbcUsage();
  mergedFm.fareCalcFareAmt() = fm.fareCalcFareAmt();

  mergedFm.inBoundAseanCurrencies() = fm.inBoundAseanCurrencies();
  mergedFm.outBoundAseanCurrencies() = fm.outBoundAseanCurrencies();
  mergedFm.travelDate() = fm.travelDate();
}

IbfErrorMessage
FareMarketMerger::mergeSoldoutStatuses(FareMarket* mergedFm,
                                       MergedFareMarket& mfm,
                                       const std::vector<FareMarket*>& fareMarkets)
{
  // IBF
  if (_trx.getRequest()->isBrandedFaresRequest() && _brandCode != nullptr)
  {
    IbfErrorMessage fmStatus = calculateSoldoutStatus(fareMarkets, *_brandCode);
    mergedFm->updateStatusForBrand(*_brandCode, Direction::BOTHWAYS, fmStatus);
    return fmStatus;
  }

  if (!_trx.isBRAll() && !_trx.activationFlags().isSearchForBrandsPricing())
    return IbfErrorMessage::IBF_EM_NOT_SET;

  // TN MULTIPLE BRANDS(BFA - BRALL) AND EXPEDIA(MULTIBRAND PRICING)
  FareMarket& firstFm = *(fareMarkets.front());
  const int16_t segmentIndex = _itin.segmentOrder(firstFm.travelSeg().front()) - 1;

  IbfErrorMessageChoiceAcc bestStatus;
  const bool useDirectionality = BrandingUtil::isDirectionalityToBeUsed(_trx);
  if (UNLIKELY(!useDirectionality))
  {
    for (const FareMarket* fm : fareMarkets)
    {
      BrandCode brandCode =
        _itin.getItinBranding().getBrandCode(_brandIndex, segmentIndex,
                                             fm->governingCarrier(), Direction::BOTHWAYS);

      IbfErrorMessage brandStatus = fm->getStatusForBrand(brandCode, Direction::BOTHWAYS);
      mergedFm->updateStatusForBrand(brandCode, Direction::BOTHWAYS, brandStatus);
      bestStatus.updateStatus(brandStatus);
    }
    return bestStatus.getStatus();
  }
  // Directional brands processing
  for (const FareMarket* fm : fareMarkets)
  {
    BrandCode brandDirectionOriginal = _itin.getItinBranding().getBrandCodeOrEmpty(_brandIndex,
                                                                                   segmentIndex, fm->governingCarrier(), Direction::ORIGINAL);
    BrandCode brandDirectionReversed = _itin.getItinBranding().getBrandCodeOrEmpty(_brandIndex,
                                                                                   segmentIndex, fm->governingCarrier(), Direction::REVERSED);
    if (!brandDirectionOriginal.empty())
    {
      IbfErrorMessage directionStatus =
        fm->getStatusForBrand(brandDirectionOriginal, Direction::ORIGINAL);
      mergedFm->updateStatusForBrand(brandDirectionOriginal, Direction::ORIGINAL, directionStatus);
      mergedFm->updateStatusForBrand(brandDirectionOriginal, Direction::BOTHWAYS, directionStatus, true);
      bestStatus.updateStatus(directionStatus);
    }
    if (!brandDirectionReversed.empty())
    {
      IbfErrorMessage directionStatus =
        fm->getStatusForBrand(brandDirectionReversed, Direction::REVERSED);
      mergedFm->updateStatusForBrand(brandDirectionReversed, Direction::REVERSED, directionStatus);
      mergedFm->updateStatusForBrand(brandDirectionReversed, Direction::BOTHWAYS, directionStatus, true);
      bestStatus.updateStatus(directionStatus);
    }
  }
  return bestStatus.getStatus();
}

bool
FareMarketMerger::copyFaresToMergedFareMarket(FareMarket& fm,
                                              FareMarket& mergedFM,
                                              MergedFareMarket& mfm,
                                              Diag993Collector* diagPtr)
{
  if (UNLIKELY(diagPtr))
    diagPtr->displayFailedFlexFaresHeader(fm, _groupId);

  addPaxTypeFare(fm, mergedFM, mfm);

  if (UNLIKELY(diagPtr))
    diagPtr->displayPassedFlexFareCount(mergedFM);

  if (UNLIKELY(!fareMarketHasValidFares(fm, mfm.brandCode())))
    return false;

  setGovCrxAndDirection(mfm,fm);
  return true;
}
/*-------------------------------------------------------------------------
 *
 *------------------------------------------------------------------------*/
bool
FareMarketMerger::mergeFareMarket(std::vector<FareMarket*>& allGovCxrFM,
                                  MergedFareMarket& mfm)
{
  if(fallback::fallbckFareMarketMergerRefactoring(&_trx))
    return mergeFareMarket_old(allGovCxrFM, mfm);

  //-----------------------------------------------------------------------------------------------

  LOG4CXX_INFO(logger, "Merged FareMarket: " << mfm.boardMultiCity() << "-" << mfm.offMultiCity());

  if (UNLIKELY(_diag993InCbsMode))
    _diagPtr->displayInputFareMarkets(allGovCxrFM);

  if (UNLIKELY(allGovCxrFM.empty()))
    return false;

  bool validFaresFound(false);

  // lint --e{413}
  FareMarket* mergedFM = nullptr;
  _trx.dataHandle().get(mergedFM);
  TSE_ASSERT(mergedFM != nullptr);

  FareMarket& firstFm = *allGovCxrFM.front();

  // NOT ALL the members of FareMarket are being copied to the
  // MergedFareMarket. We will copy more members if we need more.
  fillMergedFmDataFromFm(*mergedFM, firstFm);

  // IBF, BFA, PRICE MULTIPLE BRANDS(EXPEDIA)
  if (isMultiBrandTransaction(_trx))
  {
    IbfErrorMessage soldoutStatus = mergeSoldoutStatuses(mergedFM, mfm, allGovCxrFM);
    mfm.updateIbfErrorMessage(soldoutStatus);
    if (_diag993InCbsMode)
      _diagPtr->displaySoldoutStatus(soldoutStatus);

    if (_trx.getRequest()->isBrandedFaresRequest() &&
        useAnyBrandWhenMerging(mfm, soldoutStatus))
    {
      // IBF: CBS : If original brand is sold out('A') or not offered('O'), use fares from ANY_BRAND
      setUseAnyBrandOnFareMarket(mfm, (_diag993InCbsMode ? _diagPtr : nullptr), *_brandCode);
    }
  }

  for (FareMarket* fm : allGovCxrFM)
  {
    validFaresFound |= copyFaresToMergedFareMarket(*fm, *mergedFM, mfm, _diagPtr);
  }

  if (UNLIKELY(_diag993InCbsMode && !validFaresFound))
    _diagPtr->displayNoValidFaresForBrand(*_brandCode);

  // IBF: CBS="X": If original brand has no fares at all, use fares from ANY_BRAND, set soldout 'X'
  if (!validFaresFound &&
      _trx.getRequest()->isUseCbsForNoFares() &&
      changeBrandForSoldoutAllowedForLeg(mfm))
  {
    if (UNLIKELY(_diag993InCbsMode))
      _diagPtr->displayCbsX();

    setUseAnyBrandOnFareMarket(mfm, (_diag993InCbsMode ? _diagPtr : nullptr), *_brandCode);
    mergedFM->forceStatusForBrand(*_brandCode, Direction::BOTHWAYS,
                                  IbfErrorMessage::IBF_EM_NO_FARE_FILED);

    for (FareMarket* fm : allGovCxrFM)
    {
      fm->forceStatusForBrand(*_brandCode, Direction::BOTHWAYS,
                              IbfErrorMessage::IBF_EM_NO_FARE_FILED);

      validFaresFound |= copyFaresToMergedFareMarket(*fm, *mergedFM, mfm);
    }
  }

  for (PaxTypeBucket& cortege : firstFm.paxTypeCortege())
  {
    PaxTypeBucket* mergedCortege = mergedFM->paxTypeCortege(cortege.requestedPaxType());
    TSE_ASSERT(mergedCortege);

    mergedCortege->mutableCxrPrecalculatedTaxes() = cortege.cxrPrecalculatedTaxes();
  }

  if (validFaresFound)
  {
    if (mfm.cxrFarePreferred())
      getAllCxrFareTypes(mfm, *mergedFM);

    if (mfm.cxrFarePreferred() || allGovCxrFM.size() > 1)
    {
      sortFareMarket(mfm, *mergedFM);
    }

    mfm.mergedFareMarket().push_back(mergedFM);
  }

  if (!mfm.outboundGovCxr().empty() && !mfm.inboundGovCxr().empty())
  {
    mfm.direction() = FMDirection::UNKNOWN;
  }

  return validFaresFound;
}

bool
FareMarketMerger::mergeFareMarket_old(std::vector<FareMarket*>& allGovCxrFM,
                                      MergedFareMarket& mfm)
{
  LOG4CXX_INFO(logger, "Merged FareMarket: " << mfm.boardMultiCity() << "-" << mfm.offMultiCity());

  const bool useDirectionality = BrandingUtil::isDirectionalityToBeUsed(_trx);
  bool validFaresFound(false);

  std::vector<FareMarket*>::iterator it = allGovCxrFM.begin();

  if (UNLIKELY(allGovCxrFM.empty()))
  {
    return false;
  }

  // lint --e{413}
  FareMarket* mergedFM = nullptr;
  _trx.dataHandle().get(mergedFM);

  FareMarket& farM = *allGovCxrFM.front();

  // NOT ALL the members of FareMarket are being copied to the
  // MergedFareMarket. We will copy more members if we need more.
  //
  mergedFM->origin() = farM.origin();
  mergedFM->destination() = farM.destination();

  mergedFM->boardMultiCity() = farM.boardMultiCity();
  mergedFM->offMultiCity() = farM.offMultiCity();

  mergedFM->setGlobalDirection(farM.getGlobalDirection());
  mergedFM->geoTravelType() = farM.geoTravelType();
  mergedFM->travelBoundary() = farM.travelBoundary();

  mergedFM->travelSeg() = farM.travelSeg();

  mergedFM->sideTripTravelSeg() = farM.sideTripTravelSeg();
  mergedFM->governingCarrier() = farM.governingCarrier();
  mergedFM->governingCarrierPref() = farM.governingCarrierPref();

  mergedFM->fareBasisCode() = farM.fareBasisCode();

  if (farM.isMultiPaxUniqueFareBasisCodes())
  {
    if (!mergedFM->isMultiPaxUniqueFareBasisCodes())
      mergedFM->createMultiPaxUniqueFareBasisCodes();
    mergedFM->getMultiPaxUniqueFareBasisCodes() = farM.getMultiPaxUniqueFareBasisCodes();
  }

  mergedFM->fbcUsage() = farM.fbcUsage();
  mergedFM->fareCalcFareAmt() = farM.fareCalcFareAmt();

  mergedFM->inBoundAseanCurrencies() = farM.inBoundAseanCurrencies();
  mergedFM->outBoundAseanCurrencies() = farM.outBoundAseanCurrencies();
  mergedFM->travelDate() = farM.travelDate();

  if (_trx.getTrxType() == PricingTrx::MIP_TRX || _trx.activationFlags().isSearchForBrandsPricing())
  {
    if (UNLIKELY(_trx.getRequest()->isBrandedFaresRequest()) && (_brandCode != nullptr))
    {
      //Fare Market Merger works in context of a one particular brand.
      //Hence we're interested only in that brand and there's no need to copy all statuses
      IbfErrorMessage ibfErrorMessage = farM.getStatusForBrand(*_brandCode, Direction::BOTHWAYS);
      mergedFM->forceStatusForBrand(*_brandCode, Direction::BOTHWAYS, ibfErrorMessage);

      if (useAnyBrandWhenMerging(mfm, ibfErrorMessage)) // soldout condition, Context shopping etc
      {
        mfm.brandCode() = ANY_BRAND;
        if (_diagPtr)
          _diagPtr->writeUseAnyBrand(mfm, *_brandCode);
      }
    }
    else if (UNLIKELY(_trx.isBRAll() || _trx.activationFlags().isSearchForBrandsPricing()))
    {
      const int16_t segmentIndex = _itin.segmentOrder(farM.travelSeg().front()) - 1;
      // there's only one Brand within a given pricingOptionSpace that matches fm's govCarrier
      // so we only copy status of this one brand to the new fare market.
      if (!useDirectionality)
      {
        BrandCode brandCode =
          _itin.getItinBranding().getBrandCode(
              _brandIndex, segmentIndex, farM.governingCarrier(), Direction::BOTHWAYS);

        IbfErrorMessage ibfErrorMessage = farM.getStatusForBrand(brandCode, Direction::BOTHWAYS);
        mergedFM->forceStatusForBrand(brandCode, Direction::BOTHWAYS, ibfErrorMessage);
      }
      else
      {
        IbfErrorMessageFmAcc bothwaysStatus;
        //we don't know direction of fare here, so we have to update status for brands
        //in both directions (if only BOTHWAYS is present it will be returned anyway)
        //we want "better" status, so IbfErrorMessageFmAcc will calculate it for us.
        BrandCode brandCode = _itin.getItinBranding().getBrandCodeOrEmpty(
            _brandIndex, segmentIndex, farM.governingCarrier(), Direction::ORIGINAL);
        if (!brandCode.empty())
        {
          bothwaysStatus.updateStatus(farM.getStatusForBrand(brandCode, Direction::ORIGINAL));
          mergedFM->forceStatusForBrand(brandCode, Direction::ORIGINAL,
                                        farM.getStatusForBrand(brandCode, Direction::ORIGINAL));
        }
        brandCode = _itin.getItinBranding().getBrandCodeOrEmpty(
            _brandIndex, segmentIndex, farM.governingCarrier(), Direction::REVERSED);
        if (!brandCode.empty())
        {
          mergedFM->forceStatusForBrand(brandCode, Direction::REVERSED,
                                        farM.getStatusForBrand(brandCode, Direction::REVERSED));
          bothwaysStatus.updateStatus(farM.getStatusForBrand(brandCode, Direction::REVERSED));
        }
        //TODO(andrzej.fediuk) DIR: update only "old" (bothways) vectors - backward
        //compatibility for IBF. should be refactored
        mergedFM->forceStatusForBrand(
            brandCode, Direction::BOTHWAYS, bothwaysStatus.getStatus(), true);
      }
    }
  }

  for (FareMarket* fm : allGovCxrFM)
  {
    if (UNLIKELY(_diagPtr))
      _diagPtr->displayFailedFlexFaresHeader(*fm, _groupId);

    addPaxTypeFare(*fm, *mergedFM, mfm);

    if (UNLIKELY(_diagPtr))
      _diagPtr->displayPassedFlexFareCount(*mergedFM);

    if (fareMarketHasValidFares(*fm, mfm.brandCode()))
    {
      validFaresFound = true;
      setGovCrxAndDirection(mfm,*fm);
    }
  }

  if (!validFaresFound &&
      _trx.getRequest()->isUseCbsForNoFares() &&
      changeBrandForSoldoutAllowedForLeg(mfm))
  {
    mfm.brandCode() = ANY_BRAND;
    if (_diagPtr)
      _diagPtr->writeUseAnyBrand(mfm, *_brandCode);

    mergedFM->forceStatusForBrand(*_brandCode, Direction::BOTHWAYS,
                                  IbfErrorMessage::IBF_EM_NO_FARE_FILED);

    for (FareMarket* fm : allGovCxrFM)
    {
      fm->forceStatusForBrand(*_brandCode, Direction::BOTHWAYS,
                             IbfErrorMessage::IBF_EM_NO_FARE_FILED);

      addPaxTypeFare(*fm, *mergedFM, mfm);

      if (fareMarketHasValidFares(*fm, ANY_BRAND))
      {
        validFaresFound = true;
        setGovCrxAndDirection(mfm,*fm);
      }
    }
  }

  for (PaxTypeBucket& cortege : farM.paxTypeCortege())
  {
    PaxTypeBucket* mergedCortege = mergedFM->paxTypeCortege(cortege.requestedPaxType());
    TSE_ASSERT(mergedCortege);

    mergedCortege->mutableCxrPrecalculatedTaxes() = cortege.cxrPrecalculatedTaxes();
  }

  if (validFaresFound)
  {
    if (mfm.cxrFarePreferred())
      getAllCxrFareTypes(mfm, *mergedFM);

    if (mfm.cxrFarePreferred() || allGovCxrFM.size() > 1)
    {
      sortFareMarket(mfm, *mergedFM);
    }

    mfm.mergedFareMarket().push_back(mergedFM);
  }

  if (!mfm.outboundGovCxr().empty() && !mfm.inboundGovCxr().empty())
  {
    mfm.direction() = FMDirection::UNKNOWN;
  }

  return validFaresFound;
}

bool
FareMarketMerger::changeBrandForSoldoutAllowedForLeg(const MergedFareMarket& mfm) const
{
  if (!_trx.isContextShopping())
    return true;

  if (skipper::TNBrandsFunctions::isAnySegmentOnCurrentlyShoppedLeg(mfm.travelSeg(),
                                                                    _trx.getFixedLegs()))
    return false;

  return true;
}

bool
FareMarketMerger::useAnyBrandWhenMerging(const MergedFareMarket& mfm, IbfErrorMessage status) const
{
  if (_trx.isContextShopping() &&
      skipper::TNBrandsFunctions::isAnySegmentOnFixedLeg(mfm.travelSeg(), _trx.getFixedLegs()))
    return true;

  if (!changeBrandForSoldoutAllowedForLeg(mfm))
  {
    if (_diag993InCbsMode)
      _diagPtr->displayCbsNotAllowedForLeg();

    return false;
  }

  if (!_trx.getRequest()->isChangeSoldoutBrand())
    return false;

  if ((status == IbfErrorMessage::IBF_EM_NOT_AVAILABLE) || (status == IbfErrorMessage::IBF_EM_NOT_OFFERED))
    return true;

  return false;
}

bool
FareMarketMerger::fareMarketHasValidFares(const FareMarket& fm, const BrandCode& brandCode) const
{
  // determine if we have any valid fares
  if (PricingTrx::IS_TRX == _trx.getTrxType())
  {
    bool result(false);
    const std::vector<PaxTypeFare*>& fares(fm.allPaxTypeFare());
    for (const auto fare : fares)
    {
      if (fare->isValidForPricing())
      {
        result = true;
        break;
      }
    }
    return result;
  }
  else if (PricingTrx::MIP_TRX ==_trx.getTrxType())
  {
    if (_trx.getRequest()->isBrandedFaresRequest() && _trx.getRequest()->isUseCbsForNoFares())
    {
      const std::vector<PaxTypeFare*>& fares(fm.allPaxTypeFare());
      for (const auto fare : fares)
      {
        if (fare->isValidForBrand(_trx, &brandCode))
        {
          if (_diag993InCbsMode)
            _diagPtr->displayValidFareFound(fare->fareClass(), brandCode);

          return true;
        }
      }

      return false;
    }
  }

  return true;
}

void
FareMarketMerger::copyPaxTypeFares(FareMarket& fromFM,
                                   PaxTypeBucket& fromCortege,
                                   PaxTypeBucket& toCortege,
                                   skipper::CarrierBrandPairs& carriersBrands,
                                   bool& hardPassExists,
                                   const uint16_t paxTypeNum,
                                   const BrandCode* brandCode)
{
  bool isFlownFM = (fromFM.changeStatus() == FL);
  bool isRexTrxNewPhase = _trx.excTrxType() == PricingTrx::AR_EXC_TRX &&
                          static_cast<const RexPricingTrx&>(_trx).trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE;

  if (UNLIKELY(isFlownFM && isRexTrxNewPhase))
  {
    RexPaxTypeFareValidator rexPTFValidator(static_cast<RexPricingTrx&>(_trx));
    ValidPaxTypeFareCopier validPTFCopier(_trx, &_itin, paxTypeNum, toCortege.paxTypeFare(),
                                          carriersBrands, _carrier, &rexPTFValidator, _brandIndex,
                                          brandCode, 0, nullptr, &hardPassExists);

    for_each(fromCortege.paxTypeFare().begin(),
             fromCortege.paxTypeFare().end(),
             validPTFCopier);
  }
  else
  {
    ValidPaxTypeFareCopier validPTFCopier(_trx,
                                          &_itin,
                                          paxTypeNum,
                                          toCortege.paxTypeFare(),
                                          carriersBrands,
                                          _carrier,
                                          nullptr,
                                          _brandIndex,
                                          brandCode,
                                          _groupId,
                                          _diagPtr,
                                          &hardPassExists);

    for_each(fromCortege.paxTypeFare().begin(),
             fromCortege.paxTypeFare().end(),
             validPTFCopier);
  }
}

/*---- --------------------------------------------------------------------
 *
 *------------------------------------------------------------------------*/
bool
FareMarketMerger::addPaxTypeFare(FareMarket& fromFM, FareMarket& toFM, MergedFareMarket& mfm)
{
  // LOG4CXX_INFO(logger, "Merging FareMarket: " << fromFM.boardMultiCity() << "-"
  //                      << fromFM.governingCarrier() << "-" << fromFM.offMultiCity());

  skipper::CarrierBrandPairs carriersBrands;
  if (UNLIKELY(_trx.isBRAll() || _trx.activationFlags().isSearchForBrandsPricing()))
  {
    const TravelSeg* segment = mfm.travelSeg()[0];
    carriersBrands = _itin.getItinBranding().getCarriersBrandsForSegment(_brandIndex, segment);
  }

  std::vector<PaxTypeBucket>::iterator fromCortIt = fromFM.paxTypeCortege().begin();
  std::vector<PaxTypeBucket>::iterator fromCortItEnd = fromFM.paxTypeCortege().end();

  BrandCode* brandCode = &mfm.brandCode();

  uint16_t paxTypeNum = 0;
  for (; fromCortIt != fromCortItEnd; ++fromCortIt, ++paxTypeNum)
  {
    PaxTypeBucket& fromCortege = *fromCortIt;

    LOG4CXX_INFO(logger,
                 "Looking for PaxType Cort of = " << fromCortege.requestedPaxType()->paxType());

    std::vector<PaxTypeBucket>::iterator toCortIt = toFM.paxTypeCortege().begin();
    std::vector<PaxTypeBucket>::iterator toCortItEnd = toFM.paxTypeCortege().end();
    bool found = false;
    for (; toCortIt != toCortItEnd; ++toCortIt)
    {
      PaxTypeBucket& toCortege = *toCortIt;

      if (fromCortege.requestedPaxType() == toCortege.requestedPaxType())
      {
        found = true;
        copyPaxTypeFares(fromFM, fromCortege, toCortege, carriersBrands,
                         mfm.hardPassExists(), paxTypeNum, brandCode);

        // LOG4CXX_INFO(logger, "FOUND PaxType Cort of = "
        //                     << fromCortege.requestedPaxType()->paxType());
        // LOG4CXX_INFO(logger, "After merge paxTypeFare size = "
        //                     << toCortege.paxTypeFare().size());

        break;
      }
    }

    if (!found)
    {
      // LOG4CXX_INFO(logger, "NOT FOUND PaxType Cort of = "
      //                    << fromCortege.requestedPaxType()->paxType() << ", Adding...");

      PaxTypeBucket newCortege;
      newCortege.paxTypeFare().reserve(fromCortege.paxTypeFare().size());
      toFM.paxTypeCortege().push_back(newCortege);

      PaxTypeBucket& toCortege = toFM.paxTypeCortege().back();
      copyPaxTypeBucketInfo(fromCortege, toCortege);

      copyPaxTypeFares(fromFM, fromCortege, toCortege, carriersBrands,
                       mfm.hardPassExists(), paxTypeNum, brandCode);
    }
  }

  return true;
}

/*-------------------------------------------------------------------------
 *
 *------------------------------------------------------------------------*/
void
FareMarketMerger::copyFareMarketInfo(FareMarket& fm, MergedFareMarket& mfm)
{
  mfm.origin() = fm.origin();
  mfm.destination() = fm.destination();

  mfm.boardMultiCity() = fm.boardMultiCity();
  mfm.offMultiCity() = fm.offMultiCity();

  mfm.globalDirection() = fm.getGlobalDirection();
  mfm.geoTravelType() = fm.geoTravelType();
  mfm.travelBoundary() = fm.travelBoundary();

  mfm.travelSeg() = fm.travelSeg();

  mfm.sideTripTravelSeg() = fm.sideTripTravelSeg();

  if (ShoppingTrx* trx = dynamic_cast<ShoppingTrx*>(&_trx))
  {
    uint32_t a, b;
    const bool res = ShoppingUtil::getFareMarketLegIndices(*trx, &fm, a, b);
    TSE_ASSERT(res);
    mfm.setSegNums(a + 1, b + 1);
  }
  else
  {
    mfm.setSegNums(_itin.segmentOrder(fm.travelSeg().front()),
                   _itin.segmentOrder(fm.travelSeg().back()));
  }
}

/*-------------------------------------------------------------------------
 *
 *------------------------------------------------------------------------*/
void
FareMarketMerger::copyPaxTypeBucketInfo(PaxTypeBucket& fromCortege, PaxTypeBucket& toCortege)
{
  toCortege.requestedPaxType() = fromCortege.requestedPaxType();
  toCortege.paxIndex() = fromCortege.paxIndex();
  toCortege.actualPaxType() = fromCortege.actualPaxType();
  toCortege.inboundCurrency() = fromCortege.inboundCurrency();
  toCortege.outboundCurrency() = fromCortege.outboundCurrency();
  toCortege.fareQuoteOverrideCurrency() = fromCortege.fareQuoteOverrideCurrency();
  toCortege.setMarketCurrencyPresent(fromCortege.isMarketCurrencyPresent());
}

/*-------------------------------------------------------------------------
 *
 *------------------------------------------------------------------------*/
bool
FareMarketMerger::sameMarket(const FareMarket& fm1, const FareMarket& fm2)
{
  const std::vector<TravelSeg*>& travelSegVect1 = fm1.travelSeg();
  const std::vector<TravelSeg*>& travelSegVect2 = fm2.travelSeg();

  if (!sameTravelSegs(travelSegVect1, travelSegVect2))
  {
    return false;
  }

  // check Side Trip
  //

  if (fm1.sideTripTravelSeg().empty() && fm2.sideTripTravelSeg().empty())
  {
    return true;
  }

  if (fm1.sideTripTravelSeg().size() != fm2.sideTripTravelSeg().size())
    return false;

  std::vector<std::vector<TravelSeg*> >::const_iterator it1 = fm1.sideTripTravelSeg().begin();
  std::vector<std::vector<TravelSeg*> >::const_iterator it1End = fm1.sideTripTravelSeg().end();
  std::vector<std::vector<TravelSeg*> >::const_iterator it2 = fm2.sideTripTravelSeg().begin();
  for (; it1 != it1End; ++it1, ++it2)
  {
    if (!sameTravelSegs(*it1, *it2))
    {
      return false;
    }
  }

  return true;
}

/*-------------------------------------------------------------------------
 *
 *------------------------------------------------------------------------*/
bool
FareMarketMerger::sameTravelSegs(const std::vector<TravelSeg*>& travelSegVect1,
                                 const std::vector<TravelSeg*>& travelSegVect2)
{
  if (travelSegVect1.size() != travelSegVect2.size())
    return false;

  if (travelSegVect1 == travelSegVect2)
  {
    // if pointers match no need to go through board and off points matching
    // for Shopping pointers do not match
    return true;
  }

  std::vector<TravelSeg*>::const_iterator ts1It = travelSegVect1.begin();
  std::vector<TravelSeg*>::const_iterator ts1ItEnd = travelSegVect1.end();
  std::vector<TravelSeg*>::const_iterator ts2It = travelSegVect2.begin();
  std::vector<TravelSeg*>::const_iterator ts2ItEnd = travelSegVect2.end();

  for (; ts1It != ts1ItEnd && ts2It != ts2ItEnd; ++ts1It, ++ts2It)
  {
    TravelSeg& ts1 = *(*ts1It);
    TravelSeg& ts2 = *(*ts2It);

    if (ts1.boardMultiCity() != ts2.boardMultiCity() || ts1.offMultiCity() != ts2.offMultiCity() ||
        _itin.segmentOrder(&ts1) != _itin.segmentOrder(&ts2))
    {
      return false;
    }
  }
  if (ts1It == ts1ItEnd && ts2It == ts2ItEnd)
  {
    return true;
  }

  return false;
}

/*-------------------------------------------------------------------------
 *
 *------------------------------------------------------------------------*/
bool
FareMarketMerger::isFareMarketValid(const FareMarket& fareMarket,
                                    std::set<CarrierCode>& highestTPMCarriers)
{
  NoPNRPricingTrx* noPNRTrx = dynamic_cast<NoPNRPricingTrx*>(&_trx);

  if (UNLIKELY(noPNRTrx && noPNRTrx->isNoMatch() &&
                !fareMarket.isFareTypeGroupValid(noPNRTrx->processedFTGroup())))
    return false;

  // see if it's specified that we should only use fare markets with
  // a certain governing carrier
  if (_carrier)
  {
    bool isProcessedCarrier = (fareMarket.governingCarrier() == *_carrier);
    bool isHighTPMCarrier = highestTPMCarriers.count(fareMarket.governingCarrier());

    if (!isProcessedCarrier && !isHighTPMCarrier)
      return false;
  }

  // if carrier is not allow to combine to other carrier, we should not
  // use its fm if carrier is not specified
  if (UNLIKELY(_carrier == nullptr && fareMarket.combineSameCxr()))
  {
    return false;
  }

  ErrorResponseException::ErrorResponseCode failCode = fareMarket.failCode();
  if (failCode != ErrorResponseException::NO_ERROR)
  {
    // Invalid FareMarket, go to the next
    return false;
  }

  if (fareMarket.breakIndicator())
  {
    // LOG4CXX_INFO(logger, "breakIndicator is true, not for combinability");
    return false;
  }

  if (UNLIKELY(_trx.isBRAll() || _trx.activationFlags().isSearchForBrandsPricing()))
  {
    // it happens that when building pricing option spaces we exclude a given carrier from some of them
    // ( for example if it doesn't have brands we want it to appear only in one space )
    // therefore we don't want to use that carrier's fare markets in Pricing a given space at all
    const int16_t segmentIndex = _itin.segmentOrder(fareMarket.travelSeg().front()) - 1;
    if (!_itin.getItinBranding().isCarrierValidForSpaceSegment(_brandIndex,
                                                               segmentIndex,
                                                               fareMarket.governingCarrier()))
    {
      return false;
    }
  }

  return true;
}

/*-------------------------------------------------------------------------
 * In case of Multiple Gov-Cxr(in other words multiple FareMarket for same O&D),
 * if one one Cxr preferrs Cxr-Fare , then the MergedFareMarket will be
 * considered as cxrFarePreferred, therefore, must try Cxr Fare first even if it
 * not cheaper.
 *------------------------------------------------------------------------*/
bool
FareMarketMerger::isCxrFarePreferred(std::vector<FareMarket*>& allGovCxrFM)
{
  for (const FareMarket* fm : allGovCxrFM)
  {
    if (fm->geoTravelType() != GeoTravelType::International)
    {
      // Domestic and F. Domestic does not have YY vs CXR issue
      return false;
    }

    // LOG4CXX_ERROR(logger, "FARE Market: "  << fm->boardMultiCity() << "-"  <<
    // fm->governingCarrier()
    //                                       << "-" << fm->globalDirection() << "-" <<
    // fm->offMultiCity())
    const std::vector<const IndustryPricingAppl*>& yyPriceAppl =
        _trx.dataHandle().getIndustryPricingAppl(
            fm->governingCarrier(), fm->getGlobalDirection(), fm->travelSeg().front()->departureDT());

    const Loc& fmOrig = *(fm->origin());
    const Loc& fmDest = *(fm->destination());

    // LOG4CXX_INFO(logger, "IndustryPricingAppl vect SIZE=" << yyPriceAppl.size());

    // primePricingAppl == 'C' means use Carrier Fare first
    // primePricingAppl == 'L' means use lowest Fare first
    //
    bool found = false;
    std::vector<const IndustryPricingAppl*>::const_iterator ipaIt = yyPriceAppl.begin();
    std::vector<const IndustryPricingAppl*>::const_iterator ipaItEnd = yyPriceAppl.end();
    for (; ipaIt != ipaItEnd; ++ipaIt)
    {
      const IndustryPricingAppl& industryPricingAppl = *(*ipaIt);
      if (industryPricingAppl.carrier().empty())
      {
        // default entry will be checked at the end
        continue;
      }

      Directionality dir = industryPricingAppl.directionality();
      LocCode locCode1 = industryPricingAppl.loc1().loc();
      LocCode locCode2 = industryPricingAppl.loc2().loc();
      LocTypeCode locTypeCode1 = industryPricingAppl.loc1().locType();
      LocTypeCode locTypeCode2 = industryPricingAppl.loc2().locType();

      /****
        LOG4CXX_INFO(logger, "fmOrig="<< fmOrig.loc()
                             <<" subarea =" << fmOrig.subarea()
                             <<" fmDest="<<fmDest.loc()
                 <<" subarea fmDest="<< fmDest.subarea());
        LOG4CXX_INFO(logger, "locCode1="<<locCode1
                             <<" locCode2="<<locCode2
                                 <<" locTypeCode1="<<locTypeCode1
                 <<" locTypeCode2="<<locTypeCode2);
       ***/

      bool origMatched = false;
      bool destMatched = false;

      if (UNLIKELY(dir == TO))
      { // swap
        LocCode tmpLC = locCode1;
        locCode1 = locCode2;
        locCode2 = tmpLC;

        LocTypeCode tmpLTC = locTypeCode1;
        locTypeCode1 = locTypeCode2;
        locTypeCode2 = tmpLTC;
      }

      if (LocUtil::isInLoc(fmOrig, locTypeCode1, locCode1))
      {
        origMatched = true;
        // if(locTypeCode1 == ' ')       // asumption: both locTypeCode and locCode
        //   origEmptyMatched = true;    // will be empty simulteniously? verify...
      }
      if (LocUtil::isInLoc(fmDest, locTypeCode2, locCode2))
      {
        destMatched = true;
      }

      if ((dir == BOTH || dir == BETWEEN) && !(origMatched && destMatched))
      {
        // try other way, that might match
        //
        origMatched = false;
        destMatched = false;

        if (LocUtil::isInLoc(fmOrig, locTypeCode2, locCode2))
        {
          origMatched = true;
        }
        if (LocUtil::isInLoc(fmDest, locTypeCode1, locCode1))
        {
          destMatched = true;
        }
      }

      if (origMatched && destMatched)
      {
        found = true;
        if (industryPricingAppl.primePricingAppl() == CXR_FARE_PREF_IND)
        {
          // LOG4CXX_ERROR(logger, "MATCH, IndustryPricingAppl::primePricingAppl == 'C' ");
          return true;
        }
        break;
      }
    }

    // No entry for this CXR, use the default one, returned as the last entry of the query result
    //
    if (!found && (!yyPriceAppl.empty()) && yyPriceAppl.back()->carrier().empty() &&
        yyPriceAppl.back()->primePricingAppl() == CXR_FARE_PREF_IND)
    {
      // LOG4CXX_ERROR(logger, "Default IndustryPricingAppl::primePricingAppl == 'C' ");
      return true;
    }
  }

  // LOG4CXX_ERROR(logger, "CXR pref is 'L' ");

  return false;
}

/*-------------------------------------------------------------------------
 *
 *------------------------------------------------------------------------*/
void
FareMarketMerger::getAllCxrFareTypes(MergedFareMarket& mfm, FareMarket& fm)
{
  std::map<const PaxType*, std::map<FareType,  std::map<CarrierCode, MergedFareMarket::CxrFareTypeBitSet> > >&
  allCxrFareTypes = mfm.allCxrFareTypes();

  const GeoTravelType geoTravelType = fm.geoTravelType();

  for (PaxTypeBucket& paxTypeCortege : fm.paxTypeCortege())
  {
    PaxType* pax = paxTypeCortege.requestedPaxType();

    std::map<const PaxType*, std::map<FareType, std::map<CarrierCode, MergedFareMarket::CxrFareTypeBitSet> > >::iterator
    it = allCxrFareTypes.find(pax);

    std::set<CarrierCode> govCarriers;

    if (LIKELY(it == allCxrFareTypes.end()))
    {
      std::map<FareType, std::map<CarrierCode, MergedFareMarket::CxrFareTypeBitSet> > fareTypeMap;

      addCxrFareTypes(paxTypeCortege, fareTypeMap, geoTravelType, govCarriers);

      if (!fareTypeMap.empty())
      {
        allCxrFareTypes.insert(std::map<
            const PaxType*,
            std::map<FareType, std::map<CarrierCode, MergedFareMarket::CxrFareTypeBitSet> > >::value_type(pax,
                                                                                  fareTypeMap));
        mfm.cxrFareExist() = true;
      }
    }
    else
    {
      std::map<FareType, std::map<CarrierCode, MergedFareMarket::CxrFareTypeBitSet> >& fareTypeMap = it->second;
      addCxrFareTypes(paxTypeCortege, fareTypeMap, geoTravelType, govCarriers);
    }
  }
}

void
FareMarketMerger::addCxrFareTypes(
    PaxTypeBucket& paxTypeCortege,
    std::map<FareType, std::map<CarrierCode, MergedFareMarket::CxrFareTypeBitSet>>& fareTypeMap,
    const GeoTravelType geoTravelType,
    std::set<CarrierCode>& govCxr)
{
  for(const PaxTypeFare* ptf: paxTypeCortege.paxTypeFare())
  {
    const PaxTypeFare& paxTypeFare = *ptf;

    if (paxTypeFare.carrier() == INDUSTRY_CARRIER)
    {
      continue;
    }

    MergedFareMarket::CxrFareTypeBitSet bitSet;

    if (paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED) // Tag-1
    {
      if (paxTypeFare.directionality() == FROM)
        bitSet.set(MergedFareMarket::OB_ALL_TRIP_TYPE);
      else if (paxTypeFare.directionality() == TO)
        bitSet.set(MergedFareMarket::IB_ALL_TRIP_TYPE);
      else if (paxTypeFare.directionality() == BOTH)
        bitSet.set(MergedFareMarket::BothDir_ALL_TRIP_TYPE);
    }
    else if (paxTypeFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED) // Tag-2
    {
      if (paxTypeFare.directionality() == FROM)
        bitSet.set(MergedFareMarket::OB_OJRC_TRIP_TYPE);
      else if (LIKELY(paxTypeFare.directionality() == TO))
        bitSet.set(MergedFareMarket::IB_OJRC_TRIP_TYPE);
      else if (paxTypeFare.directionality() == BOTH)
        bitSet.set(MergedFareMarket::BothDir_ALL_TRIP_TYPE);
    }
    else if (LIKELY(paxTypeFare.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED)) // Tag-3
    {
      if (LIKELY(geoTravelType == GeoTravelType::International || geoTravelType == GeoTravelType::ForeignDomestic))
      {
        if (paxTypeFare.directionality() == FROM)
          bitSet.set(MergedFareMarket::OB_OW_TRIP_TYPE);
        else if (LIKELY(paxTypeFare.directionality() == TO))
          bitSet.set(MergedFareMarket::IB_OW_TRIP_TYPE);
        else if (paxTypeFare.directionality() == BOTH)
          bitSet.set(MergedFareMarket::BothDir_OW_TRIP_TYPE);
      }
      else
      {
        if (paxTypeFare.directionality() == FROM)
          bitSet.set(MergedFareMarket::OB_ALL_TRIP_TYPE);
        else if (paxTypeFare.directionality() == TO)
          bitSet.set(MergedFareMarket::IB_ALL_TRIP_TYPE);
        else if (paxTypeFare.directionality() == BOTH)
          bitSet.set(MergedFareMarket::BothDir_ALL_TRIP_TYPE);
      }
    }

    const FareType& fareType = paxTypeFare.fcaFareType();
    std::map<FareType, std::map<CarrierCode, MergedFareMarket::CxrFareTypeBitSet> >::iterator it =
        fareTypeMap.find(fareType);
    if (it != fareTypeMap.end())
    {

      if(paxTypeFare.validatingCarriers().empty())
      {
        CarrierCode blankCC;
        addFareTypeBitSet(blankCC, it->second, bitSet);
      }
      else
      {
         for(CarrierCode valCxr: paxTypeFare.validatingCarriers())
         {
           addFareTypeBitSet(valCxr, it->second, bitSet);
         }
      }
    }
    else
    {
      if(paxTypeFare.validatingCarriers().empty())
      {
        CarrierCode blankCC;
        std::map<CarrierCode, MergedFareMarket::CxrFareTypeBitSet> bitSetVCMap;
        bitSetVCMap.insert(
                    std::map<CarrierCode, MergedFareMarket::CxrFareTypeBitSet>::
                                                     value_type(blankCC, bitSet));
        fareTypeMap.insert(
          std::map<FareType, std::map<CarrierCode, MergedFareMarket::CxrFareTypeBitSet> >::
                                                          value_type(fareType, bitSetVCMap));
      }
      else
      {
         std::map<CarrierCode, MergedFareMarket::CxrFareTypeBitSet> bitSetVCMap;
         for(CarrierCode valCxr: paxTypeFare.validatingCarriers())
         {
           bitSetVCMap.insert(
                       std::map<CarrierCode, MergedFareMarket::CxrFareTypeBitSet>::
                                                                value_type(valCxr, bitSet));
         }
         fareTypeMap.insert(
           std::map<FareType, std::map<CarrierCode, MergedFareMarket::CxrFareTypeBitSet> >::
                                                         value_type(fareType, bitSetVCMap));
      }
    }
  }
}

//-----------------------------------------------------------------------------
void
FareMarketMerger::addFareTypeBitSet(
    CarrierCode valCxr,
    std::map<CarrierCode, MergedFareMarket::CxrFareTypeBitSet>& fareTypeBitSetByVC,
    const MergedFareMarket::CxrFareTypeBitSet& bitSet)
{
  auto it = fareTypeBitSetByVC.find(valCxr);
  if (it != fareTypeBitSetByVC.end())
  {
    it->second.combine(bitSet);
  }
  else
  {
    fareTypeBitSetByVC.insert(std::map<CarrierCode, MergedFareMarket::CxrFareTypeBitSet>::value_type(valCxr, bitSet));
  }
}

/*-------------------------------------------------------------------------
 *
 *------------------------------------------------------------------------*/
void
FareMarketMerger::setTag2FareIndicator(MergedFareMarket& mfm)
{
  if (mfm.geoTravelType() != GeoTravelType::Domestic && mfm.geoTravelType() != GeoTravelType::Transborder)
  {
    // We don't need to set the Indicator for Market other than US/CA
    return;
  }

  bool tag2FareFound = false;

  // US/CA don't have multiple Gov-Cxr/FareMarket priority issue,
  // Threrefore, there will be only one FareMarket

  FareMarket& fm = *mfm.mergedFareMarket().front();

  // LOG4CXX_DEBUG(logger, " FareMarket: " <<mfm.boardMultiCity() << "-" << mfm.offMultiCity())

  std::vector<PaxTypeBucket>::iterator paxCortegeIt = fm.paxTypeCortege().begin();
  std::vector<PaxTypeBucket>::iterator paxCortegeItEnd = fm.paxTypeCortege().end();
  for (; paxCortegeIt != paxCortegeItEnd; ++paxCortegeIt)
  {
    // LOG4CXX_DEBUG(logger, "PaxType: " << paxCortegeIt->requestedPaxType()->paxType()
    //                                << " # of Fare:"<<  paxCortegeIt->paxTypeFare().size())

    std::vector<PaxTypeFare*>::iterator ptfIt = paxCortegeIt->paxTypeFare().begin();
    std::vector<PaxTypeFare*>::iterator ptfItEnd = paxCortegeIt->paxTypeFare().end();
    for (; ptfIt != ptfItEnd; ++ptfIt)
    {
      PaxTypeFare& ptf = *(*ptfIt);
      if (ptf.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
      {
        // LOG4CXX_DEBUG(logger, "tag2FareFound")
        tag2FareFound = true;
        break;
      }
    }
    if (tag2FareFound)
    {
      break;
    }
  }

  if (tag2FareFound)
  {
    mfm.tag2FareIndicator() = MergedFareMarket::Tag2FareIndicator::Present;
  }
  else
  {
    mfm.tag2FareIndicator() = MergedFareMarket::Tag2FareIndicator::Absent;
  }
}

/*-------------------------------------------------------------------------
 *
 *------------------------------------------------------------------------*/
void
FareMarketMerger::sortFareMarket(MergedFareMarket& mfm, FareMarket& fm)
{
  std::vector<PaxTypeBucket>::iterator cortIt = fm.paxTypeCortege().begin();
  std::vector<PaxTypeBucket>::iterator cortItEnd = fm.paxTypeCortege().end();
  uint16_t paxTypeNum = 0;

  for (; cortIt != cortItEnd; ++cortIt, ++paxTypeNum)
  {
    PaxTypeBucket& paxTypeCortege = *cortIt;

    if (UNLIKELY(mfm.cxrFarePreferred() && (!_searchAlwaysLowToHigh)))
    {
      // TSELatencyData metrics( _trx, "SORTING TIME");
      FareMarketMerger::CxrPrefFareComparator cxrPrefFareComparator(
          paxTypeCortege, paxTypeNum, _trx.getOptions()->isZeroFareLogic());
      sort(paxTypeCortege.paxTypeFare().begin(),
           paxTypeCortege.paxTypeFare().end(),
           cxrPrefFareComparator);

      // LOG4CXX_ERROR(logger, "AFTER SORTING " << mfm.boardMultiCity()
      //                        << "-" << mfm.offMultiCity() << ":")
      // debugOnlyDisplayPTF(paxTypeCortege.paxTypeFare());

      reArrange(paxTypeCortege.paxTypeFare());
    }
    else
    {
      PaxTypeFare::FareComparator fareComparator(
          paxTypeCortege, paxTypeNum, _trx.getOptions()->isZeroFareLogic());
      sort(
          paxTypeCortege.paxTypeFare().begin(), paxTypeCortege.paxTypeFare().end(), fareComparator);
    }
  }
}

/*-------------------------------------------------------------------------
 *
 *------------------------------------------------------------------------*/
void
FareMarketMerger::reArrange(std::vector<PaxTypeFare*>& pxTypeFareVect)
{
  const std::vector<PaxTypeFare*>::iterator itBeg = pxTypeFareVect.begin();
  const std::vector<PaxTypeFare*>::iterator itEnd = pxTypeFareVect.end();
  IndustryFare yyFare;
  std::vector<PaxTypeFare*>::iterator yyIt = find_if(itBeg, itEnd, yyFare);
  if (yyIt == itEnd)
  {
    // there is no YY fare in the market
    return;
  }

  if (yyIt == itBeg)
  {
    // no cxr fare, very first one is yy
    return;
  }

  std::list<PaxTypeFare*> tmpList;
  tmpList.insert(tmpList.end(), itBeg, yyIt);
  for (; yyIt != itEnd; ++yyIt)
  {
    insertYYFare(tmpList, (*yyIt));
  }
  pxTypeFareVect.clear();
  pxTypeFareVect.insert(pxTypeFareVect.end(), tmpList.begin(), tmpList.end());

  // LOG4CXX_ERROR(logger, "AFTER RE-ARRANGE:")
  // debugOnlyDisplayPTF(pxTypeFareVect);
}

/*-------------------------------------------------------------------------
 *
 *------------------------------------------------------------------------*/
void
FareMarketMerger::insertYYFare(std::list<PaxTypeFare*>& pxTypeFareList, PaxTypeFare* paxTypeFare)
{

  std::list<PaxTypeFare*>::iterator it = pxTypeFareList.begin();
  std::list<PaxTypeFare*>::iterator pos = it;
  const std::list<PaxTypeFare*>::iterator itEnd = pxTypeFareList.end();
  for (; it != itEnd; ++it)
  {
    // find last Cxr fare of that fcaFareType
    if (paxTypeFare->fcaFareType() == (*it)->fcaFareType())
    {
      pos = it;
      ++pos;
    }
  }

  for (; pos != itEnd; ++pos)
  {
    MoneyAmount diff = (*pos)->totalFareAmount() - paxTypeFare->totalFareAmount();
    if (diff > EPSILON)
    {
      break;
    }
  }
  pxTypeFareList.insert(pos, paxTypeFare);
}

#if 0

/***********************************************************************
 For Debug only
 ***********************************************************************/

bool debugOnlyFilter(const FareMarket& fareMarket)
{
  log4cxx::LoggerPtr logger(
      log4cxx::Logger::getLogger("atseintl.Pricing.debugOnlyDisplayPaxTypeFare"));

  std::vector<std::string> fmOrig;
  std::vector<std::string> fmDest;

  // list the FareMarket to be allowed for PO
  fmOrig.push_back("BOG");
  fmDest.push_back("MAD");

  fmOrig.push_back("MAD");
  fmDest.push_back("BOG");


  size_t mktCount = fmOrig.size();
  for (int i=0; i < mktCount; ++i)
  {
    if(fmOrig[i] == fareMarket.boardMultiCity() &&
        fmDest[i] == fareMarket.offMultiCity())
    {
      LOG4CXX_ERROR(logger, "FareMarket:" << fmOrig[i] << "-" <<fmDest[i])
	        return false;
    }
  }


  return true;
}



void debugOnlyDisplayPTF(const std::vector<PaxTypeFare*>& ptfV)
{

  log4cxx::LoggerPtr logger(
      log4cxx::Logger::getLogger("atseintl.Pricing.debugOnlyDisplayPaxTypeFare"));

  ostringstream oss;

  oss << "\n";

  oss <<  " ======================================= \n";

  std::vector<PaxTypeFare*>::const_iterator it = ptfV.begin();
  std::vector<PaxTypeFare*>::const_iterator itEnd = ptfV.end();
  for(; it != itEnd; ++it)
  {
    oss << "Amount=" << (*it)->totalFareAmount()
	          << "  fcaFT: " << (*it)->fcaFareType()
	          << "\tFC: " << (*it)->fareClass()
	          << "\tCarrier: " << (*it)->carrier()
	          << "\tDIR: " << (*it)->directionality()
	          << "\tTag: " << (*it)->owrt()
	          << "\n";
  }
  const std::string& str  = oss.str();
  LOG4CXX_ERROR(logger, str)
}

#endif
}
