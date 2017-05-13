//----------------------------------------------------------------------------
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Diagnostic/Diag192Collector.h"

#include "Common/ItinUtil.h"
#include "Common/RtwUtil.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/BaseExchangeTrx.h"
#include "DataModel/ExcItin.h"
#include "DBAccess/AirlineAllianceCarrierInfo.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/InternalDiagUtil.h"

#include <string>

namespace tse
{

void
Diag192Collector::printTrx()
{
  *this << "**************** ITIN ANALYZER DIAGNOSTICS 192 ****************\n";

  // 192 should be called only for PricingTrx derived !
  const PricingTrx& prTrx = static_cast<const PricingTrx&>(*trx());

  if (prTrx.excTrxType() == PricingTrx::AR_EXC_TRX)
  {
    const std::string& whichItin = prTrx.diagnostic().diagParamMapItem(Diagnostic::ITIN_TYPE);

    if (whichItin != "NEW")
    {
      *this << "\n";
      *this << "************************ EXCHANGE ITIN ************************\n";
      const BaseExchangeTrx& excTrx = static_cast<const BaseExchangeTrx&>(*trx());
      printItin(*excTrx.exchangeItin().front());
    }

    if (whichItin == "EXC")
      return;

    *this << "************************** NEW ITIN ***************************\n";
    *this << "\n";
  }

  printItin(*prTrx.itin().front());
}

void
Diag192Collector::printTravelSegments(PricingTrx& prTrx, const Itin& itin)
{
  InternalDiagUtil idu(prTrx, *this);

  for (const TravelSeg* ts : itin.travelSeg())
  {
    if (RtwUtil::isRtwArunk(prTrx, ts))
      continue;

    idu.printSegmentInfo(*ts);
    idu.addIataArea(*ts);

    const AirSeg* airSeg = ts->toAirSeg();
    if (airSeg)
    {
      *this << "\n   CARRIER: " << airSeg->carrier();
      *this << " OPERATING: " << airSeg->operatingCarrierCode();
      *this << " MARKETING: " << airSeg->marketingCarrierCode();
    }
    *this << "\n";
  }
  this->printLine();
}

void
Diag192Collector::printRtwArunk(const PricingTrx& prTrx, const Itin& itin)
{
  if (itin.travelSeg().empty())
    return;

  const TravelSeg& tsBack = *itin.travelSeg().back();
  if (!RtwUtil::isRtwArunk(prTrx, &tsBack))
    return;

  const LocCode& org = tsBack.origin()->loc();
  const LocCode& dst = tsBack.destination()->loc();
  const FareMarket* fm = RtwUtil::getFirstRtwFareMarket(itin);
  const CarrierCode& cxr = fm ? fm->governingCarrier() : EMPTY_CARRIER;
  if (LocUtil::isSameMultiTransportCity(org, dst, cxr, itin.geoTravelType(), itin.travelDate()))
    return;

  *this << "\nSURFACE SEGMENT WAS ADDED AT THE END FOR RW:\n";
  InternalDiagUtil idu(prTrx, *this);
  idu.printSegmentInfo(tsBack);
  idu.addIataArea(tsBack);
  *this << "\n";
  this->printLine();
}

void
Diag192Collector::printRtwFareMarkets(PricingTrx& prTrx, const Itin& itin)
{
  if (!RtwUtil::isRtw(prTrx))
    return;

  for (const FareMarket* fm : itin.fareMarket())
  {
    if (!RtwUtil::isRtwFareMarket(itin, fm))
      continue;

    const CarrierCode& cxr = fm->governingCarrier();
    *this << "FM GOVERNING CARRIER: " << cxr << "\n";
    *this << "RTW ACTIVATED: " << (prTrx.isCustomerActivatedByFlag("RTW", &cxr) ? "Y" : "N")
          << "\n";

    const std::vector<AirlineAllianceCarrierInfo*>& airlineAllianceCarrierInfo =
        prTrx.dataHandle().getAirlineAllianceCarrier(cxr);

    for (const AirlineAllianceCarrierInfo* aaci : airlineAllianceCarrierInfo)
    {
      *this << "CARRIER ALLIANCE: " << aaci->genericName();
      *this << "\nCARRIER ALLIANCE CODE: " << aaci->genericAllianceCode();

      if (aaci->genericAllianceCode() == ONE_WORLD_ALLIANCE)
        *this << "\nNUMBER OF CONTINENTS: " << ItinUtil::countContinents(prTrx, *fm, *aaci);

      *this << "\n";
    }
  }
}

void
Diag192Collector::printItin(Itin& itin)
{
  // 192 should be called only for PricingTrx derrived !
  PricingTrx& prTrx = static_cast<PricingTrx&>(*trx());

  printTravelSegments(prTrx, itin);
  printRtwArunk(prTrx, itin);

  InternalDiagUtil idu(prTrx, *this);
  idu.addTripCharacteristic(itin);
  *this << "\nVALIDATING CARRIER: " << itin.validatingCarrier() << "\n";
  printRtwFareMarkets(prTrx, itin);
  this->printLine();

  *this << "\nTURNAROUND/FURTHEST POINT DETERMINATION\n";
  ItinUtil::gcmBasedFurthestPoint(itin, this);
  this->printLine();
}

} // namespace tse
