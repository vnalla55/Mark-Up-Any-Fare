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

class AirlineInterlineAgreementInfo;
class DeleteList;

typedef HashKey<NationCode, CrsCode, CarrierCode> AirlineInterlineAgreementKey;

class AirlineInterlineAgreementDAO
    : public DataAccessObject<AirlineInterlineAgreementKey,
                              std::vector<AirlineInterlineAgreementInfo*> >
{
public:
  static AirlineInterlineAgreementDAO& instance();

  const std::vector<AirlineInterlineAgreementInfo*>&
  get(DeleteList& del, const NationCode& country, const CrsCode& gds, const CarrierCode& carrier,
      const DateTime& ticketDate);

  AirlineInterlineAgreementKey createKey(const AirlineInterlineAgreementInfo* info);
  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override;
  bool translateKey(const ObjectKey& objectKey, AirlineInterlineAgreementKey& key) const override;
  void translateKey(const AirlineInterlineAgreementKey& key, ObjectKey& objectKey) const override;

  const std::string& cacheClass() override { return _cacheClass; }
  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<AirlineInterlineAgreementDAO>;
  static DAOHelper<AirlineInterlineAgreementDAO> _helper;

  AirlineInterlineAgreementDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<AirlineInterlineAgreementKey, std::vector<AirlineInterlineAgreementInfo*> >(
          cacheSize, cacheType, 1)
  {
  }

  std::vector<AirlineInterlineAgreementInfo*>* create(AirlineInterlineAgreementKey key) override;
  void destroy(AirlineInterlineAgreementKey key,
               std::vector<AirlineInterlineAgreementInfo*>* aiaList) override;

  void load() override;
  virtual size_t clear() override;

private:
  static AirlineInterlineAgreementDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

//-----------------------------------------------------------------------------
// HISTORICAL
//-----------------------------------------------------------------------------

typedef HashKey<NationCode, CrsCode, CarrierCode, DateTime, DateTime>
    AirlineInterlineAgreementHistoricalKey;

class AirlineInterlineAgreementHistoricalDAO
    : public HistoricalDataAccessObject<AirlineInterlineAgreementHistoricalKey,
                                        std::vector<AirlineInterlineAgreementInfo*> >
{
public:
  static AirlineInterlineAgreementHistoricalDAO& instance();

  const std::vector<AirlineInterlineAgreementInfo*>&
  get(DeleteList& del, const NationCode& country, const CrsCode& gds, const CarrierCode& carrier,
      const DateTime& ticketDate);

  AirlineInterlineAgreementHistoricalKey createKey(const AirlineInterlineAgreementInfo* info);
  bool translateKey(const ObjectKey& objectKey,
                    AirlineInterlineAgreementHistoricalKey& key) const override;
  void translateKey(const AirlineInterlineAgreementHistoricalKey& key,
                    ObjectKey& objectKey) const override;
  bool translateKey(const ObjectKey& objectKey,
                    AirlineInterlineAgreementHistoricalKey& key,
                    const DateTime ticketDate) const override;

  const std::string& cacheClass() override { return _cacheClass; }
  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<AirlineInterlineAgreementHistoricalDAO>;
  static DAOHelper<AirlineInterlineAgreementHistoricalDAO> _helper;

  AirlineInterlineAgreementHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<AirlineInterlineAgreementHistoricalKey,
                                 std::vector<AirlineInterlineAgreementInfo*> >(cacheSize, cacheType, 1)
  {
  }

  std::vector<AirlineInterlineAgreementInfo*>*
  create(AirlineInterlineAgreementHistoricalKey key) override;
  void destroy(AirlineInterlineAgreementHistoricalKey key,
               std::vector<AirlineInterlineAgreementInfo*>* aiaList) override;

private:
  static AirlineInterlineAgreementHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

}

