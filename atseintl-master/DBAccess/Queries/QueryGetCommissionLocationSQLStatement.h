#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCommissionLocation.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <typename QUERYCLASS>
class QueryGetCommissionLocationSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    // child
    VENDORC
    , ITEMNOC
    , CREATEDATETIMEC
    , ORDERNO
    , LOCTYPE
    , LOC
    , INCLEXCLIND
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
    this->Command("select c.VENDOR, c.ITEMNO, c.CREATEDATETIME, ORDERNO, LOCTYPE, LOC, INCLEXCLIND"
                  " from COMMISSIONLOCSEG c"
                  " LEFT OUTER JOIN COMMISSIONLOC p ON"
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

  static CommissionLocSegInfo* mapRow(Row* row)
  {
    CommissionLocSegInfo* info(new CommissionLocSegInfo);
    info->vendor() = row->getString(VENDORC);
    info->itemNo() = row->getLong(ITEMNOC);
    info->createDate() = row->getDate(CREATEDATETIMEC);
    if (!row->isNull(ORDERNO))
    {
     info->orderNo() = row->getLong(ORDERNO);
    }
    if (!row->isNull(LOCTYPE))
    {
      char type(' ');
      type = row->getChar(LOCTYPE);
      if (!row->isNull(LOC))
      {
        std::string location(row->getString(LOC));
        LocKey locKey;
        locKey.loc() = location;
        locKey.locType() = type;
        info->loc() = locKey;
      }
    }
    if (!row->isNull(INCLEXCLIND))
    {
      info->inclExclInd() = row->getChar(INCLEXCLIND);
    }
    return info;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <typename QUERYCLASS>
class QueryGetCommissionLocationHistoricalSQLStatement
  : public QueryGetCommissionLocationSQLStatement<QUERYCLASS>
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
