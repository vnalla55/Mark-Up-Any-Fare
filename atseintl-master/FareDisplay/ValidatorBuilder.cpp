//----------------------------------------------------------------------------
//
//  Copyright Sabre 2003
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

#include "FareDisplay/ValidatorBuilder.h"

#include "Common/Logger.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayResponse.h"
#include "DBAccess/FareDisplayInclCd.h"
#include "FareDisplay/ALLPaxTypeValidator.h"
#include "FareDisplay/DisplayTypeValidator.h"
#include "FareDisplay/FareTypeValidator.h"
#include "FareDisplay/InclusionCodeConsts.h"
#include "FareDisplay/PaxTypeValidator.h"
#include "FareDisplay/QualifierPaxTypeValidator.h"
#include "FareDisplay/RequestedPaxTypeValidator.h"
#include "FareDisplay/RuleTariffValidator.h"
#include "FareDisplay/WebICValidator.h"

namespace tse
{
FALLBACK_DECL(fallbackFareDisplayByCabinActivation);

static Logger
logger("atseintl.FareDisplay.ValidatorBuilder");

ValidatorBuilder::~ValidatorBuilder() {}

void
ValidatorBuilder::build()
{
  LOG4CXX_INFO(logger, "Entered ValidatorBuilder::build()");

  if (_trx.getRequest()->inclusionCode() == ALL_FARES)
  {
    ALLPaxTypeValidator* AllValidator(nullptr);
    _trx.dataHandle().get(AllValidator);
    Validator* validator = dynamic_cast<Validator*>(AllValidator);
    add(validator);
  }
  else if (isSpecifiedInclusionCode())
  {
    LOG4CXX_INFO(logger, "Processing Specified Inclusion Code ValidatorBuilder::build()");
    getAllSpecified();
  }
  else if (_trx.getRequest()->inclusionCode() == TRAVELOCITY_INCL)
  {
    LOG4CXX_DEBUG(logger, "Getting WEB includion code Validators");
    getAll();
  }
  else
  {
    LOG4CXX_ERROR(logger, "No Incl Validation Will be Performed--Unknown Inclusion Code Request ");
  }
}

void
ValidatorBuilder::add(Validator*& validator)
{
  if (validator->initialize(_trx))
    _validators.push_back(validator);
}

bool
ValidatorBuilder::isSpecifiedInclusionCode() const
{
  if(fallback::fallbackFareDisplayByCabinActivation(&_trx))
    return _trx.fdResponse()->fareDisplayInclCd() != nullptr;

  return (_trx.getRequest()->multiInclusionCodes() ? !_trx.fdResponse()->fdInclCdPerInclCode().empty() :
                                                     _trx.fdResponse()->fareDisplayInclCd() != nullptr);
}

void
ValidatorBuilder::getAll(const FareDisplayInclCd& inclusionCode)
{
  uint16_t priority(1);
  if (inclusionCode.displTypeAndOrFareType() != ValidatorBuilder::BLANK_IND)
  {
    LOG4CXX_DEBUG(logger, " Processing Inclusion Code for Display Type AND/OR Fare Type");
    _requestedValidators.insert(std::make_pair(priority, Validator::PAX_TYPE_VALIDATOR));
    _requestedValidators.insert(std::make_pair(++priority, Validator::FARE_TYPE_VALIDATOR));
    _requestedValidators.insert(std::make_pair(++priority, Validator::DISPLAY_TYPE_VALIDATOR));
  }
  else if (inclusionCode.fareTypeAndOrPsgType() != ValidatorBuilder::BLANK_IND)
  {
    LOG4CXX_DEBUG(logger, " Processing Inclusion Code for Pax Type AND/OR Fare Type");
    _requestedValidators.insert(std::make_pair(priority, Validator::PAX_TYPE_VALIDATOR));
    _requestedValidators.insert(std::make_pair(++priority, Validator::FARE_TYPE_VALIDATOR));
  }
  else if (inclusionCode.displTypeAndOrPsgType() != ValidatorBuilder::BLANK_IND)
  {
    LOG4CXX_DEBUG(logger, " Processing Inclusion Code for Pax Type AND/OR Display Type");
    _requestedValidators.insert(std::make_pair(priority, Validator::PAX_TYPE_VALIDATOR));
    _requestedValidators.insert(std::make_pair(++priority, Validator::DISPLAY_TYPE_VALIDATOR));
  }
  else if (inclusionCode.inclusionCode() == TRAVELOCITY_INCL)
  {
    LOG4CXX_DEBUG(logger, " Processing Inclusion Code for WEB ");
    _requestedValidators.insert(std::make_pair(priority, Validator::PAX_TYPE_VALIDATOR));
    _requestedValidators.insert(std::make_pair(++priority, Validator::WEB_VALIDATOR));
  }
  else if (inclusionCode.inclusionCode() == NET_FARES)
  {
    LOG4CXX_DEBUG(logger, " Processing Inclusion Code for NET");
    if (!_trx.getRequest()->displayPassengerTypes().empty())
      _requestedValidators.insert(std::make_pair(priority, Validator::PAX_TYPE_VALIDATOR));

    _requestedValidators.insert(std::make_pair(++priority, Validator::DISPLAY_TYPE_VALIDATOR));
    _requestedValidators.insert(std::make_pair(++priority, Validator::FARE_TYPE_VALIDATOR));
    _requestedValidators.insert(std::make_pair(++priority, Validator::RULE_TARIFF_VALIDATOR));
  }
  else
  {
    LOG4CXX_DEBUG(logger, " Processing Inclusion Code for Non-Combinable Type ");
    _requestedValidators.insert(std::make_pair(priority, Validator::PAX_TYPE_VALIDATOR));
    _requestedValidators.insert(std::make_pair(++priority, Validator::DISPLAY_TYPE_VALIDATOR));
    _requestedValidators.insert(std::make_pair(++priority, Validator::FARE_TYPE_VALIDATOR));
    _requestedValidators.insert(std::make_pair(++priority, Validator::RULE_TARIFF_VALIDATOR));
  }

  if (!_requestedValidators.empty())
    buildValidators();
}

void
ValidatorBuilder::getAllSpecified()
{
  if(fallback::fallbackFareDisplayByCabinActivation(&_trx) ||
     !_trx.getRequest()->multiInclusionCodes())
  {
     return getAll(*_trx.fdResponse()->fareDisplayInclCd());
  }

  // get all validators for each Inclusion code and collect them in the map
  std::map<uint8_t, FareDisplayInclCd*>::const_iterator iter =
                                  _trx.fdResponse()->fdInclCdPerInclCode().begin();
  std::map<uint8_t, FareDisplayInclCd*>::const_iterator iterE =
                                  _trx.fdResponse()->fdInclCdPerInclCode().end();

  for(; iter != iterE; ++iter)
  {
    if(iter->second != nullptr)
    {
      _trx.fdResponse()->fareDisplayInclCd() = iter->second;
      _validators.clear();
      getAll(*_trx.fdResponse()->fareDisplayInclCd());
      if(!_validators.empty())
        _inclusionCodeValidators.insert(std::make_pair(iter->first, _validators));
    }
  }
}

void
ValidatorBuilder::getAll()
{

  uint16_t priority(1);
  LOG4CXX_DEBUG(logger, " Processing Inclusion Code for WEB ");
  _requestedValidators.insert(std::make_pair(priority, Validator::PAX_TYPE_VALIDATOR));
  _requestedValidators.insert(std::make_pair(++priority, Validator::WEB_VALIDATOR));
  buildValidators();
}

Validator*
ValidatorBuilder::getPaxTypeValidator()
{
  if (!_trx.getRequest()->displayPassengerTypes().empty())
  {
    LOG4CXX_DEBUG(logger,
                  " Entering ValidatorBuilder::getPaxTypeValidator() - Requested Pax Type");
    RequestedPaxTypeValidator* validator(nullptr);
    _trx.dataHandle().get(validator);
    return validator;
  }
  else if (_trx.getOptions()->isChildFares() || _trx.getOptions()->isInfantFares())
  {
    LOG4CXX_DEBUG(logger,
                  " Entering ValidatorBuilder::getPaxTypeValidator() - Qualifier Pax Type");
    QualifierPaxTypeValidator* validator(nullptr);
    _trx.dataHandle().get(validator);
    return validator;
  }
  else
  {

    LOG4CXX_DEBUG(logger, " Entering ValidatorBuilder::getPaxTypeValidator() - General Pax Type");
    PaxTypeValidator* validator(nullptr);
    _trx.dataHandle().get(validator);
    return validator;
  }
}

Validator*
ValidatorBuilder::getFareTypeValidator()
{
  LOG4CXX_DEBUG(logger, " Entering ValidatorBuilder::getFareTypeValidator() - ");
  FareTypeValidator* validator(nullptr);
  _trx.dataHandle().get(validator);
  return validator;
}

Validator*
ValidatorBuilder::getDisplayTypeValidator()
{
  LOG4CXX_DEBUG(logger, " Entering ValidatorBuilder::getDisplayTypeValidator() - ");
  DisplayTypeValidator* validator(nullptr);
  _trx.dataHandle().get(validator);
  return validator;
}

Validator*
ValidatorBuilder::getRuleTariffValidator()
{
  LOG4CXX_DEBUG(logger, " Entering ValidatorBuilder::getRuleTariffValidator() - ");
  RuleTariffValidator* validator(nullptr);
  _trx.dataHandle().get(validator);
  return validator;
}

Validator*
ValidatorBuilder::getWebICValidator()
{
  LOG4CXX_DEBUG(logger, " Entering ValidatorBuilder::getWebInclusionCodeValidator() - ");
  WebICValidator* validator(nullptr);
  _trx.dataHandle().get(validator);
  return validator;
}

bool
ValidatorBuilder::buildValidators()
{
  uint16_t size(_requestedValidators.size());

  LOG4CXX_DEBUG(
      logger,
      " Entering ValidatorBuilder::buildValidators() -No of Validators Trying to Build --  "
          << size);

  for (uint16_t i = 1; i <= size; ++i)
  {
    Validator::ValidatorType validatorType = _requestedValidators[i];

    switch (validatorType)
    {
    case Validator::PAX_TYPE_VALIDATOR:
    {
      Validator* validator = getPaxTypeValidator();
      add(validator);
    }
    break;
    case Validator::FARE_TYPE_VALIDATOR:
    {
      Validator* validator = getFareTypeValidator();
      add(validator);
    }
    break;
    case Validator::DISPLAY_TYPE_VALIDATOR:
    {
      Validator* validator = getDisplayTypeValidator();
      add(validator);
    }
    break;
    case Validator::RULE_TARIFF_VALIDATOR:
    {
      Validator* validator = getRuleTariffValidator();
      add(validator);
    }
    break;
    case Validator::WEB_VALIDATOR:
    {
      Validator* validator = getWebICValidator();
      add(validator);
    }
    break;
    default:
      break; // TODO--ERROR MESSAGE
    }
  }
  LOG4CXX_DEBUG(logger,
                " Leaving ValidatorBuilder::buildValidators() -No of Validators is Built--  "
                    << _validators.size());
  return !_validators.empty();
}
}
