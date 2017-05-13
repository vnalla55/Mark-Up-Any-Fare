//----------------------------------------------------------------------------
//  File:        Diag981Collector.C
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


#include "Diagnostic/Diag981Collector.h"

#include "Common/Money.h"
#include "Common/Vendor.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Pricing/MergedFareMarket.h"

#include <algorithm>
#include <iostream>
#include <numeric>
#include <sstream>

namespace tse
{
Diag981Collector& Diag981Collector::operator<<(const FareMarket& fareMarket)
{
  DiagCollector& dc(*this);

  if (!adrelfares)
  {
    *this << "** DIAG 981 FAREMARKET'S FARES ***\n";
  }

  // Print fare market direction
  dc << " #  " << fareMarket.getDirectionAsString() << std::endl;

  // Print travel date
  dc << "Travel Date : " << std::setw(6) << fareMarket.travelDate().dateToString(DDMMM, "")
     << std::endl;

  if (!fareMarket.paxTypeCortege().empty())
  {
    for (std::vector<PaxTypeBucket>::const_iterator paxTypeCortegeIter =
             fareMarket.paxTypeCortege().begin();
         paxTypeCortegeIter != fareMarket.paxTypeCortege().end();
         paxTypeCortegeIter++)
    {
      const PaxTypeBucket& paxTypeCortege = *paxTypeCortegeIter;

      if (!paxTypeCortege.paxTypeFare().empty())
      {
        *this << "FAREMARKET: " << fareMarket.origin()->loc() << "-"
              << fareMarket.destination()->loc() << '\n';

        dc << std::endl;

        dc << "REQUESTED PAXTYPE : " << paxTypeCortege.requestedPaxType()->paxType() << std::endl;

        dc << " INBOUND CURRENCY : " << paxTypeCortege.inboundCurrency() << std::endl;

        dc << "OUTBOUND CURRENCY : " << paxTypeCortege.outboundCurrency() << std::endl;

        dc << std::endl;

        if (adrelfares)
        {
          dc << "  GI V RULE   FARE BASIS    TRF O O      AMT CUR EFFDATE     DISCDATE\n"
             << "                            NUM R I\n"
             << "- -- - ---- --------------- --- - - -------- --- ----------- -----------\n";
        }
        else
        {
          dc << "  GI V RULE   FARE BASIS    TRF O O      AMT CUR FAR PAX\n"
             << "                            NUM R I              TYP TYP\n"
             << "- -- - ---- --------------- --- - - -------- --- --- ---\n";
        }

        for (const auto elem : paxTypeCortege.paxTypeFare())
        {
          // Print diagnostics for each pax type fare
          PaxTypeFare& paxTypeFare = *elem;
          dc << paxTypeFare;
        }
      }
      else
      {
        dc << std::endl << "NO FARES FOUND FOR MARKET : " << fareMarket.origin()->loc() << '-'
           << fareMarket.destination()->loc()
           << ". REQUESTED PAXTYPE : " << paxTypeCortege.requestedPaxType()->paxType() << std::endl;
      }
    }
  }
  else
  {
    dc << std::endl << "NO FARES FOUND FOR MARKET : " << fareMarket.origin()->description() << '-'
       << fareMarket.destination()->description() << std::endl;
  }

  dc << std::endl;

  if (!adrelfares)
  {
    *this << "*** END OF DIAG 981 ***\n";
  }

  return *this;
}

Diag981Collector& Diag981Collector::operator<<(const MergedFareMarket& mfm)
{
  DiagCollector& dc(*this);

  dc << "MERGED FARE MARKETS: " << mfm.boardMultiCity() << "-" << mfm.offMultiCity() << "\n";
  dc << "NUMBER OF FARE MARKETS: " << mfm.mergedFareMarket().size() << "\n";

  PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(_trx);
  for (PaxType* paxType : pricingTrx->paxType())
  {
    const auto firstFare = mfm.firstFare(paxType);

    if (!firstFare)
      dc << "NO VALID FARES FOUND\n";
    else
    {
      const PaxTypeFare* const ptf = firstFare->ptf;
      std::string fareBasis = ptf->createFareBasis(pricingTrx, false);
      if (fareBasis.size() > 11)
        fareBasis = fareBasis.substr(0, 11) + "*";

      dc << "CHEAPEST FARE FOR " << paxType->paxType() << ":\n";
      dc << dc.cnvFlags(*ptf) << " ";

      std::string gd;
      globalDirectionToStr(gd, ptf->fare()->globalDirection());

      dc << std::setw(3) << gd << std::setw(2) << Vendor::displayChar(ptf->vendor()) << std::setw(5)
         << ptf->ruleNumber();

      dc << std::setw(12) << fareBasis << std::setw(4) << ptf->fareTariff();

      dc << std::setw(2) << DiagnosticUtil::getOwrtChar(*ptf);

      if (ptf->directionality() == FROM)
        dc << std::setw(2) << "O";
      else if (ptf->directionality() == TO)
        dc << std::setw(2) << "I";
      else
        dc << std::setw(2) << " ";

      dc << "  ";
      dc << Money(ptf->fareAmount(), ptf->currency()) << ", " << Money(ptf->nucFareAmount(), "NUC")
         << " ";
      dc << ptf->fcasPaxType();

      if (ptf->nucFareAmount() != firstFare->totalNucAmount)
        dc << " (WITH TAXES: " << Money(firstFare->totalNucAmount, "NUC") << ")";

      dc << "\n";
    }
  }

  dc << "\n";

  return *this;
}

Diag981Collector& Diag981Collector::operator<<(const PaxTypeFare& paxTypeFare)
{
  DiagCollector& dc(*this);

  // Adjust output to left justified
  dc.setf(std::ios::left, std::ios::adjustfield);

  // Print type of fare
  dc << std::setw(2) << cnvFlags(paxTypeFare);

  // Print global direction, vendor and rule number
  std::string globalDirection;
  globalDirectionToStr(globalDirection, paxTypeFare.fare()->globalDirection());

  dc << std::setw(3) << globalDirection << std::setw(2) << Vendor::displayChar(paxTypeFare.vendor())
     << std::setw(5) << paxTypeFare.ruleNumber();

  FareClassCode fareBasis = paxTypeFare.fare()->fareInfo()->fareClass();

  dc << std::setw(16) << fareBasis << std::setw(4) << paxTypeFare.fareTariff();

  // Print trip type
  dc << std::setw(2) << DiagnosticUtil::getOwrtChar(paxTypeFare);

  // Print fare directionality
  if (paxTypeFare.directionality() == FROM)
  {
    dc << std::setw(2) << "O";
  }
  else if (paxTypeFare.directionality() == TO)
  {
    dc << std::setw(2) << "I";
  }
  else
  {
    dc << " ";
  }

  // Print money amount and currency
  dc << std::setw(8) << Money(paxTypeFare.fareAmount(), paxTypeFare.currency()) << " ";

  if (!adrelfares)
  {
    // Print fare type
    if (!paxTypeFare.isFareClassAppMissing())
    {
      dc << std::setw(4) << paxTypeFare.fcaFareType();
    }
    else
    {
      dc << "UNK ";
    }

    // Print passenger type code
    if (!paxTypeFare.isFareClassAppSegMissing())
    {
      if (paxTypeFare.fcasPaxType().empty())
      {
        dc << "*** ";
      }
      else
      {
        dc << std::setw(4) << paxTypeFare.fcasPaxType();
      }
    }
    else
    {
      dc << "UNK ";
    }

    if (Vendor::displayChar(paxTypeFare.vendor()) == '*')
    {
      dc << "    " << paxTypeFare.vendor();
    }
  }
  else
  {
    dc << std::setw(4) << paxTypeFare.effectiveDate() << paxTypeFare.expirationDate();
  }

  dc << std::endl;

  return *this;
}

void
Diag981Collector::showRemFaresForDatePair(std::map<PaxTypeFare*, PaxTypeFare*>& ptFareMap)
{
  std::map<PaxTypeFare*, PaxTypeFare*>::const_iterator itBegM = ptFareMap.begin();
  std::map<PaxTypeFare*, PaxTypeFare*>::const_iterator itEndM = ptFareMap.end();
  if (itBegM != itEndM)
  {
    *this << "*** DIAG 981 ALTDATES RELEASED NOT EFFECTIVE/CAT15 FARES FOR TRAVELDATE ***\n";

    for (; itBegM != itEndM; ++itBegM)
    {
      if ((*itBegM).second && (*itBegM).first)
      {
        const FareMarket* fareMarket = (*itBegM).first->fareMarket();

        if (fareMarket == nullptr)
          return;

        *this << *fareMarket;
      }
    }

    *this << "*** END OF DIAG 981 ***\n";
  }
}
}
