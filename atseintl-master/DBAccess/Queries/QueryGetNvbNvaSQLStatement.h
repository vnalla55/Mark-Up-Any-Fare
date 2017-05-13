//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/NvbNvaInfo.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

////////////////////////////////////////////////////////////////////////////////
//   Main class
////////////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetNvbNvaSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetNvbNvaSQLStatement() {}
  virtual ~QueryGetNvbNvaSQLStatement() {}

  enum ColumnIndexes
  {
    NVBNVAID, // \.
    VENDOR, //  \.
    CARRIER, //   \.
    RULETARIFF, //    > these come from table NVBNVA
    RULE, //   /
    CREATEDATE, //  /
    EXPIREDATE, // /
    SEQUENCENUMBER, // \.
    FAREBASIS, //  \_ these come from table NVBNVASEG
    NVB, //  /
    NVA, // /
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {

    // define columns
    // \.
    //  \.
    //   \.
    //    > NVBNVA table
    //   /
    //  /
    // /
    // \.
    //  \_ NVBNVASEG table
    //  /
    // /

    // build SQL SELECT
    this->Command(
        "select n.NVBNVAID, n.VENDOR, n.CARRIER, n.RULETARIFF, n.RULE, n.CREATEDATE, n.EXPIREDATE,"
        "       s.SEQUENCENUMBER, s.FAREBASIS, s.NVB, s.NVA");

    // build SQL FROM/JOIN
    std::string from;
    std::vector<std::string> joinFields;
    joinFields.push_back("NVBNVAID");
    this->generateJoinString(
        "=NVBNVA", "n", "LEFT OUTER JOIN", "=NVBNVASEG", "s", joinFields, from);
    this->From(from);

    // build SQL WHERE
    this->Where("     n.VENDOR     = %1q "
                " and n.CARRIER    = %2q "
                " and n.RULETARIFF = %3n "
                " and n.RULE       = %4q "
                " and %cd <= n.EXPIREDATE");

    // build SQL ORDER BY
    this->OrderBy("SEQUENCENUMBER, CREATEDATE");

    // finalize
    adjustBaseSQL(); // callback to allow for replacement of SQL clauses by a derived class/template
    this->ConstructSQL(); // construct the SQL
    QUERYCLASS::registerBaseSQL(queryName,
                                *this); // Register SQL statement prior to parameter substitutions
    return *this;
  }

  static NvbNvaInfo* mapRowToNvbNvaInfo(Row* row, NvbNvaInfo* prevInfo)
  {

    // read NVBNVAID from NVBNVA table
    uint64_t id = row->getLongLong(NVBNVAID);

    // initialize info structure (to hold the record from NVBNVA table and matching NVBNVASEG
    // records in _segs)
    NvbNvaInfo* info = nullptr;
    if (prevInfo && prevInfo->nvbNvaId() == id) // if record is the same as last time, (in other
                                                // words, if segment data pertains to this record)
    {
      info = prevInfo; // keep using this object
    }
    else // if record is different,
    {
      // form a new object (pertaining NVBNVASEG data will be added to it)
      info = new NvbNvaInfo;

      // fill info structure: set key fields
      info->vendor() = row->getString(VENDOR); // NOT NULL by database constraint
      info->carrier() = row->getString(CARRIER); // NOT NULL by database constraint
      info->ruleTariff() = row->getLong(RULETARIFF); // NOT NULL by database constraint
      info->rule() = row->getString(RULE); // NOT NULL by database constraint

      // fill info structure: set other fields
      info->nvbNvaId() = row->getLongLong(NVBNVAID); // NOT NULL by database constraint
      info->createDate() = row->getDate(CREATEDATE); // NOT NULL by database constraint
      info->expireDate() = row->getDate(EXPIREDATE); // NOT NULL by database constraint
    }

    // fill info structure: attach segment info structure (record from table NVBNVASEG)
    if (!row->isNull(FAREBASIS)) // if left join returned a non-null segment record, FAREBASIS will
                                 // not be null
    {
      // create and fill segment structure
      NvbNvaSeg* seg = new NvbNvaSeg;
      seg->sequenceNumber() = row->getLongLong(SEQUENCENUMBER); // NOT NULL by database constraint
      seg->fareBasis() = row->getString(FAREBASIS); // NOT NULL by database constraint
      if (!row->isNull(NVB))
        seg->nvb() = row->getChar(NVB);
      if (!row->isNull(NVA))
        seg->nva() = row->getChar(NVA);

      // add this segment to info structure
      info->segs().push_back(seg);
    }

    // return the structure
    return info;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}

}; // class QueryGetNvbNvaSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetNvbNvaHistoricalSQLStatement : public QueryGetNvbNvaSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {

    // objects used below
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;
    std::vector<std::string> joinFields;
    std::string from;

    // build 1st SQL SELECT
    partialStatement.Command("("
                             " select n.NVBNVAID, n.VENDOR, n.CARRIER, n.RULETARIFF, n.RULE, "
                             "n.CREATEDATE, n.EXPIREDATE, "
                             "        s.SEQUENCENUMBER, s.FAREBASIS, s.NVB, s.NVA");

    // build 1st SQL FROM/JOIN
    joinFields.push_back("NVBNVAID");
    this->generateJoinString("=NVBNVA",
                             "n",
                             "LEFT OUTER JOIN",
                             "=NVBNVASEG",
                             "s",
                             joinFields,
                             from); // generateJoinString() doesn't change class' internals,
                                    // therefore it doesn't matter which object we invoke it on.
                                    // For some reason this method is non-static, and what's worse,
                                    // protected.  Thus we are forced into using 'this' object to
                                    // accomplish our task
    partialStatement.From(from);

    // build 1st SQL WHERE
    partialStatement.Where("     n.VENDOR     = %1q  "
                           " and n.CARRIER    = %2q  "
                           " and n.RULETARIFF = %3n  "
                           " and n.RULE       = %4q  "
                           " and %5n <= n.EXPIREDATE "
                           " and %6n >= n.CREATEDATE "
                           ")");

    // add the 1st SQL part to compound statement
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();

    // build 2nd SQL SELECT
    partialStatement.Command(" union all "
                             "("
                             " select n.NVBNVAID, n.VENDOR, n.CARRIER, n.RULETARIFF, n.RULE, "
                             "n.CREATEDATE, n.EXPIREDATE, "
                             "        s.SEQUENCENUMBER, s.FAREBASIS, s.NVB, s.NVA");

    // build 2nd SQL FROM/JOIN
    /* joinFields is the same as last time, thus we're not re-building it */
    this->generateJoinString("=NVBNVAH",
                             "n",
                             "LEFT OUTER JOIN",
                             "=NVBNVASEGH",
                             "s",
                             joinFields,
                             from); // generateJoinString() doesn't change class' internals,
                                    // therefore it doesn't matter which object we invoke it on.
                                    // For some reason this method is non-static, and what's worse,
                                    // protected.  Thus we are forced into using 'this' object to
                                    // accomplish our task
    partialStatement.From(from);

    // build 2nd SQL WHERE
    partialStatement.Where("     n.VENDOR     = %7q   "
                           " and n.CARRIER    = %8q   "
                           " and n.RULETARIFF = %9n   "
                           " and n.RULE       = %10q  "
                           " and %11n <= n.EXPIREDATE "
                           " and %12n >= n.CREATEDATE "
                           ")");

    // build SQL ORDER BY
    partialStatement.OrderBy("SEQUENCENUMBER, CREATEDATE");

    // add the 2nd SQL part to compound statement
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();

    // overwrite our SQL with the one built above
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
    this->OrderBy("");
  }

}; // class QueryGetNvbNvaHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause and remove OrderBy
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllNvbNvaSQLStatement : public QueryGetNvbNvaSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= n.EXPIREDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR, CARRIER, RULETARIFF, RULE, SEQUENCENUMBER, CREATEDATE");
    else
      this->OrderBy("VENDOR, CARRIER, RULETARIFF, RULE");
  }
};

} // namespace tse

