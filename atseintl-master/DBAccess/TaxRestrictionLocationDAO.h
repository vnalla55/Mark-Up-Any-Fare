//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"

namespace tse
{
class TaxRestrictionLocationInfo;
class DeleteList;

typedef HashKey<TaxRestrictionLocation> TaxRestrictionLocationKey;

class TaxRestrictionLocationDAO
    : public DataAccessObject<TaxRestrictionLocationKey,
                              std::vector<const TaxRestrictionLocationInfo*> >
{
public:
  static TaxRestrictionLocationDAO& instance();
  static log4cxx::LoggerPtr _logger;

  const TaxRestrictionLocationInfo*
  get(DeleteList& del, const TaxRestrictionLocation& location, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TaxRestrictionLocationKey& key) const override
  {
    return key.initialized = objectKey.getValue("TAXRESTRICTIONLOCATIONNO", key._a);
  }

  TaxRestrictionLocationKey createKey(const TaxRestrictionLocationInfo* info);

  void translateKey(const TaxRestrictionLocationKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("TAXRESTRICTIONLOCATIONNO", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<TaxRestrictionLocationInfo, TaxRestrictionLocationDAO>(
               flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<TaxRestrictionLocationDAO>;

  static DAOHelper<TaxRestrictionLocationDAO> _helper;

  TaxRestrictionLocationDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<TaxRestrictionLocationKey, std::vector<const TaxRestrictionLocationInfo*> >(
          cacheSize, cacheType, 4)
  {
  }

  void load() override;

  std::vector<const TaxRestrictionLocationInfo*>* create(TaxRestrictionLocationKey key) override;

  void destroy(TaxRestrictionLocationKey key,
               std::vector<const TaxRestrictionLocationInfo*>* t) override;

private:
  struct isEffective;

  static TaxRestrictionLocationDAO* _instance;

}; // class TaxRestrictionLocationDAO

// --------------------------------------------------
// Historical DAO: TaxRestrictionLocationHistoricalDAO
// --------------------------------------------------

typedef HashKey<TaxRestrictionLocation, DateTime, DateTime> TaxRestrictionLocationHistoricalKey;

class TaxRestrictionLocationHistoricalDAO
    : public HistoricalDataAccessObject<TaxRestrictionLocationHistoricalKey,
                                        std::vector<const TaxRestrictionLocationInfo*> >
{
public:
  static TaxRestrictionLocationHistoricalDAO& instance();
  static log4cxx::LoggerPtr _logger;

  const TaxRestrictionLocationInfo*
  get(DeleteList& del, const TaxRestrictionLocation& location, const DateTime& ticketDate);

  bool
  translateKey(const ObjectKey& objectKey, TaxRestrictionLocationHistoricalKey& key) const override
  {
    return key.initialized = objectKey.getValue("TAXRESTRICTIONLOCATIONNO", key._a) &&
                             objectKey.getValue("STARTDATE", key._b) &&
                             objectKey.getValue("ENDDATE", key._c);
  }

  bool translateKey(const ObjectKey& objectKey,
                    TaxRestrictionLocationHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("TAXRESTRICTIONLOCATIONNO", key._a);
  }

  void setKeyDateRange(TaxRestrictionLocationHistoricalKey& key, const DateTime date) const override
  {
    DAOUtils::getDateRange(date, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<TaxRestrictionLocationHistoricalDAO>;

  static DAOHelper<TaxRestrictionLocationHistoricalDAO> _helper;

  TaxRestrictionLocationHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<TaxRestrictionLocationHistoricalKey,
                                 std::vector<const TaxRestrictionLocationInfo*> >(
          cacheSize, cacheType, 4)
  {
  }

  std::vector<const TaxRestrictionLocationInfo*>*
  create(TaxRestrictionLocationHistoricalKey key) override;

  void destroy(TaxRestrictionLocationHistoricalKey key,
               std::vector<const TaxRestrictionLocationInfo*>* t) override;

private:
  struct isEffective;
  static TaxRestrictionLocationHistoricalDAO* _instance;

}; // class TaxRestrictionLocationHistoricalDAO

} // namespace tse

