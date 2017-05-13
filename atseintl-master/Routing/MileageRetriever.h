#pragma once

#include "Common/Singleton.h"
#include "Common/TseConsts.h"
#include "Routing/MileageDataRetriever.h"

namespace tse
{

class DataHandle;
class DateTime;
class MileageRouteItem;
class Mileage;

/**
 * @class MileageRetriever
 *
 * A singleton - stateless class to retrieve data from Mileage table.
 */
class MileageRetriever : public MileageDataRetriever
{
public:
  /**
   * @function retrieve
   *
   * Concrete implementation of retrieval from Mileage table.
   *
   * @param MileageRouteItem& - mileage route item that provides data needed to match and retrieve
   *data
   *				 from db, and to be updated with the retrieved mileage data
   * @param DataHandle& - data handle through which the data will be actually retrieved
   * @param Indicator - mileage type to identify desired records in db
   * @return bool - true if retrieval was successful, false otherwise
   */
  virtual bool retrieve(MileageRouteItem&, DataHandle&, Indicator) const override;

protected:
  /**
   * Construction, destruction and copying prohibited for a singleton.
   */
  MileageRetriever() {}
  MileageRetriever(const MileageRetriever&);
  MileageRetriever& operator=(const MileageRetriever&);
  /**
   * Protected destructor not really necessary, Should it cause any problem,
   * can be safely made back public (and friendship of unique_ptr removed).
   */
  ~MileageRetriever() {}
  friend class tse::Singleton<MileageRetriever>;
  friend class std::unique_ptr<MileageRetriever>;

private:
  /**
   * An auxiliary method to make unit tests independent of DataHandle.
   */
  virtual const Mileage*
  getData(DataHandle&, const LocCode&, const LocCode&, Indicator, GlobalDirection, const DateTime&)
      const;
};

} // namespace tse

