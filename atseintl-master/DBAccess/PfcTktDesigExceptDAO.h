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
class PfcTktDesigExcept;
class DeleteList;

typedef HashKey<CarrierCode> PfcTktDesigExceptKey;

class PfcTktDesigExceptDAO
    : public DataAccessObject<PfcTktDesigExceptKey, std::vector<const PfcTktDesigExcept*> >
{
public:
  static PfcTktDesigExceptDAO& instance();
  const std::vector<const PfcTktDesigExcept*>& get(DeleteList& del,
                                                   const CarrierCode& carrier,
                                                   const DateTime& date,
                                                   const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, PfcTktDesigExceptKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<PfcTktDesigExceptDAO>;
  static DAOHelper<PfcTktDesigExceptDAO> _helper;
  PfcTktDesigExceptDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<PfcTktDesigExceptKey, std::vector<const PfcTktDesigExcept*> >(
          cacheSize, cacheType, 2)
  {
  }

  std::vector<const PfcTktDesigExcept*>* create(PfcTktDesigExceptKey key) override;
  void destroy(PfcTktDesigExceptKey key, std::vector<const PfcTktDesigExcept*>* t) override;

private:
  struct isEffective;
  static PfcTktDesigExceptDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: PfcTktDesigExceptHistoricalDAO
// --------------------------------------------------
typedef HashKey<CarrierCode, DateTime, DateTime> PfcTktDesigExceptHistoricalKey;

class PfcTktDesigExceptHistoricalDAO
    : public HistoricalDataAccessObject<PfcTktDesigExceptHistoricalKey,
                                        std::vector<const PfcTktDesigExcept*> >
{
public:
  static PfcTktDesigExceptHistoricalDAO& instance();
  const std::vector<const PfcTktDesigExcept*>& get(DeleteList& del,
                                                   const CarrierCode& carrier,
                                                   const DateTime& date,
                                                   const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, PfcTktDesigExceptHistoricalKey& key) const override
  {
    return key.initialized = objectKey.getValue("CARRIER", key._a) &&
                             objectKey.getValue("STARTDATE", key._b) &&
                             objectKey.getValue("ENDDATE", key._c);
  }

  bool translateKey(const ObjectKey& objectKey,
                    PfcTktDesigExceptHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("CARRIER", key._a);
  }

  void setKeyDateRange(PfcTktDesigExceptHistoricalKey& key, const DateTime date) const override
  {
    DAOUtils::getDateRange(date, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<PfcTktDesigExceptHistoricalDAO>;
  static DAOHelper<PfcTktDesigExceptHistoricalDAO> _helper;
  PfcTktDesigExceptHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<PfcTktDesigExceptHistoricalKey,
                                 std::vector<const PfcTktDesigExcept*> >(cacheSize, cacheType, 2)
  {
  }

  std::vector<const PfcTktDesigExcept*>* create(PfcTktDesigExceptHistoricalKey key) override;
  void
  destroy(PfcTktDesigExceptHistoricalKey key, std::vector<const PfcTktDesigExcept*>* t) override;

private:
  struct isEffective;
  static PfcTktDesigExceptHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
