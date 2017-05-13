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
#include "Common/TsePrimitiveTypes.h"
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
class Mileage;
class DeleteList;

typedef HashKey<LocCode, LocCode, Indicator> MileageKey;

class MileageDAO : public DataAccessObject<MileageKey, std::vector<Mileage*> >
{
public:
  static MileageDAO& instance();

  const std::vector<Mileage*>& get(DeleteList& del,
                                   const LocCode& orig,
                                   const LocCode& dest,
                                   Indicator mileageType,
                                   const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, MileageKey& key) const override
  {
    return key.initialized = objectKey.getValue("ORIGLOC", key._a) &&
                             objectKey.getValue("DESTLOC", key._b) &&
                             objectKey.getValue("MILEAGETYPE", key._c);
  }

  MileageKey createKey(Mileage* info);

  void translateKey(const MileageKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("ORIGLOC", key._a);
    objectKey.setValue("DESTLOC", key._b);
    objectKey.setValue("MILEAGETYPE", key._c);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<Mileage, MileageDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<Mileage*>* vect) const override;

  virtual std::vector<Mileage*>* uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MileageDAO>;
  static DAOHelper<MileageDAO> _helper;
  MileageDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<MileageKey, std::vector<Mileage*> >(cacheSize, cacheType)
  {
  }
  void load() override;
  std::vector<Mileage*>* create(MileageKey key) override;
  void destroy(MileageKey key, std::vector<Mileage*>* t) override;

private:
  struct isEffective;
  static MileageDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: MileageHistoricalDAO
// --------------------------------------------------
typedef HashKey<LocCode, LocCode, Indicator, DateTime, DateTime> MileageHistoricalKey;

class MileageHistoricalDAO
    : public HistoricalDataAccessObject<MileageHistoricalKey, std::vector<Mileage*> >
{
public:
  static MileageHistoricalDAO& instance();

  const std::vector<Mileage*>& get(DeleteList& del,
                                   const LocCode& orig,
                                   const LocCode& dest,
                                   Indicator mileageType,
                                   const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, MileageHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("ORIGLOC", key._a) && objectKey.getValue("DESTLOC", key._b) &&
               objectKey.getValue("MILEAGETYPE", key._c) &&
               objectKey.getValue("STARTDATE", key._d) && objectKey.getValue("ENDDATE", key._e);
  }

  bool translateKey(const ObjectKey& objectKey,
                    MileageHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
    return key.initialized = objectKey.getValue("ORIGLOC", key._a) &&
                             objectKey.getValue("DESTLOC", key._b) &&
                             objectKey.getValue("MILEAGETYPE", key._c);
  }

  MileageHistoricalKey createKey(Mileage* info,
                                 const DateTime& startDate = DateTime::emptyDate(),
                                 const DateTime& endDate = DateTime::emptyDate());

  void translateKey(const MileageHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("ORIGLOC", key._a);
    objectKey.setValue("DESTLOC", key._b);
    objectKey.setValue("MILEAGETYPE", key._c);
    objectKey.setValue("STARTDATE", key._d);
    objectKey.setValue("ENDDATE", key._e);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserterWithDateRange<Mileage, MileageHistoricalDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  void setKeyDateRange(MileageHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<Mileage*>* vect) const override;

  virtual std::vector<Mileage*>* uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MileageHistoricalDAO>;
  static DAOHelper<MileageHistoricalDAO> _helper;
  MileageHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<MileageHistoricalKey, std::vector<Mileage*> >(cacheSize, cacheType)
  {
  }
  std::vector<Mileage*>* create(MileageHistoricalKey key) override;
  void destroy(MileageHistoricalKey key, std::vector<Mileage*>* t) override;
  void load() override;

private:
  struct isEffective;
  static MileageHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
