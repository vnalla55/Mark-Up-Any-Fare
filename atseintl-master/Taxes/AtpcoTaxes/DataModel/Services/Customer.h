// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once

#include "DataModel/Common/Types.h"

namespace tax
{

struct Customer
{
  bool _exemptDuG3 = false;
  bool _exemptDuJJ = false;
  bool _exemptDuT4 = false;
  type::PseudoCityCode _pcc;

  Customer() = default;
  Customer(const Customer& src) = default;

  Customer(bool exmeptDuG3, bool exmeptDuJJ, bool exmeptDuT4, type::PseudoCityCode pcc)
    : _exemptDuG3(exmeptDuG3)
    , _exemptDuJJ(exmeptDuJJ)
    , _exemptDuT4(exmeptDuT4)
    , _pcc(pcc)
  {};
};

}
