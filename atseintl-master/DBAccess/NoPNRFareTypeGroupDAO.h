//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Common/TseBoostStringTypes.h"
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
class NoPNRFareTypeGroup;
class DeleteList;

typedef int NoPNRFareTypeGroupKey;

class NoPNRFareTypeGroupDAO
    : public DataAccessObject<IntKey, std::vector<NoPNRFareTypeGroup*>, false>
{
public:
  static NoPNRFareTypeGroupDAO& instance();
  const NoPNRFareTypeGroup*
  get(DeleteList& del, const int fareTypeGroup, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, IntKey& key) const override
  {
    return key.initialized = objectKey.getValue("FARETYPEGROUP", key._a);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<NoPNRFareTypeGroupDAO>;
  static DAOHelper<NoPNRFareTypeGroupDAO> _helper;
  NoPNRFareTypeGroupDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<IntKey, std::vector<NoPNRFareTypeGroup*>, false>(cacheSize, cacheType)
  {
  }

  std::vector<NoPNRFareTypeGroup*>* create(IntKey key) override;
  void destroy(IntKey key, std::vector<NoPNRFareTypeGroup*>* t) override;

private:
  static const int dummy = 0;
  static NoPNRFareTypeGroupDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: NoPNRFareTypeGroupHistoricalDAO
// --------------------------------------------------
class NoPNRFareTypeGroupHistoricalDAO
    : public HistoricalDataAccessObject<NoPNRFareTypeGroupKey,
                                        std::vector<NoPNRFareTypeGroup*>,
                                        false>
{
public:
  static NoPNRFareTypeGroupHistoricalDAO& instance();
  const NoPNRFareTypeGroup*
  get(DeleteList& del, const int fareTypeGroup, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, NoPNRFareTypeGroupKey& key) const override
  {
    return objectKey.getValue("FARETYPEGROUP", key);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<NoPNRFareTypeGroupHistoricalDAO>;
  static DAOHelper<NoPNRFareTypeGroupHistoricalDAO> _helper;
  NoPNRFareTypeGroupHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<NoPNRFareTypeGroupKey, std::vector<NoPNRFareTypeGroup*>, false>(
          cacheSize, cacheType)
  {
  }

  std::vector<NoPNRFareTypeGroup*>* create(NoPNRFareTypeGroupKey key) override;
  void destroy(NoPNRFareTypeGroupKey key, std::vector<NoPNRFareTypeGroup*>* t) override;

private:
  static const int dummy = 0;
  static NoPNRFareTypeGroupHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
