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

class EmdInterlineAgreementInfo;
class DeleteList;

typedef HashKey<NationCode, CrsCode, CarrierCode> EmdInterlineAgreementKey;

class EmdInterlineAgreementDAO
    : public DataAccessObject<EmdInterlineAgreementKey,
                              std::vector<EmdInterlineAgreementInfo*> >
{
public:
  static EmdInterlineAgreementDAO& instance();

  const std::vector<EmdInterlineAgreementInfo*>&
  get(DeleteList& del, const NationCode& country, const CrsCode& gds, const CarrierCode& carrier,
      const DateTime& ticketDate);

  EmdInterlineAgreementKey createKey(const EmdInterlineAgreementInfo* info);
  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override;
  bool translateKey(const ObjectKey& objectKey, EmdInterlineAgreementKey& key) const override;
  void translateKey(const EmdInterlineAgreementKey& key, ObjectKey& objectKey) const override;

  const std::string& cacheClass() override { return _cacheClass; }
  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<EmdInterlineAgreementDAO>;
  static DAOHelper<EmdInterlineAgreementDAO> _helper;

  EmdInterlineAgreementDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<EmdInterlineAgreementKey, std::vector<EmdInterlineAgreementInfo*> >(
          cacheSize, cacheType, 1)
  {
  }

  std::vector<EmdInterlineAgreementInfo*>* create(EmdInterlineAgreementKey key) override;
  void
  destroy(EmdInterlineAgreementKey key, std::vector<EmdInterlineAgreementInfo*>* eiaList) override;

  void load() override;
  virtual size_t clear() override;

private:
  static EmdInterlineAgreementDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

//-----------------------------------------------------------------------------
// HISTORICAL
//-----------------------------------------------------------------------------

typedef HashKey<NationCode, CrsCode, CarrierCode, DateTime, DateTime>
    EmdInterlineAgreementHistoricalKey;

class EmdInterlineAgreementHistoricalDAO
    : public HistoricalDataAccessObject<EmdInterlineAgreementHistoricalKey,
                                        std::vector<EmdInterlineAgreementInfo*> >
{
public:
  static EmdInterlineAgreementHistoricalDAO& instance();

  const std::vector<EmdInterlineAgreementInfo*>&
  get(DeleteList& del, const NationCode& country, const CrsCode& gds, const CarrierCode& carrier,
      const DateTime& ticketDate);

  EmdInterlineAgreementHistoricalKey createKey(const EmdInterlineAgreementInfo* info);
  bool
  translateKey(const ObjectKey& objectKey, EmdInterlineAgreementHistoricalKey& key) const override;
  void
  translateKey(const EmdInterlineAgreementHistoricalKey& key, ObjectKey& objectKey) const override;
  bool translateKey(const ObjectKey& objectKey,
                    EmdInterlineAgreementHistoricalKey& key,
                    const DateTime ticketDate) const override;

  const std::string& cacheClass() override { return _cacheClass; }
  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<EmdInterlineAgreementHistoricalDAO>;
  static DAOHelper<EmdInterlineAgreementHistoricalDAO> _helper;

  EmdInterlineAgreementHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<EmdInterlineAgreementHistoricalKey,
                                 std::vector<EmdInterlineAgreementInfo*> >(cacheSize, cacheType, 1)
  {
  }

  std::vector<EmdInterlineAgreementInfo*>* create(EmdInterlineAgreementHistoricalKey key) override;
  void destroy(EmdInterlineAgreementHistoricalKey key,
               std::vector<EmdInterlineAgreementInfo*>* eiaList) override;

private:
  static EmdInterlineAgreementHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

}

