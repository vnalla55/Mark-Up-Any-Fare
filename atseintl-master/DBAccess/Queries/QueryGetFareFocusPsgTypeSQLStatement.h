#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareFocusPsgType.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <typename QUERYCLASS>
class QueryGetFareFocusPsgTypeSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    PSGTYPEITEMNO
    , CREATEDATE
    , EXPIREDATE
    // child
    , PSGTYPE

    , NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    Command("select p.PSGTYPEITEMNO, p.CREATEDATE, p.EXPIREDATE, ch.PSGTYPE ");
    std::vector<std::string> joinFields;
    joinFields.reserve(4);
    joinFields.push_back("PSGTYPEITEMNO");
    std::string from;
    this->generateJoinString("=FAREFOCUSPSGTYPE",
                             "p",
                             "LEFT OUTER JOIN",
                             "=FAREFOCUSPSGTYPEDETAIL",
                             "ch",
                             joinFields,
                             from);
    this->From(from);
    this->Where("p.PSGTYPEITEMNO = %1n"
                " and %2n <= EXPIREDATE");
    this->OrderBy("ORDERNO");
    // callback to adjust query
    adjustBaseSQL();

    ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static FareFocusPsgTypeInfo* mapRow(Row* row, FareFocusPsgTypeInfo* infoPrev)
  {
    FareFocusPsgTypeInfo* info(nullptr);
    uint64_t psgTypeItemNo(row->getLong(PSGTYPEITEMNO));
    if (infoPrev && psgTypeItemNo == infoPrev->psgTypeItemNo())
    {
      info = infoPrev;
    }
    else
    {
      info = new FareFocusPsgTypeInfo;
      info->psgTypeItemNo() = row->getLong(PSGTYPEITEMNO);
      info->createDate() = row->getDate(CREATEDATE);
      info->expireDate() = row->getDate(EXPIREDATE);
    }
    if (!row->isNull(PSGTYPE))
      info->psgType().push_back(row->getString(PSGTYPE));
    return info;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <typename QUERYCLASS>
class QueryGetFareFocusPsgTypeHistoricalSQLStatement
    : public QueryGetFareFocusPsgTypeSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("p.PSGTYPEITEMNO = %1n"
                " and %2n <= EXPIREDATE"
                " and %3n >= CREATEDATE");
  }

  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

template <class QUERYCLASS>
class QueryGetAllFareFocusPsgTypeSQLStatement
    : public QueryGetFareFocusPsgTypeSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd >= CREATEDATE"
                " and %cd <= EXPIREDATE");

    this->OrderBy("p.PSGTYPEITEMNO, ORDERNO");
  }
};

} // tse

