//-------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//-------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/FreqFlyerStatus.h"
#include "DBAccess/HistoricalDataAccessObject.h"

#include <iostream>
#include <vector>

namespace tse
{
using FFStatusKey = HashKey<CarrierCode>;

class Logger;

class FrequentFlyerStatusDAO
    : public DataAccessObject<FFStatusKey, std::vector<FreqFlyerStatus*>, false>
{
  friend class FrequentFlyerStatusDAOTest;
public:
  static FrequentFlyerStatusDAO& instance();
  Logger& getLogger() override { return _logger; }
  const std::string& cacheClass() override { return _cacheClass; }

  std::vector<const FreqFlyerStatus*> get(DeleteList& del, const CarrierCode carrier);
  bool translateKey(const ObjectKey& objectKey, FFStatusKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }
  void translateKey(const FFStatusKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
  }
  std::vector<FreqFlyerStatus*>* create(FFStatusKey key) override;

protected:
  static const std::string _name;
  static const std::string _cacheClass;
  friend class DAOHelper<FrequentFlyerStatusDAO>;
  static DAOHelper<FrequentFlyerStatusDAO> _helper;

  FrequentFlyerStatusDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FFStatusKey, std::vector<FreqFlyerStatus*>, false>(
          cacheSize, cacheType, 1)
  {
  }
  void destroy(FFStatusKey key, std::vector<FreqFlyerStatus*>* t) override;

  size_t clear() override;

private:
  static FrequentFlyerStatusDAO* _instance;
  static Logger _logger;
};

using FFStatusHistoricalKey = HashKey<CarrierCode, DateTime, DateTime>;
class FrequentFlyerStatusHistoricalDAO
    : public HistoricalDataAccessObject<FFStatusHistoricalKey, std::vector<FreqFlyerStatus*>, false>
{
public:
  static FrequentFlyerStatusHistoricalDAO& instance();
  Logger& getLogger() override { return _logger; }
  const std::string& cacheClass() override { return _cacheClass; }

  std::vector<const FreqFlyerStatus*>
  get(DeleteList& del, const CarrierCode carrier, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey,
                    FFStatusHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    setKeyDateRange(key, ticketDate);
    key.initialized = objectKey.getValue("CARRIER", key._a);
    return key.initialized;
  }

  void translateKey(const FFStatusHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
  }

  void setKeyDateRange(FFStatusHistoricalKey& key, const DateTime date) const override
  {
    DAOUtils::getDateRange(date, key._b, key._c, _cacheBy);
  }

  std::vector<FreqFlyerStatus*>* create(FFStatusHistoricalKey key) override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FrequentFlyerStatusHistoricalDAO>;
  static DAOHelper<FrequentFlyerStatusHistoricalDAO> _helper;

  FrequentFlyerStatusHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FFStatusHistoricalKey, std::vector<FreqFlyerStatus*>, false>(
          cacheSize, cacheType, 1)
  {
  }
  void destroy(FFStatusHistoricalKey key, std::vector<FreqFlyerStatus*>* statuses) override;

private:
  struct isEffective;
  static FrequentFlyerStatusHistoricalDAO* _instance;
  static Logger _logger;
};

} // tse namespace
