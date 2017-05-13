#pragma once

#include "Common/TseConsts.h"

namespace tse
{

class DataHandle;
class MileageRouteItem;

/**
 * @class MileageDataRetriever
 *
 * A protocol class for all the classes that retrieve data from
 * db/cache for individual route items with use of DataHandle.
 */
class MileageDataRetriever
{
public:
  /**
   * @function retrieve
   *
   * An abstract method - defines interface for all concrete methods retrieving data.
   *
   * @param MileageRouteItem& - mileage route item that provides data needed to match and retrieve
   *data
   *				 from db, and to be updated with the retrieved data
   * @param DataHandle& - data handle through which the data will be actually retrieved
   * @param Indicator - mileage type to identify desired records in db, default TPM is for
   *			 derivables that do not require this parameter passed
   * @return bool - true if retrieval was successful, false otherwise
   */
  virtual bool retrieve(MileageRouteItem&, DataHandle&, Indicator mileageType = TPM) const = 0;

protected:
  virtual ~MileageDataRetriever() {}
};

} // namespace tse

