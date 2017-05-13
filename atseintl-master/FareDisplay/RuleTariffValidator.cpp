//-------------------------------------------------------------------
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/RuleTariffValidator.h"

#include "Common/Logger.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareDispInclRuleTrf.h"
#include "DBAccess/FareDisplayInclCd.h"

namespace tse
{

static Logger
logger("atseintl.FareDisplay.RuleTariffValidator");

using namespace std;

bool
RuleTariffValidator::initialize(const FareDisplayTrx& trx)
{
  if (trx.fdResponse()->fareDisplayInclCd() == nullptr)
  {
    return false;
  }
  else
  {
    _fareDisplayInclCd = trx.fdResponse()->fareDisplayInclCd();
    return getData(trx);
  }
}
//------------------------------------------------------
// RuleTariffValidator::qualify()
//------------------------------------------------------

bool
RuleTariffValidator::validate(const PaxTypeFare& fare)
{
  LOG4CXX_DEBUG(logger, "Entered RuleTariffValidator::validateRuleTariff()");

  std::vector<FareDispInclRuleTrf*>::const_iterator iter = _ruleTariffRecs.begin();
  std::vector<FareDispInclRuleTrf*>::const_iterator iterEnd = _ruleTariffRecs.end();
  for (; iter != iterEnd; iter++)
  {
    if (((*iter)->vendorCode() == fare.vendor()) && ((*iter)->ruleTariff() == fare.fareTariff()))
    {
      return true;
    }
  }

  LOG4CXX_DEBUG(logger,
                "RuleTariffValidator::validate() - Fare "
                    << fare.fareClass() << "FAILED"
                    << "Vendor -- :  " << fare.vendor() << "   Tariff -- : " << fare.fareTariff());
  return false;
}

bool
RuleTariffValidator::getData(const FareDisplayTrx& trx)
{
  // Get the rule tariffs
  const std::vector<FareDispInclRuleTrf*>& ruleTariffRecs =
      trx.dataHandle().getFareDispInclRuleTrf(_fareDisplayInclCd->userApplType(),
                                              _fareDisplayInclCd->userAppl(),
                                              _fareDisplayInclCd->pseudoCityType(),
                                              _fareDisplayInclCd->pseudoCity(),
                                              _fareDisplayInclCd->inclusionCode());
  if (!ruleTariffRecs.empty())
  {
    std::copy(ruleTariffRecs.begin(), ruleTariffRecs.end(), back_inserter(_ruleTariffRecs));
    return true;
  }
  return false;
}
}
