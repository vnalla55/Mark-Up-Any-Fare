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
class Loc;
class DeleteList;

class LocDAO : public DataAccessObject<LocCodeKey, std::vector<Loc*>, false>
{
public:
  static LocDAO& instance();
  const Loc*
  get(DeleteList& del, const LocCode& key, const DateTime& date, const DateTime& ticketDate);

  NationCode getLocNation(const LocCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, LocCodeKey& key) const override
  {
    return key.initialized = objectKey.getValue("LOCCODE", key._a);
  }

  LocCodeKey createKey(Loc* info);

  void translateKey(const LocCodeKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("LOCCODE", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<Loc, LocDAO>(flatKey, objectKey).success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData* compress(const std::vector<Loc*>* vect) const override;

  virtual std::vector<Loc*>* uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<LocDAO>;
  static DAOHelper<LocDAO> _helper;
  LocDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<LocCodeKey, std::vector<Loc*>, false>(cacheSize, cacheType, 3)
  {
  }
  virtual void load() override;
  virtual size_t clear() override;

  std::vector<Loc*>* create(LocCodeKey key) override;
  void destroy(LocCodeKey key, std::vector<Loc*>* t) override;

private:
  struct isEffective;
  static LocDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: LocHistoricalDAO
// --------------------------------------------------
typedef HashKey<LocCode, DateTime, DateTime> LocHistoricalKey;

class LocHistoricalDAO
    : public HistoricalDataAccessObject<LocHistoricalKey, std::vector<Loc*>, false>
{
public:
  static LocHistoricalDAO& instance();

  const Loc*
  get(DeleteList& del, const LocCode& key, const DateTime& date, const DateTime& ticketDate);

  NationCode getLocNation(const LocCode& loc, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, LocHistoricalKey& key) const override
  {
    return key.initialized = objectKey.getValue("LOCCODE", key._a) &&
                             objectKey.getValue("STARTDATE", key._b) &&
                             objectKey.getValue("ENDDATE", key._c);
  }

  bool
  translateKey(const ObjectKey& objectKey, LocHistoricalKey& key, const DateTime ticketDate) const
      override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("LOCCODE", key._a);
  }

  void setKeyDateRange(LocHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<LocHistoricalDAO>;
  static DAOHelper<LocHistoricalDAO> _helper;
  LocHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<LocHistoricalKey, std::vector<Loc*>, false>(
          cacheSize, cacheType, 3)
  {
  }
  std::vector<Loc*>* create(LocHistoricalKey key) override;
  void destroy(LocHistoricalKey key, std::vector<Loc*>* t) override;
  virtual size_t clear() override;

  virtual sfc::CompressedData* compress(const std::vector<Loc*>* vect) const override;

  virtual std::vector<Loc*>* uncompress(const sfc::CompressedData& compressed) const override;

private:
  struct isEffective;
  static LocHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
