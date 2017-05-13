#include "Diagnostic/Diag922Collector.h"

#include "Common/ClassOfService.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/ShoppingTrx.h"
#include "DBAccess/Loc.h"

#include <iomanip>

namespace tse
{
void
Diag922Collector::printFareMarketHeader(const ShoppingTrx& trx)
{
  *this << std::setw(5) << "CXR" << std::setw(8) << "BRD CTY" << std::setw(8) << "OFF CTY"
        << std::setw(8) << "BRD A/P" << std::setw(8) << "OFF A/P" << std::setw(10) << "DIRECTION"
        << std::setw(12) << "TVL DATE" << std::setw(17) << "TVL TYPE" << std::setw(12)
        << "LOCAL/THRU" << std::setw(14) << "CUSTOM SOP FM";

  if (trx.isThroughFarePrecedencePossible())
    *this << std::setw(4) << "TFP";

  if (trx.getOptions()->getSpanishLargeFamilyDiscountLevel() != SLFUtil::DiscountLevel::NO_DISCOUNT)
  {
    *this << std::setw(11) << "SPAIN_DISC";
  }

  *this << '\n';
}

void
Diag922Collector::printFareMarket(const ShoppingTrx& trx, const FareMarket& fareMarket)
{
  Diag922Collector::printFareMarket(*this, trx, fareMarket);
}

void
Diag922Collector::printFareMarket(std::ostream& out,
                                  const ShoppingTrx& trx,
                                  const FareMarket& fareMarket)
{
  const char* thruOrLocal = "";

  if (FareMarket::SOL_FM_THRU == fareMarket.getFmTypeSol())
  {
    thruOrLocal = "THRU";
  }
  else if (FareMarket::SOL_FM_LOCAL == fareMarket.getFmTypeSol())
  {
    thruOrLocal = "LOCAL";
  }

  const std::string geoTravelTypeString = DiagnosticUtil::geoTravelTypeToString(fareMarket.geoTravelType());

  out.setf(std::ios::left, std::ios::adjustfield);
  out << std::setw(5) << fareMarket.governingCarrier() << std::setw(8)
      << fareMarket.boardMultiCity() << std::setw(8) << fareMarket.offMultiCity() << std::setw(8)
      << fareMarket.origin()->loc() << std::setw(8) << fareMarket.destination()->loc()
      << std::setw(10) << fareMarket.getDirectionAsString() << std::setw(12)
      << fareMarket.travelDate().dateToString(DDMMMYYYY, "") << std::setw(17)
      << geoTravelTypeString << std::setw(12) << thruOrLocal
      << std::setw(14) << (trx.isCustomSolutionFM(&fareMarket) ? "Yes" : "No");

  if (trx.isThroughFarePrecedencePossible())
    out << std::setw(4) << (fareMarket.isThroughFarePrecedenceNGS() ? "Yes" : "No");

  if (trx.getOptions()->getSpanishLargeFamilyDiscountLevel() != SLFUtil::DiscountLevel::NO_DISCOUNT)
  {
    out << std::setw(11) << (trx.isSpanishDiscountFM(&fareMarket) ? "Yes" : "No");
  }

  out << '\n';
}

void
Diag922Collector::printFareMarketMultiAirPort(const FareMarket& fareMarket)
{
  const FareMarket::MultiAirportInfo* multiAirportInfo = fareMarket.getMultiAirportInfo();
  if (!multiAirportInfo)
  {
    return;
  }

  *this << "origin\n";
  for (const auto& elem : multiAirportInfo->origin())
  {
    *this << elem << " ";
  }

  *this << "\ndestination\n";
  for (const auto& elem : multiAirportInfo->destination())
  {
    *this << elem << " ";
  }

  *this << "\n";
}

void
Diag922Collector::printSchedules(ShoppingTrx& trx,
                                 int sopIndex,
                                 const Itin& itin,
                                 const SOPUsage& usage)
{
  *this << "SOP:" << sopIndex << " ORIG SOP:" << usage.origSopId_
        << " START:" << usage.startSegment_ << " END:" << usage.endSegment_
        << " FLIGHTS FIRST PROCESSED IN SOP:" << usage.flightsFirstProcessedIn_
        << " DURATION:" << itin.getFlightTimeMinutes() << '\n';

  int segIdx = 0;
  for (const TravelSeg* tvlSeg : itin.travelSeg())
  {
    const int start = usage.startSegment_;
    const int end = usage.endSegment_;
    if (tvlSeg->isAir())
    {
      const AirSeg* airSeg = static_cast<const AirSeg*>(tvlSeg);
      char carrierType('-');
      if (airSeg->flowJourneyCarrier())
      {
        carrierType = 'F';
      }
      else if (airSeg->localJourneyCarrier())
      {
        carrierType = 'L';
      }
      *this << std::setw(2) << carrierType << std::setw(3) << airSeg->marketingCarrierCode()
            << std::setw(3) << airSeg->operatingCarrierCode() << std::setw(5)
            << airSeg->flightNumber() << std::setw(4) << airSeg->origin()->loc() << std::setw(4)
            << airSeg->destination()->loc() << std::setw(10)
            << airSeg->departureDT().dateToString(DDMMMYYYY, "") << std::setw(7)
            << airSeg->departureDT().timeToString(HHMM, "") << std::setw(10)
            << airSeg->arrivalDT().dateToString(DDMMMYYYY, "") << std::setw(7)
            << airSeg->arrivalDT().timeToString(HHMM, "") << '\n';
      if (segIdx >= start && segIdx <= end)
      {
        *this << "AVL:";
        for (const ClassOfService* cos : *(usage.cos_[segIdx - start]))
        {
          *this << cos->cabin().getCabinIndicator() << '/' << cos->bookingCode() << '|'
                << cos->numSeats() << ' ';
        }
        *this << '\n';
      }
    }
    else
    {
      *this << "segmentType:" << static_cast<char>(tvlSeg->segmentType()) << ",carrier:"
            << "N/A"
            << ",origin:" << tvlSeg->origin()->loc()
            << ",destination:" << tvlSeg->destination()->loc()
            << ",departureDT:" << tvlSeg->departureDT() << ",arrivalDT:" << tvlSeg->arrivalDT()
            << '\n';
    }
    ++segIdx;
  }

  *this << '\n';
}

} // namespace tse
