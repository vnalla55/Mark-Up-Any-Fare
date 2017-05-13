#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/SpanishReferenceFareInfo.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <typename QUERYCLASS>
class QueryGetSpanishReferenceFareSQLStatement : public DBAccess::SQLStatement
{
public:
    enum ColumnIndexes
    {
      TKTCARRIER = 0,
      FARECARRIER,
      ORIGINAIRPORT,
      DESTINATIONAIRPORT,
      VIAPOINT1AIRPORT,
      VIAPOINT2AIRPORT,
      CREATEDATE,
      EXPIREDATE,
      EFFDATE,
      DISCDATE,
      LASTMODDATE,
      CURRENCYCODE,
      FAREAMOUNT,
      FAREAMOUNTNODEC,
      NUMBEROFCOLUMNS
    };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command (
        "select TKTCARRIER, FARECARRIER, ORIGINAIRPORT, DESTINATIONAIRPORT,"
        " VIAPOINT1AIRPORT, VIAPOINT2AIRPORT, CREATEDATE, EXPIREDATE, EFFDATE, DISCDATE,"
        " LASTMODDATE, CURRENCYCODE, FAREAMOUNT, FAREAMOUNTNODEC,"
        " DECODE(VIAPOINT1AIRPORT, ' ', 1, 0) + DECODE(VIAPOINT2AIRPORT, ' ', 1, 0) VIA");
    this->From(" =SPANISHREFERENCEFARE");
    this->Where(" TKTCARRIER = %1q and FARECARRIER = %2q"
                " and ORIGINAIRPORT = %3q and DESTINATIONAIRPORT = %4q"
                " and VALIDITYIND = 'Y'"
                " and %cd <= EXPIREDATE");
    this->OrderBy("VIA");

    // callback to adjust query
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static SpanishReferenceFareInfo* mapRow(Row* row)
  {
    SpanishReferenceFareInfo* srfi = new tse::SpanishReferenceFareInfo;
    using namespace tag;
    srfi->get<TktCarrier>() = row->getString(TKTCARRIER);
    srfi->get<FareCarrier>() = row->getString(FARECARRIER);
    srfi->get<OriginAirport>() = row->getString(ORIGINAIRPORT);
    srfi->get<DestinationAirport>() = row->getString(DESTINATIONAIRPORT);
    srfi->get<ViaPointOneAirport>() = row->getString(VIAPOINT1AIRPORT);
    srfi->get<ViaPointTwoAirport>() = row->getString(VIAPOINT2AIRPORT);
    srfi->get<CreateDate>() = row->getDate(CREATEDATE);
    srfi->get<ExpireDate>() = row->getDate(EXPIREDATE);
    srfi->get<EffDate>() = row->getDate(EFFDATE);
    srfi->get<DiscDate>() = row->getDate(DISCDATE);
    srfi->get<LastModDate>() = row->getDate(LASTMODDATE);
    srfi->get<SCurrencyCode>() = row->getString(CURRENCYCODE);
    srfi->get<FareAmountNoDec>() = row->getInt(FAREAMOUNTNODEC);
    srfi->get<FareAmount>() =
        QUERYCLASS::adjustDecimal(row->getInt(FAREAMOUNT), srfi->get<FareAmountNoDec>());

    return srfi;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <class QUERYCLASS>
class QueryGetSpanishReferenceFareHistoricalSQLStatement
    : public QueryGetSpanishReferenceFareSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where(" TKTCARRIER = %1q and FARECARRIER = %2q"
                " and ORIGINAIRPORT = %3q and DESTINATIONAIRPORT = %4q"
                " and VALIDITYIND = 'Y'"
                " and %5n <= EXPIREDATE"
                " and %6n >= CREATEDATE");
  }
};
} // tse

