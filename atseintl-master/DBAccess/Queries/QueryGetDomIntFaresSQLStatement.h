//----------------------------------------------------------------------------
//          File:           QueryGetDomIntFaresSQLStatement.h
//          Description:    QueryGetDomIntFaresSQLStatement
//          Created:        11/01/2007
//          Authors:        Mike Lillis
//
//          Updates:
//
//     (C) 2007, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetDomIntFares.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetDomFaresSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetDomFaresSQLStatement() {};
  virtual ~QueryGetDomFaresSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    CARRIER,
    FARETARIFF,
    CUR,
    FARECLASS,
    SEQNO,
    LINKNO,
    MARKET1,
    MARKET2,
    CREATEDATE,
    EFFDATE,
    DISCDATE,
    EXPIREDATE,
    LASTMODDATE,
    FAREAMT,
    NODEC,
    FOOTNOTE1,
    FOOTNOTE2,
    OWRT,
    RULE,
    ROUTING,
    DIRECTIONALITY,
    GLOBALDIR,
    INHIBIT,
    CHANGETAG1,
    CHANGETAG2,
    CHANGETAG3,
    CHANGETAG4,
    CHANGETAG5,
    CHANGETAG11,
    BATCHCI,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,CARRIER,FARETARIFF,CUR,FARECLASS,SEQNO,LINKNO,"
                  "       MARKET1,MARKET2,CREATEDATE,EFFDATE,DISCDATE,EXPIREDATE,"
                  "       LASTMODDATE,FAREAMT,NODEC,FOOTNOTE1,FOOTNOTE2,OWRT,RULE,"
                  "       ROUTING,DIRECTIONALITY,GLOBALDIR,INHIBIT,CHANGETAG1,"
                  "       CHANGETAG2,CHANGETAG3,CHANGETAG4,CHANGETAG5,CHANGETAG11,"
                  "       BATCHCI ");
    this->From("=DOMESTICFARE");
    this->Where("MARKET1 = %1q"
                "    and MARKET2 = %2q"
                "    and CARRIER = %3q"
                "    and VALIDITYIND = 'Y'"
                "    and %cd <= DISCDATE"
                "    and %cd <= EXPIREDATE"
                "    and EFFDATE <= DISCDATE");
    this->OrderBy("VENDOR,CARRIER,FARETARIFF,CUR,FARECLASS,SEQNO,LINKNO,CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static FareInfo* mapRowToFareInfo(Row* row)
  {
    tse::FareInfo* f = new tse::FareInfo;

    f->_vendor = row->getString(VENDOR);
    f->_carrier = row->getString(CARRIER);
    f->_fareTariff = row->getInt(FARETARIFF);
    f->_currency = row->getString(CUR);
    f->_fareClass = row->getString(FARECLASS);
    f->_sequenceNumber = row->getLong(SEQNO);
    f->_linkNumber = row->getInt(LINKNO);
    f->_market1 = row->getString(MARKET1);
    f->_market2 = row->getString(MARKET2);

    f->createDate() = row->getDate(CREATEDATE);
    f->expireDate() = row->getDate(EXPIREDATE);
    f->effDate() = row->getDate(EFFDATE);
    f->discDate() = row->getDate(DISCDATE);
    f->_lastModDate = row->getDate(LASTMODDATE);

    // workaround for a Content bug where the expire date is being
    // set forward to after the disc date
    if (f->discDate().isValid() && f->discDate() < f->expireDate())
    {
      DateTime discDate = f->discDate().addDays(1);
      if (discDate < f->expireDate())
        f->expireDate() = discDate.subtractSeconds(1);
    }

    f->_noDec = row->getInt(NODEC);
    f->_fareAmount = QUERYCLASS::adjustDecimal(row->getLong(FAREAMT), f->_noDec);
    f->_originalFareAmount = f->_fareAmount;

    f->_footnote1 = row->getString(FOOTNOTE1);
    f->_footnote2 = row->getString(FOOTNOTE2);
    f->_routingNumber = row->getString(ROUTING);
    f->_ruleNumber = row->getString(RULE);

    std::string dir = row->getString(DIRECTIONALITY);
    if (UNLIKELY(dir == "F"))
      f->_directionality = FROM;
    else if (UNLIKELY(dir == "T"))
      f->_directionality = TO;
    else if (LIKELY(dir.empty() || dir == " " || dir == "B"))
      f->_directionality = BOTH;

    std::string gd = row->getString(GLOBALDIR);
    strToGlobalDirection(f->_globalDirection, gd);

    f->_owrt = row->getString(OWRT)[0];
    if (f->_owrt == ROUND_TRIP_MAYNOT_BE_HALVED)
      f->_fareAmount = f->_fareAmount / 2;

    f->_inhibit = row->getChar(INHIBIT);

    f->_increasedFareAmtTag = row->getChar(CHANGETAG1);
    f->_reducedFareAmtTag = row->getChar(CHANGETAG2);
    f->_footnoteTag = row->getChar(CHANGETAG3);
    f->_routingTag = row->getChar(CHANGETAG4);
    f->_mpmTag = row->getChar(CHANGETAG5);
    f->_effectiveDateTag = 'N';
    f->_currencyCodeTag = 'N';
    f->_ruleTag = row->getChar(CHANGETAG11);

    // Content doesn't populate directionality for domestic fares,
    // instead, it's encoded in the footnote fields.
    if (f->_footnote1 == "F")
    {
      f->_directionality = FROM;

      if (f->_vendor != ATPCO_VENDOR_CODE)
        f->_footnote1 = "";
    }
    else if (f->_footnote1 == "T")
    {
      f->_directionality = TO;

      if (f->_vendor != ATPCO_VENDOR_CODE)
        f->_footnote1 = "";
    }
    if (f->_footnote2 == "F")
    {
      if (f->_directionality == TO)
        f->_directionality = BOTH;
      else
        f->_directionality = FROM;

      if (f->_vendor != ATPCO_VENDOR_CODE)
        f->_footnote2 = "";
    }
    else if (f->_footnote2 == "T")
    {
      if (f->_directionality == FROM)
        f->_directionality = BOTH;
      else
        f->_directionality = TO;

      if (f->_vendor != ATPCO_VENDOR_CODE)
        f->_footnote2 = "";
    }
    if (f->_footnote1.empty())
    {
      f->_footnote1 = f->_footnote2;
      f->_footnote2 = "";
    }

    f->vendorFWS() = (BATCHCI_FROM_VENDR_FWS == row->getString(BATCHCI));

    return f;
  } // mapRowToFareInfo()

protected:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetDomFaresHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetDomFaresHistoricalSQLStatement : public QueryGetDomFaresSQLStatement<QUERYCLASS>
{

protected:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("("
                             "select VENDOR,CARRIER,FARETARIFF,CUR,FARECLASS,SEQNO,LINKNO,"
                             "MARKET1,MARKET2,CREATEDATE,EFFDATE,DISCDATE,EXPIREDATE,"
                             "LASTMODDATE,FAREAMT,NODEC,FOOTNOTE1,FOOTNOTE2,OWRT,RULE,"
                             "ROUTING,DIRECTIONALITY,GLOBALDIR,INHIBIT,CHANGETAG1,"
                             "CHANGETAG2,CHANGETAG3,CHANGETAG4,CHANGETAG5,CHANGETAG11,"
                             "BATCHCI ");
    partialStatement.From("=DOMESTICFAREH");
    partialStatement.Where("MARKET1 = %1q"
                           " and MARKET2 = %2q"
                           " and CARRIER = %3q"
                           " and EFFDATE < DISCDATE"
                           " and VALIDITYIND = 'Y'"
                           " and %4n <= EXPIREDATE"
                           " and %5n >= CREATEDATE"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all"
                             " ("
                             "select VENDOR,CARRIER,FARETARIFF,CUR,FARECLASS,SEQNO,LINKNO,"
                             "MARKET1,MARKET2,CREATEDATE,EFFDATE,DISCDATE,EXPIREDATE,"
                             "LASTMODDATE,FAREAMT,NODEC,FOOTNOTE1,FOOTNOTE2,OWRT,RULE,"
                             "ROUTING,DIRECTIONALITY,GLOBALDIR,INHIBIT,CHANGETAG1,"
                             "CHANGETAG2,CHANGETAG3,CHANGETAG4,CHANGETAG5,CHANGETAG11,"
                             "BATCHCI ");
    partialStatement.From("=DOMESTICFARE");
    partialStatement.Where("MARKET1 = %6q"
                           " and MARKET2 = %7q"
                           " and CARRIER = %8q"
                           " and EFFDATE < DISCDATE"
                           " and VALIDITYIND = 'Y'"
                           " and %9n <= EXPIREDATE"
                           " and %10n >= CREATEDATE"
                           ")");
    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
  }
  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

////////////////////////////////////////////////////////////////////////
// QueryGetIntFares
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetIntFaresSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetIntFaresSQLStatement() {};
  virtual ~QueryGetIntFaresSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    CARRIER,
    FARETARIFF,
    CUR,
    FARECLASS,
    SEQNO,
    LINKNO,
    MARKET1,
    MARKET2,
    CREATEDATE,
    EFFDATE,
    DISCDATE,
    EXPIREDATE,
    LASTMODDATE,
    FAREAMT,
    NODEC,
    FOOTNOTE1,
    FOOTNOTE2,
    OWRT,
    RULE,
    ROUTING,
    DIRECTIONALITY,
    GLOBALDIR,
    INHIBIT,
    ROUTECODE,
    DBECLASS,
    FAREQUALCODE,
    TARIFFFAMILY,
    CABOTAGEIND,
    GOVTAPPVLIND,
    CONSTRUCTIONIND,
    MULTILATERALIND,
    AIRPORT1,
    AIRPORT2,
    VIACITYIND,
    VIACITY,
    CHANGETAG2,
    CHANGETAG3,
    CHANGETAG5,
    CHANGETAG6,
    CHANGETAG8,
    CHANGETAG10,
    CHANGETAG17,
    CHANGETAG11,
    BATCHCI,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,CARRIER,FARETARIFF,CUR,FARECLASS,SEQNO,LINKNO,MARKET1,"
                  "       MARKET2,CREATEDATE,EFFDATE,DISCDATE,EXPIREDATE,LASTMODDATE,"
                  "       FAREAMT,NODEC,FOOTNOTE1,FOOTNOTE2,OWRT,RULE,ROUTING,DIRECTIONALITY,"
                  "       GLOBALDIR,INHIBIT,ROUTECODE,DBECLASS,FAREQUALCODE,TARIFFFAMILY,"
                  "       CABOTAGEIND,GOVTAPPVLIND,CONSTRUCTIONIND,MULTILATERALIND,AIRPORT1,"
                  "       AIRPORT2,VIACITYIND,VIACITY,CHANGETAG2,CHANGETAG3,CHANGETAG5,"
                  "       CHANGETAG6,CHANGETAG8,CHANGETAG10,CHANGETAG17,'N' CHANGETAG11,"
                  "       BATCHCI");
    this->From("=INTERNATIONALFARE");
    this->Where("MARKET1 = %1q"
                "    and MARKET2 = %2q"
                "    and CARRIER %3s"
                "    and VALIDITYIND = 'Y'"
                "    and %cd <= DISCDATE"
                "    and %cd <= EXPIREDATE"
                "    and EFFDATE <= DISCDATE");
    this->OrderBy("VENDOR,CARRIER,FARETARIFF,CUR,FARECLASS,SEQNO,LINKNO,CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }
  static FareInfo* mapRowToFareInfo(Row* row)
  {
    tse::FareInfo* f;
    VendorCode vendor = row->getString(VENDOR);
    if (vendor.equalToConst("SITA"))
      f = new tse::SITAFareInfo;
    else
      f = new tse::FareInfo;

    f->_vendor = vendor;
    f->_carrier = row->getString(CARRIER);
    f->_fareTariff = row->getInt(FARETARIFF);
    f->_currency = row->getString(CUR);
    f->_fareClass = row->getString(FARECLASS);
    f->_sequenceNumber = row->getLong(SEQNO);
    f->_linkNumber = row->getInt(LINKNO);
    f->_market1 = row->getString(MARKET1);
    f->_market2 = row->getString(MARKET2);

    f->createDate() = row->getDate(CREATEDATE);
    f->expireDate() = row->getDate(EXPIREDATE);
    f->effDate() = row->getDate(EFFDATE);
    f->discDate() = row->getDate(DISCDATE);
    f->_lastModDate = row->getDate(LASTMODDATE);

    // workaround for a Content bug where the expire date is being
    // set forward to after the disc date
    if (f->discDate().isValid() && f->discDate() < f->expireDate())
    {
      DateTime discDate = f->discDate().addDays(1);
      if (UNLIKELY(discDate < f->expireDate()))
        f->expireDate() = discDate.subtractSeconds(1);
    }

    f->_noDec = row->getInt(NODEC);
    f->_fareAmount = QUERYCLASS::adjustDecimal(row->getLong(FAREAMT), f->_noDec);
    f->_originalFareAmount = f->_fareAmount;

    f->_footnote1 = row->getString(FOOTNOTE1);
    f->_footnote2 = row->getString(FOOTNOTE2);
    f->_routingNumber = row->getString(ROUTING);
    f->_ruleNumber = row->getString(RULE);

    std::string dir = row->getString(DIRECTIONALITY);
    if (dir == "F")
      f->_directionality = FROM;
    else if (dir == "T")
      f->_directionality = TO;
    else if (dir.empty() || dir == " " || dir == "B")
      f->_directionality = BOTH;

    std::string gd = row->getString(GLOBALDIR);
    strToGlobalDirection(f->_globalDirection, gd);

    f->_owrt = row->getString(OWRT)[0];
    if (f->_owrt == ROUND_TRIP_MAYNOT_BE_HALVED)
      f->_fareAmount = f->_fareAmount / 2;

    f->_inhibit = row->getChar(INHIBIT);

    f->_increasedFareAmtTag = row->getChar(CHANGETAG2);
    f->_reducedFareAmtTag = row->getChar(CHANGETAG3);
    f->_footnoteTag = row->getChar(CHANGETAG5);
    f->_routingTag = row->getChar(CHANGETAG6);
    f->_mpmTag = row->getChar(CHANGETAG8);
    f->_effectiveDateTag = row->getChar(CHANGETAG10);
    f->_currencyCodeTag = row->getChar(CHANGETAG17);
    f->_ruleTag = row->getChar(CHANGETAG11);

    // ConstructionInd is a native SITA field. ATPCO process shouldn't use it.
    //
    // But SabreMyFares project wants the best of two worlds.
    // So, the field was moved here for SMF use.
    f->constructionInd() = row->getChar(CONSTRUCTIONIND);

    f->vendorFWS() = (BATCHCI_FROM_VENDR_FWS == row->getString(BATCHCI));

    if (vendor != "SITA")
      return f;

    ////////////////////////////// Must be SITA! //////////////////////////////
    SITAFareInfo* sf = (SITAFareInfo*)f;
    sf->routeCode() = row->getString(ROUTECODE);
    sf->dbeClass() = row->getString(DBECLASS);
    sf->fareQualCode() = row->getChar(FAREQUALCODE);
    sf->tariffFamily() = row->getChar(TARIFFFAMILY);
    sf->cabotageInd() = row->getChar(CABOTAGEIND);
    sf->govtAppvlInd() = row->getChar(GOVTAPPVLIND);
    sf->multiLateralInd() = row->getChar(MULTILATERALIND);
    sf->airport1() = row->getString(AIRPORT1);
    sf->airport2() = row->getString(AIRPORT2);
    sf->viaCityInd() = row->getChar(VIACITYIND);
    sf->viaCity() = row->getString(VIACITY);

    return sf;
  } // mapRowToFareInfo()

protected:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetIntFaresHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetIntFaresHistoricalSQLStatement : public QueryGetIntFaresSQLStatement<QUERYCLASS>
{

protected:
  // void setPartialStatement(DBAccess::SQLStatement & partialStatement, int parameterIndex);
  void setPartialStatement(DBAccess::SQLStatement& partialStatement, int parameterIndex)
  {
    if (parameterIndex < 2)
    {
      partialStatement.Command(
          "("
          "select VENDOR,CARRIER,FARETARIFF,CUR,FARECLASS,SEQNO,LINKNO,MARKET1,"
          "MARKET2,CREATEDATE,EFFDATE,DISCDATE,EXPIREDATE,LASTMODDATE,"
          "FAREAMT,NODEC,FOOTNOTE1,FOOTNOTE2,OWRT,RULE,ROUTING,DIRECTIONALITY,"
          "GLOBALDIR,INHIBIT,ROUTECODE,DBECLASS,FAREQUALCODE,TARIFFFAMILY,"
          "CABOTAGEIND,GOVTAPPVLIND,CONSTRUCTIONIND,MULTILATERALIND,AIRPORT1,"
          "AIRPORT2,VIACITYIND,VIACITY,CHANGETAG2,CHANGETAG3,CHANGETAG5,"
          "CHANGETAG6,CHANGETAG8,CHANGETAG10,CHANGETAG17,'N' CHANGETAG11,"
          "BATCHCI ");
    }
    else
    {
      partialStatement.Command(
          "("
          "select VENDOR,CARRIER,FARETARIFF,CUR,FARECLASS,SEQNO,LINKNO,MARKET1,"
          "MARKET2,CREATEDATE,EFFDATE,DISCDATE,EXPIREDATE,LASTMODDATE,"
          "FAREAMT,NODEC,FOOTNOTE1,FOOTNOTE2,OWRT,RULE,ROUTING,DIRECTIONALITY,"
          "GLOBALDIR,INHIBIT,"
          "'' ROUTECODE,'' DBECLASS,'' FAREQUALCODE,'' TARIFFFAMILY,"
          "'' CABOTAGEIND,'' GOVTAPPVLIND,'' CONSTRUCTIONIND,'' MULTILATERALIND,"
          "'' AIRPORT1,'' AIRPORT2,'' VIACITYIND,'' VIACITY,"
          "CHANGETAG1 CHANGETAG2, CHANGETAG2 CHANGETAG3, CHANGETAG3 CHANGETAG5,"
          "CHANGETAG4 CHANGETAG6, CHANGETAG5 CHANGETAG8,"
          "'N' CHANGETAG10, 'N' CHANGETAG17, CHANGETAG11, BATCHCI");
    }
    partialStatement.From("=INTERNATIONALFAREH");
    const char w[] = "MARKET1 = %1q"
                     " and MARKET2 = %2q"
                     " and CARRIER %3s"
                     " and EFFDATE < DISCDATE"
                     " and VALIDITYIND = 'Y'"
                     " and %4n <= EXPIREDATE"
                     " and %5n >= CREATEDATE"
                     ")";
    const char* p = w;
    int offset = parameterIndex * 5;
    std::string where;
    where.reserve(sizeof(w) + 5);
    while (*p != '\0')
    {
      if (*p == '%')
      {
        where.push_back('%');
        int i = *(++p) - '0' + offset;
        if (i > 9)
        {
          where.push_back((char)((i / 10) + '0'));
          i = i % 10;
        }
        where.push_back((char)(i + '0'));
        ++p;
      }
      else
        where.push_back(*p++);
    }
    partialStatement.Where(where);
  }

  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;
    int step = 0;
    setPartialStatement(partialStatement, 0);
    partialStatement.From("=INTERNATIONALFAREH");
    adjustBaseSQL(step++, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all ");
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    setPartialStatement(partialStatement, 1);
    partialStatement.From("=INTERNATIONALFARE");
    adjustBaseSQL(step++, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    while (adjustBaseSQL(step++, partialStatement))
    {
      compoundStatement.push_back(partialStatement);
      partialStatement.Reset();
    }
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
  }
  //  override this version to replace parts of the compound statement
  virtual bool adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) { return false; }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllFaresHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllFaresHistoricalSQLStatement
    : public QueryGetIntFaresHistoricalSQLStatement<QUERYCLASS>
{
protected:
  bool adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) override
  {
    switch (step)
    {
    case 2:
    case 4:
    {
      partialStatement.Command(" union all ");
      return true;
    }
    case 3:
    {
      QueryGetIntFaresHistoricalSQLStatement<QUERYCLASS>::setPartialStatement(partialStatement, 2);
      partialStatement.From("=DOMESTICFAREH");
      return true;
    }
    case 5:
    {
      QueryGetIntFaresHistoricalSQLStatement<QUERYCLASS>::setPartialStatement(partialStatement, 3);
      partialStatement.From("=DOMESTICFARE");
      return true;
    }
    }
    return false;
  }
};

////////////////////////////////////////////////////////////////////////
// QueryGetDomMarkets_Fare
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetDomMarkets_FareSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetDomMarkets_FareSQLStatement() {};
  virtual ~QueryGetDomMarkets_FareSQLStatement() {};

  enum ColumnIndexes
  {
    MARKET = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select distinct MARKET");
    this->From("=MARKET");
    this->Where("NATION = 'CA' or NATION = 'US'");
    this->OrderBy("MARKET");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL(queryName);

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static const char* mapRowToMarket(Row* row) { return row->getString(MARKET); }

protected:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL(const char* queryName) {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetDomMarkets_FareHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetDomMarkets_FareHistoricalSQLStatement
    : public QueryGetDomMarkets_FareSQLStatement<QUERYCLASS>
{
public:
  enum ColumnIndexes
  {
    MARKET = 0,
    CREATEDATE,
    EXPIREDATE,
    NUMBEROFCOLUMNS
  }; // enum

  static MarketAndDate& mapRowToMarket(Row* row, MarketAndDate& md)
  {
    md.market = row->getString(MARKET);
    md.createDate = row->getDate(CREATEDATE);
    md.expireDate = row->getDate(EXPIREDATE);
    return md;
  }

protected:
  void adjustBaseSQL(const char* queryName) override
  {
    this->Command("select distinct MARKET,CREATEDATE,EXPIREDATE ");
  }
};
} // tse
