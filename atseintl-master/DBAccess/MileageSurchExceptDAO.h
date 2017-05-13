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
class MileageSurchExcept;
class DeleteList;

typedef HashKey<VendorCode, int, CarrierCode, TariffNumber, RuleNumber> MileageSurchExceptKey;

class MileageSurchExceptDAO
    : public DataAccessObject<MileageSurchExceptKey, std::vector<MileageSurchExcept*>, false>
{
public:
  static MileageSurchExceptDAO& instance();
  const std::vector<MileageSurchExcept*>& get(DeleteList& del,
                                              const VendorCode& vendor,
                                              int textTblItemNo,
                                              const CarrierCode& governingCarrier,
                                              const TariffNumber& ruleTariff,
                                              const RuleNumber& rule,
                                              const DateTime& date,
                                              const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, MileageSurchExceptKey& key) const override
  {
    return key.initialized = objectKey.getValue("VENDOR", key._a) &&
                             objectKey.getValue("TEXTTBLITEMNO", key._b) &&
                             objectKey.getValue("CARRIER", key._c) &&
                             objectKey.getValue("TARIFFNUMBER", key._d) &&
                             objectKey.getValue("RULENUMBER", key._e);
  }

  MileageSurchExceptKey createKey(MileageSurchExcept* info);

  void translateKey(const MileageSurchExceptKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("TEXTTBLITEMNO", key._b);
    objectKey.setValue("CARRIER", key._c);
    objectKey.setValue("TARIFFNUMBER", key._d);
    objectKey.setValue("RULENUMBER", key._e);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<MileageSurchExcept, MileageSurchExceptDAO>(flatKey, objectKey)
        .success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData*
  compress(const std::vector<MileageSurchExcept*>* vect) const override;

  virtual std::vector<MileageSurchExcept*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MileageSurchExceptDAO>;
  static DAOHelper<MileageSurchExceptDAO> _helper;
  MileageSurchExceptDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<MileageSurchExceptKey, std::vector<MileageSurchExcept*>, false>(
          cacheSize, cacheType, 2)
  {
  }
  virtual void load() override;
  std::vector<MileageSurchExcept*>* create(MileageSurchExceptKey key) override;
  void destroy(MileageSurchExceptKey key, std::vector<MileageSurchExcept*>* t) override;

private:
  struct isEffective;
  static MileageSurchExceptDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: MileageSurchExceptHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, CarrierCode, TariffNumber, RuleNumber, DateTime, DateTime>
MileageSurchExceptHistoricalKey;

class MileageSurchExceptHistoricalDAO
    : public HistoricalDataAccessObject<MileageSurchExceptHistoricalKey,
                                        std::vector<MileageSurchExcept*>,
                                        false>
{
public:
  static MileageSurchExceptHistoricalDAO& instance();
  const std::vector<MileageSurchExcept*>& get(DeleteList& del,
                                              const VendorCode& vendor,
                                              int textTblItemNo,
                                              const CarrierCode& governingCarrier,
                                              const TariffNumber& ruleTariff,
                                              const RuleNumber& rule,
                                              const DateTime& date,
                                              const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, MileageSurchExceptHistoricalKey& key) const override
  {
    return key.initialized = objectKey.getValue("VENDOR", key._a) &&
                             objectKey.getValue("TEXTTBLITEMNO", key._b) &&
                             objectKey.getValue("CARRIER", key._c) &&
                             objectKey.getValue("TARIFFNUMBER", key._d) &&
                             objectKey.getValue("RULENUMBER", key._e) &&
                             objectKey.getValue("STARTDATE", key._f) &&
                             objectKey.getValue("ENDDATE", key._g);
  }

  bool translateKey(const ObjectKey& objectKey,
                    MileageSurchExceptHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._f, key._g, _cacheBy);
    return key.initialized = objectKey.getValue("VENDOR", key._a) &&
                             objectKey.getValue("TEXTTBLITEMNO", key._b) &&
                             objectKey.getValue("CARRIER", key._c) &&
                             objectKey.getValue("TARIFFNUMBER", key._d) &&
                             objectKey.getValue("RULENUMBER", key._e);
  }

  void
  setKeyDateRange(MileageSurchExceptHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._f, key._g, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  MileageSurchExceptHistoricalKey createKey(const MileageSurchExcept*,
                                            const DateTime& startDate = DateTime::emptyDate(),
                                            const DateTime& endDate = DateTime::emptyDate());

  void translateKey(const MileageSurchExceptHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("TEXTTBLITEMNO", key._b);
    objectKey.setValue("CARRIER", key._c);
    objectKey.setValue("TARIFFNUMBER", key._d);
    objectKey.setValue("RULENUMBER", key._e);
    objectKey.setValue("STARTDATE", key._f);
    objectKey.setValue("ENDDATE", key._g);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserterWithDateRange<MileageSurchExcept, MileageSurchExceptHistoricalDAO>(
               flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData*
  compress(const std::vector<MileageSurchExcept*>* vect) const override;

  virtual std::vector<MileageSurchExcept*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MileageSurchExceptHistoricalDAO>;
  static DAOHelper<MileageSurchExceptHistoricalDAO> _helper;
  MileageSurchExceptHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<MileageSurchExceptHistoricalKey,
                                 std::vector<MileageSurchExcept*>,
                                 false>(cacheSize, cacheType, 2)
  {
  }

  std::vector<MileageSurchExcept*>* create(MileageSurchExceptHistoricalKey key) override;
  void destroy(MileageSurchExceptHistoricalKey key, std::vector<MileageSurchExcept*>* t) override;
  void load() override;

private:
  struct isEffective;
  static MileageSurchExceptHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
