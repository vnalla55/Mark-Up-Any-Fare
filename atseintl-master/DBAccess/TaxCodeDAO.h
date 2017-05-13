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

namespace tse
{
class Logger;
class TaxCodeReg;
class DeleteList;

class TaxCodeDAO : public DataAccessObject<TaxCodeKey, std::vector<TaxCodeReg*>, false>
{
public:
  static TaxCodeDAO& instance();
  const std::vector<TaxCodeReg*>&
  get(DeleteList& del, const TaxCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TaxCodeKey& key) const override
  {
    return key.initialized = objectKey.getValue("TAXCODECODE", key._a);
  }

  TaxCodeKey createKey(TaxCodeReg* info);

  void translateKey(const TaxCodeKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("TAXCODECODE", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<TaxCodeReg, TaxCodeDAO>(flatKey, objectKey).success();
  }

  Logger& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<TaxCodeReg*>* vect) const override;

  virtual std::vector<TaxCodeReg*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TaxCodeDAO>;
  static DAOHelper<TaxCodeDAO> _helper;
  TaxCodeDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<TaxCodeKey, std::vector<TaxCodeReg*>, false>(cacheSize, cacheType, 4)
  {
  }
  virtual void load() override;
  std::vector<TaxCodeReg*>* create(TaxCodeKey key) override;
  void destroy(TaxCodeKey key, std::vector<TaxCodeReg*>* t) override;

private:
  static TaxCodeDAO* _instance;
  static Logger _logger;
};

// --------------------------------------------------
// Historical DAO: TaxCodeHistoricalDAO
// --------------------------------------------------

typedef HashKey<TaxCode, DateTime, DateTime> TaxCodeHistoricalKey;

class TaxCodeHistoricalDAO
    : public HistoricalDataAccessObject<TaxCodeHistoricalKey, std::vector<TaxCodeReg*>, false>
{
public:
  static TaxCodeHistoricalDAO& instance();

  const std::vector<TaxCodeReg*>&
  get(DeleteList& del, const TaxCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TaxCodeHistoricalKey& key) const override
  {
    return key.initialized = objectKey.getValue("TAXCODECODE", key._a) &&
                             objectKey.getValue("STARTDATE", key._b) &&
                             objectKey.getValue("ENDDATE", key._c);
  }

  bool translateKey(const ObjectKey& objectKey,
                    TaxCodeHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("TAXCODECODE", key._a);
  }

  void setKeyDateRange(TaxCodeHistoricalKey& key, const DateTime date) const override
  {
    DAOUtils::getDateRange(date, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<TaxCodeReg*>* vect) const override;

  virtual std::vector<TaxCodeReg*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TaxCodeHistoricalDAO>;
  static DAOHelper<TaxCodeHistoricalDAO> _helper;
  TaxCodeHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<TaxCodeHistoricalKey, std::vector<TaxCodeReg*>, false>(
          cacheSize, cacheType, 4)
  {
  }
  std::vector<TaxCodeReg*>* create(TaxCodeHistoricalKey key) override;
  void destroy(TaxCodeHistoricalKey key, std::vector<TaxCodeReg*>* t) override;

private:
  static TaxCodeHistoricalDAO* _instance;
  static Logger _logger;
};

} // namespace tse
