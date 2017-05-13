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
#include "DBAccess/FreqFlyerStatusSeg.h"
#include "DBAccess/HistoricalDataAccessObject.h"

#include <iostream>
#include <vector>

namespace tse
{
using FFStatusSegKey = HashKey<CarrierCode>;

class Logger;

class FrequentFlyerStatusSegDAO
    : public DataAccessObject<FFStatusSegKey, std::vector<FreqFlyerStatusSeg*>, false>
{
public:
  static FrequentFlyerStatusSegDAO& instance();
  Logger& getLogger() override { return _logger; }
  const std::string& cacheClass() override { return _cacheClass; }

  std::vector<const FreqFlyerStatusSeg*>
  get(DeleteList& del, const CarrierCode carrier, const DateTime& date, const DateTime& ticketDate);
  bool translateKey(const ObjectKey& objectKey, FFStatusSegKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }
  void translateKey(const FFStatusSegKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
  }
  std::vector<FreqFlyerStatusSeg*>* create(FFStatusSegKey key) override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FrequentFlyerStatusSegDAO>;
  static DAOHelper<FrequentFlyerStatusSegDAO> _helper;

  FrequentFlyerStatusSegDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FFStatusSegKey, std::vector<FreqFlyerStatusSeg*>, false>(
          cacheSize, cacheType, 1)
  {
  }
  void destroy(FFStatusSegKey key, std::vector<FreqFlyerStatusSeg*>* t) override;

  size_t clear() override;

private:
  static FrequentFlyerStatusSegDAO* _instance;
  static Logger _logger;
};

using FFStatusHistoricalSegKey = HashKey<CarrierCode, DateTime, DateTime>;
class FrequentFlyerStatusSegHistoricalDAO
    : public HistoricalDataAccessObject<FFStatusHistoricalSegKey,
                                        std::vector<FreqFlyerStatusSeg*>,
                                        false>
{
public:
  static FrequentFlyerStatusSegHistoricalDAO& instance();
  Logger& getLogger() override { return _logger; }
  const std::string& cacheClass() override { return _cacheClass; }

  std::vector<const FreqFlyerStatusSeg*>
  get(DeleteList& del, const CarrierCode carrier, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey,
                    FFStatusHistoricalSegKey& key,
                    const DateTime ticketDate) const override
  {
    setKeyDateRange(key, ticketDate);
    key.initialized = objectKey.getValue("CARRIER", key._a);
    return key.initialized;
  }

  void
  translateKey(const FFStatusHistoricalSegKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
  }

  void setKeyDateRange(FFStatusHistoricalSegKey& key, const DateTime date) const override
  {
    DAOUtils::getDateRange(date, key._b, key._c, _cacheBy);
  }

  std::vector<FreqFlyerStatusSeg*>* create(FFStatusHistoricalSegKey key) override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<FrequentFlyerStatusSegHistoricalDAO>;
  static DAOHelper<FrequentFlyerStatusSegHistoricalDAO> _helper;

  FrequentFlyerStatusSegHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FFStatusHistoricalSegKey, std::vector<FreqFlyerStatusSeg*>, false>(
          cacheSize, cacheType, 1)
  {
  }
  void destroy(FFStatusHistoricalSegKey key, std::vector<FreqFlyerStatusSeg*>* statuses) override;

private:
  struct isEffective;
  static FrequentFlyerStatusSegHistoricalDAO* _instance;
  static Logger _logger;
};
} // tse namespace
