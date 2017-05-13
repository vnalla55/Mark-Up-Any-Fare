//----------------------------------------------------------------------------
//
//  Copyright Sabre 2011
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

#include "Common/ValidatingCarrierUpdater.h"

#include "Common/DefaultValidatingCarrierFinder.h"
#include "Common/FallbackUtil.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/ValidatingCxrUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CountrySettlementPlanInfo.h"
#include "DBAccess/DataHandle.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag191Collector.h"

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

namespace tse
{

FALLBACK_DECL(fallbackValidatingCxrGTC);
FALLBACK_DECL(fallbackValidatingCxrMultiSp);
FALLBACK_DECL(fallbackNonPreferredVC)
FALLBACK_DECL(fallbackPreferredVC)
FALLBACK_DECL(fallbackPVCWithoutMultiSp)
FALLBACK_DECL(fallbackMipRexSingleItinException)
FALLBACK_DECL(fallbackNonBSPVcxrPhase1);
FALLBACK_DECL(fallbackTraditionalValidatingCxr)

static Logger
logger("atseintl.Common.ValidatingCarrierUpdater");

ValidatingCarrierUpdater::ValidatingCarrierUpdater(PricingTrx& trx) : _trx(trx), _diag(nullptr)
{
  _isShopping = (_trx.getTrxType() == PricingTrx::IS_TRX) ||
                (_trx.getTrxType() == PricingTrx::MIP_TRX) ||
                (_trx.getTrxType() == PricingTrx::FF_TRX) ||
                (_trx.getRequest()->isMultiTicketRequest() && !_trx.multiTicketMap().empty());

  if (_trx.isValidatingCxrGsaApplicable())
    TSE_ASSERT(_trx.getRequest()->ticketingAgent());
}

bool
ValidatingCarrierUpdater::isGTCProcessing(Itin& itin,
                                          Diag191Collector* diag191,
                                          bool& ignoreItin) const
{
  ignoreItin = false;

  if (_trx.isValidatingCxrGsaApplicable() && !fallback::fallbackValidatingCxrGTC(&_trx))
  {
    std::vector<CarrierCode> marketingCxr;
    ValidatingCxrUtil::getMarketingItinCxrs(itin, marketingCxr);

    if (UNLIKELY(ValidatingCxrUtil::isGTCCarriers(_trx, marketingCxr)))
    {
      if (marketingCxr.size() > 1) // Multiple GTC case
      {
        if (ValidatingCxrUtil::isThrowException(_trx))
        {
          LOG4CXX_ERROR( logger, "MULTI-GTC CARRIERS CANNOT BE TICKETED");
          if (diag191)
          {
            *diag191 << "MULTI-GTC CARRIERS CANNOT BE TICKETED";
            diag191->flushMsg();
          }
          throw tse::ErrorResponseException(ErrorResponseException::VALIDATING_CXR_ERROR,
              "NO VALID TICKETING AGREEMENTS FOUND");
        }

        if (diag191)
          *diag191 << "ITIN OR SOP(S) INVALID DUE TO MULTIPLE GTC CARRIERS: "
                   << DiagnosticUtil::containerToString(marketingCxr) << "\n";

        ignoreItin = true;
        return false;
      }

      if (_trx.getTrxType() != PricingTrx::IS_TRX)
      {
        _trx.setValidatingCxrGsaApplicable(false);

        if (diag191)
          *diag191 << "VALIDATING CXR GSA LOGIC INACTIVATED DUE TO GTC CARRIER "
                   << DiagnosticUtil::containerToString(marketingCxr) << "\n";
      }
      else if (!marketingCxr.empty())
      {
        itin.validatingCarrier() = *marketingCxr.begin();

        if (diag191)
          *diag191 << "\n** LEGACY PROCESSING FOR ITIN OR SOP(S) DUE TO GTC CARRIER "
                   << itin.validatingCarrier() << " **\n";
      }

      return true;
    }
  }

  return false;
}

void
ValidatingCarrierUpdater::update(Itin& itin,
                                 bool isSopProcessing,
                                 const SopIdVec* sops)
{
  Diag191Collector* diag191 = nullptr;
  bool isLegacyProcessing = !_trx.isValidatingCxrGsaApplicable();
  if (!isLegacyProcessing)
  {
    // TODO: change it to avoid dynamic cast
    // in order to do that we need to change the ValidatingCxrUtil
    // change the param type, and make virtual
    diag191 = diag().isActive() ? dynamic_cast<Diag191Collector*>(_diag) : nullptr;

    bool ignoreItin(false);
    if (fallback::fallbackValidatingCxrMultiSp(&_trx) && !_trx.overrideFallbackValidationCXRMultiSP())
      isLegacyProcessing = isGTCProcessing(itin, diag191, ignoreItin);

    if (UNLIKELY(diag191))
      diag191->flushMsg();

    if (ignoreItin)
      return;
  }

  if (isLegacyProcessing)
    legacyProcess(itin);
  else
  {
    if (_trx.useTraditionalValidatingCxr())
      determineTraditionalValidatingCarrier(itin);

    updateValidatingCxrList(itin, diag191, isSopProcessing, sops);

    bool isValCxrFound = false;
    if (!fallback::fallbackValidatingCxrMultiSp(&_trx) || _trx.overrideFallbackValidationCXRMultiSP())
        isValCxrFound = itin.isValidatingCxrGsaDataForMultiSp();
    else
        isValCxrFound = itin.validatingCxrGsaData() != nullptr;

    if (isValCxrFound && !isSopProcessing)
    {
      updateDefaultValidatingCarrier(itin);

      if (!fallback::fallbackTraditionalValidatingCxr(&_trx))
      {
        if (UNLIKELY(diag191))
        {
          diag191->print191DefaultValidatingCarrier(_trx, itin, _isShopping);
        }
      }
      else
      {
        if (UNLIKELY(diag191))
        {
          *diag191 << " \n";
          *diag191 << "\n  ITIN DEFAULT VALIDATING CARRIER: " << itin.validatingCarrier() << "\n";
          if (!_isShopping)
           diag191->print191Footer();
        }
      }
      if (UNLIKELY(diag191))
        diag191->flushMsg();
    }
    if (UNLIKELY(!_isShopping))
    {
      if (!fallback::fallbackValidatingCxrMultiSp(&_trx) || _trx.overrideFallbackValidationCXRMultiSP())
      {
        if (_trx.getOptions()->fbcSelected())
        {
          if (itin.validatingCarrier().empty())
          {
            std::string message("MULTIPLE VALIDATING CARRIER OPTIONS - SPECIFY #A OR #VM");
            if (diag191) {
              *diag191 << message;
              diag191->flushMsg();
            }
            throw ErrorResponseException(ErrorResponseException::VALIDATING_CXR_ERROR,
                message.c_str());
          }
          else
            itin.removeAlternateValidatingCarriers();

          if (diag191)
          {
            *diag191 << " \n";
            *diag191 << "\n  COMMAND PRICING REQUESTED ALTERNATE CARRIER PROCESS STOPPED \n";
            diag191->flushMsg();
          }
        }
      }
      else
      {
        std::vector<CarrierCode> validatingCxrs;
        itin.getValidatingCarriers(_trx, validatingCxrs);

        if (_trx.getOptions()->fbcSelected() && validatingCxrs.size() > 1 )
        {
          if (itin.validatingCarrier().empty())
          {
            validatigCarrierOverrideErrorMsg(validatingCxrs, diag191);
          }
          else
          {
            itin.removeNonDefaultValidatingCarriers(itin.validatingCarrier());
            if (diag191)
            {
              *diag191 << " \n";
              *diag191 << "\n  COMMAND PRICING REQUESTED ALTERNATE CARRIER PROCESS STOPPED \n";
              diag191->flushMsg();
            }
          }
        }
      }
    }
  }
}

void
ValidatingCarrierUpdater::legacyProcess(Itin& itin) const
{
  if (updateByOverride(itin))
    return;

  process(itin);
}

CarrierCode ValidatingCarrierUpdater::determineValidatingCarrierLegacy(const Itin &itin) const
{
  const CarrierCode *validatingCarrier(determineValidatingCarrier(itin));
  if (validatingCarrier)
    return *validatingCarrier;

  AirSegmentVec segments;
  filterSegments(itin.travelSeg(), segments);
  CarrierCode carrier = find(segments);
  ItinUtil::swapValidatingCarrier(_trx, carrier);

  return carrier;
}

void
ValidatingCarrierUpdater::update(FarePath& farePath) const
{
  process(*farePath.itin());
}

void
ValidatingCarrierUpdater::process(Itin& itin) const
{
  AirSegmentVec segments;
  filterSegments(itin.travelSeg(), segments);
  itin.validatingCarrier() = find(segments);
  ItinUtil::swapValidatingCarrier(_trx, itin);
}

bool
ValidatingCarrierUpdater::updateByOverride(Itin& itin) const
{
  const CarrierCode *validatingCarrier = determineValidatingCarrier(itin);
  if (validatingCarrier)
  {
    itin.validatingCarrier() = *validatingCarrier;
    return true;
  }

  return false;
}

const CarrierCode*
ValidatingCarrierUpdater::determineValidatingCarrier(const Itin &itin) const
{
  if (!_trx.getRequest()->validatingCarrier().empty())
  {
    if (!itin.validatingCarrier().empty())
      return &itin.validatingCarrier();

    return &_trx.getRequest()->validatingCarrier();
  }

  if (UNLIKELY(_trx.getRequest()->validatingCarrier().empty() && !_trx.billing()->partitionID().empty() &&
      MCPCarrierUtil::isMcpHost(_trx.billing()->partitionID())))
  {
    _trx.getRequest()->validatingCarrier() = _trx.billing()->partitionID();
    return &_trx.getRequest()->validatingCarrier();
  }

  return nullptr;
}

void
ValidatingCarrierUpdater::addNSPInCspi()
{
  const std::vector<CountrySettlementPlanInfo*>& cspInfos = _trx.countrySettlementPlanInfos();
  for (const CountrySettlementPlanInfo* cspInfo : cspInfos)
  {
    if(cspInfo->getSettlementPlanTypeCode() == NO_SETTLEMENTPLAN)
      return;
  }

  const Loc* posLoc = ValidatingCxrUtil::getPOSLoc(_trx, 0);
  CountrySettlementPlanInfo* nspSP = _trx.dataHandle().create<CountrySettlementPlanInfo>();
  nspSP->setCountryCode(ValidatingCxrUtil::getNation(_trx, posLoc));
  nspSP->setSettlementPlanTypeCode(NO_SETTLEMENTPLAN);
  const_cast<std::vector<CountrySettlementPlanInfo*>&>(cspInfos).push_back(nspSP);
}

bool
ValidatingCarrierUpdater::isRexWithSingleItin()
{
  return (!fallback::fallbackMipRexSingleItinException(&_trx) ?
            (_trx.getTrxType() == PricingTrx::MIP_TRX &&
            ValidatingCxrUtil::isRexPricing(_trx) && _trx.itin().size() == 1) :
            false);
}

void
ValidatingCarrierUpdater::printNSPDiag191Info(Diag191Collector* diag191)
{
  switch(_trx.getRequest()->spvInd())
  {
    case tse::spValidator::noSMV_noIEV:
      *diag191<<"SETTLEMENT PLAN VALIDATION:    F, IET VALIDATION:    F"<< std::endl;
      break;
    case tse::spValidator::noSMV_IEV:
      *diag191<<"SETTLEMENT PLAN VALIDATION:    F, IET VALIDATION:    T"<< std::endl;
      break;
    case tse::spValidator::SMV_IEV:
      *diag191<<"SETTLEMENT PLAN VALIDATION:    T, IET VALIDATION:    T"<< std::endl;
      break;
  }
  if(!_trx.getRequest()->spvCxrsCode().empty())
    *diag191<<"NSP CARRIERS:    "<<DiagnosticUtil::containerToString(_trx.getRequest()->spvCxrsCode()) << std::endl;
  if(!_trx.getRequest()->spvCntyCode().empty())
    *diag191<<"NSP COUNTRIES:    "<<DiagnosticUtil::containerToString(_trx.getRequest()->spvCntyCode()) << std::endl;
}
// This function calls the util functions to update validation carrier list in an itin
// step 1: call determineCountrySettlementPlan
// step 2: if no exception thrown in determineCountrySettlementPlan it call getItinValidatingCxrList
// if this is false
// check is this is shipping trx (MIP/IS), if not shopping throw exception with no valid carrier
// found
// else the itin is updated with list of validating carrier
// getItinValidatingCxrList also takes care of neutral validating carrier update
void
ValidatingCarrierUpdater::updateValidatingCxrList(Itin& itin,
                                                  Diag191Collector* diag191,
                                                  bool isSopProcessing,
                                                  const SopIdVec* sops)
{
  const bool lockingNeeded = _trx.isLockingNeededInShoppingPQ();

  std::vector<CarrierCode> marketingCarriers, participatingCarriers, realMarketingCxrs;
  std::string hashString;
  if(!fallback::fallbackNonPreferredVC(&_trx) && diag191)
  {
    if(!_trx.getRequest()->validatingCarrier().empty())
      *diag191 <<"SPECIFIED VALIDATING CARRIER:    "<<_trx.getRequest()->validatingCarrier() << std::endl;
    if(!_trx.getRequest()->nonPreferredVCs().empty())
      *diag191<<"NON-PREFERRED VALIDATING CARRIERS:    "<<DiagnosticUtil::containerToString(_trx.getRequest()->nonPreferredVCs()) << std::endl;
  }
  if(!fallback::fallbackPreferredVC(&_trx) && diag191)
  {
    if(!_trx.getRequest()->preferredVCs().empty())
      *diag191<<"PREFERRED VALIDATING CARRIERS:    "<<DiagnosticUtil::containerToString(_trx.getRequest()->preferredVCs()) << std::endl;
  }
  if((!fallback::fallbackNonBSPVcxrPhase1(&_trx) || _trx.overrideFallbackValidationCXRMultiSP())
    && diag191)
    printNSPDiag191Info(diag191);
  SettlementPlanType spType = _trx.getRequest()->getSettlementMethod();

  ValidatingCxrUtil::determineCountrySettlementPlan(_trx, diag191, &spType);

  ValidatingCxrUtil::getAllItinCarriers(_trx, itin, realMarketingCxrs, participatingCarriers);

  if (!_trx.getRequest()->validatingCarrier().empty())
    marketingCarriers.push_back(_trx.getRequest()->validatingCarrier());
  else
    marketingCarriers = realMarketingCxrs;

  if (LIKELY(_isShopping))
    hashString = ValidatingCxrUtil::createHashString(realMarketingCxrs, participatingCarriers);

  if (UNLIKELY(diag191))
    diag191->print191Header(&_trx, &itin, hashString, isSopProcessing, sops);

  if ((!fallback::fallbackValidatingCxrMultiSp(&_trx) || _trx.overrideFallbackValidationCXRMultiSP())
        && _isShopping)
  {
    if (SpValidatingCxrGSADataMap* spValCxrGsaDataMap =
        _trx.getSpValCxrGsaDataMap(hashString, lockingNeeded))
    {
      itin.spValidatingCxrGsaDataMap() = spValCxrGsaDataMap;
      // We may remove this once Shopping is ready for Multiple Settlement Plan logic
      resetForSingleSettlementPlan(itin, *spValCxrGsaDataMap);

      if (diag191 && _isShopping)
      {
        *diag191 << "  REUSING EXISTING RESULTS FOR " << hashString;
        if (!itin.isValidatingCxrGsaDataForMultiSp())
          *diag191 << " (NO VALIDATING CARRIERS)";
        *diag191 << "\n";
      }

      if (diag191)
        diag191->flushMsg();
      return;
    }
  }
  else
  {
    if (ValidatingCxrGSAData* valCxrGsaData =
        _trx.getValCxrGsaData(hashString, lockingNeeded))
    {
      itin.validatingCxrGsaData() = valCxrGsaData;
      if (UNLIKELY(diag191 && _isShopping))
      {
        *diag191 << "  REUSING EXISTING RESULTS FOR " << hashString;
        if (!valCxrGsaData)
          *diag191 << " (NO VALIDATING CARRIERS)";
        *diag191 << "\n";
      }

      if (UNLIKELY(diag191))
        diag191->flushMsg();
      return;
    }
  }

  if (UNLIKELY(diag191 && !_isShopping))
    diag191->printDiagMsg(_trx, itin, _trx.getRequest()->validatingCarrier());

  std::vector<CarrierCode> marketingItinCxrs;
  ValidatingCxrUtil::getMarketingItinCxrs( itin, marketingItinCxrs );

  std::string errMsg;
  bool isValCxrFound = false;
  if (!fallback::fallbackValidatingCxrMultiSp(&_trx) || _trx.overrideFallbackValidationCXRMultiSP())
  {
    if((!fallback::fallbackNonBSPVcxrPhase1(&_trx) || _trx.overrideFallbackValidationCXRMultiSP()) &&
            (_trx.getRequest()->spvInd() == tse::spValidator::noSMV_noIEV ||
             _trx.getRequest()->spvInd() == tse::spValidator::noSMV_IEV))
    {
      addNSPInCspi();

    }
    SpValidatingCxrGSADataMap* spGsaDataMap = _trx.dataHandle().create<SpValidatingCxrGSADataMap>();
    const std::vector<CountrySettlementPlanInfo*>& countrySettlementPlanInfos =
        _trx.isLockingNeededInShoppingPQ() ?
          _trx.getCopyOfCountrySettlementPlanInfos() :
          _trx.countrySettlementPlanInfos();


    for (const CountrySettlementPlanInfo* cspInfo : countrySettlementPlanInfos)
    {
      if (diag191)
        diag191->displaySettlementPlanInfo(*cspInfo);

      ValidatingCxrGSAData* valCxrGsaData =
        ValidatingCxrUtil::getValidatingCxrList(_trx, diag191, *cspInfo,
            marketingCarriers, participatingCarriers, errMsg, marketingItinCxrs);
      spGsaDataMap->insert(std::pair<SettlementPlanType,
            const ValidatingCxrGSAData*>(cspInfo->getSettlementPlanTypeCode(), valCxrGsaData));

      if (!isValCxrFound && valCxrGsaData)
        isValCxrFound = true;
    }

    itin.spValidatingCxrGsaDataMap() = spGsaDataMap;
    resetForSingleSettlementPlan(itin, *spGsaDataMap); // We can remove it once we are multi-sp

    if (_isShopping)
    {
      std::unique_lock<std::mutex> lock(_trx.validatingCxrMutex(), std::defer_lock);
      if (lockingNeeded)
        lock.lock();

      _trx.hashSpValidatingCxrGsaDataMap()[hashString] = itin.spValidatingCxrGsaDataMap();
    }
  }
  else
  {
    itin.validatingCxrGsaData() = ValidatingCxrUtil::getValidatingCxrList(
        _trx, diag191, *_trx.countrySettlementPlanInfo(), marketingCarriers,
        participatingCarriers, errMsg, marketingItinCxrs);

    {
      std::unique_lock<std::mutex> lock(_trx.validatingCxrMutex(), std::defer_lock);
      if (lockingNeeded)
        lock.lock();

      _trx.validatingCxrHashMap()[hashString] = itin.validatingCxrGsaData();
    }

    if (itin.validatingCxrGsaData())
      isValCxrFound = true;
  }

  if (!isValCxrFound)
  {
    bool isThrowException =
      (!_isShopping || (_trx.getTrxType() == PricingTrx::MIP_TRX &&
                        (_trx.billing()->actionCode() == "WPNI.C" || _trx.billing()->actionCode() == "WFR.C"))
                    || isRexWithSingleItin());

    if (errMsg.empty())
    {
      errMsg = "NO VALID TICKETING AGREEMENTS FOUND";
    }

    if (isThrowException)
    {
      if (diag191)
      {
        if (fallback::fallbackValidatingCxrMultiSp(&_trx) && !_trx.overrideFallbackValidationCXRMultiSP())
        {
          *diag191 << errMsg << "\n";
        }
        else
        {
          *diag191 << " \n" << std::endl;
          *diag191 << "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * "<< std::endl;
        }
        diag191->flushMsg();
      }
      throw ErrorResponseException(ErrorResponseException::VALIDATING_CXR_ERROR,
          errMsg.c_str());
    }
    else if (diag191)
    {
      if (!fallback::fallbackValidatingCxrMultiSp(&_trx) || _trx.overrideFallbackValidationCXRMultiSP())
      {
        *diag191 << " \n" << std::endl;
        *diag191 << "* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * "<< std::endl;
      }
      *diag191 << errMsg << "\n";
    }
    if (_trx.getRequest()->isMultiTicketRequest() &&
        !_trx.multiTicketMap().empty() &&
        itin.getMultiTktItinOrderNum() == 0)
    {
      itin.errResponseMsg() = errMsg;
      itin.errResponseCode() = ErrorResponseException::VALIDATING_CXR_ERROR;
    }
  }
  else if (UNLIKELY(diag191))
    diag191->printValidatingCxrList(_trx, itin, isSopProcessing);

  if (UNLIKELY(diag191))
    diag191->flushMsg();
}

// This function is to call to update the default validating carrier with with one itin
// step 1: check if there is a neutral validation carrier, is that is the case
//        check if it has one item, and set that as default validating carrier
// if step 1 doesn't return go step 2
// step 2: get list of validation carrier from itin.validatingCxrGsaData()
// step 3: get list of marketing carrier from validation carrier
// step 4: find the segments that match with found marketing carrier
// step 5: find the validation carrier using IATA rule
// step 6: get list(set) of validation carrier for the carrier just found using IATA rule
// step 7: If the set return one item, set that as a default validation carrier
void
ValidatingCarrierUpdater::updateDefaultValidatingCarrier(Itin& itin) const
{
  bool retVal = false;
  CarrierCode defaultValidatingCxr, defaultMarketingCxr;
  std::vector<CarrierCode> validatingCxrs;
  if (!fallback::fallbackValidatingCxrMultiSp(&_trx) || _trx.overrideFallbackValidationCXRMultiSP())
  {
    // Finding default only for primary SP at this point
    const SettlementPlanType primarySp =
      ValidatingCxrUtil::getSettlementPlanFromHierarchy(*itin.spValidatingCxrGsaDataMap());

    auto it = itin.spValidatingCxrGsaDataMap()->find(primarySp);
    if (it != itin.spValidatingCxrGsaDataMap()->end() && it->second)
    {
      itin.getValidatingCarriers(*it->second, validatingCxrs);
      DefaultValidatingCarrierFinder defValCxrFinder(_trx, itin, primarySp);
      retVal = defValCxrFinder.determineDefaultValidatingCarrier(
          validatingCxrs,
          defaultValidatingCxr,
          defaultMarketingCxr);
    }

    // In case of command pricing, check for default validating carrier acorss
    // settlement plans and stops once found
    if (!retVal && _trx.getOptions()->fbcSelected())
      retVal = setDefaultValidatingCxrForCommandPricing(
          itin,
          primarySp,
          defaultValidatingCxr,
          defaultMarketingCxr);
  }
  else
  {
    itin.getValidatingCarriers(_trx, validatingCxrs);
    retVal = determineDefaultValidatingCarrier(itin,
        validatingCxrs, defaultValidatingCxr, defaultMarketingCxr);
  }

  if (retVal)
    itin.validatingCarrier() = defaultValidatingCxr;
}

// This function goes through the air segments in an itin
// Check the segment is in the list of marketing carrier
// return only those segments that matches previous condition
void
ValidatingCarrierUpdater::getSegsFromCarrier(Itin& itin,
                                             std::vector<CarrierCode>& marketingCxrs,
                                             AirSegmentVec& segments) const
{
  for (TravelSeg* tSeg : itin.travelSeg())
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(tSeg);
    if (airSeg)
    {
      if (std::find(marketingCxrs.begin(), marketingCxrs.end(), airSeg->carrier()) !=
          marketingCxrs.end())
      {
        segments.push_back(airSeg);
      }
    }
  }
}

void ValidatingCarrierUpdater::excludeGTCCarriers(std::vector<CarrierCode>& marketingCxrs,
                                                  std::set<CarrierCode>& gtcCarriers)
{
  if (fallback::fallbackValidatingCxrGTC(&_trx))
    return;

  std::vector<CarrierCode> newMarketingCxrs;

  for (CarrierCode& carrier : marketingCxrs)
  {
    std::vector<CarrierCode> tempSet;
    tempSet.push_back(carrier);

    if (ValidatingCxrUtil::isGTCCarriers(_trx, tempSet))
      gtcCarriers.insert(carrier);
    else
      newMarketingCxrs.push_back(carrier);
  }

  marketingCxrs = newMarketingCxrs;
}

void ValidatingCarrierUpdater::updateFlightFinderValidatingCarrier()
{
  FlightFinderTrx* ffTrx = dynamic_cast<FlightFinderTrx*>(&_trx);
  if (!ffTrx)
    return;

  Diag191Collector* diag191 = diag().isActive() ? dynamic_cast<Diag191Collector*>(_diag) : nullptr;
  if (diag191)
    diag191->print191Header(&_trx);

  SettlementPlanType spType = _trx.getRequest()->getSettlementMethod();
  ValidatingCxrUtil::determineCountrySettlementPlan(_trx, diag191, &spType);

  std::vector<CarrierCode> marketingCxrs;
  ValidatingCxrUtil::getAllSopMarketingCarriers(*ffTrx, marketingCxrs);

  // Need to exclude GTC carriers since they don't participate in SM
  std::set<CarrierCode> gtcCarriers;
  if (fallback::fallbackValidatingCxrMultiSp(&_trx) && !_trx.overrideFallbackValidationCXRMultiSP())
    excludeGTCCarriers(marketingCxrs, gtcCarriers);

  bool isValCxrFound = false;
  if (!fallback::fallbackValidatingCxrMultiSp(&_trx) || _trx.overrideFallbackValidationCXRMultiSP())
  {
    SpValidatingCxrGSADataMap* spGsaDataMap = _trx.dataHandle().create<SpValidatingCxrGSADataMap>();
    for (const CountrySettlementPlanInfo* cspInfo : _trx.countrySettlementPlanInfos())
    {
      if (diag191)
        diag191->displaySettlementPlanInfo(*cspInfo);

      ValidatingCxrGSAData* valCxrGsaData = ValidatingCxrUtil::validateSettlementMethodAndGsa(
        _trx, diag191, *cspInfo, marketingCxrs);

      (*spGsaDataMap)[cspInfo->getSettlementPlanTypeCode()] = valCxrGsaData;

      if (valCxrGsaData)
        isValCxrFound = true;
    }

    if (isValCxrFound)
      setFlightFinderDefaultValidatingCarrier(*ffTrx, diag191, spGsaDataMap);
  }
  else // pre-multiSP code
  {
    ValidatingCxrGSAData* valCxrGsaData = ValidatingCxrUtil::validateSettlementMethodAndGsa(
      _trx, diag191, *_trx.countrySettlementPlanInfo(), marketingCxrs);

    if (valCxrGsaData)
      setFlightFinderDefaultValidatingCarrierOld(*ffTrx, diag191, valCxrGsaData, gtcCarriers);
    else
      isValCxrFound = false;
  }

  if (!isValCxrFound && gtcCarriers.empty())
  {
    if (diag191)
    {
      *diag191 << "NO VALIDATING CARRIERS FOUND\n";
      diag191->print191Footer();
      diag191->flushMsg();
    }
  }
}

void
ValidatingCarrierUpdater::setFlightFinderDefaultValidatingCarrierOld(FlightFinderTrx& trx,
                                                                     Diag191Collector* diag191,
                                                                     ValidatingCxrGSAData* valCxrGsaData,
                                                                     const std::set<CarrierCode>& gtcCarriers)
{
  std::vector<ShoppingTrx::Leg>& legs = trx.legs();
  if (!legs.empty())
  {
    std::vector<ShoppingTrx::Leg>::iterator legIter = trx.legs().begin();
    std::vector<ShoppingTrx::Leg>::iterator legIterEnd = trx.legs().end();
    int legid = 0;
    for (; legIter != legIterEnd; ++legIter)
    {
      std::vector<ShoppingTrx::SchedulingOption>::iterator sopIter = legIter->sop().begin();
      std::vector<ShoppingTrx::SchedulingOption>::iterator sopIterEnd = legIter->sop().end();

      for (; sopIter != sopIterEnd; ++sopIter)
      {
        bool isUpdated = false;
        const CarrierCode& govCxrCode = sopIter->itin()->fareMarket().front()->governingCarrier();
        if (gtcCarriers.count(govCxrCode))
        {
          sopIter->itin()->validatingCarrier() = govCxrCode;
          if (diag191)
            *diag191 << "GTC CARRIER " << govCxrCode << " IS ADDED AS LEGACY VALIDATING CARRIER\n";
        }
        else if (valCxrGsaData->hasCarrier(govCxrCode))
        {
          if(!setFlightFinderItinValidatingCarrierOld(*sopIter->itin(), diag191, valCxrGsaData, (*sopIter).originalSopId(), govCxrCode))
          {
             ValidatingCxrDataMap::const_iterator i, iEnd = valCxrGsaData->validatingCarriersData().end();
             for (i = valCxrGsaData->validatingCarriersData().begin(); i != iEnd; ++i)
             {
              const CarrierCode& carrier = i->first;
              if(setFlightFinderItinValidatingCarrierOld(*sopIter->itin(), diag191, valCxrGsaData, (*sopIter).originalSopId(), carrier))
              {
                 isUpdated = true;
                 break;
              }
             }
          }
        }
        else
        {
          std::set<CarrierCode> swapCarriers;
          ValidatingCxrUtil::getGenSalesAgents(_trx.dataHandle(),
              diag191,
              govCxrCode,
              _trx.countrySettlementPlanInfo()->getCountryCode(),
              _trx.getRequest()->ticketingAgent()->cxrCode(),
              _trx.countrySettlementPlanInfo()->getSettlementPlanTypeCode(),
              swapCarriers);
          for (const CarrierCode& swapCarrier : swapCarriers)
          {
            if(setFlightFinderItinValidatingCarrierOld(*sopIter->itin(), diag191, valCxrGsaData, (*sopIter).originalSopId(), swapCarrier))
            {
              isUpdated = true;
              break;
            }
          }
          if(!isUpdated)
          {
            if(trx.isSimpleTrip() && legid == 1)
            {
              if (diag191)
              {
                *diag191 << "NO VALIDATING CARRIERS FOUND\n";
              }

              (*sopIter).cabinClassValid() = false;
            }
          }
        }
      }
      ++legid;
    }
  }
  if (diag191)
  {
    diag191->print191Footer();
    diag191->flushMsg();
  }
}

bool
ValidatingCarrierUpdater::setFlightFinderItinValidatingCarrierOld(
  Itin& itin,
  Diag191Collector* diag191,
  ValidatingCxrGSAData* valCxrGsaData,
  const uint32_t sopId,
  const CarrierCode& carrierCode)
{
  std::string errMsg;
  std::string hashString;
  std::vector<CarrierCode> marketingCarriers,participatingCarriers;
  marketingCarriers.push_back(carrierCode);
  ValidatingCxrUtil::getParticipatingCxrs(_trx, itin, participatingCarriers);
  hashString = ValidatingCxrUtil::createHashString(marketingCarriers, participatingCarriers);
  ValidatingCxrGSADataHashMap::const_iterator i = _trx.validatingCxrHashMap().find(hashString);
  if (i != _trx.validatingCxrHashMap().end())
  {
    itin.validatingCarrier() = carrierCode;

    if (diag191)
      *diag191 << "\nREUSING " << hashString << " RESULTS FOR SOP ID " << sopId
                << "\nVALIDATING CARRIER: " << carrierCode << "\n";

    return true;
  }

  vcx::ValidatingCxrData vcxrData = valCxrGsaData->validatingCarriersData()[carrierCode];
  marketingCarriers.clear();
  ValidatingCxrUtil::getMarketingItinCxrs( itin, marketingCarriers );

  if(ValidatingCxrUtil::isPassInterlineAgreement(_trx,
                                  nullptr,
                                  _trx.countrySettlementPlanInfo()->getCountryCode(),
                                  vcxrData,
                                  carrierCode,
                                  participatingCarriers,
                                  marketingCarriers,
                                  errMsg))
  {
    itin.validatingCarrier() = carrierCode;
    _trx.validatingCxrHashMap()[hashString] = valCxrGsaData;

    if (diag191)
      *diag191 << "\nNEW HASH STRING: " << hashString << " FOR SOP ID " << sopId
               << "\nVALIDATING CARRIER: " << carrierCode << "\n";

    return true;
  }
  return false;
}

void
ValidatingCarrierUpdater::setFlightFinderDefaultValidatingCarrier(
  FlightFinderTrx& trx,
  Diag191Collector* diag191,
  SpValidatingCxrGSADataMap* spGsaDataMap)
{
  std::vector<SettlementPlanType> smPlans;
  SettlementPlanType mainSP = ValidatingCxrUtil::getSettlementPlanFromHierarchy(*spGsaDataMap);
  smPlans.push_back(mainSP);

  if (spGsaDataMap->size() > 1)
  {
    for (auto& spData : *spGsaDataMap)
      if (spData.first != mainSP)
        smPlans.push_back(spData.first);
  }

  setFlightFinderDefaultValidatingCarrierBase(trx, diag191, smPlans, spGsaDataMap);
}

void
ValidatingCarrierUpdater::setFlightFinderDefaultValidatingCarrierBase(
  FlightFinderTrx& trx,
  Diag191Collector* diag191,
  const std::vector<SettlementPlanType>& smPlans,
  SpValidatingCxrGSADataMap* spGsaDataMap)
{
  if (trx.legs().empty() || !spGsaDataMap)
    return;

  for (SettlementPlanType stmlPlan : smPlans)
  {
    const ValidatingCxrGSAData* valCxrGsaData = (*spGsaDataMap)[stmlPlan];

    if (!valCxrGsaData)
      continue;

    uint16_t failedCount = 0, legId = 0;

    bool lastSP = (*smPlans.rbegin() == stmlPlan);

    for (ShoppingTrx::Leg& leg : trx.legs())
    {
      for (ShoppingTrx::SchedulingOption& sop : leg.sop())
      {
        Itin& itin = *sop.itin();
        if (!itin.validatingCarrier().empty())
          continue;

        const CarrierCode& govCxrCode = sop.itin()->fareMarket().front()->governingCarrier();

        if (valCxrGsaData->hasCarrier(govCxrCode))
        {
          if (!setFlightFinderItinValidatingCarrier(itin, diag191, stmlPlan, spGsaDataMap,
                                                    sop.originalSopId(), govCxrCode))
          {
            for (auto& kv : valCxrGsaData->validatingCarriersData())
            {
              const CarrierCode& carrier = kv.first;
              if (setFlightFinderItinValidatingCarrier(itin, diag191, stmlPlan, spGsaDataMap,
                                                       sop.originalSopId(), carrier))
                break;
            }
          }
        }
        else
        {
          std::set<CarrierCode> swapCarriers;
          ValidatingCxrUtil::getGenSalesAgents(_trx.dataHandle(),
            diag191,
            govCxrCode,
            _trx.countrySettlementPlanInfos().front()->getCountryCode(),
            _trx.getRequest()->ticketingAgent()->cxrCode(),
            stmlPlan,
            swapCarriers);

          for (const CarrierCode& swapCarrier : swapCarriers)
          {
            if (setFlightFinderItinValidatingCarrier(itin, diag191, stmlPlan, spGsaDataMap,
                                                     sop.originalSopId(), swapCarrier))
              break;
          }

          if (itin.validatingCarrier().empty())
          {
            ++failedCount;
            if (lastSP && trx.isSimpleTrip() && (legId == 1))
            {
              if (diag191)
                *diag191 << "NO VALIDATING CARRIERS FOUND FOR SOPID " << sop.originalSopId() << "\n";

              sop.cabinClassValid() = false;
            }
          }
        }
      } // Per sop

      ++legId;
    } // Per leg

    if (failedCount == 0)
      break;

  } // Per SM

  if (diag191)
  {
    diag191->print191Footer();
    diag191->flushMsg();
  }
}

bool
ValidatingCarrierUpdater::setFlightFinderItinValidatingCarrier(
  Itin& itin,
  Diag191Collector* diag191,
  const SettlementPlanType stmlPlan,
  SpValidatingCxrGSADataMap* spGsaDataMap,
  const uint32_t sopId,
  const CarrierCode& carrierCode)
{
  std::vector<CarrierCode> marketingCarriers,participatingCarriers;
  marketingCarriers.push_back(carrierCode);
  ValidatingCxrUtil::getParticipatingCxrs(_trx, itin, participatingCarriers);

  std::string hashString =
    ValidatingCxrUtil::createHashString(marketingCarriers, participatingCarriers);

  HashSpValidatingCxrGSADataMap::const_iterator i =
    _trx.hashSpValidatingCxrGsaDataMap().find(hashString);

  if (i != _trx.hashSpValidatingCxrGsaDataMap().end())
  {
    if (diag191)
      *diag191 << "\nREUSING " << hashString << " RESULTS FOR SOP ID " << sopId
               << "\nVALIDATING CARRIER: " << carrierCode << "\n";
  }
  else
  {
    ValidatingCxrGSAData* valCxrGsaData =
      const_cast<ValidatingCxrGSAData*>((*spGsaDataMap)[stmlPlan]);
    vcx::ValidatingCxrData vcxrData = valCxrGsaData->validatingCarriersData()[carrierCode];

    marketingCarriers.clear();
    ValidatingCxrUtil::getMarketingItinCxrs(itin, marketingCarriers);

    std::string errMsg;
    if (!ValidatingCxrUtil::isPassInterlineAgreement(
          _trx,
          0,
          _trx.countrySettlementPlanInfos().front()->getCountryCode(),
          vcxrData,
          carrierCode,
          participatingCarriers,
          marketingCarriers,
          errMsg))
    {
      return false;
    }

    if (diag191)
      *diag191 << "\nNEW HASH STRING: " << hashString << " FOR SOP ID " << sopId
               << "\nVALIDATING CARRIER: " << carrierCode << "\n";

    _trx.hashSpValidatingCxrGsaDataMap()[hashString] = spGsaDataMap;
  }

  itin.validatingCarrier() = carrierCode;
  itin.spValidatingCxrGsaDataMap() = spGsaDataMap;
  resetForSingleSettlementPlan(itin, *spGsaDataMap); // We can remove it once we are multi-sp

  return true;
}

bool
ValidatingCarrierUpdater::findDefValCxrFromPreferred(const std::vector<CarrierCode>& valCxrs,
                                          const std::vector<CarrierCode>& prefValCxrs,
                                          CarrierCode& defValCxr) const
{
  bool retVal = false;
  for(const auto& tempValCxr : prefValCxrs)
  {
    if(std::find(valCxrs.begin(), valCxrs.end(), tempValCxr) != valCxrs.end())
    {
      defValCxr = tempValCxr;
      retVal = true;
      break;
    }
  }
  return retVal;
}

bool
ValidatingCarrierUpdater::determineDefaultValidatingCarrier(Itin& itin,
                                                                  std::vector<CarrierCode>& validatingCxrs,
                                                                  CarrierCode& defaultValidatingCxr,
                                                                  CarrierCode& defaultMarketingCxr) const
{

  if(!fallback::fallbackPVCWithoutMultiSp(&_trx) && !fallback::fallbackPreferredVC(&_trx)
           && !_trx.getRequest()->preferredVCs().empty()
           && findDefValCxrFromPreferred(validatingCxrs, _trx.getRequest()->preferredVCs(), defaultValidatingCxr))
  {
        std::vector<CarrierCode> mrktCxrs;
        std::vector<CarrierCode> valCxrs;

        valCxrs.push_back(defaultValidatingCxr);
        ValidatingCxrUtil::getMarketingCxrsFromValidatingCxrs(_trx,
              itin,
              valCxrs, mrktCxrs);
        if (!mrktCxrs.empty())
        {
          defaultMarketingCxr = mrktCxrs.front();
        }
        return true;
  }

  if(!itin.validatingCxrGsaData())
    return false;

  if ( itin.hasNeutralValidatingCarrier() )
  {
    if(1 == validatingCxrs.size())
    {
      defaultValidatingCxr = validatingCxrs[0];
      return true;
    }
    else
    {
      return false;
    }
  }
  if ( 1 == validatingCxrs.size() && itin.gsaSwapMap().empty() )
  {
    defaultValidatingCxr = validatingCxrs[0];
    return true;
  }

  std::vector<CarrierCode> marketingCxrs;
  ValidatingCxrUtil::getMarketingCxrsFromValidatingCxrs(_trx,
      itin,
      validatingCxrs, marketingCxrs);
  if (UNLIKELY( marketingCxrs.empty() ))
    return false;

  CarrierCode marketingCxr;

  if (1 == marketingCxrs.size())
  {
    marketingCxr = marketingCxrs.front();
    return setDefaultCarriers(itin, validatingCxrs, marketingCxr, defaultValidatingCxr, defaultMarketingCxr);
  }
  else
  {
    AirSegmentVec segments;
    getSegsFromCarrier(itin, marketingCxrs, segments);
    do
    {
      marketingCxr = find(segments);
      if(setDefaultCarriers(itin, validatingCxrs, marketingCxr, defaultValidatingCxr, defaultMarketingCxr))
        return true;
      else
      {
        AirSegmentVec::iterator segIt = segments.begin();
        while(segIt != segments.end())
        {
          if((**segIt).carrier() == marketingCxr)
            segIt = segments.erase(segIt);
          else
            ++segIt;
        }
        marketingCxrs.erase(std::remove(marketingCxrs.begin(),
                                        marketingCxrs.end(),
                                        marketingCxr), marketingCxrs.end()) ;
      }
    } while(!(marketingCxrs.empty() || segments.empty()));
  }
  return false;
}

//@todo remove with fallbackValidatingCxrMultiSp
bool ValidatingCarrierUpdater::setDefaultCarriers(Itin& itin,
                                                  std::vector<CarrierCode>& validatingCxrs,
                                                  CarrierCode& marketingCxr,
                                                  CarrierCode& defaultValidatingCxr,
                                                  CarrierCode& defaultMarketingCxr) const
{
  std::set<CarrierCode> validatingCxrSet;
  ValidatingCxrUtil::getValidatingCxrsFromMarketingCxr(itin, marketingCxr, validatingCxrSet);

  for (const CarrierCode& validatingCxr : validatingCxrSet)
  {
    if (UNLIKELY(std::find(validatingCxrs.begin(), validatingCxrs.end(), validatingCxr) ==
        validatingCxrs.end()))
    {
      validatingCxrSet.erase(validatingCxr);
    }
  }
  if (LIKELY(validatingCxrSet.size() == 1))
  {
    defaultValidatingCxr = *validatingCxrSet.begin();
    defaultMarketingCxr = marketingCxr;
    return true;
  }
  return false;
}


//
// Per Darrin Wei and Sterling the US/CA will be first Validating default Carrier
// For International the first International Carrier will become the default
// Validating Carrier.
//

namespace
{

struct IsSurfaceOrIndustrySegment
{
  bool operator()(const TravelSeg* seg) const
  {
    return (seg->isAir() ? static_cast<const AirSeg&>(*seg).carrier() == INDUSTRY_CARRIER : true);
  }
};

struct ToAirCast
{
  const AirSeg* operator()(const TravelSeg* seg) const { return static_cast<const AirSeg*>(seg); }
};

template <typename T, const T& (Loc::*getter)() const>
struct HasDifferent
{
  bool operator()(const AirSeg* seg) const
  {
    return ((seg->origin()->*getter)() != (seg->destination()->*getter)());
  }
};

typedef HasDifferent<IATAAreaCode, &Loc::area> HasDifferentAreas;
typedef HasDifferent<IATASubAreaCode, &Loc::subarea> HasDifferentSubAreas;
typedef HasDifferent<NationCode, &Loc::nation> HasDifferentNations;

struct IsInternational
{
  bool operator()(const AirSeg* seg) const
  {
    if (LocUtil::isScandinavia(*seg->origin()) && LocUtil::isScandinavia(*seg->destination()))
      return false;

    return LocUtil::isInternational(*seg->origin(), *seg->destination());
  }
};

inline bool
fromToArea(const AirSeg& seg, const IATAAreaCode& from, const IATAAreaCode& to)
{
  return (seg.origin()->area() == from) && (seg.destination()->area() == to);
}

template <typename P>
inline bool
findCarrier(const std::vector<const AirSeg*>& segments, CarrierCode& carrier)
{
  const std::vector<const AirSeg*>::const_iterator segment =
      std::find_if(segments.begin(), segments.end(), P());

  if (segment == segments.end())
    return false;

  carrier = (**segment).carrier();
  return true;
}

} // namespace

CarrierCode
ValidatingCarrierUpdater::find(const AirSegmentVec& segments) const
{
  if (UNLIKELY(segments.empty()))
    return CarrierCode();

  CarrierCode carrier;

  if (findCarrierBetweenAreas(segments, carrier) || findCarrierBetweenSubAreas(segments, carrier) ||
      findCarrierBetweenCountries(segments, carrier))
    return carrier;

  return segments.front()->carrier();
}

bool
ValidatingCarrierUpdater::findCarrierBetweenAreas(const AirSegmentVec& segments,
                                                  CarrierCode& carrier) const
{
  AirSegmentVec::const_iterator segment =
      std::find_if(segments.begin(), segments.end(), HasDifferentAreas());

  if (segment == segments.end())
    return false;

  if (fromToArea(**segment, IATA_AREA3, IATA_AREA2))
  {
    AirSegmentVec::const_iterator nextSegment =
        std::find_if(segment + 1, segments.end(), HasDifferentAreas());

    if ((nextSegment != segments.end()) && fromToArea(**nextSegment, IATA_AREA2, IATA_AREA1))
      segment = nextSegment;
  }

  carrier = (**segment).carrier();
  return true;
}

bool
ValidatingCarrierUpdater::findCarrierBetweenSubAreas(const AirSegmentVec& segments,
                                                     CarrierCode& carrier) const
{
  return findCarrier<HasDifferentSubAreas>(segments, carrier);
}

bool
ValidatingCarrierUpdater::findCarrierBetweenCountries(const AirSegmentVec& segments,
                                                      CarrierCode& carrier) const
{
  if (UNLIKELY(segments.empty()))
    return false;

  return (findCarrier<IsInternational>(segments, carrier) ||
          (LocUtil::isScandinavia(*segments.front()->origin()) &&
           findCarrier<HasDifferentNations>(segments, carrier)));
}

void
ValidatingCarrierUpdater::filterSegments(const std::vector<TravelSeg*>& travelSegs,
                                         AirSegmentVec& segments) const
{
  std::vector<TravelSeg*> tmp;
  tmp.reserve(travelSegs.size());
  std::remove_copy_if(
      travelSegs.begin(), travelSegs.end(), std::back_inserter(tmp), IsSurfaceOrIndustrySegment());
  segments.reserve(tmp.size());
  std::transform(tmp.begin(), tmp.end(), std::back_inserter(segments), ToAirCast());
}

DiagCollector&
ValidatingCarrierUpdater::diag() const
{
  if (_diag == nullptr)
  {
    DCFactory* factory = DCFactory::instance();
    _diag = factory->create(_trx);
    _diag->enable(Diagnostic191);
  }

  return *_diag;
}

bool
ValidatingCarrierUpdater::processSops(ShoppingTrx& shoppingTrx, const SopIdVec& sops)
{
  if (UNLIKELY(diag().isActive()))
    diag() << "\n\n** PROCESSING SOP COMBINATION";

  Itin itin;

  for (uint32_t legNo = 0; legNo < sops.size(); ++legNo)
  {
    const ShoppingTrx::SchedulingOption& sop = shoppingTrx.legs()[legNo].sop()[sops[legNo]];
    if (UNLIKELY(diag().isActive()))
      diag() << ((legNo == 0) ? " " : ",") << sop.originalSopId();

    std::vector<TravelSeg*>::const_iterator i, iEnd = sop.itin()->travelSeg().end();

    for (i = sop.itin()->travelSeg().begin(); i != iEnd; ++i)
      if (LIKELY(*i != nullptr))
        itin.travelSeg().push_back(*i);
  }

  update(itin, true);

  bool isValCxrFound = false;
  if (!fallback::fallbackValidatingCxrMultiSp(&_trx) || _trx.overrideFallbackValidationCXRMultiSP())
    isValCxrFound = itin.isValidatingCxrGsaDataForMultiSp();
  else
    isValCxrFound = (itin.validatingCxrGsaData() != nullptr);

  if (!fallback::fallbackValidatingCxrGTC(&_trx) &&
      !isValCxrFound &&
      !itin.validatingCarrier().empty())
  {
    isValCxrFound = true; // itin.validatingCarrier is populated for GTC carriers.
  }

  if (UNLIKELY(diag().isActive()))
    diag() << "\n** SOP " << (isValCxrFound ? "HAS" : "DOES NOT HAVE")
      << " VALID VALIDATING CARRIER(S) **\n";

  diag().flushMsg();
  return isValCxrFound;
}

void
ValidatingCarrierUpdater::validatigCarrierOverrideErrorMsg(std::vector<CarrierCode>& validatingCxrs,
    Diag191Collector* diag191) const
{
  std::string vcrlst;
  for (const CarrierCode& vcr : validatingCxrs)
  {
    vcrlst = vcrlst + vcr.c_str() + " ";
  }
  std::string  message = "MULTIPLE VALIDATING CARRIER OPTIONS - " + vcrlst;

  if (diag191)
  {
    *diag191 << message;
    diag191->flushMsg();
  }

  throw ErrorResponseException(ErrorResponseException::VALIDATING_CXR_ERROR, message.c_str());
}

// We need to set PricingTrx::countrySettlementPlanInfo for single settlement-plan context
void
ValidatingCarrierUpdater::resetForSingleSettlementPlan(Itin& itin,
    const SpValidatingCxrGSADataMap& spGsaDataMap)
{
  const SettlementPlanType& spType =
    ValidatingCxrUtil::getSettlementPlanFromHierarchy(spGsaDataMap);

  if (!_trx.countrySettlementPlanInfo() ||
      _trx.countrySettlementPlanInfo()->getSettlementPlanTypeCode() != spType)
  {

    const std::vector<CountrySettlementPlanInfo*>& countrySettlementPlanInfos =
        _trx.isLockingNeededInShoppingPQ() ?
          _trx.getCopyOfCountrySettlementPlanInfos() :
          _trx.countrySettlementPlanInfos();

    for (CountrySettlementPlanInfo* cspInfo : countrySettlementPlanInfos)
      if (cspInfo->getSettlementPlanTypeCode() == spType)
      {
        _trx.countrySettlementPlanInfo() = cspInfo;
        break;
      }
  }

  auto spValCxrPair = itin.spValidatingCxrGsaDataMap()->find(spType);
  if (spValCxrPair != itin.spValidatingCxrGsaDataMap()->end())
    itin.validatingCxrGsaData() = const_cast<ValidatingCxrGSAData*>(spValCxrPair->second);
}

bool
ValidatingCarrierUpdater::setDefaultValidatingCxrForCommandPricing(
  Itin& itin,
  const SettlementPlanType& primarySp,
  CarrierCode& defaultValidatingCxr,
  CarrierCode& defaultMarketingCxr) const
{
  if (!itin.spValidatingCxrGsaDataMap())
    return false;

  bool retVal = false;
  std::vector<CarrierCode> validatingCxrs;
  for (const auto& item : *itin.spValidatingCxrGsaDataMap())
  {
    if (!item.second || item.first == primarySp)
      continue;

    validatingCxrs.clear();

    defaultValidatingCxr = "";
    defaultMarketingCxr = "";

    itin.getValidatingCarriers(*item.second, validatingCxrs);
    DefaultValidatingCarrierFinder dvcxrFinder(_trx, itin, item.first);
    retVal = dvcxrFinder.determineDefaultValidatingCarrier(
        validatingCxrs,
        defaultValidatingCxr,
        defaultMarketingCxr);

    if (retVal)
      break;
  }
  return retVal;
}

void
ValidatingCarrierUpdater::determineTraditionalValidatingCarrier(Itin &itin) const
{
  AirSegmentVec segments;
  filterSegments(itin.travelSeg(), segments);
  CarrierCode carrier = find(segments);
  itin.traditionalValidatingCxr() = carrier;
}

} // tse
