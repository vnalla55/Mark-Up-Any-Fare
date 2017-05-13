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
class JointCarrier;
class DeleteList;

typedef HashKey<VendorCode, int> JointCarrierKey;

class JointCarrierDAO : public DataAccessObject<JointCarrierKey, std::vector<const JointCarrier*> >
{
public:
  static JointCarrierDAO& instance();
  const std::vector<const JointCarrier*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, JointCarrierKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  JointCarrierKey createKey(const JointCarrier* info);

  void translateKey(const JointCarrierKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("ITEMNO", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<JointCarrier, JointCarrierDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<JointCarrierDAO>;
  static DAOHelper<JointCarrierDAO> _helper;
  JointCarrierDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<JointCarrierKey, std::vector<const JointCarrier*> >(cacheSize, cacheType)
  {
  }
  void load() override;
  std::vector<const JointCarrier*>* create(JointCarrierKey key) override;
  void destroy(JointCarrierKey key, std::vector<const JointCarrier*>* t) override;

private:
  static JointCarrierDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: JointCarrierHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> JointCarrierHistoricalKey;

class JointCarrierHistoricalDAO
    : public HistoricalDataAccessObject<JointCarrierHistoricalKey,
                                        std::vector<const JointCarrier*> >
{
public:
  static JointCarrierHistoricalDAO& instance();
  const std::vector<const JointCarrier*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, JointCarrierHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    JointCarrierHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(JointCarrierHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<JointCarrierHistoricalDAO>;
  static DAOHelper<JointCarrierHistoricalDAO> _helper;
  JointCarrierHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<JointCarrierHistoricalKey, std::vector<const JointCarrier*> >(
          cacheSize, cacheType)
  {
  }
  std::vector<const JointCarrier*>* create(JointCarrierHistoricalKey key) override;
  void destroy(JointCarrierHistoricalKey key, std::vector<const JointCarrier*>* t) override;

private:
  static JointCarrierHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
