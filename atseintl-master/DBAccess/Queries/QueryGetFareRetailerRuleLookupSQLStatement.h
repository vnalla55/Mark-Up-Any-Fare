#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/FareRetailerRuleLookupInfo.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <typename QUERYCLASS>
class QueryGetFareRetailerRuleLookupSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    FARERETAILERRULEID
    , SOURCEPSEUDOCITY
    , OWNERPSEUDOCITY
    , APPLICATIONIND
    , RULESEQNO
    , NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select FARERETAILERRULEID, SOURCEPSEUDOCITY, OWNERPSEUDOCITY, APPLICATIONIND, RULESEQNO "
                  "from FARERETAILERRULE frr "
                  "LEFT OUTER JOIN FAREFOCUSSECURITY sec ON sec.SECURITYITEMNO = frr.SECURITYITEMNO " 
                  "LEFT OUTER JOIN FAREFOCUSSECURITYDETAIL secdet ON sec.SECURITYITEMNO = secdet.SECURITYITEMNO "
                  "where frr.APPLICATIONIND = %1q and frr.SOURCEPSEUDOCITY = %2q and secdet.PSEUDOCITY = %3q and frr.STATUSCD = 'A' "
                  "and %cd <= sec.EXPIREDATE "
                  "and %cd <= frr.EXPIREDATE and %cd <= frr.DISCDATETIME ");

    if (DataManager::forceSortOrder())
      this->OrderBy(" APPLICATIONIND, SOURCEPSEUDOCITY, RULESEQNO ");

    // callback to adjust query
    adjustBaseSQL();

    ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static FareRetailerRuleLookupInfo* mapRow(Row* row, FareRetailerRuleLookupInfo* info)
  {
    if (info)
    {
      FareRetailerRuleLookupId frrl;
      if (!row->isNull(FARERETAILERRULEID))
      {
        frrl._fareRetailerRuleId = row->getLong(FARERETAILERRULEID);
      }
      if (!row->isNull(FARERETAILERRULEID))
      {
        frrl._ruleSeqNo = row->getLong(RULESEQNO);
      }
      info->fareRetailerRuleLookupIds().push_back(frrl);


    }
    return info;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <typename QUERYCLASS>
class QueryGetFareRetailerRuleLookupHistoricalSQLStatement
    : public QueryGetFareRetailerRuleLookupSQLStatement<QUERYCLASS>
{
  void adjustBaseSQL() override
  {

    this->Command("select FARERETAILERRULEID, SOURCEPSEUDOCITY, OWNERPSEUDOCITY, APPLICATIONIND, RULESEQNO "
                  "from FARERETAILERRULEH frrh "
                  "LEFT OUTER JOIN FAREFOCUSSECURITY sec ON sec.SECURITYITEMNO = frrh.SECURITYITEMNO "
                  "LEFT OUTER JOIN FAREFOCUSSECURITYDETAIL secdet ON sec.SECURITYITEMNO = secdet.SECURITYITEMNO "
                  "where frrh.APPLICATIONIND = %1q and frrh.SOURCEPSEUDOCITY = %2q and frrh.STATUSCD != 'P' and secdet.PSEUDOCITY = %3q "
                  "and %4n <= sec.EXPIREDATE and %5n >= sec.CREATEDATE "
                  "and %6n <= frrh.EXPIREDATE and %7n >= frrh.CREATEDATE "
                  "union all "
                  "select FARERETAILERRULEID, SOURCEPSEUDOCITY, OWNERPSEUDOCITY, APPLICATIONIND, RULESEQNO "
                  "from FARERETAILERRULE frrh "
                  "LEFT OUTER JOIN FAREFOCUSSECURITY sec ON sec.SECURITYITEMNO = frrh.SECURITYITEMNO "
                  "LEFT OUTER JOIN FAREFOCUSSECURITYDETAIL secdet ON sec.SECURITYITEMNO = secdet.SECURITYITEMNO "
                  "where frrh.APPLICATIONIND = %8q and frrh.SOURCEPSEUDOCITY = %9q and frrh.STATUSCD != 'P' and secdet.PSEUDOCITY = %10q "
                  "and %11n <= sec.EXPIREDATE  and %12n >= sec.CREATEDATE "
                  "and %13n <= frrh.EXPIREDATE and %14n >= frrh.CREATEDATE ");

    if (DataManager::forceSortOrder())
      this->OrderBy(" APPLICATIONIND, SOURCEPSEUDOCITY, RULESEQNO ");


  }

private:
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

} // tse

