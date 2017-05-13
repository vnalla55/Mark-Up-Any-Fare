//----------------------------------------------------------------------------
//          File:           InterlineTicketCarrierDAO.h
//          Description:    InterlineTicketCarrierDAO
//          Created:        10/1/2010
//          Authors:        Anna Kulig
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
class InterlineTicketCarrierInfo;
class DeleteList;

class InterlineTicketCarrierDAO
    : public DataAccessObject<CarrierKey, std::vector<InterlineTicketCarrierInfo*>, false>
{
public:
  static InterlineTicketCarrierDAO& instance();
  const std::vector<InterlineTicketCarrierInfo*>&
  get(DeleteList& del, const CarrierCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CarrierKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  CarrierKey createKey(InterlineTicketCarrierInfo* info);

  void translateKey(const CarrierKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<InterlineTicketCarrierInfo, InterlineTicketCarrierDAO>(
               flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<InterlineTicketCarrierDAO>;
  static DAOHelper<InterlineTicketCarrierDAO> _helper;
  InterlineTicketCarrierDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CarrierKey, std::vector<InterlineTicketCarrierInfo*>, false>(
          cacheSize, cacheType, 2)
  {
  }
  virtual void load() override;
  std::vector<InterlineTicketCarrierInfo*>* create(CarrierKey key) override;
  void destroy(CarrierKey key, std::vector<InterlineTicketCarrierInfo*>* recs) override;

private:
  struct isEffective;
  static InterlineTicketCarrierDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: InterlineTicketCarrierHistoricalDAO
// --------------------------------------------------
class InterlineTicketCarrierHistoricalDAO
    : public HistoricalDataAccessObject<CarrierCode,
                                        std::vector<InterlineTicketCarrierInfo*>,
                                        false>
{
public:
  static InterlineTicketCarrierHistoricalDAO& instance();
  const std::vector<InterlineTicketCarrierInfo*>&
  get(DeleteList& del, const CarrierCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CarrierCode& key) const override
  {
    return objectKey.getValue("CARRIER", key);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<InterlineTicketCarrierHistoricalDAO>;
  static DAOHelper<InterlineTicketCarrierHistoricalDAO> _helper;
  InterlineTicketCarrierHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<CarrierCode, std::vector<InterlineTicketCarrierInfo*>, false>(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<InterlineTicketCarrierInfo*>* create(CarrierCode key) override;
  void destroy(CarrierCode key, std::vector<InterlineTicketCarrierInfo*>* recs) override;

  virtual void load() override;

private:
  struct isEffective;
  struct groupByKey;
  static InterlineTicketCarrierHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
