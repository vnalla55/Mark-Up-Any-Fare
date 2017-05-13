//-------------------------------------------------------------------------------
// @ 2007, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/HistoricalDataAccessObject.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class Waiver;
class DeleteList;

typedef HashKey<VendorCode, int> WaiverKey;

class WaiverDAO : public DataAccessObject<WaiverKey, std::vector<Waiver*> >
{
public:
  static WaiverDAO& instance();
  std::vector<Waiver*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& date);

  bool translateKey(const ObjectKey& objectKey, WaiverKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<WaiverDAO>;
  static DAOHelper<WaiverDAO> _helper;
  WaiverDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<WaiverKey, std::vector<Waiver*> >(cacheSize, cacheType, 2)
  {
  }
  std::vector<Waiver*>* create(WaiverKey key) override;
  void destroy(WaiverKey key, std::vector<Waiver*>* t) override;

private:
  static WaiverDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class WaiverDAO

// --------------------------------------------------
// Historical DAO: WaiverHistoricalDAO
// --------------------------------------------------
class WaiverHistoricalDAO : public HistoricalDataAccessObject<WaiverKey, std::vector<Waiver*> >
{
public:
  static WaiverHistoricalDAO& instance();
  std::vector<Waiver*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, WaiverKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<WaiverHistoricalDAO>;
  static DAOHelper<WaiverHistoricalDAO> _helper;
  WaiverHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<WaiverKey, std::vector<Waiver*> >(cacheSize, cacheType, 2)
  {
  }
  std::vector<Waiver*>* create(WaiverKey key) override;
  void destroy(WaiverKey key, std::vector<Waiver*>* vc) override;
  virtual void load() override;

private:
  struct groupByKey;
  static WaiverHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
