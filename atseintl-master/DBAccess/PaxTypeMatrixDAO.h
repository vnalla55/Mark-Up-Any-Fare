//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
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
class PaxTypeMatrix;
class DeleteList;

class PaxTypeMatrixDAO
    : public DataAccessObject<PaxCodeKey, std::vector<const PaxTypeMatrix*>, false>
{
public:
  static PaxTypeMatrixDAO& instance();
  const std::vector<const PaxTypeMatrix*>&
  get(DeleteList& del, const PaxTypeCode& key, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, PaxCodeKey& key) const override
  {
    return key.initialized = objectKey.getValue("PAXTYPECODE", key._a);
  }

  PaxCodeKey createKey(const PaxTypeMatrix* info);

  void translateKey(const PaxCodeKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("PAXTYPECODE", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<PaxTypeMatrix, PaxTypeMatrixDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<PaxTypeMatrixDAO>;
  static DAOHelper<PaxTypeMatrixDAO> _helper;
  PaxTypeMatrixDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<PaxCodeKey, std::vector<const PaxTypeMatrix*>, false>(cacheSize, cacheType)
  {
  }
  virtual void load() override;
  std::vector<const PaxTypeMatrix*>* create(PaxCodeKey key) override;
  void destroy(PaxCodeKey key, std::vector<const PaxTypeMatrix*>* t) override;

private:
  static PaxTypeMatrixDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: PaxTypeMatrixHistoricalDAO
// --------------------------------------------------
class PaxTypeMatrixHistoricalDAO
    : public HistoricalDataAccessObject<PaxTypeCode, std::vector<const PaxTypeMatrix*>, false>
{
public:
  static PaxTypeMatrixHistoricalDAO& instance();
  const std::vector<const PaxTypeMatrix*>&
  get(DeleteList& del, const PaxTypeCode& key, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, PaxTypeCode& key) const override
  {
    return objectKey.getValue("PAXTYPECODE", key);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<PaxTypeMatrixHistoricalDAO>;
  static DAOHelper<PaxTypeMatrixHistoricalDAO> _helper;
  PaxTypeMatrixHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<PaxTypeCode, std::vector<const PaxTypeMatrix*>, false>(cacheSize,
                                                                                        cacheType)
  {
  }
  virtual void load() override;
  std::vector<const PaxTypeMatrix*>* create(PaxTypeCode key) override;
  void destroy(PaxTypeCode key, std::vector<const PaxTypeMatrix*>* t) override;

private:
  struct groupByKey;
  static PaxTypeMatrixHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
