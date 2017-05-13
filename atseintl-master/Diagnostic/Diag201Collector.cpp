//----------------------------------------------------------------------------
//  File:         Diag201Collector.h
//  Description:  Overrides the virtual methods to follow Diagnostic 201 format
//  Authors:      Mohammad Hossan
//  Created:      Dec 2003
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2003
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

#include "Diagnostic/Diag201Collector.h"

#include "Common/FareMarketUtil.h"
#include "Common/TseEnums.h"
#include "Common/Vendor.h"
#include "DataModel/FareMarket.h"
#include "DBAccess/TariffCrossRefInfo.h"

namespace tse
{
Diag201Collector& Diag201Collector::operator << ( const FareMarket& fareMarket )
{
  if (_active)
  {
    Diag201Collector& dc = *this;

    // If we dont have travel segments, we count output this line
    if (fareMarket.travelSeg().size() == 0)
      return *this;

    dc << " \n" << FareMarketUtil::getFullDisplayString(fareMarket) << "\n \n";

    dc << "V D CXR FARE RULTRF     FARE CLS        AMT CUR INHIBIT\n"
       << "  I     TRF  CODE                                          \n"
       << "- - --- ---- ---------- --------- --------- --- -------\n";
  }

  return *this;
}

Diag201Collector& Diag201Collector::operator << (const TariffCrossRefInfo& tariffCrossRef)
{
  if (_active)
  {
    Diag201Collector& dc = *this;

    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << std::setw(2) << (tariffCrossRef.vendor() == Vendor::ATPCO
                               ? "A"
                               : (tariffCrossRef.vendor() == Vendor::SITA ? "S" : " "))
       << std::setw(2) << (tariffCrossRef.crossRefType() == DOMESTIC
                               ? "D"
                               : (tariffCrossRef.crossRefType() == INTERNATIONAL ? "I" : " "))
       << std::setw(4) << tariffCrossRef.carrier() << std::setw(5) << tariffCrossRef.fareTariff()
       << std::setw(11) << tariffCrossRef.ruleTariffCode();
  }

  return *this;
}
}
