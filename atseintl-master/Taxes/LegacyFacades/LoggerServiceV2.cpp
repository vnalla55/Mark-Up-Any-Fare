// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Common/Logger.h"
#include "Taxes/LegacyFacades/LoggerServiceV2.h"

namespace tse
{

Logger logger("atseintl.AtpcoTaxes");

void
LoggerServiceV2::log_INFO(const char* message) const
{
  LOG4CXX_INFO(logger, message);
}

void
LoggerServiceV2::log_TRACE(const char* message) const
{
  LOG4CXX_TRACE(logger, message);
}

void
LoggerServiceV2::log_DEBUG(const char* message) const
{
  LOG4CXX_DEBUG(logger, message);
}

void
LoggerServiceV2::log_WARN(const char* message) const
{
  LOG4CXX_WARN(logger, message);
}

void
LoggerServiceV2::log_ERROR(const char* message) const
{
  LOG4CXX_ERROR(logger, message);
}

} // namespace tse
