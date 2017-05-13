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
class SalesNationRestr;
class DeleteList;

class SalesNationRestrDAO : public DataAccessObject<NationCode, std::vector<SalesNationRestr*> >
{
public:
  static SalesNationRestrDAO& instance();
  const std::vector<SalesNationRestr*>&
  get(DeleteList& del, const NationCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, NationCode& key) const override
  {
    return objectKey.getValue("NATIONCODE", key);
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SalesNationRestrDAO>;
  static DAOHelper<SalesNationRestrDAO> _helper;
  SalesNationRestrDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<NationCode, std::vector<SalesNationRestr*> >(cacheSize, cacheType)
  {
  }
  std::vector<SalesNationRestr*>* create(NationCode key) override;
  void destroy(NationCode key, std::vector<SalesNationRestr*>* t) override;

private:
  struct isEffective;
  static SalesNationRestrDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: SalesNationRestrHistoricalDAO
// --------------------------------------------------
class SalesNationRestrHistoricalDAO
    : public HistoricalDataAccessObject<NationCode, std::vector<SalesNationRestr*> >
{
public:
  static SalesNationRestrHistoricalDAO& instance();
  const std::vector<SalesNationRestr*>&
  get(DeleteList& del, const NationCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, NationCode& key) const override
  {
    return objectKey.getValue("NATIONCODE", key);
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SalesNationRestrHistoricalDAO>;
  static DAOHelper<SalesNationRestrHistoricalDAO> _helper;
  SalesNationRestrHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<NationCode, std::vector<SalesNationRestr*> >(cacheSize, cacheType)
  {
  }
  virtual void load() override;
  std::vector<SalesNationRestr*>* create(NationCode key) override;
  void destroy(NationCode key, std::vector<SalesNationRestr*>* t) override;

private:
  struct isEffective;
  struct groupByKey;
  static SalesNationRestrHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
