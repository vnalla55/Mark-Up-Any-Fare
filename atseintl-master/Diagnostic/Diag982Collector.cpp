//----------------------------------------------------------------------------
//  File:        Diag982Collector.C
//  Created:     2008-08-26
//
//  Description: Diagnostic 982 formatter
//
//  Updates:
//
//  Copyright Sabre 2004
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
#include "Diagnostic/Diag982Collector.h"

#include "Common/ShoppingUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "Diagnostic/DiagCollector.h"
#include "Rules/RuleConst.h"

#include <boost/bind.hpp>

#include <iomanip>
#include <vector>

namespace tse
{
namespace
{
class PrintItinOptions : public std::binary_function<const Itin*, Diag982Collector*, void>
{
public:
  void operator()(const Itin* itin, Diag982Collector* diag) const
  {
    *diag << "FAMILY\n";
    diag->displayItin(*itin);
    for (const SimilarItinData& data : itin->getSimilarItins())
      diag->displayItin(*data.itin);
  }
};
}

Diag982Collector&
Diag982Collector::operator<<(const PricingTrx& pricingTrx)
{
  if (pricingTrx.getTrxType() == PricingTrx::MIP_TRX)
  {
    if (pricingTrx.isAltDates())
    {
      *this << "*** DATE PAIRS FOR MIP ***\n";
      outputAltDates(pricingTrx);
      *this << "******\n";
    }

    if (pricingTrx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL).empty() ||
        pricingTrx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ALL")
    {
      displayItins(pricingTrx);
    }
  }
  return *this;
}

void
Diag982Collector::outputAltDates(const PricingTrx& trx)
{
  bool bAltDates = trx.isAltDates();

  if (bAltDates == true)
  {
    for (const auto& elem : trx.altDatePairs())
    {
      DatePair myPair = elem.first;
      *this << myPair.first.dateToString(MMDDYY, "") << "   "
            << myPair.second.dateToString(MMDDYY, "") << std::endl;
    }
  }
  else
    *this << "NO ALTERNATE DATES" << std::endl;
}

void
Diag982Collector::showMIPItin(const std::map<int, Itin*>& itinMap)
{
  *this << "*** ITINS FOR MIP ***\n";

  for (const auto& itinMapValue : itinMap)
  {
    *this << std::setw(2) << "FAMILY " << itinMapValue.first << "\n";
    displayItin(*(itinMapValue.second));

    for (const SimilarItinData& similarItinData : itinMapValue.second->getSimilarItins())
      displayItin(*similarItinData.itin);
  }
  *this << "*** END OF ITINS FOR MIP ***\n";
}

void
Diag982Collector::displayItins(const PricingTrx& trx)
{
  uint16_t count = trx.itin().size();

  *this << "NUMBER OF ITINS: " << count << "\n";

  for (const Itin* itin : trx.itin())
    count += itin->getSimilarItins().size();

  *this << "NUMBER OF ITINS INCLUDING SIMILAR ITINS: " << count << "\n";
  *this << "  TO SEE SIMILAR ITINS ADD DDALL PARAM\n";

  for (const Itin* itin : trx.itin())
  {
    *this << "ITIN " << itin->itinNum() << ":\n";
    displayItin(*itin);

    if (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ALL")
    {
      for (const SimilarItinData& similarItinData : itin->getSimilarItins())
      {
        Itin& childItin = *similarItinData.itin;
        *this << " SIMILAR ITIN " << childItin.itinNum() << ":\n";
        displayItin(childItin);
      }
      *this << "----------------------- "
            << "\n";
    }
  }
}

void
Diag982Collector::displayItin(const Itin& itin)
{
  std::vector<TravelSeg*>::const_iterator tSegIter = itin.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tSegEIter = itin.travelSeg().end();

  for (int fltIndex = 1; tSegIter != tSegEIter; ++tSegIter)
  {
    const AirSeg* aSegPtr = dynamic_cast<const AirSeg*>(*tSegIter);
    const AirSeg& aSeg = *aSegPtr;

    if (aSegPtr)
    {
      *this << std::setw(2) << fltIndex << " " << std::setw(3) << aSeg.carrier();
      *this << " ";
      *this << std::setw(4) << aSeg.flightNumber() << " ";
      const DateTime& depDT = aSeg.departureDT();
      const DateTime& arrDT = aSeg.arrivalDT();
      DayOfWeek depDOW = depDT.dayOfWeek();
      std::ostringstream dowst;
      dowst << depDOW;
      std::string dowStr = dowst.str();
      dowStr = dowStr.substr(0, 1);
      std::string dowTransStr;
      char dowCh0 = dowStr[0];
      char dowCh1 = dowStr[1];

      switch (toupper(dowCh0))
      {
      case 'T':
        if (toupper(dowCh1) == 'H')
        {
          dowTransStr = "Q";
        }
        else
        {
          dowTransStr = "T";
        }
        break;
      case 'S':
        if (toupper(dowCh1) == 'A')
        {
          dowTransStr = "J";
        }
        else
        {
          dowTransStr = "S";
        }
        break;
      default:
        dowTransStr = dowCh0;
        break;
      }

      std::string depDTStr = depDT.timeToString(HHMM_AMPM, "");
      std::string arrDTStr = arrDT.timeToString(HHMM_AMPM, "");
      // Cut off the M from the time
      depDTStr = depDTStr.substr(0, depDTStr.length() - 1);
      arrDTStr = arrDTStr.substr(0, arrDTStr.length() - 1);
      *this << std::setw(2) << (*tSegIter)->getBookingCode() << " " << (*tSegIter)->bookedCabin()
            << " ";
      *this << std::setw(5) << depDT.dateToString(DDMMM, "") << " " << std::setw(2) << dowTransStr
            << " ";
      *this << std::setw(3) << aSeg.origin()->loc() << "  " << std::setw(3)
            << aSeg.destination()->loc() << " ";
      *this << std::setw(5) << depDTStr << " ";
      *this << std::setw(5) << arrDTStr << " ";
      *this << std::setw(4) << aSeg.equipmentType() << " ";
      *this << std::setw(2) << aSeg.hiddenStops().size() << " ";
      if (aSeg.eticket())
      {
        *this << std::setw(3) << "/E";
      }
    }
    else
    {
      *this << std::setw(2) << fltIndex << " " << std::setw(6) << "ARUNK";
    }
    *this << "\n";

    fltIndex++;
  }
  *this << "----------------------- "
        << "\n";
}

void
Diag982Collector::displaySplittedItins(PricingTrx& trx,
                                       const std::vector<Itin*>& splittedItins,
                                       const Itin* excItin,
                                       const std::vector<Itin*>& excRemovedItins,
                                       const std::vector<Itin*>& excSplittedItins)

{
  if (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ITINMIP")
  {
    *this << "*** DIAG 982 SPLITTED FAMILY - NEW FAMILIES SUMMARY ***\n";
    *this << " " << splittedItins.size() << " NEW FAMILIES\n";
    std::for_each(
        splittedItins.begin(), splittedItins.end(), boost::bind(PrintItinOptions(), _1, this));
    if (excItin)
    {
      *this << " Exchange Itin\n";
      displayItin(*excItin);

      *this << " " << excSplittedItins.size() << " REXEXCHANGE NEW FAMILIES\n";
      std::for_each(excSplittedItins.begin(),
                    excSplittedItins.end(),
                    boost::bind(PrintItinOptions(), _1, this));

      *this << " " << excRemovedItins.size() << " REMOVED ITINS\n";
      std::for_each(excRemovedItins.begin(),
                    excRemovedItins.end(),
                    boost::bind(PrintItinOptions(), _1, this));
    }
    *this << "*** END DIAG 982 SPLITTED FAMILY - NEW FAMILIES SUMMARY ***\n";
  }
  else if (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "SPLITIT")
  {
    *this << "*** DIAG 982 SPLITTED FAMILY SUMMARY ***\n";
    *this << " TOTAL NO OF FAMILIES: " << trx.itin().size() << "\n";
    *this << " " << splittedItins.size() << " NEW FAMILIES WERE ADDED\n";
    if (excItin)
    {
      *this << " " << excSplittedItins.size() << " REXEXCHANGE NEW FAMILIES\n";
      *this << " " << excRemovedItins.size() << " REMOVED ITINS\n";
    }

    std::for_each(trx.itin().begin(), trx.itin().end(), boost::bind(PrintItinOptions(), _1, this));
    *this << "*** END DIAG 982 SPLITTED FAMILY SUMMARY ***\n";
  }
}

void
Diag982Collector::displayRemovedItins(const std::vector<Itin*>::const_iterator begin,
                                      const std::vector<Itin*>::const_iterator end,
                                      const std::string& msg)
{
  *this << msg << "\n";
  *this << "Itins removed: "
        << "\n";
  std::vector<Itin*>::const_iterator it;
  for (it = begin; it != end; ++it)
  {
    displayItin(**it);
  }
}

void
Diag982Collector::displayKeySegments(const PricingTrx::ClassOfServiceKey& key)
{
  *this << "\nPROCESSING SEGMENTS: ";

  std::vector<TravelSeg*>::const_iterator tvlI = key.begin();
  std::vector<TravelSeg*>::const_iterator tvlIEnd = key.end();
  for (; tvlI != tvlIEnd; ++tvlI)
  {
    *this << (*tvlI)->origin()->loc() << "--" << (*tvlI)->destination()->loc();
    *this << "  ";
  }

  *this << "\n";
}

std::vector<std::string>
Diag982Collector::getMotherAvailabilityAsString(const PricingTrx& trx, const Itin& itin)
{
  std::vector<std::string> result;
  for (const FareMarket* fm: itin.fareMarket())
  {
    std::string fmstr = "FARE MARKET: ";
    fmstr.append(fm->toString());
    result.push_back(fmstr);
    const PricingTrx::ClassOfServiceKey& key = fm->travelSeg();
    if (key.empty())
      continue;
    std::vector<TravelSeg*>::const_iterator tvlI = key.begin();
    std::vector<TravelSeg*>::const_iterator tvlIEnd = key.end();
    for (size_t index = 0; tvlI != tvlIEnd; ++tvlI, ++index)
    {
      std::stringstream segstr;
      TravelSeg* seg = *tvlI;
      segstr << "SEGMENT: " << seg->origin()->loc() << "-" << seg->destination()->loc() << ":";
      std::map<BookingCode, ClassOfService*> missingCodesForSegment;
      for (ClassOfService* cos : ShoppingUtil::getClassOfService(trx, key)[index])
        segstr << " " << cos->bookingCode() << cos->numSeats();
      result.push_back(segstr.str());
    }
  }
  return result;
}

}
