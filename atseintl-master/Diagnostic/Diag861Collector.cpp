//----------------------------------------------------------------------------
//  Copyright Sabre 2011
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//  Description: Diagnostic NVB/NVA processing
//
//----------------------------------------------------------------------------
#include "Diagnostic/Diag861Collector.h"

#include "Common/FallbackUtil.h"
#include "Common/TseConsts.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/NvbNvaInfo.h"
#include "DBAccess/NvbNvaSeg.h"
#include "FareCalc/CalcTotals.h"


#include <iomanip>
#include <iostream>

namespace tse
{
void
Diag861Collector::logNvb(const Indicator& nvb)
{
  if (_active)
  {
    _isEmptyNvbNvaTable = false;
    (*this) << "PROCESS NVB: " << nvb << " - ";
    decodeNvb(nvb);
  }
}

void
Diag861Collector::decodeNvb(const Indicator& nvb)
{
  switch (nvb)
  {
  case NVB_1ST_SECTOR:
    (*this) << "1ST SECTOR";
    break;
  case NVB_1ST_INTL_SECTOR:
    (*this) << "1ST INTL SECTOR";
    break;
  case NVB_ENTIRE_OUTBOUND:
    (*this) << "ENTIRE OUTBOUND";
    break;
  case NVB_ENTIRE_JOURNEY:
    (*this) << "ENTIRE JOURNEY";
    break;
  case NVB_EMPTY:
    (*this) << "EMPTY";
    break;
  default:
    (*this) << "UNKNOWN";
    break;
  }
}

void
Diag861Collector::logNva(const Indicator& nva)
{
  if (_active)
  {
    (*this) << "    NVA: " << nva << " - ";
    decodeNva(nva);
    (*this) << std::endl;
  }
}

void
Diag861Collector::decodeNva(const Indicator& nva)
{
  switch (nva)
  {
  case NVA_1ST_SECTOR_EARLIEST:
    (*this) << "1ST SECTOR";
    break;
  case NVA_1ST_INTL_SECTOR_EARLIEST:
    (*this) << "1ST INTL SECTOR";
    break;
  case NVA_ENTIRE_OUTBOUND_EARLIEST:
    (*this) << "ENTIRE OUTBOUND";
    break;
  case NVA_EMPTY:
    (*this) << "EMPTY";
    break;
  default:
    (*this) << "UNKNOWN";
    break;
  }
}

DiagCollector& Diag861Collector::operator<<(const NegFareRestExtSeq& nfr)
{
  if (_active)
  {
    std::vector<const NegFareRestExtSeq*> negFareVectro;
    negFareVectro.push_back(&nfr);
    displayTFDPSC(*this, negFareVectro, false);
    _isEmptySuppression = false;
  }
  return *this;
}

void
Diag861Collector::logPricingUnit(const std::vector<FareUsage*>& fus,
                                 const Itin* itin,
                                 CalcTotals& calcTotals)
{
  if (_active)
  {
    setf(std::ios::right, std::ios::adjustfield);
    (*this) << "**************************************************************" << std::endl;
    (*this) << "            PRICING UNIT STATE BEFORE PROCESSING      " << std::endl;
    (*this) << "  FROM     TO      TVL DATE      NVB DATE      NVA DATE   " << std::endl;

    for (FareUsage* fu : fus)
    {
      for (TravelSeg* travelSeg : fu->travelSeg())
      {
        (*this) << std::setw(6) << travelSeg->origAirport();
        (*this) << std::setw(7) << travelSeg->destAirport();
        (*this) << std::setw(16) << travelSeg->departureDT().dateToString(YYYYMMMDD, "-");

        int16_t segOrder = segmentOrder(travelSeg, itin);
        (*this) << std::setw(14) << calcTotals.tvlSegNVB[segOrder].dateToString(YYYYMMMDD, "-");
        (*this) << std::setw(14) << calcTotals.tvlSegNVA[segOrder].dateToString(YYYYMMMDD, "-")
                << std::endl;
      }
    }
  }
}

DiagCollector& Diag861Collector::operator<<(const NvbNvaInfo& info)
{
  if (_active)
  {
    setf(std::ios::right, std::ios::adjustfield);
    (*this) << "----------------NVB NVA INFO------------- " << std::endl;
    (*this) << "  VENDOR   CARRIER   RULE TARIFF   RULE   " << std::endl;
    (*this) << std::setw(7) << info.vendor();
    (*this) << std::setw(9) << info.carrier();
    (*this) << std::setw(12) << info.ruleTariff();
    (*this) << std::setw(11) << ((info.rule() != ANY_RULE) ? info.rule() : "ANY") << std::endl;

    DateTime create = info.createDate();
    DateTime expire = info.expireDate();

    std::string createDateStr;
    std::string expireDateStr;

    if (create.isValid())
      createDateStr = create.dateToString(YYYYMMMDD, "-") + " " + create.timeToString(HHMMSS, ":");
    else
      createDateStr = "N/A";

    if (expire.isValid())
      expireDateStr = expire.dateToString(YYYYMMMDD, "-") + " " + expire.timeToString(HHMMSS, ":");
    else
      expireDateStr = "N/A";

    (*this) << "CREATE DATE: " << createDateStr << std::endl;
    (*this) << "EXPIRE DATE: " << expireDateStr << std::endl;
  }
  return *this;
}

void
Diag861Collector::logNvbNvaSeg(const NvbNvaSeg& seg, bool matchFlag)
{
  if (_active)
  {
    (*this) << "SEQNO: " << seg.sequenceNumber() << "    FARE BASIS: " << seg.fareBasis();
    if (matchFlag)
    {
      (*this) << std::endl << "    NVB: ";
      decodeNvb(seg.nvb());
      (*this) << "    NVA: ";
      decodeNva(seg.nva());
      (*this) << std::endl << "MATCHED" << std::endl << " " << std::endl;
    }
    else
    {
      uint8_t fareSize = seg.fareBasis().size();
      (*this) << std::setw((getFareBasicWide() - fareSize) + 14) << " - NOT MATCHED" << std::endl;
    }
  }
}

DiagCollector& Diag861Collector::operator<<(const FareUsage& fareUsage)
{
  if (_active)
  {
    const PaxTypeFare* fare = fareUsage.paxTypeFare();
    if (nullptr != fare)
    {
      (*this) << (*fare) << std::endl;
    }
  }
  return *this;
}

void
Diag861Collector::logTravelSeg(int16_t idx, const FarePath& farePath)
{
  if (_active)
  {
    for (TravelSeg* travelSeg : farePath.itin()->travelSeg())
    {
      int16_t segOrder = segmentOrder(travelSeg, farePath.itin());
      if (idx == segOrder)
      {
        (*this) << "PNR SEGMENT: " << segOrder << "    FROM: " << travelSeg->origAirport();
        (*this) << "    TO: " << travelSeg->destAirport() << std::endl;
        return;
      }
    }
  }
}

void
Diag861Collector::printSuppressionHeader()
{
  if (_active)
  {
    (*this) << " " << std::endl;
    (*this) << "**************************************************************" << std::endl;
    (*this) << "*****************NVB NVA SUPPRESSION SECTION******************" << std::endl;
    (*this) << "**************************************************************" << std::endl;
  }
}

void
Diag861Collector::printSuppressionFooter()
{
  if (_active)
  {
    if (_isEmptySuppression)
    {
      setf(std::ios::right, std::ios::adjustfield);
      (*this) << std::setw(38) << "NOT APPLICABLE" << std::endl;
    }
    (*this) << "**************END OF NVB NVA SUPPRESSION SECTION**************" << std::endl;
  }
}

void
Diag861Collector::printNvbNvaTableHeader()
{
  if (_active)
  {
    (*this) << " " << std::endl;
    (*this) << "**************************************************************" << std::endl;
    (*this) << "***************NVB NVA TABLE PROCESSING SECTION***************" << std::endl;
  }
}

void
Diag861Collector::printNvbNvaTableFooter()
{
  if (_active)
  {
    (*this) << "************END OF NVB NVA TABLE PROCESSING SECTION***********" << std::endl;
  }
}

void
Diag861Collector::clearLogIfDataEmpty()
{
  if (_active)
  {
    if (_isEmptyNvbNvaTable && _isEmptySuppression)
    {
      std::string noDataToPring;
      noDataToPring = "*****************NVB NVA SUPPRESSION SECTION******************\n";
      noDataToPring += "**************************************************************\n";
      noDataToPring += "*********  RESULTS FOR SMF NVB NVA UNIQUE TABLE   ************\n";
      noDataToPring += "*********       NON QUALIFYING ITINERARY           ***********\n";
      noDataToPring += "            SMF TABLE NOT APPLICABLE \n";
      noDataToPring += "**************************************************************\n";
      (*this).str(noDataToPring);
    }
  }
}

int16_t
Diag861Collector::segmentOrder(const TravelSeg* travelSeg, const Itin* itin) const
{
  return itin->segmentOrder(travelSeg);
}
}
