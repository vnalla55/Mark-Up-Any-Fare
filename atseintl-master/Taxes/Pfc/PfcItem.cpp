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

#include "Taxes/Pfc/PfcItem.h"

#include "Common/FallbackUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TSEException.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxTrx.h"
#include "DBAccess/Customer.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/PfcCollectExcpt.h"
#include "DBAccess/PfcCollectMeth.h"
#include "DBAccess/PfcCxrExcpt.h"
#include "DBAccess/PfcEssAirSvc.h"
#include "DBAccess/PfcEssAirSvcProv.h"
#include "DBAccess/PfcMultiAirport.h"
#include "DBAccess/PfcPFC.h"
#include "DBAccess/PfcTktDesigExcept.h"
#include "DBAccess/TaxCodeReg.h"
#include "Diagnostic/DiagManager.h"
#include "PfcTaxesExemption/AutomaticPfcTaxExemption.h"
#include "Rules/RuleUtil.h"
#include "Taxes/Common/PricingTrxOps.h"
#include "Taxes/LegacyTaxes/PfcAbsorption.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"

#include <algorithm>

using namespace tse;
using namespace std;

log4cxx::LoggerPtr
PfcItem::_logger(log4cxx::Logger::getLogger("atseintl.Taxes.PfcItem"));

namespace tse
{
  FIXEDFALLBACK_DECL(taxShoppingPfcInfant);
  FALLBACK_DECL(fallbackPFCOvercollectionInRTItinMax4);
}

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------
PfcItem::PfcItem()
  : _absorptionInd(0),
    _allTravelDomesticUS(0),
    _roundTrip(0),
    _bypassTktDes(false),
    _pfcAmt(0),
    _pfcCurrency(""),
    _pfcNumDec(0),
    _couponNumber(0),
    _pfcAirport(""),
    _legId(-1),
    _carrierCode(""),
    _travelSeg(nullptr)
{
}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------
PfcItem::~PfcItem() {}

// ----------------------------------------------------------------------------
// Description:  PfcItem
// ----------------------------------------------------------------------------
void
PfcItem::build(PricingTrx& trx, TaxResponse& taxResponse)
{

  if (TrxUtil::isAutomaticPfcTaxExemptionEnabled(trx) &&
      AutomaticPfcTaxExemption::isAutomaticPfcExemtpionEnabled(trx, taxResponse))
  {
    if (taxResponse.automaticPFCTaxExemptionData()->pfcExemptionOption() ==
        AutomaticPfcTaxExemptionData::EXEMPT_ALL)
    {
      DiagManager diagMgr(trx, Diagnostic807);
      if (diagMgr.isActive())
      {
        diagMgr.collector()
            << " \n*************** APPLIED PFC EXEMPTIONS *********************\n \n";
        diagMgr.collector().flushMsg();
      }
      else
      {
        return;
      }
    }
  }
  else
  {
    if (trx.getRequest()->exemptPFC())
      return;
  }

  uint8_t locType = setPfcLocType(trx, taxResponse);

  const Loc* pointOfSaleLocation = TrxUtil::ticketingLoc(trx);

  if (trx.getRequest()->ticketPointOverride().empty())
  {
    pointOfSaleLocation = TrxUtil::saleLoc(trx);
  }

  bool domesticFormulaPOS =
      LocUtil::isUSPossession(*pointOfSaleLocation) || LocUtil::isUS(*pointOfSaleLocation);

  pointOfSaleLocation = TrxUtil::saleLoc(trx);

  if (TrxUtil::isAutomaticPfcTaxExemptionEnabled(trx))
  {
    _bypassTktDes = AutomaticPfcTaxExemption::bypassTktDes(*taxResponse.farePath());
  }

  if ((allTravelDomesticUS()) || ((locType != GEO_TYPE_4) && (domesticFormulaPOS)))
  {
    domesticFormula(trx, taxResponse);
  }
  else
  {
    internationalFormula(trx, taxResponse, pointOfSaleLocation);
  }

  if (TrxUtil::isAutomaticPfcTaxExemptionEnabled(trx) &&
      AutomaticPfcTaxExemption::isAutomaticPfcExemtpionEnabled(trx, taxResponse) &&
      taxResponse.automaticPFCTaxExemptionData()->pfcExemptionOption() ==
          AutomaticPfcTaxExemptionData::EXEMPT_ALL)
  {
    DiagManager diagMgr(trx, Diagnostic807);
    if (diagMgr.isActive())
    {
      diagMgr.collector() << " \n************************************************************\n";
      diagMgr.collector().flushMsg();
    }

    taxResponse.pfcItemVector().clear();
  }
}

// ----------------------------------------------------------------------------
// Description:  setPfcLocType
// ----------------------------------------------------------------------------

uint8_t
PfcItem::setPfcLocType(PricingTrx& trx, TaxResponse& taxResponse)
{

  TravelSeg* travelSeg = taxResponse.farePath()->itin()->travelSeg().front();
  TravelSeg* travelSegBack = taxResponse.farePath()->itin()->travelSeg().back();

  if (travelSeg->origin()->loc() == travelSegBack->destination()->loc())
  {
    _roundTrip = true;
  }

  if (travelSeg->origin()->loc() != travelSegBack->destination()->loc())
  {
    const PfcMultiAirport* pfcMultiAirport = trx.dataHandle().getPfcMultiAirport(
        travelSeg->origin()->loc(), trx.getRequest()->ticketingDT());

    if (pfcMultiAirport)
    {
      _roundTrip = false;

      std::vector<PfcCoterminal*>::const_iterator pfcCoterminalIter =
          pfcMultiAirport->coterminals().begin();
      std::vector<PfcCoterminal*>::const_iterator pfcCoterminalEndIter =
          pfcMultiAirport->coterminals().end();

      for (; pfcCoterminalIter != pfcCoterminalEndIter; pfcCoterminalIter++)
      {
        if ((*pfcCoterminalIter)->cotermLoc() == travelSegBack->destination()->loc())
        {
          _roundTrip = true;
          break;
        }
      }
    }
  }

  bool boardPointUS = false;
  bool boardPointUSTerritory = false;
  bool boardPointUSPossesion = false;
  _allTravelDomesticUS = true;

  const AirSeg* airSeg;
  std::vector<TravelSeg*>::const_iterator travelSegI;
  travelSegI = taxResponse.farePath()->itin()->travelSeg().begin();

  for (; travelSegI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
      continue;

    if ((LocUtil::isUS(*(*travelSegI)->origin())) &&
        !(LocUtil::isUSTerritoryOnly(*(*travelSegI)->origin())))
      boardPointUS = true;

    if ((LocUtil::isUSTerritoryOnly(*(*travelSegI)->origin())))
      boardPointUSTerritory = true;

    if (UNLIKELY((LocUtil::isUSPossession(*(*travelSegI)->origin())) &&
        !(LocUtil::isUS(*(*travelSegI)->origin()))))
      boardPointUSPossesion = true;

    if ((!LocUtil::isUS(*(*travelSegI)->origin())) &&
        (!LocUtil::isUSTerritoryOnly(*(*travelSegI)->origin())))
      _allTravelDomesticUS = false;

    if ((!LocUtil::isUS(*(*travelSegI)->destination())) &&
        (!LocUtil::isUSTerritoryOnly(*(*travelSegI)->destination())))
      _allTravelDomesticUS = false;
  }

  if ((boardPointUS) && (!boardPointUSTerritory) && (!boardPointUSPossesion))
    return GEO_TYPE_1;

  if ((boardPointUS) && ((boardPointUSTerritory) || (boardPointUSPossesion)))
    return GEO_TYPE_2;

  if ((!boardPointUS) && ((boardPointUSTerritory) || (boardPointUSPossesion)))
    return GEO_TYPE_3;

  return GEO_TYPE_4;
}

FareUsage*
PfcItem::findFareUsage(TravelSeg& travelSeg, TaxResponse& taxResponse) const
{
  Itin* itin = taxResponse.farePath()->itin();
  FarePath* farePath = taxResponse.farePath();

  for (const PricingUnit* puI : farePath->pricingUnit())
  {
    if (puI)
    {
      for (FareUsage* fuI : puI->fareUsage())
      {
        if (fuI)
        {
          for(TravelSeg* seg : (*fuI).travelSeg())
          {
            if (seg && itin->segmentOrder(seg) == itin->segmentOrder(&travelSeg))
              return fuI;
          }
        }
      }
    }
  }
  return nullptr;
}

// ----------------------------------------------------------------------------
// Description:  domesticFormula
// ----------------------------------------------------------------------------

void
PfcItem::domesticFormula(PricingTrx& trx, TaxResponse& taxResponse)
{
  std::vector<TravelSeg*>::const_iterator travelSegI;
  travelSegI = taxResponse.farePath()->itin()->travelSeg().begin();

  for (; travelSegI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegI++)
  {
    if (!fallback::fallbackPFCOvercollectionInRTItinMax4(&trx) && _roundTrip)
    {
      const FareUsage* fu = findFareUsage(**travelSegI, taxResponse);
      if (fu && fu->isInbound())
      {
        --travelSegI; // pointing to last seg of O/B
        break;
      }
    }

    if (!isValidPfc(trx, taxResponse, **travelSegI))
      continue;

    initializePfc(trx, taxResponse, **travelSegI);

    if (taxResponse.pfcItemVector().size() == 2)
      break;
  }

  std::vector<TravelSeg*>::const_iterator travelSegEndI;
  travelSegEndI = taxResponse.farePath()->itin()->travelSeg().end();

  if ((!_roundTrip) || (travelSegI == travelSegEndI))
    return;

  travelSegEndI--;

  TravelSeg* travelSeg1 = nullptr;
  TravelSeg* travelSeg2 = nullptr;

  for (; travelSegI != travelSegEndI; travelSegEndI--)
  {
    if (!fallback::fallbackPFCOvercollectionInRTItinMax4(&trx))
    {
      const FareUsage* fu = findFareUsage(**travelSegEndI, taxResponse);
      if (fu && fu->isOutbound())
        break;
    }

    if (!isValidPfc(trx, taxResponse, **travelSegEndI))
      continue;

    if (!travelSeg2)
    {
      travelSeg2 = *travelSegEndI;
      continue;
    }

    travelSeg1 = *travelSegEndI;
    break;
  }

  if (travelSeg1)
    initializePfc(trx, taxResponse, *travelSeg1);

  if (travelSeg2)
    initializePfc(trx, taxResponse, *travelSeg2);
}

// ----------------------------------------------------------------------------
// Description:  internationalFormula
// ----------------------------------------------------------------------------

void
PfcItem::internationalFormula(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              const Loc* pointOfSaleLocation)
{
  const AirSeg* airSeg;
  std::vector<TravelSeg*>::const_iterator travelSegEndI;
  std::vector<TravelSeg*>::const_iterator travelSegI;

  CarrierCode validatingCarrier = taxResponse.farePath()->itin()->validatingCarrier();

  if (validatingCarrier.empty())
  {
    bool area1 = false;
    // bool area2 = false;
    bool area3 = false;

    travelSegI = taxResponse.farePath()->itin()->travelSeg().begin();

    for (; travelSegI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegI++)
    {
      airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

      if (!airSeg)
        continue;

      if (((*travelSegI)->origin()->area() == IATA_AREA1) ||
          ((*travelSegI)->destination()->area() == IATA_AREA1))
        area1 = true;

      // if ( ((*travelSegI)->origin()->area() == IATA_AREA2) ||
      //     ((*travelSegI)->destination()->area() == IATA_AREA2) )
      //   area2 = true;
      // Never used

      if (((*travelSegI)->origin()->area() == IATA_AREA3) ||
          ((*travelSegI)->destination()->area() == IATA_AREA3))
        area3 = true;
    }

    travelSegI = taxResponse.farePath()->itin()->travelSeg().begin();

    for (; travelSegI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegI++)
    {
      airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

      if (!airSeg)
        continue;

      validatingCarrier = airSeg->marketingCarrierCode();

      if (area1 || area3)
      {
        if (((*travelSegI)->origin()->area() == IATA_AREA1) ||
            ((*travelSegI)->destination()->area() == IATA_AREA1))
          break;

        continue;
      }

      if ((*travelSegI)->origin()->area() != (*travelSegI)->destination()->area())
        break;
    }
  }

  Indicator option = 0;

  const std::vector<PfcCollectMeth*>& pfcCollectMeth =
      trx.dataHandle().getPfcCollectMeth(validatingCarrier, trx.getRequest()->ticketingDT());

  std::vector<PfcCollectMeth*>::const_iterator pfcCollectMethI = pfcCollectMeth.begin();

  for (; pfcCollectMethI != pfcCollectMeth.end(); pfcCollectMethI++)
  {
    option = (*pfcCollectMethI)->collectOption();

    std::vector<PfcCollectExcpt*>::const_iterator PfcCollectExcptI;
    PfcCollectExcptI = (*pfcCollectMethI)->excpts().begin();

    for (; PfcCollectExcptI != (*pfcCollectMethI)->excpts().end(); PfcCollectExcptI++)
    {
      if ((*PfcCollectExcptI)->nation() != pointOfSaleLocation->nation())
        continue;

      option = (*PfcCollectExcptI)->collectOption();

      break;
    }
  }
  //
  // PL#88308 For Collection Method 1 GUAM should charge PFC..
  //

  if (option == COLLECTION_TYPE_1)
  {
    std::vector<TravelSeg*>::const_reverse_iterator rit(
        taxResponse.farePath()->itin()->travelSeg().rbegin());
    std::vector<TravelSeg*>::const_reverse_iterator ritend(
        taxResponse.farePath()->itin()->travelSeg().rend());

    for (; rit != ritend; ++rit)
    {
      airSeg = dynamic_cast<const AirSeg*>(*rit);

      if (!airSeg)
        continue;

      if (LocUtil::isUS(*(*rit)->origin()) && LocUtil::isUS(*(*rit)->destination()))
        continue;

      if (!LocUtil::isUS(*(*rit)->origin()) && !LocUtil::isUS(*(*rit)->destination()))
      {
        if (LIKELY(!LocUtil::isUSPossession(*(*rit)->origin())))
          continue;
      }

      if (!LocUtil::isUS(*(*rit)->origin()) && LocUtil::isUS(*(*rit)->destination()))
      {
        if (!LocUtil::isUSPossession(*(*rit)->origin()))
          continue;
      }

      if (*rit != taxResponse.farePath()->itin()->travelSeg().back())
      {
        if ((*rit)->destination()->nation() != (*(rit - 1))->origin()->nation())
          continue;
      }

      if (!isValidPfc(trx, taxResponse, **rit))
        continue;

      initializePfc(trx, taxResponse, **rit);
      return;
    }
    return;
  }

  if (option == COLLECTION_TYPE_2)
    domesticFormula(trx, taxResponse);
}

// ----------------------------------------------------------------------------
// Description:  isValidPfc
// ----------------------------------------------------------------------------

bool
PfcItem::isValidPfc(PricingTrx& trx, TaxResponse& taxResponse, TravelSeg& travelSeg)
{
  const AirSeg* airSeg = dynamic_cast<const AirSeg*>(&travelSeg);

  if (!airSeg)
    return false;

  // CR Request to Allow for Intra Hawaii PFC From Gary Nash
  //
  //  if ( (LocUtil::isHawaii(*travelSeg.origin())) &&
  //       (LocUtil::isHawaii(*travelSeg.destination())) )
  //     return false;

  //  if ( (LocUtil::isUSPossession(*travelSeg.origin())) ||
  //       (LocUtil::isUSTerritoryOnly(*travelSeg.origin())) )
  //     return false;

  if (!_bypassTktDes)
  {
    std::string designator = EMPTY_STRING();

    if (UNLIKELY(trx.getRequest()->isSpecifiedTktDesignatorEntry()))
    {
      designator = trx.getRequest()->specifiedTktDesignator(travelSeg.segmentOrder()).c_str();
    }

    CarrierCode governingCarrier = EMPTY_STRING();
    TaxTrx* taxTrx = dynamic_cast<TaxTrx*>(&trx);

    if (LIKELY(!taxTrx))
    {
      FareUsage* fareUsage = nullptr;
      if (!fallback::fallbackPFCOvercollectionInRTItinMax4(&trx))
      {
        fareUsage = findFareUsage(travelSeg, taxResponse);
      }
      else
      {
        Itin* itin = taxResponse.farePath()->itin();
        FarePath* farePath = taxResponse.farePath();

        std::vector<PricingUnit*>::iterator puI;
        std::vector<FareUsage*>::iterator fuI;
        std::vector<TravelSeg*>::const_iterator tvlSegI;

        for (puI = farePath->pricingUnit().begin(); puI != farePath->pricingUnit().end(); puI++)
        {
          for (fuI = (*puI)->fareUsage().begin(); fuI != (*puI)->fareUsage().end(); fuI++)
          {
            for (tvlSegI = (*fuI)->travelSeg().begin(); tvlSegI != (*fuI)->travelSeg().end();
                 tvlSegI++)
            {
              if (itin->segmentOrder(*tvlSegI) == itin->segmentOrder(&travelSeg))
              {
                fareUsage = (*fuI);
                break;
              }
            } // TvlSeg Loop
          } // FU Loop
        } // PU Loop
      }

      if (fareUsage)
      {
        if (LIKELY(designator.empty()))
          fareUsage->paxTypeFare()->createTktDesignator(designator);

        governingCarrier = fareUsage->paxTypeFare()->carrier();

        if (governingCarrier != "YY")
          governingCarrier = fareUsage->paxTypeFare()->fareMarket()->governingCarrier();
      }
    }

    if (!designator.empty())
    {
      while (1)
      {
        const std::vector<const PfcTktDesigExcept*>& pfcTktDesigExcept =
            trx.dataHandle().getPfcTktDesigExcept(governingCarrier,
                                                  trx.getRequest()->ticketingDT());

        if (!pfcTktDesigExcept.empty())
        {
          std::vector<const PfcTktDesigExcept*>::const_iterator pfcTktDesigExceptIter =
              pfcTktDesigExcept.begin();
          std::vector<const PfcTktDesigExcept*>::const_iterator pfcTktDesigExceptEndIter =
              pfcTktDesigExcept.end();

          for (; pfcTktDesigExceptIter != pfcTktDesigExceptEndIter; pfcTktDesigExceptIter++)
          {
            FareClassCode taxRestrTktDsgs = (*pfcTktDesigExceptIter)->tktDesignator().c_str();
            FareClassCode tktDesignator = designator.c_str();

            if (UNLIKELY(RuleUtil::matchFareClass(taxRestrTktDsgs.c_str(), tktDesignator.c_str())))
            {
              TaxDiagnostic::collectErrors(
                  trx, taxResponse, TaxDiagnostic::TICKET_DESIGNATOR, Diagnostic809);

              LOG4CXX_DEBUG(_logger, "PFC Ticket Designator Exemption : " << tktDesignator);
              return false;
            }
          }
        }

        if (!governingCarrier.empty())
          governingCarrier = EMPTY_STRING();
        else
          break;
      }
    }
  } // end !_bypassTktDes

  const PfcPFC* pPfc =
      trx.dataHandle().getPfcPFC(travelSeg.origin()->loc(), trx.getRequest()->ticketingDT());

  if (pPfc == nullptr)
  {
    LOG4CXX_DEBUG(_logger,
                  "PFC Information Not Available for Location : " << travelSeg.origin()->loc());

    TaxDiagnostic::collectErrors(
        trx, taxResponse, TaxDiagnostic::BAD_DATA_HANDLER_POINTER, Diagnostic840);

    return false;
  }

  if (pPfc->pfcAmt1() == 0.0)
    return false;

  std::vector<PfcCxrExcpt*>::const_iterator pfcCxrExcptI;

  for (pfcCxrExcptI = pPfc->cxrExcpts().begin(); pfcCxrExcptI != pPfc->cxrExcpts().end();
       pfcCxrExcptI++)
  {
    if ((*pfcCxrExcptI)->excpCarrier() != airSeg->marketingCarrierCode())
      continue;

    if (((*pfcCxrExcptI)->flt2() == 0) &&
        ((*pfcCxrExcptI)->flt1() == airSeg->marketingFlightNumber()))
      return false;

    if (((*pfcCxrExcptI)->flt1() > airSeg->marketingFlightNumber()) ||
        (airSeg->marketingFlightNumber() > (*pfcCxrExcptI)->flt2()))
      continue;

    return false;
  }

  const std::vector<PfcEssAirSvc*>& pfcEssAirSvc = trx.dataHandle().getPfcEssAirSvc(
      travelSeg.origin()->loc(), travelSeg.destination()->loc(), trx.getRequest()->ticketingDT());

  if (!pfcEssAirSvc.empty())
  {
    std::vector<PfcEssAirSvc*>::const_iterator pfcEssAirSvcI;
    pfcEssAirSvcI = pfcEssAirSvc.begin();

    if ((*pfcEssAirSvcI)->segCnt() == 0)
      return false;

    std::vector<PfcEssAirSvcProv*>::const_iterator pfcEssAirSvcProvI;

    for (pfcEssAirSvcProvI = (*pfcEssAirSvcI)->asProvs().begin();
         pfcEssAirSvcProvI != (*pfcEssAirSvcI)->asProvs().end();
         pfcEssAirSvcProvI++)
    {
      if ((*pfcEssAirSvcProvI)->easCarrier() == DOLLAR_CARRIER)
        return false;

      if ((*pfcEssAirSvcProvI)->easCarrier() == airSeg->marketingCarrierCode())
      {
        if (((*pfcEssAirSvcProvI)->flt1() == 0) && ((*pfcEssAirSvcProvI)->flt2() == 0))
          return false;

        if (((*pfcEssAirSvcProvI)->flt2() == 0) &&
            ((*pfcEssAirSvcProvI)->flt1() == airSeg->marketingFlightNumber()))
          return false;

        if (((*pfcEssAirSvcProvI)->flt1() > airSeg->marketingFlightNumber()) ||
            (airSeg->marketingFlightNumber() > (*pfcEssAirSvcProvI)->flt2()))
          continue;

        return false;
      }
    }
  }

  if (LocUtil::isAlaska(*travelSeg.origin()))
  {
    const PfcEquipTypeExempt* pPfcEquipTypeExempt = trx.dataHandle().getPfcEquipTypeExempt(
        travelSeg.equipmentType(), "", trx.getRequest()->ticketingDT());

    if (pPfcEquipTypeExempt)
      return false;
  }

  return true;
}

// ----------------------------------------------------------------------------
// Description:  initializePfc
// ----------------------------------------------------------------------------

void
PfcItem::initializePfc(PricingTrx& trx, TaxResponse& taxResponse, TravelSeg& travelSeg)
{

  if (isInfantExempt(trx, taxResponse, travelSeg))
    return;

  PfcItem* pPfcItem = nullptr;
  trx.dataHandle().get(pPfcItem);

  if (pPfcItem == nullptr)
  {
    LOG4CXX_WARN(_logger, "NO MEMORY AVAILABLE");

    TaxDiagnostic::collectErrors(
        trx, taxResponse, TaxDiagnostic::BAD_DATA_HANDLER_POINTER, Diagnostic820);
    return;
  }

  if (trx.snapRequest() && travelSeg.isAir())
  {
    AirSeg* const as = static_cast<AirSeg*>(&travelSeg);

    if (as)
    {
      _carrierCode = as->marketingCarrierCode();
      pPfcItem->carrierCode() = _carrierCode;
    }
    else
    {
      LOG4CXX_ERROR(_logger, "Air segment is NULL.");
    }
  }

  _legId = travelSeg.legId();
  pPfcItem->legId() = travelSeg.legId();
  pPfcItem->_travelSeg = &travelSeg;

  const PfcPFC* pPfc =
      trx.dataHandle().getPfcPFC(travelSeg.origin()->loc(), trx.getRequest()->ticketingDT());

  if (pPfc == nullptr)
  {
    LOG4CXX_WARN(_logger, "Bad PFC Data Encountered");

    TaxDiagnostic::collectErrors(
        trx, taxResponse, TaxDiagnostic::BAD_DATA_HANDLER_POINTER, Diagnostic840);

    return;
  }

  Money moneyPayment(pPfc->pfcCur1());

  // lint --e{413}
  pPfcItem->_pfcAmt = pPfc->pfcAmt1();
  pPfcItem->_pfcNumDec = moneyPayment.noDec(trx.ticketingDate());
  pPfcItem->_pfcCurrency = pPfc->pfcCur1();
  pPfcItem->_couponNumber = pPfc->segCnt();
  pPfcItem->_pfcAirport = pPfc->pfcAirport();

  uint8_t locType = setPfcLocType(trx, taxResponse);

  pPfcItem->_absorptionInd = false;

  PfcAbsorption pfcAbsorption;
  pPfcItem->_absorptionInd = pfcAbsorption.pfcAbsorptionApplied(
      trx, taxResponse, travelSeg, pPfcItem->_pfcCurrency, pPfcItem->_pfcAmt, locType);

  if (TrxUtil::isAutomaticPfcTaxExemptionEnabled(trx) &&
      AutomaticPfcTaxExemption::isAutomaticPfcExemtpionEnabled(trx, taxResponse) &&
      taxResponse.automaticPFCTaxExemptionData()->pfcExemptionOption() ==
          AutomaticPfcTaxExemptionData::EXEMPT_ALL)
  {
    DiagManager diagMgr(trx, Diagnostic807);
    if (diagMgr.isActive())
    {
      diagMgr.collector() << std::setw(14) << std::fixed << std::setprecision(pPfcItem->_pfcNumDec)
                          << pPfcItem->_pfcAmt << ":" << pPfcItem->_pfcCurrency << std::setw(14)
                          << std::fixed << std::setprecision(pPfcItem->_pfcNumDec)
                          << pPfcItem->_pfcAmt << " *" << pPfc->pfcAirport() << "*\n";

      diagMgr.collector().flushMsg();
    }
  }

  if (taxResponse.taxItemVector().empty() && taxResponse.pfcItemVector().empty())
    addUniqueTaxResponse(taxResponse, trx);

  taxResponse.pfcItemVector().push_back(pPfcItem);
}

// ---------------------------------------------------------------------------
// Description:  isInfantExempt
// ----------------------------------------------------------------------------

bool
PfcItem::isInfantExempt(PricingTrx& trx, TaxResponse& taxResponse, TravelSeg& travelSeg)
{
  if (!taxResponse.farePath()->paxType()->paxTypeInfo()->isInfant())
    return false;

  int16_t currentSegmentOrder = travelSeg.segmentOrder();

  FarePath* farePath = taxResponse.farePath();
  std::vector<PricingUnit*>::const_iterator puI;
  std::vector<FareUsage*>::iterator fuI;
  std::vector<TravelSeg*>::const_iterator tsI;

  const bool isTaxShoppingTrx = TrxUtil::isShoppingTaxRequest(&trx) && !fallback::fixed::taxShoppingPfcInfant();
  for (puI = farePath->pricingUnit().begin(); puI != farePath->pricingUnit().end(); puI++)
  {
    for (fuI = (*puI)->fareUsage().begin(); fuI != (*puI)->fareUsage().end(); fuI++)
    {

      PaxTypeCode paxTypeCode = (*fuI)->paxTypeFare()->fcasPaxType();
      if (isTaxShoppingTrx)
        paxTypeCode = (*fuI)->paxTypeFare()->actualPaxType()->paxType();

      bool isInfant = PaxTypeUtil::isInfant(trx, paxTypeCode, (*fuI)->paxTypeFare()->vendor());
      if (!isInfant)
        continue;

      for (tsI = (*fuI)->travelSeg().begin(); tsI != (*fuI)->travelSeg().end(); tsI++)
      {
        if ((*tsI)->segmentOrder() == currentSegmentOrder)
        {

          if ((*fuI)->paxTypeFare()->nucFareAmount() == 0)
            return true;

          try
          {
            const DiscountInfo& discountInfo = (*fuI)->paxTypeFare()->discountInfo();

            if (discountInfo.category() == 19 &&
                discountInfo.discPercent() <= INFANT_DISCOUNT_PERCENTAGE)
              return true;
            else
              return false;
          }
          catch (TSEException& e) { return false; }
        }
      }
    } // End of FOR FU
  } // End of FOR PU

  return false;
}

