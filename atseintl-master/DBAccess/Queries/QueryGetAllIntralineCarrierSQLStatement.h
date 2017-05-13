//----------------------------------------------------------------------------
//     (c)2015, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetAllIntralineCarrier.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetAllIntralineCarrierSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetAllIntralineCarrierSQLStatement() {};
  virtual ~QueryGetAllIntralineCarrierSQLStatement() {};

  enum ColumnIndexes
  {
    NAME = 0,
    PARTNERS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(" select NAME,PARTNERS");
    this->From("=INTRALINE_CARRIER ");
    this->OrderBy("NAME,PARTNERS");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::IntralineCarrierInfo* mapRowToIntralineCarrier(Row* row)
  {
    tse::IntralineCarrierInfo* intralineCxr = new tse::IntralineCarrierInfo;
    intralineCxr->name() = row->getString(NAME);

    const std::string& partners = row->getString(PARTNERS);
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> separator(",");
    tokenizer tokens(partners, separator);
    tokenizer::iterator tokenI = tokens.begin();

    for (; tokenI != tokens.end(); ++tokenI)
      intralineCxr->partners().push_back(tokenI->data());

    return intralineCxr;

  } // mapRowToIntralineCarrier()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllIntralineCarrierHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllIntralineCarrierHistoricalSQLStatement
    : public QueryGetAllIntralineCarrierSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override { this->OrderBy("NAME,PARTNERS"); }
};

} // tse
