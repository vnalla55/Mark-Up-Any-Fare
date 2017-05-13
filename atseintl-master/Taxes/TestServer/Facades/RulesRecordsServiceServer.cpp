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
#include "RulesRecordsServiceServer.h"


#include "DataModel/Services/RulesRecord.h"
#include "Rules/BusinessRulesContainer.h"
#include "Rules/TaxData.h"


#include <algorithm>

namespace tax
{
namespace
{
typedef std::map<RulesRecordsServiceServer::ByNationKey, RulesRecordsServiceServer::ByNationValue>
Map;
RulesRecordsServiceServer::ByNationValue
get(Map& map, const RulesRecordsServiceServer::ByNationKey& key)
{
  Map::iterator iter = map.find(key);
  if (iter == map.end())
  {
    RulesRecordsServiceServer::ByNationValue ans =
        std::make_shared<RulesRecordsServiceServer::RulesContainersGroupedByTaxName>();
    map.insert(std::make_pair(key, ans));
    return ans;
  }
  else
  {
    return iter->second;
  }
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

} // anonymous namespace

RulesRecordsServiceServer::RulesRecordsServiceServer()
{
}

RulesRecordsServiceServer::~RulesRecordsServiceServer()
{
}

RulesRecordsServiceServer::ByNationConstValue
RulesRecordsServiceServer::getTaxRulesContainers(const type::Nation& nation,
                                                 const type::TaxPointTag& taxPointTag,
                                                 const type::Timestamp&) const
{
  return get(_rulesContainersMap, ByNationKey(nation, taxPointTag));
}

void
RulesRecordsServiceServer::updateKeys()
{
  static std::vector<tax::type::ProcessingGroup> allGroups{tax::type::ProcessingGroup::OC,
                                                           tax::type::ProcessingGroup::OB,
                                                           tax::type::ProcessingGroup::ChangeFee,
                                                           tax::type::ProcessingGroup::Itinerary,
                                                           tax::type::ProcessingGroup::Baggage};

  for (const RulesRecord& rulesRecord : _rulesRecords)
  {
    TaxName const& taxName = rulesRecord.taxName;
    type::Nation const& nation = taxName.nation();
    type::TaxPointTag const& taxPointTag = taxName.taxPointTag();
    ByNationKey searchedKey = std::make_pair(nation, taxPointTag);

    ByNationValue searchedValue = get(_rulesContainersMap, searchedKey);
    TaxData* currentTaxData = 0;
    for (TaxData& taxData : *searchedValue)
    {
      if (taxData.getTaxName() == taxName)
      {
        currentTaxData = &taxData;
        break;
      }
    }
    if (!currentTaxData)
    {
      searchedValue->push_back(new TaxData(taxName, rulesRecord.vendor));
      currentTaxData = &searchedValue->back();
    }

    for (const tax::type::ProcessingGroup processingGroup : allGroups)
      addRulesContainer(rulesRecord, processingGroup, currentTaxData->get(processingGroup));
  }
}

RulesRecordsService::SharedTaxRulesRecord
RulesRecordsServiceServer::getTaxRecord(type::Nation nation,
                                        type::TaxCode code,
                                        type::TaxType type,
                                        type::Index seqNo,
                                        const type::Timestamp&) const
{
  auto matchRec = [&](const RulesRecord& rec)
  {
    return rec.taxName.nation() == nation &&
           rec.taxName.taxType() == type &&
           rec.taxName.taxCode() == code &&
           rec.seqNo == seqNo;
  };

  auto it = std::find_if(_rulesRecords.begin(), _rulesRecords.end(), matchRec);

  if (it != _rulesRecords.end())
    return std::make_shared<RulesRecord>(*it);
  else
    return {};
}

auto RulesRecordsServiceServer::getTaxRecords(const type::Nation&,
                                              const type::TaxPointTag&,
                                              const type::Timestamp&) const
                                              -> std::vector<RulesRecord>
{
  std::vector<RulesRecord> ans(_rulesRecords.begin(), _rulesRecords.end());
  return ans;
}

}

