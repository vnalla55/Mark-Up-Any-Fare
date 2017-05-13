#pragma once

// FIXME: <memory> should be included by Singleton.h
#include "Common/Singleton.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Routing/MileageExclusion.h"
#include "Routing/MileageRoute.h"


#include <memory>
#include <vector>

namespace tse
{

class TpdPsr;
class DataHandle;
class FareCalcVisitor;

/**
 * @class TicketedPointDeduction
 *
 * Ticketed Point Deduction for mileage route from WP or WN.
 */
class TicketedPointDeduction : public MileageExclusion
{
public:
  /**
   * @function apply
   *
   * Entry method. Tries to apply TPD to the whole route only, i.e. if succeeds,
   * only last item and the route itself have TPD set to the highest TPD from
   * all TpdPsr records retrieved from the database and matched by all criteria.
   *
   * @note This functionality is sufficient for WP. WN needs to construct
   *	    incremental routes and invoke apply() for them, thus collecting TPDs for
   *	    all items where applicable.
   *
   * @param MileageRoute& - mileage route to which TPD is tried to apply
   * @return bool - true if TPD applied, false otherwise
   */
  virtual bool apply(MileageRoute&) const override;
  bool apply(MileageRoute&, FareCalcVisitor* visitor) const;

protected:
  /**
   * Construction and copying prohibited for a singleton.
   */
  TicketedPointDeduction() {}
  TicketedPointDeduction(const TicketedPointDeduction&);
  TicketedPointDeduction& operator=(const TicketedPointDeduction&);
  friend class tse::Singleton<TicketedPointDeduction>;

  /**
   * Methods to break the algorithm down.
   */
  bool processSubRoute(MileageRouteItems&,
                       const std::vector<TpdPsr*>&,
                       const CarrierCode&,
                       DataHandle&,
                       FareCalcVisitor*) const;

  bool processViaGeoLocs(MileageRouteItems&, TpdPsr&, DataHandle&, FareCalcVisitor*, bool origLoc1)
      const;

  bool processThruMktCxrs(const MileageRouteItems&, const TpdPsr&, const CarrierCode&) const;
  bool processViaCxrLocs(const MileageRouteItems&, const TpdPsr&, const CarrierCode&) const;
  bool processViaExcepts(const MileageRouteItems&, const TpdPsr&, const CarrierCode&) const;
  bool processThruViaMktOnly(const MileageRouteItems& subRoute, const TpdPsr& tpd) const;

  friend class TicketedPointDeductionTest;

private:
  /**
   * Auxiliary method to make unit tests independent of DataHandle.
   */
  const std::vector<TpdPsr*>&
  getData(DataHandle&, Indicator, const CarrierCode&, Indicator, Indicator, const DateTime&) const;
};

} // namespace tse

