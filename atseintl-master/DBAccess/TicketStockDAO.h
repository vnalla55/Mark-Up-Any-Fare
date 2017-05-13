//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

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
class TicketStock;
class DeleteList;

class TicketStockDAO : public DataAccessObject<IntKey, std::vector<TicketStock*>, false>
{
public:
  static TicketStockDAO& instance();
  const std::vector<TicketStock*>&
  get(DeleteList& del, int key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, IntKey& key) const override
  {
    return key.initialized = objectKey.getValue("TKTSTOCKCODE", key._a);
  }

  IntKey createKey(TicketStock* info);

  void translateKey(const IntKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("TKTSTOCKCODE", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<TicketStock, TicketStockDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TicketStockDAO>;
  static DAOHelper<TicketStockDAO> _helper;
  TicketStockDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<IntKey, std::vector<TicketStock*>, false>(cacheSize, cacheType)
  {
  }
  virtual void load() override;
  std::vector<TicketStock*>* create(IntKey key) override;
  void destroy(IntKey key, std::vector<TicketStock*>* recs) override;

private:
  struct isEffective;
  static TicketStockDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: TicketStockHistoricalDAO
// --------------------------------------------------
class TicketStockHistoricalDAO
    : public HistoricalDataAccessObject<int, std::vector<TicketStock*>, false>
{
public:
  static TicketStockHistoricalDAO& instance();
  const std::vector<TicketStock*>&
  get(DeleteList& del, int key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, int& key) const override
  {
    return objectKey.getValue("TKTSTOCKCODE", key);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TicketStockHistoricalDAO>;
  static DAOHelper<TicketStockHistoricalDAO> _helper;
  TicketStockHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<int, std::vector<TicketStock*>, false>(cacheSize, cacheType)
  {
  }

  std::vector<TicketStock*>* create(int key) override;
  void destroy(int key, std::vector<TicketStock*>* recs) override;

  virtual void load() override;

private:
  struct isEffective;
  struct groupByKey;
  static TicketStockHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
