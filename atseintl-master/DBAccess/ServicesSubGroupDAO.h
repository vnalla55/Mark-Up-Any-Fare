// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

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
class ServicesSubGroup;

typedef HashKey<ServiceGroup, ServiceGroup> ServicesSubGroupKey;

class ServicesSubGroupDAO
    : public DataAccessObject<ServicesSubGroupKey, std::vector<ServicesSubGroup*> >
{
public:
  static ServicesSubGroupDAO& instance();

  const ServicesSubGroup* get(DeleteList& del,
                              const ServiceGroup& serviceGroup,
                              const ServiceGroup& serviceSubGroup,
                              const DateTime& ticketDate);

  ServicesSubGroupKey createKey(ServicesSubGroup* info);

  bool translateKey(const ObjectKey& objectKey, ServicesSubGroupKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("SVCGROUP", key._a) && objectKey.getValue("SVCSUBGROUP", key._b);
  }

  void translateKey(const ServicesSubGroupKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("SVCGROUP", key._a);
    objectKey.setValue("SVCSUBGROUP", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<ServicesSubGroup, ServicesSubGroupDAO>(flatKey, objectKey).success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<ServicesSubGroupDAO>;

  static DAOHelper<ServicesSubGroupDAO> _helper;
  ServicesSubGroupDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<ServicesSubGroupKey, std::vector<ServicesSubGroup*> >(cacheSize, cacheType)
  {
  }
  void load() override;
  std::vector<ServicesSubGroup*>* create(ServicesSubGroupKey key) override;
  void destroy(ServicesSubGroupKey key, std::vector<ServicesSubGroup*>* t) override;

private:
  static ServicesSubGroupDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
}

