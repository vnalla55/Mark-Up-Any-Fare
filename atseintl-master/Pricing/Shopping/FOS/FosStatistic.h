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
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Pricing/Shopping/FOS/FosTypes.h"

#include <map>

#include <tr1/array>

namespace tse
{
class ShoppingTrx;

namespace fos
{

class FosStatistic
{
public:
  struct CarrierCounter
  {
    uint32_t value;
    uint32_t limit;

    CarrierCounter() : value(0), limit(0) {}
  };

  typedef std::map<CarrierCode, CarrierCounter> CarrierCounterMap;
  typedef std::tr1::array<uint32_t, VALIDATOR_LAST + 1> ValidatorCounters;
  explicit FosStatistic(const ShoppingTrx& trx);

  void setCounterLimit(ValidatorType vt, uint32_t limit);
  uint32_t getCounterLimit(ValidatorType vt) const { return _validatorCounterLimit[vt]; }
  uint32_t getCounter(ValidatorType vt) const { return _validatorCounter[vt]; }
  ValidatorBitMask getLackingValidators() const { return _lackingValidatorsBitMask; }

  const ValidatorCounters& getValidatorCounters() const { return _validatorCounter; }

  CarrierCounter& getCarrierCounter(CarrierCode cxr) { return _carrierCounter[cxr]; }
  CarrierCounter& getDirectCarrierCounter(CarrierCode cxr) { return _directCarrierCounter[cxr]; }

  const CarrierCounterMap& getCarrierCounterMap() const { return _carrierCounter; }
  const CarrierCounterMap& getDirectCarrierCounterMap() const { return _directCarrierCounter; }

  void addFOS(ValidatorBitMask countersBitMask, const SopIdVec& combination);
  void clear();

private:
  void updateLackingValidators(ValidatorType vt);

  const ShoppingTrx& _trx;

  ValidatorCounters _validatorCounterLimit;
  ValidatorCounters _validatorCounter;
  ValidatorBitMask _lackingValidatorsBitMask;

  std::map<CarrierCode, CarrierCounter> _carrierCounter;
  std::map<CarrierCode, CarrierCounter> _directCarrierCounter;
};

} // fos
} // tse
