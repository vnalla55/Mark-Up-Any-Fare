#pragma once

#include "Common/Singleton.h"
#include "Common/TseConsts.h"
#include "Routing/MileageDataRetriever.h"
#include "Routing/MileageRoute.h"

#include <memory>

namespace tse
{

class DataHandle;
class TravelSeg;
class PricingTrx;

/**
 * @class GlobalDirectionRetriever
 *
 * A singleton - stateless class to retrieve global direction with use of GlobalDirectionFinder.
 */
class GlobalDirectionRetriever : public MileageDataRetriever
{
public:
  /**
   * @function retrieve
   *
   * Concrete implementation of global direction retrieval
   *
   * @param MileageRouteItem& - mileage route item that provides data needed to match and retrieve
   *data,
   *				 and to be updated with TPM global direction
   * @param DataHandle& - data handle for creating objects in pooled memory
   * @param Indicator - ignored
   * @return bool - true if retrieval was successful, false otherwise
   */
  virtual bool retrieve(MileageRouteItem&, DataHandle&, Indicator mileageType = TPM) const override;
  /* for WN MPM */
  /**
   * @function retrieve
   *
   * Retrieves MPM global direction for WN processing
   *
   * @param MileageRouteItems& - mileage route items that provide data needed to match and retrieve
   *data,
   *				  and (the last one) to be updated with MPM global direction
   * @param DataHandle& - data handle for creating objects in pooled memory
   * @return bool - true if retrieval was successful, false otherwise
   */
  bool retrieve(MileageRouteItems&, DataHandle&) const;

  bool retrieve(MileageRouteItem&, DataHandle&, PricingTrx* pricingTrx, Indicator mileageType = TPM)
      const;

protected:
  /**
   * Construction, destruction and copying prohibited for a singleton.
   */
  GlobalDirectionRetriever() {}
  GlobalDirectionRetriever(const GlobalDirectionRetriever&);
  GlobalDirectionRetriever& operator=(const GlobalDirectionRetriever&);
  /**
   * Protected destructor not really necessary, Should it cause any problem,
   * can be safely made back public (and friendship of unique_ptr removed).
   */
  ~GlobalDirectionRetriever() {}
  friend class tse::Singleton<GlobalDirectionRetriever>;
  friend class std::unique_ptr<GlobalDirectionRetriever>;
  friend class GlobalDirectionRetrieverTest;

private:
  /**
   * An auxiliary method to make unit tests independent of GlobalDirectionFinder.
   */
  virtual bool getData(DateTime&,
                       const std::vector<TravelSeg*>&,
                       GlobalDirection&,
                       PricingTrx* pricingTrx) const;
};

} // namespace tse

