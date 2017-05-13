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

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class BasicBookingRequest;
class DeleteList;

class BasicBookingRequestDAO
    : public DataAccessObject<CarrierKey, std::vector<BasicBookingRequest*>, false>
{
public:
  static BasicBookingRequestDAO& instance();

  const BasicBookingRequest* get(DeleteList& del, const CarrierCode& cxr, const DateTime& date);

  const std::vector<BasicBookingRequest*>& getAll(DeleteList& del, const DateTime& date);

  bool translateKey(const ObjectKey& objectKey, CarrierKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  CarrierKey createKey(BasicBookingRequest* info);

  void translateKey(const CarrierKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<BasicBookingRequest, BasicBookingRequestDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  std::vector<BasicBookingRequest*>* create(CarrierKey key) override;

  void destroy(CarrierKey key, std::vector<BasicBookingRequest*>* recs) override;

protected:
  static std::string _name;
  static std::string _cacheClass;

  std::vector<BasicBookingRequest*> loadList;
  virtual void load() override;

  friend class DAOHelper<BasicBookingRequestDAO>;

  static DAOHelper<BasicBookingRequestDAO> _helper;

  BasicBookingRequestDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CarrierKey, std::vector<BasicBookingRequest*>, false>(cacheSize, cacheType)
  {
  }

private:
  struct isEffective;
  static BasicBookingRequestDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
