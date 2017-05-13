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
//-------------------------------------------------------------------
#include "ServiceFees/OptionalFeeCollector.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/ServiceFeeUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TSELatencyData.h"
#include "Common/TseUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TrxAborter.h"
#include "DBAccess/MerchActivationInfo.h"
#include "DBAccess/OptionalServicesActivationInfo.h"
#include "DBAccess/ServiceGroupInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag875Collector.h"
#include "Diagnostic/Diag877Collector.h"
#include "Diagnostic/Diag880Collector.h"
#include "Rules/RuleUtil.h"
#include "ServiceFees/CombinationGenerator.h"
#include "ServiceFees/MerchActivationValidator.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/OptionalFeeConcurValidator.h"
#include "ServiceFees/OptionalServicesGroupCodeFilter.h"
#include "ServiceFees/OptionalServicesValidator.h"
#include "ServiceFees/PseudoFarePathBuilder.h"
#include "ServiceFees/ServiceFeesGroup.h"

#include <boost/bind.hpp>

#include <algorithm>
#include <utility>

namespace tse
{
FALLBACK_DECL(fallbackAB240);
FALLBACK_DECL(fallbackNewGroup99Validation);

namespace
{
ConfigurableValue<int>
sliceAndDiceTimeoutMS("SERVICE_FEES_SVC", "SLICE_AND_DICE_TIMEOUT_IN_MS", 0);

Logger
logger("atseintl.ServiceFees.OptionalFeeCollector");
}

const Indicator OptionalFeeCollector::S5_INDCRXIND_CARRIER = 'C';
const Indicator OptionalFeeCollector::S5_INDCRXIND_INDUSTRY = 'I';

namespace
{
struct IsConfirmed : public std::unary_function<const tse::TravelSeg*, bool>
{
  bool operator()(const tse::TravelSeg* seg) const
  {
    return seg->segmentType() == tse::Arunk || seg->segmentType() == tse::Surface ||
           seg->resStatus() == tse::CONFIRM_RES_STATUS ||
           seg->resStatus() == tse::NOSEAT_RES_STATUS;
  }
};

class IsSameSrvGroup
{
  const ServiceGroup _srvGroup;

public:
  IsSameSrvGroup(const ServiceGroup& srvGroup) : _srvGroup(srvGroup) {}

  bool operator()(const ServiceGroup* srvGroup) const { return (*srvGroup) == _srvGroup; }
};

class IsSameSrvGroupInCxrMap
{
  const ServiceGroup _srvGroup;

public:
  IsSameSrvGroupInCxrMap(const ServiceGroup& srvGroup) : _srvGroup(srvGroup) {}

  bool
  operator()(std::map<const CarrierCode, std::vector<ServiceGroup*>>::value_type const& pair) const
  {
    return std::find_if(pair.second.begin(), pair.second.end(), IsSameSrvGroup(_srvGroup)) !=
           pair.second.end();
  }
};

class IsSameOCFeeInCxrMap
{
  const OCFees& _fees;

public:
  IsSameOCFeeInCxrMap(const OCFees& fees) : _fees(fees) {}

  bool
  operator()(std::map<const CarrierCode, std::vector<ServiceGroup*>>::value_type const& pair) const
  {
    if (pair.first == _fees.carrierCode())
    {
      return std::find_if(pair.second.begin(),
                          pair.second.end(),
                          IsSameSrvGroup(_fees.subCodeInfo()->serviceGroup())) != pair.second.end();
    }
    return false;
  }
};

class IsNotIncluded
{
public:
  bool operator()(MerchActivationInfo* mai) const { return mai->includeInd() == 'N'; }
};
}

OptionalFeeCollector::OptionalFeeCollector(PricingTrx& trx)
  : SliceAndDice(trx,
                 _isInternationalOCF,
                 _isRoundTripOCF,
                 _stopMatchProcessOCF,
                 _ts2ssOCF,
                 _cXrGrpOCF,
                 _allGroupCodesOCF,
                 _shoppingOCF,
                 _needFirstMatchOnlyOCF,
                 _numberOfOcFeesForItinOCF,
                 _timeOutOCF,
                 _mutexOCF),
    _shoppingOCF(false),
    _numberOfOcFeesForItinOCF(0),
    _stopMatchProcessOCF(false),
    _needFirstMatchOnlyOCF(false),
    _checkMerchCxrPref(false),
    _timeOutOCF(false)
{
}

OptionalFeeCollector::OptionalFeeCollector(const OptionalFeeCollector& rhs)
  : SliceAndDice(rhs),
    _shoppingOCF(rhs._shoppingOCF),
    _numberOfOcFeesForItinOCF(rhs._numberOfOcFeesForItinOCF),
    _stopMatchProcessOCF(rhs._stopMatchProcessOCF),
    _needFirstMatchOnlyOCF(rhs._needFirstMatchOnlyOCF),
    _checkMerchCxrPref(rhs._checkMerchCxrPref),
    _timeOutOCF(rhs._timeOutOCF)
{
}

void
OptionalFeeCollector::collect()
{
  TSELatencyData metrics(_trx, "OC COLLECT PROCESS");
  LOG4CXX_DEBUG(logger, "Entered OptionalFeeCollector::collect()");
  createDiag();

  if (!isAgencyActive()) // Agency PCC is not active
    return;

  if (!setsUpIndForShopping()) // Shopping does not supply Group codes
    return;

  std::vector<Itin*>::const_iterator itinsIt = _trx.itin().begin();
  std::vector<Itin*>::const_iterator itinsEndIt = _trx.itin().end();

  for (; itinsIt != itinsEndIt; ++itinsIt)
  {
    if (*itinsIt)
    {
      _isInternationalOCF.insert(std::make_pair(*itinsIt, internationalJouneyType(**itinsIt)));
      _isRoundTripOCF.insert(std::make_pair(*itinsIt, roundTripJouneyType(**itinsIt)));
    }
  }

  _trx.getOptions()->isOCHistorical() = TrxUtil::determineIfNotCurrentRequest(_trx);
  bool multiTkt = _trx.getRequest()->multiTicketActive();

  // Deprecated - to be replaced by OptionalServicesGroupCodeFilter
  std::vector<ServiceGroup> groupNV; // Group codes not valid
  std::vector<ServiceGroup> groupNA; // Group codes not active
  std::vector<ServiceGroup> groupNP; // Group codes not processed

  // Deprecated, to be replaced by OptionalServicesGroupCodeFilter
  identifyGroupCodesToBeProcessed(groupNV, groupNA, groupNP);

  std::vector<Itin*>::const_iterator itinI = _trx.itin().begin();

  for (; itinI != _trx.itin().end(); ++itinI)
  {
    Itin& itin = **itinI;

    if (itin.errResponseCode() != ErrorResponseException::NO_ERROR)
      continue;

    if (_shopping || multiTkt)
    {
      _numberOfOcFeesForItinOCF = 0;
      _stopMatchProcessOCF = false;
    }

    if (_timeOut && _shopping)
    {
      timeOutPostProcessing(itin, groupNV, groupNA);
      continue;
    }

    std::vector<FarePath*> fps = itin.farePath();
    for (const auto fp : fps)
    {
      _farePath = fp;
      if (_farePath == nullptr)
        continue;

      processOCFees();
      if (!finalCheckShoppingPricing(itin))
        break;
    }
    if (_timeOut)
      timeOutPostProcessing(itin, groupNV, groupNA);
    else
      cleanUpOCFees(itin);
    addInvalidGroups(itin, groupNA, groupNP, groupNV);
  }
  endDiag();
  LOG4CXX_DEBUG(logger, "Leaving OptionalFeeCollector::collect() - OC Fee done all Itins");
}

bool
OptionalFeeCollector::isAgencyActive()
{
  MerchActivationValidator marchAct(_trx, diag875());
  CarrierCode deactivated = ANY_CARRIER;
  const PseudoCityCode& pcc = _trx.getRequest()->ticketingAgent()->tvlAgencyPCC();

  const std::vector<MerchActivationInfo*>& merchActInfos =
      getMerchActivation(marchAct, deactivated, pcc);

  if (std::find_if(merchActInfos.begin(), merchActInfos.end(), IsNotIncluded()) !=
      merchActInfos.end())
  {
    printPccIsDeactivated(pcc);
    endDiag();
    LOG4CXX_DEBUG(logger,
                  "Leaving OptionalFeeCollector::isAgencyActive() - PCC is not active for OC Fee");
    return false;
  }
  return true;
}

bool
OptionalFeeCollector::setsUpIndForShopping()
{
  _shoppingOCF = false;
  // Shopping must populate Group code(s) or 'AE' in the PricingOption
  // for OC Fees processing
  if (_trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    if ((false == _trx.getOptions()->isProcessAllGroups()) &&
        (_trx.getOptions()->serviceGroupsVec().empty()))
    {
      printNoGroupCodeProvided();
      endDiag();
      return false;
    }
    if (_trx.getOptions()->maxNumberOfOcFeesForItin() == 0)
    {
      printNoOptionsRequested();
      endDiag();
      return false;
    }
    if (_trx.getOptions()->maxNumberOfOcFeesForItin() == -1) // behave as pricing
    {
    }
    else
      _shoppingOCF = true;
  }
  return true;
}

void
OptionalFeeCollector::identifyGroupCodesToBeProcessed(
    std::vector<ServiceGroup>& grNotValid,
    std::vector<ServiceGroup>& grNotValidForTicketDate,
    std::vector<ServiceGroup>& grNotProcessedForTicketDate)
{
  std::vector<ServiceGroup> grValid;
  getAllATPGroupCodes(); // Access SERVICEGROUP table to get all ATPCO defined Group codes
  std::vector<ServiceGroup> groupCodes; // Deprecated
  _trx.getOptions()->getGroupCodes(_trx.getOptions()->serviceGroupsVec(), groupCodes); // Deprecated

  if (!fallback::fallbackAB240(&_trx))
  {
    std::set<ServiceGroup> grActive;
    this->getAllActiveGroupCodes(grActive);

    if (_trx.getOptions()->serviceGroupsVec().empty() &&
        (!_trx.getOptions()->isProcessAllGroups()) && (_trx.getOptions()->isWPwithOutAE()))
    {
      _needFirstMatchOnlyOCF = true;
    }

    OptionalServicesGroupCodeFilter groupFilter(
        _trx, this->shouldProcessAllGroups(), _allGroupCodes, grActive);

    std::copy(
        groupFilter.grValid().begin(), groupFilter.grValid().end(), std::back_inserter(grValid));
    std::copy(groupFilter.grNotValid().begin(),
              groupFilter.grNotValid().end(),
              std::back_inserter(grNotValid));
    std::copy(groupFilter.grNotActive().begin(),
              groupFilter.grNotActive().end(),
              std::back_inserter(grNotValidForTicketDate));
    std::copy(groupFilter.grNotProcessed().begin(),
              groupFilter.grNotProcessed().end(),
              std::back_inserter(grNotProcessedForTicketDate));
  }
  else
  { // Deprecated - to be removed
    if (groupCodes.empty()) // it's pricing entry, shopping already tested before this point
    {
      getAllActiveGroupCodes(grValid); // access the OPTIONALSERVICEACTIVATION table
      if (!_trx.getOptions()->isProcessAllGroups())
      {
        if (_trx.getOptions()->isWPwithOutAE())
          _needFirstMatchOnlyOCF = true;
      }
      sortOCFeeGroup(grValid);
      getGroupCodesNotProcessedForTicketingDate(
          grValid, grNotValidForTicketDate, grNotProcessedForTicketDate);
    }
    else
    {
      filterOutInputGroupCodes(groupCodes, grValid, grNotValid, grNotValidForTicketDate);
      selectActiveNotActiveGroupCodes(
          grValid, grNotValidForTicketDate, grNotProcessedForTicketDate);
    }

    filterOutForACS(grValid, grNotValidForTicketDate);
  }

  for (std::vector<Itin*>::const_iterator itinI = _trx.itin().begin(); itinI != _trx.itin().end();
       ++itinI)
  {
    std::transform(grValid.begin(),
                   grValid.end(),
                   std::back_inserter((*itinI)->ocFeesGroup()),
                   boost::bind(&OptionalFeeCollector::addNewServiceGroup, this, _1));
  }

  printGroupCodesRequested(groupCodes, grNotValid, grNotValidForTicketDate);
  LOG4CXX_DEBUG(
      logger,
      "Leaving OptionalFeeCollector::identifyGroupCodesToBeProcessed(), valid Group codes "
          << (grValid.empty() ? "not found" : "are found"));
}

void
OptionalFeeCollector::filterOutForACS(std::vector<ServiceGroup>& grValid,
                                      std::vector<ServiceGroup>& grNotValidForTicketDate)
{
  const AncRequest* req = dynamic_cast<AncRequest*>(_trx.getRequest());

  if (_trx.billing()->requestPath() == ACS_PO_ATSE_PATH ||
      _trx.billing()->actionCode().substr(0, 5) == "MISC6" || (req && req->isWPBGRequest()))
  {
    std::vector<ServiceGroup>::iterator newEndIT =
        std::remove_if(grValid.begin(), grValid.end(), ServiceFeeUtil::checkServiceGroupForAcs);
    grValid.erase(newEndIT, grValid.end());

    sortOCFeeGroup(grValid); // to remove duplicate group codes
    std::vector<ServiceGroup>::iterator newBeginIT1 = grValid.begin();
    // add not valid groups of ACS to not valid vector
    for (; newBeginIT1 != grValid.end(); newBeginIT1++)
    {
      ServiceGroup sg = *newBeginIT1;
      if (ServiceFeeUtil::isServiceGroupInvalidForAcs(sg))
        grNotValidForTicketDate.push_back(sg);
    }
    std::vector<ServiceGroup>::iterator newBeginIT2 =
        std::remove_if(grValid.begin(), grValid.end(), ServiceFeeUtil::isServiceGroupInvalidForAcs);
    grValid.erase(newBeginIT2, grValid.end());
  } // if ACS
}

void
OptionalFeeCollector::getAllATPGroupCodes()
{
  _allGroupCodesOCF = _trx.dataHandle().getAllServiceGroups();
  LOG4CXX_DEBUG(
      logger,
      "Leaving OptionalFeeCollector::getAllATPGroupCodes(): collect all ATPCO defined Group codes");
}

void
OptionalFeeCollector::sortOCFeeGroup(std::vector<ServiceGroup>& grValid)
{
  if (!grValid.empty())
  {
    std::sort(grValid.begin(), grValid.end());
    grValid.erase(std::unique(grValid.begin(), grValid.end()), grValid.end()); // remove duplicates
  }
}

void
OptionalFeeCollector::filterOutInputGroupCodes(std::vector<ServiceGroup>& groupCodes,
                                               std::vector<ServiceGroup>& grValid,
                                               std::vector<ServiceGroup>& grNotValid,
                                               std::vector<ServiceGroup>& grNotValidForTicketDate)
{
  for (const auto gc : groupCodes)
  {
    bool invalid = false;
    if (!isATPActiveGroupFound(gc, invalid, grNotValidForTicketDate))
    {
      grNotValid.push_back(gc);
      invalid = true;
    }
    if (!invalid)
      grValid.push_back(gc);
  }
}

bool
OptionalFeeCollector::isATPActiveGroupFound(ServiceGroup sg,
                                            bool& invalid,
                                            std::vector<ServiceGroup>& grNotValidForTicketDate)
{
  for (const auto elem : _allGroupCodes)
  {
    if (elem->svcGroup() == sg)
    {
      if (!_trx.ticketingDate().isBetween(elem->effDate().date(), elem->discDate().date()))
      {
        grNotValidForTicketDate.push_back(sg);
        invalid = true;
      }
      return true;
    }
  }
  return false;
}

void
OptionalFeeCollector::selectActiveNotActiveGroupCodes(
    std::vector<ServiceGroup>& grValid,
    std::vector<ServiceGroup>& grNotValidForTicketDate,
    std::vector<ServiceGroup>& grNotProcessedForTicketDate)
{
  std::vector<ServiceGroup> grActive;
  getAllActiveGroupCodes(grActive);

  if (grActive.empty())
  {
    swap(grNotValidForTicketDate, grValid);
    grValid.clear();
    return;
  }

  for (std::vector<ServiceGroup>::iterator grI = grValid.begin(); grI != grValid.end();)
  {
    ServiceGroup sg = *grI;
    if (find(grActive.begin(), grActive.end(), sg) == grActive.end())
    {
      grNotValidForTicketDate.push_back(sg);
      grValid.erase(grI);
    }
    else
      ++grI;
  }

  if (shouldProcessAllGroups()) // Pricing - process all groups instead of just requested
  {
    sortOCFeeGroup(grActive); // sort not requested groups
    for (std::vector<ServiceGroup>::iterator grI = grActive.begin(); grI != grActive.end();)
    {
      ServiceGroup sg = *grI;
      if (find(grValid.begin(), grValid.end(), sg) == grValid.end())
      {
        grValid.push_back(sg);
      }
      else
        ++grI;
    }
    // swap(grValid, grActive); //Process all group codes that are active instead of just requested
    // - so swap
    getGroupCodesNotProcessedForTicketingDate(
        grValid, grNotValidForTicketDate, grNotProcessedForTicketDate);
  }
}

bool
OptionalFeeCollector::shouldProcessAllGroups() const
{
  return !_shopping;
}

void
OptionalFeeCollector::getGroupCodesNotProcessedForTicketingDate(
    std::vector<ServiceGroup>& grValid,
    std::vector<ServiceGroup>& grNotValidForTicketDate,
    std::vector<ServiceGroup>& grNotProcessedForTicketDate)
{
  for (const auto elem : _allGroupCodes)
  {
    ServiceGroup sg = elem->svcGroup();
    if (find(grValid.begin(), grValid.end(), sg) == grValid.end() &&
        find(grNotValidForTicketDate.begin(), grNotValidForTicketDate.end(), sg) ==
            grNotValidForTicketDate.end() &&
        find(grNotProcessedForTicketDate.begin(), grNotProcessedForTicketDate.end(), sg) ==
            grNotProcessedForTicketDate.end())
    {
      grNotProcessedForTicketDate.push_back(sg);
    }
  }
  sortOCFeeGroup(grNotProcessedForTicketDate);
}

UserApplCode
OptionalFeeCollector::getUserApplCode() const
{
  UserApplCode userCode = EMPTY_STRING();
  if (_trx.activationFlags().isAB240())
    userCode = _trx.billing()->partitionID();
  else if (_trx.getRequest()->ticketingAgent()->sabre1SUser())
    userCode = SABRE_USER;
  else if (_trx.getRequest()->ticketingAgent()->abacusUser())
    userCode = ABACUS_USER;
  else if (_trx.getRequest()->ticketingAgent()->axessUser())
    userCode = AXESS_USER;
  else
    userCode = INFINI_USER;
  return userCode;
}

void
OptionalFeeCollector::getAllActiveGroupCodes(std::set<ServiceGroup>& grCodes)
{
  Indicator crs = _trx.activationFlags().isAB240() ? MULTIHOST_USER_APPL : CRS_USER_APPL;
  UserApplCode userCode = getUserApplCode();

  static const std::string appPricing = "PRICING";
  getAllActiveGroupCodesForApplication(grCodes, crs, userCode, appPricing);

  static const std::string appAll = "ALL";
  getAllActiveGroupCodesForApplication(grCodes, crs, userCode, appAll);
  LOG4CXX_DEBUG(
      logger,
      "Leaving OptionalFeeCollector::getAllActiveGroupCodes(): collect all active Group codes");
}

void
OptionalFeeCollector::getAllActiveGroupCodesForApplication(std::set<ServiceGroup>& gValid,
                                                           const Indicator crs,
                                                           const UserApplCode userCode,
                                                           const std::string& application)
{
  const std::vector<OptionalServicesActivationInfo*>& optServiseActivInfos =
      getOptServiceActivation(crs, userCode, application);

  if (!optServiseActivInfos.empty())
  {
    for (const auto optServiseActivInfo : optServiseActivInfos)
    {
      if (!_trx.ticketingDate().isBetween(optServiseActivInfo->effDate().date(),
                                          optServiseActivInfo->discDate().date()))
        continue;

      gValid.insert(optServiseActivInfo->groupCode());
    }
  }
}

void
OptionalFeeCollector::getAllActiveGroupCodes(std::vector<ServiceGroup>& gValid)
{
  Indicator crs = _trx.activationFlags().isAB240() ? MULTIHOST_USER_APPL : CRS_USER_APPL;
  UserApplCode userCode = getUserApplCode();

  std::string application = "PRICING";
  getAllActiveGroupCodesForApplication(gValid, crs, userCode, application);

  application = "ALL";
  getAllActiveGroupCodesForApplication(gValid, crs, userCode, application);
  LOG4CXX_DEBUG(
      logger,
      "Leaving OptionalFeeCollector::getAllActiveGroupCodes(): collect all active Group codes");
}

void
OptionalFeeCollector::getAllActiveGroupCodesForApplication(std::vector<ServiceGroup>& gValid,
                                                           const Indicator crs,
                                                           const UserApplCode userCode,
                                                           const std::string& application)
{
  const std::vector<OptionalServicesActivationInfo*>& optServiseActivInfos =
      getOptServiceActivation(crs, userCode, application);

  if (!optServiseActivInfos.empty())
  {
    for (const auto optServiseActivInfo : optServiseActivInfos)
    {
      if (!_trx.ticketingDate().isBetween(optServiseActivInfo->effDate().date(),
                                          optServiseActivInfo->discDate().date()))
        continue;

      gValid.push_back(optServiseActivInfo->groupCode());
    }
  }
}

const std::vector<OptionalServicesActivationInfo*>&
OptionalFeeCollector::getOptServiceActivation(Indicator crs,
                                              const UserApplCode& userCode,
                                              const std::string& application)
{
  return _trx.dataHandle().getOptServiceActivation(crs, userCode, application);
}

const std::vector<MerchActivationInfo*>&
OptionalFeeCollector::getMerchActivation(MerchActivationValidator& validator,
                                         const CarrierCode& carrierCode,
                                         const PseudoCityCode& pcc) const
{
  return validator.getMerchActivation(carrierCode, pcc);
}

bool
OptionalFeeCollector::retrieveMerchActivation(MerchActivationValidator& validator,
                                              const CarrierCode& carrierCode,
                                              std::vector<ServiceGroup*>& groupCode,
                                              std::vector<ServiceGroup*>& dispOnlyGroups) const
{
  return validator.retrieveMerchActivaionData(carrierCode, groupCode, dispOnlyGroups);
}

void
OptionalFeeCollector::processOCFees()
{
  setJourneyDestination();
  printJourneyDestination();
  defineMarketingOperatingCarriers();
  if (!checkAllSegsConfirmed(
          _farePath->itin()->travelSeg().begin(), _farePath->itin()->travelSeg().end(), false))
    checkAllSegsUnconfirmed(_farePath->itin()->travelSeg().begin(),
                            _farePath->itin()->travelSeg().end());
  if (!processCarrierMMRecords())
    return;

  if (_diag875)
    _diag875->displayTicketDetail(&_trx);

  if (TrxUtil::isUSAirAncillaryActivated(_trx))
  {
    _checkMerchCxrPref = true;
    printActiveMrktDrivenCxrHeader();
    std::vector<MerchCarrierPreferenceInfo*> mCxrPrefVec;
    if (!processMerchCxrPrefData(mCxrPrefVec))
      return;

    if (mCxrPrefVec.empty())
      printActiveMrktDrivenNCxrNoData();
    else
      printActiveMrktDrivenCxrData(mCxrPrefVec);
  }

  ServiceFeeUtil::collectSegmentStatus(*_farePath, _ts2ssOCF);

  for (int unitNo = 0; unitNo < 2; unitNo++) // Loop to process portions of travel
  {
    _slice_And_Dice = false;
    processPortionOfTravel(
        unitNo,
        std::bind1st(std::mem_fun(&OptionalFeeCollector::processLargestPortionOfTvl), this));
    if (_stopMatchProcess)
    {
      LOG4CXX_DEBUG(logger,
                    "Leaving OptionalFeeCollector::processOCFees() - Stop Match process found");
      break;
    }
  }
}

bool
OptionalFeeCollector::processMerchCxrPrefData(std::vector<MerchCarrierPreferenceInfo*>& mCxrPrefVec)
    const
{
  std::vector<tse::CarrierCode> cxrCodes;
  if (!collectOperMktCxr(cxrCodes, _checkMerchCxrPref))
    return false;

  typedef std::vector<ServiceFeesGroup*>::const_iterator SrvFeesGrpI;
  for (SrvFeesGrpI srvFeesGrpI = _farePath->itin()->ocFeesGroup().begin();
       srvFeesGrpI != _farePath->itin()->ocFeesGroup().end();
       ++srvFeesGrpI)
  {
    if ((*srvFeesGrpI)->groupDescription().empty())
      continue;

    typedef std::vector<tse::CarrierCode>::const_iterator CandCarrierI;

    for (CandCarrierI candCarrierI = cxrCodes.begin(); candCarrierI != cxrCodes.end();
         ++candCarrierI)
    {
      // loop through each carrier code and each service group and get data from merch carrier
      // preference table
      const MerchCarrierPreferenceInfo* merchCxrPrefInfos =
          getMerchCxrPrefInfo(*candCarrierI, (*srvFeesGrpI)->groupCode());
      if (merchCxrPrefInfos != nullptr)
      {
        mCxrPrefVec.push_back(const_cast<MerchCarrierPreferenceInfo*>(
            merchCxrPrefInfos)); // get all merch carrier preference records
        if (merchCxrPrefInfos->altProcessInd() == 'Y')
          (*srvFeesGrpI)->merchCxrPref()[*candCarrierI] =
              const_cast<MerchCarrierPreferenceInfo*>(merchCxrPrefInfos);
      } // if condition for mCxrPref records
    } // Carrier Loop
  } // Service group loop
  return true;
}

const MerchCarrierPreferenceInfo*
OptionalFeeCollector::getMerchCxrPrefInfo(const CarrierCode& carrier, const ServiceGroup& groupCode)
    const
{
  return _trx.dataHandle().getMerchCarrierPreference(carrier, groupCode);
}

void
OptionalFeeCollector::getValidRoutes(
    std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                          std::vector<TravelSeg*>::const_iterator>>& routes,
    std::vector<TravelSeg*>::const_iterator startSeg,
    std::vector<TravelSeg*>::const_iterator endSeg)
{
  if (std::distance(startSeg, endSeg) > 1)
  {
    typedef std::vector<TravelSeg*>::const_iterator TSI;
    if (!dynamic_cast<ArunkSeg*>(*startSeg))
    {
      for (TSI tsie = startSeg + 1; tsie != endSeg; ++tsie)
      {
        if (dynamic_cast<ArunkSeg*>(*(tsie - 1)) ||
            (*startSeg)->origAirport() == (*(tsie - 1))->destAirport())
          continue;
        routes.push_back(std::make_pair(startSeg, tsie));
      }
    }
    for (TSI tsi = startSeg + 1; tsi != endSeg; ++tsi)
    {
      if (dynamic_cast<ArunkSeg*>(*tsi))
        continue;
      for (TSI tsie = tsi; tsie != endSeg; ++tsie)
      {
        if (dynamic_cast<ArunkSeg*>(*tsie) || (*tsi)->origAirport() == (*tsie)->destAirport())
          continue;
        routes.push_back(std::make_pair(tsi, tsie + 1));
      }
    }
  }
}

int
OptionalFeeCollector::getNumberOfSegments(std::vector<TravelSeg*>::const_iterator firstSeg,
                                          std::vector<TravelSeg*>::const_iterator endSeg) const
{
  return std::distance(firstSeg, endSeg);
}

ServiceFeesGroup::FindSolution
OptionalFeeCollector::getProcessingMethod(ServiceFeesGroup& srvGroup)
{
  return srvGroup.getFindSolutionForSubCode();
}

void
OptionalFeeCollector::multiThreadSliceAndDice(
    std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                          std::vector<TravelSeg*>::const_iterator>>& routes,
    int unitNo)
{
  multiThreadSliceAndDiceImpl<OptionalFeeCollector>(routes, unitNo);
}

void
OptionalFeeCollector::sliceAndDice(int unitNo)
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " unitNo=" << unitNo);
  TSELatencyData metrics(_trx, "OC SLICE AND DICE PROCESS");
  std::vector<TravelSeg*>::const_iterator firstSeg = _beginsOfUOT[unitNo];
  std::vector<TravelSeg*>::const_iterator endSeg = _beginsOfUOT[unitNo + 1];

  LOG4CXX_DEBUG(logger, "Starts OptionalFeeCollector::sliceAndDice()");
  std::vector<std::pair<std::vector<TravelSeg*>::const_iterator,
                        std::vector<TravelSeg*>::const_iterator>> routes;
  getValidRoutes(routes, firstSeg, endSeg);

  multiThreadSliceAndDice(routes, unitNo);

  defineMarketingOperatingCarriers();

  TseUtil::SolutionSet solutions;
  CombinationGenerator generator(
      std::distance(_beginsOfUOT[unitNo], _beginsOfUOT[unitNo + 1]), routes, _beginsOfUOT[unitNo]);

  int sdTimeoutMS = 0;
  TrxAborter* aborter = _trx.aborter();
  if (aborter == nullptr)
  {
    sdTimeoutMS = sliceAndDiceTimeoutMS.getValue();
  }
  else
  {
    sdTimeoutMS = (aborter->getHurryAt() - time(nullptr)) * 1000;
  }
  sdTimeoutMS /=
      _farePath->itin()->farePath().size() * (_isRoundTrip.getValForKey(_farePath->itin()) ? 2 : 1);

  LOG4CXX_DEBUG(logger, "OptionalFeeCollector::sliceAndDice(): calls generator.generate()");
  generator.generate(solutions, sdTimeoutMS);
  int numberOfSegments = getNumberOfSegments(_beginsOfUOT[unitNo], _beginsOfUOT[unitNo + 1]);

  for (int i = 1; i < numberOfSegments; ++i)
  {
    if (isTimeOut(i))
    {
      LOG4CXX_DEBUG(
          logger,
          "Leaving OptionalFeeCollector::sliceAndDice(): time out after generator.generate()");
      return;
    }

    if (isAncillaryPricing())
    {
      generator.generate(solutions, i, getFilterMask(unitNo), sdTimeoutMS);
    }
    else
      generator.generate(solutions, i, sdTimeoutMS);
  }
  if (solutions.empty())
    return;

  uint16_t multiTicketNbr = 0;
  if (_trx.getRequest() && _trx.getRequest()->multiTicketActive() &&
      _farePath->itin()->getMultiTktItinOrderNum() > 0)
    multiTicketNbr = _farePath->itin()->getMultiTktItinOrderNum();

  LOG4CXX_DEBUG(logger,
                "Starts OptionalFeeCollector::sliceAndDice():ServiceFeesGroup::findSolution()");
  typedef std::vector<ServiceFeesGroup*>::const_iterator SrvFeesGrpI;
  for (SrvFeesGrpI srvFeesGrpI = _farePath->itin()->ocFeesGroup().begin();
       srvFeesGrpI != _farePath->itin()->ocFeesGroup().end();
       ++srvFeesGrpI)
  {
    if (TrxUtil::isUSAirAncillaryActivated(_trx) && _singleFeesGroupValidation &&
        _singleFeesGroupValidation != *srvFeesGrpI)
      continue;
    (*srvFeesGrpI)->findSolution(unitNo,
                                 solutions,
                                 firstSeg,
                                 endSeg,
                                 _diag880,
                                 _trx.ticketingDate(),
                                 getProcessingMethod(**srvFeesGrpI),
                                 multiTicketNbr);
  }
  for (SrvFeesGrpI srvFeesGrpI = _farePath->itin()->ocFeesGroup().begin();
       srvFeesGrpI != _farePath->itin()->ocFeesGroup().end();
       ++srvFeesGrpI)
  {
    if (!(*srvFeesGrpI)->sliceDicePassed())
    {
      _stopMatchProcessOCF = true;
      _timeOut = true;
      LOG4CXX_DEBUG(logger,
                    "Leaving OptionalFeeCollector::sliceAndDice(): time out after findSolution()");
      return;
    }
  }
  LOG4CXX_DEBUG(logger, "Leaving OptionalFeeCollector::sliceAndDice(): all solutions processed");
}

void
OptionalFeeCollector::processLargestPortionOfTvl(int unitNo)
{
  TSELatencyData metrics(_trx, "OC LARGEST PORTION PROCESS");

  processPortionOfTvl(unitNo);

  if (_stopMatchProcess || isTimeOut(unitNo))
  {
    LOG4CXX_DEBUG(logger,
                  "Leaving OptionalFeeCollector::processLargestPortionOfTvl(): time out "
                  "after processServiceFeesGroups()");
    return;
  }

  _slice_And_Dice = true;
  sliceAndDice(unitNo);
}

bool
OptionalFeeCollector::collectOperMktCxr(std::vector<tse::CarrierCode>& cxrCodes,
                                        bool callFrmMerchCxrPref) const
{
  for (int unitNo = 0; unitNo < 2;
       unitNo++) // Loop to process carrier data from different portions of tarvel
  {
    for (const auto elem : _operatingCxr[unitNo])
      cxrCodes.push_back(elem); // get all operating carriers

    for (const auto elem : _marketingCxr[unitNo])
      cxrCodes.push_back(elem); // get all marketing carriers
  }

  if (!fallback::fallbackNewGroup99Validation(&_trx) && TrxUtil::isRequestFromAS(_trx))
  {
    CarrierCode partitionCxr = MCPCarrierUtil::swapToActual(&_trx, _trx.billing()->partitionID());
    cxrCodes.push_back(partitionCxr); // add partition carrier
  }

  if (!_trx.billing()->validatingCarrier().empty())
  {
    CarrierCode validatingCxr =
        MCPCarrierUtil::swapToActual(&_trx, _trx.billing()->validatingCarrier());
    cxrCodes.push_back(validatingCxr);
  }

  if (cxrCodes.empty())
  {
    if (!callFrmMerchCxrPref)
      printNoMerchActiveCxrGroup();
    return false;
  }

  std::sort(cxrCodes.begin(), cxrCodes.end());
  cxrCodes.erase(std::unique(cxrCodes.begin(), cxrCodes.end()),
                 cxrCodes.end()); // process duplicate carriers
  if (!callFrmMerchCxrPref)
    printActiveCxrGroupHeader();

  return true;
}

bool
OptionalFeeCollector::processCarrierMMRecords()
{
  std::vector<tse::CarrierCode> cxrCodes;
  if (!collectOperMktCxr(cxrCodes))
    return false;

  std::vector<CarrierCode>::const_iterator cxrItr = cxrCodes.begin();
  for (; cxrItr != cxrCodes.end(); ++cxrItr)
  {
    MerchActivationValidator marchAct(_trx, diag875());
    std::vector<ServiceGroup*> groups;
    std::vector<ServiceGroup*> dispOnlyGroups;
    if (!retrieveMerchActivation(marchAct, *cxrItr, groups, dispOnlyGroups))
    {
      if (cxrItr->empty()) // request from neiether HC nor TN
        return false;
    }
    else
    {
      const CarrierCode carrier = *cxrItr;
      _cXrGrpOCF.insert(
          std::map<const CarrierCode, std::vector<ServiceGroup*>>::value_type(carrier, groups));
      _cXrDispOnlyGrp.insert(std::map<const CarrierCode, std::vector<ServiceGroup*>>::value_type(
          carrier, dispOnlyGroups));
    }
  } // carrier for-loop

  if (_operatingCxr[0].empty() && _operatingCxr[1].empty() && _marketingCxr[0].empty() &&
      _marketingCxr[1].empty())
  {
    printNoMerchActiveCxrGroup();
    return false;
  }
  return true;
}

bool
OptionalFeeCollector::checkAllSegsUnconfirmed(std::vector<TravelSeg*>::const_iterator begin,
                                              std::vector<TravelSeg*>::const_iterator end) const
{
  std::vector<TravelSeg*>::const_iterator pos = std::find_if(begin, end, IsConfirmed());
  if (pos == end)
  {
    _farePath->itin()->allSegsUnconfirmed() = true;
    return true;
  }
  return false;
}

// overrides for database/external code
bool
OptionalFeeCollector::roundTripJouneyType(const Itin& itin) const
{
  ServiceFeeUtil util(_trx);
  return util.isRoundTripJourneyType(
      itin, itin.validatingCarrier(), _isInternational.getValForKey(&itin));
}

bool
OptionalFeeCollector::internationalJouneyType(const Itin& itin) const
{
  return ServiceFeeUtil::isInternationalJourneyType(itin);
}

bool
OptionalFeeCollector::finalCheckShoppingPricing(Itin& itin)
{
  if (!_stopMatchProcess)
    return true;

  itin.moreFeesAvailable() = true;
  if (!_shopping && !_timeOut)
    printStopAtFirstMatchMsg(); // print stop at first match msg
  return false;
}

void
OptionalFeeCollector::cleanUpOCFees(Itin& itin)
{
  LOG4CXX_DEBUG(logger, "Entering OptionalFeeCollector::cleanUpOCFees()");
  TSELatencyData metrics(_trx, "OC CLEAN UP PROCESS");

  bool isOcFeesFound = false;

  typedef std::vector<ServiceFeesGroup*>::const_iterator SrvFeesGrpI;
  for (SrvFeesGrpI srvFeesGrpI = itin.ocFeesGroup().begin();
       srvFeesGrpI != itin.ocFeesGroup().end();
       ++srvFeesGrpI)
  {
    ServiceFeesGroup* sfG = *srvFeesGrpI;

    LOG4CXX_DEBUG(logger,
                  "OptionalFeeCollector::cleanUpAllOCFees(Itin): processing group "
                      << sfG->groupCode());

    std::map<const FarePath*, std::vector<OCFees*>>::iterator mI;
    for (mI = sfG->ocFeesMap().begin(); mI != sfG->ocFeesMap().end();)
    {
      std::vector<OCFees*>& vecOCFees = mI->second;
      for (std::vector<OCFees*>::iterator ocfee = vecOCFees.begin(); ocfee != vecOCFees.end();)
      {
        if (!(*ocfee)->optFee())
        {
          ocfee = vecOCFees.erase(ocfee);
        }
        else
        {
          updateServiceFeesDisplayOnlyState(*ocfee);
          ++ocfee;
        }
      }
      if (vecOCFees.empty())
      {
        sfG->ocFeesMap().erase(mI++);
      }
      else
      {
        ++mI;
      }
    }

    if (!sfG->ocFeesMap().empty())
    {
      isOcFeesFound = true;
    }

    updateServiceFeesGroupState(sfG);
  }

  itin.setOcFeesFound(isOcFeesFound);
  LOG4CXX_DEBUG(logger,
                "Leaving OptionalFeeCollector::cleanUpOCFees(): - OCFees "
                    << (isOcFeesFound ? "found" : "NOT found"));
}

void
OptionalFeeCollector::updateServiceFeesDisplayOnlyState(OCFees* ocFee)
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " - entering");
  std::map<const CarrierCode, std::vector<ServiceGroup*>>::const_iterator dispOnlyGrpCxrI =
      std::find_if(_cXrDispOnlyGrp.begin(), _cXrDispOnlyGrp.end(), IsSameOCFeeInCxrMap(*ocFee));
  if (dispOnlyGrpCxrI != _cXrDispOnlyGrp.end())
  {
    ocFee->setDisplayOnly(true);
  }
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " - exiting");
}

void
OptionalFeeCollector::addInvalidGroups(Itin& itin,
                                       const std::vector<ServiceGroup>& grNA,
                                       const std::vector<ServiceGroup>& grNP,
                                       const std::vector<ServiceGroup>& grNV)
{
  if (!_shopping && (!grNA.empty() || !grNP.empty()))
  {
    addNotAvailableGroups(itin, grNA, ServiceFeesGroup::NOT_AVAILABLE);
    addNotAvailableGroups(itin, grNP, ServiceFeesGroup::NOT_AVAILABLE);
  }
}

void
OptionalFeeCollector::addNotAvailableGroups(Itin& itin,
                                            const std::vector<ServiceGroup>& grNAP,
                                            ServiceFeesGroup::StateCode state)
{
  for (const auto elem : grNAP)
  {
    ServiceFeesGroup* srvFeesGroup;
    _trx.dataHandle().get(srvFeesGroup);
    srvFeesGroup->groupCode() = elem;
    srvFeesGroup->state() = state;
    itin.ocFeesGroup().push_back(srvFeesGroup);
  }
}

void
OptionalFeeCollector::printNoMerchActiveCxrGroup() const
{
  if (diag875() == nullptr)
    return;

  diag875()->displayNoActiveCxrGroupForOC();
}

void
OptionalFeeCollector::printActiveCxrGroupHeader() const
{
  if (diag875() == nullptr)
    return;

  diag875()->displayActiveCxrGroupHeader();
}

void
OptionalFeeCollector::printActiveMrktDrivenCxrHeader() const
{
  if (diag875() == nullptr)
    return;

  diag875()->displayMrktDrivenCxrHeader();
}

void
OptionalFeeCollector::printActiveMrktDrivenNCxrNoData() const
{
  if (diag875() == nullptr)
    return;

  diag875()->displayMrktDrivenCxrNoData();
}

void
OptionalFeeCollector::printActiveMrktDrivenCxrData(
    std::vector<MerchCarrierPreferenceInfo*>& mCxrPrefVec) const
{
  if (diag875() == nullptr)
    return;

  diag875()->displayMrktDrivenCxrData(mCxrPrefVec);
}

void
OptionalFeeCollector::printJourneyDestination()
{
  if (diag875() == nullptr)
    return;

  LocCode city = EMPTY_STRING();
  if (_journeyDestination)
  {
    city = _journeyDestination->city();
    if (city.empty())
      city = _journeyDestination->loc();
  }
  diag875()->printJourneyDestination(city);
}

void
OptionalFeeCollector::printGroupCodesRequested(const std::vector<ServiceGroup>& input,
                                               const std::vector<ServiceGroup>& vecInv,
                                               const std::vector<ServiceGroup>& vecInvTkt)
{
  if (diag875() == nullptr)
    return;
  diag875()->printGroupCodesInTheRequest(
      _trx, input, vecInv, vecInvTkt, _isRoundTripOCF, _isInternationalOCF, _needFirstMatchOnlyOCF);
}

void
OptionalFeeCollector::printNoGroupCodeProvided()
{
  if (diag875() != nullptr)
    diag875()->printNoGroupCodeProvided();
}

void
OptionalFeeCollector::printNoOptionsRequested()
{
  if (diag875() != nullptr)
    diag875()->printNoOptionsRequested();
}

void
OptionalFeeCollector::printPccIsDeactivated(const PseudoCityCode& pcc)
{
  if (diag875() != nullptr)
    diag875()->printPccIsDeactivated(pcc);
}

void
OptionalFeeCollector::printCanNotCollect(const StatusS5Validation rc) const
{
  if (diag875() == nullptr)
    return;

  diag875()->printCanNotCollect(rc);
  endDiag();
}

void
OptionalFeeCollector::printStopAtFirstMatchMsg() const
{
  if (diag875() == nullptr)
    return;

  diag875()->printStopAtFirstMatchMsg();
  endDiag();
}

void
OptionalFeeCollector::endDiag() const
{
  if (diag875())
  {
    diag875()->lineSkip(1);
    diag875()->flushMsg();
  }
  if (diag877())
  {
    diag877()->lineSkip(1);
    diag877()->flushMsg();
  }
}

void
OptionalFeeCollector::collectExcludedSFG(std::vector<ServiceGroup>& excludedsfG,
                                         const std::vector<ServiceGroup>& grNV,
                                         const std::vector<ServiceGroup>& grNA)
{
  if (!grNV.empty())
  {
    for (const auto elem : grNV)
    {
      excludedsfG.push_back(elem);
    }
  }
  if (!grNA.empty())
  {
    for (const auto elem : grNA)
    {
      excludedsfG.push_back(elem);
    }
  }
}

void
OptionalFeeCollector::cleanUpGroupCode(ServiceFeesGroup& sfG)
{
  std::map<const FarePath*, std::vector<OCFees*>>::iterator mI;
  for (mI = sfG.ocFeesMap().begin(); mI != sfG.ocFeesMap().end();)
  {
    std::vector<OCFees*>& vecOCFees = mI->second;
    for (std::vector<OCFees*>::iterator ocfee = vecOCFees.begin(); ocfee != vecOCFees.end();)
      vecOCFees.erase(ocfee);
    sfG.ocFeesMap().erase(mI++);
  }
  LOG4CXX_DEBUG(logger,
                "Leaving OptionalFeeCollector::cleanUpGroupCode(): for " << sfG.groupCode()
                                                                         << ", time out");
}

void
OptionalFeeCollector::timeOutPostProcessing(Itin& itin,
                                            const std::vector<ServiceGroup>& grNV,
                                            const std::vector<ServiceGroup>& grNA)
{
  TSELatencyData metrics(_trx, "OC TIMEOUT POST PROCESS");
  LOG4CXX_DEBUG(logger, "Entering OptionalFeeCollector::timeOutPostProcessing():");
  typedef std::vector<ServiceFeesGroup*>::const_iterator SrvFeesGrpI;

  if (_trx.getOptions()->serviceGroupsVec().empty())
  {
    cleanUpAllOCFees(itin);
    if (_trx.getOptions()->isProcessAllGroups())
      itin.timeOutForExceeded() = true;
    else
      itin.timeOutOCFForWP() = true;

    LOG4CXX_DEBUG(
        logger,
        "Leaving OptionalFeeCollector::timeOutPostProcessing(): time out - no fees returned");
    return;
  }
  else // WPAE-XX
  {
    std::vector<ServiceGroup> groupCodes;
    _trx.getOptions()->getGroupCodes(_trx.getOptions()->serviceGroupsVec(), groupCodes);

    std::vector<ServiceGroup> excludedSfG;
    if (!grNV.empty() || !grNA.empty())
      collectExcludedSFG(excludedSfG, grNV, grNA);

    // clean up all other group codes except requested
    for (SrvFeesGrpI srvFeesGrpI = itin.ocFeesGroup().begin();
         srvFeesGrpI != itin.ocFeesGroup().end();
         ++srvFeesGrpI)
    {
      ServiceFeesGroup* sfG = *srvFeesGrpI;
      // check if group code is in request
      if (find(groupCodes.begin(), groupCodes.end(), sfG->groupCode()) == groupCodes.end())
      {
        // group code is not in the request - do clean up
        cleanUpGroupCode(*sfG);
      }
      else // group code is in the request
      {
        if (!excludedSfG.empty() &&
            find(excludedSfG.begin(), excludedSfG.end(), sfG->groupCode()) != excludedSfG.end())
          continue;

        if (sfG->ocFeesMap().empty() || // OC was not even try to be processed
            !sfG->sliceDicePassed()) // or slice & dice was not completed
        {
          // clean up all and out
          cleanUpAllOCFees(itin);
          itin.timeOutForExceeded() = true;
          LOG4CXX_DEBUG(logger,
                        "Leaving OptionalFeeCollector::timeOutPostProcessing(): time out "
                        "- no requested fees returned");
          return;
        }
        else // check if OC is processed for all largest portions
        {
          if (!isOCFeesProcessedForLargestPortions(itin, *sfG))
            return;
        }
      }
    }
    if (!itin.timeOutForExceededSFGpresent())
      itin.timeOutForExceeded() = true;
    cleanUpOCFees(itin);
    return;
  }
}

void
OptionalFeeCollector::createPseudoPricingSolution()
{
  PseudoFarePathBuilder builder(_trx);
  builder.build();
}

bool
OptionalFeeCollector::isOCFeesProcessedForLargestPortions(Itin& itin, ServiceFeesGroup& sfG)
{
  int unitNo = 0;
  bool out = false;
  bool in = false;
  for (; unitNo < 2; unitNo++) // Loop to process portions of travel
  {
    std::vector<TravelSeg*>::const_iterator firstSeg = _beginsOfUOT[unitNo];
    std::vector<TravelSeg*>::const_iterator endSeg = _beginsOfUOT[unitNo + 1];

    if (checkLargestPortion(firstSeg, endSeg, sfG))
    {
      if (unitNo == 0)
        out = true;
      else
        in = true;
    }
    if (endSeg == _farePath->itin()->travelSeg().end())
      break;
  }
  if ((unitNo == 0 && !out) || (unitNo == 1 && (!out || !in)))
  {
    cleanUpAllOCFees(itin);
    itin.timeOutForExceeded() = true;
    LOG4CXX_DEBUG(
        logger,
        "Leaving SliceAndDice::isOCFeesProcessedForLargestPortions(): time out - all cleared");
    return false;
  }
  itin.timeOutForExceededSFGpresent() = true;
  LOG4CXX_DEBUG(logger,
                "Leaving SliceAndDice::isOCFeesProcessedForLargestPortions(): time out - "
                "requested completely processed");
  return true;
}

bool
OptionalFeeCollector::checkLargestPortion(std::vector<TravelSeg*>::const_iterator firstSeg,
                                          std::vector<TravelSeg*>::const_iterator endSeg,
                                          ServiceFeesGroup& sfG) const
{
  for (; firstSeg != endSeg; ++firstSeg)
  {
    TravelSeg* tvl = *firstSeg;
    if (checkOCFeeMap(tvl, sfG))
      return true;
  }
  return false;
}

bool
OptionalFeeCollector::checkOCFeeMap(const TravelSeg* tvl, ServiceFeesGroup& sfG) const
{
  const boost::lock_guard<boost::mutex> guard(sfG.mutex());
  std::map<const FarePath*, std::vector<OCFees*>>::iterator mI;
  for (mI = sfG.ocFeesMap().begin(); mI != sfG.ocFeesMap().end(); ++mI)
  {
    std::vector<OCFees*>& vecOCFees = mI->second;
    if (isTravelSegFound(tvl, vecOCFees))
      return true;
  }
  return false;
}

bool
OptionalFeeCollector::isTravelSegFound(const TravelSeg* tvl, std::vector<OCFees*>& vecOCFees) const
{
  for (const auto ocfee : vecOCFees)
  {
    if (tvl == ocfee->travelStart() || tvl == ocfee->travelEnd())
    {
      return true;
    }
  }
  return false;
}

void
OptionalFeeCollector::cleanUpAllOCFees(Itin& itin)
{
  LOG4CXX_DEBUG(logger, "OptionalFeeCollector::cleanUpAllOCFees(Itin)");

  bool isOcFeesFound = false;

  typedef std::vector<ServiceFeesGroup*>::const_iterator SrvFeesGrpI;
  for (SrvFeesGrpI srvFeesGrpI = itin.ocFeesGroup().begin();
       srvFeesGrpI != itin.ocFeesGroup().end();
       ++srvFeesGrpI)
  {
    ServiceFeesGroup* sfG = *srvFeesGrpI;
    LOG4CXX_DEBUG(logger,
                  "OptionalFeeCollector::cleanUpAllOCFees(Itin): cleaning group "
                      << sfG->groupCode());
    std::map<const FarePath*, std::vector<OCFees*>>::iterator mI;
    {
      const boost::lock_guard<boost::mutex> guard(sfG->mutex());
      for (mI = sfG->ocFeesMap().begin(); mI != sfG->ocFeesMap().end();)
      {
        std::vector<OCFees*>& vecOCFees = mI->second;
        for (std::vector<OCFees*>::iterator ocfee = vecOCFees.begin(); ocfee != vecOCFees.end();)
          vecOCFees.erase(ocfee);
        sfG->ocFeesMap().erase(mI++);
      }
    }

    if (!sfG->ocFeesMap().empty())
    {
      isOcFeesFound = true;
    }

    updateServiceFeesGroupState(sfG);
  }

  itin.setOcFeesFound(isOcFeesFound);
  LOG4CXX_DEBUG(logger,
                "Leaving OptionalFeeCollector::cleanUpAllOCFees(): - OCFees "
                    << (isOcFeesFound ? "found" : "NOT found"));
}

void
OptionalFeeCollector::updateServiceFeesGroupState(ServiceFeesGroup* sfG)
{
  const boost::lock_guard<boost::mutex> guard(sfG->mutex());
  if (sfG->ocFeesMap().empty())
  {
    std::map<const CarrierCode, std::vector<ServiceGroup*>>::const_iterator srvGrpCxrI =
        std::find_if(_cXrGrp.begin(), _cXrGrp.end(), IsSameSrvGroupInCxrMap(sfG->groupCode()));

    if (srvGrpCxrI != _cXrGrp.end())
    {
      LOG4CXX_DEBUG(logger,
                    "OptionalFeeCollector::updateServiceFeesGroupState(ServiceFeesGroup*) - "
                        << "Setting state of ServiceGroup \"" << sfG->groupCode()
                        << "\": " << sfG->stateStr() << "=>EMPTY");
      sfG->state() = ServiceFeesGroup::EMPTY;
    }
    else
    {
      LOG4CXX_DEBUG(logger,
                    "OptionalFeeCollector::updateServiceFeesGroupState(ServiceFeesGroup*) - "
                        << "Setting state of ServiceGroup \"" << sfG->groupCode()
                        << "\": " << sfG->stateStr() << "=>NOT_AVAILABLE");
      sfG->state() = ServiceFeesGroup::NOT_AVAILABLE;
    }
  }
}
}
