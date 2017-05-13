#include "Xform/AwardPsgTypeChecker.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/ConfigList.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ShoppingTrx.h"

namespace tse
{
namespace
{
ConfigurableValue<ConfigSet<std::string>>
awardPsgType("SHOPPING_OPT", "AWARD_PSG_TYPE");
}

void
AwardPsgTypeChecker::checkPsgType(const PricingTrx& trx)
{
  getPaxTypeCodeVec();
  checkPsgType(trx.paxType());

  const ShoppingTrx* const shoppingTrx = dynamic_cast<const ShoppingTrx*>(&trx);
  if (shoppingTrx)
    checkPsgType(shoppingTrx->excludedPaxType());
}

void
AwardPsgTypeChecker::checkPsgType(const std::vector<PaxType*>& paxTypeVec)
{
  if (!paxTypeVec.empty())
  {
    std::vector<PaxType*>::const_iterator psgIt = paxTypeVec.begin();
    const std::vector<PaxType*>::const_iterator psgItEnd = paxTypeVec.end();
    for (; psgIt != psgItEnd; ++psgIt)
    {
      checkPsgType(*psgIt);
    }
  }
}

void
AwardPsgTypeChecker::getPaxTypeCodeVec()
{
  for (const auto token : awardPsgType.getValue())
    _paxTypeCodeVec.push_back(token.data());
}

void
AwardPsgTypeChecker::checkPsgType(const PaxType* const paxType)
{
  if (!isSupported(paxType))
  {
    std::stringstream msg;
    msg << "Passenger type: " << paxType->paxType() << " not supported for award request."
        << " Supported psg types: ";
    bool first = true;
    std::vector<PaxTypeCode>::const_iterator psgCodeIt = _paxTypeCodeVec.begin();
    const std::vector<PaxTypeCode>::const_iterator psgItCodeEnd = _paxTypeCodeVec.end();
    for (; psgCodeIt != psgItCodeEnd; ++psgCodeIt)
    {
      if (!first)
        msg << ", ";
      else
        first = false;

      msg << "'" << *psgCodeIt << "'";
    }

    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, msg.str().c_str());
  }
}

bool
AwardPsgTypeChecker::isSupported(const PaxType* const paxType) const
{
  if (!paxType)
    return false;

  bool found = false;

  std::vector<PaxTypeCode>::const_iterator it = _paxTypeCodeVec.begin();
  const std::vector<PaxTypeCode>::const_iterator itEnd = _paxTypeCodeVec.end();
  while ((!found) && (it != itEnd))
  {
    if (*it == paxType->paxType())
      found = true;
    else
      ++it;
  }

  return found;
}

} // namespace tse
