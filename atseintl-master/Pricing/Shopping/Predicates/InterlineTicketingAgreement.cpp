
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

#include "Pricing/Shopping/Predicates/InterlineTicketingAgreement.h"

#include "Common/Logger.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/Utils/ShoppingUtils.h"

namespace tse
{

namespace utils
{

namespace
{
Logger
logger("atseintl.Pricing.ShoppingUtils.InterlineTicketingAgreement");
}

bool
InterlineTicketingAgreement::
operator()(const SopCombination& sopIds)
{
  bool answer;

  if (_trx.isValidatingCxrGsaApplicable())
    answer = _validatingCarrierUpdater.processSops(_trx, sopIds);
  else
  {
    if ((_trx.getRequest()->processVITAData() == false) ||
        (_trx.getOptions()->validateTicketingAgreement() == false))
    {
      answer = true;
    }
    else
    {
      answer = _vitaValidator(sopIds);
    }
  }

  LOG4CXX_DEBUG(logger, "Returning " << answer << " for combination: " << sopIds);
  return answer;
}

} // namespace utils

} // namespace tse
