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

class PfcEquipTypeExempt;
class DeleteList;

typedef HashKey<EquipmentType, StateCode> PfcEquipTypeExemptKey;

class PfcEquipTypeExemptDAO
    : public DataAccessObject<PfcEquipTypeExemptKey, std::vector<PfcEquipTypeExempt*>, false>
{
public:
  static PfcEquipTypeExemptDAO& instance();
  const PfcEquipTypeExempt* get(DeleteList& del,
                                const EquipmentType& equip,
                                const StateCode& state,
                                const DateTime& date,
                                const DateTime& ticketDate);

  const std::vector<PfcEquipTypeExempt*>& getAll(DeleteList& del);

  bool translateKey(const ObjectKey& objectKey, PfcEquipTypeExemptKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("EQUIP", key._a) && objectKey.getValue("STATE", key._b);
  }

  PfcEquipTypeExemptKey createKey(PfcEquipTypeExempt* info);

  void translateKey(const PfcEquipTypeExemptKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("EQUIP", key._a);
    objectKey.setValue("STATE", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<PfcEquipTypeExempt, PfcEquipTypeExemptDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<PfcEquipTypeExemptDAO>;
  static DAOHelper<PfcEquipTypeExemptDAO> _helper;
  PfcEquipTypeExemptDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<PfcEquipTypeExemptKey, std::vector<PfcEquipTypeExempt*>, false>(cacheSize,
                                                                                       cacheType)
  {
  }
  virtual void load() override;
  std::vector<PfcEquipTypeExempt*>* create(PfcEquipTypeExemptKey key) override;
  void destroy(PfcEquipTypeExemptKey key, std::vector<PfcEquipTypeExempt*>* t) override;

private:
  struct isEffective;
  static PfcEquipTypeExemptDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: PfcEquipTypeExemptHistoricalDAO
// --------------------------------------------------
typedef HashKey<EquipmentType, StateCode, DateTime, DateTime> PfcEquipTypeExemptHistoricalKey;

class PfcEquipTypeExemptHistoricalDAO
    : public HistoricalDataAccessObject<PfcEquipTypeExemptHistoricalKey,
                                        std::vector<PfcEquipTypeExempt*>,
                                        false>
{
public:
  static PfcEquipTypeExemptHistoricalDAO& instance();
  const PfcEquipTypeExempt* get(DeleteList& del,
                                const EquipmentType& equip,
                                const StateCode& state,
                                const DateTime& date,
                                const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, PfcEquipTypeExemptHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("EQUIP", key._a) && objectKey.getValue("STATE", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    PfcEquipTypeExemptHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("EQUIP", key._a) && objectKey.getValue("STATE", key._b);
  }

  void setKeyDateRange(PfcEquipTypeExemptHistoricalKey& key, const DateTime date) const override
  {
    DAOUtils::getDateRange(date, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<PfcEquipTypeExemptHistoricalDAO>;
  static DAOHelper<PfcEquipTypeExemptHistoricalDAO> _helper;
  PfcEquipTypeExemptHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<PfcEquipTypeExemptHistoricalKey,
                                 std::vector<PfcEquipTypeExempt*>,
                                 false>(cacheSize, cacheType)
  {
  }
  std::vector<PfcEquipTypeExempt*>* create(PfcEquipTypeExemptHistoricalKey key) override;
  void destroy(PfcEquipTypeExemptHistoricalKey key, std::vector<PfcEquipTypeExempt*>* t) override;

private:
  struct isEffective;
  static PfcEquipTypeExemptHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
