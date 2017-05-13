//-------------------------------------------------------------------
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
//-------------------------------------------------------------------
#include "ServiceFees/AncillarySecurityValidator.h"

#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/SvcFeesSecurityInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/SvcFeesDiagCollector.h"
#include "Rules/RuleUtil.h"
#include "ServiceFees/ExtendedSecurityValidator.h"

#include <boost/algorithm/string/trim.hpp>

namespace tse
{

static Logger
logger("atseintl.ServiceFees.AncillarySecurityValidator");

//--------------------------------------------------------------------
AncillarySecurityValidator::AncillarySecurityValidator(
    PricingTrx& trx,
    const std::vector<TravelSeg*>::const_iterator segI,
    const std::vector<TravelSeg*>::const_iterator segIE)
  : ExtendedSecurityValidator(trx, segI, segIE)
{
}

//--------------------------------------------------------------------
AncillarySecurityValidator::~AncillarySecurityValidator() {}

//--------------------------------------------------------------------
bool
AncillarySecurityValidator::checkDutyCode(const SvcFeesSecurityInfo* secInfo) const
{
  LOG4CXX_DEBUG(logger, "Entered AncillarySecurityValidator::checkDutyCode()");
  if (!secInfo->dutyFunctionCode().empty())
  {
    if (!RuleUtil::checkDutyFunctionCode(_trx.getRequest()->ticketingAgent(),
                                         secInfo->dutyFunctionCode()))
      return false;
  }
  return true;
}

bool
AncillarySecurityValidator::processAirlineSpecificX(const SvcFeesSecurityInfo* secInfo) const
{
  if (!TrxUtil::isPerformOACCheck(_trx))
    return false;

  std::string tmpCode = secInfo->code().loc();
  boost::trim_left(tmpCode);

  return tmpCode == _trx.getRequest()->ticketingAgent()->officeStationCode();
}


} // tse
