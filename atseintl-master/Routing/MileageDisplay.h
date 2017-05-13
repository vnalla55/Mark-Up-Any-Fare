//----------------------------------------------------------------------------
//
//  Copyright Sabre 2003
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

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"

namespace tse
{

class MileageTrx;
class MileageRouteItem;
class TpdPsrViaGeoLoc;
class MileageRoute;
class GDPrompt;

/**
*   @class MileageDisplay
*
*   Description:
*   MileageDisplay is responsible for displaying the WN request
*   and response.
*
*/
class MileageDisplay
{
  friend class MileageDisplayTest;

public:
  MileageDisplay(MileageTrx& trx);
  virtual ~MileageDisplay();

  bool displayMileageRequest(const MileageTrx& trx);
  bool displayHeader(const MileageTrx& trx);
  bool displayMileageRoute(const MileageRoute&);
  bool displayGDPrompt(const MileageTrx&, const GDPrompt*);

private:
  /* Displays first row */
  bool displayMileageRouteItem(const MileageRouteItem&);
  /* Displays 2nd and other rows */
  bool displayMileageRouteItem(
      const MileageRoute&, const MileageRouteItem&, uint16_t, uint16_t, bool, bool);

  bool displayConstructed();
  bool displaySouthAtlantic(const MileageRoute&);
  bool displayEqualization(const MileageRoute&);
  bool displayPSR();
  bool displayPSRMayApply(const MileageRoute& mileageRoute);
  bool displaySurchargeInfo(const MileageRouteItem& mItem, uint16_t cumMileage, uint16_t eMS);
  bool displayEMS(const MileageRouteItem& mItem, uint16_t cumMileage, uint16_t& eMS);
  bool displaySurfaceSector(const std::vector<std::pair<const LocCode*, const LocCode*> >&);
  bool displayOriginDestinationRetransitted();
  bool displayIntermediateRetransitted();
  bool displayTPD();
  bool displayConditionalTPDs(const std::vector<const std::vector<const TpdPsrViaGeoLoc*>*>&);
  bool displayMileageUnavailable();
  bool isFailedDirService(const MileageRoute&);

  MileageTrx& _trx;
};

} // namespace tse

