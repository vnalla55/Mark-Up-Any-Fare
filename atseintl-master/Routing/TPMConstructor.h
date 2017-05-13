#pragma once

#include "Common/Singleton.h"
#include "Common/TseConsts.h"
#include "Routing/MileageDataRetriever.h"

namespace tse
{

class DataHandle;
class MileageRouteItem;

/**
 * @class TPMConstructor
 *
 * A singleton - stateless class to construct TPM based on retrieved MPM or GCM.
 */
class TPMConstructor : public MileageDataRetriever
{
public:
  /**
   * @function retrieve
   *
   * Concrete implementation of TPM construction
   *
   * @param MileageRouteItem& - mileage route item that provides data needed to match and retrieve
   *data,
   *				 and to be updated with constructed TPM
   * @param DataHandle& - data handle for creating objects in pooled memory
   * @param Indicator - ignored
   * @return bool - true if retrieval was successful, false otherwise
   */
  virtual bool retrieve(MileageRouteItem&, DataHandle&, Indicator mileageType = TPM) const override;

protected:
  /**
   * Construction, destruction and copying prohibited for a singleton.
   */
  TPMConstructor() {}
  TPMConstructor(const TPMConstructor&);
  TPMConstructor& operator=(const TPMConstructor&);
  /**
   * Protected destructor not really necessary, Should it cause any problem,
   * can be safely made back public (and friendship of unique_ptr removed).
   */
  ~TPMConstructor() {}
  friend class tse::Singleton<TPMConstructor>;
  friend class std::unique_ptr<TPMConstructor>;

private:
  /**
   * An auxiliary method to make unit tests independent of MileageRetriever.
   */
  virtual bool getData(MileageRouteItem&, DataHandle&) const;
};

} // namespace tse

