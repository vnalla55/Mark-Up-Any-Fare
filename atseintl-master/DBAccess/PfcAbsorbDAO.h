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
class PfcAbsorb;
class DeleteList;

typedef HashKey<LocCode, CarrierCode> PfcAbsorbKey;

class PfcAbsorbDAO : public DataAccessObject<PfcAbsorbKey, std::vector<PfcAbsorb*>, false>
{
public:
  static PfcAbsorbDAO& instance();
  const std::vector<PfcAbsorb*>& get(DeleteList& del,
                                     const LocCode& pfcAirport,
                                     const CarrierCode& localCarrier,
                                     const DateTime& date,
                                     const DateTime& ticketDate);

  const std::vector<PfcAbsorb*>& getAll(DeleteList& del);

  bool translateKey(const ObjectKey& objectKey, PfcAbsorbKey& key) const override
  {
    return key.initialized = objectKey.getValue("PFCAIRPORT", key._a) &&
                             objectKey.getValue("LOCALCARRIER", key._b);
  }

  PfcAbsorbKey createKey(PfcAbsorb* info);

  void translateKey(const PfcAbsorbKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("PFCAIRPORT", key._a);
    objectKey.setValue("LOCALCARRIER", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<PfcAbsorb, PfcAbsorbDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<PfcAbsorbDAO>;
  virtual size_t clear() override;
  virtual size_t invalidate(const ObjectKey& objectKey) override;
  static DAOHelper<PfcAbsorbDAO> _helper;
  PfcAbsorbDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<PfcAbsorbKey, std::vector<PfcAbsorb*>, false>(cacheSize, cacheType, 2)
  {
  }

  virtual void load() override;

  std::vector<PfcAbsorb*>* create(PfcAbsorbKey key) override;
  void destroy(PfcAbsorbKey key, std::vector<PfcAbsorb*>* t) override;

  virtual sfc::CompressedData* compress(const std::vector<PfcAbsorb*>* vect) const override;

  virtual std::vector<PfcAbsorb*>* uncompress(const sfc::CompressedData& compressed) const override;

private:
  struct isEffective;
  static PfcAbsorbDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: PfcAbsorbHistoricalDAO
// --------------------------------------------------

typedef HashKey<LocCode, CarrierCode, DateTime, DateTime> PfcAbsorbHistoricalKey;

class PfcAbsorbHistoricalDAO
    : public HistoricalDataAccessObject<PfcAbsorbHistoricalKey, std::vector<PfcAbsorb*>, false>
{
public:
  static PfcAbsorbHistoricalDAO& instance();
  const std::vector<PfcAbsorb*>& get(DeleteList& del,
                                     const LocCode& pfcAirport,
                                     const CarrierCode& localCarrier,
                                     const DateTime& date,
                                     const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, PfcAbsorbHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("PFCAIRPORT", key._a)
                             && objectKey.getValue("LOCALCARRIER", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    PfcAbsorbHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized = objectKey.getValue("PFCAIRPORT", key._a) &&
                             objectKey.getValue("LOCALCARRIER", key._b);
  }

  void setKeyDateRange(PfcAbsorbHistoricalKey& key, const DateTime& date) const
  {
    DAOUtils::getDateRange(date, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<PfcAbsorb*>* vect) const override;

  virtual std::vector<PfcAbsorb*>* uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<PfcAbsorbHistoricalDAO>;
  static DAOHelper<PfcAbsorbHistoricalDAO> _helper;
  PfcAbsorbHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<PfcAbsorbHistoricalKey, std::vector<PfcAbsorb*>, false>(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<PfcAbsorb*>* create(PfcAbsorbHistoricalKey key) override;
  void destroy(PfcAbsorbHistoricalKey key, std::vector<PfcAbsorb*>* t) override;

private:
  struct isEffective;
  static PfcAbsorbHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse
