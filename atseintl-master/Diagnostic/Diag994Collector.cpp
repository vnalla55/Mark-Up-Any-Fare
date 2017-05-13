#include "Diagnostic/Diag994Collector.h"

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"

using namespace std;

namespace tse
{
void
Diag994Collector::displayItins(PricingTrx& trx, const bool processingSwitched)
{
  Diag994Collector& dc = *this;
  dc << "***************** START DIAG 994 ************************** \n" << endl;

  if (processingSwitched)
    dc << "  PROCESSING SWITCHED TO ONEWAY\n" << endl;
  else
    displayItinsDetails(trx);

  dc << "***************** END DIAG 994 ************************** \n" << endl;
}

void
Diag994Collector::displayItinsDetails(PricingTrx& trx)
{
  Diag994Collector& dc = *this;

  dc << "NUMBER OF COMBINED ITINS : " << trx.itin().size() << endl;
  dc << "NUMBER OF OUTBOUNDS: " << trx.subItinVecOutbound().size() << endl;
  dc << "NUMBER OF INBOUNDS: " << trx.subItinVecInbound().size() << endl;

  uint16_t itinId = 1;
  bool brandedFareEntry = trx.getRequest()->brandedFareEntry();

  for (Itin* curItin : trx.itin())
  {
    dc << "OPTION " << itinId << "\n";

    if (brandedFareEntry)
      dc << "SOLUTIONS FOR BRANDS: ";

    set<uint16_t> brandIds;
    for (FarePath* fp : curItin->farePath())
    {
      if (brandedFareEntry && brandIds.count(fp->brandIndex()) == 0)
      {
        brandIds.insert(fp->brandIndex());
        dc << trx.getRequest()->brandId(fp->brandIndex()) << "  ";
      }
    }
    dc << "\n";

    size_t fltIndex = 1;

    for (TravelSeg* seg : curItin->travelSeg())
    {
      const AirSeg* segPtr = dynamic_cast<const AirSeg*>(seg);
      if (segPtr == 0)
        continue;

      const AirSeg& aSeg = *segPtr;

      dc << std::setw(2) << fltIndex++ << " " << std::setw(3) << aSeg.carrier();
      dc << " ";
      dc << std::setw(4) << aSeg.flightNumber() << " ";
      const DateTime& depDT = aSeg.departureDT();
      const DateTime& arrDT = aSeg.arrivalDT();

      std::string depDTStr = depDT.timeToString(HHMM_AMPM, "");
      std::string arrDTStr = arrDT.timeToString(HHMM_AMPM, "");

      depDTStr = depDTStr.substr(0, depDTStr.length() - 1);
      arrDTStr = arrDTStr.substr(0, arrDTStr.length() - 1);
      dc << std::setw(2) << aSeg.getBookingCode() << " " << aSeg.bookedCabin().getCabinIndicator()
         << " ";
      dc << std::setw(5) << depDT.dateToString(DDMMM, "") << " ";
      dc << std::setw(3) << aSeg.origin()->loc() << "  " << std::setw(3)
         << aSeg.destination()->loc() << " ";
      dc << std::setw(5) << depDTStr << " ";
      dc << std::setw(5) << arrDTStr << " ";
      dc << "\n";

    } // end for seg

    if (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "SOLUTION")
    {
      uint16_t solutionIdx = 1;
      uint8_t fuCount = 0;

      for (FarePath* fp : curItin->farePath())
      {
        ostringstream outStr;
        dc << solutionIdx;
        dc << "\n";

        for (PricingUnit* pu : fp->pricingUnit())
        {
          for (FareUsage* fu : pu->fareUsage())
          {
            outStr << " " << setw(10) << fu->paxTypeFare()->fareClass();
            outStr << " " << setw(6) << fu->paxTypeFare()->fareAmount() << " " << setw(4)
                   << fu->paxTypeFare()->currency() << "\n";
            fuCount++;
          }
        }
        string fareInfo = outStr.str();
        outStr.flush();

        dc << fareInfo;
        dc << "\n";
        solutionIdx++;
      }
    }

    dc << "\n";
    itinId++;
  } // end for itin
}
} // end namespace
