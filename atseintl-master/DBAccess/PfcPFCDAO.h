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
class PfcPFC;
class DeleteList;

class PfcPFCDAO : public DataAccessObject<LocCodeKey, std::vector<PfcPFC*>, false>
{
public:
  static PfcPFCDAO& instance();
  const PfcPFC*
  get(DeleteList& del, const LocCode& key, const DateTime& date, const DateTime& ticketDate);

  const std::vector<PfcPFC*>& getAll(DeleteList& del);

  bool translateKey(const ObjectKey& objectKey, LocCodeKey& key) const override
  {
    return key.initialized = objectKey.getValue("PFCAIRPORT", key._a);
  }

  LocCodeKey createKey(PfcPFC* info);

  void translateKey(const LocCodeKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("PFCAIRPORT", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<PfcPFC, PfcPFCDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<PfcPFC*>* vect) const override;

  virtual std::vector<PfcPFC*>* uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<PfcPFCDAO>;
  virtual size_t clear() override;
  virtual size_t invalidate(const ObjectKey& objectKey) override;
  static DAOHelper<PfcPFCDAO> _helper;
  PfcPFCDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<LocCodeKey, std::vector<PfcPFC*>, false>(cacheSize, cacheType)
  {
  }

  virtual void load() override;
  std::vector<PfcPFC*>* create(LocCodeKey key) override;
  void destroy(LocCodeKey key, std::vector<PfcPFC*>* t) override;

private:
  struct isEffective;
  static PfcPFCDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: PfcPFCHistoricalDAO
// --------------------------------------------------
typedef HashKey<NationCode, DateTime, DateTime> PfcPFCHistoricalKey;

class PfcPFCHistoricalDAO
    : public HistoricalDataAccessObject<PfcPFCHistoricalKey, std::vector<PfcPFC*>, false>
{
public:
  static PfcPFCHistoricalDAO& instance();
  const PfcPFC*
  get(DeleteList& del, const LocCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, PfcPFCHistoricalKey& key) const override
  {
    return key.initialized = objectKey.getValue("PFCAIRPORT", key._a) &&
                             objectKey.getValue("STARTDATE", key._b) &&
                             objectKey.getValue("ENDDATE", key._c);
  }

  bool translateKey(const ObjectKey& objectKey,
                    PfcPFCHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("PFCAIRPORT", key._a);
  }

  void setKeyDateRange(PfcPFCHistoricalKey& key, const DateTime date) const override
  {
    DAOUtils::getDateRange(date, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<PfcPFC*>* vect) const override;

  virtual std::vector<PfcPFC*>* uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<PfcPFCHistoricalDAO>;
  static DAOHelper<PfcPFCHistoricalDAO> _helper;
  PfcPFCHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<PfcPFCHistoricalKey, std::vector<PfcPFC*>, false>(cacheSize,
                                                                                   cacheType)
  {
  }

  std::vector<PfcPFC*> loadList;

  std::vector<PfcPFC*>* create(PfcPFCHistoricalKey key) override;
  void destroy(PfcPFCHistoricalKey key, std::vector<PfcPFC*>* t) override;

private:
  struct isEffective;
  static PfcPFCHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
