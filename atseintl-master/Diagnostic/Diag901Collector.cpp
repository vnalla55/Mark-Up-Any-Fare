//----------------------------------------------------------------------------
//  File:        Diag901Collector.C
//  Authors:     Adrienne A. Stipe, David White
//  Created:     2004-07-15
//
//  Description: Diagnostic 901 formatter
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
#include "Diagnostic/Diag901Collector.h"

#include "Common/Money.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/IndustryFare.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/DiagCollector.h"

#include <iomanip>

namespace tse
{
Diag901Collector&
Diag901Collector::operator<<(const PaxTypeFare& paxFare)
{
  if (_active)
  {
    DiagCollector& dc(*this);

    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << std::setw(2) << cnvFlags(paxFare);

    std::string gd;
    globalDirectionToStr(gd, paxFare.fare()->globalDirection());

    dc << std::setw(3) << gd << std::setw(2)
       << (paxFare.vendor() == Vendor::ATPCO ? "A" : (paxFare.vendor() == Vendor::SITA ? "S" : "?"))
       << std::setw(5) << paxFare.ruleNumber();
    std::string fareBasis = paxFare.createFareBasis(*_trx, false);
    if (fareBasis.size() > 15)
      fareBasis = fareBasis.substr(0, 15) + "*"; // Cross-of-lorraine?
    dc << std::setw(16) << fareBasis << std::setw(4) << paxFare.fareTariff();

    dc << std::setw(2) << DiagnosticUtil::getOwrtChar(paxFare);

    if (paxFare.directionality() == FROM)
      dc << std::setw(2) << "O";

    else if (paxFare.directionality() == TO)
      dc << std::setw(2) << "I";

    else
      dc << " ";

    dc << std::setw(8) << Money(paxFare.fareAmount(), paxFare.currency()) << " ";

    if (!paxFare.isFareClassAppMissing())
    {
      dc << std::setw(4) << paxFare.fcaFareType();
    }
    else
    {
      dc << "UNK ";
    }

    if (!paxFare.isFareClassAppSegMissing())
    {
      if (paxFare.fcasPaxType().empty())
        dc << "*** ";
      else
        dc << std::setw(4) << paxFare.fcasPaxType();
    }
    else
    {
      dc << "UNK ";
    }

    dc << '\n';
  }

  return *this;
}

Diag901Collector&
Diag901Collector::operator<<(const FareMarket& fareMarket)
{

  if (_active)
  {
    DiagCollector& dc(*this);

    if (fareMarket.travelSeg().size() == 0)
      return *this;

    std::vector<TravelSeg*>::const_iterator tvlSegItr;
    tvlSegItr = fareMarket.travelSeg().begin();

    dc << "\n";

    dc << (*tvlSegItr)->origin()->loc();

    for (tvlSegItr = fareMarket.travelSeg().begin(); tvlSegItr != fareMarket.travelSeg().end();
         tvlSegItr++)
    {
      AirSeg* airSeg = dynamic_cast<AirSeg*>(*tvlSegItr);
      if (airSeg != nullptr)
      {
        dc << "-";
        dc << airSeg->carrier();
        dc << "-";
        dc << airSeg->destination()->loc();
      }
    }

    dc << "    /GOVERNING CARRIER: " << fareMarket.governingCarrier() << "/";
    dc << "\n";
    dc << "\n";

    const std::vector<PaxTypeBucket>& paxTypeCortegeVec = fareMarket.paxTypeCortege();

    bool foundFares = false;
    bool showTitle = true;

    for (const auto& ptcv : paxTypeCortegeVec)
    {
      for (const auto elem : ptcv.paxTypeFare())
      {
        foundFares = true;
        if (showTitle)
        {
          dc << "  GI V RULE   FARE CLS   TRF O O      AMT CUR FAR PAX   CNV   \n"
             << "                         NUM R I              TPE TPE   AMT   \n"
             << "- -- - ---- ------------ --- - - -------- --- --- --- --------\n";
          showTitle = false;
        }
        dc << *elem;
      }
    }

    if (!foundFares)
    {
      dc << "NO FARES FOUND FOR MARKET : " << fareMarket.origin()->description() << '-'
         << fareMarket.destination()->description() << '\n';
    }

    dc << "\n";
  }

  return *this;
}

Diag901Collector& Diag901Collector::operator<<(const ShoppingTrx& shoppingTrx)
{
  if (_active)
  {
    DiagCollector& dc(*this);

    if (shoppingTrx.legs().empty())
    {
      return (*this);
    }

    dc << "***************************************************" << std::endl;
    dc << "901 : governing carrier and governing thru-fares" << std::endl;
    dc << "***************************************************" << std::endl;

    FareMarket* fmPtr = shoppingTrx.fareMarket().front();
    dc << *fmPtr;
  }

  return (*this);
}
}
