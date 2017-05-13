#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCommissionTvlDate.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <typename QUERYCLASS>
class QueryGetCommissionTvlDateSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    // child
    VENDORC
    , ITEMNOC
    , CREATEDATETIMEC
    , ORDERNO
    , FIRSTTRAVELDATETIME
    , ENDTRAVELDATETIME
    // parent
    , VENDORP
    , ITEMNOP
    , CREATEDATETIMEP
    , VALIDITYIND
    , NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    // child
    // parent
    this->Command("select c.VENDOR, c.ITEMNO, c.CREATEDATETIME, ORDERNO,"
                  " FIRSTTRAVELDATETIME, ENDTRAVELDATETIME"
                  " from COMMISSIONTRAVELDATESEG c"
                  " LEFT OUTER JOIN COMMISSIONTRAVELDATE p ON"
                  " p.VENDOR = c.VENDOR and p.ITEMNO = c.ITEMNO and p.CREATEDATETIME = c.CREATEDATETIME ");

    this->Where(" p.VENDOR = %1q and p.ITEMNO = %2n and VALIDITYIND = 'Y'");

    this->OrderBy(" p.CREATEDATETIME, ORDERNO");

    // callback to adjust query
    adjustBaseSQL();

    ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static CommissionTravelDatesSegInfo* mapRow(Row* row)
  {
    CommissionTravelDatesSegInfo* info(new CommissionTravelDatesSegInfo);
    info->vendor() = row->getString(VENDORC);
    info->itemNo() = row->getLong(ITEMNOC);
    info->createDate() = row->getDate(CREATEDATETIMEC);
    if (!row->isNull(ORDERNO))
    {
     info->orderNo() = row->getLong(ORDERNO);
    }
    if (!row->isNull(FIRSTTRAVELDATETIME))
    {
      info->firstTravelDate() = row->getDate(FIRSTTRAVELDATETIME);
    }
    if (!row->isNull(ENDTRAVELDATETIME))
    {
      info->endTravelDate() = row->getDate(ENDTRAVELDATETIME);
    }
    return info;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <typename QUERYCLASS>
class QueryGetCommissionTvlDateHistoricalSQLStatement
  : public QueryGetCommissionTvlDateSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where(" p.VENDOR = %1q and p.ITEMNO = %2n and VALIDITYIND = 'Y'"
                " and %3n >= p.CREATEDATETIME");

    this->OrderBy(" p.CREATEDATETIME, ORDERNO");
  }

  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

} // tse
