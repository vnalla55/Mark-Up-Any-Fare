//-------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#pragma once

namespace tse
{
class BagValidationOpt;

enum AllowanceStepResult
{
  S5_FAIL,
  S7_FAIL,
  S7_PASS,
  S7_DEFER,
};

class AllowanceStepProcessor
{
public:
  virtual ~AllowanceStepProcessor() = default;
  virtual AllowanceStepResult matchAllowance(const BagValidationOpt& opt) = 0;
  virtual void dotTableCheckFailed(const BagValidationOpt& opt) = 0;
};

}
