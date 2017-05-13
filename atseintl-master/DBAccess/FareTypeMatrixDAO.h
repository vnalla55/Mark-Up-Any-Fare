//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Common/TseBoostStringTypes.h"
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
class FareTypeMatrix;
class DeleteList;

class FareTypeMatrixDAO : public DataAccessObject<IntKey, std::vector<FareTypeMatrix*>, false>
{
public:
  static FareTypeMatrixDAO& instance();

  const FareTypeMatrix*
  get(DeleteList& del, const FareType& key, const DateTime& date, const DateTime& ticketDate);

  const std::vector<FareTypeMatrix*>& getAll(DeleteList& del, const DateTime& date);

  bool translateKey(const ObjectKey& objectKey, IntKey& key) const override
  {
    key = IntKey(0);
    return key.initialized;
  }

  IntKey createKey(FareTypeMatrix* info);

  void translateKey(const IntKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("DUMMY", 0);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override;

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  std::vector<FareTypeMatrix*>* create(IntKey key) override;
  void destroy(IntKey key, std::vector<FareTypeMatrix*>* recs) override;

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FareTypeMatrixDAO>;

  static DAOHelper<FareTypeMatrixDAO> _helper;

  FareTypeMatrixDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<IntKey, std::vector<FareTypeMatrix*>, false>(cacheSize, cacheType, 2)
  {
  }

  virtual void load() override;

private:
  static FareTypeMatrixDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: FareTypeMatrixHistoricalDAO
// --------------------------------------------------
class FareTypeMatrixHistoricalDAO
    : public HistoricalDataAccessObject<int, std::vector<FareTypeMatrix*>, false>
{
public:
  static FareTypeMatrixHistoricalDAO& instance();

  const FareTypeMatrix*
  get(DeleteList& del, const FareType& key, const DateTime& date, const DateTime& ticketDate);

  const std::vector<FareTypeMatrix*>& getAll(DeleteList& del, const DateTime& date);

  bool translateKey(const ObjectKey& objectKey, int& key) const override
  {
    key = dummy;
    return true;
  }

  const std::string& cacheClass() override { return _cacheClass; }

  std::vector<FareTypeMatrix*>* create(int key) override;
  void destroy(int key, std::vector<FareTypeMatrix*>* recs) override;

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FareTypeMatrixHistoricalDAO>;

  static DAOHelper<FareTypeMatrixHistoricalDAO> _helper;

  FareTypeMatrixHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<int, std::vector<FareTypeMatrix*>, false>(cacheSize, cacheType)
  {
  }

  virtual void load() override;

private:
  static const int dummy = 0;
  static FareTypeMatrixHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
