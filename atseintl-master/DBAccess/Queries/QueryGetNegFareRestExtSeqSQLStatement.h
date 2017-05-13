//----------------------------------------------------------------------------
//          File:           QueryGetNegFareRestExtSeqSQLStatement.h
//          Description:    QueryGetNegFareRestExtSeqSQLStatement
//          Created:        9/9/2010
//          Authors:        Artur Krezel
//
//          Updates:
//
//     � 2010, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetNegFareRestExtSeq.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetNegFareRestExtSeqSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetNegFareRestExtSeqSQLStatement() {};
  virtual ~QueryGetNegFareRestExtSeqSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    CITYFROM,
    CITYTO,
    CARRIER,
    VIACITY1,
    VIACITY2,
    VIACITY3,
    VIACITY4,
    PUBLISHEDFAREBASIS,
    UNIQUEFAREBASIS,
    SUPPRESSNVBNVA,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR, ITEMNO, SEQNO, CREATEDATE, EXPIREDATE, CITYFROM, CITYTO,"
                  "       CARRIER, VIACITY1, VIACITY2, VIACITY3, VIACITY4, PUBLISHEDFAREBASIS,"
                  "       UNIQUEFAREBASIS, SUPPRESSNVBNVA");

    this->From("=NEGFARERESTEXTSEQ ");
    this->Where("VENDOR = %1q"
                " and ITEMNO = %2n"
                " and %cd <= EXPIREDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("SEQNO");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::NegFareRestExtSeq* mapRowToNegFareRestExtSeq(Row* row)
  {
    tse::NegFareRestExtSeq* negFareRestExtSeq = new tse::NegFareRestExtSeq;

    negFareRestExtSeq->vendor() = row->getString(VENDOR);
    negFareRestExtSeq->itemNo() = row->getInt(ITEMNO);
    negFareRestExtSeq->seqNo() = row->getLong(SEQNO);
    negFareRestExtSeq->createDate() = row->getDate(CREATEDATE);
    negFareRestExtSeq->expireDate() = row->getDate(EXPIREDATE);
    if (!row->isNull(CITYFROM))
      negFareRestExtSeq->cityFrom() = row->getString(CITYFROM);
    if (!row->isNull(CITYTO))
      negFareRestExtSeq->cityTo() = row->getString(CITYTO);
    if (!row->isNull(CARRIER))
      negFareRestExtSeq->carrier() = row->getString(CARRIER);
    if (!row->isNull(VIACITY1))
      negFareRestExtSeq->viaCity1() = row->getString(VIACITY1);
    if (!row->isNull(VIACITY2))
      negFareRestExtSeq->viaCity2() = row->getString(VIACITY2);
    if (!row->isNull(VIACITY3))
      negFareRestExtSeq->viaCity3() = row->getString(VIACITY3);
    if (!row->isNull(VIACITY4))
      negFareRestExtSeq->viaCity4() = row->getString(VIACITY4);
    if (!row->isNull(PUBLISHEDFAREBASIS))
      negFareRestExtSeq->publishedFareBasis() = row->getString(PUBLISHEDFAREBASIS);
    if (!row->isNull(UNIQUEFAREBASIS))
      negFareRestExtSeq->uniqueFareBasis() = row->getString(UNIQUEFAREBASIS);
    if (!row->isNull(SUPPRESSNVBNVA))
      negFareRestExtSeq->suppressNvbNva() = row->getChar(SUPPRESSNVBNVA);

    return negFareRestExtSeq;
  } // mapRowToNegFareRestExtSeq()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetNegFareRestExtHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetNegFareRestExtSeqHistoricalSQLStatement
    : public QueryGetNegFareRestExtSeqSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command(
        "("
        "select VENDOR, ITEMNO, SEQNO, CREATEDATE, EXPIREDATE, CITYFROM, CITYTO,"
        "       CARRIER, VIACITY1, VIACITY2, VIACITY3, VIACITY4, PUBLISHEDFAREBASIS,"
        "       UNIQUEFAREBASIS, SUPPRESSNVBNVA");
    partialStatement.From("=NEGFARERESTEXTSEQH");
    partialStatement.Where("VENDOR = %1q"
                           " and ITEMNO = %2n"
                           " and %3n <= EXPIREDATE"
                           " and %4n >= CREATEDATE"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(
        " union all"
        " ("
        "select VENDOR, ITEMNO, SEQNO, CREATEDATE, EXPIREDATE, CITYFROM, CITYTO,"
        "       CARRIER, VIACITY1, VIACITY2, VIACITY3, VIACITY4, PUBLISHEDFAREBASIS,"
        "       UNIQUEFAREBASIS, SUPPRESSNVBNVA");
    partialStatement.From("=NEGFARERESTEXTSEQ");
    partialStatement.Where("VENDOR = %5q"
                           " and ITEMNO = %6n"
                           " and %7n <= EXPIREDATE"
                           " and %8n >= CREATEDATE"
                           ")");

    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
    if (DataManager::forceSortOrder())
      this->OrderBy("SEQNO");
  }
  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}

}; // class QueryGetNegFareRestExtHistoricalSQLStatement

} // tse
