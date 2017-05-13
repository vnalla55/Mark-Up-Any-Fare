
#include "Diagnostic/Diag986Collector.h"

#include "Common/TseCodeTypes.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ExcItin.h"
#include "DataModel/RexShoppingTrx.h"

#include <iomanip>
#include <iostream>

namespace tse
{
Diag986Collector& Diag986Collector::operator<<(RexShoppingTrx& trx)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;
    dc << "***************** START DIAG 986 **************************\n";
    dc << "************* REC 3 CAT 31 CONSTRAINTS ********************\n";
    dc << "***********************************************************\n";
    RexShoppingTrx::OADDataMap::const_iterator oadIt = trx.oadData().begin();

    for (; oadIt != trx.oadData().end(); ++oadIt)
    {
      const FareMarket* fareMarket = oadIt->first;
      const RexShoppingTrx::R3SeqsConstraintMap& oadData = oadIt->second;

      dc << "ORG-DST: " << fareMarket->origin()->loc() << "-" << fareMarket->destination()->loc()
         << "\n";

      const std::vector<TravelSeg*>& segments = fareMarket->travelSeg();

      printSegments(segments);

      if (!oadData.empty())
      {
        for (const auto& rec3 : oadData)
        {
          dc << "\nR3 ITEM NUMBER: " << rec3.first;

          for (const auto& seqConstraint : rec3.second)
          {
            dc << "\n  RESTRICTIONS FOR RANGE: " << seqConstraint.calendarRange;
            dc << "\n   RANGE APPLICATION: "
               << ExchShopCalendar::getDateApplicationInString(seqConstraint.calendarAppl);

            RexShoppingTrx::PortionMergeTvlVectType portionMerge = seqConstraint.portionMerge;
            dc << "\n  PORTION REST: ";
            if (!portionMerge.empty())
            {
              copy(portionMerge.begin(), portionMerge.end(), std::ostream_iterator<int>(dc, " "));
            }

            const std::set<LocCode>& forcedConnections = seqConstraint.forcedConnection;
            dc << "\n  FORCED CONX: ";
            if (!forcedConnections.empty())
            {
              copy(forcedConnections.begin(),
                   forcedConnections.end(),
                   std::ostream_iterator<LocCode>(dc, " "));
            }

            dc << "\n  FIRST BREAK REST: " << ((seqConstraint.firstBreakStatus) ? "Y" : "N");

            dc << "\n  FARE BYTE CXR APPL TBL990 REST: ";
            const RexShoppingTrx::FareByteCxrAppl fareByteCxrAppl = seqConstraint.fareByteCxrAppl;
            dc << "\n   REST CXR: ";
            if (!fareByteCxrAppl.restCxr.empty())
            {
              copy(fareByteCxrAppl.restCxr.begin(), fareByteCxrAppl.restCxr.end(),
                   std::ostream_iterator<CarrierCode>(dc, " "));
            }
            dc << "\n   APPL CXR: ";
            if (!fareByteCxrAppl.applCxr.empty())
            {
              copy(fareByteCxrAppl.applCxr.begin(),
                   fareByteCxrAppl.applCxr.end(),
                   std::ostream_iterator<CarrierCode>(dc, " "));
            }

            dc << "\n  FLIGHT NUMBER REST: " << ((seqConstraint.flightNumberRestriction) ? "Y"
                                                                                         : "N");
            dc << "\n  OUTBOUND PORTION REST: ";

            RexShoppingTrx::PortionMergeTvlVectType outboundPortion = seqConstraint.outboundPortion;
            if (!outboundPortion.empty())
            {
              copy(outboundPortion.begin(),
                   outboundPortion.end(),
                   std::ostream_iterator<int>(dc, " "));
            }
          }
        }
      }
      else
      {
        dc << " NO DATA COLLECTED";
      }
      RexShoppingTrx::OADResponseDataMap& oadMergedR3Data = trx.oadResponse();
      const auto& oadMergedResponses = oadMergedR3Data[fareMarket];

      for (const auto& oadMergedResponse : oadMergedResponses)
      {
        dc << "\n\nMERGED CAT31 REC3 CONSTRAINTS: ";

        dc << "\n  RESTRICTIONS FOR RANGE: " << oadMergedResponse.calendarRange;

        dc << "\n  PORTION REST: ";
        if (!oadMergedResponse.portion.empty())
        {
          copy(oadMergedResponse.portion.begin(), oadMergedResponse.portion.end(),
               std::ostream_iterator<int>(dc, " "));
        }

        dc << "\n  FORCED CONX: ";
        if (!oadMergedResponse.forcedConnections.empty())
        {
          copy(oadMergedResponse.forcedConnections.begin(),
               oadMergedResponse.forcedConnections.end(), std::ostream_iterator<LocCode>(dc, " "));
        }
        dc << "\n  FIRST BREAK RESTRICTED: " << ((oadMergedResponse.firstBreakRest) ? "Y" : "N");

        dc << "\n  FARE BYTE CXR APPL TBL990 CXR: ";
        if (!oadMergedResponse.fareByteCxrAppl.cxrList.empty())
        {
          copy(oadMergedResponse.fareByteCxrAppl.cxrList.begin(),
               oadMergedResponse.fareByteCxrAppl.cxrList.end(),
               std::ostream_iterator<CarrierCode>(dc, " "));
        }
        dc << "\n   EXCLUDE: " << ((oadMergedResponse.fareByteCxrAppl.excluded) ? "Y" : "N");

        dc << "\n  FLIGHT NUMBER REST: "
           << ((oadMergedResponse.flightNumberRestriction) ? "Y" : "N");
        dc << "\n  OUTBOUND PORTION REST: ";
        if (!oadMergedResponse.outboundPortion.empty())
        {
          copy(oadMergedResponse.outboundPortion.begin(), oadMergedResponse.outboundPortion.end(),
               std::ostream_iterator<int>(dc, " "));
        }
      }

      dc << "\n***********************************************************\n";
    }
  }
  return *this;
}

void Diag986Collector::printSegments(const std::vector<TravelSeg*>& segments)
{
  DiagCollector& dc = (DiagCollector&)*this;

  std::vector<TravelSeg*>::const_iterator tvlSeg = segments.begin();
  for (; tvlSeg != segments.end(); ++tvlSeg)
  {
    AirSeg* tvlSegA = dynamic_cast<AirSeg*>(*tvlSeg);
    dc << std::setw(2);
    dc << (*tvlSeg)->pnrSegment();
    if (tvlSegA)
    {
      dc << std::setw(3) << tvlSegA->carrier();
      if (tvlSegA->segmentType() == Open)
        dc << "OPEN";
      else
        dc << std::setw(4) << tvlSegA->flightNumber();
      dc << tvlSegA->getBookingCode();
    }
    else
      dc << std::setw(5) << "ARNK"
         << "   ";

    dc << std::setw(8);

    if ((*tvlSeg)->segmentType() == Open)
    {
      std::string sDepartureDate = (*tvlSeg)->pssDepartureDate();

      if (sDepartureDate != "")
      {
        sDepartureDate += " 12:00";
        DateTime dtDepartureDate = DateTime(sDepartureDate);
        dc << dtDepartureDate.dateToString(DDMMMYY, "");
      }
      else
        dc << "NONE";
    }
    else
      dc << (*tvlSeg)->departureDT().dateToString(DDMMMYY, "");

    dc << std::setw(4);

    dc << (*tvlSeg)->origAirport() << (*tvlSeg)->destAirport();
    dc << std::endl;
  }
}
}
