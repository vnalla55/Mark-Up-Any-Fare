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
class CopParticipatingNation;
class DeleteList;

typedef HashKey<NationCode, NationCode> CopParticipatingNationKey;

class CopParticipatingNationDAO : public DataAccessObject<CopParticipatingNationKey,
                                                          std::vector<CopParticipatingNation*>,
                                                          false>
{
public:
  static CopParticipatingNationDAO& instance();

  const CopParticipatingNation* get(DeleteList& del,
                                    const NationCode& nation,
                                    const NationCode& copNation,
                                    const DateTime& date,
                                    const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CopParticipatingNationKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("NATION", key._a) && objectKey.getValue("COPNATION", key._b);
  }

  CopParticipatingNationKey createKey(CopParticipatingNation* info);

  void translateKey(const CopParticipatingNationKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("NATION", key._a);
    objectKey.setValue("COPNATION", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<CopParticipatingNation, CopParticipatingNationDAO>(
               flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<CopParticipatingNationDAO>;
  static DAOHelper<CopParticipatingNationDAO> _helper;

  CopParticipatingNationDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CopParticipatingNationKey, std::vector<CopParticipatingNation*>, false>(
          cacheSize, cacheType)
  {
  }

  virtual void load() override;
  std::vector<CopParticipatingNation*>* create(CopParticipatingNationKey key) override;
  void destroy(CopParticipatingNationKey key, std::vector<CopParticipatingNation*>* t) override;

private:
  struct isEffective;
  static CopParticipatingNationDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: CopParticipatingNationHistoricalDAO
// --------------------------------------------------
class CopParticipatingNationHistoricalDAO
    : public HistoricalDataAccessObject<CopParticipatingNationKey,
                                        std::vector<CopParticipatingNation*>,
                                        false>
{
public:
  static CopParticipatingNationHistoricalDAO& instance();

  const CopParticipatingNation* get(DeleteList& del,
                                    const NationCode& nation,
                                    const NationCode& copNation,
                                    const DateTime& date,
                                    const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CopParticipatingNationKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("NATION", key._a) && objectKey.getValue("COPNATION", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<CopParticipatingNationHistoricalDAO>;
  static DAOHelper<CopParticipatingNationHistoricalDAO> _helper;

  CopParticipatingNationHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<CopParticipatingNationKey,
                                 std::vector<CopParticipatingNation*>,
                                 false>(cacheSize, cacheType)
  {
  }

  std::vector<CopParticipatingNation*>* create(CopParticipatingNationKey key) override;
  void destroy(CopParticipatingNationKey key, std::vector<CopParticipatingNation*>* t) override;

  virtual void load() override;

private:
  struct isEffective;
  struct groupByKey;
  static CopParticipatingNationHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
