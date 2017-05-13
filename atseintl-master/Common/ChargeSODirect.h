#pragma once

#include "Common/TsePrimitiveTypes.h"

namespace tse
{
class ChargeSODirect
{
public:
  ChargeSODirect()
    : _direction(' '),
      _charge1_FirstAmt_SOnumber_applied(0),
      _charge1_FirstAdd_SOnumber_applied(0),
      _charge2_FirstAmt_SOnumber_applied(0),
      _charge2_FirstAdd_SOnumber_applied(0),
      _ruleItemNumber(0)
  {
  }

  ChargeSODirect(const Indicator direction,
                 const int16_t charge1_FirstAmt,
                 const int16_t charge1_FirstAdd,
                 const int16_t charge2_FirstAmt,
                 const int16_t charge2_FirstAdd,
                 const uint32_t ruleItemNumber)
    : _direction(direction),
      _charge1_FirstAmt_SOnumber_applied(charge1_FirstAmt),
      _charge1_FirstAdd_SOnumber_applied(charge1_FirstAdd),
      _charge2_FirstAmt_SOnumber_applied(charge2_FirstAmt),
      _charge2_FirstAdd_SOnumber_applied(charge2_FirstAdd),
      _ruleItemNumber(ruleItemNumber)
  {
  }

  virtual ~ChargeSODirect() {}

  const Indicator& direction() const { return _direction; }
  Indicator& direction() { return _direction; }

  const int16_t& charge1_FirstAmt() const { return _charge1_FirstAmt_SOnumber_applied; }
  int16_t& charge1_FirstAmt() { return _charge1_FirstAmt_SOnumber_applied; }

  const int16_t& charge1_FirstAdd() const { return _charge1_FirstAdd_SOnumber_applied; }
  int16_t& charge1_FirstAdd() { return _charge1_FirstAdd_SOnumber_applied; }

  const int16_t& charge2_FirstAmt() const { return _charge2_FirstAmt_SOnumber_applied; }
  int16_t& charge2_FirstAmt() { return _charge2_FirstAmt_SOnumber_applied; }

  const int16_t& charge2_FirstAdd() const { return _charge2_FirstAdd_SOnumber_applied; }
  int16_t& charge2_FirstAdd() { return _charge2_FirstAdd_SOnumber_applied; }

  const uint32_t& ruleItemNumber() const { return _ruleItemNumber; }
  uint32_t& ruleItemNumber() { return _ruleItemNumber; }

private:
  Indicator _direction;
  int16_t _charge1_FirstAmt_SOnumber_applied;
  int16_t _charge1_FirstAdd_SOnumber_applied;
  int16_t _charge2_FirstAmt_SOnumber_applied;
  int16_t _charge2_FirstAdd_SOnumber_applied;
  uint32_t _ruleItemNumber;
};
} // namespace tse
