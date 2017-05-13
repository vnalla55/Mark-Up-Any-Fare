// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include <vector>

#include "Common/OCUtil.h"
#include "DomainDataObjects/Request.h"
#include "Processor/RawSubjectsCollector.h"
#include "ServiceInterfaces/Services.h"
#include "Util/BranchPrediction.h"

namespace tax
{

RawSubjectsCollector::RawSubjectsCollector(const type::ProcessingGroup& processingGroup,
                                           const Request& request,
                                           const type::Index itinId,
                                           const Geo& taxPoint,
                                           const Geo& nextPrevTaxPoint)
{
  switch (processingGroup)
  {
  case type::ProcessingGroup::Itinerary:
    addYqYrs(request, itinId, taxPoint, nextPrevTaxPoint, true);
    break;

  case type::ProcessingGroup::OC:
  case type::ProcessingGroup::Baggage:
    addOptionalServices(
        request, itinId, taxPoint.loc().tag(), processingGroup, taxPoint.id());
    break;

  case type::ProcessingGroup::ChangeFee:
    addChangeFee(request, itinId);
    break;

  case type::ProcessingGroup::OB:
    addTicketingFee(request, itinId);
    break;

  default:
    break;
  }
}

void
RawSubjectsCollector::addChangeFee(const Request& request, const type::Index& itinId)
{
  const CompactOptional<type::Index>& changeFeeRefId = request.getItinByIndex(itinId).changeFeeRefId();
  if (changeFeeRefId.has_value())
  {
    _subjects._itinerary.changeFeeAmount() =
        request.changeFees()[changeFeeRefId.value()].amount();
  }
}

void
RawSubjectsCollector::addYqYrs(const Request& request,
                               const type::Index itinId,
                               const Geo& taxPoint,
                               const Geo& nextPrevTaxPoint,
                               bool onAllBaseFare /* = false */)
{
  const Itin& itin = request.getItinByIndex(itinId);
  CompactOptional<type::Index> yqyrGeoIndex = itin.yqYrPathGeoPathMappingRefId();

  if (!yqyrGeoIndex.has_value())
    return;

  if (UNLIKELY(!itin.yqYrPath()))
    return;

  assert (yqyrGeoIndex.value() < request.geoPathMappings().size());

  const bool noRangeCheck = onAllBaseFare || (taxPoint.loc().tag() == type::TaxPointTag::Sale);
  const GeoPathMapping& yqYrGeoPathMapping = request.geoPathMappings()[yqyrGeoIndex.value()];
  const YqYrPath& yqYrPath = *itin.yqYrPath();
  const std::vector<YqYr>& yqYrs = request.yqYrs();
  const type::Index yqYrsSize = yqYrPath.yqYrUsages().size();

  const type::Index taxPointId = taxPoint.id();
  std::vector<TaxableYqYr> taxableYqYrVector;
  std::vector<std::pair<type::Index, type::Index> > ranges;
  std::vector<type::Index> usageIds;
  taxableYqYrVector.reserve(yqYrsSize);
  ranges.reserve(yqYrsSize);
  usageIds.reserve(yqYrsSize);
  for (type::Index i = 0; i < yqYrsSize; ++i)
  {
    const Mapping& mapping = yqYrGeoPathMapping.mappings()[i];
    std::pair<type::Index, type::Index> range =
        std::make_pair(mapping.maps().front().index(), mapping.maps().back().index());

    if (UNLIKELY(!noRangeCheck && (taxPointId < range.first || taxPointId > range.second)))
      continue;

    ranges.push_back(range);
    usageIds.push_back(i);
    taxableYqYrVector.push_back(TaxableYqYr(yqYrs[yqYrPath.yqYrUsages()[i].index()]));
  }
  TaxableYqYrs& taxableYrYrs = _subjects._yqyrs;
  taxableYrYrs.init(taxableYqYrVector, usageIds);
  for (TaxableData& data : taxableYrYrs._data)
  {
    data._taxPointLoc2 = &nextPrevTaxPoint;
    data._taxPointEnd = &nextPrevTaxPoint;
  }
  taxableYrYrs._ranges.swap(ranges);
}

void
RawSubjectsCollector::addOptionalServices(const Request& request,
                                          const type::Index itinId,
                                          const type::TaxPointTag& tag,
                                          const type::ProcessingGroup& processingGroup,
                                          const type::Index taxPointBeginId)
{
  Itin const& itin = request.getItinByIndex(itinId);
  CompactOptional<type::Index> ocIndex = itin.optionalServicePathRefId();
  CompactOptional<type::Index> ocGeoIndex = itin.optionalServicePathGeoPathMappingRefId();

  if (!ocIndex.has_value() || !ocGeoIndex.has_value())
    return;

  assert (ocIndex.value() < request.optionalServicePaths().size());
  assert (ocGeoIndex.value() < request.geoPathMappings().size());

  const OptionalServicePath& optionalServicePath = request.optionalServicePaths()[ocIndex.value()];
  const GeoPathMapping& geoPathMapping = request.geoPathMappings()[ocGeoIndex.value()];
  const GeoPath& geoPath = request.geoPaths()[itin.geoPathRefId()];

  bool reverseDirection = (tag == type::TaxPointTag::Arrival);
  type::Index mappingIndex = 0;

  boost::ptr_vector<OptionalService>& optionalServiceItems =
      _subjects._itinerary.optionalServiceItems();
  for (Mapping const& mapping : geoPathMapping.mappings())
  {
    type::Index osBegin =
        reverseDirection ? mapping.maps().back().index() : mapping.maps().front().index();
    type::Index osEnd =
        reverseDirection ? mapping.maps().front().index() : mapping.maps().back().index();
    if (tag == type::TaxPointTag::Sale || osBegin == taxPointBeginId)
    {
      OptionalService const& optionalService =
          request.optionalServices()
              [optionalServicePath.optionalServiceUsages()[mappingIndex].index()];

      if (!OCUtil::isOCValidForGroup(optionalService.type(), processingGroup))
      {
        ++mappingIndex;
        continue;
      }

      optionalServiceItems.push_back(new OptionalService(optionalService));
      optionalServiceItems.back().setTaxPointBegin(geoPath.geos()[osBegin]);
      optionalServiceItems.back().setTaxPointEnd(geoPath.geos()[osEnd]);
      optionalServiceItems.back().setTaxPointLoc2(geoPath.geos()[osEnd]);
    }

    ++mappingIndex;
  }
}

void
RawSubjectsCollector::addTicketingFee(const Request& request, const type::Index& itinId)
{
  const Itin& itin = request.getItinByIndex(itinId);
  _subjects._itinerary.ticketingFees() = itin.ticketingFees();
}
}
