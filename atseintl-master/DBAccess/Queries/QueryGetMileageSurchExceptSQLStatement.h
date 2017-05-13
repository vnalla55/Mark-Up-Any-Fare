//----------------------------------------------------------------------------
//          File:           QueryGetMileageSurchExceptSQLStatement.h
//          Description:    QueryGetMileageSurchExceptSQLStatement
//          Created:        10/26/2007
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
#include "DBAccess/Queries/QueryGetMileageSurchExcept.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetOneMileageSurchExceptBaseSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetOneMileageSurchExceptBaseSQLStatement() {};
  virtual ~QueryGetOneMileageSurchExceptBaseSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    TEXTTBLITEMNO,
    GOVERNINGCARRIER,
    RULETARIFF,
    RULE,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    RULETARIFFCODE,
    FARECLASS,
    MPMSURCHEXCEPT,
    MUSTVIACXREXCEPT,
    GLOBALDIR,
    DIRECTIONALITY,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    MUSTVIALOCTYPE,
    MUSTVIALOC,
    MUSTONLYVIAIND,
    MUSTVIAALLIND,
    NOSTOPOVERIND,
    MUSTNOTVIALOCTYPE,
    MUSTNOTVIALOC,
    PSGTYPEINFANT,
    PSGTYPECHILD,
    CARRIER,
    CARRIERGROUPSET,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select e.VENDOR,e.TEXTTBLITEMNO,e.GOVERNINGCARRIER,e.RULETARIFF,e.RULE,"
                  "       e.CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,RULETARIFFCODE,FARECLASS,"
                  "       MPMSURCHEXCEPT,MUSTVIACXREXCEPT,GLOBALDIR,DIRECTIONALITY,LOC1TYPE,"
                  "       LOC1,LOC2TYPE,LOC2,MUSTVIALOCTYPE,MUSTVIALOC,MUSTONLYVIAIND,"
                  "       MUSTVIAALLIND,NOSTOPOVERIND,MUSTNOTVIALOCTYPE,MUSTNOTVIALOC,"
                  "       PSGTYPEINFANT,PSGTYPECHILD,c.CARRIER,c.CARRIERGROUPSET");

    //		        this->From("=MILEAGESURCHEXCEPT e LEFT OUTER JOIN =MILEAGESURCHCXR c"
    //		                   " USING(VENDOR,TEXTTBLITEMNO,GOVERNINGCARRIER,"
    //		                   "                                    RULETARIFF,RULE,CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(6);
    joinFields.push_back("VENDOR");
    joinFields.push_back("TEXTTBLITEMNO");
    joinFields.push_back("GOVERNINGCARRIER");
    joinFields.push_back("RULETARIFF");
    joinFields.push_back("RULE");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=MILEAGESURCHEXCEPT", "e", "LEFT OUTER JOIN", "=MILEAGESURCHCXR", "c", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("e.VENDOR = %1q"
                "    and e.TEXTTBLITEMNO = %2n"
                "    and e.GOVERNINGCARRIER = %3q"
                "    and e.RULETARIFF = %4n"
                "    and e.RULE = %5q"
                "    and %cd <= EXPIREDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::MileageSurchExcept*
  mapRowToMileageSurchExceptBase(Row* row, MileageSurchExcept* msePrev)
  { // Load up Parent Determinant Fields
    VendorCode vendor = row->getString(VENDOR);
    int textTblItemNo = row->getInt(TEXTTBLITEMNO);
    CarrierCode governingCarrier = row->getString(GOVERNINGCARRIER);
    int ruleTariff = row->getInt(RULETARIFF);
    RuleNumber rule = row->getString(RULE);
    DateTime createDate = row->getDate(CREATEDATE);

    MileageSurchExcept* mse;

    // If Parent hasn't changed, add to Children
    if (msePrev != nullptr && msePrev->vendor() == vendor && msePrev->textTblItemNo() == textTblItemNo &&
        msePrev->governingCarrier() == governingCarrier && msePrev->ruleTariff() == ruleTariff &&
        msePrev->rule() == rule && msePrev->createDate() == createDate)
    { // Just add to Prev
      mse = msePrev;
    }
    else
    { // Time for a new Parent
      mse = new tse::MileageSurchExcept;
      mse->vendor() = vendor;
      mse->textTblItemNo() = textTblItemNo;
      mse->governingCarrier() = governingCarrier;
      mse->ruleTariff() = ruleTariff;
      mse->rule() = rule;
      mse->createDate() = createDate;

      mse->expireDate() = row->getDate(EXPIREDATE);
      mse->effDate() = row->getDate(EFFDATE);
      mse->discDate() = row->getDate(DISCDATE);
      mse->ruleTariffCode() = row->getString(RULETARIFFCODE);
      mse->fareClass() = row->getString(FARECLASS);
      mse->mpmSurchExcept() = row->getChar(MPMSURCHEXCEPT);
      mse->mustViaCxrExcept() = row->getChar(MUSTVIACXREXCEPT);
      strToGlobalDirection(mse->globalDir(), row->getString(GLOBALDIR));

      std::string direct = row->getString(DIRECTIONALITY);
      if (direct == "F")
        mse->directionality() = FROM;
      else if (direct == "W")
        mse->directionality() = WITHIN;
      else if (direct == "O")
        mse->directionality() = ORIGIN;
      else if (direct == "X")
        mse->directionality() = TERMINATE;
      else if (direct.empty() || direct == " " || direct == "B")
        mse->directionality() = BETWEEN;

      LocKey* loc = &mse->loc1();
      loc->locType() = row->getChar(LOC1TYPE);
      loc->loc() = row->getString(LOC1);

      loc = &mse->loc2();
      loc->locType() = row->getChar(LOC2TYPE);
      loc->loc() = row->getString(LOC2);

      loc = &mse->mustViaLoc();
      loc->locType() = row->getChar(MUSTVIALOCTYPE);
      loc->loc() = row->getString(MUSTVIALOC);

      mse->mustOnlyViaInd() = row->getChar(MUSTONLYVIAIND);
      mse->mustViaAllInd() = row->getChar(MUSTVIAALLIND);
      mse->noStopoverInd() = row->getChar(NOSTOPOVERIND);

      loc = &mse->mustNotViaLoc();
      loc->locType() = row->getChar(MUSTNOTVIALOCTYPE);
      loc->loc() = row->getString(MUSTNOTVIALOC);

      mse->psgTypeInfant() = row->getChar(PSGTYPEINFANT);
      mse->psgTypeChild() = row->getChar(PSGTYPECHILD);
    } // New Parent

    // Add new Carrier & return
    if (!row->isNull(CARRIER))
    {
      MileageSurchCxr* msc = new MileageSurchCxr;
      msc->carrier() = row->getString(CARRIER);
      msc->carrierGroupSet() = row->getString(CARRIERGROUPSET);
      mse->cxrs().push_back(msc);
    }
    return mse;
  } // mapRowToMileageSurchExceptBase()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetOneMileageSurchExceptBaseSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetOneMileageSurchExceptBaseHistoricalSQLStatement
    : public QueryGetOneMileageSurchExceptBaseSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("e.VENDOR = %1q"
                " and e.TEXTTBLITEMNO = %2n"
                " and e.GOVERNINGCARRIER = %3q"
                " and e.RULETARIFF = %4n"
                " and e.RULE = %5q"
                " and %6n <= EXPIREDATE"
                " and %7n >= e.CREATEDATE");
    this->OrderBy("VENDOR,TEXTTBLITEMNO,GOVERNINGCARRIER,RULETARIFF,RULE,CARRIER, "
                  "CARRIERGROUPSET,CREATEDATE");
  };
}; // class QueryGetOneMileageSurchExceptBaseHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllMileageSurchExceptBaseSQLStatement
    : public QueryGetOneMileageSurchExceptBaseSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= EXPIREDATE");
    this->OrderBy("VENDOR,TEXTTBLITEMNO,GOVERNINGCARRIER,RULETARIFF,RULE,CARRIER, "
                  "CARRIERGROUPSET,CREATEDATE");
  };
};

////////////////////////////////////////////////////////////////////////
// SQL for QueryGetMlgSurchPsgTypes
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMlgSurchPsgTypesSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMlgSurchPsgTypesSQLStatement() {};
  virtual ~QueryGetMlgSurchPsgTypesSQLStatement() {};

  enum ColumnIndexes
  {
    PSGTYPE = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select PSGTYPE");
    this->From("=MILEAGESURCHPSGTYPE");
    this->Where("VENDOR =  %1q"
                " and TEXTTBLITEMNO = %2n"
                " and GOVERNINGCARRIER = %3q"
                " and RULETARIFF = %4n"
                " and RULE = %5q"
                " and CREATEDATE = %6n");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static PaxTypeCode mapRowToPsgType(Row* row) { return row->getString(PSGTYPE); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// SQL for QueryCxrInMlgSurchExcpt
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryCxrInMlgSurchExcptSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryCxrInMlgSurchExcptSQLStatement() {};
  virtual ~QueryCxrInMlgSurchExcptSQLStatement() {};

  enum ColumnIndexes
  {
    CXRCOUNT = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select count(*) CXRCOUNT");
    this->From("=MILEAGESURCHEXCEPT");
    this->Where("GOVERNINGCARRIER = %1q"
                " and %cd <= EXPIREDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // QueryCxrInMlgSurchExcptSQLStatement

///////////////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause for QueryCxrInMlgSurchExcptHistorical
///////////////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryCxrInMlgSurchExcptHistoricalSQLStatement
    : public QueryCxrInMlgSurchExcptSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("GOVERNINGCARRIER = %1q"
                " and %2n <= EXPIREDATE"
                " and %3n >= CREATEDATE");
  };
}; // class QueryCxrInMlgSurchExcptHistoricalSQLStatement
} // tse
