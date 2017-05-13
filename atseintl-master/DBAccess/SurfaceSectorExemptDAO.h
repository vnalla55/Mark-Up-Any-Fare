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
#include "DBAccess/HistoricalDataAccessObject.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class SurfaceSectorExempt;
class DeleteList;

typedef HashKey<LocCode, LocCode> SurfaceSectorExemptKey;

class SurfaceSectorExemptDAO
    : public DataAccessObject<SurfaceSectorExemptKey, std::vector<SurfaceSectorExempt*>, false>
{
public:
  static SurfaceSectorExemptDAO& instance();
  const SurfaceSectorExempt* get(DeleteList& del,
                                 const LocCode& origLoc,
                                 const LocCode& destLoc,
                                 const DateTime& date,
                                 const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SurfaceSectorExemptKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("ORIGLOC", key._a) && objectKey.getValue("DESTLOC", key._b);
  }

  SurfaceSectorExemptKey createKey(SurfaceSectorExempt* info);

  void translateKey(const SurfaceSectorExemptKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("ORIGLOC", key._a);
    objectKey.setValue("DESTLOC", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<SurfaceSectorExempt, SurfaceSectorExemptDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SurfaceSectorExemptDAO>;
  static DAOHelper<SurfaceSectorExemptDAO> _helper;
  SurfaceSectorExemptDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<SurfaceSectorExemptKey, std::vector<SurfaceSectorExempt*>, false>(cacheSize,
                                                                                         cacheType)
  {
  }
  std::vector<SurfaceSectorExempt*>* create(SurfaceSectorExemptKey key) override;
  void destroy(SurfaceSectorExemptKey key, std::vector<SurfaceSectorExempt*>* t) override;
  virtual void load() override;

  virtual sfc::CompressedData*
  compress(const std::vector<SurfaceSectorExempt*>* vect) const override;

  virtual std::vector<SurfaceSectorExempt*>*
  uncompress(const sfc::CompressedData& compressed) const override;

private:
  struct isEffective;
  static SurfaceSectorExemptDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: SurfaceSectorExemptHistoricalDAO
// --------------------------------------------------
class SurfaceSectorExemptHistoricalDAO
    : public HistoricalDataAccessObject<SurfaceSectorExemptKey,
                                        std::vector<SurfaceSectorExempt*>,
                                        false>
{
public:
  static SurfaceSectorExemptHistoricalDAO& instance();
  const SurfaceSectorExempt* get(DeleteList& del,
                                 const LocCode& origLoc,
                                 const LocCode& destLoc,
                                 const DateTime& date,
                                 const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SurfaceSectorExemptKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("ORIGLOC", key._a) && objectKey.getValue("DESTLOC", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SurfaceSectorExemptHistoricalDAO>;
  static DAOHelper<SurfaceSectorExemptHistoricalDAO> _helper;
  SurfaceSectorExemptHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<SurfaceSectorExemptKey, std::vector<SurfaceSectorExempt*>, false>(
          cacheSize, cacheType)
  {
  }
  std::vector<SurfaceSectorExempt*>* create(SurfaceSectorExemptKey key) override;
  void destroy(SurfaceSectorExemptKey key, std::vector<SurfaceSectorExempt*>* t) override;
  virtual void load() override;

  virtual sfc::CompressedData*
  compress(const std::vector<SurfaceSectorExempt*>* vect) const override;

  virtual std::vector<SurfaceSectorExempt*>*
  uncompress(const sfc::CompressedData& compressed) const override;

private:
  struct isEffective;
  struct groupByKey;
  static SurfaceSectorExemptHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
