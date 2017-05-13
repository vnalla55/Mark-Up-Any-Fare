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

#include <boost/ptr_container/ptr_vector.hpp>

#include "DataModel/Services/RepricingEntry.h"
#include "ServiceInterfaces/RepricingService.h"

namespace tax
{

class RepricingServiceServer : public RepricingService
{
public:
  RepricingServiceServer();

  type::MoneyAmount getFareUsingUSDeductMethod(const type::Index& taxPointBegin,
                                               const type::Index& taxPointEnd,
                                               const type::Index& itinId) const override;

  type::MoneyAmount getFareFromFareList(const type::Index& taxPointBegin,
                                        const type::Index& taxPointEnd,
                                        const type::Index& itinId) const override;

  type::MoneyAmount getSimilarDomesticFare(const type::Index& taxPointBegin,
                                           const type::Index& taxPointEnd,
                                           const type::Index& itinId,
                                           bool& fareFound) const override;

  type::MoneyAmount getBahamasSDOMFare(const type::Index& taxPointBegin,
                                       const type::Index& taxPointEnd,
                                       const type::Index& itinId) const override;

  boost::ptr_vector<RepricingEntry>& repricingEntries() { return _repricingEntries; }
  const boost::ptr_vector<RepricingEntry>& repricingEntries() const
  {
    return _repricingEntries;
  };

private:
  boost::ptr_vector<RepricingEntry> _repricingEntries;
};
}
