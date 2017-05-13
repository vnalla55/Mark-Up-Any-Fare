//----------------------------------------------------------------------------
//
//     File:           ORACLEStatementHelperImpl.cpp
//     Description:    ORACLEStatementHelperImpl
//     Created:        04/20/2009
//     Authors:        Andrew Ahmad
//
//     Updates:
//
//     Copyright @2009, Sabre Inc.  All rights reserved.
//         This software/documentation is the confidential and proprietary
//         product of Sabre Inc.
//         Any unauthorized use, reproduction, or transfer of this
//         software/documentation, in any medium, or incorporation of this
//         software/documentation into any system or publication,
//         is strictly prohibited.
//
//----------------------------------------------------------------------------

#include "DBAccess/ORACLEStatementHelperImpl.h"

#include "Common/DateTime.h"
#include "DBAccess/SQLStatementHelperState.h"

#include <cstdio>
#include <sstream>

namespace DBAccess
{
void
ORACLEStatementHelperImpl::generateJoinString(const SQLStatementHelperState& state,
                                              const std::string& table1,
                                              const std::string& alias1,
                                              const std::string& joinType,
                                              const std::string& table2,
                                              const std::string& alias2,
                                              const std::vector<std::string>& joinFields,
                                              std::string& result) const
{
  result = table1 + " " + alias1 + " " + joinType + " " + table2 + " " + alias2 + " ON ";

  std::vector<std::string>::const_iterator iterStart = joinFields.begin();
  std::vector<std::string>::const_iterator iter = joinFields.begin();
  std::vector<std::string>::const_iterator iterEnd = joinFields.end();

  for (; iter != iterEnd; ++iter)
  {
    if (iter != iterStart)
    {
      result += " AND ";
    }

    // Note: We may need to use the full table name to qualify
    //       each join field if table alias is blank
    //
    if (!alias1.empty())
    {
      result += alias1 + ".";
    }
    else
    {
      result += table1 + ".";
    }

    result += *iter + " = ";

    if (!alias2.empty())
    {
      result += alias2 + ".";
    }
    else
    {
      result += table2 + ".";
    }

    result += *iter + " ";
  }
}

void
ORACLEStatementHelperImpl::generateLimitString(const SQLStatementHelperState& state,
                                               uint32_t limit,
                                               std::string& result) const
{
  std::ostringstream oss;
  oss << " ROWNUM <= ";
  oss << limit;
  result += oss.str();
}

void
ORACLEStatementHelperImpl::generateLimitString(const SQLStatementHelperState& state,
                                               const std::string& limit,
                                               std::string& result) const
{
  std::ostringstream oss;
  oss << " ROWNUM <= ";
  oss << limit;
  result += oss.str();
}

void
ORACLEStatementHelperImpl::formatDateTimeString(const tse::DateTime& dt, std::string& result) const
{
  if (dt.isInfinity())
  {
    result = "'9999-12-31:23:59:59.999000'";
  }
  else if (dt.isNegInfinity())
  {
    result = "'0001-01-01:00:00:00.000000'";
  }
  else
  {
    char buf[32];
    snprintf(buf,
             sizeof(buf),
             "'%.4d-%.2d-%.2d:%.2d:%.2d:%.2d'",
             (int)dt.year(),
             (int)dt.month(),
             (int)dt.day(),
             (int)dt.hours(),
             (int)dt.minutes(),
             (int)dt.seconds());
    if (dt.fractionalSeconds())
    {
      snprintf(buf + 20, sizeof(buf) - 20, ".%.6ld'", (long)dt.fractionalSeconds());
    }
    result = buf;
  }
}

void
ORACLEStatementHelperImpl::formatDateString(const tse::DateTime& dt, std::string& result) const
{
  if (dt.isInfinity())
  {
    result = "'9999-12-31:23:59:59'";
  }
  else if (dt.isNegInfinity())
  {
    result = "'0001-01-01:00:00:00'";
  }
  else
  {
    char buf[32];
    snprintf(buf,
             sizeof(buf),
             "'%.4d-%.2d-%.2d:%.2d:%.2d:%.2d'",
             (int)dt.year(),
             (int)dt.month(),
             (int)dt.day(),
             0,
             0,
             0);

    result = buf;
  }
}

void
ORACLEStatementHelperImpl::constructStatement(const SQLStatementHelperState& state,
                                              const std::string& command,
                                              const std::string& from,
                                              const std::string& where,
                                              const std::string& groupby,
                                              const std::string& orderby,
                                              const std::string& limit,
                                              std::string& result) const
{
  std::string sLimit = limit;
  if (state.hasLimit())
  {
    if (state.hasWhere())
      sLimit = " AND " + limit;
    else
      sLimit = " WHERE " + limit;
  }
  result = command + from + where + sLimit + groupby + orderby;

  std::string::size_type pos = result.find("''");
  while (pos != std::string::npos)
  {
    result.replace(pos, 2, "' '");
    pos = result.find("''");
  }
}

bool
ORACLEStatementHelperImpl::checkStateValidity(const SQLStatementHelperState& state,
                                              std::string& errorMessage) const
{
  if (state.hasOrderBy() && state.hasLimit())
  {
    errorMessage = "SQL Statement not permitted: ORDER BY used together with ROWNUM <= x (aka: "
                   "LIMIT) does not produce the expected result with this database type (ORACLE)";
    return false;
  }
  return true;
}
}
