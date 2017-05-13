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
#include <map>
#include <vector>

#include "Common/DateTime.h"
#include "Common/FallbackUtil.h"
#include "Common/Money.h"
#include "Common/MoneyUtil.h"
#include "Common/ServiceFeeUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "DataModel/AirSeg.h"
#include "DataModel/BaggageCharge.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/RequestResponse/InputGeoPathMapping.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/TaxSpecConfigReg.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/InputRequest.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "Taxes/LegacyFacades/OptionalServicesBuilder.h"

namespace tse
{
FALLBACK_DECL(ATPCO_TAX_AcceptOCTagPrePaid);
FALLBACK_DECL(monetaryDiscountTaxesForAllSegments);
FALLBACK_DECL(monetaryDiscountFlatTaxesApplication);
}

namespace tax
{
namespace
{

static const tse::Indicator FREE_SERVICE_F = 'F';
static const tse::Indicator FREE_SERVICE_G = 'G';

static const tse::Indicator TAXINCLIND = 'X';
static const tse::Indicator TAXEXEMPTIND = 'Y';

typedef std::map<const tse::TravelSeg*, std::vector<tax::InputGeo*> > TravelSegGeoItems;

class GeoAppender
{
  InputMapping& _mapping;
  TravelSegGeoItems& _seg2Geos;

public:
  GeoAppender(InputMapping& mapping, TravelSegGeoItems& seg2Geos)
    : _mapping(mapping), _seg2Geos(seg2Geos) {}

  void operator()(tse::TravelSeg* seg) const
  {
    std::vector<tax::InputGeo*> const& inputGeos = _seg2Geos[seg];

    for(const InputGeo* inputGeo : inputGeos)
    {
      InputMap* map = new InputMap();
      map->_geoRefId = inputGeo->_id;
      _mapping.maps().push_back(map);
    }
  }
};

void
applyBetween(const tse::TravelSeg* start, const tse::TravelSeg* end,
             const std::vector<tse::TravelSeg*>& segments, GeoAppender geoAppender)
{
  std::vector<tse::TravelSeg*>::const_iterator it =
    std::find(segments.begin(), segments.end(), start);

  for(; it != segments.end(); ++it)
  {
    geoAppender(*it);

    if (*it == end)
      break;
  }
}

} // anonymous namespace

log4cxx::LoggerPtr
OptionalServicesBuilder::_logger(
    log4cxx::Logger::getLogger("atseintl.AtpcoTaxes.OptionalServicesBuilder"));

const std::string OptionalServicesBuilder::ATPOCINC_NAME = "ATPOCINC";

OptionalServicesBuilder::OptionalServicesBuilder(tse::PricingTrx& trx,
                                                 InputRequest& inputRequest,
                                                 tax::InputItin& itin,
                                                 TravelSegGeoItems& items,
                                                 const tse::FarePath& tseFarePath,
                                                 OptionalServicesRefs& optionalServicesMapping,
                                                 std::map<tax::type::Index, std::pair<tse::OCFees*, size_t> >& ocTaxInclIndMap)
  : _trx(trx),
    _inputRequest(inputRequest),
    _itin(itin),
    _items(items),
    _tseFarePath(tseFarePath),
    _optionalServicesMapping(optionalServicesMapping),
    _ocTaxInclIndMap(ocTaxInclIndMap),
    _geoPathMappingsAdded(false)
{
}

bool
OptionalServicesBuilder::checkTaxInclInd(const tse::OCFees& ocFees) const
{
  if (ocFees.optFee()->taxInclInd() != TAXINCLIND)
    return false;

  return !tse::TrxUtil::isBOTAllowed(_trx);
}

OptionalServicesBuilder::~OptionalServicesBuilder() {}

void
OptionalServicesBuilder::addServiceFeesGroup(tse::ServiceFeesGroup& group)
{
  std::map<const tse::FarePath*, std::vector<tse::OCFees*> >& ocFeesMap = group.ocFeesMap();

  for(tse::OCFees * ocFees : ocFeesMap[&_tseFarePath])
  {
    tse::OCFees::Memento memento = ocFees->saveToMemento();

    for(size_t segIndex = 0; segIndex < ocFees->segCount(); ++segIndex)
    {
      ocFees->setSeg(segIndex);

      if (ocFees->subCodeInfo()->fltTktMerchInd() == tse::BAGGAGE_CHARGE ||
          ocFees->subCodeInfo()->fltTktMerchInd() == tse::BAGGAGE_ALLOWANCE ||
          ocFees->subCodeInfo()->fltTktMerchInd() == tse::BAGGAGE_EMBARGO)
        continue;

      if (checkTaxInclInd(*ocFees) || ocFees->optFee()->taxExemptInd() == TAXEXEMPTIND)
        continue;

      if (ocFees->optFee()->notAvailNoChargeInd() == FREE_SERVICE_F ||
          ocFees->optFee()->notAvailNoChargeInd() == FREE_SERVICE_G)
        continue;

      int index = addOCFees(*ocFees, segIndex);

      if (!checkTaxInclInd(*ocFees))
        addToTaxInclIndMap(*ocFees, segIndex, index);
    }

    ocFees->restoreFromMemento(memento);
  }
}

void
OptionalServicesBuilder::addOptionalServices()
{
  for(tse::ServiceFeesGroup * serviceFeesGroup : _tseFarePath.itin()->ocFeesGroup())
  {
    addServiceFeesGroup(*serviceFeesGroup);
  }
}


void
OptionalServicesBuilder::addBaggage()
{
  for(const tse::BaggageTravel* baggageTravel : _tseFarePath.baggageTravels())
  {
    for (tse::BaggageCharge* bc : baggageTravel->_charges)
      if (UNLIKELY(bc))
        addOCFees(*bc);

    for(tse::BaggageCharge* bc : baggageTravel->_chargeVector)
      addOCFees(*bc);
  }

  for(const tse::BaggageTravel* baggageTravel : _tseFarePath.baggageTravelsPerSector())
  {
    for (tse::BaggageCharge* bc : baggageTravel->_chargeVector)
      addOCFees(*bc);
  }
}

void
OptionalServicesBuilder::addGeoPathMappings()
{
  _inputRequest.optionalServicePaths().push_back(new InputOptionalServicePath());
  tax::type::Index optionalServicePathId = _inputRequest.optionalServicePaths().size() - 1;
  _inputRequest.optionalServicePaths().back()._id = optionalServicePathId;
  _itin._optionalServicePathRefId = optionalServicePathId;

  _inputRequest.geoPathMappings().push_back(new InputGeoPathMapping());
  tax::type::Index getPathMappingId = _inputRequest.geoPathMappings().size() - 1;
  _inputRequest.geoPathMappings().back()._id = getPathMappingId;
  _itin._optionalServicePathGeoPathMappingRefId = getPathMappingId;

  _geoPathMappingsAdded = true;
}

void
OptionalServicesBuilder::addToTaxInclIndMap(tse::OCFees& ocFees, int index)
{
  addToTaxInclIndMap(ocFees, 0, index);
}

void
OptionalServicesBuilder::addToTaxInclIndMap(tse::OCFees& ocFees, size_t segIndex, int index)
{
  if (ocFees.optFee()->taxInclInd() == TAXINCLIND)
    _ocTaxInclIndMap[index] = std::make_pair(&ocFees, segIndex);
}

int
OptionalServicesBuilder::addOCFees(tse::OCFees& ocFees, size_t segIndex)
{
  if (!_geoPathMappingsAdded)
    this->addGeoPathMappings();

  InputOptionalService* optionalService = new InputOptionalService();
  optionalService->_subCode = tse::toTaxOcSubCode(ocFees.subCodeInfo()->serviceSubTypeCode());
  optionalService->_serviceGroup = ocFees.subCodeInfo()->serviceGroup();
  optionalService->_serviceSubGroup = ocFees.subCodeInfo()->serviceSubGroup();
  optionalService->_type = getOptionalServiceTag(ocFees.subCodeInfo()->fltTktMerchInd());
  optionalService->_ownerCarrier = tse::toTaxCarrierCode(ocFees.carrierCode());

  if (ocFees.getSegPtr(segIndex)->_ancPriceModification)
    optionalService->_quantity = ocFees.getSegPtr(segIndex)->_ancPriceModification.get().second._quantity;

  //the logic is taken from AncillaryPricingResponseFormatter.cpp
  const bool isFlightRelatedService = ocFees.subCodeInfo()->fltTktMerchInd() == tse::FLIGHT_RELATED_SERVICE;
  const bool isPrepaidBaggage = ocFees.subCodeInfo()->fltTktMerchInd() == tse::PREPAID_BAGGAGE;
  const bool isBaggageRequest = _trx.getRequest()->majorSchemaVersion() >= 2 &&
        tse::ServiceFeeUtil(_trx).checkServiceGroupForAcs(ocFees.subCodeInfo()->serviceGroup());

  if(_trx.activationFlags().isAB240() && (isPrepaidBaggage || isFlightRelatedService))
  {
    optionalService->_amount = tax::doubleToAmount(tse::ServiceFeeUtil(_trx)
                                                   .convertOCFeeCurrency(&ocFees)
                                                   .value());
  }
  else if (isBaggageRequest)
  {
    optionalService->_amount = tax::doubleToAmount(tse::ServiceFeeUtil(_trx)
                                                   .convertBaggageFeeCurrency(&ocFees)
                                                   .value());
  }
  else
  {
    optionalService->_amount = tax::doubleToAmount(tse::ServiceFeeUtil(_trx)
                                                   .convertOCFeeCurrency(&ocFees)
                                                   .value());
  }

  optionalService->_taxInclInd = (ocFees.optFee()->taxInclInd() == TAXINCLIND);

  if(optionalService->_taxInclInd)
    optionalService->_feeAmountInSellingCurrencyPlusTaxes = optionalService->_amount;

  _inputRequest.optionalServices().push_back(optionalService);
  optionalService->_id = _inputRequest.optionalServices().size() - 1;

  _optionalServicesMapping[optionalService->_id] = std::make_pair(&ocFees, segIndex);

  InputOptionalServiceUsage* optionalServiceUsage = new InputOptionalServiceUsage();
  _inputRequest.optionalServicePaths().back()._optionalServiceUsages.push_back(
      optionalServiceUsage);
  optionalServiceUsage->_optionalServiceRefId = optionalService->_id;

  buildMappings(ocFees);

  return optionalService->_id;
}

void
OptionalServicesBuilder::addOCFees(tse::OCFees& ocFees)
{
  if (!tse::fallback::monetaryDiscountTaxesForAllSegments(&_trx))
  {
    tse::OCFees::Memento memento = ocFees.saveToMemento();

    for(size_t segIndex = 0; segIndex < ocFees.segCount(); ++segIndex)
    {
      ocFees.setSeg(segIndex);

      if (_ocFeesSegsChecker.shouldAdd(ocFees.getCurrentSeg()))
      {
        int index = addOCFees(ocFees, segIndex);
        if (!checkTaxInclInd(ocFees))
          addToTaxInclIndMap(ocFees, segIndex, index);
      }
    }

    ocFees.restoreFromMemento(memento);
  }
  else
  {
    if (_ocFeesChecker.shouldAdd(&ocFees))
    {
      int index = addOCFees(ocFees, 0);
      if (!checkTaxInclInd(ocFees))
        addToTaxInclIndMap(ocFees, 0, index);
    }
  }
}

type::OptionalServiceTag
OptionalServicesBuilder::getOptionalServiceTag(const tse::Indicator& fltTktMerchInd) const
{
  switch (fltTktMerchInd)
  {
  case 'P':
    //if(!_fallbackService.isSet(tse::fallback::ATPCO_TAX_AcceptOCTagPrePaid))
    if (!tse::fallback::ATPCO_TAX_AcceptOCTagPrePaid(&_trx))
    {
      return type::OptionalServiceTag::PrePaid;
    }
    // fallthrough

  case 'F':
    return type::OptionalServiceTag::FlightRelated;

  case 'T':
    return type::OptionalServiceTag::TicketRelated;

  case 'M':
    return type::OptionalServiceTag::Merchandise;

  case 'R':
    return type::OptionalServiceTag::FareRelated;

  case 'C':
    return type::OptionalServiceTag::BaggageCharge;

  default:
    return type::OptionalServiceTag::Blank;
  }
}

void
OptionalServicesBuilder::buildMappings(const tse::OCFees& ocFees)
{
  std::unique_ptr<InputMapping> mapping(new InputMapping());

  tse::Itin const* itin = _tseFarePath.itin();
  assert(itin);
  const std::vector<tse::TravelSeg*>& segments = itin->travelSeg();

  GeoAppender geoAppender(*mapping, _items);
  applyBetween(ocFees.travelStart(), ocFees.travelEnd(), segments, geoAppender);

  TSE_ASSERT(mapping->maps().size());

  _inputRequest.geoPathMappings().back()._mappings.push_back(mapping.release());
}
} // end of namespace tax
