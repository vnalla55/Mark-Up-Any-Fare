//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/ChildCacheNotifier.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/HistoricalDataAccessObject.h"

#include <vector>

namespace tse
{
class TaxReportingRecordInfo;
class DeleteList;

typedef HashKey<VendorCode, NationCode, CarrierCode, TaxCode, TaxType> TaxReportingRecordKey;

class TaxReportingRecordDAO
    : public DataAccessObject<TaxReportingRecordKey, std::vector<const TaxReportingRecordInfo*> >,
      public ChildCacheNotifier<TaxReportingRecordKey>
{
  friend class DAOHelper<TaxReportingRecordDAO>;

public:
  static TaxReportingRecordDAO& instance();

  size_t invalidate(const ObjectKey& objectKey) override;

  const std::vector<const TaxReportingRecordInfo*> get(DeleteList& del,
                                                       const VendorCode& vendor,
                                                       const NationCode& nation,
                                                       const CarrierCode& taxCarrier,
                                                       const TaxCode& taxCode,
                                                       const TaxType& taxType,
                                                       const DateTime& date,
                                                       const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TaxReportingRecordKey& key) const override;

  void translateKey(const TaxReportingRecordKey& key, ObjectKey& objectKey) const override;

  const TaxReportingRecordKey createKey(const TaxReportingRecordInfo* info);

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override;

  Logger& getLogger() override { return _logger; }
  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static const std::string _name;
  static const std::string _cacheClass;
  static DAOHelper<TaxReportingRecordDAO> _helper;

  TaxReportingRecordDAO(int cacheSize = 0, const std::string& cacheType = "", size_t version = 2);

  void load() override;
  std::vector<const TaxReportingRecordInfo*>* create(const TaxReportingRecordKey key) override;
  void
  destroy(const TaxReportingRecordKey key, std::vector<const TaxReportingRecordInfo*>* t) override;
  virtual size_t clear() override;

private:
  static TaxReportingRecordDAO* _instance;
  static Logger _logger;
};

typedef HashKey<VendorCode, NationCode, CarrierCode, TaxCode, TaxType, DateTime, DateTime>
TaxReportingRecordHistoricalKey;

class TaxReportingRecordHistoricalDAO
    : public HistoricalDataAccessObject<TaxReportingRecordHistoricalKey,
                                        std::vector<const TaxReportingRecordInfo*> >,
      public ChildCacheNotifier<TaxReportingRecordHistoricalKey>
{
  friend class DAOHelper<TaxReportingRecordHistoricalDAO>;

public:
  static TaxReportingRecordHistoricalDAO& instance();

  size_t invalidate(const ObjectKey& objectKey) override;

  const std::vector<const TaxReportingRecordInfo*> get(DeleteList& del,
                                                       const VendorCode& vendor,
                                                       const NationCode& nation,
                                                       const CarrierCode& taxCarrier,
                                                       const TaxCode& taxCode,
                                                       const TaxType& taxType,
                                                       const DateTime& date,
                                                       const DateTime& ticketDate);

  bool
  translateKey(const ObjectKey& objectKey, TaxReportingRecordHistoricalKey& key) const override;

  bool translateKey(const ObjectKey& objectKey,
                    TaxReportingRecordHistoricalKey& key,
                    const DateTime ticketDate) const override;

  void
  setKeyDateRange(TaxReportingRecordHistoricalKey& key, const DateTime ticketDate) const override;

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  TaxReportingRecordHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "", size_t version = 2);

  std::vector<const TaxReportingRecordInfo*>*
  create(const TaxReportingRecordHistoricalKey key) override;

  void destroy(const TaxReportingRecordHistoricalKey key,
               std::vector<const TaxReportingRecordInfo*>* t) override;
  virtual size_t clear() override;

  static const std::string _name;
  static const std::string _cacheClass;
  static DAOHelper<TaxReportingRecordHistoricalDAO> _helper;

private:
  static TaxReportingRecordHistoricalDAO* _instance;
  static Logger _logger;
};

// BY-CODE version

typedef HashKey<TaxCode> TaxReportingRecordByCodeKey;

class TaxReportingRecordByCodeDAO
    : public DataAccessObject<TaxReportingRecordByCodeKey,
                              std::vector<const TaxReportingRecordInfo*> >,
      public ChildCacheNotifier<TaxReportingRecordByCodeKey>
{
  friend class DAOHelper<TaxReportingRecordByCodeDAO>;

public:
  static TaxReportingRecordByCodeDAO& instance();

  std::vector<const TaxReportingRecordInfo*> get(DeleteList& del, const TaxCode& taxCode);

  bool translateKey(const ObjectKey& objectKey, TaxReportingRecordByCodeKey& key) const override;

  void translateKey(const TaxReportingRecordByCodeKey& key, ObjectKey& objectKey) const override;

  const TaxReportingRecordByCodeKey createKey(const TaxReportingRecordInfo* info);

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override;

  Logger& getLogger() override { return _logger; }
  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static const std::string _name;
  static const std::string _cacheClass;
  static DAOHelper<TaxReportingRecordByCodeDAO> _helper;

  TaxReportingRecordByCodeDAO(int cacheSize = 0, const std::string& cacheType = "", size_t version = 2);

  void load() override;
  std::vector<const TaxReportingRecordInfo*>*
  create(const TaxReportingRecordByCodeKey key) override;
  void destroy(const TaxReportingRecordByCodeKey key,
               std::vector<const TaxReportingRecordInfo*>* t) override;
  virtual size_t clear() override;

private:
  static TaxReportingRecordByCodeDAO* _instance;
  static Logger _logger;
};

typedef HashKey<TaxCode> TaxReportingRecordByCodeHistoricalKey;

class TaxReportingRecordByCodeHistoricalDAO
    : public HistoricalDataAccessObject<TaxReportingRecordByCodeHistoricalKey,
                                        std::vector<const TaxReportingRecordInfo*> >,
      public ChildCacheNotifier<TaxReportingRecordByCodeHistoricalKey>
{
  friend class DAOHelper<TaxReportingRecordByCodeHistoricalDAO>;

public:
  static TaxReportingRecordByCodeHistoricalDAO& instance();

  std::vector<const TaxReportingRecordInfo*> get(DeleteList& del, const TaxCode& taxCode);

  bool translateKey(const ObjectKey& objectKey,
                    TaxReportingRecordByCodeHistoricalKey& key) const override;

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  TaxReportingRecordByCodeHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "", size_t version = 2);

  std::vector<const TaxReportingRecordInfo*>*
  create(const TaxReportingRecordByCodeHistoricalKey key) override;

  void destroy(const TaxReportingRecordByCodeHistoricalKey key,
               std::vector<const TaxReportingRecordInfo*>* t) override;
  virtual size_t clear() override;

  static const std::string _name;
  static const std::string _cacheClass;
  static DAOHelper<TaxReportingRecordByCodeHistoricalDAO> _helper;

private:
  static TaxReportingRecordByCodeHistoricalDAO* _instance;
  static Logger _logger;
};

} // namespace tse

