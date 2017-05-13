#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareFocusCarrier.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <typename QUERYCLASS>
class QueryGetFareFocusCarrierSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    CARRIERITEMNO
    , CREATEDATE
    , EXPIREDATE
    , CARRIER
    , NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    Command("select p.CARRIERITEMNO, p.CREATEDATE, p.EXPIREDATE, ch.CARRIER ");
    std::vector<std::string> joinFields;
    joinFields.reserve(4);
    joinFields.push_back("CARRIERITEMNO");
    std::string from;
    this->generateJoinString("=FAREFOCUSCARRIER",
                             "p",
                             "LEFT OUTER JOIN",
                             "=FAREFOCUSCARRIERDETAIL",
                             "ch",
                             joinFields,
                             from);
    this->From(from);
    this->Where("p.CARRIERITEMNO = %1n"
                " and %2n <= EXPIREDATE");
    this->OrderBy("ORDERNO");
    // callback to adjust query
    adjustBaseSQL();

    ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static FareFocusCarrierInfo* mapRow(Row* row, FareFocusCarrierInfo* infoPrev)
  {
    FareFocusCarrierInfo* info(nullptr);
    uint64_t carrierItemNo(row->getLong(CARRIERITEMNO));
    if (infoPrev && carrierItemNo == infoPrev->carrierItemNo())
    {
      info = infoPrev;
    }
    else
    {
      info = new FareFocusCarrierInfo;
      info->carrierItemNo() = row->getLong(CARRIERITEMNO);
      info->createDate() = row->getDate(CREATEDATE);
      info->expireDate() = row->getDate(EXPIREDATE);
    }
    info->carrier().push_back(row->getString(CARRIER));
    return info;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <typename QUERYCLASS>
class QueryGetFareFocusCarrierHistoricalSQLStatement
    : public QueryGetFareFocusCarrierSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("p.CARRIERITEMNO = %1n"
                " and %2n <= EXPIREDATE"
                " and %3n >= CREATEDATE");
  }

  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

template <class QUERYCLASS>
class QueryGetAllFareFocusCarrierSQLStatement
    : public QueryGetFareFocusCarrierSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd >= CREATEDATE"
                " and %cd <= EXPIREDATE");

    this->OrderBy("p.CARRIERITEMNO, ORDERNO");
  }
};

} // tse


