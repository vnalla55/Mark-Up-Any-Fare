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
#include "Diagnostic/LoggingService.h"
#include "ServiceInterfaces/RulesRecordsService.h"
#include "Common/Handle.h"

#include <boost/core/noncopyable.hpp>
#include <memory>

namespace tax
{

class LoggingRulesRecordService : public RulesRecordsService
                                , public LoggingService
                                , private boost::noncopyable
{
  Handle<RulesRecordsService> _base;
  class Impl;
  std::unique_ptr<Impl> _impl;
  Impl& impl();
  const Impl& impl() const;

public:
  typedef RulesRecordsService BaseService;
  LoggingRulesRecordService(std::unique_ptr<BaseService> base);
  LoggingRulesRecordService(BaseService& base);
  ~LoggingRulesRecordService();

  ByNationConstValue
    getTaxRulesContainers(const type::Nation& nation,
                          const type::TaxPointTag& taxPointTag,
                          const type::Timestamp& ticketingDate) const final;

  SharedTaxRulesRecord // can be null
    getTaxRecord(type::Nation nation,
                 type::TaxCode taxCode,
                 type::TaxType taxType,
                 type::Index seqNo,
                 const type::Timestamp& ticketingDate) const final;

  auto getTaxRecords(const type::Nation& nation,
                     const type::TaxPointTag& taxPointTag,
                     const type::Timestamp& ticketingDate) const
                     -> std::vector<RulesRecord>
                     final;

  std::shared_ptr<const std::vector<SharedTaxRulesRecord>>
  getTaxRecords(const type::TaxCode& taxCode,
                const type::Timestamp& ticketingDate) const final override;

  std::string getLog() override;
};

} // namespace tax

