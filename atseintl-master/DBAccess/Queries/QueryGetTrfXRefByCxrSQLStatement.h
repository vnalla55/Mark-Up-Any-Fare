//----------------------------------------------------------------------------
//          File:           QueryGetTrfXRefByCxrSQLStatement.h
//          Description:    QueryGetTrfXRefByCxrSQLStatement
//          Created:        10/5/2007
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

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTrfXRefByCxr.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetTrfXRefByCxrSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTrfXRefByCxrSQLStatement() {}
  virtual ~QueryGetTrfXRefByCxrSQLStatement() {}

  enum ColumnIndexes
  {
    VENDOR = 0,
    CARRIER,
    TARIFFCROSSREFTYPE,
    GLOBALDIR,
    FARETARIFF,
    FARETARIFFCODE,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    TARIFFCAT,
    RULETARIFF,
    RULETARIFFCODE,
    GOVERNINGTARIFF,
    GOVERNINGTARIFFCODE,
    ROUTINGTARIFF1,
    ROUTINGTARIFF1CODE,
    ROUTINGTARIFF2,
    ROUTINGTARIFF2CODE,
    ADDONTARIFF1,
    ADDONTARIFF1CODE,
    ADDONTARIFF2,
    ADDONTARIFF2CODE,
    ZONENO,
    ZONEVENDOR,
    ZONETYPE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,CARRIER,TARIFFCROSSREFTYPE,GLOBALDIR, FARETARIFF,"
                  "       FARETARIFFCODE,CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,"
                  "       TARIFFCAT,RULETARIFF,RULETARIFFCODE,GOVERNINGTARIFF,"
                  "       GOVERNINGTARIFFCODE,ROUTINGTARIFF1,ROUTINGTARIFF1CODE,"
                  "       ROUTINGTARIFF2,ROUTINGTARIFF2CODE,ADDONTARIFF1,ADDONTARIFF1CODE,"
                  "       ADDONTARIFF2,ADDONTARIFF2CODE,ZONENO,ZONEVENDOR,ZONETYPE");
    this->From("=TARIFFCROSSREF");
    this->Where("VALIDITYIND = 'Y' "
                "    and CARRIER = %1q "
                "    and VENDOR = %2q "
                "    and TARIFFCROSSREFTYPE = %3q "
                "    and EXPIREDATE >= %cd");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR,TARIFFCROSSREFTYPE,CARRIER,FARETARIFF,RULETARIFFCODE,CREATEDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::TariffCrossRefInfo* mapRowToTariffCrossRefInfo(Row* row)
  {
    tse::TariffCrossRefInfo* pTar = new tse::TariffCrossRefInfo;

    pTar->_addonTariff1 = row->getInt(ADDONTARIFF1);
    pTar->_addonTariff2 = row->getInt(ADDONTARIFF2);
    pTar->_addonTariff1Code = row->getString(ADDONTARIFF1CODE);
    pTar->_addonTariff2Code = row->getString(ADDONTARIFF2CODE);
    pTar->_carrier = row->getString(CARRIER);

    std::string crossType = row->getString(TARIFFCROSSREFTYPE);
    if (crossType == "D")
      pTar->_crossRefType = DOMESTIC;
    else if (crossType == "I")
      pTar->_crossRefType = INTERNATIONAL;

    pTar->effDate() = row->getDate(EFFDATE);
    pTar->createDate() = row->getDate(CREATEDATE);
    pTar->expireDate() = row->getDate(EXPIREDATE);
    pTar->discDate() = row->getDate(DISCDATE);
    pTar->_fareTariff = row->getInt(FARETARIFF);
    pTar->_fareTariffCode = row->getString(FARETARIFFCODE);
    std::string gd = row->getString(GLOBALDIR);
    strToGlobalDirection(pTar->_globalDirection, gd);
    pTar->_governingTariff = row->getInt(GOVERNINGTARIFF);
    pTar->_governingTariffCode = row->getString(GOVERNINGTARIFFCODE);
    pTar->_routingTariff1 = row->getInt(ROUTINGTARIFF1);
    pTar->_routingTariff1Code = row->getString(ROUTINGTARIFF1CODE);
    pTar->_routingTariff2 = row->getInt(ROUTINGTARIFF2);
    pTar->_routingTariff2Code = row->getString(ROUTINGTARIFF2CODE);
    pTar->_ruleTariff = row->getInt(RULETARIFF);
    pTar->_ruleTariffCode = row->getString(RULETARIFFCODE);
    pTar->_tariffCat = row->getInt(TARIFFCAT);
    pTar->_vendor = row->getString(VENDOR);
    pTar->_zoneNo = row->getString(ZONENO);
    pTar->_zoneVendor = row->getString(ZONEVENDOR);
    pTar->_zoneType = row->getChar(ZONETYPE);

    return pTar;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // Class QueryGetTrfXRefByCxrSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetTrfXRefByCxrHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetTrfXRefByCxrHistoricalSQLStatement
    : public QueryGetTrfXRefByCxrSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("VALIDITYIND = 'Y' "
                " and CARRIER = %1q "
                " and VENDOR = %2q "
                " and TARIFFCROSSREFTYPE = %3q "
                " and %4n <= EXPIREDATE"
                " and %5n >= CREATEDATE");
    this->OrderBy(
        "VENDOR, TARIFFCROSSREFTYPE, CARRIER, FARETARIFF, RULETARIFFCODE, CREATEDATE desc ");
  }
};
} // tse
