//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
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

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class CustomerActivationControl;

typedef HashKey<std::string> CustomerActivationControlKey;
typedef HashKey<std::string, DateTime, DateTime> CustomerActivationControlHistoricalKey;

class CustomerActivationControlDAO
    : public DataAccessObject<CustomerActivationControlKey,
                              std::vector<CustomerActivationControl*> >
{
public:
  friend class DAOHelper<CustomerActivationControlDAO>;

  static CustomerActivationControlDAO& instance();

  const std::vector<tse::CustomerActivationControl*>& get(DeleteList& del,
                                                          const std::string& projCode,
                                                          const DateTime& date,
                                                          const DateTime& ticketDate);

  CustomerActivationControlKey createKey(CustomerActivationControl* info);

  bool translateKey(const ObjectKey& objectKey, CustomerActivationControlKey& key) const override
  {
    return key.initialized = objectKey.getValue("PROJCODE", key._a);
  }

  void translateKey(const CustomerActivationControlKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("PROJCODE", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<CustomerActivationControl, CustomerActivationControlDAO>(
               flatKey, objectKey).success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

private:
  CustomerActivationControlDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CustomerActivationControlKey, std::vector<CustomerActivationControl*> >(
          cacheSize, cacheType)
  {
  }

  std::vector<CustomerActivationControl*>* create(CustomerActivationControlKey key) override;
  void
  destroy(CustomerActivationControlKey key, std::vector<CustomerActivationControl*>* recs) override;
  void load() override;

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<CustomerActivationControlDAO> _helper;
  static CustomerActivationControlDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

class CustomerActivationControlHistoricalDAO
    : public HistoricalDataAccessObject<CustomerActivationControlHistoricalKey,
                                        std::vector<CustomerActivationControl*> >
{
public:
  friend class DAOHelper<CustomerActivationControlHistoricalDAO>;

  static CustomerActivationControlHistoricalDAO& instance();

  const std::vector<tse::CustomerActivationControl*>& get(DeleteList& del,
                                                          const std::string& carrier,
                                                          const DateTime& date,
                                                          const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CustomerActivationControlHistoricalKey& key) const
      override
  {
    key.initialized = objectKey.getValue("PROJCODE", key._a);
    if (key.initialized && (_cacheBy != DAOUtils::NODATES))
    {
      key.initialized =
          objectKey.getValue("STARTDATE", key._b) && objectKey.getValue("ENDDATE", key._c);
    }
    return key.initialized;
  }

  bool translateKey(const ObjectKey& objectKey,
                    CustomerActivationControlHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);

    return key.initialized = objectKey.getValue("PROJCODE", key._a);
  }

  void setKeyDateRange(CustomerActivationControlHistoricalKey& key, const DateTime ticketDate) const
      override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

private:
  CustomerActivationControlHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<CustomerActivationControlHistoricalKey,
                                 std::vector<CustomerActivationControl*> >(cacheSize, cacheType)
  {
  }
  std::vector<CustomerActivationControl*>*
  create(CustomerActivationControlHistoricalKey key) override;
  void destroy(CustomerActivationControlHistoricalKey key,
               std::vector<CustomerActivationControl*>* t) override;

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<CustomerActivationControlHistoricalDAO> _helper;
  static CustomerActivationControlHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // tse

