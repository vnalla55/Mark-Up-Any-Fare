//-------------------------------------------------------------------
//  Copyright Sabre 2012
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

#include "ServiceFees/CommonSoftMatchValidator.h"

#include "Common/Logger.h"
#include "DBAccess/SvcFeesCxrResultingFCLInfo.h"
#include "DataModel/PaxTypeFare.h"
#include "ServiceFees/OCFees.h"

namespace tse
{
static Logger
logger("atseintl.ServiceFees.CommonSoftMatchValidator");

bool
CommonSoftMatchValidator::matchRuleTariff(const uint16_t& ruleTariff,
                                          const PaxTypeFare& ptf,
                                          OCFees& ocFees) const
{
  if (ptf.tcrRuleTariff() == 0 && ptf.fareClassAppInfo()->_ruleTariff == 0)
  {
    ocFees.softMatchS7Status().set(OCFees::S7_RULETARIFF_SOFT);
    return true;
  }
  return OptionalServicesValidator::matchRuleTariff(ruleTariff, ptf, ocFees);
}

bool
CommonSoftMatchValidator::checkRuleTariff(const uint16_t& ruleTariff, OCFees& ocFees) const
{
  LOG4CXX_DEBUG(logger, "Entered CommonSoftMatchValidator::checkRuleTariff()");
  if (ruleTariff == (uint16_t) - 1)
    return true;

  if (_processedFares.empty())
  {
    ocFees.softMatchS7Status().set(OCFees::S7_RULETARIFF_SOFT);
    return true;
  }

  for (const auto fare : _processedFares)
  {
    if (!matchRuleTariff(ruleTariff, *fare, ocFees))
      return false;
  }
  return true;
}

bool
CommonSoftMatchValidator::checkRule(const RuleNumber& rule, OCFees& ocFees) const
{
  LOG4CXX_DEBUG(logger, "Entered CommonSoftMatchValidator::checkRule()");
  if (rule.empty())
    return true;

  if (_processedFares.empty())
  {
    ocFees.softMatchS7Status().set(OCFees::S7_RULE_SOFT);
    return true;
  }

  for (const auto fare : _processedFares)
  {
    if (fare->ruleNumber().empty())
    {
      ocFees.softMatchS7Status().set(OCFees::S7_RULE_SOFT);
      return true;
    }
    if (fare->ruleNumber() != rule)
      return false;
  }
  return true;
}

StatusT171
CommonSoftMatchValidator::isValidFareClassFareType(const PaxTypeFare* ptf,
                                                   SvcFeesCxrResultingFCLInfo& fclInfo,
                                                   OCFees& ocFees) const
{
  LOG4CXX_DEBUG(logger, "Entered CommonSoftMatchValidator::isValidFareClassFareType()");
  if (!fclInfo.resultingFCL().empty() && ptf->fare()->fareInfo()->fareClass().empty())
  {
    ocFees.softMatchS7Status().set(OCFees::S7_RESULTING_FARE_SOFT);
    ocFees.softMatchResultingFareClassT171().push_back(&fclInfo);
    return SOFTPASS_FARE_CLASS;
  }
  else if (!fclInfo.resultingFCL().empty() && !ptf->fare()->fareInfo()->fareClass().empty() &&
           isValidFareClass(ptf, fclInfo) && !fclInfo.fareType().empty() &&
           ptf->fcaFareType().empty())
  {
    ocFees.softMatchS7Status().set(OCFees::S7_RESULTING_FARE_SOFT);
    ocFees.softMatchResultingFareClassT171().push_back(&fclInfo);
    return SOFTPASS_FARE_TYPE;
  }
  else
    return OptionalServicesValidator::isValidFareClassFareType(ptf, fclInfo, ocFees);
}
}
