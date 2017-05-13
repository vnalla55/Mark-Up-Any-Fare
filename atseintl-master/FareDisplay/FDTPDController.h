//----------------------------------------------------------------------------
//  File: FDTPDController.h
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
#include "Routing/MileageRoute.h"
#include "Routing/MileageRouteBuilder.h"
#include "Routing/TravelRouteBuilder.h"


namespace tse
{

class FDTPDController
{
  friend class FDTPDControllerTest;

protected:
  class GDNotEqualNorBlank : public std::unary_function<TpdPsr, bool>
  {
  public:
    GDNotEqualNorBlank(GlobalDirection globalDir) : _globalDir(globalDir) {}
    bool operator()(const TpdPsr* tpdPsr) const
    {
      return tpdPsr->globalDir() != _globalDir && tpdPsr->globalDir() != GlobalDirection::ZZ;
    }

  private:
    const GlobalDirection _globalDir;
  };

  bool isFrom(const Loc& origin, const Loc& destination, const TpdPsr& tpd) const
  {
    return LocUtil::isFrom(tpd.loc1(), tpd.loc2(), origin, destination);
  }

  virtual const std::vector<TpdPsr*>& getTPDData(DataHandle& dataHandle,
                                                 const CarrierCode& carrier,
                                                 Indicator area1,
                                                 Indicator area2,
                                                 const DateTime& travelDate)
  {
    return dataHandle.getTpdPsr(TPD, carrier, area1, area2, travelDate);
  }

  virtual DataHandle& getDataHandle(FareDisplayTrx& trx) const { return trx.dataHandle(); }

  virtual bool processThruMktCxrs(const MileageRouteItems& subRoute,
                                  const TpdPsr& tpd,
                                  const CarrierCode& governingCarrier) const;

  bool buildMileageRoute(FareDisplayTrx& trx, MileageRoute& mileageRoute) const;

  virtual bool validate(FareDisplayTrx& trx,
                        const GlobalDirection& globalDir,
                        const CarrierCode& carrierCode,
                        RoutingInfo& routingInfo,
                        const std::vector<TpdPsr*>& psrList) const;

public:
  virtual bool getTPD(FareDisplayTrx& trx,
                      const CarrierCode& carrier,
                      GlobalDirection globalDir,
                      RoutingInfo& routingInfo);

  FDTPDController();
  virtual ~FDTPDController() {}

}; // End of Class FDTPDController

} // end of namespace

