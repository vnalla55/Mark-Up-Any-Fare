//----------------------------------------------------------------------------
//
//  File:           FareClassAppSegInfoQualifier.cpp
//
//  Description:    Qualifies PaxTypeFare.
//
//  Updates:
//
//  Copyright Sabre 2006
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------

#include "FareDisplay/FareClassAppSegInfoQualifier.h"

#include "BookingCode/FareDisplayBookingCode.h"
#include "Common/TSELatencyData.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "Rules/RuleConst.h"

namespace tse
{
FareClassAppSegInfoQualifier::FareClassAppSegInfoQualifier() {}

FareClassAppSegInfoQualifier::~FareClassAppSegInfoQualifier() {}

const tse::PaxTypeFare::FareDisplayState
FareClassAppSegInfoQualifier::qualify(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const
{
  TSELatencyData metrics(trx, "QUAL F CLASS APP SEG");

  LOG4CXX_DEBUG(_logger,
                "FareClassAppSegInfoQualifier,  ptFare.fcasCarrierApplTblItemNo() ="
                    << ptFare.fcasCarrierApplTblItemNo());
  LOG4CXX_DEBUG(_logger, "FareClassAppSegInfoQualifier,  ptFare.carrier =" << ptFare.carrier());
  LOG4CXX_DEBUG(_logger,
                "FareClassAppSegInfoQualifier,  governingCarrier ="
                    << ptFare.fareMarket()->governingCarrier());
  if (ptFare.fcasCarrierApplTblItemNo() == 0)
  {
    LOG4CXX_DEBUG(_logger, "FareClassAppSegInfoQualifier,  ptFare.fcasCarrierApplTblItemNo() == 0");
    return retProc(trx, ptFare);
  }
  else if (ptFare.carrier() == ptFare.fareMarket()->governingCarrier())
  {
    return retProc(trx, ptFare);
  }
  else
  {
    const std::vector<CarrierApplicationInfo*>& carrierApplList =
        getCarrierApplList(trx.dataHandle(), ptFare.vendor(), ptFare.fcasCarrierApplTblItemNo());
    // carrierApplList = trx.dataHandle().getCarrierApplication(ptFare.vendor(),
    // ptFare.fcasCarrierApplTblItemNo());

    if (carrierApplList.empty())
    {
      return retProc(trx, ptFare);
    }

    FareDisplayBookingCode fdbc;

    if (fdbc.findCXR(ptFare.fareMarket()->governingCarrier(), carrierApplList))
    {
      return retProc(trx, ptFare);
    }
  }
  LOG4CXX_DEBUG(_logger,
                "FareClassAppSegInfo invalidating: " << ptFare.fareClass() << " Gov. Cxr: "
                                                     << ptFare.fareMarket()->governingCarrier());
  return PaxTypeFare::FD_FareClassApp_Unavailable;
}

const std::vector<CarrierApplicationInfo*>&
FareClassAppSegInfoQualifier::getCarrierApplList(DataHandle& dh,
                                                 const VendorCode& vendor,
                                                 int itemNo) const
{
  return dh.getCarrierApplication(vendor, itemNo);
}

bool
FareClassAppSegInfoQualifier::setup(FareDisplayTrx& trx)
{
  return true;
}
}
