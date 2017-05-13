//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class TariffMileageAddon;
class DeleteList;

typedef HashKey<CarrierCode, LocCode, GlobalDirection> TariffMileageAddonKey;

class TariffMileageAddonDAO
    : public DataAccessObject<TariffMileageAddonKey, std::vector<TariffMileageAddon*>, false>
{
public:
  static TariffMileageAddonDAO& instance();
  const TariffMileageAddon* get(DeleteList& del,
                                const CarrierCode& carrier,
                                const LocCode& unpublishedAddonLoc,
                                const GlobalDirection& globalDir,
                                const DateTime& date,
                                const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TariffMileageAddonKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a) &&
                             objectKey.getValue("LOCCODE", key._b) &&
                             objectKey.getValue("GLOBALDIRECTION", key._c);
  }

  TariffMileageAddonKey createKey(TariffMileageAddon* info);

  void translateKey(const TariffMileageAddonKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
    objectKey.setValue("LOCCODE", key._b);
    objectKey.setValue("GLOBALDIRECTION", key._c);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<TariffMileageAddon, TariffMileageAddonDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TariffMileageAddonDAO>;
  static DAOHelper<TariffMileageAddonDAO> _helper;
  TariffMileageAddonDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<TariffMileageAddonKey, std::vector<TariffMileageAddon*>, false>(cacheSize,
                                                                                       cacheType)
  {
  }
  virtual void load() override;
  std::vector<TariffMileageAddon*>* create(TariffMileageAddonKey key) override;
  void destroy(TariffMileageAddonKey key, std::vector<TariffMileageAddon*>* t) override;

private:
  struct isEffective;
  static TariffMileageAddonDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: TariffMileageAddonHistoricalDAO
// --------------------------------------------------
class TariffMileageAddonHistoricalDAO
    : public HistoricalDataAccessObject<TariffMileageAddonKey,
                                        std::vector<TariffMileageAddon*>,
                                        false>
{
public:
  static TariffMileageAddonHistoricalDAO& instance();
  const TariffMileageAddon* get(DeleteList& del,
                                const CarrierCode& carrier,
                                const LocCode& unpublishedAddonLoc,
                                const GlobalDirection& globalDir,
                                const DateTime& date,
                                const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TariffMileageAddonKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a) &&
                             objectKey.getValue("LOCCODE", key._b) &&
                             objectKey.getValue("GLOBALDIRECTION", key._c);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TariffMileageAddonHistoricalDAO>;
  static DAOHelper<TariffMileageAddonHistoricalDAO> _helper;
  TariffMileageAddonHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<TariffMileageAddonKey, std::vector<TariffMileageAddon*>, false>(
          cacheSize, cacheType)
  {
  }
  std::vector<TariffMileageAddon*>* create(TariffMileageAddonKey key) override;
  void destroy(TariffMileageAddonKey key, std::vector<TariffMileageAddon*>* t) override;

  virtual void load() override;

private:
  struct isEffective;
  struct groupByKey;
  static TariffMileageAddonHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
