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

#include "Common/DateTime.h"
#include "DBAccess/TaxRulesRecordDAO.h"
#include "ServiceInterfaces/RulesRecordsService.h"
#include "Taxes/LegacyFacades/ApplicationCache.h"

#include <memory>

namespace tse
{
class DateTime;

class RulesRecordsServiceV2 : public tax::RulesRecordsService
{
public:
  RulesRecordsServiceV2(const RulesRecordsServiceV2&) = delete;
  RulesRecordsServiceV2& operator=(const RulesRecordsServiceV2&) = delete;

  static RulesRecordsServiceV2& instance();

  // overrides
  ByNationConstValue
  getTaxRulesContainers(const tax::type::Nation& nation,
                        const tax::type::TaxPointTag& taxPointTag,
                        const tax::type::Timestamp& ticketingDate) const final override;

  SharedTaxRulesRecord // can be null
  getTaxRecord(tax::type::Nation nation,
               tax::type::TaxCode taxCode,
               tax::type::TaxType taxType,
               tax::type::Index seqNo,
               const tax::type::Timestamp& ticketingDate) const final override;

  auto getTaxRecords(const tax::type::Nation& nation,
                     const tax::type::TaxPointTag& taxPointTag,
                     const tax::type::Timestamp& ticketingDate) const
                     -> std::vector<tax::RulesRecord>
                     final override;

  std::shared_ptr<const std::vector<SharedTaxRulesRecord>>
  getTaxRecords(const tax::type::TaxCode& taxCode,
                const tax::type::Timestamp& ticketingDate) const final override;

protected:
  RulesRecordsServiceV2() = default;

private:
  mutable ApplicationCache<TaxRulesRecordKey,
                           RulesContainersGroupedByTaxName> _byNationCache{TaxRulesRecordDAO::instance()};

  mutable ApplicationCache<RulesRecordHistoricalKey,
                           RulesContainersGroupedByTaxName> _byNationCacheHistorical{TaxRulesRecordHistoricalDAO::instance()};

  mutable ApplicationCache<TaxRulesRecordByCodeKey,
                           std::vector<SharedTaxRulesRecord>> _byCodeCache{TaxRulesRecordByCodeDAO::instance()};

  mutable ApplicationCache<RulesRecordByCodeHistoricalKey,
                           std::vector<SharedTaxRulesRecord>> _byCodeCacheHistorical{TaxRulesRecordByCodeHistoricalDAO::instance()};
};

} // namespace tse

