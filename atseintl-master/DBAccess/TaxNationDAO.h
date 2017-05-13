//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"

namespace tse
{
class TaxNation;
class DeleteList;

class TaxNationDAO : public DataAccessObject<NationKey, std::vector<TaxNation*> >
{
public:
  static TaxNationDAO& instance();
  TaxNation*
  get(DeleteList& del, const NationCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, NationKey& key) const override
  {
    return key.initialized = objectKey.getValue("NATIONCODE", key._a);
  }

  NationKey createKey(const TaxNation* info);

  void translateKey(const NationKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("NATIONCODE", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<TaxNation, TaxNationDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<TaxNation*>* vect) const override;

  virtual std::vector<TaxNation*>* uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TaxNationDAO>;
  static DAOHelper<TaxNationDAO> _helper;
  TaxNationDAO(int cacheSize = 0, const std::string& cacheType = "", size_t version = 2)
    : DataAccessObject<NationKey, std::vector<TaxNation*> >(cacheSize, cacheType, version)
  {
  }
  std::vector<TaxNation*>* create(NationKey key) override;
  void destroy(NationKey key, std::vector<TaxNation*>* recs) override;
  void load() override;

private:
  struct isEffective;
  static TaxNationDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: TaxNationHistoricalDAO
// --------------------------------------------------
typedef HashKey<NationCode, DateTime, DateTime> TaxNationHistoricalKey;

class TaxNationHistoricalDAO
    : public HistoricalDataAccessObject<TaxNationHistoricalKey, std::vector<TaxNation*>, false>
{
public:
  static TaxNationHistoricalDAO& instance();
  TaxNation*
  get(DeleteList& del, const NationCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TaxNationHistoricalKey& key) const override
  {
    return key.initialized = objectKey.getValue("NATIONCODE", key._a) &&
                             objectKey.getValue("STARTDATE", key._b) &&
                             objectKey.getValue("ENDDATE", key._c);
  }

  bool translateKey(const ObjectKey& objectKey,
                    TaxNationHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("NATIONCODE", key._a);
  }

  void setKeyDateRange(TaxNationHistoricalKey& key, const DateTime date) const override
  {
    DAOUtils::getDateRange(date, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<TaxNation*>* vect) const override;

  virtual std::vector<TaxNation*>* uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TaxNationHistoricalDAO>;
  static DAOHelper<TaxNationHistoricalDAO> _helper;
  TaxNationHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "", size_t version = 2)
    : HistoricalDataAccessObject<TaxNationHistoricalKey, std::vector<TaxNation*>, false>(cacheSize,
                                                                                         cacheType,
                                                                                         version)
  {
  }
  std::vector<TaxNation*>* create(TaxNationHistoricalKey key) override;
  void destroy(TaxNationHistoricalKey key, std::vector<TaxNation*>* recs) override;

private:
  struct isEffective;
  static TaxNationHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
