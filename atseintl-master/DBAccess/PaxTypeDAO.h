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
#include "DBAccess/HistoricalDataAccessObject.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class PaxTypeInfo;
class DeleteList;

typedef HashKey<PaxTypeCode, VendorCode> PaxTypeKey;

class PaxTypeDAO : public DataAccessObject<IntKey, std::multimap<PaxTypeKey, PaxTypeInfo*>, false>
{
public:
  static PaxTypeDAO& instance();
  const PaxTypeInfo* get(DeleteList& del,
                         const PaxTypeCode& paxType,
                         const VendorCode& vendor,
                         const DateTime& ticketDate);

  const std::vector<PaxTypeInfo*>& getAll(DeleteList& del);

  bool translateKey(const ObjectKey& objectKey, IntKey& key) const override
  {
    key = IntKey(0);
    return key.initialized;
  }

  IntKey createKey(PaxTypeInfo* info);

  void translateKey(const IntKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("DUMMY", 0);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override;

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<PaxTypeDAO>;
  static DAOHelper<PaxTypeDAO> _helper;
  PaxTypeDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<IntKey, std::multimap<PaxTypeKey, PaxTypeInfo*>, false>(cacheSize, cacheType)
  {
  }
  virtual void load() override;

  std::multimap<PaxTypeKey, PaxTypeInfo*>* create(IntKey key) override;
  void destroy(IntKey key, std::multimap<PaxTypeKey, PaxTypeInfo*>* recs) override;

private:
  static PaxTypeDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: PaxTypeHistoricalDAO
// --------------------------------------------------
class PaxTypeHistoricalDAO
    : public HistoricalDataAccessObject<int, std::multimap<PaxTypeKey, PaxTypeInfo*>, false>
{
public:
  static PaxTypeHistoricalDAO& instance();
  const PaxTypeInfo* get(DeleteList& del,
                         const PaxTypeCode& paxType,
                         const VendorCode& vendor,
                         const DateTime& ticketDate);

  const std::vector<PaxTypeInfo*>& getAll(DeleteList& del, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, int& key) const override
  {
    key = dummy;
    return true;
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<PaxTypeHistoricalDAO>;
  static DAOHelper<PaxTypeHistoricalDAO> _helper;
  PaxTypeHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<int, std::multimap<PaxTypeKey, PaxTypeInfo*>, false>(cacheSize,
                                                                                      cacheType)
  {
  }
  virtual void load() override;

  std::multimap<PaxTypeKey, PaxTypeInfo*>* create(int key) override;
  void destroy(int key, std::multimap<PaxTypeKey, PaxTypeInfo*>* recs) override;

private:
  static const int dummy = 0;
  static PaxTypeHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
