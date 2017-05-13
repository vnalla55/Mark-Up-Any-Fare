//-------------------------------------------------------------------------------
// @ 2007, Sabre Inc.  All rights reserved.
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
class DST;
class DeleteList;

typedef HashKey<DSTGrpCode> DSTKey;

class DSTDAO : public DataAccessObject<DSTKey, DST>
{
public:
  static DSTDAO& instance();

  DST* get(DeleteList& del, const DSTGrpCode& dstGrp);

  bool translateKey(const ObjectKey& objectKey, DSTKey& key) const override
  {
    return key.initialized = objectKey.getValue("DSTGROUP", key._a);
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  DSTKey createKey(const DST* info);

  void translateKey(const DSTKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("DSTGROUP", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<DST, DSTDAO>(flatKey, objectKey).success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<DSTDAO>;

  static DAOHelper<DSTDAO> _helper;

  DSTDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<DSTKey, DST>(cacheSize, cacheType, 2)
  {
  }

  DST* create(DSTKey key) override;

  void destroy(DSTKey key, DST* t) override;

  void load() override;

private:
  static DSTDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse

