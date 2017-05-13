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
class MileageSubstitution;
class DeleteList;

class MileageSubstitutionDAO
    : public DataAccessObject<LocCodeKey, std::vector<MileageSubstitution*>, false>
{
public:
  static MileageSubstitutionDAO& instance();
  const MileageSubstitution*
  get(DeleteList& del, const LocCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, LocCodeKey& key) const override
  {
    return key.initialized = objectKey.getValue("LOCCODE", key._a);
  }

  LocCodeKey createKey(MileageSubstitution* info);

  void translateKey(const LocCodeKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("LOCCODE", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<MileageSubstitution, MileageSubstitutionDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<MileageSubstitution*>* vect) const override;

  virtual std::vector<MileageSubstitution*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MileageSubstitutionDAO>;
  static DAOHelper<MileageSubstitutionDAO> _helper;
  MileageSubstitutionDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<LocCodeKey, std::vector<MileageSubstitution*>, false>(cacheSize, cacheType)
  {
  }
  virtual void load() override;
  std::vector<MileageSubstitution*>* create(LocCodeKey key) override;
  void destroy(LocCodeKey key, std::vector<MileageSubstitution*>* t) override;

private:
  struct isEffective;
  static MileageSubstitutionDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: MileageSubstitutionHistoricalDAO
// --------------------------------------------------
class MileageSubstitutionHistoricalDAO
    : public HistoricalDataAccessObject<LocCode, std::vector<MileageSubstitution*>, false>
{
public:
  static MileageSubstitutionHistoricalDAO& instance();
  const MileageSubstitution*
  get(DeleteList& del, const LocCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, LocCode& key) const override
  {
    return objectKey.getValue("LOCCODE", key);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<MileageSubstitution*>* vect) const override;

  virtual std::vector<MileageSubstitution*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MileageSubstitutionHistoricalDAO>;
  static DAOHelper<MileageSubstitutionHistoricalDAO> _helper;
  MileageSubstitutionHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<LocCode, std::vector<MileageSubstitution*>, false>(cacheSize,
                                                                                    cacheType)
  {
  }
  std::vector<MileageSubstitution*>* create(LocCode key) override;
  void destroy(LocCode key, std::vector<MileageSubstitution*>* t) override;

  virtual void load() override;

private:
  struct isEffective;
  struct groupByKey;
  static MileageSubstitutionHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
