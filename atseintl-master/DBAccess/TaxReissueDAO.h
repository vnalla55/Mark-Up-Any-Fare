//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/FallbackUtil.h"
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
FIXEDFALLBACK_DECL(fallbackErrorsInTaxDAOFix);

class TaxReissue;
class DeleteList;

class TaxReissueDAO : public DataAccessObject<TaxCode, std::vector<TaxReissue*> >
{
public:
  static TaxReissueDAO& instance();
  const std::vector<TaxReissue*>&
  get(DeleteList& del, const TaxCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TaxCode& key) const override
  {
    if (fallback::fixed::fallbackErrorsInTaxDAOFix())
      return objectKey.getValue("TAXCODECODE", key);
    else
      return objectKey.getValue("TAXCODE", key);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<TaxReissue*>* vect) const override;

  virtual std::vector<TaxReissue*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TaxReissueDAO>;
  static DAOHelper<TaxReissueDAO> _helper;
  TaxReissueDAO(int cacheSize = 0, const std::string& cacheType = "", size_t version = 3)
    : DataAccessObject<TaxCode, std::vector<TaxReissue*> >(cacheSize, cacheType, version)
  {
  }
  std::vector<TaxReissue*>* create(TaxCode key) override;
  void destroy(TaxCode key, std::vector<TaxReissue*>* t) override;

private:
  static TaxReissueDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: TaxReissueHistoricalDAO
// --------------------------------------------------
class TaxReissueHistoricalDAO
    : public HistoricalDataAccessObject<TaxCode, std::vector<TaxReissue*> >
{
public:
  static TaxReissueHistoricalDAO& instance();
  const std::vector<TaxReissue*>&
  get(DeleteList& del, const TaxCode& key, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TaxCode& key) const override
  {
    if (fallback::fixed::fallbackErrorsInTaxDAOFix())
      return objectKey.getValue("TAXCODECODE", key);
    else
      return objectKey.getValue("TAXCODE", key);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<TaxReissue*>* vect) const override;

  virtual std::vector<TaxReissue*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TaxReissueHistoricalDAO>;
  static DAOHelper<TaxReissueHistoricalDAO> _helper;
  TaxReissueHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "", size_t version = 3)
    : HistoricalDataAccessObject<TaxCode, std::vector<TaxReissue*> >(cacheSize, cacheType, version)
  {
  }
  std::vector<TaxReissue*>* create(TaxCode key) override;
  void destroy(TaxCode key, std::vector<TaxReissue*>* t) override;

  virtual void load() override;

private:
  struct groupByKey;
  static TaxReissueHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
