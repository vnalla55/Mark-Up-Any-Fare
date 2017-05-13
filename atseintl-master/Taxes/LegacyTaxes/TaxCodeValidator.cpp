// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
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

#include "Common/FallbackUtil.h"
#include "Common/LocUtil.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseEnums.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/BaseExchangeTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Customer.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxRestrictionLocationInfo.h"
#include "DBAccess/TaxRestrictionPsg.h"
#include "Taxes/Common/TaxUtility.h"
#include "Taxes/LegacyFacades/ItinSelector.h"
#include "Taxes/LegacyTaxes/GetTicketingDate.h"
#include "Taxes/LegacyTaxes/TaxCodeValidator.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Util/BranchPrediction.h"

using namespace tse;
using namespace std;

namespace tse
{
FALLBACK_DECL(taxAddUtcToDiag);
}

const string
TaxCodeValidator::TAX_CODE_AY("AY");
const string
TaxCodeValidator::TAX_CODE_US2("US2");
const string
TaxCodeValidator::TAX_CODE_ZP("ZP");
const string
TaxCodeValidator::PAX_TYPE_CODE_CMP("CMP");

static const Indicator SALE = 'S';
static const Indicator ISSUE = 'I';
static const Indicator INCLUDE = 'I';

bool
TaxCodeValidator::validateTaxCode(PricingTrx& trx,
                                  TaxResponse& taxResponse,
                                  TaxCodeReg& taxCodeReg,
                                  const VendorCode& vendorCode,
                                  const ZoneType& zoneType,
                                  const LocUtil::ApplicationType& applicationType)
{
  if (UNLIKELY((trx.diagnostic().diagnosticType() == FailTaxCodeDiagnostic) ||
      (trx.diagnostic().diagnosticType() == Diagnostic811)))
    taxResponse.diagCollector()->enable(FailTaxCodeDiagnostic, Diagnostic811);

  if (taxCodeReg.displayonlyInd() == YES)
    return false;

  if (UNLIKELY(taxCodeReg.taxexcessbagInd() == YES))
    return false;

  if (UNLIKELY(!validateEffectiveDate(trx, taxCodeReg)))
  {
    if (taxResponse.diagCollector()->isActive())
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::SALES_DATE, Diagnostic811);
    }

    return false;
  }

  if (!validatePointOfSale(trx, taxCodeReg, vendorCode, zoneType, applicationType))
  {
    if (UNLIKELY(taxResponse.diagCollector()->isActive()))
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::POINT_OF_SALE, Diagnostic811);
    }

    return false;
  }

  if (!validatePartitionId(trx, taxCodeReg))
  {
    if (UNLIKELY(taxResponse.diagCollector()->isActive()))
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::PARTITION_ID, Diagnostic811);
    }

    return false;
  }

  if (!validatePointOfIssue(trx, taxCodeReg, vendorCode, zoneType, applicationType))
  {
    if (UNLIKELY(taxResponse.diagCollector()->isActive()))
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::POINT_OF_ISSUE, Diagnostic811);
    }

    return false;
  }

  if (!validateOriginDate(trx, taxResponse, taxCodeReg))
  {
    if (UNLIKELY(taxResponse.diagCollector()->isActive()))
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::TRAVEL_DATE, Diagnostic811);
    }

    return false;
  }

  if (!validatePassengerRestrictions(trx, taxResponse, taxCodeReg, taxResponse.farePath()->paxType()))
  {
    if (UNLIKELY(taxResponse.diagCollector()->isActive()))
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::PAX_TYPE, Diagnostic811);
    }

    return false;
  }

  if (!validateFreeTktExemption(trx, taxResponse, taxCodeReg))
  {
    if (taxResponse.diagCollector()->isActive())
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::FREE_TKT_EXEMPT, Diagnostic811);
    }

    return false;
  }

  if (!validateSellCurrencyRestrictions(trx, taxCodeReg))
  {
    if (UNLIKELY(taxResponse.diagCollector()->isActive()))
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::SELL_CURRENCY, Diagnostic811);
    }

    return false;
  }

  if (!validateCarrierRestrictions(trx, taxResponse, taxCodeReg))
  {
    if (UNLIKELY(taxResponse.diagCollector()->isActive()))
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::VALID_CXR, Diagnostic811);
    }

    return false;
  }

  if (!validateTravelType(trx, taxResponse, taxCodeReg))
  {
    if (UNLIKELY(taxResponse.diagCollector()->isActive()))
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::TRAVEL_TYPE, Diagnostic811);
    }

    return false;
  }

  if (!validateTripOwRt(trx, taxResponse, taxCodeReg))
  {
    if (taxResponse.diagCollector()->isActive())
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::ITINERARY, Diagnostic811);
    }

    return false;
  }

  if (!validateOrigin(trx, taxResponse, taxCodeReg, vendorCode, zoneType, applicationType))
  {
    if (UNLIKELY(taxResponse.diagCollector()->isActive()))
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::ORIGIN_LOCATION, Diagnostic811);
    }

    return false;
  }

  if (UNLIKELY(!validateFormOfPayment(trx, taxCodeReg)))
  {
    if (taxResponse.diagCollector()->isActive())
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::FORM_OF_PAYMENT, Diagnostic811);
    }

    return false;
  }

  if (!validateApplyTaxOnce(trx, taxResponse, taxCodeReg))
  {
    if (taxResponse.diagCollector()->isActive())
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::APPLY_ONCE_PER_ITIN, Diagnostic811);
    }

    return false;
  }

  if (UNLIKELY(!validateSegmentFee(trx, taxResponse, taxCodeReg)))
  {
    if (taxResponse.diagCollector()->isActive())
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::CXR_EXCL, Diagnostic811);
    }

    return false;
  }

  if (UNLIKELY(!validateRestrictionLocation(trx, taxCodeReg, vendorCode, zoneType, applicationType)))
  {
    if (taxResponse.diagCollector()->isActive())
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::CXR_EXCL, Diagnostic811);
    }

    return false;
  }

  if (UNLIKELY(!validateAnciliaryServiceCode(trx, taxResponse, taxCodeReg)))
  {
    if (taxResponse.diagCollector()->isActive())
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::OTHER, Diagnostic811);
    }

    return false;
  }

  if (!validateOriginalTicket(trx, taxCodeReg))
  {
    if (UNLIKELY(taxResponse.diagCollector()->isActive()))
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::ORIGINAL_TICKET_ONLY, Diagnostic811);
    }

    return false;
  }

  return true;
}

bool
TaxCodeValidator::validateEffectiveDate(PricingTrx& trx, TaxCodeReg& taxCodeReg)
{
  if (UNLIKELY(trx.getRequest()->ticketingDT() > taxCodeReg.expireDate()))
    return false;

  if (UNLIKELY((taxCodeReg.effDate() > trx.getRequest()->ticketingDT()) ||
      (taxCodeReg.discDate().date() < trx.getRequest()->ticketingDT().date())))
    return false;

  return true;
}

bool
TaxCodeValidator::validatePartitionId(PricingTrx& trx, TaxCodeReg& taxCodeReg) const
{
  const std::string partitionId(utc::partitionId(trx, taxCodeReg));
  if (partitionId.empty() || partitionId == trx.billing()->partitionID())
    return true;

  return false;
}

bool
TaxCodeValidator::validateOriginalTicket(PricingTrx& trx, TaxCodeReg& taxCodeReg) const
{
  if (trx.isExchangeTrx() && ItinSelector(trx).isNewItin() &&
      utc::doesApplyOnlyOriginalTicket(trx, taxCodeReg))
  {
    if (trx.excTrxType() != PricingTrx::PORT_EXC_TRX ||
        static_cast<BaseExchangeTrx&>(trx).reqType() != AGENT_PRICING_MASK)
      return false;
  }

  return true;
}

bool
TaxCodeValidator::validatePointOfSale(PricingTrx& trx,
                                      TaxCodeReg& taxCodeReg,
                                      const VendorCode& vendorCode,
                                      const ZoneType& zoneType,
                                      const LocUtil::ApplicationType& applicationType)
{
  if (taxCodeReg.posLoc().empty())
    return true;

  const Loc* pointOfSaleLocation = TrxUtil::saleLoc(trx);

  bool locMatch = LocUtil::isInLoc(*(pointOfSaleLocation),
                                   taxCodeReg.posLocType(),
                                   taxCodeReg.posLoc(),
                                   vendorCode,
                                   zoneType,
                                   applicationType,
                                   GeoTravelType::International,
                                   EMPTY_STRING(),
                                   trx.getRequest()->ticketingDT());

  if ((locMatch && taxCodeReg.posExclInd() == YES) || (!locMatch && taxCodeReg.posExclInd() != YES))
    return false;

  return true;
}

bool
TaxCodeValidator::validatePointOfIssue(PricingTrx& trx,
                                       TaxCodeReg& taxCodeReg,
                                       const VendorCode& vendorCode,
                                       const ZoneType& zoneType,
                                       const LocUtil::ApplicationType& applicationType)
{
  if (taxCodeReg.poiLoc().empty())
    return true;

  const Loc* pointOfSaleLocation = TrxUtil::ticketingLoc(trx);

  bool locMatch = LocUtil::isInLoc(*(pointOfSaleLocation),
                                   taxCodeReg.poiLocType(),
                                   taxCodeReg.poiLoc(),
                                   vendorCode,
                                   zoneType,
                                   applicationType);

  if ((locMatch && taxCodeReg.poiExclInd() == YES) || (!locMatch && taxCodeReg.poiExclInd() != YES))
    return false;

  return true;
}

bool
TaxCodeValidator::validateOriginDate(PricingTrx& trx,
                                     TaxResponse& taxResponse,
                                     TaxCodeReg& taxCodeReg)
{
  //disable validator for old itin (exchange)
  if (_travelSeg)
    return true;

  std::vector<TravelSeg*>::const_iterator travelSegI = getTravelSeg(taxResponse).begin();

  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

  if (taxCodeReg.tvlDateasoriginInd() == YES)
  {
    if (UNLIKELY(((airSeg) && (*travelSegI)->departureDT().date() < taxCodeReg.firstTvlDate().date()) ||
        (*travelSegI)->departureDT().date() > taxCodeReg.lastTvlDate().date()))
      return false;
  }
  else
  {
    for (; travelSegI != getTravelSeg(taxResponse).end(); travelSegI++)
    {
      airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

      if ((airSeg) && (*travelSegI)->departureDT().date() >= taxCodeReg.firstTvlDate().date())
        return true;
    }
    return false;
  }
  return true;
}

bool
TaxCodeValidator::validatePassengerRestrictions(PricingTrx& trx,
                                                TaxResponse& taxResponse,
                                                TaxCodeReg& taxCodeReg,
                                                const PaxType* paxType)
{
  if (taxCodeReg.restrictionPsg().empty())
    return true;

  std::vector<TaxRestrictionPsg>::iterator taxRestrictionPsgI;
  bool paxFound = false;

  for (taxRestrictionPsgI = taxCodeReg.restrictionPsg().begin();
       taxRestrictionPsgI != taxCodeReg.restrictionPsg().end();
       taxRestrictionPsgI++)
  {
    if ((*taxRestrictionPsgI).psgType() != paxType->paxType())
      continue;

    uint16_t paxTypeAge = paxType->age();
    if (TrxUtil::isTaxExemptionForChildActive(trx))
    {
      paxTypeAge = PaxTypeUtil::getDifferenceInYears(paxType->birthDate(), trx.ticketingDate());
    }

    paxFound = true;

    if (((*taxRestrictionPsgI).minAge() == 0) && ((*taxRestrictionPsgI).maxAge() == 0) &&
        ((*taxRestrictionPsgI).fareZeroOnly() != YES) && (taxCodeReg.psgrExclInd() == YES))
      return false;

    if (UNLIKELY(((*taxRestrictionPsgI).minAge()) &&
        (paxTypeAge >= (*taxRestrictionPsgI).minAge()) &&
        ((*taxRestrictionPsgI).maxAge()) &&
        (paxTypeAge <= (*taxRestrictionPsgI).maxAge()) &&
        (taxCodeReg.psgrExclInd() == YES)))
      return false;

    if (UNLIKELY(((*taxRestrictionPsgI).minAge()) && ((*taxRestrictionPsgI).maxAge() == 0) &&
        (taxCodeReg.psgrExclInd() == YES) &&
        (paxTypeAge >= (*taxRestrictionPsgI).minAge())))
      return false;

    if (((*taxRestrictionPsgI).maxAge()) && ((*taxRestrictionPsgI).minAge() == 0) &&
        (taxCodeReg.psgrExclInd() == YES) &&
        (paxTypeAge <= (*taxRestrictionPsgI).maxAge()))
      return false;

    if (((*taxRestrictionPsgI).fareZeroOnly() == YES) && (taxCodeReg.psgrExclInd() == YES) &&
        (taxResponse.farePath()->getTotalNUCAmount() == 0))
      return false;

    if (((*taxRestrictionPsgI).minAge()) && (taxCodeReg.psgrExclInd() != YES) &&
        (paxTypeAge < (*taxRestrictionPsgI).minAge()))
      return false;

    if (((*taxRestrictionPsgI).maxAge()) && (taxCodeReg.psgrExclInd() != YES) &&
        (paxTypeAge > (*taxRestrictionPsgI).maxAge()))
      return false;
  }

  if ((!paxFound) && (taxCodeReg.psgrExclInd() != YES))
    return false;

  return true;
}

bool
TaxCodeValidator::validateFreeTktExemption(PricingTrx& trx,
                                           TaxResponse& taxResponse,
                                           TaxCodeReg& taxCodeReg)
{
  if (taxCodeReg.taxCode() == TAX_CODE_US2)
  {
    Itin* itin = taxResponse.farePath()->itin();

    if (UNLIKELY(taxResponse.paxTypeCode() == PAX_TYPE_CODE_CMP))
      return true;

    const CarrierPreference* cp = trx.dataHandle().getCarrierPreference(
        itin->validatingCarrier(), trx.getRequest()->ticketingDT());
    if (LIKELY(cp != nullptr))
    {
      if (TypeConvert::pssCharToBool(cp->applyUS2TaxOnFreeTkt()))
        return true;
    }

    if (UNLIKELY(taxUtil::doUsTaxesApplyOnYQYR(trx, *(taxResponse.farePath()))))
      return true;
  }

  if (taxCodeReg.taxCode() == TAX_CODE_ZP)
  {
    if (UNLIKELY(taxUtil::doUsTaxesApplyOnYQYR(trx, *(taxResponse.farePath()))))
      return true;
  }

  if ((taxCodeReg.freeTktexempt() == YES) &&
      (taxResponse.farePath()->getTotalNUCAmount() < EPSILON) &&
      trx.getRequest()->equivAmountOverride() < EPSILON)
    return false;

  return true;
}

bool
TaxCodeValidator::validateSellCurrencyRestrictions(PricingTrx& trx, TaxCodeReg& taxCodeReg)
{
  if (taxCodeReg.sellCur().empty())
    return true;

  CurrencyCode currencyCode = trx.getRequest()->ticketingAgent()->currencyCodeAgent();

  if (LIKELY(!trx.getOptions()->currencyOverride().empty()))
  {
    currencyCode = trx.getOptions()->currencyOverride();
  }

  if ((taxCodeReg.sellCur() == currencyCode) && (taxCodeReg.sellCurExclInd() == YES))
    return false;

  if ((taxCodeReg.sellCur() != currencyCode) && (taxCodeReg.sellCurExclInd() != YES))
    return false;

  return true;
}

bool
TaxCodeValidator::validateCarrierRestrictions(PricingTrx& trx,
                                              TaxResponse& taxResponse,
                                              TaxCodeReg& taxCodeReg)
{
  if (taxCodeReg.restrictionValidationCxr().empty())
    return true;

  Itin* itin = taxResponse.farePath()->itin();

  if (UNLIKELY(itin->validatingCarrier().empty()))
    return true;

  std::vector<CarrierCode>::iterator restrictionValidationCxrI;

  for (restrictionValidationCxrI = taxCodeReg.restrictionValidationCxr().begin();
       restrictionValidationCxrI != taxCodeReg.restrictionValidationCxr().end();
       restrictionValidationCxrI++)
  {

    if (taxCodeReg.valcxrExclInd() == YES)
    {

      if ((*restrictionValidationCxrI) == itin->validatingCarrier())
        return false;
    }
    else
    {
      if ((*restrictionValidationCxrI) == itin->validatingCarrier())
        return true;
    }
  }

  if (taxCodeReg.valcxrExclInd() == YES)
    return true;

  return false;
}

bool
TaxCodeValidator::validateTravelType(PricingTrx& trx,
                                     TaxResponse& taxResponse,
                                     TaxCodeReg& taxCodeReg)
{
  std::vector<TravelSeg*>::const_iterator travelSegI = getTravelSeg(taxResponse).begin();

  bool domesticTravelOnly = true;
  const AirSeg* airSeg;

  for (; travelSegI != getTravelSeg(taxResponse).end(); travelSegI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
      continue;

    if (LocUtil::isUSTerritoryOnly(*airSeg->origin()) ||
        LocUtil::isUSTerritoryOnly(*airSeg->destination()))
    {
      domesticTravelOnly = false;
      break;
    }

    if (((*travelSegI)->geoTravelType() == GeoTravelType::International) ||
        ((*travelSegI)->geoTravelType() == GeoTravelType::Transborder))
    {
      domesticTravelOnly = false;
      break;
    }
  }

  utc::OverrideJourneyType journeyType(trx, taxCodeReg);
  if (journeyType.isOverrideJourneyType())
    return validateJourneyType(trx, taxResponse, taxCodeReg, journeyType);

  if ((taxCodeReg.travelType() == DOMESTIC) && (!domesticTravelOnly))
    return false;

  if ((taxCodeReg.travelType() == INTERNATIONAL) && (domesticTravelOnly))
    return false;

  return true;
}

bool
TaxCodeValidator::validateTripOwRt(PricingTrx& trx,
                                   TaxResponse& taxResponse,
                                   TaxCodeReg& taxCodeReg)
{
  if ((taxCodeReg.itineraryType() != CIRCLE_TRIP) && (taxCodeReg.itineraryType() != ONEWAY_TRIP))
    return true;

  std::vector<TravelSeg*>::const_iterator travelSegI;
  travelSegI = taxResponse.farePath()->itin()->travelSeg().begin();

  TravelSeg* tvlSegFront = taxResponse.farePath()->itin()->travelSeg().front();
  TravelSeg* tvlSegBack = taxResponse.farePath()->itin()->travelSeg().back();

  bool domesticTravelOnly = true;

  for (; travelSegI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegI++)
  {
    if (((*travelSegI)->geoTravelType() == GeoTravelType::International) ||
        ((*travelSegI)->geoTravelType() == GeoTravelType::Transborder))
    {
      domesticTravelOnly = false;
      break;
    }
  }

  bool oneWay = false;
  bool roundTrip = true;

  if (domesticTravelOnly)
  {
    // lint --e{530}
    SmallBitSet<uint16_t, Itin::TripCharacteristics>& tripCharacteristics =
        taxResponse.farePath()->itin()->tripCharacteristics();

    if (tripCharacteristics.isSet(Itin::OneWay))
    {
      oneWay = true;
      roundTrip = false;
    }
  }
  else
  {
    if ((tvlSegFront)->origin()->nation() != (tvlSegBack)->destination()->nation())
    {
      roundTrip = false;
      oneWay = true;
    }
  }

  if (taxCodeReg.taxCode() == TAX_CODE_AY)
  {
    //
    // TODO Remove All AY code after After December 31 2006
    //
    if (taxCodeReg.specialProcessNo() != 16)
      return true;

    if ((!domesticTravelOnly) &&
        ((tvlSegFront)->origin()->loc() != (tvlSegBack)->destination()->loc()))
    {
      roundTrip = false;
      oneWay = true;
    }

    if (((tvlSegFront)->origin()->city() == NEWYORK || (tvlSegFront)->origin()->city() == NEWARK) &&
        ((tvlSegBack)->destination()->city() == NEWYORK ||
         (tvlSegBack)->destination()->city() == NEWARK))
    {
      roundTrip = true;
      oneWay = false;
    }
  }

  if ((taxCodeReg.itineraryType() == CIRCLE_TRIP) && (oneWay))
    return false;

  if ((taxCodeReg.itineraryType() == ONEWAY_TRIP) && (roundTrip))
    return false;

  return true;
}

bool
TaxCodeValidator::validateOrigin(PricingTrx& trx,
                                 TaxResponse& taxResponse,
                                 TaxCodeReg& taxCodeReg,
                                 const VendorCode& vendorCode,
                                 const ZoneType& zoneType,
                                 const LocUtil::ApplicationType& applicationType)
{
  if (taxCodeReg.originLoc().empty())
    return true;

  // lint -e{530}
  TravelSeg* travelSeg = getTravelSeg(taxResponse).front();

  bool locMatch = LocUtil::isInLoc(*travelSeg->origin(),
                                   taxCodeReg.originLocType(),
                                   taxCodeReg.originLoc(),
                                   vendorCode,
                                   zoneType,
                                   applicationType,
                                   GeoTravelType::International,
                                   EMPTY_STRING(),
                                   trx.getRequest()->ticketingDT());

  if ((locMatch && taxCodeReg.originLocExclInd() == YES) ||
      (!locMatch && taxCodeReg.originLocExclInd() != YES))
    return false;

  return true;
}

bool
TaxCodeValidator::validateFormOfPayment(PricingTrx& trx, TaxCodeReg& taxCodeReg)
{
  if (LIKELY(taxCodeReg.formOfPayment() == ALL))
    return true;

  if ((taxCodeReg.formOfPayment() == CASH) && (!trx.getRequest()->isFormOfPaymentCash()))
    return false;

  if ((taxCodeReg.formOfPayment() == CHECK) && (!trx.getRequest()->isFormOfPaymentCheck()))
    return false;

  if ((taxCodeReg.formOfPayment() == CARD) && (!trx.getRequest()->isFormOfPaymentCard()))
    return false;

  return true;
}

bool
TaxCodeValidator::isTaxOnChangeFee(PricingTrx& trx, TaxCodeReg& taxCodeReg) const
{
  return utc::isTaxOnChangeFee(trx, taxCodeReg, trx.getRequest()->ticketingDT());
}

bool
TaxCodeValidator::validateApplyTaxOnce(PricingTrx& trx,
                                       TaxResponse& taxResponse,
                                       TaxCodeReg& taxCodeReg)
{
  if (taxResponse.taxItemVector().empty())
    return true;

  if ((taxCodeReg.occurrence() != APPLIED) && (taxCodeReg.occurrence() != BOOKLET) &&
      (taxCodeReg.taxType() != PERCENTAGE) && (taxCodeReg.rangeType() != FARE) &&
      (taxCodeReg.loc1Appl() != ORIGIN) && (taxCodeReg.loc2Appl() != TERMINATION) &&
      (taxCodeReg.tripType() != WHOLLY_WITHIN) &&
      ((taxCodeReg.loc1Appl() != BLANK) || (taxCodeReg.loc2Appl() != BLANK) ||
       (taxCodeReg.tripType() != BLANK)))
    return true;

  if (isTaxOnChangeFee(trx, taxCodeReg))
  {
    std::vector<TaxItem*>::const_iterator taxItemI = taxResponse.changeFeeTaxItemVector().begin();

    for (; taxItemI != taxResponse.changeFeeTaxItemVector().end(); taxItemI++)
    {
      if ((*taxItemI)->taxCode() == taxCodeReg.taxCode())
        return false;
    }
  }
  else
  {
    std::vector<TaxItem*>::const_iterator taxItemI = taxResponse.taxItemVector().begin();

    for (; taxItemI != taxResponse.taxItemVector().end(); taxItemI++)
    {
      if ((*taxItemI)->taxCode() == taxCodeReg.taxCode())
        return false;
    }
  }
  return true;
}

bool
TaxCodeValidator::validateSegmentFee(PricingTrx& trx,
                                     TaxResponse& taxResponse,
                                     TaxCodeReg& taxCodeReg)
{
  if (LIKELY(taxCodeReg.specialProcessNo() != 64))
    return true;

  if (trx.getRequest()->ticketingAgent()->agentTJR() == nullptr)
    return false;

  if (TypeConvert::pssCharToBool(
          trx.getRequest()->ticketingAgent()->agentTJR()->doNotApplySegmentFee()))
    return false;

  return true;
}

bool
TaxCodeValidator::validateAnciliaryServiceCode(PricingTrx& trx,
                                               TaxResponse& taxResponse,
                                               TaxCodeReg& taxCodeReg)
{
  TaxTrx* taxTrx = dynamic_cast<TaxTrx*>(&trx);

  if (LIKELY(!taxTrx))
    return true;

  Itin* itin = taxResponse.farePath()->itin();

  if (itin)
  {
    if (itin->anciliaryServiceCode().empty())
      return true;
  }

  if (taxCodeReg.specialProcessNo() == 8000)
    return true;

  return false;
}

bool
TaxCodeValidator::validateRestrictionLocation(PricingTrx& trx,
                                              TaxCodeReg& taxCodeReg,
                                              const VendorCode& vendorCode,
                                              const ZoneType& zoneType,
                                              const LocUtil::ApplicationType& applicationType)
{
  if (taxCodeReg.taxRestrictionLocNo().empty())
    return true;

  const TaxRestrictionLocationInfo* location =
      trx.dataHandle().getTaxRestrictionLocation(taxCodeReg.taxRestrictionLocNo());
  if (location == nullptr)
    return true;

  return checkRestrictionLocation(location, trx, vendorCode, zoneType, applicationType);
}

bool
TaxCodeValidator::checkRestrictionLocation(const TaxRestrictionLocationInfo* location,
                                           PricingTrx& trx,
                                           const VendorCode& vendorCode,
                                           const ZoneType& zoneType,
                                           const LocUtil::ApplicationType& applicationType)
{
  const std::vector<TaxRestrictionLocationInfo::TaxRestrictionLocationInfoSeq>& locationSeqs =
      location->seqs();
  std::vector<TaxRestrictionLocationInfo::TaxRestrictionLocationInfoSeq>::const_iterator
  locationSeqI = locationSeqs.begin();
  std::vector<TaxRestrictionLocationInfo::TaxRestrictionLocationInfoSeq>::const_iterator
  locationSeqEndI;
  int include = 0;
  int exclude = 0;

  for (locationSeqI = locationSeqs.begin(), locationSeqEndI = locationSeqs.end();
       locationSeqI != locationSeqEndI;
       locationSeqI++)
  {
    int score = 0;

    if (locationSeqI->locType() == LOCTYPE_PCC)
    {
      if (locationSeqI->loc() == trx.getRequest()->ticketingAgent()->tvlAgencyPCC())
        score = 8;
    }
    else if (locationSeqI->locType() == LOCTYPE_PCC_ARC)
    {
      if (locationSeqI->loc() == trx.getRequest()->ticketingAgent()->tvlAgencyIATA())
        score = 7;
    }
    else
    {
      bool locMatch = false;

      if (locationSeqI->saleIssueInd() != ISSUE)
      {
        const Loc* pointOfSaleLocation = TrxUtil::saleLoc(trx);
        locMatch = LocUtil::isInLoc(*(pointOfSaleLocation),
                                    locationSeqI->locType(),
                                    locationSeqI->loc(),
                                    vendorCode,
                                    zoneType,
                                    applicationType);
      }

      if ((locationSeqI->saleIssueInd()) != SALE &&
          (locationSeqI->saleIssueInd() == ISSUE || locMatch))
      {
        const Loc* pointOfIssueLocation = TrxUtil::ticketingLoc(trx);
        locMatch = LocUtil::isInLoc(*(pointOfIssueLocation),
                                    locationSeqI->locType(),
                                    locationSeqI->loc(),
                                    vendorCode,
                                    zoneType,
                                    applicationType);
      }

      if (locMatch)
      {
        switch (locationSeqI->locType())
        {
        case LOCTYPE_CITY:
          score = 6;
          break;
        case LOCTYPE_STATE:
          score = 5;
          break;
        case LOCTYPE_NATION:
          score = 4;
          break;
        case LOCTYPE_ZONE:
          score = 3;
          break;
        case LOCTYPE_SUBAREA:
          score = 2;
          break;
        case LOCTYPE_AREA:
          score = 1;
          break;
        }
      }
    }

    if (score != 0)
    {
      if (locationSeqI->inclExclInd() == INCLUDE)
      {
        if (score > include)
          include = score;
      }
      else
      {
        if (score > exclude)
          exclude = score;
      }
    }
  }
  return include >= exclude;
}

bool
TaxCodeValidator::validateJourneyType(PricingTrx& trx,
                                      TaxResponse& taxResponse,
                                      TaxCodeReg& taxCodeReg,
                                      utc::OverrideJourneyType& journeyType)
{
  bool locMatch = true;
  const AirSeg* airSeg;

  std::vector<TravelSeg*>::const_iterator travelSegI = getTravelSeg(taxResponse).begin();
  std::vector<TravelSeg*>::const_iterator travelSegEnd = getTravelSeg(taxResponse).end();

  for (; travelSegI != travelSegEnd; ++travelSegI)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
      continue;

    locMatch = LocUtil::isInLoc(*(*travelSegI)->origin(),
                                journeyType.getLocType(),
                                journeyType.getLocCode(),
                                Vendor::SABRE,
                                MANUAL,
                                LocUtil::TAXES,
                                GeoTravelType::International,
                                EMPTY_STRING(),
                                trx.getRequest()->ticketingDT());

    if ((locMatch && journeyType.getExclInd() == YES) ||
        (!locMatch && journeyType.getExclInd() != YES))
    {
      locMatch = false;
      break;
    }
    locMatch = LocUtil::isInLoc(*(*travelSegI)->destination(),
                                journeyType.getLocType(),
                                journeyType.getLocCode(),
                                Vendor::SABRE,
                                MANUAL,
                                LocUtil::TAXES,
                                GeoTravelType::International,
                                EMPTY_STRING(),
                                trx.getRequest()->ticketingDT());

    if ((locMatch && journeyType.getExclInd() == YES) ||
        (!locMatch && journeyType.getExclInd() != YES))
    {
      locMatch = false;
      break;
    }
  }

  if (!locMatch)
  {
    TaxDiagnostic::collectErrors(trx,
                                 taxCodeReg,
                                 taxResponse,
                                 TaxDiagnostic::JOURNEY_TYPE,
                                 Diagnostic816);
  }

  return locMatch;
}

const std::vector<TravelSeg*>&
TaxCodeValidator::getTravelSeg(const TaxResponse& taxResponse) const
{
  return _travelSeg ? *_travelSeg : taxResponse.farePath()->itin()->travelSeg();
}
