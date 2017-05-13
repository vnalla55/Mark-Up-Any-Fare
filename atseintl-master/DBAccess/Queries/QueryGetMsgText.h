//----------------------------------------------------------------------------
//
//     File:           QueryGetMsgText.h
//     Description:    QueryGetMsgText
//     Created:        3/2/2006
//     Authors:        Mike Lillis
//
//     Updates:
//          07/26/2006  Quan Ta
//
//     � 2006, Sabre Inc.  All rights reserved.  This software/documentation
//     is the confidential and proprietary product of Sabre Inc. Any
//     unauthorized use, reproduction, or transfer of this software/documentation,
//     in any medium, or incorporation of this software/documentation into any
//     system or publication, is strictly prohibited
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/FareCalcConfigText.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetMsgText : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetMsgText(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetMsgText(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetMsgText() {};

  virtual const char* getQueryName() const override;

  void findMsgText(FareCalcConfigText::FCCTextMap& fccTextMap,
                   const Indicator userApplType,
                   const UserApplCode& userAppl,
                   const PseudoCityCode& pseudoCity);

  static void initialize();

  const QueryGetMsgText& operator=(const QueryGetMsgText& Another)
  {
    if (this != &Another)
      *((SQLQuery*)this) = (SQLQuery&)Another;

    return *this;
  };
  const QueryGetMsgText& operator=(const std::string& Another)
  {
    if (this != &Another)
      *((SQLQuery*)this) = Another;

    return *this;
  };

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
};

} // namespace tse

