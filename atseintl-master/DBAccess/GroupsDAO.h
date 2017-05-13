//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
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
class Groups;
class DeleteList;

typedef HashKey<VendorCode, int> GroupsKey;

class GroupsDAO : public DataAccessObject<GroupsKey, std::vector<Groups*> >
{
public:
  static GroupsDAO& instance();
  const Groups*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, GroupsKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  GroupsKey createKey(const Groups* info);

  void translateKey(const GroupsKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<Groups, GroupsDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<GroupsDAO>;
  static DAOHelper<GroupsDAO> _helper;
  GroupsDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<GroupsKey, std::vector<Groups*> >(cacheSize, cacheType)
  {
  }
  std::vector<Groups*>* create(GroupsKey key) override;
  void destroy(GroupsKey key, std::vector<Groups*>* t) override;
  void load() override;

private:
  static GroupsDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: GroupsHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> GroupsHistoricalKey;

class GroupsHistoricalDAO
    : public HistoricalDataAccessObject<GroupsHistoricalKey, std::vector<Groups*> >
{
public:
  static GroupsHistoricalDAO& instance();
  const Groups*
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, GroupsHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    GroupsHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(GroupsHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<GroupsHistoricalDAO>;
  static DAOHelper<GroupsHistoricalDAO> _helper;
  GroupsHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<GroupsHistoricalKey, std::vector<Groups*> >(cacheSize, cacheType)
  {
  }
  std::vector<Groups*>* create(GroupsHistoricalKey key) override;
  void destroy(GroupsHistoricalKey key, std::vector<Groups*>* t) override;

private:
  static GroupsHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
