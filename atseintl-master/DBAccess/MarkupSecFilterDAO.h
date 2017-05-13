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
class MarkupSecFilter;
class DeleteList;

typedef HashKey<VendorCode, CarrierCode, TariffNumber, RuleNumber> MarkupSecFilterKey;

class MarkupSecFilterDAO
    : public DataAccessObject<MarkupSecFilterKey, std::vector<MarkupSecFilter*> >
{
public:
  static MarkupSecFilterDAO& instance();
  const std::vector<MarkupSecFilter*>& get(DeleteList& del,
                                           const VendorCode& vendor,
                                           const CarrierCode& carrier,
                                           const TariffNumber& ruleTariff,
                                           const RuleNumber& rule,
                                           const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, MarkupSecFilterKey& key) const override
  {
    return key.initialized = objectKey.getValue("VENDOR", key._a) &&
                             objectKey.getValue("CARRIER", key._b) &&
                             objectKey.getValue("TARIFFNUMBER", key._c) &&
                             objectKey.getValue("RULENUMBER", key._d);
  }

  MarkupSecFilterKey createKey(MarkupSecFilter* info);

  void translateKey(const MarkupSecFilterKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("TARIFFNUMBER", key._c);
    objectKey.setValue("RULENUMBER", key._d);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<MarkupSecFilter, MarkupSecFilterDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<MarkupSecFilter*>* vect) const override;

  virtual std::vector<MarkupSecFilter*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MarkupSecFilterDAO>;
  static DAOHelper<MarkupSecFilterDAO> _helper;
  MarkupSecFilterDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<MarkupSecFilterKey, std::vector<MarkupSecFilter*> >(cacheSize, cacheType, 2)
  {
  }
  void load() override;
  std::vector<MarkupSecFilter*>* create(MarkupSecFilterKey key) override;
  void destroy(MarkupSecFilterKey key, std::vector<MarkupSecFilter*>* t) override;

private:
  static MarkupSecFilterDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: MarkupSecFilterHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, CarrierCode, TariffNumber, RuleNumber, DateTime, DateTime>
MarkupSecFilterHistoricalKey;

class MarkupSecFilterHistoricalDAO
    : public HistoricalDataAccessObject<MarkupSecFilterHistoricalKey,
                                        std::vector<MarkupSecFilter*> >
{
public:
  static MarkupSecFilterHistoricalDAO& instance();
  const std::vector<MarkupSecFilter*>& get(DeleteList& del,
                                           const VendorCode& vendor,
                                           const CarrierCode& carrier,
                                           const TariffNumber& ruleTariff,
                                           const RuleNumber& rule,
                                           const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, MarkupSecFilterHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
               objectKey.getValue("TARIFFNUMBER", key._c) &&
               objectKey.getValue("RULENUMBER", key._d) &&
               objectKey.getValue("STARTDATE", key._e) && objectKey.getValue("ENDDATE", key._f);
  }

  bool translateKey(const ObjectKey& objectKey,
                    MarkupSecFilterHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
    return key.initialized = objectKey.getValue("VENDOR", key._a) &&
                             objectKey.getValue("CARRIER", key._b) &&
                             objectKey.getValue("TARIFFNUMBER", key._c) &&
                             objectKey.getValue("RULENUMBER", key._d);
  }

  void setKeyDateRange(MarkupSecFilterHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<MarkupSecFilter*>* vect) const override;

  virtual std::vector<MarkupSecFilter*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MarkupSecFilterHistoricalDAO>;
  static DAOHelper<MarkupSecFilterHistoricalDAO> _helper;
  MarkupSecFilterHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<MarkupSecFilterHistoricalKey, std::vector<MarkupSecFilter*> >(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<MarkupSecFilter*>* create(MarkupSecFilterHistoricalKey key) override;
  void destroy(MarkupSecFilterHistoricalKey key, std::vector<MarkupSecFilter*>* t) override;

private:
  static MarkupSecFilterHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
