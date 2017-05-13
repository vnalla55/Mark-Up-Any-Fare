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

#include "Common/PointeeComparator.h"
#include "Common/TaxName.h"
#include "Common/Timestamp.h"
#include "DataModel/Common/Types.h"

#include <boost/ptr_container/ptr_vector.hpp>

#include <memory>
#include <utility>
#include <vector>

namespace tax
{
class RulesRecord;
class TaxData;

class RulesRecordsService
{
public:
  typedef std::pair<type::Nation, type::TaxPointTag> ByNationKey;
  typedef std::pair<type::TaxCode, type::TaxType> ByCodeKey;

  typedef boost::ptr_vector<TaxData> RulesContainersGroupedByTaxName;

  typedef std::shared_ptr<RulesContainersGroupedByTaxName> ByNationValue;

  typedef std::shared_ptr<const RulesContainersGroupedByTaxName> ByNationConstValue;

  typedef std::shared_ptr<const RulesRecord> SharedTaxRulesRecord;

  RulesRecordsService(void) {};
  virtual ~RulesRecordsService(void) {};

  virtual ByNationConstValue getTaxRulesContainers(const type::Nation& nation,
                                                   const type::TaxPointTag& taxPointTag,
                                                   const type::Timestamp& ticketingDate) const = 0;

  virtual SharedTaxRulesRecord // can be null
  getTaxRecord(type::Nation nation,
               type::TaxCode taxCode,
               type::TaxType taxType,
               type::Index seqNo,
               const type::Timestamp& ticketingDate) const = 0;

  virtual auto getTaxRecords(const type::Nation& nation,
                             const type::TaxPointTag& taxPointTag,
                             const type::Timestamp& ticketingDate) const
                             -> std::vector<RulesRecord>
                             = 0;

  virtual std::shared_ptr<const std::vector<SharedTaxRulesRecord>>
  getTaxRecords(const type::TaxCode& taxCode,
                const type::Timestamp& ticketingDate) const = 0;

}; // class RulesRecordsService

} // namespace tax

