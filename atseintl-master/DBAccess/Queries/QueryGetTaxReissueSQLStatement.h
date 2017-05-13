//----------------------------------------------------------------------------
//          File:           QueryGetTaxReissueSQLStatement.h
//          Description:    QueryGetTaxReissueSQLStatement
//          Created:        10/8/2007
//          Authors:         Mike Lillis
//
//          Updates:
//
//     (C) 2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/FallbackUtil.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTaxReissue.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetTaxReissueSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTaxReissueSQLStatement() {}
  virtual ~QueryGetTaxReissueSQLStatement() {}

  enum ColumnIndexes
  {
    TAXCODE = 0,
    TAXTYPE,
    VERSIONDATE,
    CREATEDATE,
    SEQNO,
    EXPIREDATE,
    LOCKDATE,
    EFFDATE,
    DISCDATE,
    AMT,
    MEMONO,
    NEWSEQNO,
    NODEC,
    CUR,
    REISSUELOCEXCLIND,
    REISSUELOCTYPE,
    REISSUELOC,
    REFUNDIND,
    TKTLCXREXCLIND,
    TKTCARRIER,
    CAT33ONLYIND,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select tr.TAXCODE,tr.TAXTYPE,tr.VERSIONDATE,tr.CREATEDATE,tr.SEQNO,EXPIREDATE,"
                  " LOCKDATE,EFFDATE,DISCDATE,AMT,MEMONO,NEWSEQNO,NODEC,CUR,REISSUELOCEXCLIND,"
                  " REISSUELOCTYPE,REISSUELOC,REFUNDIND,TKTLCXREXCLIND,trs.TKTCARRIER,tr.CAT33ONLYIND");

    //		        this->From("=TAXREISSUE tr left outer join =TAXREISSUESEG trs"
    //		                   "   using(TAXCODE,VERSIONDATE,CREATEDATE,SEQNO)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(5);
    joinFields.push_back("TAXCODE");
    joinFields.push_back("TAXTYPE");
    joinFields.push_back("VERSIONDATE");
    joinFields.push_back("CREATEDATE");
    joinFields.push_back("SEQNO");
    this->generateJoinString(
        "=TAXREISSUE", "tr", "left outer join", "=TAXREISSUESEG", "trs", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("tr.TAXCODE = %1q"
                " and %cd <= tr.EXPIREDATE");
    this->OrderBy("VERSIONDATE,SEQNO,CREATEDATE,TKTCARRIER");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::TaxReissue* mapRowToTaxReissue(Row* row, TaxReissue* prev)
  { // Get the Key Info
    TaxCode taxCode = row->getString(TAXCODE);
    DateTime versionDate = row->getDate(VERSIONDATE);
    DateTime createDate = row->getDate(CREATEDATE);
    long seqNo = row->getLong(SEQNO);

    TaxReissue* tr = nullptr;

    if (prev != nullptr && prev->taxCode() == taxCode && prev->versionDate() == versionDate &&
        prev->createDate() == createDate && prev->seqNo() == seqNo)
    {
      tr = prev;
    }
    else
    {
      tr = new TaxReissue();
      tr->taxCode() = taxCode;
      tr->taxType() = row->isNull(TAXTYPE) ? "000" : row->getString(TAXTYPE);
      tr->versionDate() = versionDate;
      tr->createDate() = createDate;
      tr->seqNo() = seqNo;
      tr->expireDate() = row->getDate(EXPIREDATE);
      tr->lockDate() = row->getDate(LOCKDATE);
      tr->effDate() = row->getDate(EFFDATE);
      tr->discDate() = row->getDate(DISCDATE);
      tr->currencyNoDec() = row->getInt(NODEC);

      tr->taxAmt() = QUERYCLASS::adjustDecimal(row->getInt(AMT), tr->currencyNoDec());

      tr->currencyCode() = row->getString(CUR);
      tr->reissueExclLocInd() = row->getChar(REISSUELOCEXCLIND);
      tr->reissueLocType() = LocType(row->getString(REISSUELOCTYPE)[0]);
      tr->reissueLoc() = row->getString(REISSUELOC);
      tr->refundInd() = row->getChar(REFUNDIND);
      tr->tktlCxrExclInd() = row->getChar(TKTLCXREXCLIND);
      tr->cat33onlyInd() = row->getChar(CAT33ONLYIND);
    }

    if (!row->isNull(TKTCARRIER))
    {
      tr->validationCxr().push_back(row->getString(TKTCARRIER));
    }

    return tr;
  } // mapRowToTaxReissue()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetTaxReissueSQLStatement

////////////////////////////////////////////////////////////////////////
// Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetTaxReissueHistoricalSQLStatement : public QueryGetTaxReissueSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override { this->Where("tr.TAXCODE = %1q"); }
}; // class QueryGetTaxReissueHistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
// Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllTaxReissueHistoricalSQLStatement
    : public QueryGetTaxReissueSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("1 = 1");
    this->OrderBy("TAXCODE,VERSIONDATE,SEQNO,CREATEDATE,TKTCARRIER");
  };
}; // class QueryGetTaxReissueHistoricalSQLStatement
}
