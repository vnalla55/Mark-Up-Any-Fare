//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
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

#include "DataModel/IbfAvailabilityTracker.h"

#include "Common/Assert.h"
#include "Common/Logger.h"
#include "DataModel/IbfAvailabilityStatusAccumulators.h"


namespace tse
{

namespace
{
Logger
logger("atseintl.DataModel.IbfAvailabilityTracker");
}

void
IbfAvlTracker::setStatus(const BrandCode& brandCode, IbfErrorMessage ibfErrorMessage)
{
  LOG4CXX_INFO(logger, "Setting " << ibfErrorMessage << " for brand " << brandCode);
  _brandErrors[brandCode] = ibfErrorMessage;
}

void
IbfAvlTracker::setStatusForLeg(const BrandCode& brandCode,
                               LegId legId,
                               IbfErrorMessage ibfErrorMessage)
{
  LOG4CXX_INFO(logger,
               "Setting " << ibfErrorMessage << " for brand " << brandCode << " for leg " << legId);

  IbfLegErrorMessageMap& mapForBrand = _brandErrorsPerLeg[brandCode];
  if (mapForBrand.find(legId) != mapForBrand.end())
  {
    LOG4CXX_WARN(logger,
                 "value for brandCode " << brandCode << " and legId " << legId << " already set to "
                                        << mapForBrand.find(legId)->second
                                        << ", now overwriting to " << ibfErrorMessage);
  }
  mapForBrand[legId] = ibfErrorMessage;
}

IbfErrorMessage
IbfAvlTracker::getStatusForLeg(const BrandCode& brandCode, LegId legId) const
{
  IbfErrorMessageMapPerLeg::const_iterator found = _brandErrorsPerLeg.find(brandCode);
  if (found == _brandErrorsPerLeg.end())
  {
    LOG4CXX_DEBUG(logger, "No status for brand " << brandCode);
    return IbfErrorMessage::IBF_EM_NOT_SET;
  }

  const IbfLegErrorMessageMap& errorsPerLeg = found->second;
  IbfLegErrorMessageMap::const_iterator statFound = errorsPerLeg.find(legId);
  if (statFound == errorsPerLeg.end())
  {
    LOG4CXX_DEBUG(logger, "No status for leg " << legId);
    return IbfErrorMessage::IBF_EM_NOT_SET;
  }

  LOG4CXX_DEBUG(logger,
                "Returning " << statFound->second << " for brand " << brandCode << " and leg id "
                             << legId);
  return statFound->second;
}

IbfErrorMessage
IbfAvlTracker::getStatusForBrand(const BrandCode& brandCode) const
{
  BrandErrors::const_iterator found = _brandErrors.find(brandCode);
  if (found == _brandErrors.end())
  {
    LOG4CXX_DEBUG(logger, "No leg record for brand " << brandCode);
    return IbfErrorMessage::IBF_EM_NOT_SET;
  }

  LOG4CXX_DEBUG(logger, "Returning " << found->second << " for brand " << brandCode);
  return found->second;
}

bool IbfAvlTracker::atLeastOneLegValidForBrand(const BrandCode& brandCode) const
{

  if(_brandErrorsPerLeg.find(brandCode) == _brandErrorsPerLeg.end())
    return false;

  for (const IbfLegErrorMessageMap::value_type& i : _brandErrorsPerLeg.at(brandCode))
    if(i.second == IbfErrorMessage::IBF_EM_NO_FARE_FOUND)
      return true;

  return false;
}

} // namespace tse
