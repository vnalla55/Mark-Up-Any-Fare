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

#include "DataModel/Common/SafeEnums.h"
#include "DataModel/Common/Types.h"
#include "DBAccess/DAOUtils.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/TaxRulesRecord.h"
#include "Rules/BusinessRulesContainer.h"
#include "Rules/TaxData.h"
#include "Taxes/AtpcoTaxes/Common/Timestamp.h"
#include "Taxes/AtpcoTaxes/DataModel/Common/Types.h"
#include "Taxes/AtpcoTaxes/DataModel/Services/RulesRecord.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "Taxes/LegacyFacades/DaoDataFormatConverter.h"
#include "Taxes/LegacyFacades/RulesRecordsServiceV2.h"

#include <functional>
#include <memory>

namespace tse
{

namespace
{

DateTime
convertDate(const tax::type::Timestamp& ticketingDate)
{
  return DateTime(ticketingDate.year(),
                  ticketingDate.month(),
                  ticketingDate.day(),
                  ticketingDate.hour(),
                  ticketingDate.min());
}

tax::type::Timestamp
convertDate(const DateTime& t)
{
  return {tax::type::Date{static_cast<int16_t>(t.year()),
                          static_cast<int16_t>(t.month()),
                          static_cast<int16_t>(t.day())},
          tax::type::Time{static_cast<int16_t>(t.hours()),
                          static_cast<int16_t>(t.minutes())}};
}

void
addRulesContainer(const tax::RulesRecord& rulesRecord,
                  const tax::type::ProcessingGroup processingGroup,
                  std::vector<std::shared_ptr<tax::BusinessRulesContainer>>& rulesContainerVector)
{
  std::shared_ptr<tax::BusinessRulesContainer> rulesContainer =
      std::make_shared<tax::BusinessRulesContainer>(rulesRecord, processingGroup);

  if (!rulesContainer->isValid())
    return;

  rulesContainerVector.push_back(rulesContainer);
}

RulesRecordsServiceV2::ByNationConstValue
readRulesRecords(DataHandle& dataHandle,
                 const TaxRulesRecordKey& searchedKey,
                 const DateTime& ticketingDT)
{
  std::vector<tax::RulesRecord> rules =
    RulesRecordsServiceV2::instance().getTaxRecords(toTaxNationCode(searchedKey._a),
                                                    tax::type::TaxPointTag(searchedKey._b),
                                                    convertDate(ticketingDT));

  RulesRecordsServiceV2::ByNationValue newData(
      new RulesRecordsServiceV2::RulesContainersGroupedByTaxName());

  if (rules.empty())
    return newData;

  newData->push_back(new tax::TaxData(rules.front().taxName, rules.front().vendor));

  static std::vector<tax::type::ProcessingGroup> allGroups{tax::type::ProcessingGroup::OC,
                                                           tax::type::ProcessingGroup::OB,
                                                           tax::type::ProcessingGroup::ChangeFee,
                                                           tax::type::ProcessingGroup::Itinerary,
                                                           tax::type::ProcessingGroup::Baggage};
  // Records will be ordered by nation, taxCode, taxType and seqNo
  for (const tax::RulesRecord& rulesRecord : rules)
  {
    if (!(newData->back().getTaxName() == rulesRecord.taxName))
      newData->push_back(new tax::TaxData(rulesRecord.taxName, rulesRecord.vendor));

    for (const tax::type::ProcessingGroup processingGroup : allGroups)
      addRulesContainer(rulesRecord, processingGroup, newData->back().get(processingGroup));
  }

  return newData;
}

RulesRecordsServiceV2::ByNationConstValue
readRulesRecordsHistorical(DataHandle& dataHandle,
                           const RulesRecordHistoricalKey& searchedKey,
                           const DateTime& ticketingDT)
{
  // DataHandle will choose proper (historical) data
  return readRulesRecords(dataHandle,
                          TaxRulesRecordKey(searchedKey._a, searchedKey._b),
                          ticketingDT);
}

std::shared_ptr<const std::vector<RulesRecordsServiceV2::SharedTaxRulesRecord>>
readRulesRecordsByCode(DataHandle& dataHandle,
                       const TaxRulesRecordByCodeKey& searchedKey,
                       const DateTime& ticketingDT)
{
  const std::vector<const TaxRulesRecord*>& tseRules =
      dataHandle.getTaxRulesRecordByCode(searchedKey._a, ticketingDT);

  std::shared_ptr<std::vector<RulesRecordsServiceV2::SharedTaxRulesRecord>> atpcoRules =
      std::make_shared<std::vector<RulesRecordsServiceV2::SharedTaxRulesRecord>>(tseRules.size());

  for (const TaxRulesRecord* tseRule : tseRules)
  {
    std::shared_ptr<tax::RulesRecord> atpcoRule = std::make_shared<tax::RulesRecord>();
    tse::DaoDataFormatConverter::convert(*tseRule, *atpcoRule);
    atpcoRules->push_back(atpcoRule);
  }

  return atpcoRules;
}

std::shared_ptr<const std::vector<RulesRecordsServiceV2::SharedTaxRulesRecord>>
readRulesRecordsByCodeHistorical(DataHandle& dataHandle,
                                 const RulesRecordByCodeHistoricalKey& searchedKey,
                                 const DateTime& ticketingDT)
{
  // DataHandle will choose proper (historical) data
  return readRulesRecordsByCode(dataHandle,
                                TaxRulesRecordByCodeKey(searchedKey._a),
                                ticketingDT);
}

RulesRecordsServiceV2::SharedTaxRulesRecord
getTaxRecordV2(tse::NationCode nation,
               tse::TaxCode taxCode,
               tse::TaxType taxType,
               int32_t seqNum,
               const tse::DateTime& ticketingDate)
{
  auto matchRec = [&](const TaxRulesRecord* rec)
  {
    return rec->taxCode() == taxCode &&
           rec->taxType() == taxType &&
           rec->seqNo() == seqNum &&
           rec->nation() == nation;
  };

  DataHandle dataHandle {ticketingDate};
  const std::vector<const TaxRulesRecord*>& records =
      dataHandle.getTaxRulesRecordByCodeAndType(taxCode, taxType, ticketingDate);

  auto it = std::find_if(records.begin(), records.end(), matchRec);
  if (it == records.end())
  {
    return {};
  }
  else
  {
    auto ans = std::make_shared<tax::RulesRecord>();
    tse::DaoDataFormatConverter::convert(**it, *ans);
    return ans;
  }
}

} // anonymous namespace


RulesRecordsServiceV2&
RulesRecordsServiceV2::instance()
{
  static RulesRecordsServiceV2 instance;
  return instance;
}

RulesRecordsServiceV2::ByNationConstValue
RulesRecordsServiceV2::getTaxRulesContainers(const tax::type::Nation& nation,
                                             const tax::type::TaxPointTag& taxPointTag,
                                             const tax::type::Timestamp& ticketingDate) const
{
  DateTime tseTicketingDate = convertDate(ticketingDate);
  DataHandle dataHandle(tseTicketingDate);

  if (dataHandle.isHistorical())
  {
    DateTime startDate, endDate;
    DAOUtils::getDateRange(tseTicketingDate,
                           startDate,
                           endDate,
                           TaxRulesRecordHistoricalDAO::instance().getCacheBy());

    RulesRecordHistoricalKey key(toTseNationCode(nation),
                                 static_cast<char>(taxPointTag),
                                 startDate,
                                 endDate);

    auto func = std::bind(&readRulesRecordsHistorical,
                          std::ref(dataHandle),
                          std::placeholders::_1,
                          tseTicketingDate);

    return RulesRecordsServiceV2::_byNationCacheHistorical.get(key, func);
  }
  else
  {
    auto func = std::bind(&readRulesRecords,
                          std::ref(dataHandle),
                          std::placeholders::_1,
                          tseTicketingDate);
    TaxRulesRecordKey key(toTseNationCode(nation), static_cast<char>(taxPointTag));

    return RulesRecordsServiceV2::_byNationCache.get(key, func);
  }
}

tax::RulesRecordsService::SharedTaxRulesRecord
RulesRecordsServiceV2:: getTaxRecord(tax::type::Nation nation,
                                     tax::type::TaxCode taxCode,
                                     tax::type::TaxType taxType,
                                     tax::type::Index seqNo,
                                     const tax::type::Timestamp& ticketingDate) const
{
  return getTaxRecordV2(toTseNationCode(nation),
                        toTseTaxCode(taxCode),
                        toTseTaxType(taxType),
                        seqNo,
                        convertDate(ticketingDate));
}

auto RulesRecordsServiceV2::getTaxRecords(const tax::type::Nation& nation,
                                          const tax::type::TaxPointTag& taxPointTag,
                                          const tax::type::Timestamp& ticketingDate) const
                                          -> std::vector<tax::RulesRecord>
{
  DateTime date = convertDate(ticketingDate);
  DataHandle dataHandle(date);
  const std::vector<const TaxRulesRecord*>& taxRulesRecords =
    dataHandle.getTaxRulesRecord(tse::toTseNationCode(nation),
                                 static_cast<tse::Indicator>(taxPointTag),
                                 date);
  std::vector<tax::RulesRecord> ans;
  ans.reserve(taxRulesRecords.size());
  for (const TaxRulesRecord* taxRulesRecord : taxRulesRecords)
  {
    ans.emplace_back();
    tse::DaoDataFormatConverter::convert(*taxRulesRecord, ans.back());
  }
  return ans;
}

std::shared_ptr<const std::vector<RulesRecordsServiceV2::SharedTaxRulesRecord>>
RulesRecordsServiceV2::getTaxRecords(const tax::type::TaxCode& taxCode,
                                     const tax::type::Timestamp& ticketingDate) const
{
  DateTime tseTicketingDate = convertDate(ticketingDate);
  DataHandle dataHandle(tseTicketingDate);

  if (dataHandle.isHistorical())
  {
    DateTime startDate, endDate;
    DAOUtils::getDateRange(tseTicketingDate,
                           startDate,
                           endDate,
                           TaxRulesRecordByCodeHistoricalDAO::instance().getCacheBy());

    RulesRecordByCodeHistoricalKey key(toTseTaxCode(taxCode),
                                       startDate,
                                       endDate);
    auto func = std::bind(&readRulesRecordsByCodeHistorical,
                          std::ref(dataHandle),
                          std::placeholders::_1,
                          tseTicketingDate);

    return RulesRecordsServiceV2::_byCodeCacheHistorical.get(key, func);
  }

  auto func = std::bind(&readRulesRecordsByCode,
                        std::ref(dataHandle),
                        std::placeholders::_1,
                        tseTicketingDate);
  TaxRulesRecordByCodeKey key(toTseTaxCode(taxCode));

  return RulesRecordsServiceV2::_byCodeCache.get(key, func);
}

} // namespace tse
