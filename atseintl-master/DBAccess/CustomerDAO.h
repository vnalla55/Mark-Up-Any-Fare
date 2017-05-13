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
class Customer;
class DeleteList;
typedef HashKey<PseudoCityCode> CustomerKey;

class CustomerDAO : public DataAccessObject<CustomerKey, std::vector<Customer*> >
{
public:
  static CustomerDAO& instance();
  const std::vector<Customer*>&
  get(DeleteList& del, const PseudoCityCode& key, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CustomerKey& key) const override
  {
    return key.initialized = objectKey.getValue("PSEUDOCITY", key._a);
  }

  CustomerKey createKey(const Customer* info);

  void translateKey(const CustomerKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("PSEUDOCITY", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<Customer, CustomerDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<CustomerDAO>;
  static DAOHelper<CustomerDAO> _helper;
  CustomerDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CustomerKey, std::vector<Customer*> >(cacheSize, cacheType, 8)
  {
  }
  std::vector<Customer*>* create(CustomerKey key) override;
  void destroy(CustomerKey key, std::vector<Customer*>* t) override;
  void load() override;

  virtual sfc::CompressedData*
    compress(const std::vector<Customer*>* vect) const override;

  virtual std::vector<Customer*>*
    uncompress(const sfc::CompressedData& compressed) const override;

private:
  static CustomerDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: CustomerHistoricalDAO
// --------------------------------------------------
class CustomerHistoricalDAO
    : public HistoricalDataAccessObject<PseudoCityCode, std::vector<Customer*> >
{
public:
  static CustomerHistoricalDAO& instance();
  const std::vector<Customer*>&
  get(DeleteList& del, const PseudoCityCode& key, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, PseudoCityCode& key) const override
  {
    return objectKey.getValue("PSEUDOCITY", key);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData*
    compress(const std::vector<Customer*>* vect) const override;

  virtual std::vector<Customer*>*
    uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<CustomerHistoricalDAO>;
  static DAOHelper<CustomerHistoricalDAO> _helper;
  CustomerHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<PseudoCityCode, std::vector<Customer*> >(cacheSize, cacheType, 8)
  {
  }
  std::vector<Customer*>* create(PseudoCityCode key) override;
  void destroy(PseudoCityCode key, std::vector<Customer*>* t) override;

private:
  static CustomerHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
