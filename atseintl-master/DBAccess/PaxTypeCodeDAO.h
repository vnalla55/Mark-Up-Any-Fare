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
class PaxTypeCodeInfo;
class DeleteList;

typedef HashKey<VendorCode, int> PaxTypeCodeKey;

class PaxTypeCodeDAO : public DataAccessObject<PaxTypeCodeKey, std::vector<const PaxTypeCodeInfo*> >
{
  friend class DAOHelper<PaxTypeCodeDAO>;

public:
  static PaxTypeCodeDAO& instance();

  const std::vector<const PaxTypeCodeInfo*>
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, PaxTypeCodeKey& key) const override;

  void translateKey(const PaxTypeCodeKey& key, ObjectKey& objectKey) const override;

  const PaxTypeCodeKey createKey(const PaxTypeCodeInfo* info);

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override;

  log4cxx::LoggerPtr& getLogger() override { return _logger; }
  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static const std::string _name;
  static const std::string _cacheClass;
  static DAOHelper<PaxTypeCodeDAO> _helper;

  PaxTypeCodeDAO(int cacheSize = 0, const std::string& cacheType = "");

  void load() override;
  std::vector<const PaxTypeCodeInfo*>* create(const PaxTypeCodeKey key) override;
  void destroy(const PaxTypeCodeKey key, std::vector<const PaxTypeCodeInfo*>* t) override;

private:
  static PaxTypeCodeDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

typedef HashKey<VendorCode, int, DateTime, DateTime> PaxTypeCodeHistoricalKey;

class PaxTypeCodeHistoricalDAO
    : public HistoricalDataAccessObject<PaxTypeCodeHistoricalKey,
                                        std::vector<const PaxTypeCodeInfo*> >
{
  friend class DAOHelper<PaxTypeCodeHistoricalDAO>;

public:
  static PaxTypeCodeHistoricalDAO& instance();

  const std::vector<const PaxTypeCodeInfo*>
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, PaxTypeCodeHistoricalKey& key) const override;

  bool translateKey(const ObjectKey& objectKey,
                    PaxTypeCodeHistoricalKey& key,
                    const DateTime ticketDate) const override;

  void setKeyDateRange(PaxTypeCodeHistoricalKey& key, const DateTime ticketDate) const override;

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  PaxTypeCodeHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "");

  std::vector<const PaxTypeCodeInfo*>* create(const PaxTypeCodeHistoricalKey key) override;

  void destroy(const PaxTypeCodeHistoricalKey key, std::vector<const PaxTypeCodeInfo*>* t) override;

  static const std::string _name;
  static const std::string _cacheClass;
  static DAOHelper<PaxTypeCodeHistoricalDAO> _helper;

private:
  static PaxTypeCodeHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

