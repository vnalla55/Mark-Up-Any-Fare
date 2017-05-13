#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareFocusRouting.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <typename QUERYCLASS>
class QueryGetFareFocusRoutingSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    ROUTINGITEMNO
    , CREATEDATE
    , EXPIREDATE
    // child
    , ROUTINGTYPE
    , ROUTINGNO

    , NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    Command("select p.ROUTINGITEMNO, CREATEDATE, EXPIREDATE, ROUTINGTYPE, ROUTINGNO ");
    std::vector<std::string> joinFields;
    joinFields.reserve(4);
    joinFields.push_back("ROUTINGITEMNO");
    std::string from;
    this->generateJoinString("=FAREFOCUSROUTING",
                             "p",
                             "LEFT OUTER JOIN",
                             "=FAREFOCUSROUTINGDETAIL",
                             "c",
                             joinFields,
                             from);
    this->From(from);
    this->Where("p.ROUTINGITEMNO = %1n"
                " and %2n <= EXPIREDATE");
    this->OrderBy("ORDERNO");
    // callback to adjust query
    adjustBaseSQL();

    ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static FareFocusRoutingInfo* mapRow(Row* row, FareFocusRoutingInfo* infoPrev)
  {
    FareFocusRoutingInfo* info(nullptr);
    uint64_t routingItemNo(row->getLong(ROUTINGITEMNO));
    if (infoPrev && routingItemNo == infoPrev->routingItemNo())
    {
      info = infoPrev;
    }
    else
    {
      info = new FareFocusRoutingInfo;
      info->routingItemNo() = row->getLong(ROUTINGITEMNO);
      info->createDate() = row->getDate(CREATEDATE);
      info->expireDate() = row->getDate(EXPIREDATE);
    }
    FareFocusRoutingDetailInfo* detail(new FareFocusRoutingDetailInfo);
    detail->routingType() = row->getChar(ROUTINGTYPE);
    detail->routingNo() = row->getString(ROUTINGNO);
    info->details().push_back(detail);
    return info;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <typename QUERYCLASS>
class QueryGetFareFocusRoutingHistoricalSQLStatement
    : public QueryGetFareFocusRoutingSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("p.ROUTINGITEMNO = %1n"
                " and %2n <= EXPIREDATE"
                " and %3n >= CREATEDATE");
  }

  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

template <class QUERYCLASS>
class QueryGetAllFareFocusRoutingSQLStatement
    : public QueryGetFareFocusRoutingSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd >= CREATEDATE"
                " and %cd <= EXPIREDATE");

    this->OrderBy("p.ROUTINGITEMNO, ORDERNO");
  }
};

} // tse

