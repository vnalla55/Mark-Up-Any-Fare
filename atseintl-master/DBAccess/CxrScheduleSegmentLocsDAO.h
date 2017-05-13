//----------------------------------------------------------------------------
//          File:           CxrCxrScheduleSegmentLocssDAO.h
//          Description:    CxrCxrScheduleSegmentLocssDAO
//          Created:        11/01/2010
//          Authors:
//
//          Updates:
//
//     � 2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
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

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class DeleteList;
class CxrScheduleSegmentLocs;

typedef HashKey<CarrierCode> CxrScheduleSegmentLocsKey;

class CxrScheduleSegmentLocsDAO
    : public DataAccessObject<CxrScheduleSegmentLocsKey, CxrScheduleSegmentLocs>
{
public:
  static CxrScheduleSegmentLocsDAO& instance();

  const CxrScheduleSegmentLocs& get(DeleteList& del, const CarrierCode& vendor);

  bool translateKey(const ObjectKey& objectKey, CxrScheduleSegmentLocsKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  void translateKey(const CxrScheduleSegmentLocsKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<CxrScheduleSegmentLocs, CxrScheduleSegmentLocsDAO>(
               flatKey, objectKey).success();
  }

  CxrScheduleSegmentLocsKey createKey(CxrScheduleSegmentLocs*& info);

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

private:
  friend class DAOHelper<CxrScheduleSegmentLocsDAO>;

  CxrScheduleSegmentLocsDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CxrScheduleSegmentLocsKey, CxrScheduleSegmentLocs>(cacheSize, cacheType)
  {
  }

  CxrScheduleSegmentLocs* create(CxrScheduleSegmentLocsKey key) override;
  void destroy(CxrScheduleSegmentLocsKey key, CxrScheduleSegmentLocs* t) override;

  static CxrScheduleSegmentLocsDAO* _instance;
  static log4cxx::LoggerPtr _logger;
  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<CxrScheduleSegmentLocsDAO> _helper;
};

} // namespace tse

