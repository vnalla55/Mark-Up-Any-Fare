//----------------------------------------------------------------------------
//          File:           QueryGetCarrierCombSQLStatement.h
//          Description:    QueryGetCarrierCombSQLStatement
//          Created:        10/29/2007
//          Authors:        Mike Lillis
//
//          Updates:
//
//     � 2007, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCarrierComb.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetCarrierCombSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetCarrierCombSQLStatement() {};
  virtual ~QueryGetCarrierCombSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    CATEGORY,
    ORDERNO,
    TVLSECTSI,
    CARRIERCOMBAPPL,
    CARRIER1,
    CARRIER2,
    CARRIER3,
    CARRIER4,
    CARRIER5,
    CARRIER6,
    TVLSECOVERWATERIND,
    TVLSECINTLIND,
    TVLSECINOUTBOUNDIND,
    TVLSECCONSTIND,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select "
                  "r.VENDOR,r.ITEMNO,r.CREATEDATE,m.EXPIREDATE,r.CATEGORY,ORDERNO,TVLSECTSI,"
                  "CARRIERCOMBAPPL,"
                  "       CARRIER1,CARRIER2,CARRIER3,CARRIER4,CARRIER5,CARRIER6,"
                  "       TVLSECOVERWATERIND,TVLSECINTLIND,TVLSECINOUTBOUNDIND,"
                  "       TVLSECCONSTIND,LOC1TYPE,LOC1,LOC2TYPE,LOC2");
    this->From("=CARRIERCOMBSEQ r join =MINORCOMBSUBCAT m "
               "    on  r.VENDOR = m.VENDOR "
               "    and r.ITEMNO = m.ITEMNO "
               "    and r.CREATEDATE = m.CREATEDATE "
               "    and r.CATEGORY = m.CATEGORY ");
    this->Where("r.VENDOR = %1q  "
                "    and r.ITEMNO = %2n"
                "    and m.EXPIREDATE >= %cd");
    this->OrderBy("ORDERNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::CarrierCombination* mapRowToCarrierCombination(Row* row)
  {
    tse::CarrierCombination* carrierComb = new tse::CarrierCombination;

    carrierComb->vendor() = row->getString(VENDOR);
    carrierComb->itemNo() = row->getInt(ITEMNO);
    carrierComb->createDate() = row->getDate(CREATEDATE);
    carrierComb->expireDate() = row->getDate(EXPIREDATE);
    carrierComb->orderNo() = row->getInt(ORDERNO);
    carrierComb->tvlSectSi() = row->getInt(TVLSECTSI);
    carrierComb->carrierCombAppl() = row->getChar(CARRIERCOMBAPPL);
    carrierComb->carriers().push_back(row->getString(CARRIER1));
    carrierComb->carriers().push_back(row->getString(CARRIER2));
    carrierComb->carriers().push_back(row->getString(CARRIER3));
    carrierComb->carriers().push_back(row->getString(CARRIER4));
    carrierComb->carriers().push_back(row->getString(CARRIER5));
    carrierComb->carriers().push_back(row->getString(CARRIER6));
    carrierComb->tvlSecOverwaterInd() = row->getChar(TVLSECOVERWATERIND);
    carrierComb->tvlSecIntlInd() = row->getChar(TVLSECINTLIND);
    carrierComb->tvlSecInOutboundInd() = row->getChar(TVLSECINOUTBOUNDIND);
    carrierComb->tvlSecConstInd() = row->getChar(TVLSECCONSTIND);
    carrierComb->loc1Type() = LocType(row->getString(LOC1TYPE)[0]);
    carrierComb->loc1() = row->getString(LOC1);
    carrierComb->loc2Type() = LocType(row->getString(LOC2TYPE)[0]);
    carrierComb->loc2() = row->getString(LOC2);

    return carrierComb;
  } // mapRowToCarrierCombination()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetCarrierCombHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetCarrierCombHistoricalSQLStatement : public QueryGetCarrierCombSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("r.VENDOR = %1q"
                "  and r.ITEMNO = %2n"
                "  and %3n <= m.EXPIREDATE"
                "  and %4n >= r.CREATEDATE");
  }
}; // class QueryGetCarrierCombHistoricalSQLStatement
} // tse
