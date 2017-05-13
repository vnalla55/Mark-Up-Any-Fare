//----------------------------------------------------------------------------
//
//              File:           QueryGetHeaderMsgText.h
//              Description:    QueryGetHeaderMsgText
//              Created:        3/2/2006
//     Authors:         Mike Lillis
//
//              Updates:
//
//         � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//         and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//         or transfer of this software/documentation, in any medium, or incorporation of this
//         software/documentation into any system or publication, is strictly prohibited
//
//        ----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetHeaderMsgText : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetHeaderMsgText(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetHeaderMsgText(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetHeaderMsgText() {};

  virtual const char* getQueryName() const override;

  void findHeaderMsgText(std::vector<const std::string*>& lstSF, const uint64_t& itemNo);

  static void initialize();

  const QueryGetHeaderMsgText& operator=(const QueryGetHeaderMsgText& Another)
  {
    if (this != &Another)
      *((SQLQuery*)this) = (SQLQuery&)Another;

    return *this;
  };
  const QueryGetHeaderMsgText& operator=(const std::string& Another)
  {
    if (this != &Another)
      *((SQLQuery*)this) = Another;

    return *this;
  };

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetHeaderMsgText
} // namespace tse

