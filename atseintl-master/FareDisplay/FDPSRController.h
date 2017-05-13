//----------------------------------------------------------------------------
//  File: FDPSRController.h
//
//  Author: Partha Kumar Chakraborti
//
//  Copyright Sabre 2005
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/LocUtil.h"
#include "Common/Vendor.h"
#include "DataModel/FareDisplayTrx.h"


namespace tse
{

class FDPSRController //: virtual public PermissibleSpecifiedRouting
{
  friend class FDPSRControllerTest;

private:
  virtual const std::vector<TpdPsr*>& getPSRData(DataHandle& dataHandle,
                                                 const CarrierCode& carrier,
                                                 Indicator area1,
                                                 Indicator area2,
                                                 const DateTime& travelDate)
  {
    return dataHandle.getTpdPsr(PSR, carrier, area1, area2, travelDate);
  }

  virtual DataHandle& getDataHandle(FareDisplayTrx& trx) const { return trx.dataHandle(); }

  bool processThruMktCxrs(const CarrierCode& carrierCode, TpdPsr& psr) const;

  bool validateLoc(const Loc& reqLoc, const Loc* reqMultiLoc, LocKey& psrLoc) const;

  bool validate(FareDisplayTrx& trx,
                const GlobalDirection& globalDir,
                const CarrierCode& carrierCode,
                RoutingInfo& routingInfo,
                const std::vector<TpdPsr*>& psrList) const;

public:
  virtual bool
  getPSR(FareDisplayTrx& trx, const GlobalDirection& globalDir, RoutingInfo& routingInfo);

  FDPSRController();
  virtual ~FDPSRController() {}

}; // End of Class FDPSRController

} // end of namespace

