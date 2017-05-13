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

#include "Pricing/Shopping/FOS/FosTaskScope.h"

namespace tse
{
namespace fos
{

class AdditionalDirectFosTaskScope : public FosTaskScope
{
public:
  AdditionalDirectFosTaskScope(uint32_t numAdditionalDirectFos,
                               const std::map<CarrierCode, uint32_t>& numFosPerCarrier)
  {
    FosTaskScope::_deferredAdditionalNSProcessing = true;

    FosTaskScope::_numDirectFos = numAdditionalDirectFos;
    FosTaskScope::_numDirectFosPerCarrier = numFosPerCarrier;
  }
};

} // ns fos
} // ns tse

