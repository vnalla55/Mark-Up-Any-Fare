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
class MultiTransport;
class DeleteList;

class MultiTransportCityDAO
    : public DataAccessObject<LocCodeKey, std::vector<MultiTransport*>, false>
{
public:
  static MultiTransportCityDAO& instance();

  const LocCode get(DeleteList& del, const LocCode& locCode, const DateTime& ticketDate);

  const std::vector<MultiTransport*>& get(DeleteList& del,
                                          const LocCode& locCode,
                                          const CarrierCode& carrierCode,
                                          GeoTravelType tvlType,
                                          const DateTime& tvlDate,
                                          const DateTime& ticketDate);

  const LocCode getLocCode(const LocCode& locCode,
                           const CarrierCode& carrierCode,
                           GeoTravelType tvlType,
                           const DateTime& tvlDate);

  bool translateKey(const ObjectKey& objectKey, LocCodeKey& key) const override
  {
    return key.initialized = objectKey.getValue("MULTITRANSLOC", key._a);
  }

  LocCodeKey createKey(MultiTransport* info);

  void translateKey(const LocCodeKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("MULTITRANSLOC", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<MultiTransport, MultiTransportCityDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual size_t invalidate(const ObjectKey& objectKey) override;

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<MultiTransport*>* vect) const override;

  virtual std::vector<MultiTransport*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MultiTransportCityDAO>;
  static DAOHelper<MultiTransportCityDAO> _helper;
  MultiTransportCityDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<LocCodeKey, std::vector<MultiTransport*>, false>(cacheSize, cacheType)
  {
  }
  std::vector<MultiTransport*>* create(LocCodeKey key) override;
  void destroy(LocCodeKey key, std::vector<MultiTransport*>* recs) override;

  virtual void load() override;

private:
  static MultiTransportCityDAO* _instance;
  struct matchKeys;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: MultiTransportCityHistoricalDAO
// --------------------------------------------------
typedef HashKey<LocCode, DateTime, DateTime> MultiTransportCityHistoricalKey;

class MultiTransportCityHistoricalDAO
    : public HistoricalDataAccessObject<MultiTransportCityHistoricalKey,
                                        std::vector<MultiTransport*>,
                                        false>
{
public:
  static MultiTransportCityHistoricalDAO& instance();

  const LocCode get(DeleteList& del, const LocCode& locCode, const DateTime& ticketDate);

  const std::vector<MultiTransport*>& get(DeleteList& del,
                                          const LocCode& locCode,
                                          const CarrierCode& carrierCode,
                                          GeoTravelType tvlType,
                                          const DateTime& tvlDate,
                                          const DateTime& ticketDate);

  const LocCode getLocCode(const LocCode& locCode,
                           const CarrierCode& carrierCode,
                           GeoTravelType tvlType,
                           const DateTime& tktDate,
                           const DateTime& tvlDate);

  bool translateKey(const ObjectKey& objectKey, MultiTransportCityHistoricalKey& key) const override
  {
    return key.initialized = objectKey.getValue("MULTITRANSLOC", key._a) &&
                             objectKey.getValue("STARTDATE", key._b) &&
                             objectKey.getValue("ENDDATE", key._c);
  }

  bool translateKey(const ObjectKey& objectKey,
                    MultiTransportCityHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("MULTITRANSLOC", key._a);
  }

  void setKeyDateRange(MultiTransportCityHistoricalKey& key, const DateTime date) const override
  {
    DAOUtils::getDateRange(date, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<MultiTransport*>* vect) const override;

  virtual std::vector<MultiTransport*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MultiTransportCityHistoricalDAO>;
  static DAOHelper<MultiTransportCityHistoricalDAO> _helper;
  MultiTransportCityHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<MultiTransportCityHistoricalKey,
                                 std::vector<MultiTransport*>,
                                 false>(cacheSize, cacheType)
  {
  }
  std::vector<MultiTransport*>* create(MultiTransportCityHistoricalKey key) override;
  void destroy(MultiTransportCityHistoricalKey key, std::vector<MultiTransport*>* recs) override;

private:
  static MultiTransportCityHistoricalDAO* _instance;
  struct matchKeysH;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
