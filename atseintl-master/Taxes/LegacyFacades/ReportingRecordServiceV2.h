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

#include "DBAccess/ChildCache.h"
#include "DBAccess/TaxReportingRecordDAO.h"
#include "ServiceInterfaces/ReportingRecordService.h"
#include "Taxes/LegacyFacades/ApplicationCache.h"

namespace tse
{
class DateTime;
class ReportingRecord;

class ReportingRecordServiceV2 : public tax::ReportingRecordService
{
public:
  ReportingRecordServiceV2(const DateTime& ticketingDT);
  virtual ~ReportingRecordServiceV2();

  virtual SharedConstSingleValue
  getReportingRecord(const tax::type::Vendor& vendor,
                     const tax::type::Nation& nation,
                     const tax::type::CarrierCode& taxCarrierCode,
                     const tax::type::TaxCode& taxCode,
                     const tax::type::TaxType& taxType) const override;

  virtual SharedConstGroupValue
  getReportingRecords(const tax::type::TaxCode& taxCode) const override;

private:
  ReportingRecordServiceV2(const ReportingRecordServiceV2&);
  ReportingRecordServiceV2& operator=(const ReportingRecordServiceV2&);

  const DateTime& _ticketingDT;

  class SingleService : public ChildCache<TaxReportingRecordKey>
  {
  public:
    static SingleService& instance();
    void keyRemoved(const TaxReportingRecordKey& key) override;
    void cacheCleared() override;
    SharedConstSingleValue get(const SingleKey&, const DateTime&);

    ~SingleService();
  private:
    SingleService();
    SingleService(const SingleService&);
    SingleService& operator=(const SingleService&);

    ApplicationCache<SingleKey, SingleValue> _cache;
  };

  class GroupService : public ChildCache<TaxReportingRecordByCodeKey>
  {
  public:
    static GroupService& instance();
    void keyRemoved(const TaxReportingRecordByCodeKey& key) override;
    void cacheCleared() override;
    SharedConstGroupValue get(const GroupKey&, const DateTime&);

    ~GroupService();
  private:
    GroupService();
    GroupService(const GroupService&);
    GroupService& operator=(const GroupService&);

    ApplicationCache<GroupKey, GroupValue> _cache;
  };
};
}

