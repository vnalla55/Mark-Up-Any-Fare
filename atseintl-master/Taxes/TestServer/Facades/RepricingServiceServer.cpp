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


#include "RepricingServiceServer.h"

namespace tax
{
RepricingServiceServer::RepricingServiceServer() {}

type::MoneyAmount
RepricingServiceServer::getFareUsingUSDeductMethod(const type::Index& taxPointBegin,
                                                   const type::Index& taxPointEnd,
                                                   const type::Index& itinId) const
{
  for(RepricingEntry const& entry : _repricingEntries)
  {
    if (entry.taxPointBegin == taxPointBegin && entry.taxPointEnd == taxPointEnd &&
        entry.itinId == itinId)
    {
      return entry.repricedAmount;
    }
  }
  return 0;
}

type::MoneyAmount
RepricingServiceServer::getFareFromFareList(const type::Index& taxPointBegin,
                                            const type::Index& taxPointEnd,
                                            const type::Index& itinId) const
{
  for (RepricingEntry const& entry : _repricingEntries)
  {
    if (entry.taxPointBegin == taxPointBegin && entry.taxPointEnd == taxPointEnd &&
        entry.itinId == itinId)
    {
      return entry.repricedAmount;
    }
  }
  return 0;
}

type::MoneyAmount
RepricingServiceServer::getSimilarDomesticFare(const type::Index& taxPointBegin,
                                               const type::Index& taxPointEnd,
                                               const type::Index& itinId,
                                               bool& fareFound) const
{
  fareFound = false;
  for(RepricingEntry const& entry : _repricingEntries)
  {
    if (entry.taxPointBegin == taxPointBegin && entry.taxPointEnd == taxPointEnd &&
        entry.itinId == itinId)
    {
      fareFound = true;
      return entry.repricedAmount;
    }
  }
  return 0;
}



type::MoneyAmount
RepricingServiceServer::getBahamasSDOMFare(const type::Index& taxPointBegin,
                                           const type::Index& taxPointEnd,
                                           const type::Index& itinId) const
{
  for(RepricingEntry const& entry : _repricingEntries)
  {
    if (entry.taxPointBegin == taxPointBegin && entry.taxPointEnd == taxPointEnd &&
        entry.itinId == itinId)
    {
      return entry.repricedAmount;
    }
  }
  return 0;
}

}
