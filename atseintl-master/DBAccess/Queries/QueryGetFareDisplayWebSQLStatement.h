//----------------------------------------------------------------------------
//          File:           QueryGetFareDisplayWebSQLStatement.h
//          Description:    QueryGetFareDisplayWebSQLStatement
//          Created:        11/02/2007
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
#include "DBAccess/Queries/QueryGetFareDisplayWeb.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetFareDisplayWebSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFareDisplayWebSQLStatement() {};
  virtual ~QueryGetFareDisplayWebSQLStatement() {};

  enum ColumnIndexes
  {
    CARRIER = 0,
    DISPLAYIND,
    VENDOR,
    RULETARIFF,
    RULE,
    FARECLASS,
    TKTDESIGNATOR,
    PSGTYPE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select w.CARRIER, w.DISPLAYIND, w.VENDOR, w.RULETARIFF, w.RULE,"
                  " w.FARECLASS, w.TKTDESIGNATOR, s.PSGTYPE");

    //		        this->From("=FAREDISPLAYWEB w LEFT OUTER JOIN =FAREDISPLAYWEBSEG s"
    //		                   "   using (CARRIER, DISPLAYIND, VENDOR, RULETARIFF, "
    //		                   "   RULE, FARECLASS, TKTDESIGNATOR)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(7);
    joinFields.push_back("CARRIER");
    joinFields.push_back("DISPLAYIND");
    joinFields.push_back("VENDOR");
    joinFields.push_back("RULETARIFF");
    joinFields.push_back("RULE");
    joinFields.push_back("FARECLASS");
    joinFields.push_back("TKTDESIGNATOR");
    this->generateJoinString(
        "=FAREDISPLAYWEB", "w", "LEFT OUTER JOIN", "=FAREDISPLAYWEBSEG", "s", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("w.CARRIER = %1q");
    this->OrderBy("w.CARRIER, w.DISPLAYIND, w.VENDOR, w.RULETARIFF,"
                  "  w.RULE, w.FARECLASS, w.TKTDESIGNATOR,s.PSGTYPE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::FareDisplayWeb* mapRowToFareDisplayWeb(Row* row)
  {
    tse::FareDisplayWeb* fdWeb = new tse::FareDisplayWeb;

    fdWeb->carrier() = row->getString(CARRIER);
    fdWeb->displayInd() = row->getChar(DISPLAYIND);
    fdWeb->vendor() = row->getString(VENDOR);
    fdWeb->ruleTariff() = row->getInt(RULETARIFF);
    fdWeb->rule() = row->getString(RULE);
    fdWeb->fareClass() = row->getString(FARECLASS);
    fdWeb->tktDesignator() = row->getString(TKTDESIGNATOR);
    if (!row->isNull(PSGTYPE))
      fdWeb->paxType() = row->getString(PSGTYPE);
    if (fdWeb->paxType().empty())
      fdWeb->paxType() = ADULT;

    return fdWeb;
  } // mapRowToFareDisplayWeb()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllFareDisplayWeb
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllFareDisplayWebSQLStatement : public QueryGetFareDisplayWebSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override { this->Where("1=1"); }
};
} // tse
