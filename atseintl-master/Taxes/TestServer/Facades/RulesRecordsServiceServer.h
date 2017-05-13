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

#include <map>
#include <boost/ptr_container/ptr_vector.hpp>

#include "DataModel/Common/Types.h"
#include "ServiceInterfaces/RulesRecordsService.h"

namespace tax
{

class RulesRecordsServiceServer : public RulesRecordsService
{
public:
  RulesRecordsServiceServer(void);
  virtual ~RulesRecordsServiceServer(void);

  ByNationConstValue getTaxRulesContainers(const type::Nation& nation,
                                           const type::TaxPointTag& taxPointTag,
                                           const type::Timestamp&) const final;

  auto getTaxRecords(const type::Nation& nation,
                     const type::TaxPointTag& taxPointTag,
                     const type::Timestamp& ticketingDate) const
                     -> std::vector<RulesRecord>
                     final;

  SharedTaxRulesRecord // can be null
  getTaxRecord(type::Nation,
               type::TaxCode,
               type::TaxType,
               type::Index,
               const type::Timestamp&) const final;

  std::shared_ptr<const std::vector<SharedTaxRulesRecord>>
  getTaxRecords(const type::TaxCode& /* taxCode */,
                const type::Timestamp& /* ticketingDate */) const final override
                { return std::shared_ptr<const std::vector<SharedTaxRulesRecord>>(); }

  void updateKeys();

  const boost::ptr_vector<RulesRecord>& rulesRecords() const { return _rulesRecords; };

  boost::ptr_vector<RulesRecord>& rulesRecords() { return _rulesRecords; };

private:
  mutable std::map<ByNationKey, ByNationValue> _rulesContainersMap;
  boost::ptr_vector<RulesRecord> _rulesRecords;
};
}

