//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
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

#include <vector>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class SectorDetailInfo;
class DeleteList;

typedef HashKey<VendorCode, int> SectorDetailKey;

class SectorDetailDAO
    : public DataAccessObject<SectorDetailKey, std::vector<const SectorDetailInfo*> >
{
  friend class DAOHelper<SectorDetailDAO>;

public:
  static SectorDetailDAO& instance();

  const std::vector<const SectorDetailInfo*>
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SectorDetailKey& key) const override;

  void translateKey(const SectorDetailKey& key, ObjectKey& objectKey) const override;

  const SectorDetailKey createKey(const SectorDetailInfo* info);

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override;

  log4cxx::LoggerPtr& getLogger() override { return _logger; }
  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static const std::string _name;
  static const std::string _cacheClass;
  static DAOHelper<SectorDetailDAO> _helper;

  SectorDetailDAO(int cacheSize = 0, const std::string& cacheType = "");

  void load() override;
  std::vector<const SectorDetailInfo*>* create(const SectorDetailKey key) override;
  void destroy(const SectorDetailKey key, std::vector<const SectorDetailInfo*>* t) override;

private:
  static SectorDetailDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

typedef HashKey<VendorCode, int, DateTime, DateTime> SectorDetailHistoricalKey;

class SectorDetailHistoricalDAO
    : public HistoricalDataAccessObject<SectorDetailHistoricalKey,
                                        std::vector<const SectorDetailInfo*> >
{
  friend class DAOHelper<SectorDetailHistoricalDAO>;

public:
  static SectorDetailHistoricalDAO& instance();

  const std::vector<const SectorDetailInfo*>
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SectorDetailHistoricalKey& key) const override;

  bool translateKey(const ObjectKey& objectKey,
                    SectorDetailHistoricalKey& key,
                    const DateTime ticketDate) const override;

  void setKeyDateRange(SectorDetailHistoricalKey& key, const DateTime ticketDate) const override;

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  SectorDetailHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "");

  std::vector<const SectorDetailInfo*>* create(const SectorDetailHistoricalKey key) override;

  void
  destroy(const SectorDetailHistoricalKey key, std::vector<const SectorDetailInfo*>* t) override;

  static const std::string _name;
  static const std::string _cacheClass;
  static DAOHelper<SectorDetailHistoricalDAO> _helper;

private:
  static SectorDetailHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse

