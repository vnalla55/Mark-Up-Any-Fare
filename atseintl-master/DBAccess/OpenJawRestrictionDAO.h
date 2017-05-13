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
#include "DBAccess/HistoricalDataAccessObject.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class OpenJawRestriction;
class DeleteList;

typedef HashKey<VendorCode, int> OpenJawRestrictionKey;

class OpenJawRestrictionDAO
    : public DataAccessObject<OpenJawRestrictionKey, std::vector<OpenJawRestriction*> >
{
public:
  static OpenJawRestrictionDAO& instance();
  const std::vector<OpenJawRestriction*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, OpenJawRestrictionKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<OpenJawRestrictionDAO>;
  static DAOHelper<OpenJawRestrictionDAO> _helper;
  OpenJawRestrictionDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<OpenJawRestrictionKey, std::vector<OpenJawRestriction*> >(cacheSize,
                                                                                 cacheType)
  {
  }
  std::vector<OpenJawRestriction*>* create(OpenJawRestrictionKey key) override;
  void destroy(OpenJawRestrictionKey key, std::vector<OpenJawRestriction*>* t) override;

private:
  static OpenJawRestrictionDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: OpenJawRestrictionHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> OpenJawRestrictionHistoricalKey;

class OpenJawRestrictionHistoricalDAO
    : public HistoricalDataAccessObject<OpenJawRestrictionHistoricalKey,
                                        std::vector<OpenJawRestriction*> >
{
public:
  static OpenJawRestrictionHistoricalDAO& instance();
  const std::vector<OpenJawRestriction*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, OpenJawRestrictionHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    OpenJawRestrictionHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void
  setKeyDateRange(OpenJawRestrictionHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<OpenJawRestrictionHistoricalDAO>;
  static DAOHelper<OpenJawRestrictionHistoricalDAO> _helper;
  OpenJawRestrictionHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<OpenJawRestrictionHistoricalKey,
                                 std::vector<OpenJawRestriction*> >(cacheSize, cacheType)
  {
  }
  std::vector<OpenJawRestriction*>* create(OpenJawRestrictionHistoricalKey key) override;
  void destroy(OpenJawRestrictionHistoricalKey key, std::vector<OpenJawRestriction*>* t) override;

private:
  static OpenJawRestrictionHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
