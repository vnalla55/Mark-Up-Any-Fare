//----------------------------------------------------------------------------
//          File:           InterlineTicketCarrierStatusDAO.h
//          Description:    InterlineTicketCarrierStatus
//          Created:        02/27/2012
//          Authors:        M Dantas
//
//          Updates:
//
//     (c)2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
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
class InterlineTicketCarrierStatus;
class DeleteList;

typedef HashKey<CarrierCode, CrsCode> IETKey;

class InterlineTicketCarrierStatusDAO
    : public DataAccessObject<IETKey, std::vector<InterlineTicketCarrierStatus*>, false>
{
public:
  static InterlineTicketCarrierStatusDAO& instance();

  const std::vector<InterlineTicketCarrierStatus*>& get(DeleteList& del,
                                                        const CarrierCode& carrier,
                                                        const CrsCode& crsCode,
                                                        const DateTime& date,
                                                        const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, IETKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("CARRIER", key._a) && objectKey.getValue("CRSCODE", key._b);
  }

  IETKey createKey(InterlineTicketCarrierStatus* info);

  void translateKey(const IETKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
    objectKey.setValue("CRSCODE", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<InterlineTicketCarrierStatus, InterlineTicketCarrierStatusDAO>(
               flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<InterlineTicketCarrierStatusDAO>;
  static DAOHelper<InterlineTicketCarrierStatusDAO> _helper;
  InterlineTicketCarrierStatusDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<IETKey, std::vector<InterlineTicketCarrierStatus*>, false>(cacheSize,
                                                                                  cacheType)
  {
  }
  virtual void load() override;
  std::vector<InterlineTicketCarrierStatus*>* create(IETKey key) override;
  void destroy(IETKey key, std::vector<InterlineTicketCarrierStatus*>* recs) override;

private:
  static InterlineTicketCarrierStatusDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class InterlineTicketCarrierStatusDAO

// --------------------------------------------------
// Historical DAO: InterlineTicketCarrierStatusHistoricalDAO
// --------------------------------------------------
class InterlineTicketCarrierStatusHistoricalDAO
    : public HistoricalDataAccessObject<IETKey, std::vector<InterlineTicketCarrierStatus*>, false>
{
public:
  static InterlineTicketCarrierStatusHistoricalDAO& instance();
  const std::vector<InterlineTicketCarrierStatus*>& get(DeleteList& del,
                                                        const CarrierCode& carrier,
                                                        const CrsCode& crsCode,
                                                        const DateTime& date,
                                                        const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, IETKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("CARRIER", key._a) && objectKey.getValue("CRSCODE", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<InterlineTicketCarrierStatusHistoricalDAO>;
  static DAOHelper<InterlineTicketCarrierStatusHistoricalDAO> _helper;
  InterlineTicketCarrierStatusHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<IETKey, std::vector<InterlineTicketCarrierStatus*>, false>(
          cacheSize, cacheType)
  {
  }
  virtual void load() override;
  std::vector<InterlineTicketCarrierStatus*>* create(IETKey key) override;
  void destroy(IETKey key, std::vector<InterlineTicketCarrierStatus*>* recs) override;

private:
  struct groupByKey;
  static InterlineTicketCarrierStatusHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class InterlineTicketCarrierStatusHistoricalDAO
} // namespace tse
