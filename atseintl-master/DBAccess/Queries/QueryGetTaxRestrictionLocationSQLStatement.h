//----------------------------------------------------------------------------
//          File:           QueryGetTaxRestrictionLocationSQLStatement.h
//          Description:    QueryGetTaxRestrictionLocationSQLStatement
//          Created:        1/14/2010
//          Authors:        Piotr Badarycz
//
//          Updates:
//
//     (c)2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTaxRestrictionLocation.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetTaxRestrictionLocationSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTaxRestrictionLocationSQLStatement() {}
  virtual ~QueryGetTaxRestrictionLocationSQLStatement() {}

  enum ColumnIndexes
  {
    TAXRESTRICTIONLOCATIONNO,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    SEQNO,
    SALEISSUEIND,
    INCLEXCLIND,
    LOCTYPE,
    LOC,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select z.TAXRESTRICTIONLOCATIONNO, z.CREATEDATE, EXPIREDATE, EFFDATE, DISCDATE,"
                  "      SEQNO, SALEISSUEIND, INCLEXCLIND, LOCTYPE, LOC");
    this->From("=TAXRESTRICTIONLOCATION z, =TAXRESTRICTIONLOCATIONSEQ s");
    this->Where("z.TAXRESTRICTIONLOCATIONNO = s.TAXRESTRICTIONLOCATIONNO"
                "    and z.CREATEDATE = s.CREATEDATE"
                "    and z.TAXRESTRICTIONLOCATIONNO = %1q "
                "    and %cd <= EXPIREDATE");
    this->OrderBy("z.TAXRESTRICTIONLOCATIONNO, z.CREATEDATE, s.SEQNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static TaxRestrictionLocationInfo*
  mapRowToTaxRestrictionLocation(Row* row, TaxRestrictionLocationInfo* prev)
  {
    TaxRestrictionLocation location = row->getString(TAXRESTRICTIONLOCATIONNO);
    DateTime createDate = row->getDate(CREATEDATE);

    TaxRestrictionLocationInfo* locationInfo = nullptr;

    if (prev != nullptr && prev->location() == location && prev->createDate() == createDate)
    {
      locationInfo = prev;
    }
    else
    {
      locationInfo = new TaxRestrictionLocationInfo;
      locationInfo->location() = row->getString(TAXRESTRICTIONLOCATIONNO);
      locationInfo->createDate() = row->getDate(CREATEDATE);
      locationInfo->expireDate() = row->getDate(EXPIREDATE);
      locationInfo->effDate() = row->getDate(EFFDATE);
      locationInfo->discDate() = row->getDate(DISCDATE);
    }

    locationInfo->seqs().resize(locationInfo->seqs().size() + 1);
    TaxRestrictionLocationInfo::TaxRestrictionLocationInfoSeq& seq = locationInfo->seqs().back();
    seq.seqNo() = row->getLong(SEQNO);
    seq.saleIssueInd() = row->getChar(SALEISSUEIND);
    seq.inclExclInd() = row->getChar(INCLEXCLIND);
    seq.locType() = row->getChar(LOCTYPE);
    seq.loc() = row->getString(LOC);

    return locationInfo;
  } // mapRowToTaxRestrictionLocation()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetTaxRestrictionLocationHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetTaxRestrictionLocationHistoricalSQLStatement
    : public QueryGetTaxRestrictionLocationSQLStatement<QUERYCLASS>

{

private:
  void adjustBaseSQL() override
  {
    this->Where("z.TAXRESTRICTIONLOCATIONNO = s.TAXRESTRICTIONLOCATIONNO"
                "    and z.CREATEDATE = s.CREATEDATE"
                "    and z.TAXRESTRICTIONLOCATIONNO = %1q "
                "    and %2n <= z.EXPIREDATE"
                "    and %3n >= z.CREATEDATE");
  }
  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};
}

