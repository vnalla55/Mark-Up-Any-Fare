//-------------------------------------------------------------------
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/MaxPermittedAmountFareInfo.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
template <typename QUERYCLASS>
class QueryGetMaxPermittedAmountFareSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  { ORIGINAIRPORT = 0,
    ORIGINCITY,
    ORIGINNATION,
    DESTAIRPORT,
    DESTCITY,
    DESTNATION,
    SEQUENCE,
    LOC1,
    LOC1TYPE,
    LOC2,
    LOC2TYPE,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    LASTMODDATE,
    CURRENCYCODE,
    MARKETPRICE,
    MARKETPRICENODEC,
    NUMBEROFCOLUMNS };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select %1q ORIGINAIRPORT, %2q ORIGINCITY, %3q ORIGINNATION, "
                  " %4q DESTAIRPORT, %5q DESTCITY, %6q DESTNATION,"
                  " SEQUENCE, LOC1, LOC1TYPE, LOC2, LOC2TYPE,"
                  " CREATEDATE, EXPIREDATE, EFFDATE, DISCDATE, LASTMODDATE,"
                  " CURRENCYCODE, MARKETPRICE, MARKETPRICENODEC");
    this->From(" =SPANISH_MAX_PERMT_AMT");
    this->Where(" LOC1 = DECODE(LOC1TYPE, 'P', %7q, 'C', %8q, 'N', %9q, LOC1)"
                " and LOC2 = DECODE(LOC2TYPE, 'P', %10q, 'C', %11q, 'N', %12q, LOC2)"
                " and VALIDITYIND = 'Y'"
                " and %cd <= EXPIREDATE");
    this->OrderBy("SEQUENCE");

    // callback to adjust query
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static MaxPermittedAmountFareInfo* mapRow(Row* row)
  {
    MaxPermittedAmountFareInfo* mpafi = new tse::MaxPermittedAmountFareInfo;
    using namespace tag;
    mpafi->get<OriginAirport>() = row->getString(ORIGINAIRPORT);
    mpafi->get<OriginCity>() = row->getString(ORIGINCITY);
    mpafi->get<OriginNation>() = row->getString(ORIGINNATION);
    mpafi->get<DestAirport>() = row->getString(DESTAIRPORT);
    mpafi->get<DestCity>() = row->getString(DESTCITY);
    mpafi->get<DestNation>() = row->getString(DESTNATION);
    mpafi->get<SSequence>() = row->getInt(SEQUENCE);
    mpafi->get<Loc1>().loc() = row->getString(LOC1);
    mpafi->get<Loc1>().locType() = row->getChar(LOC1TYPE);
    mpafi->get<Loc2>().loc() = row->getString(LOC2);
    mpafi->get<Loc2>().locType() = row->getChar(LOC2TYPE);
    mpafi->get<CreateDate>() = row->getDate(CREATEDATE);
    mpafi->get<ExpireDate>() = row->getDate(EXPIREDATE);
    mpafi->get<EffDate>() = row->getDate(EFFDATE);
    mpafi->get<DiscDate>() = row->getDate(DISCDATE);
    mpafi->get<LastModDate>() = row->getDate(LASTMODDATE);
    mpafi->get<SCurrencyCode>() = row->getString(CURRENCYCODE);
    mpafi->get<MarketPriceNoDec>() = row->getInt(MARKETPRICENODEC);
    mpafi->get<MarketPrice>() =
        QUERYCLASS::adjustDecimal(row->getInt(MARKETPRICE), mpafi->get<MarketPriceNoDec>());

    return mpafi;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <class QUERYCLASS>
class QueryGetMaxPermittedAmountFareHistoricalSQLStatement
    : public QueryGetMaxPermittedAmountFareSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where(" LOC1 = DECODE(LOC1TYPE, 'P', %7q, 'C', %8q, 'N', %9q, LOC1)"
                " and LOC2 = DECODE(LOC2TYPE, 'P', %10q, 'C', %11q, 'N', %12q, LOC2)"
                " and VALIDITYIND = 'Y'"
                " and %13n <= EXPIREDATE"
                " and %14n >= CREATEDATE");
  }
};
} // tse
