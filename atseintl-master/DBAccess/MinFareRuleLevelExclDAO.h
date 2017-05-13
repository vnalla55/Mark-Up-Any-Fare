//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
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
class MinFareRuleLevelExcl;
class DeleteList;

typedef HashKey<VendorCode, int, CarrierCode, TariffNumber> MinFareRuleLevelExclKey;

class MinFareRuleLevelExclDAO
    : public DataAccessObject<MinFareRuleLevelExclKey, std::vector<MinFareRuleLevelExcl*> >
{
public:
  static MinFareRuleLevelExclDAO& instance();

  const std::vector<MinFareRuleLevelExcl*>& get(DeleteList& del,
                                                const VendorCode& vendor,
                                                int textTblItemNo,
                                                const CarrierCode& governingCarrier,
                                                const TariffNumber& ruleTariff,
                                                const DateTime& date,
                                                const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, MinFareRuleLevelExclKey& key) const override
  {
    return key.initialized = objectKey.getValue("VENDOR", key._a) &&
                             objectKey.getValue("TEXTTBLITEMNO", key._b) &&
                             objectKey.getValue("CARRIER", key._c) &&
                             objectKey.getValue("TARIFFNUMBER", key._d);
  }

  virtual size_t invalidate(const ObjectKey& objectKey) override;

  MinFareRuleLevelExclKey createKey(MinFareRuleLevelExcl* info);

  void translateKey(const MinFareRuleLevelExclKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("TEXTTBLITEMNO", key._b);
    objectKey.setValue("CARRIER", key._c);
    objectKey.setValue("TARIFFNUMBER", key._d);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<MinFareRuleLevelExcl, MinFareRuleLevelExclDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<MinFareRuleLevelExcl*>* vect) const override;

  virtual std::vector<MinFareRuleLevelExcl*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MinFareRuleLevelExclDAO>;
  static DAOHelper<MinFareRuleLevelExclDAO> _helper;
  MinFareRuleLevelExclDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<MinFareRuleLevelExclKey, std::vector<MinFareRuleLevelExcl*> >(
          cacheSize, cacheType, 3)
  {
  }
  void load() override;
  std::vector<MinFareRuleLevelExcl*>* create(MinFareRuleLevelExclKey key) override;
  void destroy(MinFareRuleLevelExclKey key, std::vector<MinFareRuleLevelExcl*>* t) override;

private:
  static MinFareRuleLevelExclDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical Stuff
// /////////////////////////////////////////////////////////////////////////////////
typedef HashKey<VendorCode, int, CarrierCode, TariffNumber, DateTime, DateTime>
MinFareRuleLevelExclHistoricalKey;

class MinFareRuleLevelExclHistoricalDAO
    : public HistoricalDataAccessObject<MinFareRuleLevelExclHistoricalKey,
                                        std::vector<MinFareRuleLevelExcl*> >
{
public:
  static MinFareRuleLevelExclHistoricalDAO& instance();

  const std::vector<MinFareRuleLevelExcl*>& get(DeleteList& del,
                                                const VendorCode& vendor,
                                                int textTblItemNo,
                                                const CarrierCode& governingCarrier,
                                                const TariffNumber& ruleTariff,
                                                const DateTime& date,
                                                const DateTime& ticketDate);

  bool
  translateKey(const ObjectKey& objectKey, MinFareRuleLevelExclHistoricalKey& key) const override
  {
    return key.initialized = objectKey.getValue("VENDOR", key._a) &&
                             objectKey.getValue("TEXTTBLITEMNO", key._b) &&
                             objectKey.getValue("CARRIER", key._c) &&
                             objectKey.getValue("TARIFFNUMBER", key._d) &&
                             objectKey.getValue("STARTDATE", key._e) &&
                             objectKey.getValue("ENDDATE", key._f);
  }

  bool translateKey(const ObjectKey& objectKey,
                    MinFareRuleLevelExclHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
    return key.initialized = objectKey.getValue("VENDOR", key._a) &&
                             objectKey.getValue("TEXTTBLITEMNO", key._b) &&
                             objectKey.getValue("CARRIER", key._c) &&
                             objectKey.getValue("TARIFFNUMBER", key._d);
  }

  void
  setKeyDateRange(MinFareRuleLevelExclHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
  compress(const std::vector<MinFareRuleLevelExcl*>* vect) const override;

  virtual std::vector<MinFareRuleLevelExcl*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MinFareRuleLevelExclHistoricalDAO>;
  static DAOHelper<MinFareRuleLevelExclHistoricalDAO> _helper;
  MinFareRuleLevelExclHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<MinFareRuleLevelExclHistoricalKey,
                                 std::vector<MinFareRuleLevelExcl*> >(cacheSize, cacheType, 3)
  {
  }
  std::vector<MinFareRuleLevelExcl*>* create(MinFareRuleLevelExclHistoricalKey key) override;
  void
  destroy(MinFareRuleLevelExclHistoricalKey key, std::vector<MinFareRuleLevelExcl*>* t) override;

private:
  static MinFareRuleLevelExclHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
