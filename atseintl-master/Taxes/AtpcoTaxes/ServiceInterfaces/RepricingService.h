// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include <boost/optional.hpp>

namespace tax
{

class RepricingService
{
public:
  RepricingService() {}

  virtual ~RepricingService() {}

  virtual type::MoneyAmount getFareUsingUSDeductMethod(const type::Index& taxPointBegin,
                                                       const type::Index& taxPointEnd,
                                                       const type::Index& itinId) const = 0;

  virtual type::MoneyAmount getFareFromFareList(const type::Index& taxPointBegin,
                                                const type::Index& taxPointEnd,
                                                const type::Index& itinId) const = 0;

  virtual type::MoneyAmount getSimilarDomesticFare(const type::Index& taxPointBegin,
                                                   const type::Index& taxPointEnd,
                                                   const type::Index& itinId,
                                                   bool& fareFound) const = 0;

  virtual type::MoneyAmount getBahamasSDOMFare(const type::Index& taxPointBegin,
                                               const type::Index& taxPointEnd,
                                               const type::Index& itinId) const = 0;

private:
  RepricingService(const RepricingService&);
  RepricingService& operator=(const RepricingService&);
};
}
