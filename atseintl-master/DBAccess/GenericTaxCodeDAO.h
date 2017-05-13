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
class GenericTaxCode;
class DeleteList;

class GenericTaxCodeDAO : public DataAccessObject<IntKey, std::vector<GenericTaxCode*>, false>
{
public:
  static GenericTaxCodeDAO& instance();
  const std::vector<GenericTaxCode*>&
  get(DeleteList& del, const TaxCode& key, const DateTime& date, const DateTime& ticketDate);

  const std::vector<GenericTaxCode*>& getAll(DeleteList& del, const DateTime& date);

  bool translateKey(const ObjectKey& objectKey, IntKey& key) const override
  {
    key = IntKey(0);
    return key.initialized;
  }

  IntKey createKey(GenericTaxCode* info);

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
  friend class DAOHelper<GenericTaxCodeDAO>;
  static DAOHelper<GenericTaxCodeDAO> _helper;
  GenericTaxCodeDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<IntKey, std::vector<GenericTaxCode*>, false>(cacheSize, cacheType)
  {
  }

  virtual void load() override;
  std::vector<GenericTaxCode*>* create(IntKey key) override;
  void destroy(IntKey key, std::vector<GenericTaxCode*>* t) override;

private:
  static GenericTaxCodeDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: GenericTaxCodeHistoricalDAO
// --------------------------------------------------

typedef HashKey<DateTime, DateTime> GenericTaxCodeHistoricalKey;

class GenericTaxCodeHistoricalDAO : public HistoricalDataAccessObject<GenericTaxCodeHistoricalKey,
                                                                      std::vector<GenericTaxCode*>,
                                                                      false>
{
public:
  static GenericTaxCodeHistoricalDAO& instance();
  const std::vector<GenericTaxCode*>&
  get(DeleteList& del, const TaxCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, GenericTaxCodeHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("STARTDATE", key._a) && objectKey.getValue("ENDDATE", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    GenericTaxCodeHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._a, key._b, _cacheBy);
    return key.initialized = true;
  }

  void setKeyDateRange(GenericTaxCodeHistoricalKey& key, const DateTime date) const override
  {
    DAOUtils::getDateRange(date, key._a, key._b, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<GenericTaxCodeHistoricalDAO>;
  static DAOHelper<GenericTaxCodeHistoricalDAO> _helper;
  GenericTaxCodeHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<GenericTaxCodeHistoricalKey, std::vector<GenericTaxCode*>, false>(
          cacheSize, cacheType)
  {
  }

  std::vector<GenericTaxCode*>* create(GenericTaxCodeHistoricalKey& key);
  void destroy(GenericTaxCodeHistoricalKey key, std::vector<GenericTaxCode*>* recs) override;

private:
  static GenericTaxCodeHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

