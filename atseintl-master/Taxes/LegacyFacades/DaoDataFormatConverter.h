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
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "Taxes/AtpcoTaxes/Common/Timestamp.h"
#include "Taxes/AtpcoTaxes/DataModel/Services/PassengerTypeCode.h"

#include <cstdint>
#include <set>
#include <vector>

namespace tax
{
class CarrierApplication;
class CarrierFlight;
class LocZone;
class PassengerTypeCodeItem;
class ServiceFeeSecurityItem;
class ReportingRecord;
class ReportingRecordEntry;
class RulesRecord;
class RulesRecord;
class SectorDetail;
class SectorDetailEntry;
class ServiceBaggage;
class ServiceBaggageEntry;
class TaxReissue;
}

namespace tse
{
class TaxCarrierAppl;
class TaxCarrierFlightInfo;
class PaxTypeCodeInfo;
class SectorDetailInfo;
class ServiceBaggageInfo;
class TaxReportingRecordInfo;
class TaxRulesRecord;
class SvcFeesSecurityInfo;
class TaxReissue;

class DaoDataFormatConverter
{
public:
  static void convert(const tse::TaxRulesRecord& trr, tax::RulesRecord& rr);

  static void convert(const tse::TaxCarrierAppl& from, tax::CarrierApplication& to);

  static void convert(const tse::TaxCarrierFlightInfo& v2cf, tax::CarrierFlight& cf);

  static void convert(const tse::PaxTypeCodeInfo& v2pax, tax::PassengerTypeCodeItem& pas);

  static void convert(const tse::ServiceBaggageInfo& v2Entry, tax::ServiceBaggageEntry& taxEntry);

  static void convert(const tse::SectorDetailInfo& v2Entry, tax::SectorDetailEntry& taxEntry);

  static void
  convert(const tse::TaxReportingRecordInfo& v2Entry, tax::ReportingRecordEntry& taxEntry);

  static void convert(const std::vector<const tse::TaxReportingRecordInfo*>& v2Data,
                      tax::ReportingRecord& taxData);

  static void convert(const tse::TaxReportingRecordInfo& v2Data, tax::ReportingRecord& taxData);

  static void
  convert(const tse::SvcFeesSecurityInfo& v2Entry, tax::ServiceFeeSecurityItem& taxEntry);

  static void
  convert(const tse::TaxReissue& v2Reissue, tax::TaxReissue& taxReissue);

  template <class V, class T>
  static void convert(const std::vector<V*>& v2Data, T& taxData)
  {
    if (v2Data.empty())
      return;

    taxData.vendor = toTaxVendorCode(v2Data.front()->vendor());
    taxData.itemNo = v2Data.front()->itemNo();

    convertEntries(v2Data, taxData);
  }

  template <class V, class T>
  static void convertEntries(const std::vector<V*>& v2Data, T& taxData)
  {
    for (V * elem: v2Data)
    {
      if (nullptr != elem)
      {
        taxData.entries.push_back(new typename T::entry_type);
        DaoDataFormatConverter::convert(*elem, taxData.entries.back());
      }
    }
  }

  static tax::type::Date fromDate(const tse::DateTime& tseDT, bool isUpperLimit = false);

  static tax::type::Timestamp fromTimestamp(const tse::DateTime& tseDT, bool isUpperLimit = false);

  static void
  setLocZone(tax::LocZone& locZone, tse::LocTypeCode type, uint32_t zone, tse::LocCode locCode);

private:
  static void addConnectionsTag(std::set<tax::type::ConnectionsTag>& tagSet, char tag);
};

} // namespace tse

