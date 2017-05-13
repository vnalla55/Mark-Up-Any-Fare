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
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/HistoricalDataAccessObject.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class DiscountInfo;
class DeleteList;

typedef HashKey<VendorCode, int, int> DiscountKey;

const std::string DISCOUNT_TYPE_CHILDREN = "CHILDRENSDISCOUNTS";
const std::string DISCOUNT_TYPE_TOUR = "TOURCONDUCTORDISCNT";
const std::string DISCOUNT_TYPE_AGENT = "AGENTDISCOUNTS";
const std::string DISCOUNT_TYPE_OTHER = "OTHERDISCOUNTS";
const std::string DISCOUNT_TYPE_INVALID = "";

class DiscountDAO : public DataAccessObject<DiscountKey, std::vector<DiscountInfo*> >
{
public:
  static DiscountDAO& instance();

  static bool keyFromNotify(const ObjectKey& objectKey, DiscountKey& key)
  {
    key.initialized = objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);

    if (key.initialized)
    {
      if (objectKey.entityType() == DISCOUNT_TYPE_CHILDREN)
      {
        key._c = DiscountInfo::CHILD;
      }
      else if (objectKey.entityType() == DISCOUNT_TYPE_TOUR)
      {
        key._c = DiscountInfo::TOUR;
      }
      else if (objectKey.entityType() == DISCOUNT_TYPE_AGENT)
      {
        key._c = DiscountInfo::AGENT;
      }
      else if (objectKey.entityType() == DISCOUNT_TYPE_OTHER)
      {
        key._c = DiscountInfo::OTHERS;
      }
      else
      {
        key._c = 0;
        key.initialized = false;
      }
    }

    return key.initialized;
  }

  static void notifyFromKey(const DiscountKey& key, ObjectKey& objectKey)
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
    switch (key._c)
    {
    case DiscountInfo::CHILD:
      objectKey.entityType() = DISCOUNT_TYPE_CHILDREN;
      break;
    case DiscountInfo::TOUR:
      objectKey.entityType() = DISCOUNT_TYPE_TOUR;
      break;
    case DiscountInfo::AGENT:
      objectKey.entityType() = DISCOUNT_TYPE_AGENT;
      break;
    case DiscountInfo::OTHERS:
      objectKey.entityType() = DISCOUNT_TYPE_OTHER;
      break;
    default:
      objectKey.entityType() = DISCOUNT_TYPE_INVALID;
      break;
    }
  }

  const DiscountInfo* get(DeleteList& del,
                          const VendorCode& vendor,
                          int itemNo,
                          int category,
                          const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, DiscountKey& key) const override
  {
    return keyFromNotify(objectKey, key);
  }

  DiscountKey createKey(DiscountInfo* info);

  void translateKey(const DiscountKey& key, ObjectKey& objectKey) const override
  {
    notifyFromKey(key, objectKey);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<DiscountInfo, DiscountDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<DiscountInfo*>* vect) const override;

  virtual std::vector<DiscountInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<DiscountDAO>;
  static DAOHelper<DiscountDAO> _helper;
  DiscountDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<DiscountKey, std::vector<DiscountInfo*> >(cacheSize, cacheType, 3)
  {
  }
  void load() override;
  std::vector<DiscountInfo*>* create(DiscountKey key) override;
  void destroy(DiscountKey key, std::vector<DiscountInfo*>* t) override;

private:
  static DiscountDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class DiscountDAO

// --------------------------------------------------
// Historical DAO: DiscountHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, int, DateTime, DateTime> DiscountHistoricalKey;

template <>
template <>
DiscountKey&
DiscountKey::
operator=<DiscountHistoricalKey>(const DiscountHistoricalKey& other)
{
  initialized = other.initialized;
  _a = other._a;
  _b = other._b;
  _c = other._c;
  return *this;
}

template <>
template <>
DiscountHistoricalKey&
DiscountHistoricalKey::
operator=<DiscountKey>(const DiscountKey& other)
{
  initialized = other.initialized;
  _a = other._a;
  _b = other._b;
  _c = other._c;
  return *this;
}

class DiscountHistoricalDAO
    : public HistoricalDataAccessObject<DiscountHistoricalKey, std::vector<DiscountInfo*> >
{
public:
  static DiscountHistoricalDAO& instance();
  const DiscountInfo* get(DeleteList& del,
                          const VendorCode& vendor,
                          int itemNo,
                          int category,
                          const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, DiscountHistoricalKey& key) const override
  {
    DiscountKey nhKey;
    if (DiscountDAO::keyFromNotify(objectKey, nhKey))
    {
      key = nhKey;
      return key.initialized =
                 objectKey.getValue("STARTDATE", key._d) && objectKey.getValue("ENDDATE", key._e);
    }
    return key.initialized = false;
  }

  bool translateKey(const ObjectKey& objectKey,
                    DiscountHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DiscountKey nhKey;
    if (DiscountDAO::keyFromNotify(objectKey, nhKey))
    {
      key = nhKey;
      DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
      return key.initialized = true;
    }
    return key.initialized = false;
  }

  void translateKey(const DiscountHistoricalKey& key, ObjectKey& objectKey) const override
  {
    DiscountKey nhKey;
    nhKey = key;
    DiscountDAO::notifyFromKey(nhKey, objectKey);
    objectKey.setValue("STARTDATE", key._d);
    objectKey.setValue("ENDDATE", key._e);
  }

  void setKeyDateRange(DiscountHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<DiscountInfo*>* vect) const override;

  virtual std::vector<DiscountInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<DiscountHistoricalDAO>;
  static DAOHelper<DiscountHistoricalDAO> _helper;
  DiscountHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<DiscountHistoricalKey, std::vector<DiscountInfo*> >(
          cacheSize, cacheType, 3)
  {
  }
  std::vector<DiscountInfo*>* create(DiscountHistoricalKey key) override;
  void destroy(DiscountHistoricalKey key, std::vector<DiscountInfo*>* t) override;

private:
  static DiscountHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class DiscountHistoricalDAO
} // namespace tse

