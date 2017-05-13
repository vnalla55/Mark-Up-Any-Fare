#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/FareFocusLookupInfo.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <typename QUERYCLASS>
class QueryGetFareFocusLookupSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    FAREFOCUSRULEID
    , NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select distinct FAREFOCUSRULEID "
                  "from FAREFOCUSRULE ffrule "
                  "LEFT OUTER JOIN FAREFOCUSSECURITY sec ON sec.SECURITYITEMNO = ffrule.SECURITYITEMNO " 
                  "LEFT OUTER JOIN FAREFOCUSSECURITYDETAIL secdet ON sec.SECURITYITEMNO = secdet.SECURITYITEMNO "
                  "where PSEUDOCITY = %1q and STATUSCD = 'A' and INHIBITCD = 'N' "
                  "and %cd <= sec.EXPIREDATE "
                  "and %cd <= ffrule.EXPIREDATE and %cd <= ffrule.DISCDATETIME "
                  "order by FAREFOCUSRULEID");

    // callback to adjust query
    adjustBaseSQL();

    ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static FareFocusLookupInfo* mapRow(Row* row, FareFocusLookupInfo* info)
  {
    if (info)
    {
      if (!row->isNull(FAREFOCUSRULEID))
      {
        info->fareFocusRuleIds().push_back(row->getLong(FAREFOCUSRULEID));
      }
    }
    return info;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <typename QUERYCLASS>
class QueryGetFareFocusLookupHistoricalSQLStatement
    : public QueryGetFareFocusLookupSQLStatement<QUERYCLASS>
{
  void adjustBaseSQL() override
  {
    this->Command("select distinct FAREFOCUSRULEID "
                  "from FAREFOCUSRULEH ffruleh "
                  "LEFT OUTER JOIN FAREFOCUSSECURITY sec ON sec.SECURITYITEMNO = ffruleh.SECURITYITEMNO "
                  "LEFT OUTER JOIN FAREFOCUSSECURITYDETAIL secdet ON sec.SECURITYITEMNO = secdet.SECURITYITEMNO "
                  "where PSEUDOCITY = %1q and STATUSCD != 'P' and INHIBITCD = 'N' "
                  "and %2n <= sec.EXPIREDATE and %3n >= sec.CREATEDATE "
                  "and %4n <= ffruleh.EXPIREDATE and %5n >= ffruleh.CREATEDATE "
                  "union all "
                  "select distinct FAREFOCUSRULEID "
                  "from FAREFOCUSRULE ffrule "
                  "LEFT OUTER JOIN FAREFOCUSSECURITY sec ON sec.SECURITYITEMNO = ffrule.SECURITYITEMNO "
                  "LEFT OUTER JOIN FAREFOCUSSECURITYDETAIL secdet ON sec.SECURITYITEMNO = secdet.SECURITYITEMNO "
                  "where PSEUDOCITY = %6q and STATUSCD != 'P' and INHIBITCD = 'N' "
                  "and %7n <= sec.EXPIREDATE  and %8n >= sec.CREATEDATE "
                  "and %9n <= ffrule.EXPIREDATE and %10n >= ffrule.CREATEDATE");

  }

private:
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

} // tse

