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

#include "Common/Timestamp.h"
#include "Common/SafeEnumToString.h"
#include "DataModel/Common/CodeIO.h"
#include "DomainDataObjects/Request.h"
#include "ServiceInterfaces/MileageGetter.h"
#include "ServiceInterfaces/MileageService.h"
#include "ServiceInterfaces/SectorDetailService.h"
#include "ServiceInterfaces/Services.h"
#include "Rules/RuleDescriptionFormater.h"
#include "Rules/TaxOnFareRule.h"
#include "Rules/TaxOnFareApplicator.h"

namespace tax
{

TaxOnFareRule::TaxOnFareRule(type::NetRemitApplTag const& netRemitApplTag,
                             type::TaxAppliesToTagInd const& taxAppliesToTagInd,
                             type::Index const& itemNo,
                             type::Vendor const& vendor,
                             type::TaxProcessingApplTag processingApplTag)
  : _netRemitApplTag(netRemitApplTag),
    _taxAppliesToTagInd(taxAppliesToTagInd),
    _itemNo(itemNo),
    _vendor(vendor),
    _processingApplTag(processingApplTag)
{
}

TaxOnFareRule::~TaxOnFareRule() {}

TaxOnFareRule::ApplicatorType
TaxOnFareRule::createApplicator(type::Index const& itinIndex,
                                const Request& request,
                                Services& services,
                                RawPayments& /*itinPayments*/) const
{
  Itin const& itin = request.getItinByIndex(itinIndex);
  assert(itin.geoPathMapping());
  std::shared_ptr<SectorDetail const> sectorDetail =
      services.sectorDetailService().getSectorDetail(_vendor, _itemNo);
  const type::Timestamp travelDate(itin.travelOriginDate(), type::Time(0, 0));
  const MileageGetter& mileageGetter = services.mileageService().getMileageGetter(
      *itin.geoPath(), itin.flightUsages(), travelDate);

  return ApplicatorType(*this,
                        services,
                        sectorDetail,
                        request.fares(),
                        *itin.farePath(),
                        request.flights(),
                        itin.flightUsages(),
                        *itin.geoPath(),
                        *itin.geoPathMapping(),
                        itin.id(),
                        request.processing().usingRepricing(),
                        request.processing().isRtw(),
                        mileageGetter);
}

std::string
TaxOnFareRule::getDescription(Services& services) const
{
  std::ostringstream buf;
  buf << "CALCULATE TAX ON FARES IN ACCORD WITH TAXAPPLIESTOTAGIND=" << _taxAppliesToTagInd << "\n";
  if (_taxAppliesToTagInd == type::TaxAppliesToTagInd::MatchingSectorDetail)
  {
    buf << " AND RESTRICTIONS FROM SECTOR DETAIL ITEM NUMBER " << _itemNo << " FROM\n"
        << " VENDOR " << _vendor << "\n"
        << " ITEM " << _itemNo;
    RuleDescriptionFormater::format(
        buf, services.sectorDetailService().getSectorDetail(_vendor, _itemNo));
  }
  return buf.str();
}
}
