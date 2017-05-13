// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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
class TaxExemption;
class DeleteList;

typedef HashKey<TaxCode, PseudoCityCode> TaxExemptionKey;

class TaxExemptionDAO : public DataAccessObject<TaxExemptionKey, std::vector<TaxExemption*>>
{
public:
  static TaxExemptionDAO& instance();

  const std::vector<TaxExemption*>& get(DeleteList& del,
      const TaxExemptionKey& key,
      const DateTime& date,
      const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TaxExemptionKey& key) const override
  {
    return key.initialized =
      objectKey.getValue("T", key._a) && objectKey.getValue("C", key._b);
  }

  void translateKey(const TaxExemptionKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("T", key._a);
    objectKey.setValue("C", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  sfc::CompressedData* compress(const std::vector<TaxExemption*>* vect) const override;
  std::vector<TaxExemption*>* uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TaxExemptionDAO>;
  static DAOHelper<TaxExemptionDAO> _helper;

  TaxExemptionDAO(int cacheSize = 0, const std::string& cacheType = "", size_t version = 1)
    : DataAccessObject<TaxExemptionKey, std::vector<TaxExemption*> >(cacheSize, cacheType, version)
  {}

  std::vector<TaxExemption*>* create(TaxExemptionKey key) override;
  void destroy(TaxExemptionKey key, std::vector<TaxExemption*>* t) override;

private:
  static TaxExemptionDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};


class TaxExemptionHistoricalDAO
    : public HistoricalDataAccessObject<TaxExemptionKey, std::vector<TaxExemption*> >
{
public:
  static TaxExemptionHistoricalDAO& instance();

  const std::vector<TaxExemption*>& get(DeleteList& del,
      const TaxExemptionKey& key,
      const DateTime& date, const
      DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, TaxExemptionKey& key) const override
  {
    return key.initialized =
      objectKey.getValue("T", key._a) && objectKey.getValue("C", key._b);
  }

  void translateKey(const TaxExemptionKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("T", key._a);
    objectKey.setValue("C", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  sfc::CompressedData* compress(const std::vector<TaxExemption*>* vect) const override;
  std::vector<TaxExemption*>* uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<TaxExemptionHistoricalDAO>;
  static DAOHelper<TaxExemptionHistoricalDAO> _helper;

  TaxExemptionHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "", size_t version = 1)
    : HistoricalDataAccessObject<TaxExemptionKey, std::vector<TaxExemption*> >
      (cacheSize, cacheType, version)
  {}

  std::vector<TaxExemption*>* create(TaxExemptionKey key) override;
  void destroy(TaxExemptionKey key, std::vector<TaxExemption*>* t) override;

private:
  static TaxExemptionHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

}
