//------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
//
#pragma once

#include "Common/TsePrimitiveTypes.h"
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
class FareDispTemplateSeg;
class DeleteList;

typedef HashKey<int, Indicator> FareDispTemplateSegKey;

class FareDispTemplateSegDAO
    : public DataAccessObject<FareDispTemplateSegKey, std::vector<FareDispTemplateSeg*>, false>
{
public:
  static FareDispTemplateSegDAO& instance();

  const std::vector<FareDispTemplateSeg*>& get(DeleteList& del,
                                               const int& templateID,
                                               const Indicator& templateType,
                                               const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareDispTemplateSegKey& key) const override
  {
    return key.initialized = objectKey.getValue("TEMPLATEID", key._a) &&
                             objectKey.getValue("TEMPLATETYPE", key._b);
  }

  FareDispTemplateSegKey createKey(FareDispTemplateSeg* info);

  void translateKey(const FareDispTemplateSegKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("TEMPLATEID", key._a);
    objectKey.setValue("TEMPLATETYPE", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FareDispTemplateSeg, FareDispTemplateSegDAO>(flatKey, objectKey)
        .success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FareDispTemplateSegDAO>;

  static DAOHelper<FareDispTemplateSegDAO> _helper;

  FareDispTemplateSegDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FareDispTemplateSegKey, std::vector<FareDispTemplateSeg*>, false>(cacheSize,
                                                                                         cacheType)
  {
  }

  std::vector<FareDispTemplateSeg*>* create(FareDispTemplateSegKey key) override;
  void destroy(FareDispTemplateSegKey key, std::vector<FareDispTemplateSeg*>* recs) override;

  virtual void load() override;

private:
  static FareDispTemplateSegDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: FareDispTemplateSegHistoricalDAO
// --------------------------------------------------
class FareDispTemplateSegHistoricalDAO
    : public HistoricalDataAccessObject<FareDispTemplateSegKey,
                                        std::vector<FareDispTemplateSeg*>,
                                        false>
{
public:
  static FareDispTemplateSegHistoricalDAO& instance();

  const std::vector<FareDispTemplateSeg*>& get(DeleteList& del,
                                               const int& templateID,
                                               const Indicator& templateType,
                                               const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FareDispTemplateSegKey& key) const override
  {
    return key.initialized = objectKey.getValue("TEMPLATEID", key._a) &&
                             objectKey.getValue("TEMPLATETYPE", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FareDispTemplateSegHistoricalDAO>;

  static DAOHelper<FareDispTemplateSegHistoricalDAO> _helper;

  FareDispTemplateSegHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FareDispTemplateSegKey, std::vector<FareDispTemplateSeg*>, false>(
          cacheSize, cacheType)
  {
  }

  std::vector<FareDispTemplateSeg*>* create(FareDispTemplateSegKey key) override;
  void destroy(FareDispTemplateSegKey key, std::vector<FareDispTemplateSeg*>* recs) override;

private:
  static FareDispTemplateSegHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

