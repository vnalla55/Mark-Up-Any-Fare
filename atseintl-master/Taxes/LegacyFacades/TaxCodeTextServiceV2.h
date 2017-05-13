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

#include "Taxes/AtpcoTaxes/DataModel/Common/Types.h"
#include "Taxes/AtpcoTaxes/ServiceInterfaces/TaxCodeTextService.h"

#include <string>
#include <vector>

namespace tse
{

class DataHandle;

class TaxCodeTextServiceV2 : public tax::TaxCodeTextService
{
public:
  explicit TaxCodeTextServiceV2(DataHandle& dataHandle);

  void getTaxCodeText(tax::type::Index itemNo,
                      tax::type::Vendor vendor,
                      std::vector<std::string>& out) const final;

private:
  DataHandle& _dataHandle;
};

} /* namespace tse */
