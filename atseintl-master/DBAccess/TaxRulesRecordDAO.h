// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/ChildCache.h"
#include "DBAccess/ChildCacheNotifier.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/HistoricalDataAccessObject.h"

#include <vector>

namespace sfc
{
class CompressedData;
}

namespace tse
{
class DeleteList;

typedef HashKey<NationCode, Indicator> TaxRulesRecordKey;

class TaxRulesRecordDAO
 : public DataAccessObject<TaxRulesRecordKey, std::vector<const TaxRulesRecord*> >,
   public ChildCacheNotifier<TaxRulesRecordKey>
{
  friend class DAOHelper<TaxRulesRecordDAO>;

public:
  static TaxRulesRecordDAO& instance();

  size_t invalidate(const ObjectKey& objectKey) override;

  const std::vector<const TaxRulesRecord*>& get(DeleteList& del,
                                                      const NationCode& nation,
                                                      const Indicator& taxPointTag,
                                                      const DateTime& date,
                                                      const DateTime& ticketDate);

  TaxRulesRecordKey createKey(const TaxRulesRecord* rec);

  bool translateKey(const ObjectKey& objectKey, TaxRulesRecordKey& key) const override;

  void translateKey(const TaxRulesRecordKey& key, ObjectKey& objectKey) const override;

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override;

  const std::string& cacheClass() override { return _cacheClass; }
  Logger& getLogger() override { return _logger; }

  virtual sfc::CompressedData*
  compress(const std::vector<const TaxRulesRecord*>* recList) const override;

  virtual std::vector<const TaxRulesRecord*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static const std::string _name;
  static const std::string _cacheClass;
  static DAOHelper<TaxRulesRecordDAO> _helper;
  TaxRulesRecordDAO(int cacheSize = 0, const std::string& cacheType = "");

  void load() override;
  std::vector<const TaxRulesRecord*>* create(TaxRulesRecordKey key) override;
  void destroy(TaxRulesRecordKey key, std::vector<const TaxRulesRecord*>* recs) override;
  virtual size_t clear() override;

private:
  struct isTravelDate;
  static TaxRulesRecordDAO* _instance;
  static Logger _logger;
};

typedef HashKey<NationCode, Indicator, DateTime, DateTime> RulesRecordHistoricalKey;

class TaxRulesRecordHistoricalDAO
    : public HistoricalDataAccessObject<RulesRecordHistoricalKey,
                                        std::vector<const TaxRulesRecord*> >,
      public ChildCacheNotifier<RulesRecordHistoricalKey>
{
public:
  static TaxRulesRecordHistoricalDAO& instance();

  size_t invalidate(const ObjectKey& objectKey) override;

  const std::vector<const TaxRulesRecord*>& get(DeleteList& del,
                                                      const NationCode& nation,
                                                      const Indicator& taxPointTag,
                                                      const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, RulesRecordHistoricalKey& key) const override;

  bool translateKey(const ObjectKey& objectKey,
                    RulesRecordHistoricalKey& key,
                    const DateTime ticketDate) const override;

  void translateKey(const RulesRecordHistoricalKey& key, ObjectKey& objectKey) const override;

  void setKeyDateRange(RulesRecordHistoricalKey& key, const DateTime ticketDate) const override;

  RulesRecordHistoricalKey
  createKey(const TaxRulesRecord* rec, const DateTime& startDate, const DateTime& endDate);

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override;

  const std::string& cacheClass() override { return _cacheClass; }
  Logger& getLogger() override { return _logger; }

  virtual sfc::CompressedData*
  compress(const std::vector<const TaxRulesRecord*>* recList) const override;

  virtual std::vector<const TaxRulesRecord*>*
  uncompress(const sfc::CompressedData& compressed) const override;

  const DAOUtils::CacheBy getCacheBy() const { return _cacheBy; }

protected:
  static const std::string _name;
  static const std::string _cacheClass;
  friend class DAOHelper<TaxRulesRecordHistoricalDAO>;
  static DAOHelper<TaxRulesRecordHistoricalDAO> _helper;

  TaxRulesRecordHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "");

  std::vector<const TaxRulesRecord*>* create(RulesRecordHistoricalKey key) override;
  void destroy(RulesRecordHistoricalKey key, std::vector<const TaxRulesRecord*>* recs) override;

private:
  struct isTravelDate;
  static TaxRulesRecordHistoricalDAO* _instance;
  static Logger _logger;
};


// BY-TAX-CODE version
//
typedef HashKey<TaxCode> TaxRulesRecordByCodeKey;

class TaxRulesRecordByCodeDAO
 : public DataAccessObject<TaxRulesRecordByCodeKey, std::vector<const TaxRulesRecord*> >,
   public ChildCacheNotifier<TaxRulesRecordByCodeKey>
{
  friend class DAOHelper<TaxRulesRecordByCodeDAO>;

public:
  static TaxRulesRecordByCodeDAO& instance();

  const std::vector<const TaxRulesRecord*>& get(DeleteList& del,
                                                const TaxCode& taxCode,
                                                const DateTime& date,
                                                const DateTime& ticketDate);

  TaxRulesRecordByCodeKey createKey(const TaxRulesRecord* rec);

  bool translateKey(const ObjectKey& objectKey, TaxRulesRecordByCodeKey& key) const override;

  void translateKey(const TaxRulesRecordByCodeKey& key, ObjectKey& objectKey) const override;

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override;

  const std::string& cacheClass() override { return _cacheClass; }
  Logger& getLogger() override { return _logger; }

  virtual sfc::CompressedData*
  compress(const std::vector<const TaxRulesRecord*>* recList) const override;

  virtual std::vector<const TaxRulesRecord*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static const std::string _name;
  static const std::string _cacheClass;
  static DAOHelper<TaxRulesRecordByCodeDAO> _helper;
  TaxRulesRecordByCodeDAO(int cacheSize = 0,
                          const std::string& cacheType = "",
                          size_t version = 2);

  void load() override;
  std::vector<const TaxRulesRecord*>* create(TaxRulesRecordByCodeKey key) override;
  void destroy(TaxRulesRecordByCodeKey key, std::vector<const TaxRulesRecord*>* recs) override;
  virtual size_t clear() override;

private:
  struct isTravelDate;
  static TaxRulesRecordByCodeDAO* _instance;
  static Logger _logger;
};

typedef HashKey<TaxCode, DateTime, DateTime> RulesRecordByCodeHistoricalKey;

class TaxRulesRecordByCodeHistoricalDAO
    : public HistoricalDataAccessObject<RulesRecordByCodeHistoricalKey,
                                        std::vector<const TaxRulesRecord*> >,
      public ChildCacheNotifier<RulesRecordByCodeHistoricalKey>
{
public:
  static TaxRulesRecordByCodeHistoricalDAO& instance();

  const std::vector<const TaxRulesRecord*>& get(DeleteList& del,
                                                const TaxCode& taxCode,
                                                const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, RulesRecordByCodeHistoricalKey& key) const override;

  bool translateKey(const ObjectKey& objectKey,
                    RulesRecordByCodeHistoricalKey& key,
                    const DateTime ticketDate) const override;

  void translateKey(const RulesRecordByCodeHistoricalKey& key, ObjectKey& objectKey) const override;

  void
  setKeyDateRange(RulesRecordByCodeHistoricalKey& key, const DateTime ticketDate) const override;

  RulesRecordByCodeHistoricalKey
  createKey(const TaxRulesRecord* rec, const DateTime& startDate, const DateTime& endDate);

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override;

  const std::string& cacheClass() override { return _cacheClass; }
  Logger& getLogger() override { return _logger; }

  virtual sfc::CompressedData*
  compress(const std::vector<const TaxRulesRecord*>* recList) const override;

  virtual std::vector<const TaxRulesRecord*>*
  uncompress(const sfc::CompressedData& compressed) const override;

  const DAOUtils::CacheBy getCacheBy() const { return _cacheBy; }

protected:
  static const std::string _name;
  static const std::string _cacheClass;
  friend class DAOHelper<TaxRulesRecordByCodeHistoricalDAO>;
  static DAOHelper<TaxRulesRecordByCodeHistoricalDAO> _helper;

  TaxRulesRecordByCodeHistoricalDAO(int cacheSize = 0,
                                    const std::string& cacheType = "",
                                    size_t version = 2);

  std::vector<const TaxRulesRecord*>* create(RulesRecordByCodeHistoricalKey key) override;
  void
  destroy(RulesRecordByCodeHistoricalKey key, std::vector<const TaxRulesRecord*>* recs) override;

private:
  struct isTravelDate;
  static TaxRulesRecordByCodeHistoricalDAO* _instance;
  static Logger _logger;
};

// DEPRECATED
// Remove with ATPCO_TAX_X1byCodeDAORefactor fallback removal
typedef HashKey<TaxCode, TaxType> TaxRulesRecordByCodeAndTypeKey;

class TaxRulesRecordByCodeAndTypeDAO
 : public DataAccessObject<TaxRulesRecordByCodeAndTypeKey, std::vector<const TaxRulesRecord*> >,
   public ChildCacheNotifier<TaxRulesRecordByCodeAndTypeKey>
{
  friend class DAOHelper<TaxRulesRecordByCodeAndTypeDAO>;

public:
  static TaxRulesRecordByCodeAndTypeDAO& instance();

  const std::vector<const TaxRulesRecord*>& get(DeleteList& del,
                                                const TaxCode& taxCode,
                                                const TaxType& taxType,
                                                const DateTime& date,
                                                const DateTime& ticketDate);

  TaxRulesRecordByCodeAndTypeKey createKey(const TaxRulesRecord* rec);

  bool translateKey(const ObjectKey& objectKey, TaxRulesRecordByCodeAndTypeKey& key) const override;

  void translateKey(const TaxRulesRecordByCodeAndTypeKey& key, ObjectKey& objectKey) const override;

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override;

  const std::string& cacheClass() override { return _cacheClass; }
  Logger& getLogger() override { return _logger; }

  virtual sfc::CompressedData*
  compress(const std::vector<const TaxRulesRecord*>* recList) const override;

  virtual std::vector<const TaxRulesRecord*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static const std::string _name;
  static const std::string _cacheClass;
  static DAOHelper<TaxRulesRecordByCodeAndTypeDAO> _helper;
  TaxRulesRecordByCodeAndTypeDAO(int cacheSize = 0, const std::string& cacheType = "", size_t version = 2);

  void load() override;
  std::vector<const TaxRulesRecord*>* create(TaxRulesRecordByCodeAndTypeKey key) override;
  void destroy(TaxRulesRecordByCodeAndTypeKey key, std::vector<const TaxRulesRecord*>* recs) override;
  virtual size_t clear() override;

private:
  struct isTravelDate;
  static TaxRulesRecordByCodeAndTypeDAO* _instance;
  static Logger _logger;
};

// DEPRECATED
// Remove with ATPCO_TAX_X1byCodeDAORefactor fallback removal
typedef HashKey<TaxCode, TaxType, DateTime, DateTime> RulesRecordByCodeAndTypeHistoricalKey;

class TaxRulesRecordByCodeAndTypeHistoricalDAO
    : public HistoricalDataAccessObject<RulesRecordByCodeAndTypeHistoricalKey,
                                        std::vector<const TaxRulesRecord*> >,
      public ChildCacheNotifier<RulesRecordByCodeAndTypeHistoricalKey>
{
public:
  static TaxRulesRecordByCodeAndTypeHistoricalDAO& instance();

  const std::vector<const TaxRulesRecord*>& get(DeleteList& del,
                                                const TaxCode& taxCode,
                                                const TaxType& taxType,
                                                const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, RulesRecordByCodeAndTypeHistoricalKey& key) const override;

  bool translateKey(const ObjectKey& objectKey,
                    RulesRecordByCodeAndTypeHistoricalKey& key,
                    const DateTime ticketDate) const override;

  void translateKey(const RulesRecordByCodeAndTypeHistoricalKey& key, ObjectKey& objectKey) const override;

  void
  setKeyDateRange(RulesRecordByCodeAndTypeHistoricalKey& key, const DateTime ticketDate) const override;

  RulesRecordByCodeAndTypeHistoricalKey
  createKey(const TaxRulesRecord* rec, const DateTime& startDate, const DateTime& endDate);

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override;

  const std::string& cacheClass() override { return _cacheClass; }
  Logger& getLogger() override { return _logger; }

  virtual sfc::CompressedData*
  compress(const std::vector<const TaxRulesRecord*>* recList) const override;

  virtual std::vector<const TaxRulesRecord*>*
  uncompress(const sfc::CompressedData& compressed) const override;

  const DAOUtils::CacheBy getCacheBy() const { return _cacheBy; }

protected:
  static const std::string _name;
  static const std::string _cacheClass;
  friend class DAOHelper<TaxRulesRecordByCodeAndTypeHistoricalDAO>;
  static DAOHelper<TaxRulesRecordByCodeAndTypeHistoricalDAO> _helper;

  TaxRulesRecordByCodeAndTypeHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "", size_t version = 2);

  std::vector<const TaxRulesRecord*>* create(RulesRecordByCodeAndTypeHistoricalKey key) override;
  void
  destroy(RulesRecordByCodeAndTypeHistoricalKey key, std::vector<const TaxRulesRecord*>* recs) override;

private:
  struct isTravelDate;
  static TaxRulesRecordByCodeAndTypeHistoricalDAO* _instance;
  static Logger _logger;
};

} // namespace tse


