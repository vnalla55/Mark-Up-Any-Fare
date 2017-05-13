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

#include "Taxes/LegacyTaxes/TaxZP_00.h"

#include "Common/BSRCollectionResults.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/ErrorResponseException.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/TaxRound.h"
#include "Common/TrxUtil.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/ConsolidatorPlusUp.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/BankerSellRate.h"
#include "DBAccess/Currency.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Nation.h"
#include "DBAccess/NUCInfo.h"
#include "DBAccess/TaxCodeReg.h"
#include "Rules/RuleUtil.h"
#include "Taxes/Common/LocRestrictionValidator.h"
#include "Taxes/LegacyTaxes/CarrierValidator.h"
#include "Taxes/LegacyTaxes/EquipmentValidator.h"
#include "Taxes/LegacyTaxes/FareClassValidator.h"
#include "Taxes/LegacyTaxes/MirrorImage.h"
#include "Taxes/LegacyTaxes/TaxApply.h"
#include "Taxes/LegacyTaxes/TaxCodeValidator.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxLocIterator.h"
#include "Taxes/LegacyTaxes/TaxOnTax.h"
#include "Taxes/LegacyTaxes/TaxRange.h"
#include "Taxes/LegacyTaxes/TransitValidator.h"
#include "Taxes/LegacyTaxes/TripTypesValidator.h"

using namespace tse;
using namespace std;

log4cxx::LoggerPtr
TaxZP_00::_logger(log4cxx::Logger::getLogger("atseintl.Taxes.TaxZP_00"));

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

TaxZP_00::TaxZP_00() {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

TaxZP_00::~TaxZP_00() {}

bool
TaxZP_00::validateUS1(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg, int16_t segId)
{
  bool bUS1TaxPresent = false;
  std::vector<TaxItem*>::const_iterator taxItemI;
  static std::string US1_TAXCODE = "US1";

  for (taxItemI = taxResponse.taxItemVector().begin();
       taxItemI != taxResponse.taxItemVector().end();
       taxItemI++)
  {
    if ((*taxItemI)->taxCode() == US1_TAXCODE)
    {
      if (segId<0 ||
          (segId >= (*taxItemI)->travelSegStartIndex() && segId <= (*taxItemI)->travelSegEndIndex()) )
      {
        bUS1TaxPresent = true;
        break;
      }
    }
  }

  if (!bUS1TaxPresent)
  {
    if (segId<0)
      TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::NO_SPECIAL_TAX, Diagnostic816);

    LOG4CXX_DEBUG(_logger, "*** TaxZP_00: Generic tax restrictions failed on NO US1, segment:" << segId);

    return false;
  }

  LOG4CXX_DEBUG(_logger, "*** TaxZP_00: Generic tax restrictions US1 found, segment:" <<segId);
  return true;
}

bool
TaxZP_00::isInLoc(const Loc& loc,
                  const LocTypeCode& locTypeCode,
                  const LocCode& locCode,
                  const DateTime& ticketingDate,
                  const Indicator& exclInd) const
{
  bool bMatch = LocUtil::isInLoc(loc,
                                 locTypeCode,
                                 locCode,
                                 Vendor::SABRE,
                                 MANUAL,
                                 LocUtil::TAXES,
                                 GeoTravelType::International,
                                 EMPTY_STRING(),
                                 ticketingDate);

  if ((bMatch && exclInd == LocRestrictionValidator::TAX_EXCLUDE) ||
      (!bMatch && exclInd != LocRestrictionValidator::TAX_EXCLUDE))
    return false;

  return true;
}

void
TaxZP_00::apply(PricingTrx& trx,
                TaxResponse& taxResponse,
                TaxCodeReg& taxCodeReg,
                TravelSeg* travelSeg)
{
  uint16_t segmentOrder = getSegmentOrder(trx, taxResponse, travelSeg);
  if (travelSeg->hiddenStops().size() > 0)
  {
    std::vector<const Loc*> segs;

    segs.push_back(travelSeg->origin());
    segs.insert(segs.begin() + 1, travelSeg->hiddenStops().begin(), travelSeg->hiddenStops().end());
    segs.push_back(travelSeg->destination());

    LOG4CXX_DEBUG(_logger, "*** TaxZP_00: hidden segments: " << segs.size());

    for (int i = 0; i < (int)segs.size() - 1; i++)
    {

      LOG4CXX_DEBUG(_logger,
                    "*** TaxZP_00: (non rural)? hidden segment " << segs[i]->loc() << "-"
                                                                 << segs[i + 1]->loc());

      Tax tax;

      tax.taxCreate(trx, taxResponse, taxCodeReg, segmentOrder, segmentOrder);
      tax.adjustTax(trx, taxResponse, taxCodeReg);
      tax.doTaxRound(trx, taxCodeReg);

      if (segs[i]->ruralarpind() != true && segs[i + 1]->ruralarpind() != true)
      {
        _taxAmount = tax.taxAmount();
        _taxableFare = tax.taxableFare();
      }
      else
      {
        LOG4CXX_DEBUG(_logger,
                      "*** TaxZP_00: hidden rural airport on origin or destination "
                          << segs[i] << "-" << segs[i + 1]);
        _taxAmount = 0;
        _taxableFare = 0;
      }
      _paymentCurrency = tax.paymentCurrency();
      _paymentCurrencyNoDec = tax.paymentCurrencyNoDec();
      _travelSegStartIndex = segmentOrder;
      _travelSegEndIndex = segmentOrder;
      _hiddenBrdAirport = segs[i]->loc();
      _hiddenOffAirport = segs[i + 1]->loc();

      TaxApply taxApply;

      taxApply.initializeTaxItem(trx, *this, taxResponse, taxCodeReg);

      _hiddenBrdAirport = "";
      _hiddenOffAirport = "";
      _taxAmount = 0;
    }
  }
  else
  {
    LOG4CXX_DEBUG(_logger, "*** TaxZP_00: apply on " << segmentOrder);

    Tax tax;
    tax.taxCreate(trx, taxResponse, taxCodeReg, segmentOrder, segmentOrder);
    tax.adjustTax(trx, taxResponse, taxCodeReg);
    tax.doTaxRound(trx, taxCodeReg);

    if (travelSeg->origin()->ruralarpind() != true &&
        travelSeg->destination()->ruralarpind() != true)
    {
      _taxAmount = tax.taxAmount();
      _taxableFare = tax.taxableFare();
    }
    else
    {
      LOG4CXX_DEBUG(_logger,
                    "*** TaxZP_00: rural airport on origin or destination "
                        << travelSeg->origin()->loc() << "-" << travelSeg->destination()->loc());
      _taxAmount = 0;
      _taxableFare = 0;
    }
    _paymentCurrency = tax.paymentCurrency();
    _paymentCurrencyNoDec = tax.paymentCurrencyNoDec();
    _travelSegStartIndex = segmentOrder;
    _travelSegEndIndex = segmentOrder;

    TaxApply taxApply;

    taxApply.initializeTaxItem(trx, *this, taxResponse, taxCodeReg);
  }
}

// ----------------------------------------------------------------------------
// Description:  Tax
// ----------------------------------------------------------------------------
bool
TaxZP_00::validateFinalGenericRestrictions(PricingTrx& trx,
                                           TaxResponse& taxResponse,
                                           TaxCodeReg& taxCodeReg,
                                           uint16_t& startIndex,
                                           uint16_t& endIndex)
{
  LOG4CXX_DEBUG(_logger,
                "*** TaxZP_00: Generic tax restrictions " << taxCodeReg.seqNo() << ": "
                                                          << startIndex << "-" << endIndex);

  if (startIndex > 0)
  {
    std::vector<TaxItem*>::const_iterator taxItemI;

    for (taxItemI = taxResponse.taxItemVector().begin();
         taxItemI != taxResponse.taxItemVector().end();
         taxItemI++)
    {
      if ((*taxItemI)->taxCode() == taxCodeReg.taxCode() &&
          (*taxItemI)->seqNo() == taxCodeReg.seqNo())
      {

        LOG4CXX_DEBUG(_logger, "*** TaxZP_00: seqno already applied");
        return false;
      }
    }
  }

  // no US1 tax mean no ZP tax
  if (!validateUS1(trx, taxResponse, taxCodeReg))
    return false;

  // international or domestic?
  if (taxCodeReg.travelType() != 'I')
  {

    LOG4CXX_DEBUG(_logger,
                  "*** TaxZP_00: domestic application " << taxCodeReg.seqNo() << ": " << startIndex
                                                        << "-" << endIndex);

    EquipmentValidator equipmentValidator;
    CarrierValidator carrierValidator;
    // for each segment check orig/dest match loc and other restrictions.
    //
    std::vector<TravelSeg*>::iterator travelSegI =
        taxResponse.farePath()->itin()->travelSeg().begin();
    for (; travelSegI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegI++)
    {
      const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

      if (!airSeg)
        continue;

      uint16_t const segmentOrder =
          getSegmentOrder(trx,
                          taxResponse,
                          *travelSegI,
                          travelSegI - taxResponse.farePath()->itin()->travelSeg().begin());

      if (!isInLoc(*(*travelSegI)->origin(),
                   taxCodeReg.loc1Type(),
                   taxCodeReg.loc1(),
                   trx.getRequest()->ticketingDT(),
                   taxCodeReg.loc1ExclInd()))
      {
        LOG4CXX_DEBUG(_logger, "*** TaxZP_00: enplanement failed on " << segmentOrder);
        if (taxResponse.diagCollector()->isActive())
          TaxDiagnostic::collectErrors(
              trx, taxCodeReg, taxResponse, TaxDiagnostic::ENPLANEMENT_LOCATION, Diagnostic816);
        break;
      }

      if (!isInLoc(*(*travelSegI)->destination(),
                   taxCodeReg.loc2Type(),
                   taxCodeReg.loc2(),
                   trx.getRequest()->ticketingDT(),
                   taxCodeReg.loc2ExclInd()))
      {
        LOG4CXX_DEBUG(_logger, "*** TaxZP_00: deplanement failed on " << segmentOrder);
        if (taxResponse.diagCollector()->isActive())
          TaxDiagnostic::collectErrors(
              trx, taxCodeReg, taxResponse, TaxDiagnostic::DEPLANEMENT_LOCATION, Diagnostic816);
        break;
      }
    }

    if (travelSegI == taxResponse.farePath()->itin()->travelSeg().end())
    {
      travelSegI = taxResponse.farePath()->itin()->travelSeg().begin();
      for (std::vector<TravelSeg*>::iterator travelSegI =
               taxResponse.farePath()->itin()->travelSeg().begin();
           travelSegI != taxResponse.farePath()->itin()->travelSeg().end();
           travelSegI++)
      {
        const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

        if (!airSeg)
          continue;

        uint16_t segmentOrder =
            getSegmentOrder(trx,
                            taxResponse,
                            *travelSegI,
                            travelSegI - taxResponse.farePath()->itin()->travelSeg().begin());
        if (!equipmentValidator.validateEquipment(trx, taxResponse, taxCodeReg, (*travelSegI)))
        {
          LOG4CXX_DEBUG(_logger, "*** TaxZP_00: equipment failed on " << segmentOrder);
          continue;
        }

        if (!carrierValidator.validateCarrier(
                trx,
                taxResponse,
                taxCodeReg,
                travelSegI - taxResponse.farePath()->itin()->travelSeg().begin()))
        {
          LOG4CXX_DEBUG(_logger, "*** TaxZP_00: carrier failed on " << segmentOrder);
          continue;
        }
        apply(trx, taxResponse, taxCodeReg, (*travelSegI));
      }
    }
  }
  else
  {
    LOG4CXX_DEBUG(_logger,
                  "*** TaxZP_00: international application " << taxCodeReg.seqNo() << ": "
                                                             << startIndex << "-" << endIndex);

    uint16_t eIndex = taxResponse.farePath()->itin()->travelSeg().size() - 1; // endIndex;

    while (startIndex <= eIndex)
    {
      LOG4CXX_DEBUG(_logger,
                    "*** TaxZP_00: international logic " << taxCodeReg.seqNo() << ": " << startIndex
                                                         << "-" << eIndex);

      std::vector<TravelSeg*>::iterator travelSegI =
          taxResponse.farePath()->itin()->travelSeg().begin() + startIndex;
      const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

      if (!airSeg)
      {
        startIndex++;
        continue;
      }

      Indicator exclInd = 'N';

      int16_t iEnd = -1;
      int16_t iStart = -1;
      int16_t iLastUSA = -1;

      bool bOrgUS = false;

      for (int16_t iSeg = startIndex; iSeg >= 0 /* <= endIndex*/; iSeg--)
      {
        LOG4CXX_DEBUG(_logger, "*** TaxZP_00: international US/NonUS find prev stop: " << iSeg);
        if (!validateTransitTime(trx, taxResponse, taxCodeReg, iSeg, false))
        {
          LOG4CXX_DEBUG(_logger,
                        "*** TaxZP_00: international US/NonUS find prev stop: " << iSeg
                                                                                << " isStop 1 ");

          std::vector<TravelSeg*>::iterator travelSegI =
              taxResponse.farePath()->itin()->travelSeg().begin() + iSeg;
          if (isInLoc(*(*travelSegI)->origin(),
                      taxCodeReg.loc1Type(),
                      taxCodeReg.loc1(),
                      trx.getRequest()->ticketingDT(),
                      exclInd) &&
              isInLoc(*(*travelSegI)->destination(),
                      taxCodeReg.loc1Type(),
                      taxCodeReg.loc1(),
                      trx.getRequest()->ticketingDT(),
                      exclInd))
          {
            bOrgUS = true;
          }
          LOG4CXX_DEBUG(_logger,
                        "*** TaxZP_00: international US/NonUS find prev stop: " << iSeg << " orgUS "
                                                                                << bOrgUS);
          break;
        }
      }

      /*if( isInLoc( *(*travelSegI)->origin(), taxCodeReg.loc1Type(), taxCodeReg.loc1(),
        trx.getRequest()->ticketingDT(), exclInd) &&
          isInLoc( *(*travelSegI)->destination(), taxCodeReg.loc1Type(), taxCodeReg.loc1(),
        trx.getRequest()->ticketingDT(), exclInd))
        bOrgUS = true;
  */
      bool bCheckUs1Seg = false;
      for (uint16_t iSeg = startIndex;
           iSeg < taxResponse.farePath()->itin()->travelSeg().size() /* <= endIndex*/;
           iSeg++)
      {
        std::vector<TravelSeg*>::iterator travelSegI =
            taxResponse.farePath()->itin()->travelSeg().begin() + iSeg;
        const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*travelSegI);
        if (!airSeg)
          continue;

        bool isUSA = isInLoc(*(*travelSegI)->origin(),
                             taxCodeReg.loc1Type(),
                             taxCodeReg.loc1(),
                             trx.getRequest()->ticketingDT(),
                             exclInd) &&
                     isInLoc(*(*travelSegI)->destination(),
                             taxCodeReg.loc1Type(),
                             taxCodeReg.loc1(),
                             trx.getRequest()->ticketingDT(),
                             exclInd);

        bool isStop = !validateTransitTime(trx, taxResponse, taxCodeReg, iSeg, bOrgUS);

        LOG4CXX_DEBUG(_logger,
                      "*** TaxZP_00: international US/NonUS " << bOrgUS << " isStop " << isStop
                                                              << " isUSA " << isUSA);

        if (isUSA)
          iLastUSA = iSeg;

        if (bOrgUS)
        {
          if (iStart == -1 && isUSA)
          {
            iStart = iSeg;
            iEnd = -1;
          }

          if (iStart != -1 && !isUSA)
          {
            if (iEnd == -1 && iLastUSA != -1
                && iSeg == taxResponse.farePath()->itin()->travelSeg().size()-1 )
            {
              iEnd=iLastUSA;
              bCheckUs1Seg = true;
            }
            break;
          }
          if (iStart != -1 && isStop)
          {
            iEnd = iSeg;
            // startIndex = iSeg+1;
          }
        }
        else
        {
          if (iStart == -1 && isUSA && isStop)
          {
            iStart = iEnd = iSeg;
          }

          if (iStart != -1 && isUSA)
          {
            iEnd = iSeg;
            // startIndex = iSeg+1;
          }
          if (iStart != -1 && !isUSA)
            break;
        }
      } //for

      LOG4CXX_DEBUG(_logger, "*** TaxZP_00: international from " << iStart << " to " << iEnd);
      if (iStart != -1 && iEnd != -1)
      {

        EquipmentValidator equipmentValidator;
        CarrierValidator carrierValidator;

        for (uint16_t iS = iStart; iS <= (uint16_t)iEnd; iS++)
        {

          std::vector<TravelSeg*>::iterator travelSegI =
              taxResponse.farePath()->itin()->travelSeg().begin() + iS;

          uint16_t segmentOrder = getSegmentOrder(trx, taxResponse, *travelSegI, iS);

          if (!equipmentValidator.validateEquipment(trx, taxResponse, taxCodeReg, *travelSegI))
          {
            LOG4CXX_DEBUG(_logger, "*** TaxZP_00: equipment failed on " << segmentOrder);
            continue;
          }

          if (!carrierValidator.validateCarrier(trx, taxResponse, taxCodeReg, iS))
          {
            LOG4CXX_DEBUG(_logger, "*** TaxZP_00: carrier failed on " << segmentOrder);
            continue;
          }

          if (bCheckUs1Seg && !validateUS1(trx, taxResponse, taxCodeReg, segmentOrder) )
          {
            LOG4CXX_DEBUG(_logger, "*** TaxZP_00: lack of US1 on " << segmentOrder);
            continue;
          }

          apply(trx, taxResponse, taxCodeReg, (*travelSegI));
        }
        startIndex = iEnd + 1;
      }
      else
        break; // return false;
    }
  }
  return false;
}

void
TaxZP_00::taxCreate(PricingTrx& trx,
                    TaxResponse& taxResponse,
                    TaxCodeReg& taxCodeReg,
                    uint16_t travelSegStartIndex,
                    uint16_t travelSegEndIndex)
{
}

bool
TaxZP_00::validateTransitTime(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              TaxCodeReg& taxCodeReg,
                              uint16_t startIndex,
                              bool bOrgUS)
{
  if (taxCodeReg.restrictionTransit().empty())
    return false;

  const AirSeg* airSeg;

  int32_t transitHours = 0;
  int32_t transitMinutes = 0;

  int64_t connectionMinutes = 0;
  int64_t transitTotalMinutes = 0;
  int64_t workTime = 0;

  TaxRestrictionTransit& restrictTransit = taxCodeReg.restrictionTransit().front();

  transitHours = restrictTransit.transitHours();
  transitMinutes = restrictTransit.transitMinutes();

  if ((transitHours >= 0) && (transitMinutes < 0))
    transitMinutes = 0;

  if ((transitHours < 0) && (transitMinutes >= 0))
    transitHours = 0;

  TravelSeg* travelSegTo = taxResponse.farePath()->itin()->travelSeg()[startIndex];
  TravelSeg* travelSegFrom = taxResponse.farePath()->itin()->travelSeg()[startIndex];

  if (bOrgUS)
  {
    if (travelSegFrom == taxResponse.farePath()->itin()->travelSeg().back())
      return false;

    travelSegTo = taxResponse.farePath()->itin()->travelSeg()[startIndex + 1];

    airSeg = dynamic_cast<const AirSeg*>(travelSegTo);

    if (!airSeg)
    {
      if (taxResponse.farePath()->itin()->validatingCarrier().equalToConst("WN"))
      {
        size_t nextAirInd = startIndex + 2;
        if (nextAirInd > taxResponse.farePath()->itin()->travelSeg().size())
          return false; // It should never happen because an ARUNK cannot be the last segment but
                        // don't want a core dump for malformed manually built requests.
        travelSegTo = taxResponse.farePath()->itin()->travelSeg()[nextAirInd];
      }
      else
        return false;
    }
  }
  else
  {
    if (travelSegTo == taxResponse.farePath()->itin()->travelSeg().front())
      return false;

    travelSegFrom = taxResponse.farePath()->itin()->travelSeg()[startIndex - 1];

    airSeg = dynamic_cast<const AirSeg*>(travelSegFrom);

    if (!airSeg)
    {
      if (taxResponse.farePath()->itin()->validatingCarrier().equalToConst("WN"))
      {
        int prevAirInd = startIndex - 2;
        if (prevAirInd < 0)
          return false; // It should never happen because an ARUNK cannot be the first segment but
                        // don't want a core dump for malformed manually built requests
        travelSegFrom = taxResponse.farePath()->itin()->travelSeg()[prevAirInd];
      }
      else
        return false;
    }
  }

  if (restrictTransit.sameDayInd() == YES)
  {
    if (travelSegTo->departureDT().day() != travelSegFrom->arrivalDT().day() ||
        travelSegTo->departureDT().month() != travelSegFrom->arrivalDT().month())
      return false;

    return true;
  }

  if ((transitHours <= 0) && (transitMinutes <= 0))
    return false;

  workTime = DateTime::diffTime(travelSegTo->departureDT(), travelSegFrom->arrivalDT());

  connectionMinutes = workTime / 60;
  transitTotalMinutes = (transitHours * 60) + transitMinutes; // lint !e647

  LOG4CXX_DEBUG(
      _logger, "*** TaxZP_00: transit check " << connectionMinutes << " - " << transitTotalMinutes);

  if (connectionMinutes > transitTotalMinutes)
    return false;

  return true;
}

uint16_t
TaxZP_00::getSegmentOrder(const PricingTrx& trx,
                          const TaxResponse& taxResponse,
                          TravelSeg* const travelSeg,
                          int16_t dflt) const
{
  if (trx.getTrxType() != PricingTrx::MIP_TRX)
  {
    uint16_t itinSegOrder = taxResponse.farePath()->itin()->segmentOrder(travelSeg) - 1;
    uint16_t travelSegOrder = travelSeg->segmentOrder() - 1;
    if (travelSegOrder > itinSegOrder)
      return itinSegOrder;
    else
      return travelSegOrder;
  }

  if (dflt != -1)
    return static_cast<uint16_t>(dflt);

  return taxResponse.farePath()->itin()->segmentOrder(travelSeg) - 1;
}

namespace tse
{
class TaxCodeValidatorZP : public TaxCodeValidator
{
  TaxZP_00* m_Parent;

public:
  TaxCodeValidatorZP (TaxZP_00 &parent) : TaxCodeValidator(), m_Parent(&parent) {};

  bool
  validateTravelType(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg) override
  {
    bool bRet = TaxCodeValidator::validateTravelType(trx, taxResponse, taxCodeReg);
    if ( !bRet && taxCodeReg.travelType() == INTERNATIONAL)
    {
      utc::OverrideJourneyType journeyType(trx, taxCodeReg);
      if (!journeyType.isOverrideJourneyType())
      {
        std::vector<TravelSeg*>::const_iterator travelSegI =
          taxResponse.farePath()->itin()->travelSeg().begin();

        const AirSeg* airSeg;
        bool bTmp;

        bool bZone = false;
        bool bOutside = false;

        for (; travelSegI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegI++)
        {
          airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

          if (!airSeg)
            continue;

          if (((*travelSegI)->geoTravelType() == GeoTravelType::International) ||
              ((*travelSegI)->geoTravelType() == GeoTravelType::Transborder))
          {
            bRet = true;
            break;
          }

          bTmp = m_Parent->isInLoc(*(*travelSegI)->origin(),
                     taxCodeReg.loc1Type(),
                     taxCodeReg.loc1(),
                     trx.getRequest()->ticketingDT(),
                     taxCodeReg.loc1ExclInd());

          bZone |= bTmp;
          bOutside |= !bTmp;

          if (bZone && bOutside)
          {
            bRet = true;
            break;
          }

          bTmp = m_Parent->isInLoc(*(*travelSegI)->destination(),
                     taxCodeReg.loc2Type(),
                     taxCodeReg.loc2(),
                     trx.getRequest()->ticketingDT(),
                     taxCodeReg.loc2ExclInd());

          bZone |= bTmp;
          bOutside |= !bTmp;

          if (bZone && bOutside)
          {
            bRet = true;
            break;
          }
        } //for
      }//if
    }//if

    return bRet;
  }
};

bool
TaxZP_00::validateItin(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  TaxCodeValidatorZP taxCodeValidator(*this);
  return taxCodeValidator.validateTaxCode(trx, taxResponse, taxCodeReg);
}
}
