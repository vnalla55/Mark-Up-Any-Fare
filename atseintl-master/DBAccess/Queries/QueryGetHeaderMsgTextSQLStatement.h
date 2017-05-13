//----------------------------------------------------------------------------
//          File:           QueryGetHeaderMsgTextSQLStatement.h
//          Description:    QueryGetHeaderMsgTextSQLStatement
//          Created:        3/2/2006
// Authors:         Mike Lillis
//
//          Updates:
//
//     � 2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetHeaderMsgText.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetHeaderMsgTextSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetHeaderMsgTextSQLStatement() {};
  virtual ~QueryGetHeaderMsgTextSQLStatement() {};

  enum ColumnIndexes
  {
    TEXT = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select a.TEXT");
    this->From("=FREETEXTSEG a");
    this->Where(" a.MESSAGETYPE = 'FAREDISP' and a.ITEMNO = %1q");
    this->OrderBy("a.SEQNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }; // RegisterColumnsAndBaseSQL()

  static std::string* mapRowToFDHeaderMsgText(Row* row)
  {
    std::string* sf = new std::string;
    (*sf) = row->getString(TEXT);
    return sf;
  } // mapRowToFDHeaderMsgText()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetFlightAppRuleSQLStatement
}

