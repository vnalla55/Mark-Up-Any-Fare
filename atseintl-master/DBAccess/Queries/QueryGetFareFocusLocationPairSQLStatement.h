#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareFocusLocationPair.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <typename QUERYCLASS>
class QueryGetFareFocusLocationPairSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    LOCATIONPAIRITEMNO
    , CREATEDATE
    , EXPIREDATE
    // child
    , LOC1
    , LOC1TYPE
    , LOC2
    , LOC2TYPE

    , NUMBEROFCOLUMNS
  };


  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    Command("select p.LOCATIONPAIRITEMNO, p.CREATEDATE, p.EXPIREDATE, c.LOC1, c.LOC1TYPE, c.LOC2, c.LOC2TYPE ");
    std::vector<std::string> joinFields;
    //joinFields.reserve(4);
    joinFields.push_back("LOCATIONPAIRITEMNO");
    std::string from;

    this->generateJoinString("=FAREFOCUSLOCATIONPAIR",
                                 "p",
                                 "LEFT OUTER JOIN",
                                 "=FAREFOCUSLOCATIONPAIRDETAIL",
                                 "c",
                                 joinFields,
                                 from);

    this->From(from);

    this->Where("p.LOCATIONPAIRITEMNO = %1n"
                " and %2n <= EXPIREDATE");

    this->OrderBy("ORDERNO");
    // callback to adjust query
    adjustBaseSQL();

    ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static FareFocusLocationPairInfo* mapRow(Row* row, FareFocusLocationPairInfo* infoPrev)
  {
    FareFocusLocationPairInfo* info(nullptr);
    uint64_t locationPairItemNo(row->getLong(LOCATIONPAIRITEMNO));
    if (infoPrev && locationPairItemNo == infoPrev->locationPairItemNo())
    {
      info = infoPrev;
    }
    else
    {
      info = new FareFocusLocationPairInfo;
      info->locationPairItemNo() = row->getLong(LOCATIONPAIRITEMNO);
      info->createDate() = row->getDate(CREATEDATE);
      info->expireDate() = row->getDate(EXPIREDATE);
    }
    FareFocusLocationPairDetailInfo* detail(new FareFocusLocationPairDetailInfo);
    if (!row->isNull(LOC1))
      detail->loc1().loc() = row->getString(LOC1);
    if (!row->isNull(LOC1TYPE))
      detail->loc1().locType() = row->getChar(LOC1TYPE);
    if (!row->isNull(LOC2))
      detail->loc2().loc() = row->getString(LOC2);
    if (!row->isNull(LOC2TYPE))
      detail->loc2().locType() = row->getChar(LOC2TYPE);
    info->details().push_back(detail);
    return info;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <typename QUERYCLASS>
class QueryGetFareFocusLocationPairHistoricalSQLStatement
    : public QueryGetFareFocusLocationPairSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("p.LOCATIONPAIRITEMNO = %1n"
                " and %2n <= EXPIREDATE"
                " and %3n >= CREATEDATE");
  }

  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

template <class QUERYCLASS>
class QueryGetAllFareFocusLocationPairSQLStatement
    : public QueryGetFareFocusLocationPairSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd >= CREATEDATE"
                " and %cd <= EXPIREDATE");

    this->OrderBy("p.LOCATIONPAIRITEMNO, ORDERNO");
  }
};

} // tse

