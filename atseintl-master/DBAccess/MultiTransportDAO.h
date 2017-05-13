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

class MultiTransportDAO : public DataAccessObject<LocCodeKey, std::vector<MultiTransport*>, false>
{
public:
  static MultiTransportDAO& instance();
  const std::vector<MultiTransport*>&
  get(DeleteList& del, const LocCode& key, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, LocCodeKey& key) const override
  {
    return key.initialized = objectKey.getValue("MULTITRANSCITY", key._a);
  }

  LocCodeKey createKey(MultiTransport* info);

  void translateKey(const LocCodeKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("MULTITRANSCITY", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<MultiTransport, MultiTransportDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<MultiTransport*>* vect) const override;

  virtual std::vector<MultiTransport*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MultiTransportDAO>;
  static DAOHelper<MultiTransportDAO> _helper;
  MultiTransportDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<LocCodeKey, std::vector<MultiTransport*>, false>(cacheSize, cacheType)
  {
  }

  std::vector<MultiTransport*>* create(LocCodeKey key) override;
  void destroy(LocCodeKey key, std::vector<MultiTransport*>* recs) override;

  virtual void load() override;

private:
  static MultiTransportDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: MultiTransportHistoricalDAO
// --------------------------------------------------
typedef HashKey<LocCode, DateTime, DateTime> MultiTransportHistoricalKey;

class MultiTransportHistoricalDAO : public HistoricalDataAccessObject<MultiTransportHistoricalKey,
                                                                      std::vector<MultiTransport*>,
                                                                      false>
{
public:
  static MultiTransportHistoricalDAO& instance();
  std::vector<MultiTransport*>&
  get(DeleteList& del, const LocCode& key, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, MultiTransportHistoricalKey& key) const override
  {
    return key.initialized = objectKey.getValue("MULTITRANSCITY", key._a) &&
                             objectKey.getValue("STARTDATE", key._b) &&
                             objectKey.getValue("ENDDATE", key._c);
  }

  bool translateKey(const ObjectKey& objectKey,
                    MultiTransportHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return objectKey.getValue("MULTITRANSCITY", key._a);
  }

  void setKeyDateRange(MultiTransportHistoricalKey& key, const DateTime date) const override
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
  friend class DAOHelper<MultiTransportHistoricalDAO>;
  static DAOHelper<MultiTransportHistoricalDAO> _helper;
  MultiTransportHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<MultiTransportHistoricalKey, std::vector<MultiTransport*>, false>(
          cacheSize, cacheType)
  {
  }

  std::vector<MultiTransport*>* create(MultiTransportHistoricalKey key) override;
  void destroy(MultiTransportHistoricalKey key, std::vector<MultiTransport*>* recs) override;

private:
  static MultiTransportHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
