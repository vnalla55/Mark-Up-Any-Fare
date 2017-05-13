//----------------------------------------------------------------------------
//  File:        Diag990Collector.C
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
#include "Diagnostic/Diag990Collector.h"

#include "Common/ClassOfService.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/FareUsage.h"
#include "Pricing/FPPQItem.h"
#include "Pricing/GroupFarePath.h"

using namespace std;
namespace tse
{
void
Diag990Collector::addItinNotApplicable(const Itin* itin, Diag990NotAppReason reason)
{
  std::string msg = "ITIN " + boost::lexical_cast<std::string>(itin->itinNum()) + ": ";
  switch (reason)
  {
  case SameCarriers:
    msg += "SAME CARRIERS FAILED";
    break;

  case SameCarriersSeatAviailable:
    msg += "SAME CARRIERS SEAT AVAILABLE FAILED" +
           (failedBookingCode[itin].empty() ? "" : " ON " + failedBookingCode[itin]);
    break;

  case SeatAvailable:
    msg += "SEAT AVAILABLE FAILED";
    break;

  case GetBookingCodeForDiffCarrier:
    msg += "GET BOOKING CODE FOR DIFF CARRIER FAILED" +
           (bookingCodeForDiffCxFailedReason[itin].empty()
                ? ""
                : " - " + bookingCodeForDiffCxFailedReason[itin]);
    break;

  case FlightApplicationRule:
    msg += "CAT 4 FLIGHT APPLICATION FAILED";
    break;
  case StopoverRule:
    msg += "CAT 8 STOPOVER RULE FAILED";
    break;

  case CloningFailed:
    msg += "FAREPATH CLONING FAILED";
    break;

  case GetBookingCodeForChildItin:
    msg += "GET BOOKING CODE FOR CHILD ITIN FAILED" +
           (bookingCodeForChildFailedReason[itin].empty()
                ? ""
                : " - " + bookingCodeForChildFailedReason[itin]);
    break;

  case Routing:
    msg += "ROUTING FAILED";
    break;

  case Combinability:
    msg += "COMBINABILITY FAILED";
    break;

  case FinalRevalidation:
    msg += "FINAL REVALIDATION STEP FAILED";
    break;

  default:
    msg += "WRONG REASON";
    break;
  }

  itinInfo.push_back(Diag990ItinInfo(itin, msg));
}

void
Diag990Collector::printBookingClass(const Itin& itin, const std::vector<FPPQItem*>& gfp)
{
  *this << getBookingClass(itin, gfp, itin);
}

void
Diag990Collector::printBookingClass(const Itin& itin, const GroupFarePath& gfp, const Itin& estItin)
{
  *this << getBookingClass(itin, gfp, estItin);
}

void
Diag990Collector::printItinNotApplicable(const Itin& motherItin, const GroupFarePath& gfp)
{
  if (itinInfo.size() == 0)
    return;

  *this << "ITINS NOT APLICABLE:\n";
  for (const auto& elem : itinInfo)
  {
    *this << elem.msg << "\n";
    DiagCollector::operator<<(*elem.itin);
    *this << getBookingClass(motherItin, gfp, *elem.itin);
  }
}

void
Diag990Collector::clearChildBookingClass()
{
  _bookingclassChild.str(std::string());
  _bookingclassChild.clear();
}

void
Diag990Collector::updateChildBookingClass(std::vector<std::vector<ClassOfService*>*>& cosVec)
{
  for (const auto elem : cosVec)
  {
    const auto& cos = *elem;

    for (const auto co : cos)
    {
      _bookingclassChild << co->bookingCode() << co->numSeats();
    }
    _bookingclassChild << std::endl;
  }
}

void
Diag990Collector::printItinNotApplicable()
{
  if (itinInfo.size() == 0)
    return;

  *this << "ITINS NOT APLICABLE:\n";
  for (const auto& elem : itinInfo)
  {
    *this << elem.msg << "\n";
    DiagCollector::operator<<(*elem.itin);
    printBookingClassChildItinNew(elem.itin);
    *this << "\n";
  }
}

void
Diag990Collector::clearChildBookingClassNew(const Itin* child)
{
  bookingClassForChild[child].erase();
}

void
Diag990Collector::updateChildBookingClassNew(
    const Itin* child, const std::vector<std::vector<ClassOfService*>*>& cosVec)
{
  std::stringstream bkcString;
  for (const auto cos : cosVec)
  {
    for (const auto co : *cos)
    {
      bkcString << co->bookingCode() << co->numSeats();
    }
    bkcString << std::endl;
  }

  bookingClassForChild[child] += bkcString.str();
}

void
Diag990Collector::printBookingClassChildItinNew(const Itin* child)
{
  *this << bookingClassForChild[child];
}

std::string
Diag990Collector::getBookingClass(const Itin& motherItin,
                                  const GroupFarePath& gfp,
                                  const Itin& itin)
{
  return getBookingClass(motherItin, gfp.groupFPPQItem(), itin);
}

std::string
Diag990Collector::getBookingClass(const Itin& motherItin,
                                  const std::vector<FPPQItem*>& fpPQItem,
                                  const Itin& itin)
{
  const PricingTrx* pricingTrx = dynamic_cast<const PricingTrx*>(trx());
  if (!pricingTrx)
    return "PRICING TRX REQUIRED";

  std::stringstream bookingClass;
  std::vector<TravelSeg*> estSegs;
  std::vector<FPPQItem*>::const_iterator f = fpPQItem.begin();
  std::vector<FPPQItem*>::const_iterator fEnd = fpPQItem.end();
  // loop through fare path to match booking code for all passenger type
  for (; f != fEnd; ++f)
  {
    const FarePath* fp = (*f)->farePath();
    int segIndex = 0;
    std::vector<TravelSeg*>::const_iterator segIt = motherItin.travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator segItEnd = motherItin.travelSeg().end();
    // loop through travel seg of mother itin
    for (; segIt != segItEnd; ++segIt, ++segIndex)
    {
      if ((*segIt)->segmentType() == Arunk)
      {
        continue; // do not check if it's ARUNK segment
      }
      bool matchSegFound = false;
      std::vector<PricingUnit*>::const_iterator p = fp->pricingUnit().begin();
      std::vector<PricingUnit*>::const_iterator pEnd = fp->pricingUnit().end();
      for (; p != pEnd && !matchSegFound; ++p)
      {
        for (std::vector<FareUsage*>::const_iterator fu = (*p)->fareUsage().begin();
             fu != (*p)->fareUsage().end();
             ++fu)
        {
          const uint16_t index =
              std::find((*fu)->travelSeg().begin(), (*fu)->travelSeg().end(), *segIt) -
              (*fu)->travelSeg().begin();
          if (index >= (*fu)->travelSeg().size())
          {
            continue;
          }
          matchSegFound = true;
          BookingCode code = (*fu)->segmentStatus()[index]._bkgCodeReBook;
          if (code.empty())
            code = (*fu)->travelSeg()[index]->getBookingCode();

          estSegs.push_back(itin.travelSeg()[segIndex]);
          // if avial break here then try match the child itin avail
          if ((*fu)->segmentStatus()[index]._bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_AVAIL_BREAK))
          {
            // get availability for the estimated itinerary
            const std::vector<ClassOfServiceList>& avail =
                ShoppingUtil::getClassOfService(*pricingTrx, estSegs);
            for (size_t n = 0; n < avail.size(); ++n)
            {
              // but first try to find interline avl
              ClassOfServiceList* interCos = nullptr;
              if (n < estSegs.size())
              {
                Itin::InterlineJourneyInfoMap::const_iterator cosIt =
                    itin.interlineJourneyInfo().find(estSegs[n]);
                if (cosIt != itin.interlineJourneyInfo().end())
                {
                  interCos = cosIt->second;
                }
              }
              const std::vector<ClassOfService*>& cos = interCos ? (*interCos) : avail[n];
              for (const auto co : cos)
              {
                bookingClass << co->bookingCode() << co->numSeats();
              }
              bookingClass << std::endl;
            }

            estSegs.clear();
          }
        }
      }
    } // for FareUsage
  }
  bookingClass << std::endl;

  bookingClass << getInterlineBookingClass(itin);

  return bookingClass.str();
}

std::string
Diag990Collector::getInterlineBookingClass(const Itin& itin)
{
  if (_trx->diagnostic().diagParamMapItem("DD") != "INTAVL")
    return "";

  std::stringstream bookingClass;
  bookingClass << "Interline Availability:" << std::endl;

  for (TravelSeg* tvSeg : itin.travelSeg())
  {
    Itin::InterlineJourneyInfoMap::const_iterator cosIt = itin.interlineJourneyInfo().find(tvSeg);
    if (cosIt != itin.interlineJourneyInfo().end())
    {
      Itin::InterlineJourneyMarketMap::const_iterator mktIt =
          itin.interlineJourneyMarket().find(tvSeg);
      if (mktIt != itin.interlineJourneyMarket().end())
        bookingClass << "Interline Market " << mktIt->second.first->origAirport() << "-"
                     << mktIt->second.second->destAirport() << " ";
      ClassOfServiceList* cos = cosIt->second;
      for (std::vector<ClassOfService*>::const_iterator c = cos->begin(); c != cos->end(); ++c)
        bookingClass << (*c)->bookingCode() << (*c)->numSeats();
      bookingClass << std::endl;
    }
    else
    {
      bookingClass << "NO INTERLINE AVAILABILITY" << std::endl;
    }
  }
  bookingClass << std::endl;

  return bookingClass.str();
}

Diag990Collector& Diag990Collector::operator<<(const Itin& itin)
{
  DiagCollector::operator<<(itin);

  if (bookingCodeForDiffCx[&itin] != "")
    *this << "BOOKING CODES FOR DIFF CARRIER: " << bookingCodeForDiffCx[&itin] << "\n";

  if (bookingCodeForChild[&itin] != "")
    *this << "BOOKING CODES FOR CHILD ITIN: " << bookingCodeForChild[&itin] << "\n";

  return *this;
}

Diag990Collector& Diag990Collector::operator<<(const std::vector<FPPQItem*>& fpPQItem)
{
  DiagCollector::operator<<(fpPQItem);

  *this << "REQUIRED BOOKING CODES: ";
  std::string bookingCodes;
  // loop through fare path to match booking code for all passenger type
  for (const FPPQItem* item : fpPQItem)
  {
    const FarePath* fp = item->farePath();
    for (const PricingUnit* pricingUnit : fp->pricingUnit())
    {
      for (const FareUsage* fu : pricingUnit->fareUsage())
      {
        for (size_t segIndex = 0; segIndex < fu->segmentStatus().size(); ++segIndex)
        {
          BookingCode code = fu->segmentStatus()[segIndex]._bkgCodeReBook;
          if (code.empty())
            code = fu->travelSeg()[segIndex]->getBookingCode();

          if (bookingCodes != "")
            bookingCodes = bookingCodes + " ";
          bookingCodes = bookingCodes + code;
        }
      }
    }
    bookingCodes = bookingCodes + "\n\n";
  }
  *this << bookingCodes;

  return *this;
}

void
Diag990Collector::printSimilarItinFarePaths(const std::vector<FarePath*>& farePaths)
{
  *this << "GFP:\n";
  this->setf(std::ios::right, std::ios::adjustfield);
  this->setf(std::ios::fixed, std::ios::floatfield);
  this->precision(2);

  for (const auto fp : farePaths)
  {
    const FarePath& farePath = *fp;
    *this << std::setw(8) << " AMOUNT: " << farePath.getTotalNUCAmount()
          << " REQUESTED PAXTYPE: " << farePath.paxType()->paxType() << "\n";
  }
  *this << "\n";
}

void
Diag990Collector::setBookingCodeForChildFailedReason(const Itin* itin, std::string rs)
{
  bookingCodeForChildFailedReason[itin] = rs;
}

void
Diag990Collector::setBookingCodeForChildFailedReason(const Itin* itin, std::string rs, size_t index)
{
  std::stringstream ss;
  ss << rs << ". SEGMENT INDEX: " << index;
  bookingCodeForChildFailedReason[itin] = ss.str();
}

void
Diag990Collector::printPnrCollocation(PricingTrx& trx, const Itin* itin)
{
  std::stringstream ss;
  PricingTrx::PnrSegmentCollocation& pnrSegColl = trx.getPnrSegmentCollocation();

  if (!pnrSegColl.empty())
  {
    std::vector<TravelSeg*>::const_iterator tvlSeg = itin->travelSeg().begin();
    for (; tvlSeg != itin->travelSeg().end(); ++tvlSeg)
    {
      int16_t itinPnrSegment = (*tvlSeg)->pnrSegment();

      if (pnrSegColl.count(itinPnrSegment) > 0)
      {
        const std::set<int16_t>& similarItinPnr = pnrSegColl[itinPnrSegment];
        ss << "\n MOTHER PNR " << itinPnrSegment << " : ";
        std::set<int16_t>::const_iterator similarPnr = similarItinPnr.begin();
        for (; similarPnr != similarItinPnr.end(); ++similarPnr)
        {
          ss << std::setw(3) << *similarPnr;
        }
      }
    }
  }
  *this << ss.str() << "\n\n";
}

void
Diag990Collector::printFamily(PricingTrx& trx,
                              const std::vector<FPPQItem*>& groupFPath,
                              const Itin& motherItin)
{
  *this << "\nMOTHER ITIN " << motherItin.itinNum() << ":\n";
  *this << motherItin;
  printBookingClass(motherItin, groupFPath);
  *this << "GFP:\n";
  *this << groupFPath;
  *this << "NUMBER OF SIMILAR ITINS: " << motherItin.getSimilarItins().size() << "\n\n";

  if (trx.getTrxType() == PricingTrx::MIP_TRX && !trx.isAltDates() &&
      trx.diagnostic().diagParamMapItem("DD") == "PNR")
  {
    printPnrCollocation(trx, &motherItin);
  }
}

} // namespace tse
