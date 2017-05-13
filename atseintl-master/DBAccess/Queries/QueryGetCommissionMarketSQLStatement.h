#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCommissionMarket.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <typename QUERYCLASS>
class QueryGetCommissionMarketSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    // child
    VENDORC
    , ITEMNOC
    , CREATEDATETIMEC
    , ORDERNO
    , ORIGLOC
    , ORIGLOCTYPE
    , DESTLOCTYPE
    , DESTLOC
    , BIDIRECTIONAL
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
    this->Command("select c.VENDOR, c.ITEMNO, c.CREATEDATETIME, ORDERNO, ORIGLOC, ORIGLOCTYPE,"
                  " DESTLOCTYPE, DESTLOC, BIDIRECTIONAL, INCLEXCLIND"
                  " from COMMISSIONMARKETSEG c"
                  " LEFT OUTER JOIN COMMISSIONMARKET p ON"
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

  static CommissionMarketSegInfo* mapRow(Row* row)
  {
    CommissionMarketSegInfo* info(new CommissionMarketSegInfo);
    info->vendor() = row->getString(VENDORC);
    info->itemNo() = row->getLong(ITEMNOC);
    info->createDate() = row->getDate(CREATEDATETIMEC);
    if (!row->isNull(ORDERNO))
    {
     info->orderNo() = row->getLong(ORDERNO);
    }
    if (!row->isNull(ORIGLOCTYPE))
    {
      char type(' ');
      type = row->getChar(ORIGLOCTYPE);
      if (!row->isNull(ORIGLOC))
      {
        std::string location(row->getString(ORIGLOC));
        LocKey locKey;
        locKey.loc() = location;
        locKey.locType() = type;
        info->origin() = locKey;
      }
    }
    if (!row->isNull(DESTLOCTYPE))
    {
      char type(' ');
      type = row->getChar(DESTLOCTYPE);
      if (!row->isNull(DESTLOC))
      {
        std::string location(row->getString(DESTLOC));
        LocKey locKey;
        locKey.loc() = location;
        locKey.locType() = type;
        info->destination() = locKey;
      }
    }
    if (!row->isNull(BIDIRECTIONAL))
    {
      info->bidirectional() = row->getChar(BIDIRECTIONAL);
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
class QueryGetCommissionMarketHistoricalSQLStatement
  : public QueryGetCommissionMarketSQLStatement<QUERYCLASS>
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
