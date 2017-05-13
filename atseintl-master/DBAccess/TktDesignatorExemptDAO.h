//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
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
class TktDesignatorExemptInfo;
class DeleteList;

typedef HashKey<CarrierCode> TktDesignatorExemptKey;

class TktDesignatorExemptDAO
    : public DataAccessObject<TktDesignatorExemptKey, std::vector<TktDesignatorExemptInfo*> >
{
public:
  static TktDesignatorExemptDAO& instance();

  const std::vector<TktDesignatorExemptInfo*>&
  get(DeleteList& del, const CarrierCode& carrier, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TktDesignatorExemptKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  TktDesignatorExemptKey createKey(const TktDesignatorExemptInfo* info);

  void translateKey(const TktDesignatorExemptKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TktDesignatorExemptDAO>;
  static DAOHelper<TktDesignatorExemptDAO> _helper;
  TktDesignatorExemptDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<TktDesignatorExemptKey, std::vector<TktDesignatorExemptInfo*> >(
          cacheSize, cacheType, 2)
  {
  }
  virtual std::vector<TktDesignatorExemptInfo*>* create(TktDesignatorExemptKey key) override;
  void destroy(TktDesignatorExemptKey key, std::vector<TktDesignatorExemptInfo*>* recs) override;

private:
  static TktDesignatorExemptDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical
typedef HashKey<CarrierCode, DateTime, DateTime> TktDesignatorExemptHistoricalKey;

class TktDesignatorExemptHistoricalDAO
    : public HistoricalDataAccessObject<TktDesignatorExemptHistoricalKey,
                                        std::vector<TktDesignatorExemptInfo*> >
{
public:
  static TktDesignatorExemptHistoricalDAO& instance();

  const std::vector<TktDesignatorExemptInfo*>&
  get(DeleteList& del, const CarrierCode& carrier, const DateTime& ticketDate);

  bool
  translateKey(const ObjectKey& objectKey, TktDesignatorExemptHistoricalKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a) &&
                             objectKey.getValue("STARTDATE", key._b) &&
                             objectKey.getValue("ENDDATE", key._c);
  }

  bool translateKey(const ObjectKey& objectKey,
                    TktDesignatorExemptHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  void
  translateKey(const TktDesignatorExemptHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("CARRIER", key._a);
    objectKey.setValue("STARTDATE", key._b);
    objectKey.setValue("ENDDATE", key._c);
  }

  void
  setKeyDateRange(TktDesignatorExemptHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  TktDesignatorExemptHistoricalKey createKey(const TktDesignatorExemptInfo* info,
                                             const DateTime& startDate = DateTime::emptyDate(),
                                             const DateTime& endDate = DateTime::emptyDate());

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TktDesignatorExemptHistoricalDAO>;
  static DAOHelper<TktDesignatorExemptHistoricalDAO> _helper;

  TktDesignatorExemptHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<TktDesignatorExemptHistoricalKey,
                                 std::vector<TktDesignatorExemptInfo*> >(cacheSize, cacheType, 2)
  {
  }
  std::vector<TktDesignatorExemptInfo*>* create(TktDesignatorExemptHistoricalKey key) override;
  void destroy(TktDesignatorExemptHistoricalKey key,
               std::vector<TktDesignatorExemptInfo*>* recs) override;

private:
  static TktDesignatorExemptHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
