//----------------------------------------------------------------------------
//
//              File:           QueryGetHeaderMsg.h
//              Description:    QueryGetHeaderMsg
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
#include "DBAccess/FDHeaderMsg.h"
#include "DBAccess/SQLQuery.h"

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

namespace tse
{


class QueryGetHeaderMsg : public tse::SQLQuery
{
private:
  template <typename Query>
  friend class DBAccessTestHelper;
  template <typename Query>
  friend class SQLQueryInitializerHelper;
  static void deinitialize() { _isInitialized = false; }

public:
  QueryGetHeaderMsg(DBAdapter* dbAdapt) : SQLQuery(dbAdapt, _baseSQL) {};
  QueryGetHeaderMsg(DBAdapter* dbAdapt, const std::string& sqlStatement)
    : SQLQuery(dbAdapt, sqlStatement) {};
  virtual ~QueryGetHeaderMsg() {};

  virtual const char* getQueryName() const override;

  void findHeaderMsgs(std::vector<const tse::FDHeaderMsg*>& lstSF,
                      const PseudoCityCode& pseudoCityCode,
                      const PseudoCityType& pseudoCityType,
                      const Indicator& userApplType,
                      const std::string& userAppl,
                      const TJRGroup& tjrGroup,
                      const InclusionCode& inclusionCode);

  static void initialize();

  const QueryGetHeaderMsg& operator=(const QueryGetHeaderMsg& Another)
  {
    if (this != &Another)
      *((SQLQuery*)this) = (SQLQuery&)Another;

    return *this;
  };
  const QueryGetHeaderMsg& operator=(const std::string& Another)
  {
    if (this != &Another)
      *((SQLQuery*)this) = Another;

    return *this;
  };

private:
  static log4cxx::LoggerPtr _logger;
  static std::string _baseSQL;
  static bool _isInitialized;
}; // class QueryGetHeaderMsg
} // namespace tse

