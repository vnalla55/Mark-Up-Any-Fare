#include "Common/SecurityHandshakeValidator.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/CustomerSecurityHandshakeInfo.h"

namespace tse
{

bool
SecurityHandshakeValidator::validateSecurityHandshake(
    PricingTrx& trx,
    const SecurityHandshakeProductCode& productCode,
    const PseudoCityCode& agencyPCC)
{
  const DateTime localTimeDT = DateTime::localTime();
  const std::vector<CustomerSecurityHandshakeInfo*>& cSH =
    trx.dataHandle().getCustomerSecurityHandshake(agencyPCC, productCode, localTimeDT);

  return (!cSH.empty());
}

}

