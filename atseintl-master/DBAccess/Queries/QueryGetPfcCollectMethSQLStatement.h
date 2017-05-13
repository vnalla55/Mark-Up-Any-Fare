//----------------------------------------------------------------------------
//          File:           QueryGetPfcCollectMethSQLStatement.h
//          Description:    QueryGetPfcCollectMethSQLStatement
//          Created:        10/8/2007
//          Authors:         Mike Lillis
//
//          Updates:
//
//      2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetPfcCollectMeth.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetPfcCollectMethSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetPfcCollectMethSQLStatement() {}
  virtual ~QueryGetPfcCollectMethSQLStatement() {}

  enum ColumnIndexes
  {
    VENDOR = 0,
    CARRIER,
    EFFDATE,
    CREATEDATE,
    EXPIREDATE,
    DISCDATE,
    COLLECTOPTION,
    NATION,
    COLLOPTE,
    NUMBEROFCOLUMNS
  }; // enum
  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select m.VENDOR,m.CARRIER,m.EFFDATE,m.CREATEDATE,EXPIREDATE,"
                  "DISDATE DISCDATE,m.COLLECTOPTION,x.NATION,x.COLLECTOPTION COLLOPTE");

    //		        this->From(" =PFCCOLLECTIONMETHOD m LEFT OUTER JOIN =PFCCOLLECTIONEXCEPT x"
    //		                  " USING (VENDOR,CARRIER,EFFDATE,CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(4);
    joinFields.push_back("VENDOR");
    joinFields.push_back("CARRIER");
    joinFields.push_back("EFFDATE");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString("=PFCCOLLECTIONMETHOD",
                             "m",
                             "LEFT OUTER JOIN",
                             "=PFCCOLLECTIONEXCEPT",
                             "x",
                             joinFields,
                             from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where(" VALIDITYIND = 'Y'"
                "    and m.VENDOR = 'ATP' "
                "    and m.CARRIER = %1q ");
    if (DataManager::forceSortOrder())
      this->OrderBy("m.VENDOR,m.CARRIER,m.EFFDATE,m.CREATEDATE,x.NATION");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::PfcCollectMeth* mapRowToPfcCollectMeth(Row* row, PfcCollectMeth* cmPrev)
  {
    VendorCode vendor = row->getString(VENDOR);
    CarrierCode carrier = row->getString(CARRIER);
    DateTime effDate = row->getDate(EFFDATE);
    DateTime createDate = row->getDate(CREATEDATE);

    PfcCollectMeth* cm;

    // If Parent hasn't changed, add to Child (segs)
    if (cmPrev != nullptr && cmPrev->vendor() == vendor && cmPrev->carrier() == carrier &&
        cmPrev->effDate() == effDate && cmPrev->createDate() == createDate)
    { // Add to Prev
      cm = cmPrev;
    } // Previous Parent
    else
    { // Time for a new Parent
      cm = new tse::PfcCollectMeth;
      cm->vendor() = vendor;
      cm->carrier() = carrier;
      cm->effDate() = effDate;
      cm->createDate() = createDate;

      cm->expireDate() = row->getDate(EXPIREDATE);
      cm->discDate() = row->getDate(DISCDATE);
      cm->collectOption() = row->getChar(COLLECTOPTION);
    }
    if (!row->isNull(NATION))
    {
      PfcCollectExcpt* ce = new PfcCollectExcpt;
      ce->nation() = row->getString(NATION);
      ce->collectOption() = row->getChar(COLLOPTE);
      cm->excpts().push_back(ce);
    }
    return cm;
  } // mapRowToPfcCollectMeth()
private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//
//   Template used to get replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllPfcCollectMethSQLStatement : public QueryGetPfcCollectMethSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("VALIDITYIND = 'Y'");
    this->OrderBy("VENDOR, CARRIER, EFFDATE, CREATEDATE, NATION");
  }
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetPfcCollectMethHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetPfcCollectMethHistoricalSQLStatement
    : public QueryGetPfcCollectMethSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where(" VALIDITYIND = 'Y'"
                "    and m.VENDOR = 'ATP' "
                "    and m.CARRIER = %1q "
                "    and %2n <= m.EXPIREDATE"
                "    and (%3n >= m.CREATEDATE"
                "     or %4n >= m.EFFDATE)");
  }
};
}

