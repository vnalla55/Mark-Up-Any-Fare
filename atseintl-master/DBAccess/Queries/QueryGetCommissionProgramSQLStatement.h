#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCommissionProgram.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <typename QUERYCLASS>
class QueryGetCommissionProgramSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    VENDOR
    , PROGRAMID
    , CREATEDATETIME
    , EFFDATETIME
    , DISCDATETIME
    , EXPIREDATETIME
    , PROGRAMNAME
    , POINTOFSALEITEMNO
    , POINTOFORIGINITEMNO
    , TRAVELDATESITEMNO
    , STARTTKTDATE
    , ENDTKTDATE
    , MARKETITEMNO
    , CONTRACTID
    , QSURCHARGEIND
    , THROUGHFAREIND
    , MAXCONNECTIONTIME
    , LANDAGREEMENTIND
    , INHIBIT
    , VALIDITYIND
    , NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    Command("select VENDOR, PROGRAMID, CREATEDATETIME, EFFDATETIME, DISCDATETIME, EXPIREDATETIME,"
            " PROGRAMNAME, POINTOFSALEITEMNO, POINTOFORIGINITEMNO, TRAVELDATESITEMNO, "
            " STARTTKTDATE, ENDTKTDATE, MARKETITEMNO, CONTRACTID, QSURCHARGEIND, THROUGHFAREIND,"
            " MAXCONNECTIONTIME, LANDAGREEMENTIND, INHIBIT, VALIDITYIND from COMMISSIONPROGRAM");

    this->Where("VENDOR = %1q and CONTRACTID = %2n and INHIBIT = 'N' and VALIDITYIND = 'Y'"
                " and EXPIREDATETIME >= %cd and DISCDATETIME >= %cd and EFFDATETIME <= DISCDATETIME");

    this->OrderBy("VENDOR, PROGRAMID");

    // callback to adjust query
    adjustBaseSQL();

    ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static CommissionProgramInfo* mapRow(Row* row)
  {
    CommissionProgramInfo* info(new CommissionProgramInfo);
    info->vendor() = row->getString(VENDOR);
    info->programId() = row->getLong(PROGRAMID);
    info->createDate() = row->getDate(CREATEDATETIME);
    if (!row->isNull(EFFDATETIME))
    {
      info->effDate() = row->getDate(EFFDATETIME);
    }
    if (!row->isNull(DISCDATETIME))
    {
      info->discDate() = row->getDate(DISCDATETIME);
    }
    if (!row->isNull(EXPIREDATETIME))
    {
      info->expireDate() = row->getDate(EXPIREDATETIME);
    }
    if (!row->isNull(PROGRAMNAME))
    {
      info->programName() = row->getString(PROGRAMNAME);
    }
    if (!row->isNull(PROGRAMNAME))
    {
      info->programName() = row->getString(PROGRAMNAME);
    }
    if (!row->isNull(POINTOFSALEITEMNO))
    {
      info->pointOfSaleItemNo() = row->getLong(POINTOFSALEITEMNO);
    }
    if (!row->isNull(POINTOFORIGINITEMNO))
    {
      info->pointOfOriginItemNo() = row->getLong(POINTOFORIGINITEMNO);
    }
    if (!row->isNull(TRAVELDATESITEMNO))
    {
      info->travelDatesItemNo() = row->getLong(TRAVELDATESITEMNO);
    }
    if (!row->isNull(STARTTKTDATE))
    {
      info->startTktDate() = row->getDate(STARTTKTDATE);
    }
    if (!row->isNull(ENDTKTDATE))
    {
      info->endTktDate() = row->getDate(ENDTKTDATE);
    }
    if (!row->isNull(MARKETITEMNO))
    {
      info->marketItemNo() = row->getLong(MARKETITEMNO);
    }
    if (!row->isNull(CONTRACTID))
    {
      info->contractId() = row->getLong(CONTRACTID);
    }
    if (!row->isNull(QSURCHARGEIND))
    {
      info->qSurchargeInd() = row->getChar(QSURCHARGEIND);
    }
    if (!row->isNull(THROUGHFAREIND))
    {
      info->throughFareInd() = row->getChar(THROUGHFAREIND);
    }
    info->maxConnectionTime() = row->getLong(MAXCONNECTIONTIME);
    if (!row->isNull(LANDAGREEMENTIND))
    {
      info->landAgreementInd() = row->getChar(LANDAGREEMENTIND);
    }
    if (!row->isNull(INHIBIT))
    {
      info->inhibit() = row->getChar(INHIBIT);
    }
    if (!row->isNull(VALIDITYIND))
    {
      info->validityInd() = row->getChar(VALIDITYIND);
    }
    return info;
  }

private:
  virtual void adjustBaseSQL() {}
};

template <typename QUERYCLASS>
class QueryGetCommissionProgramHistoricalSQLStatement
    : public QueryGetCommissionProgramSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where(" VENDOR = %1q and CONTRACTID = %2n and INHIBIT = 'N' and VALIDITYIND = 'Y'"
                " and %3n <= EXPIREDATETIME"
                " and %4n >= CREATEDATETIME");

    this->OrderBy("VENDOR, PROGRAMID");
  }

  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

template <class QUERYCLASS>
class QueryGetAllCommissionProgramSQLStatement
    : public QueryGetCommissionProgramSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("INHIBIT = 'N' and VALIDITYIND = 'Y'"
                " and %cd >= CREATEDATETIME"
                " and %cd <= EXPIREDATETIME");

    this->OrderBy("VENDOR, PROGRAMID");
  }
};

} // tse
