#include "Common/VCTR.h"

#include "Util/BranchPrediction.h"

namespace tse
{
void
VCTR::clear()
{
  _vendor = "";
  _carrier = "";
  _tariff = 0;
  _rule = "";
  _sequenceNumber = 0;
}

VCTR&
VCTR::
operator=(const VCTR& rhs)
{
  if (UNLIKELY(this == &rhs))
    return *this;

  _vendor = rhs._vendor;
  _carrier = rhs._carrier;
  _tariff = rhs._tariff;
  _rule = rhs._rule;
  _sequenceNumber = rhs._sequenceNumber;

  return *this;
}

bool
VCTR::
operator==(const VCTR& rhs) const
{
  if (UNLIKELY(this == &rhs))
    return true;

  if ((_vendor == rhs._vendor) && (_carrier == rhs._carrier) && (_tariff == rhs._tariff) &&
      (_rule == rhs._rule) && (_sequenceNumber == rhs._sequenceNumber))
  {
    return true;
  }
  return false;
}
} // namespace tse

