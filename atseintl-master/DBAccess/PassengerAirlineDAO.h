//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/HistoricalDataAccessObject.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{

class PassengerAirlineInfo;
class DeleteList;

typedef HashKey<CarrierCode> PassengerAirlineKey;

class PassengerAirlineDAO : public DataAccessObject<PassengerAirlineKey,
                                                    std::vector<PassengerAirlineInfo*> >
{
public:
  static PassengerAirlineDAO& instance();

  const PassengerAirlineInfo*
  get(DeleteList& del,
      const CarrierCode& airlineCode,
      const DateTime& travelDate,
      const DateTime& ticketDate);

  PassengerAirlineKey createKey(const PassengerAirlineInfo* info);
  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override;
  bool translateKey(const ObjectKey& objectKey, PassengerAirlineKey& key) const override;
  void translateKey(const PassengerAirlineKey& key, ObjectKey& objectKey) const override;

  const std::string& cacheClass() override { return _cacheClass; }
  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<PassengerAirlineDAO>;
  static DAOHelper<PassengerAirlineDAO> _helper;

  PassengerAirlineDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<PassengerAirlineKey, std::vector<PassengerAirlineInfo*> >(
        cacheSize, cacheType, 1)
  {
  }

  std::vector<PassengerAirlineInfo*>* create(PassengerAirlineKey key) override;
  void destroy(PassengerAirlineKey key, std::vector<PassengerAirlineInfo*>* paiList) override;

  void load() override;
  virtual size_t clear() override;

private:
  static PassengerAirlineDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

//-----------------------------------------------------------------------------
// HISTORICAL
//-----------------------------------------------------------------------------

typedef HashKey<CarrierCode, DateTime, DateTime> PassengerAirlineHistoricalKey;

class PassengerAirlineHistoricalDAO
    : public HistoricalDataAccessObject<PassengerAirlineHistoricalKey,
                                        std::vector<PassengerAirlineInfo*> >
{
public:
  static PassengerAirlineHistoricalDAO& instance();

  const PassengerAirlineInfo*
  get(DeleteList& del,
      const CarrierCode& airlineCode,
      const DateTime& travelDate,
      const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, PassengerAirlineHistoricalKey& key) const override;
  void translateKey(const PassengerAirlineHistoricalKey& key, ObjectKey& objectKey) const override;
  bool translateKey(const ObjectKey& objectKey,
                    PassengerAirlineHistoricalKey& key,
                    const DateTime ticketDate) const override;

  const std::string& cacheClass() override { return _cacheClass; }
  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<PassengerAirlineHistoricalDAO>;
  static DAOHelper<PassengerAirlineHistoricalDAO> _helper;

  PassengerAirlineHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<PassengerAirlineHistoricalKey,
                                 std::vector<PassengerAirlineInfo*> >(cacheSize, cacheType, 1)
  {
  }

  std::vector<PassengerAirlineInfo*>* create(PassengerAirlineHistoricalKey key) override;
  void
  destroy(PassengerAirlineHistoricalKey key, std::vector<PassengerAirlineInfo*>* paiList) override;

private:
  static PassengerAirlineHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

}

