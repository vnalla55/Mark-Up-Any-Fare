//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
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
#include "DBAccess/HistoricalDataAccessObject.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class State;
class DeleteList;

typedef HashKey<NationCode, StateCode> StateKey;

class StateDAO : public DataAccessObject<StateKey, std::vector<State*>, false>
{
public:
  static StateDAO& instance();

  const State* get(DeleteList& del,
                   const NationCode& nation,
                   const StateCode& state,
                   const DateTime& date,
                   const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, StateKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("NATIONCODE", key._a) && objectKey.getValue("STATECODE", key._b);
  }

  StateKey createKey(State* info);

  void translateKey(const StateKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("NATIONCODE", key._a);
    objectKey.setValue("STATECODE", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<State, StateDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<StateDAO>;
  static DAOHelper<StateDAO> _helper;
  StateDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<StateKey, std::vector<State*>, false>(cacheSize, cacheType)
  {
  }
  virtual void load() override;
  std::vector<State*>* create(StateKey key) override;
  void destroy(StateKey key, std::vector<State*>* recs) override;

private:
  static StateDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class StateDAO

// --------------------------------------------------
// Historical DAO: StateHistoricalDAO
// --------------------------------------------------
class StateHistoricalDAO : public HistoricalDataAccessObject<StateKey, std::vector<State*>, false>
{
public:
  static StateHistoricalDAO& instance();
  const State* get(DeleteList& del,
                   const NationCode& nation,
                   const StateCode& state,
                   const DateTime& date,
                   const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, StateKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("NATIONCODE", key._a) && objectKey.getValue("STATECODE", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<StateHistoricalDAO>;
  static DAOHelper<StateHistoricalDAO> _helper;
  StateHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<StateKey, std::vector<State*>, false>(cacheSize, cacheType)
  {
  }
  virtual void load() override;
  std::vector<State*>* create(StateKey key) override;
  void destroy(StateKey key, std::vector<State*>* recs) override;

private:
  struct groupByKey;
  static StateHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class StateHistoricalDAO
} // namespace tse
