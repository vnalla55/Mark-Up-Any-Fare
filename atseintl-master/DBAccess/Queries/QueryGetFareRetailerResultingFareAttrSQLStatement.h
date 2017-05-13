#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareRetailerResultingFareAttr.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <typename QUERYCLASS>
class QueryGetFareRetailerResultingFareAttrSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    RESULTINGFAREATTRITEMNO
    , REDISTRIBUTEIND
    , UPDATEIND
    , SELLIND
    , TICKETIND
    , VIEWNETIND
    , TKTDESIGNATOR
    , ACCOUNTCDTYPE
    , ACCOUNTCD
    , CREATEDATE
    , EXPIREDATE
    , LASTMODDATE
    , NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    Command("select RESULTINGFAREATTRITEMNO, REDISTRIBUTEIND, UPDATEIND, SELLIND, TICKETIND, VIEWNETIND, "
            " TKTDESIGNATOR, ACCOUNTCDTYPE, ACCOUNTCD, CREATEDATE, EXPIREDATE, LASTMODDATE "
            "from FARERETAILERRESULTINGFAREATTR ");

    this->Where("RESULTINGFAREATTRITEMNO = %1n"
                " and %2n <= EXPIREDATE");

    // callback to adjust query
    adjustBaseSQL();

    ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static FareRetailerResultingFareAttrInfo* mapRow(Row* row, FareRetailerResultingFareAttrInfo*)
  {
    FareRetailerResultingFareAttrInfo* info(new FareRetailerResultingFareAttrInfo);
    info->resultingFareAttrItemNo() = row->getLong(RESULTINGFAREATTRITEMNO);
    if (!row->isNull(REDISTRIBUTEIND))
      info->redistributeInd() = row->getChar(REDISTRIBUTEIND);
    if (!row->isNull(UPDATEIND))
      info->updateInd() = row->getChar(UPDATEIND);
    if (!row->isNull(SELLIND))
      info->sellInd() = row->getChar(SELLIND);
    if (!row->isNull(TICKETIND))
      info->ticketInd() = row->getChar(TICKETIND);
    if (!row->isNull(VIEWNETIND))
      info->viewNetInd() = row->getChar(VIEWNETIND);
    if (!row->isNull(TKTDESIGNATOR))
      info->tktDesignator() = row->getString(TKTDESIGNATOR);
    if (!row->isNull(ACCOUNTCDTYPE))
      info->accountCdType() = row->getChar(ACCOUNTCDTYPE);
    if (!row->isNull(ACCOUNTCD))
      info->accountCd() = row->getString(ACCOUNTCD);
    info->createDate() = row->getDate(CREATEDATE);
    info->expireDate() = row->getDate(EXPIREDATE);
    info->lastModDate() = row->getDate(LASTMODDATE);

    return info;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <typename QUERYCLASS>
class QueryGetFareRetailerResultingFareAttrHistoricalSQLStatement
    : public QueryGetFareRetailerResultingFareAttrSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("RESULTINGFAREATTRITEMNO = %1n"
                " and %2n <= EXPIREDATE"
                " and %3n >= CREATEDATE");
  }
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

template <class QUERYCLASS>
class QueryGetAllFareRetailerResultingFareAttrSQLStatement
    : public QueryGetFareRetailerResultingFareAttrSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd >= CREATEDATE"
                " and %cd <= EXPIREDATE");
  }
};

} // tse



