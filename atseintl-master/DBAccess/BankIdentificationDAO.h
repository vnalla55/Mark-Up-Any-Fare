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

class BankIdentificationInfo;
class DeleteList;

typedef HashKey<FopBinNumber> BinKey;

class BankIdentificationDAO : public DataAccessObject<BinKey, std::vector<BankIdentificationInfo*> >
{
public:
  static BankIdentificationDAO& instance();

  const BankIdentificationInfo* get(DeleteList& del,
                                    const FopBinNumber& binNumber,
                                    const DateTime& date,
                                    const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, BinKey& key) const override
  {
    return (key.initialized = (objectKey.getValue("BANKIDENTIFICATIONNBR", key._a)));
  }

  BinKey createKey(BankIdentificationInfo* info);

  void translateKey(const BinKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("BANKIDENTIFICATIONNBR", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<BankIdentificationInfo, BankIdentificationDAO>(flatKey, objectKey)
        .success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<BankIdentificationDAO>;
  static DAOHelper<BankIdentificationDAO> _helper;
  BankIdentificationDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<BinKey, std::vector<BankIdentificationInfo*> >(cacheSize, cacheType)
  {
  }
  std::vector<BankIdentificationInfo*>* create(BinKey key) override;
  void destroy(BinKey key, std::vector<BankIdentificationInfo*>* recs) override;

  void load() override;

private:
  static BankIdentificationDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

typedef HashKey<FopBinNumber, DateTime, DateTime> BinHistoricalKey;

class BankIdentificationHistoricalDAO
    : public HistoricalDataAccessObject<BinHistoricalKey, std::vector<BankIdentificationInfo*> >
{
public:
  static BankIdentificationHistoricalDAO& instance();

  const BankIdentificationInfo* get(DeleteList& del,
                                    const FopBinNumber& binNumber,
                                    const DateTime& date,
                                    const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, BinHistoricalKey& key) const override
  {
    key.initialized = objectKey.getValue("BANKIDENTIFICATIONNBR", key._a);
    if (key.initialized && (_cacheBy != DAOUtils::NODATES))
    {
      key.initialized =
          objectKey.getValue("STARTDATE", key._b) && objectKey.getValue("ENDDATE", key._c);
    }
    return key.initialized;
  }

  bool
  translateKey(const ObjectKey& objectKey, BinHistoricalKey& key, const DateTime ticketDate) const
      override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
    return key.initialized = objectKey.getValue("BANKIDENTIFICATIONNBR", key._a);
  }

  void setKeyDateRange(BinHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<BankIdentificationHistoricalDAO>;
  static DAOHelper<BankIdentificationHistoricalDAO> _helper;
  BankIdentificationHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<BinHistoricalKey, std::vector<BankIdentificationInfo*> >(cacheSize,
                                                                                          cacheType)
  {
  }
  std::vector<BankIdentificationInfo*>* create(BinHistoricalKey key) override;
  void destroy(BinHistoricalKey key, std::vector<BankIdentificationInfo*>* t) override;

private:
  static BankIdentificationHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
}

