//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"

namespace tse
{
// class Row
class EmdInterlineAgreementInfo;

class QueryGetEmdInterlineAgreement: public tse::SQLQuery
{
private:
  template <typename Query>
  friend
  class DBAccessTestHelper;

  template <typename Query>
  friend
  class SQLQueryInitializerHelper;

  static
  void deinitialize() { _isInitialized = false; }

public:
  explicit QueryGetEmdInterlineAgreement(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL)
  {}

  QueryGetEmdInterlineAgreement(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement)
  {}

  virtual
  ~QueryGetEmdInterlineAgreement()
  {}

  virtual const char* getQueryName() const override;

  void findEmdInterlineAgreement(std::vector<EmdInterlineAgreementInfo*>& eiaList,
                                 const NationCode& country,
                                 const CrsCode& gds,
                                 const CarrierCode& carrier);

  static
  void initialize();

  const QueryGetEmdInterlineAgreement& operator=(const QueryGetEmdInterlineAgreement& rhs);

  const QueryGetEmdInterlineAgreement& operator=(const std::string& rhs);

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

} // tse

