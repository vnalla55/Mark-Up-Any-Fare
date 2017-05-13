// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "TaxDisplay/ViewX2TaxReissueBuilder.h"

#include "DataModel/Common/Codes.h"
#include "DataModel/Common/Types.h"
#include "DataModel/Services/ReportingRecord.h"
#include "DataModel/Services/TaxReissue.h"
#include "ServiceInterfaces/Services.h"
#include "ServiceInterfaces/TaxReissueService.h"
#include "ServiceInterfaces/LocService.h"
#include "TaxDisplay/Common/TaxDisplayRequest.h"

#include <map>
#include <vector>

namespace tax
{
namespace display
{

std::map<type::Nation, ViewX2TaxReissue::ReissueViewData>
ViewX2TaxReissueBuilder::getData(
    const TaxDisplayRequest& request,
    const ReportingRecordsDataSet& reportingRecords)
{
  _hasGotAnyReissues = false;
  std::map<type::Nation, ViewX2TaxReissue::ReissueViewData> viewDataMap;
  for (const ReportingRecordService::SharedConstSingleValue& record : reportingRecords)
  {
    ViewX2TaxReissue::ReissueViewData& mapEntry = viewDataMap[record->nation];
    std::vector<ViewX2TaxReissue::ReissueTaxData>& reissueTaxDataVec =
        mapEntry.reissueTaxDataVec;

    const auto found = std::find_if(reissueTaxDataVec.begin(), reissueTaxDataVec.end(),
                                    [&record](const ViewX2TaxReissue::ReissueTaxData& reissueTaxData)
                                    {
                                      return record->taxCode == reissueTaxData.taxReissue->taxCode &&
                                             record->taxType == reissueTaxData.taxReissue->taxType;
                                    });
    if (found != reissueTaxDataVec.end())
      continue; // we already have this reissue and don't want to get it again from DB

    std::vector<std::shared_ptr<const TaxReissue>> taxReissues =
        _services.taxReissueService().getTaxReissues(record->taxCode,
                                                     request.requestDate);


    reissueTaxDataVec.reserve(reissueTaxDataVec.size() + taxReissues.size());
    for (const auto& taxReissue : taxReissues)
    {
      if (taxReissue->taxType != record->taxType)
        continue;

      assert(!record->entries.empty());
      reissueTaxDataVec.push_back({taxReissue, record->entries.front().taxLabel});
      _hasGotAnyReissues = true;
    }
    reissueTaxDataVec.shrink_to_fit();

    if (mapEntry.nationName.empty())
      mapEntry.nationName = _services.locService().getNationName(record->nation);
  }

  return viewDataMap;
}

} // namespace tax
} // namespace display
