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

#include "Common/Logger.h"
#include "DBAccess/BankIdentificationInfo.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{

class QueryGetBankIdentification : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetBankIdentification(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetBankIdentification(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetBankIdentification() {};
  virtual const char* getQueryName() const override;

  void findBankIdentificationInfo(std::vector<tse::BankIdentificationInfo*>& bins,
                                  const FopBinNumber& binNumber);

  static void initialize();

  const QueryGetBankIdentification& operator=(const QueryGetBankIdentification& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = (SQLQuery&)Another;
    }
    return *this;
  };
  const QueryGetBankIdentification& operator=(const std::string& Another)
  {
    if (this != &Another)
    {
      *((SQLQuery*)this) = Another;
    }
    return *this;
  };

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

class QueryGetBankIdentificationHistorical : public QueryGetBankIdentification
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetBankIdentificationHistorical(DBAdapter* dbAdapt)
    : QueryGetBankIdentification(dbAdapt, _baseSQL) {};
  virtual ~QueryGetBankIdentificationHistorical() {}
  virtual const char* getQueryName() const override;

  void findBankIdentificationInfo(std::vector<tse::BankIdentificationInfo*>& bins,
                                  const FopBinNumber& binNumber,
                                  const DateTime& startDate,
                                  const DateTime& endDate);

  static void initialize();

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};
}

