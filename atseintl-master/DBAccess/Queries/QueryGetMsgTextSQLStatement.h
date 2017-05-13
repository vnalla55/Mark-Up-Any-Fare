//----------------------------------------------------------------------------
//          File:           QueryGetMsgTextSQLStatement.h
//          Description:    QueryGetMsgTextSQLStatement
//          Created:        10/26/2007
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
#include "DBAccess/FareCalcConfigText.h"
#include "DBAccess/Queries/QueryGetMsgText.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetMsgTextSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMsgTextSQLStatement() {};
  virtual ~QueryGetMsgTextSQLStatement() {};

  enum ColumnIndexes
  {
    TEXTAPPL = 0,
    TEXT,
    NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select a.TEXTAPPL, b.TEXT");
    this->From("=FARECALCCONFIGTEXT a, =FREETEXTSEG b");
    this->Where("a.USERAPPLTYPE = %1q"
                "   and a.USERAPPL = %2q"
                "   and a.PSEUDOCITY = %3q"
                "   and a.MESSAGETYPE = b.MESSAGETYPE"
                "   and a.ITEMNO = b.ITEMNO");
    this->OrderBy("b.SEQNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static void mapRowToMsgText(FareCalcConfigText::FCCTextMap& fccTextMap, Row* row)
  {
    int appl = row->getInt(TEXTAPPL);
    std::string text = row->getString(TEXT);
    if (appl > 0 && appl < FareCalcConfigText::MAX_TEXT_APPL && !text.empty())
    {
      fccTextMap.insert(std::make_pair(static_cast<FareCalcConfigText::TextAppl>(appl), text));
    }
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};
} // tse
