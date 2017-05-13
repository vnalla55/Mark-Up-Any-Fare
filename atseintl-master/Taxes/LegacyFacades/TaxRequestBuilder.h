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

#include "DataModel/Common/Types.h"
#include "ServiceFees/OCFees.h"
#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/InputPointOfSale.h"
#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/InputRequest.h"
#include "Taxes/Common/AtpcoTypes.h"
#include "Taxes/AtpcoTaxes/Common/AtpcoTaxesActivationStatus.h"
#include "Taxes/LegacyFacades/ServicesFeesMap.h"

#include <iosfwd>
#include <vector>

namespace tse
{
class PricingTrx;
class ShoppingTrx;
class Itin;
class TravelSeg;
class TaxItem;
class OCFees;
}

namespace tax
{
class InputRequest;
class InputGeo;
class InputGeoPath;
class InputGeoPathMapping;
class InputYqYrPath;
class InputItin;
class V2TrxMappingDetails;

namespace FareDisplayUtil
{
bool
checkTicketingAgentInformation(tse::PricingTrx& trx);
}

class TaxRequestBuilder_DEPRECATED final
{
  friend class TaxRequestBuilderTest;

public:
  typedef std::map<const tse::TravelSeg*, std::vector<tax::InputGeo*> > TravelSegGeoItems;
  typedef std::map<ItinFarePathKey, TravelSegGeoItems> InputGeoMap;
  typedef std::map<tax::type::Index, std::pair<tse::OCFees*, size_t> > OptionalServicesRefs;

  tax::InputRequest* buildInputRequest(tse::PricingTrx& trx,
                                       const ServicesFeesMap& servicesFees,
                                       V2TrxMappingDetails& v2MappingDetails,
                                       std::ostream* gsaDiag = nullptr) const;

  tax::InputRequest* buildInputRequest(tse::ShoppingTrx& trx,
                                       const ServicesFeesMap& servicesFees,
                                       V2TrxMappingDetails& v2MappingDetails) const;

private:
  class Impl;

  static void addYqYrServicesFees(const tax::ServicesFeesMapVal& items,
                                  tax::InputItin& itin,
                                  InputRequest& inputRequest,
                                  const std::vector<type::Index>& arunksBeforeSeg,
                                  const std::vector<type::Index>& hiddenBeforeSeg,
                                  const std::vector<tse::TravelSeg*>& travelSegs);

  static void processYqYrTaxItem(const tse::TaxItem& taxItem,
                                 InputRequest& inputRequest,
                                 tax::InputGeoPathMapping& geoPathMapping,
                                 tax::InputYqYrPath& yqYrPath,
                                 const std::vector<type::Index>& arunksBeforeSeg,
                                 const std::vector<type::Index>& hiddenBeforeSeg,
                                 const std::vector<tse::TravelSeg*>& travelSegs);

  static tax::type::Index addYqYr(const tse::TaxItem& taxItem, InputRequest::InputYqYrs& yqYrs);

  static void buildYqYrMappings(const tse::TaxItem& taxItem,
                                tax::InputGeoPathMapping& geoPathMapping,
                                const std::vector<type::Index>& arunksBeforeSeg,
                                const std::vector<type::Index>& hiddenBeforeSeg,
                                const std::vector<tse::TravelSeg*>& travelSegs);

  static void processApplicableGroups(tse::PricingTrx& pricingTrx,
                                      const AtpcoTaxesActivationStatus&,
                                      InputProcessingOptions&);

  static std::vector<type::Index>
  computeArunksBeforeSeg(const std::vector<tse::TravelSeg*>& segments);
  static std::vector<type::Index>
  computeHiddenBeforeSeg(const std::vector<tse::TravelSeg*>& segments);
};
}

