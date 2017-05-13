//------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
//
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
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
class BrandedFareApp;
class DeleteList;

typedef HashKey<Indicator, UserApplCode, CarrierCode> BrandedFareAppKey;

class BrandedFareAppDAO
    : public DataAccessObject<BrandedFareAppKey, std::vector<BrandedFareApp*>, false>
{
public:
  static BrandedFareAppDAO& instance();

  const std::vector<BrandedFareApp*>& get(DeleteList& del,
                                          const Indicator& userApplType,
                                          const UserApplCode& userAppl,
                                          const CarrierCode& carrier,
                                          const DateTime& travelDate,
                                          const DateTime& ticketDate,
                                          bool isHistorical);

  bool translateKey(const ObjectKey& objectKey, BrandedFareAppKey& key) const override
  {
    return key.initialized = objectKey.getValue("USERAPPLTYPE", key._a) &&
                             objectKey.getValue("USERAPPL", key._b) &&
                             objectKey.getValue("CARRIER", key._c);
  }

  BrandedFareAppKey createKey(const BrandedFareApp* info);

  void translateKey(const BrandedFareAppKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("USERAPPLTYPE", key._a);
    objectKey.setValue("USERAPPL", key._b);
    objectKey.setValue("CARRIER", key._c);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<BrandedFareApp, BrandedFareAppDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<BrandedFareAppDAO>;

  static DAOHelper<BrandedFareAppDAO> _helper;

  BrandedFareAppDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<BrandedFareAppKey, std::vector<BrandedFareApp*>, false>(cacheSize, cacheType)
  {
  }

  std::vector<BrandedFareApp*>* create(BrandedFareAppKey key) override;
  void destroy(BrandedFareAppKey key, std::vector<BrandedFareApp*>* recs) override;
  void load() override;

private:
  struct isEffective;
  static BrandedFareAppDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse

