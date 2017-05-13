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

#include <string>
#include <vector>

namespace tax
{

class TaxCodeTextService
{
public:
  TaxCodeTextService() {}
  virtual ~TaxCodeTextService() {}

  virtual void getTaxCodeText(type::Index itemNo,
                              tax::type::Vendor vendor,
                              std::vector<std::string>& out) const = 0;
};

}
