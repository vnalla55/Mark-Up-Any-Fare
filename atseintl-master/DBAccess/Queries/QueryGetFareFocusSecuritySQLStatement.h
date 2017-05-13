#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareFocusSecurity.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <typename QUERYCLASS>
class QueryGetFareFocusSecuritySQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    SECURITYITEMNO
    , CREATEDATE
    , EXPIREDATE
    // child
    , PSEUDOCITY
    , PSEUDOCITYTYPE

    , NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    Command("select p.SECURITYITEMNO, CREATEDATE, EXPIREDATE, PSEUDOCITY, PSEUDOCITYTYPE ");
    std::vector<std::string> joinFields;
    joinFields.reserve(4);
    joinFields.push_back("SECURITYITEMNO");
    std::string from;
    this->generateJoinString("=FAREFOCUSSECURITY",
                             "p",
                             "LEFT OUTER JOIN",
                             "=FAREFOCUSSECURITYDETAIL",
                             "c",
                             joinFields,
                             from);
    this->From(from);
    this->Where("p.SECURITYITEMNO = %1n"
                " and %2n <= EXPIREDATE");
    this->OrderBy("ORDERNO");
    // callback to adjust query
    adjustBaseSQL();

    ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static FareFocusSecurityInfo* mapRow(Row* row, FareFocusSecurityInfo* infoPrev)
  {
    FareFocusSecurityInfo* info(nullptr);
    uint64_t securityItemNo(row->getLong(SECURITYITEMNO));
    if (infoPrev && securityItemNo == infoPrev->securityItemNo())
    {
      info = infoPrev;
    }
    else
    {
      info = new FareFocusSecurityInfo;
      info->securityItemNo() = row->getLong(SECURITYITEMNO);
      info->createDate() = row->getDate(CREATEDATE);
      info->expireDate() = row->getDate(EXPIREDATE);
    }
    FareFocusSecurityDetailInfo* detail(new FareFocusSecurityDetailInfo);
    detail->pseudoCity() = row->getString(PSEUDOCITY);
    detail->pseudoCityType() = row->getChar(PSEUDOCITYTYPE);
    info->details().push_back(detail);
    return info;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <typename QUERYCLASS>
class QueryGetFareFocusSecurityHistoricalSQLStatement
    : public QueryGetFareFocusSecuritySQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("p.SECURITYITEMNO = %1n"
                " and %2n <= EXPIREDATE"
                " and %3n >= CREATEDATE");
  }

  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

template <class QUERYCLASS>
class QueryGetAllFareFocusSecuritySQLStatement
    : public QueryGetFareFocusSecuritySQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd >= CREATEDATE"
                " and %cd <= EXPIREDATE");

    this->OrderBy("p.SECURITYITEMNO, ORDERNO");
  }
};

} // tse

