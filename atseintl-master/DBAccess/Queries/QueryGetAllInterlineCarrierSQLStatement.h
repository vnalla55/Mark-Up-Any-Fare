//----------------------------------------------------------------------------
//     (c)2015, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetAllInterlineCarrier.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetAllInterlineCarrierSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetAllInterlineCarrierSQLStatement() {};
  virtual ~QueryGetAllInterlineCarrierSQLStatement() {};

  enum ColumnIndexes
  {
    CARRIER = 0,
    PARTNERS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(" select CARRIER,PARTNERS");
    this->From("=INTERLINE_CARRIER ");
    this->OrderBy("CARRIER,PARTNERS");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::InterlineCarrierInfo* mapRowToInterlineCarrier(Row* row)
  {
    tse::InterlineCarrierInfo* interlineCxr = new tse::InterlineCarrierInfo;
    interlineCxr->carrier() = row->getString(CARRIER);

    const std::string& partners = row->getString(PARTNERS);
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> separator(",");
    tokenizer tokens(partners, separator);
    tokenizer::iterator tokenI = tokens.begin();

    for (; tokenI != tokens.end(); ++tokenI)
      interlineCxr->partners().push_back(tokenI->data());

    return interlineCxr;

  } // mapRowToInterlineCarrier()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllInterlineCarrierHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllInterlineCarrierHistoricalSQLStatement
    : public QueryGetAllInterlineCarrierSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override { this->OrderBy("CARRIER,PARTNERS"); }
};

} // tse
