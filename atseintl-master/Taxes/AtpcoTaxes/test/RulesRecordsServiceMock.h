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

#include "Common/TaxName.h"
#include "DataModel/Services/RulesRecord.h"
#include "ServiceInterfaces/RulesRecordsService.h"

#include <tuple>

namespace tax
{

class RulesRecordsServiceMock : public RulesRecordsService
{
  using ByCodeKey = std::tuple<type::TaxCode, type::TaxType>;

public:
  ByNationConstValue
  getTaxRulesContainers(const type::Nation& /*nation*/,
                        const type::TaxPointTag& /*taxPointTag*/,
                        const type::Timestamp& /*ticketingDate*/) const final { return _taxRulesContainers; }

  SharedTaxRulesRecord
  getTaxRecord(type::Nation /*nation*/,
               type::TaxCode /*taxCode*/,
               type::TaxType /*taxType*/,
               type::Index /*seqNo*/,
               const type::Timestamp& /*ticketingDate*/) const final { return _taxRecord; }

  auto getTaxRecords(const type::Nation& /*nation*/,
                     const type::TaxPointTag& /*taxPointTag*/,
                     const type::Timestamp& /*ticketingDate*/) const
                     -> std::vector<RulesRecord>
                     final
                     { return {}; }

  std::shared_ptr<const std::vector<SharedTaxRulesRecord>>
  getTaxRecords(const type::TaxCode& taxCode,
                const type::Timestamp& /* ticketingDate */) const final override;

  ByNationConstValue& taxRulesContainers() { return _taxRulesContainers; }
  SharedTaxRulesRecord& taxRecord() { return _taxRecord; }
  void addByCodeRecord(const SharedTaxRulesRecord& record)
  {
    auto found = _taxRecords.find(record->taxName.taxCode());
    if (found != _taxRecords.end())
    {
      found->second->push_back(record);
    }
    else
    {
      auto vector = std::make_shared<std::vector<SharedTaxRulesRecord>>();
      vector->push_back(record);
      _taxRecords.insert({record->taxName.taxCode(), vector});
    }
  }

private:
  ByNationConstValue _taxRulesContainers;
  mutable std::map<type::TaxCode, std::shared_ptr<std::vector<SharedTaxRulesRecord>> > _taxRecords;
  SharedTaxRulesRecord _taxRecord;
};

} /* namespace tax */
