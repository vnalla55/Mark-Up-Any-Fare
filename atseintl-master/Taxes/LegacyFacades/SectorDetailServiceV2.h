// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "ServiceInterfaces/SectorDetailService.h"
#include "Taxes/LegacyFacades/TransactionCache.h"

#include <memory>

namespace tse
{

class SectorDetailServiceV2 : public tax::SectorDetailService
{
public:
  typedef std::pair<tax::type::Vendor, tax::type::Index> Key;
  typedef tax::SectorDetail Value;
  typedef std::shared_ptr<const Value> SharedConstValue;

  SectorDetailServiceV2(const DateTime& ticketingDate);
  virtual ~SectorDetailServiceV2() {}

  std::shared_ptr<const tax::SectorDetail>
  getSectorDetail(const tax::type::Vendor& vendor, tax::type::Index itemNo) const final;

private:
  SectorDetailServiceV2(const SectorDetailServiceV2&);
  SectorDetailServiceV2& operator=(const SectorDetailServiceV2&);

  mutable TransactionCache<Key, Value> _cache;
};
}
