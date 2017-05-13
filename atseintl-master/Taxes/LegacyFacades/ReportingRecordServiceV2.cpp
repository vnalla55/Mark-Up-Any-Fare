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
#include "Taxes/LegacyFacades/ReportingRecordServiceV2.h"

#include "DataModel/Services/ReportingRecord.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/TaxReportingRecordInfo.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "Taxes/LegacyFacades/DaoDataFormatConverter.h"

#include <tuple>

#include <boost/bind.hpp>

namespace tse
{
namespace
{
typedef ReportingRecordServiceV2::SharedConstSingleValue SharedConstSingleValue;
typedef ReportingRecordServiceV2::SharedSingleValue SharedSingleValue;
typedef ReportingRecordServiceV2::SingleValue SingleValue;
typedef ReportingRecordServiceV2::SingleKey SingleKey;
typedef ReportingRecordServiceV2::SharedConstGroupValue SharedConstGroupValue;
typedef ReportingRecordServiceV2::SharedGroupValue SharedGroupValue;
typedef ReportingRecordServiceV2::GroupValue GroupValue;
typedef ReportingRecordServiceV2::GroupKey GroupKey;

SharedConstSingleValue
readSingleRecord(DataHandle& handle, const SingleKey& key, const DateTime& ticketingDate)
{
  const std::vector<const TaxReportingRecordInfo*> taxReportingRecords =
      handle.getTaxReportingRecord(toTseVendorCode(std::get<0>(key)),
                                   toTseNationCode(std::get<1>(key)),
                                   toTseCarrierCode(std::get<2>(key)),
                                   toTseTaxCode(std::get<3>(key)),
                                   toTseTaxType(std::get<4>(key)),
                                   ticketingDate);

  if (taxReportingRecords.empty())
    return SharedSingleValue();

  SharedSingleValue reportingRecordRecord = std::make_shared<SingleValue>();

  tse::DaoDataFormatConverter::convert(taxReportingRecords, *reportingRecordRecord);

  return reportingRecordRecord;
}

SharedConstGroupValue
readAllRecords(DataHandle& handle, const GroupKey& key, const DateTime& ticketingDate)
{
  const std::vector<const TaxReportingRecordInfo*> taxReportingRecords =
      handle.getAllTaxReportingRecords(TaxCode(toTseTaxCode(key)));

  SharedGroupValue ans = std::make_shared<GroupValue>();
  for (const TaxReportingRecordInfo* recInfo : taxReportingRecords)
  {
    SharedSingleValue reportingRecordRecord = std::make_shared<SingleValue>();
    tse::DaoDataFormatConverter::convert(*recInfo, *reportingRecordRecord);
    ans->push_back(reportingRecordRecord);
  }
  return ans;
}
}

ReportingRecordServiceV2::ReportingRecordServiceV2(const DateTime& ticketingDT)
  : _ticketingDT(ticketingDT)
{
}

ReportingRecordServiceV2::~ReportingRecordServiceV2()
{
}

ReportingRecordServiceV2::SharedConstSingleValue
ReportingRecordServiceV2::getReportingRecord(const tax::type::Vendor& vendor,
                                             const tax::type::Nation& nation,
                                             const tax::type::CarrierCode& taxCarrierCode,
                                             const tax::type::TaxCode& taxCode,
                                             const tax::type::TaxType& taxType) const
{
  SingleKey key = std::make_tuple(vendor, nation, taxCarrierCode, taxCode, taxType);
  return SingleService::instance().get(key, _ticketingDT);
}

ReportingRecordServiceV2::SharedConstGroupValue
ReportingRecordServiceV2::getReportingRecords(const tax::type::TaxCode& taxCode) const
{
  return GroupService::instance().get(taxCode, _ticketingDT);
}

// Single Service
ReportingRecordServiceV2::SingleService&
ReportingRecordServiceV2::SingleService::instance()
{
  static SingleService instance;
  return instance;
}

ReportingRecordServiceV2::SingleService::SingleService()
{
  TaxReportingRecordDAO::instance().addListener(*this);
}

ReportingRecordServiceV2::SingleService::~SingleService()
{
  TaxReportingRecordDAO::instance().removeListener(*this);
}

void
ReportingRecordServiceV2::SingleService::keyRemoved(const TaxReportingRecordKey& key)
{
  SingleKey internalKey = std::make_tuple(toTaxVendorCode(key._a),
                                            toTaxNationCode(key._b),
                                            toTaxCarrierCode(key._c),
                                            toTaxCode(key._d),
                                            toTaxType(key._e));
  _cache.deleteKey(internalKey);
}

void
ReportingRecordServiceV2::SingleService::cacheCleared()
{
  _cache.clear();
}

ReportingRecordServiceV2::SharedConstSingleValue
ReportingRecordServiceV2::SingleService::get(const SingleKey& key, const DateTime& ticketingDate)
{
  DataHandle dataHandle(ticketingDate);
  return _cache.get(key, boost::bind(&readSingleRecord, boost::ref(dataHandle), _1, ticketingDate));
}

// Group Service
ReportingRecordServiceV2::GroupService&
ReportingRecordServiceV2::GroupService::instance()
{
  static GroupService instance;
  return instance;
}

ReportingRecordServiceV2::GroupService::GroupService()
{
  TaxReportingRecordByCodeDAO::instance().addListener(*this);
}

ReportingRecordServiceV2::GroupService::~GroupService()
{
  TaxReportingRecordByCodeDAO::instance().removeListener(*this);
}

void
ReportingRecordServiceV2::GroupService::keyRemoved(const TaxReportingRecordByCodeKey& key)
{
  _cache.deleteKey(toTaxCode(key._a));
}

void
ReportingRecordServiceV2::GroupService::cacheCleared()
{
  _cache.clear();
}

ReportingRecordServiceV2::SharedConstGroupValue
ReportingRecordServiceV2::GroupService::get(const GroupKey& key, const DateTime& ticketingDate)
{
  DataHandle dataHandle(ticketingDate);
  return _cache.get(key, boost::bind(&readAllRecords, boost::ref(dataHandle), _1, ticketingDate));
}

} // namespace tax
