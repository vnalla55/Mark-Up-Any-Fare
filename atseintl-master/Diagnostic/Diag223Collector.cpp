//----------------------------------------------------------------------------
//  File:        Diag223Collector.C
//  Authors:
//  Created:     July 2004
//
//  Description: Diagnostic 23 formatter
//
//  Updates:
//          date - initials - description.
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

#include "Diagnostic/Diag223Collector.h"

#include "Common/FareMarketUtil.h"
#include "Common/Money.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MiscFareTag.h"

#include <iomanip>

namespace tse
{
void
Diag223Collector::writeSeparator(SeparatorType st)
{
  if (_active)
  {
    if (st == FARE_HEADER)
    {

      ((DiagCollector&)*this) << "  GI V RULE   FARE CLS   TRF O O      AMT CUR FAR PAX   CNV   \n"
                              << "                         NUM R I              TPE TPE   AMT   \n"
                              << "- -- - ---- ------------ --- - - -------- --- --- --- --------\n";
    }
  }
}

Diag223Collector&
Diag223Collector::operator<<(const PaxTypeFare& paxFare)
{
  if (!_active || !checkPaxTypeFare(paxFare))
    return *this;

  DiagCollector& dc(*this);

  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << std::setw(2) << cnvFlags(paxFare);

  std::string gd;
  globalDirectionToStr(gd, paxFare.fare()->globalDirection());

  dc << std::setw(3) << gd << std::setw(2)
     << (paxFare.vendor() == Vendor::ATPCO ? "A" : (paxFare.vendor() == Vendor::SITA ? "S" : "?"))
     << std::setw(5) << paxFare.ruleNumber() << std::setw(13) << paxFare.fareClass() << std::setw(4)
     << paxFare.fareTariff() << std::setw(2)
     << (paxFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED ? "R" : "O") << std::setw(2);

  if (paxFare.directionality() == FROM)
    dc << "O";
  else if (paxFare.directionality() == TO)
    dc << "I";
  else
    dc << " ";

  dc << std::setw(8) << Money(paxFare.fareAmount(), paxFare.currency()) << " ";
  dc << std::setw(4) << paxFare.fcaFareType();
  dc << std::setw(4) << paxFare.fcasPaxType();
  dc.setf(std::ios::right, std::ios::adjustfield);
  dc.setf(std::ios::fixed, std::ios::floatfield);
  dc.precision(2);
  dc << std::setw(8) << paxFare.nucFareAmount();
  dc << std::endl;

  // Dump MiscFareTags:
  if (paxFare.isMiscFareTagValid())
  {
    const MiscFareTag* miscFareTag = paxFare.miscFareTag();

    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << "ITEM NO - " << miscFareTag->itemNo() << std::endl;
    dc.setf(std::ios::right, std::ios::adjustfield);
    dc << "CONSTR - ";
    dc << std::setw(2) << miscFareTag->constInd() << " ; ";
    dc << "PRORATE - ";
    dc << std::setw(2) << miscFareTag->prorateInd() << " ; ";
    dc << "DIFF CALC - ";
    dc << std::setw(2) << miscFareTag->diffcalcInd() << " ; ";
    dc << "UNAVAIL - ";
    dc << std::setw(2) << miscFareTag->unavailtag() << " ;" << std::endl;
    dc << "REF CALC:";
    dc << std::setw(2) << miscFareTag->refundcalcInd() << " ; ";
    dc << "PROPORT - ";
    dc << std::setw(2) << miscFareTag->proportionalInd() << " ; ";
    dc << "CURR ADJ - ";
    dc << std::setw(2) << miscFareTag->curradjustInd() << " ; ";
    dc << std::endl;
    dc << "GEO TABLE NO - ";
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << std::setw(15) << miscFareTag->geoTblItemNo() << " ;" << std::endl;
    dc << "FARE 1: ";
    dc << "TYPE - " << std::setw(2) << miscFareTag->fareClassType1Ind() << " ; "
                                                                           "FARE CLASS FARE/TYPE - "
       << std::setw(12) << miscFareTag->fareClassType1() << std::endl;
    dc << "FARE 2: ";
    dc << "TYPE - " << std::setw(2) << miscFareTag->fareClassType2Ind() << " ; "
                                                                           "FARE CLASS FARE/TYPE - "
       << std::setw(12) << miscFareTag->fareClassType2() << std::endl;
  }
  return *this;
}

bool
Diag223Collector::checkPaxTypeFare(const PaxTypeFare& paxFare)
{
  if (_active)
  {
    if (!_fareClass.empty())
    {
      if (paxFare.fareClass() != _fareClass)
        return false;
    }

    if (_checkSpecial)
    {
      if (_isSpecial != paxFare.isSpecial())
        return false;
    }

    if (_globalDirection != GlobalDirection::ZZ)
    {
      if (paxFare.globalDirection() == GlobalDirection::ZZ)
        return true;

      return (_globalDirection == paxFare.globalDirection());
    }
  }
  return true;
}

Diag223Collector&
Diag223Collector::operator<<(const FareMarket& fareMarket)
{
  if (!_active)
    return *this;

  DiagCollector& dc = (DiagCollector&)*this;

  // If we dont have travel segments, we count output this line
  if (fareMarket.travelSeg().size() == 0)
    return *this;

  std::vector<TravelSeg*>::const_iterator tvlSegItr;
  tvlSegItr = fareMarket.travelSeg().begin();

  dc << "\n";
  dc << FareMarketUtil::getBoardMultiCity(fareMarket, **tvlSegItr); // origin()->loc();
  for (tvlSegItr = fareMarket.travelSeg().begin(); tvlSegItr != fareMarket.travelSeg().end();
       tvlSegItr++)
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(*tvlSegItr);
    if (airSeg != nullptr)
    {
      dc << "-";
      dc << airSeg->carrier();
      dc << "-";
      dc << FareMarketUtil::getOffMultiCity(fareMarket, *airSeg); // destination()->loc();
    }
  }
  dc << "    /CXR-" << fareMarket.governingCarrier() << "/";

  std::string globalDirStr;
  globalDirectionToStr(globalDirStr, fareMarket.getGlobalDirection());

  dc << " #GI-" << globalDirStr << "#  " << fareMarket.getDirectionAsString() << "\n";

  const std::vector<PaxTypeBucket>& paxTypeCortegeVec = fareMarket.paxTypeCortege();
  if (!paxTypeCortegeVec.empty())
  {
    std::vector<PaxTypeBucket>::const_iterator ptcIt = paxTypeCortegeVec.begin();
    std::vector<PaxTypeBucket>::const_iterator ptcEnd = paxTypeCortegeVec.end();

    while (ptcIt != ptcEnd)
    {
      const PaxTypeBucket& cortege = *ptcIt;
      const std::vector<PaxTypeFare*>& paxFareVec = cortege.paxTypeFare();
      if (!paxFareVec.empty())
      {
        dc << '\n';
        dc << "REQUESTED PAXTYPE : " << cortege.requestedPaxType()->paxType() << '\n';

        dc << " INBOUND CURRENCY : " << cortege.inboundCurrency() << '\n';

        dc << "OUTBOUND CURRENCY : " << cortege.outboundCurrency() << '\n';
        dc << '\n';

        writeSeparator(FARE_HEADER);

        std::vector<PaxTypeFare*>::const_iterator ptfIt = paxFareVec.begin();
        std::vector<PaxTypeFare*>::const_iterator ptfEnd = paxFareVec.end();

        while (ptfIt != ptfEnd)
        {
          PaxTypeFare& paxFare = **ptfIt;
          if (paxFare.isMiscFareTagValid())
            dc << paxFare; // Show only the fares with MicsFareTag attached...
          ++ptfIt;
        }
      }
      else
      {
        dc << '\n' << "NO FARES FOUND FOR MARKET : " << fareMarket.origin()->loc() << '-'
           << fareMarket.destination()->loc()
           << ". REQUESTED PAXTYPE : " << cortege.requestedPaxType()->paxType() << '\n';
      }
      ++ptcIt;
    }
  }
  else
  {
    dc << '\n' << "NO FARES FOUND FOR MARKET : " << fareMarket.origin()->description() << '-'
       << fareMarket.destination()->description() << '\n';
  }
  dc << '\n';
  return *this;
}

Diag223Collector&
Diag223Collector::operator<<(const Itin& itn)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;

    dc << "TICKETING CARRIER : " << itn.ticketingCarrier() << '\n';
  }
  return *this;
}

Diag223Collector&
Diag223Collector::operator<<(const PricingTrx& trx)
{
  if (_active)
  {
    DiagCollector& dc = (DiagCollector&)*this;

    std::string altCurrency = trx.getOptions()->alternateCurrency();
    if (altCurrency.size() == 0)
      altCurrency = "NUC";

    dc << "CONVERTED CURRENCY : " << altCurrency << '\n';
  }
  return *this;
}

void
Diag223Collector::parseQualifiers(const PricingTrx& trx)
{
  const DiagParamMap& diagMap = trx.diagnostic().diagParamMap();

  DiagParamMap::const_iterator it = diagMap.find("FC");
  if (it != diagMap.end())
    _fareClass = it->second;

  it = diagMap.find("FA");
  if (it != diagMap.end())
  {
    _checkSpecial = true;
    _isSpecial = (it->second == "S");
  }

  it = diagMap.find("GD");
  if (it != diagMap.end())
  {
    strToGlobalDirection(_globalDirection, (it->second).substr(0, 2));
  }
}
}
