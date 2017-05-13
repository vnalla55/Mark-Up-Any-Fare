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
class VoluntaryChangesConfig;
class DeleteList;

class VoluntaryChangesConfigDAO
    : public DataAccessObject<CarrierKey, std::vector<VoluntaryChangesConfig*>, false>
{
public:
  static VoluntaryChangesConfigDAO& instance();
  const DateTime& get(DeleteList& del,
                      const CarrierCode& key,
                      const DateTime& currentDate,
                      const DateTime& originalTktIssueDate);

  bool translateKey(const ObjectKey& objectKey, CarrierKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<VoluntaryChangesConfigDAO>;
  static DAOHelper<VoluntaryChangesConfigDAO> _helper;
  VoluntaryChangesConfigDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CarrierKey, std::vector<VoluntaryChangesConfig*>, false>(cacheSize,
                                                                                cacheType)
  {
  }
  //  virtual void load();
  std::vector<VoluntaryChangesConfig*>* create(CarrierKey carrier) override;
  void destroy(CarrierKey key, std::vector<VoluntaryChangesConfig*>* t) override;

private:
  //  struct groupByKey;
  static VoluntaryChangesConfigDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse
