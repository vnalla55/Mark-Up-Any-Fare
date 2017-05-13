//------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class BrandedCarrier;
class DeleteList;

class BrandedCarrierDAO : public DataAccessObject<IntKey, std::vector<BrandedCarrier*> >
{
public:
  static BrandedCarrierDAO& instance();

  const std::vector<BrandedCarrier*>& getAll(DeleteList& del);

  const std::vector<BrandedCarrier*>& get(DeleteList& del, const BFVersion& code);

  bool translateKey(const ObjectKey& objectKey, IntKey& key) const override
  {
    key = IntKey(0);
    return key.initialized;
  }

  IntKey createKey(BrandedCarrier* info);

  void translateKey(const IntKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("DUMMY", 0);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override;

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<BrandedCarrierDAO>;
  static DAOHelper<BrandedCarrierDAO> _helper;
  BrandedCarrierDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<IntKey, std::vector<BrandedCarrier*> >(cacheSize, cacheType, 2)
  {
  }

  virtual void load() override;
  std::vector<BrandedCarrier*>* create(IntKey key) override;

  void destroy(IntKey key, std::vector<BrandedCarrier*>* t) override;

private:
  static BrandedCarrierDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse

