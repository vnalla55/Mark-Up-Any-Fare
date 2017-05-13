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
#include "AtpcoTaxes/Factories/MakeSabreCode.h"
#include "AtpcoTaxes/DataModel/Common/CompactOptional.h"
#include "AtpcoTaxes/DataModel/RequestResponse/OutputTaxDetails.h"
#include "AtpcoTaxes/DataModel/Services/RulesRecord.h"
#include "AtpcoTaxes/ServiceInterfaces/RulesRecordsService.h"
#include "AtpcoTaxes/ServiceInterfaces/ServiceBaggageService.h"
#include "LegacyFacades/TaxDependencies.h"

#include <cassert>
#include <utility>

namespace tse
{

namespace
{

using OptIndex = tax::CompactOptional<tax::type::Index>;
using SABTableId = std::pair<OptIndex, tax::type::Vendor>;

bool viableX1rec(const tax::RulesRecord& x1Rec)
{
  return x1Rec.serviceBaggageItemNo != 0 &&  // zero indicates no records in S|B table
         x1Rec.serviceBaggageApplTag == tax::type::ServiceBaggageApplTag::E;
}

bool viableSABEntry(const tax::ServiceBaggageEntry& entry)
{
  return entry.taxCode != "OC" &&                                  // filter out OC taxes
         entry.applTag == tax::type::ServiceBaggageAppl::Positive; // skip negatives
}

tax::type::PercentFlatTag taxKindForUS(tax::type::TaxType type)
{
  if (type == "002" || type == "005" || type == "006" || type == "010" || type == "011")
    return tax::type::PercentFlatTag::Flat;
  else
    return tax::type::PercentFlatTag::Percent;
}

std::string makeTaxCode(const tax::ServiceBaggageEntry& entry)
{
  if (entry.taxTypeSubcode.empty())
    return entry.taxCode.asString() + '*';

  if (entry.taxCode == "YQ" || entry.taxCode == "YR")
    return entry.taxCode.asString() + entry.taxTypeSubcode.back();

  if (entry.taxTypeSubcode.length() != 3)
    return {};

  tax::type::TaxType taxType;
  codeFromString(entry.taxTypeSubcode, taxType);
  return tax::makeItinSabreCode(entry.taxCode, taxType, taxKindForUS(taxType));
}

SABTableId
getSABTableIndex(const tax::OutputTaxDetails& taxDetail,
                 const tax::RulesRecordsService& x1Service, const tax::type::Timestamp& ticketingDate)
{
  tax::RulesRecordsService::SharedTaxRulesRecord recPtr =
    x1Service.getTaxRecord(taxDetail.nation(), taxDetail.code(), taxDetail.type(),
                           taxDetail.seqNo(), ticketingDate);

  if (recPtr && viableX1rec(*recPtr))
    return std::make_pair(OptIndex{recPtr->serviceBaggageItemNo}, recPtr->vendor);
  else
    return {};
}

std::vector<std::string>
getSabreTaxCodes(const SABTableId& id, const tax::ServiceBaggageService& svcBagService)
{
  std::vector<std::string> ans;

  if (id.first.has_value())
  {
    assert (id.first.value() != 0);
    std::shared_ptr<const tax::ServiceBaggage> sbRecs =
        svcBagService.getServiceBaggage(id.second, id.first.value());

    if (sbRecs)
    {
      for (const tax::ServiceBaggageEntry& entry : sbRecs->entries)
      {
        if (viableSABEntry(entry))
        {
          ans.push_back(makeTaxCode(entry));
        }
      }
    }
  }

  return ans;
}

} // anonymous namespace

std::vector<std::string>
TaxDependencies::taxDependencies(const tax::OutputTaxDetails& taxDetail) const
{
  std::vector<std::string> ans;
  SABTableId id = getSABTableIndex(taxDetail, _x1Service, _ticketingDate);
  return getSabreTaxCodes(id, _svcBagService);
}

} // namespace tse
