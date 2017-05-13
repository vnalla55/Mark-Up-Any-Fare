//----------------------------------------------------------------------------
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

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TseEnums.h"

#include <vector>

namespace tse
{
class Itin;
class TravelSeg;

class TravelSegAnalysis final
{
public:
  //--------------------------------------------------------------------------
  // @function TravelSegAnalysis::selectTravelBoundary
  //
  // Description: Scans the origin and destination of each travel segment
  //              to select the travel Boundary of Itin and FareMarket.
  //
  // @param tvlSegs - valid reference to a vector of pointers to TravelSegs
  // @return Boundary - a boundary
  //--------------------------------------------------------------------------
  static Boundary selectTravelBoundary(const std::vector<TravelSeg*>& tvlSegs);

  //--------------------------------------------------------------------------
  // @function TravelSegAnalysis::getFirstIntlFlt
  //
  // Description: Helper method to determine govering carrier when the
  //              travel is wholly within europe.
  //
  // @param tvlSegs - valid reference to a vector of pointers to TravelSegs
  // @return CarrierCode - a carrier code
  //--------------------------------------------------------------------------
  static CarrierCode
  getFirstIntlFlt(const std::vector<TravelSeg*>& tvlSegs, TravelSeg*& primarySector);

  CarrierCode getFirstIntlFlt(const std::vector<TravelSeg*>& tvlSegs) const;

  //--------------------------------------------------------------------------
  // @function TravelSegAnalysis::getLastIntlFlt
  //
  // Description: Helper method to determine govering carrier when the
  //              travel is wholly within europe.
  //
  // @param tvlSegs - valid reference to a vector of pointers to TravelSegs
  // @return CarrierCode - a carrier code
  //--------------------------------------------------------------------------
  static CarrierCode
  getLastIntlFlt(const std::vector<TravelSeg*>& tvlSegs, TravelSeg*& primarySector);

  //--------------------------------------------------------------------------
  // @function TravelSegAnalysis::getHighestTPMSector
  //
  // Description: Returns the travel segment with highest mileage
  //
  //
  // @param tvlSegs - valid reference to a vector of pointers to TravelSegs
  // @return travel segment with highest mileage
  //--------------------------------------------------------------------------
  TravelSeg* getHighestTPMSector(const std::vector<TravelSeg*>& tvlSegs);

  bool getCarrierInIATA1(Itin& itn,
                         std::vector<TravelSeg*>::iterator first,
                         std::vector<TravelSeg*>::iterator last) const;

  uint32_t getTPM(const TravelSeg& tvlSeg) const;

  void setGeoTravelType(TravelSeg*& tvlSeg) const;
};
}

