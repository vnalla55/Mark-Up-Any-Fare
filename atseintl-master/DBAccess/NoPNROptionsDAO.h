//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Common/TseCodeTypes.h"
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
class NoPNROptions;
class DeleteList;

typedef HashKey<Indicator, UserApplCode> NoPNROptionsKey;

class NoPNROptionsDAO : public DataAccessObject<NoPNROptionsKey, std::vector<NoPNROptions*>, false>
{
public:
  static NoPNROptionsDAO& instance();
  const std::vector<NoPNROptions*>& get(DeleteList& del,
                                        Indicator userApplType,
                                        const UserApplCode& userAppl,
                                        const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, NoPNROptionsKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("USERAPPL", key._a) && objectKey.getValue("USERAPPLCODE", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<NoPNROptionsDAO>;
  static DAOHelper<NoPNROptionsDAO> _helper;
  NoPNROptionsDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<NoPNROptionsKey, std::vector<NoPNROptions*>, false>(cacheSize, cacheType)
  {
  }

  std::vector<NoPNROptions*>* create(NoPNROptionsKey key) override;
  void destroy(NoPNROptionsKey key, std::vector<NoPNROptions*>* t) override;

private:
  static NoPNROptionsDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
// --------------------------------------------------
// Historical DAO: NoPNROptionsHistoricalDAO
// --------------------------------------------------
class NoPNROptionsHistoricalDAO
    : public HistoricalDataAccessObject<NoPNROptionsKey, std::vector<NoPNROptions*>, false>
{
public:
  static NoPNROptionsHistoricalDAO& instance();
  const std::vector<NoPNROptions*>& get(DeleteList& del,
                                        Indicator userApplType,
                                        const UserApplCode& userAppl,
                                        const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, NoPNROptionsKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("USERAPPL", key._a) && objectKey.getValue("USERAPPLCODE", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<NoPNROptionsHistoricalDAO>;
  static DAOHelper<NoPNROptionsHistoricalDAO> _helper;
  NoPNROptionsHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<NoPNROptionsKey, std::vector<NoPNROptions*>, false>(cacheSize,
                                                                                     cacheType)
  {
  }

  std::vector<NoPNROptions*>* create(NoPNROptionsKey key) override;
  void destroy(NoPNROptionsKey key, std::vector<NoPNROptions*>* t) override;

private:
  static NoPNROptionsHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
