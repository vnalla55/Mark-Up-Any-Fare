//----------------------------------------------------------------------------
//  File:        Diag989Collector.C
//
//  Copyright Sabre 2009
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
#include "Diagnostic/Diag989Collector.h"

#include "Common/FallbackUtil.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/RexShoppingTrx.h"
#include "DBAccess/ReissueSequence.h"
#include "DBAccess/VoluntaryChangesInfo.h"
#include "Rules/RuleUtil.h"

namespace tse
{
FALLBACK_DECL(exsCalendar)
FALLBACK_DECL(exscDiag989FixOndDisplay)

Diag989Collector&
Diag989Collector::operator<<(RexShoppingTrx& trx)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;
    dc << "***************************************************" << std::endl;
    dc << "Diagnostic 989 : Exchange Shopping response BEGIN" << std::endl;
    dc << "************* CONSOLIDATED  CONSTRAINS ************" << std::endl;
    dc << "***************************************************" << std::endl;

    if (trx.isShopOnlyCurrentItin())
      dc << "\nSHOP ONLY CURRENT ITINERARY\n";

    if (trx.oadConsRest().empty())
    {
      dc << "\nNO OAD DATA COLLECTED";
    }
    else
    {
      std::vector<TravelSeg*> excItinSeg;
      std::vector<TravelSeg*>::const_iterator excTravelSeg =
          trx.exchangeItin().front()->travelSeg().begin();
      for (; excTravelSeg != trx.exchangeItin().front()->travelSeg().end(); ++excTravelSeg)
      {
        if ((*excTravelSeg)->segmentType() != Arunk)
        {
          excItinSeg.push_back(*excTravelSeg);
        }
      }

      RexShoppingTrx::OADConsolidatedConstrainsVect::const_iterator oadConsRest =
          trx.oadConsRest().begin();
      for (; oadConsRest != trx.oadConsRest().end(); ++oadConsRest)
      {
        RexShoppingTrx::OADConsolidatedConstrains& oadConsolConstr = **oadConsRest;
        dc << "\nOAD: " << oadConsolConstr.origAirport << "-" << oadConsolConstr.destAirport << " "
           << oadConsolConstr.travelDate << " " << (oadConsolConstr.unflown ? "UNFLOWN" : "FLOWN");

        std::vector<RexShoppingTrx::SubOriginDestination*>::const_iterator sodIter =
            oadConsolConstr.sod.begin();
        for (; sodIter != oadConsolConstr.sod.end(); ++sodIter)
        {
          RexShoppingTrx::SubOriginDestination& sod = **sodIter;
          dc << "\n SOD: " << sod.origAirport << "-" << sod.destAirport << " " << sod.travelDate
             << " "
             << "CHANGE=" << (sod.change ? "Y" : "N");

          if (sod.change && sod.carriers)
          {
            if (!sod.carriers->usrCxr.cxrList.empty())
              dc << " EXACT_CXR=" << (sod.exact_cxr ? "Y" : "N");
            if (!sod.carriers->govCxr.cxrList.empty())
              dc << " PREFERRED_CXR=" << (sod.preferred_cxr ? "Y" : "N");
          }
          dc << "\n  FLIGHT LIST: ";
          std::set<int>::const_iterator fltIter = sod.flights.begin();
          for (; fltIter != sod.flights.end(); ++fltIter)
          {
            const TravelSeg* currSeg = excItinSeg[(*fltIter) - 1];
            dc << "\n   SEG POS: " << *fltIter << " - SEG INFO: " << currSeg->origAirport() << "-"
               << currSeg->destAirport() << " " << currSeg->departureDT();

            if (sod.bkcCanChange.count(*fltIter) != 0)
            {
              dc << " - BKC_CHANGE=Y";
            }
          }
          if (sod.change)
          {
            dc << "\n  GOV CARRIER LIST: ";
            if (sod.carriers && !sod.carriers->govCxr.cxrList.empty())
            {
              copy(sod.carriers->govCxr.cxrList.begin(),
                   sod.carriers->govCxr.cxrList.end(),
                   std::ostream_iterator<CarrierCode>(dc, " "));
            }

            dc << "\n  USER CARRIER LIST: ";
            if (sod.carriers && !sod.carriers->usrCxr.cxrList.empty())
            {
              copy(sod.carriers->usrCxr.cxrList.begin(),
                   sod.carriers->usrCxr.cxrList.end(),
                   std::ostream_iterator<CarrierCode>(dc, " "));
            }
          }
        }
      }

      if (!fallback::exscDiag989FixOndDisplay(_trx))
      {
        dc << std::endl;
        // FORCED CONNECTIONS
        for (const auto* oadConsRest : trx.oadConsRest())
        {
          ExchShopCalendar::DateRange calendarRange;

          const auto& sods = oadConsRest->sod;
          if (!sods.empty())
          {
            calendarRange.firstDate = sods.front()->calendarRange.firstDate;
            calendarRange.lastDate = sods.back()->calendarRange.lastDate;
          }

          dc << "\nCALENDAR RANGE: " << calendarRange;

          const auto& frcConxRest = oadConsRest->forcedConnection;
          dc << "\nFORCED CONNECTIONS: ";
          if (!frcConxRest.empty())
          {
            copy(frcConxRest.begin(), frcConxRest.end(), std::ostream_iterator<LocCode>(dc, " "));
          }
          dc << std::endl;
        }
      }
      else
      {
        if (!trx.oadResponse().empty())
        {
          for (const auto& oadRespMapPair : trx.oadResponse())
          {
            for (const auto& oadResponse : oadRespMapPair.second)
            {
              dc << "\nCALENDAR RANGE: " << oadResponse.calendarRange;

              const auto& frcConxRest = oadResponse.forcedConnections;
              dc << "\nFORCED CONNECTIONS: ";
              if (!frcConxRest.empty())
              {
                copy(frcConxRest.begin(), frcConxRest.end(), std::ostream_iterator<LocCode>(dc, " "));
              }
              dc << std::endl;
            }
          }
        }
      }

      std::vector<const PaxTypeFare*> ptfv;
      RuleUtil::getAllPTFs(ptfv, *trx.exchangeItin().front()->farePath().front());
      for (const auto ptf : ptfv)
      {
        printFareComponentInfo(ptf, trx);
      }

      dc << "\n***************************************************" << std::endl;
      dc << "Diagnostic 989 : Exchange Shopping response END" << std::endl;
      dc << "***************************************************" << std::endl;
    }
  }

  return *this;
}

void
Diag989Collector::printFareComponentInfo(const PaxTypeFare* ptf, RexShoppingTrx& trx)
{
  DiagCollector& dc = (DiagCollector&)*this;

  FareCompInfo* fareComponent = trx.exchangeItin().front()->findFareCompInfo(ptf->fareMarket());
  if (fareComponent)
  {
    dc << "\nFARE COMPONENT NUMBER: " << fareComponent->fareCompNumber();

    if (ptf)
    {
      printVCTR(ptf, fareComponent->fareMarket());
      printRecord3Info(ptf, trx);
    }
  }
}

void
Diag989Collector::printVCTR(const PaxTypeFare* ptf, const FareMarket* fareMarket)
{
  DiagCollector& dc = (DiagCollector&)*this;

  dc << "\n VCTR INFO: " << ptf->vendor() << " " << ptf->carrier() << " " << ptf->fareTariff()
     << " " << ptf->ruleNumber() << " - " << ptf->retrievalDate().dateToString(YYYYMMDD, "-");
}

void
Diag989Collector::printRecord3Info(const PaxTypeFare* ptf, RexShoppingTrx& trx)
{
  DiagCollector& dc = (DiagCollector&)*this;

  std::vector<ReissueOptions::R3WithDateRange> r3s;
  trx.reissueOptions().getRec3s(ptf, r3s);

  if (r3s.empty() && ptf->isFareByRule())
  {
    ptf = ptf->baseFare();
    trx.reissueOptions().getRec3s(ptf, r3s);
  }
  if (!r3s.empty())
  {
    for (const auto& r3Pair : r3s)
    {
      dc << "\n RECORD 3 NUMBER: " << r3Pair.first->itemNo();
      dc << "\n DATE RANGE: " << r3Pair.second;

      std::vector<ReissueOptions::ReissueSeqWithDateRange> t988v;
      trx.reissueOptions().getT988s(ptf, r3Pair.first, t988v);
      if (!t988v.empty())
      {
        for (const auto& t988Pair : t988v)
        {
          if (fallback::exsCalendar(_trx) && !(t988Pair.first))
            continue;
          dc << "\n  TAB 988 SEQ NUMBER: " << t988Pair.first->seqNo();
          dc << "\n  DATE RANGE: " << t988Pair.second;
        }
      }
      else
      {
        dc << "\n  TAB 988 SEQ IS EMPTY";
      }
    }
  }
  else
  {
    dc << "\n RECORD 3 SEQ IS EMPTY";
  }
}
} // namespace tse
