//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseBoostStringTypes.h"

#include <vector>

namespace tse
{
class PricingTrx;
class SvcFeesAccCodeInfo;
class SvcFeesDiagCollector;

class SvcFeesAccountCodeValidator
{
  friend class SvcFeesAccountCodeValidatorTest;

public:
  SvcFeesAccountCodeValidator(PricingTrx& trx, SvcFeesDiagCollector* diag);
  virtual ~SvcFeesAccountCodeValidator() {}

  bool validate(int itemNo) const;

protected:
  bool validateAccCode(const SvcFeesAccCodeInfo& accCode) const;
  bool validateMultiAccCode(const SvcFeesAccCodeInfo& accCode) const;
  bool validateSingleAccCode(const SvcFeesAccCodeInfo& accCode) const;
  static bool matchAccountCode(const char* accCodeT172CStr, const char* accCodeInputCStr);
  static bool beginsWithTwoDigits(const char* text);

  // Database overrides
  virtual const std::vector<SvcFeesAccCodeInfo*>& getSvcFeesAccCodeInfo(int itemNo) const;

private:
  PricingTrx& _trx;
  SvcFeesDiagCollector* _diag;
  static const std::string ASTERISK;
};
}

