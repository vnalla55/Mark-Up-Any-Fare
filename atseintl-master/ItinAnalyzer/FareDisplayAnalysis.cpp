#include "ItinAnalyzer/FareDisplayAnalysis.h"

#include "Common/Logger.h"
#include "Common/NonFatalErrorResponseException.h"
#include "Common/PaxTypeUtil.h"
#include "DataModel/FareDisplayTrx.h"
#include "DBAccess/DataHandle.h"
#include "FareDisplay/InclusionCodeConsts.h"
#include "ItinAnalyzer/FDPrefRetriever.h"
#include "ItinAnalyzer/InclusionCodePaxType.h"
#include "ItinAnalyzer/InclusionCodeRetriever.h"

namespace tse
{
FALLBACK_DECL(fallbackFareDisplayByCabinActivation);

static Logger
logger("atseintl.ItinAnalyzerService.FareDisplayAnalysis");

FareDisplayAnalysis::FareDisplayAnalysis(FareDisplayTrx& trx)
  : _trx(trx),
    _request(*trx.getRequest()),
    _options(*trx.getOptions()),
    _dataHandle(trx.dataHandle())
{
}

void
FareDisplayAnalysis::process()
{
  FDPrefRetriever fdPref(_trx);
  fdPref.retrieve();
  if (_trx.getOptions()->fareDisplayPref() == nullptr)
  {
    LOG4CXX_ERROR(logger, "NO USER PREFERENCE RECORD FOUND");
    _trx.errorResponse() << "NO USER PREFERENCE FOUND FOR DISPLAY" << std::endl;
    throw ErrorResponseException(ErrorResponseException::NO_USER_PREFERENCE_FOUND,
                                 _trx.errorResponse().str().c_str());
  }

  LOG4CXX_INFO(logger, " Retrieving Inclusioin Code Record and Passenger Types");
  checkWebInclCd();
  setInclusionCode(_request, _options);
  LOG4CXX_INFO(logger, "PROCESSING INCLUISON CODE  : " << _request.inclusionCode());
  setFamilyPlanDefaults(_request, _options);
  populatePassengerTypes();
  initializePaxType();
}

void
FareDisplayAnalysis::setFamilyPlanDefaults(const FareDisplayRequest& _request,
                                           FareDisplayOptions& _options)
{
  bool hasQualifier(_options.isAdultFares() || _options.isChildFares() || _options.isInfantFares());

  if (!hasQualifier && _request.inclusionCode() == FAMILY_PLAN_FARES)
  {
    LOG4CXX_INFO(logger, "SETTING A-C-I to TRUE because of Family Plan Request ");
    _options.adultFares() = 'Y';
    _options.childFares() = 'Y';
    _options.infantFares() = 'Y';
  }
}
bool
FareDisplayAnalysis::isQualifierPresent(const FareDisplayRequest& _request,
                                        const FareDisplayOptions& _options)
{

  if (!_request.displayPassengerTypes().empty() || !_request.fareBasisCode().empty() ||
      !_request.bookingCode().empty() || !_request.accountCode().empty() ||
      !_request.corporateID().empty() || !_request.ticketDesignator().empty() ||
      _options.isPrivateFares() || _options.isPublicFares() || _options.isUniqueAccountCode() ||
      _options.isUniqueCorporateID())
  {
    return true;
  }
  return false;
}

bool
FareDisplayAnalysis::setInclusionCode(FareDisplayRequest& _request, FareDisplayOptions& _options)
{
  if (_request.inclusionCode().empty() && !isQualifierPresent(_request, _options))
  {
    _request.inclusionCode() = NORMAL_AND_EXCURSION_FARES;
    return true;
  }

  if (_request.inclusionCode().empty())
  {
    if (!_request.displayPassengerTypes().empty())
    {
      _request.inclusionCode() = ALL_FARES;
    }

    if (!_request.bookingCode().empty())
    {
      if (_request.inclusionCode().empty())
        _request.inclusionCode() = NORMAL_AND_EXCURSION_FARES;
    }

    if (!_request.fareBasisCode().empty())
    {
      if (_request.inclusionCode().empty())
      {
        _request.inclusionCode() = ALL_FARES;
      }
    }

    if (!_request.ticketDesignator().empty())
    {
      if (_request.inclusionCode().empty())
      {
        _request.inclusionCode() = ALL_FARES;
      }
    }

    if (!_request.fareBasisCode().empty() && !_request.ticketDesignator().empty())
    {
      if (_request.inclusionCode().empty())
      {
        _request.inclusionCode() = ALL_FARES;
      }
    }

    if (_options.isUniqueAccountCode() || _options.isUniqueCorporateID())
    {
      if (!_request.displayPassengerTypes().empty())
        _request.inclusionCode() = ALL_FARES;
      else if (_request.inclusionCode().empty())
      {
        _request.inclusionCode() = NORMAL_AND_EXCURSION_FARES;
      }
    }

    if (_request.inclusionCode().empty())
      _request.inclusionCode() = NORMAL_AND_EXCURSION_FARES;
  }
  return !_request.inclusionCode().empty();
}

void
FareDisplayAnalysis::initializePaxType()
{

  if (!_request.passengerTypes().empty())
  {
    PaxTypeUtil::initialize(_trx);
  }
  else
  {
    // error condition, this should never happen

    LOG4CXX_ERROR(logger,
                  "Can Not Find any Pax Type For the Requested Inclusion Code : "
                      << _request.inclusionCode());

    PaxType* paxType;
    _dataHandle.get(paxType); // lint !e530

    paxType->paxType() = ADULT;
    paxType->number() = 1;
    paxType->age() = 0;
    paxType->stateCode() = "";
    paxType->inputOrder() = 1;

    PaxTypeUtil::initialize(_trx,
                            *paxType,
                            paxType->paxType(),
                            paxType->number(),
                            paxType->age(),
                            paxType->stateCode(),
                            paxType->inputOrder());
    _trx.paxType().push_back(paxType);
  }
}

void
FareDisplayAnalysis::populatePassengerTypes()
{
  if(fallback::fallbackFareDisplayByCabinActivation(&_trx) ||
     !_request.multiInclusionCodes())
  {
    InclusionCodePaxType* inclPaxType = InclusionCodePaxType::getInclusionCodePaxType(_trx);

    if (inclPaxType != nullptr)
    {
      LOG4CXX_DEBUG(logger, "Invoking getPaxType For   : " << _request.inclusionCode());
      inclPaxType->getPaxType(_trx);
    }
    else
    {
      LOG4CXX_ERROR(
          logger,
         "Can Not Find Valid Inclusion Code Pax Type  For   : " << _request.inclusionCode());
    }
  }
  else // code to handle multiple cabin inclusion codes
  {
    InclusionCode saveInclCodes = _request.inclusionCode();
    uint8_t numberOfInclCodes = _request.inclusionCode().size() / 2;
    for (uint8_t position = 0; position < numberOfInclCodes; ++position)
    {
      _request.inclusionCode() = saveInclCodes.substr(position*2, 2);

      InclusionCodePaxType* inclPaxType = InclusionCodePaxType::getInclusionCodePaxType(_trx);

      if (inclPaxType != nullptr)
      {
        LOG4CXX_DEBUG(logger, "Invoking getPaxType For   : " << _request.inclusionCode());
        inclPaxType->getPaxType(_trx);
      }
      else
      {
        LOG4CXX_ERROR(
            logger,
           "Can Not Find Valid Inclusion Code Pax Type  For   : " << _request.inclusionCode());
      }
    }
    _request.inclusionCode() = saveInclCodes;
  }
}
void
FareDisplayAnalysis::setFareMarketCat19Flags(FareDisplayTrx& trx, FareMarket* fareMarket)
{

  // If no specific input from user to indicate cat 19 processing, do not bypass set flags in
  // FareMarket::initialize
  if (trx.getRequest()->displayPassengerTypes().empty() &&
      trx.getRequest()->fareBasisCode().empty() && trx.getRequest()->ticketDesignator().empty() &&
      !trx.getOptions()->isChildFares() && !trx.getOptions()->isInfantFares())
  {
    fareMarket->setBypassCat19FlagsSet(false);
    return;
  }
  else
  {
    fareMarket->setBypassCat19FlagsSet(true);
  }

  // Any child or infant qualifier present?
  if (trx.getOptions()->isChildFares())
    fareMarket->setChildNeeded(true);
  if (trx.getOptions()->isInfantFares())
    fareMarket->setInfantNeeded(true);
  if (fareMarket->isChildNeeded() && fareMarket->isInfantNeeded())
  {
    if (trx.getRequest()->inputPassengerTypes().empty())
      return;
  }

  // Any child or infant passenger types requested?
  // iterate over the displayPassengerTypes vector
  if (!trx.getRequest()->displayPassengerTypes().empty() ||
      !trx.getRequest()->inputPassengerTypes().empty())
  {
    std::vector<PaxTypeCode>::iterator iter = trx.getRequest()->displayPassengerTypes().begin();
    std::vector<PaxTypeCode>::iterator iterEnd = trx.getRequest()->displayPassengerTypes().end();
    if (trx.getRequest()->displayPassengerTypes().empty())
    {
      iter = trx.getRequest()->inputPassengerTypes().begin();
      iterEnd = trx.getRequest()->inputPassengerTypes().end();
    }
    fareMarket->setChildNeeded(false);
    fareMarket->setInfantNeeded(false);
    for (; iter != iterEnd; iter++)
    {
      if (!fareMarket->isChildNeeded())
        fareMarket->setChildNeeded(PaxTypeUtil::isChild(*iter, Vendor::ATPCO) ||
                                   PaxTypeUtil::isChildFD(*iter, Vendor::SITA));
      if (!fareMarket->isInfantNeeded())
        fareMarket->setInfantNeeded(PaxTypeUtil::isInfant(*iter, Vendor::ATPCO) ||
                                    PaxTypeUtil::isInfantFD(*iter, Vendor::SITA));

      if (fareMarket->isChildNeeded() && fareMarket->isInfantNeeded())
      {
        return;
      }
    }
    return;
  }

  // Any child or infant in the farebasis code or ticket designator

  if (trx.getRequest()->ticketDesignator().empty() && trx.getRequest()->fareBasisCode().empty())
  {
    return;
  }

  std::string tktDesignator = trx.getRequest()->ticketDesignator().c_str();
  std::string fareBCode = trx.getRequest()->fareBasisCode().c_str();

  // check if ticket designator was appended to farebasiscode and if so break them up
  if (!fareBCode.empty())
  {
    std::string::size_type loc = fareBCode.find('/');
    if (loc != std::string::npos)
    {
      tktDesignator.assign(fareBCode, loc + 1, fareBCode.size() - loc - 1);
      fareBCode.assign(trx.getRequest()->fareBasisCode().c_str(), 0, loc);
    }
  }

  if (!fareMarket->isChildNeeded())
  {
    fareMarket->setChildNeeded(matchChInAppend(fareBCode, true) ||
                               matchChInAppend(tktDesignator, true));
  }
  if (!fareMarket->isInfantNeeded())
  {
    fareMarket->setInfantNeeded(matchChInAppend(fareBCode, false) ||
                                matchChInAppend(tktDesignator, false));
  }
}

// pass in fareclass part or tkt designator part
// isChild == true,  return true for strings ending with CH, CHx CHxx
// isChild == false, return true for strings ending with IN, INx or INxx
// (where xx is two digits)
// return false otherwise
// note that some child/infant fares may not have this suffix

bool
FareDisplayAnalysis::matchChInAppend(std::string& partialFareBasis, bool isChild)
{
  if (partialFareBasis.empty())
    return false;

  std::string::reverse_iterator i = partialFareBasis.rbegin();
  std::string::reverse_iterator iend = partialFareBasis.rend();

  // only need to strip 2 digits of percentage (optional format)
  if (i != iend && isdigit(*i))
  {
    i++;
    if (i != iend && isdigit(*i))
      i++;
  }
  // get the substring without digits
  std::string partialFareBasisWithoutDigits = partialFareBasis.substr(0, std::distance(i, iend));

  if (isChild)
  {
    // check for child
    return (endsWith(partialFareBasisWithoutDigits, "CH") ||
            endsWith(partialFareBasisWithoutDigits, "CNE"));
  }

  else
  {
    // check for infant
    return (endsWith(partialFareBasisWithoutDigits, "IN") ||
            endsWith(partialFareBasisWithoutDigits, "INE"));
  } // endif - Child or Infant
}
bool
FareDisplayAnalysis::endsWith(const std::string& str, const std::string& end)
{

  if (str.size() < end.size())
    return false;
  std::string::size_type pos = str.rfind(end);
  if (pos == std::string::npos)
    return false;
  return (pos == str.size() - end.size());
}

void
FareDisplayAnalysis::checkWebInclCd()
{
  if (_request.inclusionCode() != TRAVELOCITY_INCL)
    return;

  if (!_request.displayPassengerTypes().empty())
  {
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_WEB_AND_PAXTYPE);
  }

  std::set<CarrierCode> carriers;
  if (_trx.getOptions()->isAllCarriers())
  {
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_WEB_MULTICARRIER);
  }
  else
  {
    carriers = _trx.preferredCarriers();
  }
  if (carriers.empty())
  {
    throw NonFatalErrorResponseException(ErrorResponseException::NEED_PREFERRED_CARRIER);
  }
  if (carriers.size() > 1 || (*carriers.begin()) == ANY_CARRIER)
  {
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_WEB_MULTICARRIER);
  }
}
}
