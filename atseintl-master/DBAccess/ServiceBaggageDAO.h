//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/HistoricalDataAccessObject.h"

#include <log4cxx/helpers/objectptr.h>

#include <vector>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class ServiceBaggageInfo;
class DeleteList;

typedef HashKey<VendorCode, int> ServiceBaggageKey;

class ServiceBaggageDAO
    : public DataAccessObject<ServiceBaggageKey, std::vector<const ServiceBaggageInfo*> >
{
  friend class DAOHelper<ServiceBaggageDAO>;

public:
  static ServiceBaggageDAO& instance();

  const std::vector<const ServiceBaggageInfo*>
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, ServiceBaggageKey& key) const override;

  void translateKey(const ServiceBaggageKey& key, ObjectKey& objectKey) const override;

  const ServiceBaggageKey createKey(const ServiceBaggageInfo* info);

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override;

  log4cxx::LoggerPtr& getLogger() override { return _logger; }
  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static const std::string _name;
  static const std::string _cacheClass;
  static DAOHelper<ServiceBaggageDAO> _helper;

  ServiceBaggageDAO(int cacheSize = 0, const std::string& cacheType = "");

  void load() override;
  std::vector<const ServiceBaggageInfo*>* create(const ServiceBaggageKey key) override;
  void destroy(const ServiceBaggageKey key, std::vector<const ServiceBaggageInfo*>* t) override;

private:
  static ServiceBaggageDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

typedef HashKey<VendorCode, int, DateTime, DateTime> ServiceBaggageHistoricalKey;

class ServiceBaggageHistoricalDAO
    : public HistoricalDataAccessObject<ServiceBaggageHistoricalKey,
                                        std::vector<const ServiceBaggageInfo*> >
{
  friend class DAOHelper<ServiceBaggageHistoricalDAO>;

public:
  static ServiceBaggageHistoricalDAO& instance();

  const std::vector<const ServiceBaggageInfo*>
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, ServiceBaggageHistoricalKey& key) const override;

  bool translateKey(const ObjectKey& objectKey,
                    ServiceBaggageHistoricalKey& key,
                    const DateTime ticketDate) const override;

  void setKeyDateRange(ServiceBaggageHistoricalKey& key, const DateTime ticketDate) const override;

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  ServiceBaggageHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "");

  std::vector<const ServiceBaggageInfo*>* create(const ServiceBaggageHistoricalKey key) override;

  void destroy(const ServiceBaggageHistoricalKey key,
               std::vector<const ServiceBaggageInfo*>* t) override;

  static const std::string _name;
  static const std::string _cacheClass;
  static DAOHelper<ServiceBaggageHistoricalDAO> _helper;

private:
  static ServiceBaggageHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse

