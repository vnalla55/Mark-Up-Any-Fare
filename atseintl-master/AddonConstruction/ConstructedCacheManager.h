/**
 * @file     ConstructedCacheManager.h
 * @date     02/16/2005
 * @author   Konstantin Sidorin, Vadim Nikushin
 *
 * @brief    Header file for Constructed Fares Cache Manager.
 *
 *  Copyright Sabre 2005
 *
 *          The copyright to the computer program(s) herein
 *          is the property of Sabre.
 *          The program(s) may be used and/or copied only with
 *          the written permission of Sabre or in accordance
 *          with the terms and conditions stipulated in the
 *          agreement/contract under which the program(s)
 *          have been supplied.
 *
 */

#pragma once

#include "AddonConstruction/ACKeyedFactory.h"
#include "AddonConstruction/ACLRUCache.h"
#include "AddonConstruction/ConstructionDefs.h"
#include "DBAccess/DataHandle.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
};

namespace tse
{
class ConstructedCacheManager;
/**
 * @class ConstructedCacheManager ConstructedCacheManager.h
 *
 * @brief manages Constructed Fares Cache.
 *
 * for this class.
 *
 */


class ConstructedCacheManager
{

public:
  static ConstructedCacheManager& instance();

  ~ConstructedCacheManager() { delete _instance; }

  bool useCache() { return _useCache; }

  inline ACLRUCache& cache() { return _aclruCache; }
  inline ACLRUCache& aclruCache() { return _aclruCache; }
  inline uint32_t tableVersion() const { return _aclruCache.tableVersion(); }
  const std::string& name() { return _aclruCache.getName(); }
  inline log4cxx::LoggerPtr& getLogger() { return _logger; }

  template <typename T>
  void get(T*& t)
  {
    t = new T;
  }

  void flushAll();

  /**
  *   @method flush
  *
  *   Description: Flushes everything from the Addon Constructed Cache
  *                for this vendor and carrier.
  *
  *   @param  VendorCode        - vendor
  *   @param  CarrierCode       - carrier
  *
  *   @return void
  */
  void flushVendorCxrFares(const VendorCode& vendor, const CarrierCode& carrier);

  /**
  *   @method flush
  *
  *   Description: This method is invoked when the International Fare table
  *                FARINT is updated. Retrieves the Addon Construction Job
  *                and checks to see if either market1 or market2 matches
  *                any of the gateway pairs. If so this gateway pair is
  *                marked as invalid and when Addon Construction is run
  *                again only these gateway pairs will be reconstructed.
  *
  *   @param  LocCode        - market1
  *   @param  LocCode        - market2
  *   @param  VendorCode     - vendor
  *   @param  CarrierCode    - carrier
  *
  *   @return void
  */
  void flushSpecifiedFares(const LocCode& market1,
                           const LocCode& market2,
                           const VendorCode& vendor,
                           const CarrierCode& carrier);

  /**
  *   @method flush
  *
  *   Description: This method is invoked when the International Fare table
  *                FARADO is updated.
  *
  *   @param  LocCode        - interiorMarket
  *   @param  LocCode        - gatewayMarket
  *   @param  VendorCode     - vendor
  *   @param  CarrierCode    - carrier
  *
  *   @return void
  */
  void flushAddonFares(const LocCode& interiorMarket,
                       const LocCode& gatewayMarket,
                       const VendorCode& vendor,
                       const CarrierCode& carrier);

  size_t size();

protected:
  bool _useCache;

private:
  ConstructedCacheManager();
  ConstructedCacheManager(const ConstructedCacheManager&);
  ConstructedCacheManager& operator=(const ConstructedCacheManager&);

  friend class MockDataManager; // mock object for testing
  friend class AddonConstructionTest;

private:
  static ACKeyedFactory _acKeyedFactory;
  static ACLRUCache _aclruCache;
  static ConstructedCacheManager* _instance;

  static log4cxx::LoggerPtr _logger;
};

} // namespace tse
