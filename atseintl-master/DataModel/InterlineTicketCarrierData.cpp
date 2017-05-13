//-------------------------------------------------------------------
//
//  File:       InterlineTicketCarrierData.cpp
//  Created:    October 19, 2010
//
//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DataModel/InterlineTicketCarrierData.h"

#include "Common/Assert.h"
#include "Common/FareCalcUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ShoppingTrx.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/InterlineTicketCarrierInfo.h"
#include "DBAccess/InterlineTicketCarrierStatus.h"

#include <algorithm>

namespace tse
{

Logger
interlineTicketCarrierLogger("atseintl.PricingOrchestrator");

////////////////////////////////////////////////////////////////////////////////////////////////////

InterlineTicketCarrier::ValidateStatus::ValidateStatus(bool collectAgreements)
  : _collectAgreements(collectAgreements),
    _status(OK),
    _validatingCarrierParticipate(false),
    _normalRelationship(false),
    _superPseudoInterlineRelationship(false),
    _profileStatus(PROFILE_OK)
{
}

void
InterlineTicketCarrier::ValidateStatus::setNoAgreement(const CarrierCode& carrier)
{
  _noAgreementCarrier = carrier;
  _status = NO_AGREEMENT;
}

void
InterlineTicketCarrier::ValidateStatus::setNoRelationshipAgreement()
{
  _status = NO_RELATIONSHIP_AGREEMENT;
}

void
InterlineTicketCarrier::ValidateStatus::setEmptySegments()
{
  _status = EMPTY_SEGMENTS;
}

const CarrierCode&
InterlineTicketCarrier::ValidateStatus::noAgreementCarrier() const
{
  TSE_ASSERT(_status == NO_AGREEMENT);
  return _noAgreementCarrier;
}

void
InterlineTicketCarrier::ValidateStatus::addAgreement(
    const InterlineTicketCarrierInfo& interlineCarrier)
{
  if (UNLIKELY(_collectAgreements))
  {
    if (std::find(_agreements.begin(), _agreements.end(), &interlineCarrier) == _agreements.end())
    {
      _agreements.push_back(&interlineCarrier);
    }
  }
}

void
InterlineTicketCarrier::ValidateStatus::setProfileStatus(const CrsCode& agentCode,
                                                         ProfileStatus profileStatus)
{
  _agentCode = agentCode;
  _profileStatus = profileStatus;
  if (_profileStatus != PROFILE_OK)
  {
    _status = NO_VALID_PROFILE;
  }
}

std::string
InterlineTicketCarrier::ValidateStatus::toString() const
{
  std::string result;
  switch (_status)
  {
  case OK:
    result = "OK";
    break;

  case NO_AGREEMENT:
    result = "NO INTERLINE TICKETING AGREEMENT WITH " + _noAgreementCarrier;
    break;

  case NO_RELATIONSHIP_AGREEMENT:
    result = "NO VALID TICKETING AGREEMENT";
    break;

  case NO_VALID_PROFILE:
    result = profileStatusToString();
    break;

  case EMPTY_SEGMENTS:
    result = "EMPTY SEGMENTS";
    break;
  }

  return result;
}

std::string
InterlineTicketCarrier::ValidateStatus::profileStatusToString() const
{
  std::string result;
  switch (_profileStatus)
  {
  case PROFILE_OK:
    result = "PROFILE OK";
    break;

  case PROFILE_NOT_AVAILABLE:
    result = "IET PROFILE NOT AVAILABLE";
    break;

  case PROFILE_DEACTIVATED:
    result = "CRS " + _agentCode + " - DEACTIVATED";
    break;

  case PROFILE_NOT_INITIALIZED:
    result = "CRS " + _agentCode + " - NOT INITIALIZED";
    break;

  case PROFILE_BETA:
    result = "CRS " + _agentCode + " - BETA";
    break;
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

const InterlineTicketCarrier::CarrierInfoMap&
InterlineTicketCarrier::getInterlineCarriers(const PricingTrx& trx,
                                             const CarrierCode& validatingCarrier)
{
  InterlineTicketCarrierMap::const_iterator it = _interlineTicketCarrierMap.find(validatingCarrier);
  if (it != _interlineTicketCarrierMap.end())
  {
    return it->second;
  }
  else
  {
    return loadInterlineTicketCarrierDataFromDB(trx, validatingCarrier);
  }
}

const InterlineTicketCarrier::CarrierInfoMap&
InterlineTicketCarrier::loadInterlineTicketCarrierDataFromDB(const PricingTrx& trx,
                                                             const CarrierCode& carrier)
{
  const std::vector<InterlineTicketCarrierInfo*>& interlineInfoVec =
      trx.dataHandle().getInterlineTicketCarrier(carrier, DateTime::localTime());

  CarrierInfoMap& interlineCarriers = _interlineTicketCarrierMap[carrier];

  std::vector<InterlineTicketCarrierInfo*>::const_iterator iter = interlineInfoVec.begin();
  std::vector<InterlineTicketCarrierInfo*>::const_iterator iterEnd = interlineInfoVec.end();
  for (; iter != iterEnd; ++iter)
  {
    const InterlineTicketCarrierInfo& carrierInfo = **iter;
    interlineCarriers.insert(std::make_pair(carrierInfo.interlineCarrier(), &carrierInfo));
  }

  return interlineCarriers;
}

std::string
InterlineTicketCarrier::getInterlineCarriersForValidatingCarrier(const PricingTrx& trx,
                                                                 const CarrierCode& carrier)
{
  if (isCarrierAgreementDataPrinted(carrier))
  {
    return std::string();
  }

  std::string intCarriers;
  intCarriers.reserve(64);
  intCarriers.append("\n INTERLINE CARRIERS FOR VALIDATING CARRIER ");
  intCarriers.append(carrier);
  intCarriers.append(":\n   ");

  const CarrierInfoMap& interlineCarriers = getInterlineCarriers(trx, carrier);

  for (CarrierInfoMap::const_iterator infIt = interlineCarriers.begin();
       infIt != interlineCarriers.end();
       infIt++)
  {
    intCarriers.append(infIt->first);

    const InterlineTicketCarrierInfo& interlineCarrier = *infIt->second;
    intCarriers.append(":");
    if (interlineCarrier.superPseudoInterline() == 'Y')
    {
      intCarriers.append("S");
    }
    else if (interlineCarrier.pseudoInterline() == 'Y')
    {
      intCarriers.append("P");
    }
    else if (interlineCarrier.hostInterline() == 'Y')
    {
      intCarriers.append("H");
    }
    else
    {
      intCarriers.append("N");
    }

    intCarriers.append(" ");
  }

  intCarriers.append("\n");

  intCarriers.append(" S - SUPER PSEUDO INTERLINE AGREEMENT\n"
                     " P - PSEUDO INTERLINE AGREEMENT\n"
                     " H - HOSTED INTERLINE AGREEMENT\n"
                     " N - NORMAL INTERLINE AGREEMENT\n");

  _interlineTicketCarrierPrinted.insert(carrier);

  return intCarriers;
}

void
InterlineTicketCarrier::validateInterlineTicketCarrierAgreement(
    const PricingTrx& trx,
    const CarrierCode& validatingCarrier,
    const std::vector<TravelSeg*>& travelSegments,
    ValidateStatus& validateStatus)
{
  if (UNLIKELY(travelSegments.empty()))
  {
    LOG4CXX_DEBUG(interlineTicketCarrierLogger, "Travel segment is empty.");
    validateStatus.setEmptySegments();
    return;
  }

  const CarrierInfoMap& interlineCarriers = getInterlineCarriers(trx, validatingCarrier);

  for (std::vector<TravelSeg*>::const_iterator travelSegIt = travelSegments.begin();
       travelSegIt != travelSegments.end() && validateStatus.isOk();
       ++travelSegIt)
  {
    const AirSeg* airSeg = (*travelSegIt)->toAirSeg();
    if (UNLIKELY(!airSeg))
    {
      continue;
    }

    const CarrierCode& marketingCarrier = airSeg->marketingCarrierCode();
    if (validatingCarrier == marketingCarrier)
    {
      validateStatus.setValidatingCarrierParticipate();
    }
    else
    {
      processAgreement(interlineCarriers, marketingCarrier, validateStatus);
    }

    const CarrierCode& operatingCarrier = airSeg->operatingCarrierCode();
    if (validateStatus.isOk() && !operatingCarrier.empty() && operatingCarrier != marketingCarrier)
    {
      if (validatingCarrier == operatingCarrier)
      {
        validateStatus.setValidatingCarrierParticipate();
      }
      else
      {
        processAgreement(interlineCarriers, operatingCarrier, validateStatus);
      }
    }
  }

  if (validateStatus.isOk())
  {
    if (LIKELY(validateStatus.validatingCarrierParticipate() ||
        validateStatus.superPseudoInterlineRelationship()))
    {
      return;
    }
    else if (validateStatus.normalRelationship())
    {
      validateStatus.setNoRelationshipAgreement();
    }
  }
}

bool
InterlineTicketCarrier::validateInterlineTicketCarrierAgreement(
    const PricingTrx& trx,
    const CarrierCode& validatingCarrier,
    const std::vector<TravelSeg*>& travelSegments,
    std::string* validationMessage)
{
  ValidateStatus validateStatus(false);
  validateInterlineTicketCarrierAgreement(trx, validatingCarrier, travelSegments, validateStatus);

  if (!validateStatus.isOk() && validationMessage)
  {
    *validationMessage = validateStatus.toString();
  }

  return validateStatus.isOk();
}

bool
InterlineTicketCarrier::validateAgreementBetweenValidatingAndInterlineCarrier(
    const PricingTrx& trx,
    const CarrierCode& validatingCarrier,
    const CarrierCode& interlineCarrier)
{
  if (validatingCarrier == interlineCarrier)
  {
    return true;
  }

  const CarrierInfoMap& interlineCarriers = getInterlineCarriers(trx, validatingCarrier);
  return interlineCarriers.find(interlineCarrier) != interlineCarriers.end();
}

void
InterlineTicketCarrier::validateInterlineTicketCarrierStatus(const PricingTrx& trx,
                                                             const CarrierCode& validatingCarrier,
                                                             const CrsCode& agentCode,
                                                             ValidateStatus& validateStatus)
{
  const InterlineTicketCarrierStatus* interlineTicketCarrierStatus =
      getInterlineTicketCarrierStatus(trx, validatingCarrier, agentCode);

  if (!interlineTicketCarrierStatus)
  {
    validateStatus.setProfileStatus(agentCode, ValidateStatus::PROFILE_NOT_AVAILABLE);
  }
  else if (interlineTicketCarrierStatus->status() == 'D')
  {
    validateStatus.setProfileStatus(agentCode, ValidateStatus::PROFILE_DEACTIVATED);
  }
  else if (interlineTicketCarrierStatus->status() == 'N')
  {
    validateStatus.setProfileStatus(agentCode, ValidateStatus::PROFILE_NOT_INITIALIZED);
  }
  else if (interlineTicketCarrierStatus->status() == 'B')
  {
    validateStatus.setProfileStatus(agentCode, ValidateStatus::PROFILE_BETA);
  }
  else
  {
    validateStatus.setProfileStatus(agentCode, ValidateStatus::PROFILE_OK);
  }
}

bool
InterlineTicketCarrier::isPriceInterlineActivated(PricingTrx& trx)
{
  if (trx.getRequest() && trx.getRequest()->ticketingAgent() &&
      !trx.getRequest()->ticketingAgent()->tvlAgencyPCC().empty() &&
      !TrxUtil::isExchangeOrTicketing(trx))
  {
    const FareCalcConfig* fareCalcConfig = FareCalcUtil::getFareCalcConfig(trx);
    if (fareCalcConfig)
    {
      return fareCalcConfig->ietPriceInterlineActive() != 'N';
    }
    else
    {
      LOG4CXX_ERROR(interlineTicketCarrierLogger,
                    "Error getting Fare Calc Config for PCC: "
                        << trx.getRequest()->ticketingAgent()->tvlAgencyPCC());
      return false;
    }
  }
  else
  {
    return false;
  }
}

bool
InterlineTicketCarrier::isCarrierAgreementDataPrinted(const CarrierCode& carrier) const
{
  return _interlineTicketCarrierPrinted.find(carrier) != _interlineTicketCarrierPrinted.end();
}

void
InterlineTicketCarrier::processAgreement(const CarrierInfoMap& interlineCarriers,
                                         const CarrierCode& carrier,
                                         ValidateStatus& validateStatus)
{
  CarrierInfoMap::const_iterator interlineCarrierIt = interlineCarriers.find(carrier);
  if (interlineCarrierIt == interlineCarriers.end())
  {
    validateStatus.setNoAgreement(carrier);
  }
  else
  {
    const InterlineTicketCarrierInfo& interlineCarrier = *interlineCarrierIt->second;
    if (interlineCarrier.superPseudoInterline() == 'Y')
    {
      // A super pseudo interline agreement exists between the validating carrier (NN)
      // and one of the carriers in the itinerary (XX). Another carrier in the itinerary (ZZ)
      // has a normal agreement with the validating carrier (NN).
      // The validating carrier is not required to be part of the itinerary.
      validateStatus.setSuperPseudoInterlineRelationship();
    }
    else if (interlineCarrier.pseudoInterline() == 'Y')
    {
      // The validating carrier is not required to be part of the itinerary.
    }
    else if (interlineCarrier.hostInterline() == 'Y')
    {
      // The validating carrier is not required to be part of the itinerary.
    }
    else
    {
      // The validating carrier must be part of the itinerary.
      validateStatus.setNormalRelationship();
    }

    validateStatus.addAgreement(interlineCarrier);
  }
}

const InterlineTicketCarrierStatus*
InterlineTicketCarrier::getInterlineTicketCarrierStatus(const PricingTrx& trx,
                                                        const CarrierCode& validatingCarrier,
                                                        const CrsCode& agentCode)
{
  CarrierStatusMap::const_iterator carrierIt =
      _carrierStatusMap.find(std::make_pair(validatingCarrier, agentCode));
  if (carrierIt == _carrierStatusMap.end())
  {
    const InterlineTicketCarrierStatus* interlineTicketCarrierStatus =
        trx.dataHandle().getInterlineTicketCarrierStatus(validatingCarrier, agentCode);
    const std::pair<CarrierCode, CrsCode> statusKey(validatingCarrier, agentCode);
    carrierIt =
        _carrierStatusMap.insert(std::make_pair(statusKey, interlineTicketCarrierStatus)).first;
  }

  return carrierIt->second;
}

} // namespace tse
