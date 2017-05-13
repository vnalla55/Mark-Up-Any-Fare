//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class ATPResNationZones;
class DeleteList;

class ATPResNationZonesDAO
    : public DataAccessObject<NationKey, std::vector<ATPResNationZones*>, false>
{
public:
  static ATPResNationZonesDAO& instance();

  const std::vector<ATPResNationZones*>& get(DeleteList& del, const NationCode& key);

  virtual bool translateKey(const ObjectKey& objectKey, NationKey& key) const override
  {
    return key.initialized = objectKey.getValue("NATIONCODE", key._a);
  }

  NationKey createKey(ATPResNationZones* info);

  void translateKey(const NationKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("NATIONCODE", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<ATPResNationZones, ATPResNationZonesDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<ATPResNationZones*>* vect) const override;

  virtual std::vector<ATPResNationZones*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<ATPResNationZonesDAO>;
  static DAOHelper<ATPResNationZonesDAO> _helper;

  ATPResNationZonesDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<NationKey, std::vector<ATPResNationZones*>, false>(cacheSize, cacheType)
  {
  }

  virtual void load() override;
  std::vector<ATPResNationZones*>* create(NationKey key) override;
  void destroy(NationKey key, std::vector<ATPResNationZones*>* t) override;

private:
  static ATPResNationZonesDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

