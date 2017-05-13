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
class TaxCodeTextInfo;
class DeleteList;

typedef HashKey<VendorCode, int> TaxCodeTextKey;

class TaxCodeTextDAO : public DataAccessObject<TaxCodeTextKey, std::vector<const TaxCodeTextInfo*> >
{
  friend class DAOHelper<TaxCodeTextDAO>;

public:
  static TaxCodeTextDAO& instance();

  const std::vector<const TaxCodeTextInfo*>
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TaxCodeTextKey& key) const override;

  void translateKey(const TaxCodeTextKey& key, ObjectKey& objectKey) const override;

  const TaxCodeTextKey createKey(const TaxCodeTextInfo* info);

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override;

  log4cxx::LoggerPtr& getLogger() override { return _logger; }
  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static const std::string _name;
  static const std::string _cacheClass;
  static DAOHelper<TaxCodeTextDAO> _helper;

  TaxCodeTextDAO(int cacheSize = 0, const std::string& cacheType = "");

  void load() override;
  std::vector<const TaxCodeTextInfo*>* create(const TaxCodeTextKey key) override;
  void destroy(const TaxCodeTextKey key, std::vector<const TaxCodeTextInfo*>* t) override;

private:
  static TaxCodeTextDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

typedef HashKey<VendorCode, int, DateTime, DateTime> TaxCodeTextHistoricalKey;

class TaxCodeTextHistoricalDAO
    : public HistoricalDataAccessObject<TaxCodeTextHistoricalKey,
                                        std::vector<const TaxCodeTextInfo*> >
{
  friend class DAOHelper<TaxCodeTextHistoricalDAO>;

public:
  static TaxCodeTextHistoricalDAO& instance();

  const std::vector<const TaxCodeTextInfo*>
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TaxCodeTextHistoricalKey& key) const override;

  bool translateKey(const ObjectKey& objectKey,
                    TaxCodeTextHistoricalKey& key,
                    const DateTime ticketDate) const override;

  void setKeyDateRange(TaxCodeTextHistoricalKey& key, const DateTime ticketDate) const override;

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  TaxCodeTextHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "");

  std::vector<const TaxCodeTextInfo*>* create(const TaxCodeTextHistoricalKey key) override;

  void destroy(const TaxCodeTextHistoricalKey key, std::vector<const TaxCodeTextInfo*>* t) override;

  static const std::string _name;
  static const std::string _cacheClass;
  static DAOHelper<TaxCodeTextHistoricalDAO> _helper;

private:
  static TaxCodeTextHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

