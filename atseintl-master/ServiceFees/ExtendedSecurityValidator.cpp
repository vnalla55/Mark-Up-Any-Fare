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

#include "ServiceFees/ExtendedSecurityValidator.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/SvcFeesSecurityInfo.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"

namespace tse
{
static Logger
logger("atseintl.ServiceFees.ExtendedSecurityValidator");

ExtendedSecurityValidator::ExtendedSecurityValidator(
    PricingTrx& trx,
    const std::vector<TravelSeg*>::const_iterator segI,
    const std::vector<TravelSeg*>::const_iterator segIE)
  : SecurityValidator(trx, segI, segIE)
{
}

ExtendedSecurityValidator::~ExtendedSecurityValidator() {}

StatusT183Security
ExtendedSecurityValidator::validateSequence(const SvcFeesSecurityInfo* secInfo, bool& i) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  if (UNLIKELY(!checkTvlAgency(secInfo)))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_TVL_AGENCY");
    return FAIL_TVL_AGENCY;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_TVL_AGENCY");
  }

  if (!checkGds(secInfo))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_CXR_GDS");
    return FAIL_CXR_GDS;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_CXR_GDS");
  }

  if (UNLIKELY(!checkDutyCode(secInfo)))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_DUTY");
    return FAIL_DUTY;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_DUTY");
  }

  if (!checkLoc(secInfo))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_GEO");
    return FAIL_GEO;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_GEO");
  }

  if (!checkCode(secInfo))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_TYPE_CODE");
    return FAIL_TYPE_CODE;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_TYPE_CODE");
  }

  if (UNLIKELY(!checkViewIndicator(secInfo)))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_VIEW_IND");
    return FAIL_VIEW_IND;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_VIEW_IND");
  }

  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning PASS_T183");
  return PASS_T183;
}

bool
ExtendedSecurityValidator::checkDutyCode(const SvcFeesSecurityInfo* secInfo) const
{
  LOG4CXX_DEBUG(logger, "Entered ExtendedSecurityValidator::checkDutyCode()");
  if (UNLIKELY(!secInfo->dutyFunctionCode().empty()))
  {
    if (!RuleUtil::validateDutyFunctionCode(_trx.getRequest()->ticketingAgent(),
                                            secInfo->dutyFunctionCode()))
      return false;
  }
  return true;
}

bool
ExtendedSecurityValidator::checkViewIndicator(const SvcFeesSecurityInfo* secInfo) const
{
  LOG4CXX_DEBUG(logger, "Entered ExtendedSecurityValidator::checkViewIndicator()");
  return secInfo->viewBookTktInd() != '2';
}
} // tse
