// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#include "Pricing/Shopping/FOS/FosStatistic.h"

#include "Pricing/Shopping/FOS/FosCommonUtil.h"

#include <algorithm>
#include <cstring>

namespace tse
{
namespace fos
{

FosStatistic::FosStatistic(const ShoppingTrx& trx) : _trx(trx), _lackingValidatorsBitMask(0)
{
  std::fill(_validatorCounter.begin(), _validatorCounter.end(), 0u);
  std::fill(_validatorCounterLimit.begin(), _validatorCounterLimit.end(), 0u);
}

void
FosStatistic::setCounterLimit(ValidatorType vt, uint32_t limit)
{
  _validatorCounterLimit[vt] = limit;
  updateLackingValidators(vt);
}

void
FosStatistic::addFOS(ValidatorBitMask countersBitMask, const SopIdVec& combination)
{
  for (uint32_t i = 0; i <= VALIDATOR_LAST; ++i)
  {
    ValidatorType vt = static_cast<ValidatorType>(i);
    if (validatorBitMask(vt) & countersBitMask)
    {
      ++_validatorCounter[vt];
      updateLackingValidators(vt);
    }
  }

  if (validatorBitMask(VALIDATOR_ONLINE) & countersBitMask)
  {
    CarrierCode cxr = FosCommonUtil::detectCarrier(_trx, combination);
    ++getCarrierCounter(cxr).value;
  }

  if (validatorBitMask(VALIDATOR_NONSTOP) & countersBitMask)
  {
    CarrierCode cxr = FosCommonUtil::detectCarrier(_trx, combination);
    ++getDirectCarrierCounter(cxr).value;
  }
}

void
FosStatistic::clear()
{
  std::fill(_validatorCounter.begin(), _validatorCounter.end(), 0u);
  std::fill(_validatorCounterLimit.begin(), _validatorCounterLimit.end(), 0u);
  _lackingValidatorsBitMask = 0;
  _carrierCounter.clear();
  _directCarrierCounter.clear();
}

void
FosStatistic::updateLackingValidators(ValidatorType vt)
{
  uint32_t validatorBM = validatorBitMask(vt);
  if (_validatorCounter[vt] < _validatorCounterLimit[vt])
    _lackingValidatorsBitMask |= validatorBM;
  else
    _lackingValidatorsBitMask &= ~validatorBM;
}

} // fos
} // tse
