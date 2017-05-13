// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOUtils.h"
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
class AirlineAllianceContinentInfo;
class DeleteList;

typedef HashKey<GenericAllianceCode> GenericAllianceCodeKey;

class AirlineAllianceContinentDAO
    : public DataAccessObject<GenericAllianceCodeKey, std::vector<AirlineAllianceContinentInfo*>>
{
public:
  static AirlineAllianceContinentDAO& instance();

  const std::vector<AirlineAllianceContinentInfo*>&
  get(DeleteList& del,
      const GenericAllianceCode& genericAllianceCode,
      const DateTime& ticketDate,
      bool reduceTemporaryVectorsFallback);

  bool translateKey(const ObjectKey& objectKey, GenericAllianceCodeKey& key) const override
  {
    return (key.initialized = (objectKey.getValue("GENERICAIRLINECODE", key._a)));
  }

  GenericAllianceCodeKey createKey(AirlineAllianceContinentInfo* info);

  void translateKey(const GenericAllianceCodeKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("GENERICAIRLINECODE", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<AirlineAllianceContinentInfo, AirlineAllianceContinentDAO>(
               flatKey, objectKey).success();
  }
  virtual size_t invalidate(const ObjectKey& objectKey) override;

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<AirlineAllianceContinentDAO>;
  static DAOHelper<AirlineAllianceContinentDAO> _helper;
  AirlineAllianceContinentDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<GenericAllianceCodeKey, std::vector<AirlineAllianceContinentInfo*>>(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<AirlineAllianceContinentInfo*>* create(GenericAllianceCodeKey key) override;
  void
  destroy(GenericAllianceCodeKey key, std::vector<AirlineAllianceContinentInfo*>* recs) override;

  void load() override;

private:
  static AirlineAllianceContinentDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

typedef HashKey<GenericAllianceCode, DateTime, DateTime> GenericAllianceCodeHistoricalKey;

class AirlineAllianceContinentHistoricalDAO
    : public HistoricalDataAccessObject<GenericAllianceCodeHistoricalKey,
                                        std::vector<AirlineAllianceContinentInfo*>>
{
public:
  static AirlineAllianceContinentHistoricalDAO& instance();

  const std::vector<AirlineAllianceContinentInfo*>&
  get(DeleteList& del,
      const GenericAllianceCode& genericAllianceCode,
      const DateTime& ticketDate,
      bool reduceTemporaryVectorsFallback);

  bool
  translateKey(const ObjectKey& objectKey, GenericAllianceCodeHistoricalKey& key) const override
  {
    key.initialized = objectKey.getValue("GENERICAIRLINECODE", key._a);
    if (key.initialized && (_cacheBy != DAOUtils::NODATES))
    {
      key.initialized =
          objectKey.getValue("STARTDATE", key._b) && objectKey.getValue("ENDDATE", key._c);
    }
    return key.initialized;
  }

  bool translateKey(const ObjectKey& objectKey,
                    GenericAllianceCodeHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("GENERICAIRLINECODE", key._a);
  }

  void
  setKeyDateRange(GenericAllianceCodeHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }
  virtual size_t invalidate(const ObjectKey& objectKey) override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<AirlineAllianceContinentHistoricalDAO>;
  static DAOHelper<AirlineAllianceContinentHistoricalDAO> _helper;
  AirlineAllianceContinentHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<GenericAllianceCodeHistoricalKey,
                                 std::vector<AirlineAllianceContinentInfo*>>(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<AirlineAllianceContinentInfo*>* create(GenericAllianceCodeHistoricalKey key) override;
  void destroy(GenericAllianceCodeHistoricalKey key,
               std::vector<AirlineAllianceContinentInfo*>* t) override;

private:
  static AirlineAllianceContinentHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
}

