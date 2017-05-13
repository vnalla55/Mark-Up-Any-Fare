//----------------------------------------------------------------------------
//          File:           QueryGetRoutingSQLStatement.h
//          Description:    QueryGetRoutingSQLStatement
//          Created:        10/7/2007
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
#include "DBAccess/Queries/QueryGetRouting.h"
#include "DBAccess/SQLStatement.h"
#include "DBAccess/SQLStatementHelper.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetRoutingSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetRoutingSQLStatement() {}
  virtual ~QueryGetRoutingSQLStatement() {}

  enum ColumnIndexes
  { VENDOR = 0,
    CARRIER,
    ROUTINGTARIFF,
    ROUTING,
    CREATEDATE,
    EFFDATE,
    DISCDATE,
    EXPIREDATE,
    NOOFMAPS,
    LINKNO,
    NOOFHEADERS,
    NOOFRESTRICTIONS,
    NOOFTEXTS,
    BATCHCI,
    VALIDITYIND,
    INHIBIT,
    DIRECTIONALIND,
    DOMRTGVALIND,
    COMMONPOINTIND,
    JOINTROUTINGOPT,
    ENTRYEXITPOINTIND,
    UNTICKETEDPOINTIND,
    LNKMAPSEQUENCE,
    LOC1NO,
    LOC1TYPE,
    LOCTAG,
    NEXTLOCNO,
    ALTLOCNO,
    LOC1,
    LOCALROUTING,
    NATION,
    NUMBEROFCOLUMNS }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select rt.VENDOR,rt.CARRIER,rt.ROUTINGTARIFF,rt.ROUTING,rt.CREATEDATE,"
                  " rt.EFFDATE,rt.DISCDATE,rt.EXPIREDATE,NOOFMAPS,LINKNO,NOOFHEADERS,"
                  " NOOFRESTRICTIONS,NOOFTEXTS,BATCHCI,VALIDITYIND,INHIBIT,DIRECTIONALIND,"
                  " DOMRTGVALIND,COMMONPOINTIND,JOINTROUTINGOPT,ENTRYEXITPOINTIND,"
                  " UNTICKETEDPOINTIND,LNKMAPSEQUENCE,LOC1NO,LOC1TYPE,LOCTAG,NEXTLOCNO,"
                  " ALTLOCNO,LOC1,LOCALROUTING,mkt.NATION");

    this->From(" =ROUTING rt"
               " left outer join =MAPSEQ map"
               "    on rt.VENDOR = map.VENDOR "
               "    and rt.CARRIER = map.CARRIER "
               "    and rt.ROUTINGTARIFF = map.ROUTINGTARIFF "
               "    and rt.ROUTING = map.ROUTING "
               "    and rt.EFFDATE = map.EFFDATE "
               "    and rt.CREATEDATE = map.CREATEDATE"
               " left outer join =MARKET mkt"
               "    on mkt.MARKET = map.LOC1"
               "    and %cd between mkt.CREATEDATE and mkt.EXPIREDATE");

    this->Where(" rt.VENDOR = %1q"
                " and rt.CARRIER = %2q"
                " and rt.ROUTINGTARIFF = %3n"
                " and rt.ROUTING = %4q"
                " and %cd <= rt.EXPIREDATE"
                " and (   mkt.MARKET is NULL "
                "      or map.LOC1TYPE = 'C' ) ");

    if (DataManager::forceSortOrder())
      this->OrderBy(
          "VENDOR,CARRIER,ROUTINGTARIFF,ROUTING,EFFDATE,CREATEDATE,LNKMAPSEQUENCE,LOC1TYPE,LOC1");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::RoutingMap* mapRowToRoutingMap(Row* row)
  {
    tse::RoutingMap* routingMap = new tse::RoutingMap;

    routingMap->vendor() = row->getString(VENDOR);
    routingMap->carrier() = row->getString(CARRIER);
    routingMap->routingTariff() = row->getInt(ROUTINGTARIFF);
    routingMap->routing() = row->getString(ROUTING);
    routingMap->effDate() = row->getDate(EFFDATE);
    routingMap->lnkmapsequence() = row->getInt(LNKMAPSEQUENCE);
    routingMap->loc1No() = row->getInt(LOC1NO);
    routingMap->loc1().locType() = row->getChar(LOC1TYPE);
    routingMap->loc1().loc() = row->getString(LOC1);
    routingMap->loctag() = row->getChar(LOCTAG);
    routingMap->nextLocNo() = row->getInt(NEXTLOCNO);
    routingMap->altLocNo() = row->getInt(ALTLOCNO);
    routingMap->localRouting() = row->getString(LOCALROUTING);

    if (!row->isNull(NATION))
    {
      routingMap->nation() = row->getString(NATION);
    }

    return routingMap;
  } // mapRowToRoutingMap()
  static tse::Routing* mapRowToRouting(Row* row, Routing* routingPrev)
  {
    VendorCode vendor = row->getString(VENDOR);
    CarrierCode carrier = row->getString(CARRIER);
    TariffNumber routingTariff = row->getInt(ROUTINGTARIFF);
    RoutingNumber routingNo = row->getString(ROUTING);
    DateTime effDate = row->getDate(EFFDATE);
    DateTime createDate = row->getDate(CREATEDATE);

    // now check if the parent has changed, if yes then push the routing back.
    if (routingPrev != nullptr && routingPrev->vendor() == vendor && routingPrev->carrier() == carrier &&
        routingPrev->routingTariff() == routingTariff && routingPrev->routing() == routingNo &&
        routingPrev->effDate() == effDate && routingPrev->createDate() == createDate)
    {
      tse::RoutingMap* rmaps = mapRowToRoutingMap(row);
      routingPrev->rmaps().push_back(rmaps);
      return routingPrev;
    }
    else
    {
      DBAccess::SQLStatementHelper helper;

      tse::Routing* routing = new tse::Routing;

      routing->vendor() = vendor;
      routing->carrier() = carrier;
      routing->routingTariff() = routingTariff;
      routing->routing() = routingNo;
      routing->effDate() = effDate;
      routing->createDate() = createDate;
      routing->discDate() = row->getDate(DISCDATE);
      routing->expireDate() = row->getDate(EXPIREDATE);
      routing->linkNo() = row->getInt(LINKNO);
      routing->noofheaders() = row->getInt(NOOFHEADERS);
      routing->noofRestrictions() = row->getInt(NOOFRESTRICTIONS);
      routing->nooftexts() = row->getInt(NOOFTEXTS);
      routing->validityInd() = row->getChar(VALIDITYIND);
      routing->inhibit() = row->getChar(INHIBIT);
      routing->directionalInd() = row->getChar(DIRECTIONALIND);
      routing->domRtgvalInd() = row->getChar(DOMRTGVALIND);
      routing->commonPointInd() = row->getChar(COMMONPOINTIND);
      routing->jointRoutingOpt() = row->getChar(JOINTROUTINGOPT);
      routing->entryExitPointInd() = row->getChar(ENTRYEXITPOINTIND);
      routing->unticketedPointInd() = row->getChar(UNTICKETEDPOINTIND);

      // Make sure that there is a map
      // if there is no map row then the below column will return 0
      if (!row->isNull(LNKMAPSEQUENCE))
      {
        tse::RoutingMap* rmaps = mapRowToRoutingMap(row);
        routing->rmaps().push_back(rmaps);
      }
      return routing;
    }
  } // mapRowToRouting()
private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetRoutingSQLStatement

template <class QUERYCLASS>
class QueryGetRoutingHistoricalSQLStatement : public QueryGetRoutingSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command(
        "("
        "select rth.VENDOR,rth.CARRIER,rth.ROUTINGTARIFF,rth.ROUTING,rth.CREATEDATE,"
        " rth.EFFDATE,rth.DISCDATE,rth.EXPIREDATE,NOOFMAPS,LINKNO,NOOFHEADERS,"
        " NOOFRESTRICTIONS,NOOFTEXTS,BATCHCI,VALIDITYIND,INHIBIT,DIRECTIONALIND,"
        " DOMRTGVALIND,COMMONPOINTIND,JOINTROUTINGOPT,ENTRYEXITPOINTIND,"
        " UNTICKETEDPOINTIND,LNKMAPSEQUENCE,LOC1NO,LOC1TYPE,LOCTAG,NEXTLOCNO,"
        " ALTLOCNO,LOC1,LOCALROUTING,mkth.NATION");

    partialStatement.From("=ROUTINGH rth"
                          " left outer join =MAPSEQH maph"
                          "     on rth.VENDOR = maph.VENDOR "
                          "     and rth.CARRIER = maph.CARRIER "
                          "     and rth.ROUTINGTARIFF = maph.ROUTINGTARIFF "
                          "     and rth.ROUTING = maph.ROUTING "
                          "     and rth.EFFDATE = maph.EFFDATE "
                          "     and rth.CREATEDATE = maph.CREATEDATE "
                          " left outer join =MARKET mkth"
                          "     on mkth.MARKET = maph.LOC1"
                          "     and rth.EFFDATE between mkth.EFFDATE and mkth.EXPIREDATE");

    partialStatement.Where(" rth.VENDOR = %1q"
                           " and rth.CARRIER = %2q"
                           " and rth.ROUTINGTARIFF = %3n"
                           " and rth.ROUTING = %4q"
                           " and %5n <= rth.EXPIREDATE"
                           " and (   %6n >= rth.CREATEDATE"
                           "      or %7n >= rth.EFFDATE)"
                           " and (   mkth.MARKET is NULL "
                           "      or maph.LOC1TYPE = 'C' ) )");

    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(
        " union all"
        " ("
        "select rt.VENDOR,rt.CARRIER,rt.ROUTINGTARIFF,rt.ROUTING,rt.CREATEDATE,"
        " rt.EFFDATE,rt.DISCDATE,rt.EXPIREDATE,NOOFMAPS,LINKNO,NOOFHEADERS,"
        " NOOFRESTRICTIONS,NOOFTEXTS,BATCHCI,VALIDITYIND,INHIBIT,DIRECTIONALIND,"
        " DOMRTGVALIND,COMMONPOINTIND,JOINTROUTINGOPT,ENTRYEXITPOINTIND,"
        " UNTICKETEDPOINTIND, LNKMAPSEQUENCE,LOC1NO,LOC1TYPE,LOCTAG,NEXTLOCNO,"
        " ALTLOCNO,LOC1,LOCALROUTING,mkt.NATION");

    partialStatement.From("=ROUTING rt"
                          " left outer join =MAPSEQ map"
                          "     on rt.VENDOR = map.VENDOR "
                          "     and rt.CARRIER = map.CARRIER "
                          "     and rt.ROUTINGTARIFF = map.ROUTINGTARIFF "
                          "     and rt.ROUTING = map.ROUTING "
                          "     and rt.EFFDATE = map.EFFDATE "
                          "     and rt.CREATEDATE = map.CREATEDATE "
                          " left outer join =MARKET mkt"
                          "     on mkt.MARKET = map.LOC1"
                          "     and rt.EFFDATE between mkt.EFFDATE and mkt.EXPIREDATE");

    partialStatement.Where(" rt.VENDOR = %8q"
                           " and rt.CARRIER = %9q"
                           " and rt.ROUTINGTARIFF = %10n"
                           " and rt.ROUTING = %11q"
                           " and %12n <= rt.EXPIREDATE"
                           " and (   %13n >= rt.CREATEDATE"
                           "      or %14n >= rt.EFFDATE)"
                           " and (    mkt.MARKET is NULL "
                           "      or  map.LOC1TYPE = 'C' ) )");

    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
  }

  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

template <class QUERYCLASS>
class QueryGetRoutingRestSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetRoutingRestSQLStatement() {}
  virtual ~QueryGetRoutingRestSQLStatement() {}

  enum ColumnIndexes
  { VENDOR = 0,
    CARRIER,
    ROUTINGTARIFF,
    ROUTING,
    EFFDATE,
    CREATEDATE,
    RESTRSEQNO,
    RESTRICTION,
    MARKET1,
    MARKET2,
    VIAMARKET,
    VIACARRIER,
    MARKETAPPL,
    NEGVIAAPPL,
    VIATYPE,
    NONSTOPDIRECTIND,
    AIRSURFACEIND,
    MPM,
    MARKET1TYPE,
    MARKET2TYPE,
    NUMBEROFCOLUMNS }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,CARRIER,ROUTINGTARIFF,ROUTING,EFFDATE,CREATEDATE,"
                  " RESTRSEQNO,RESTRICTION,MARKET1,MARKET2,VIAMARKET,VIACARRIER,"
                  " MARKETAPPL,NEGVIAAPPL,VIATYPE,NONSTOPDIRECTIND,AIRSURFACEIND,"
                  " MPM,MARKET1TYPE,MARKET2TYPE");
    this->From(" =ROUTINGREST");
    this->Where(" VENDOR = %1q"
                " and CARRIER = %2q"
                " and ROUTINGTARIFF = %3n"
                " and ROUTING = %4q"
                " and EFFDATE = %5n"
                " and CREATEDATE = %6n");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR,CARRIER,ROUTINGTARIFF,ROUTING,EFFDATE,CREATEDATE,RESTRSEQNO");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::RoutingRestriction* mapRowToRoutingRestriction(Row* row)
  {
    tse::RoutingRestriction* rest = new tse::RoutingRestriction;

    rest->restrSeqNo() = row->getInt(RESTRSEQNO);
    rest->restriction() = row->getString(RESTRICTION);
    rest->marketAppl() = row->getChar(MARKETAPPL);
    rest->negViaAppl() = row->getChar(NEGVIAAPPL);
    rest->viaType() = row->getChar(VIATYPE);
    rest->nonStopDirectInd() = row->getChar(NONSTOPDIRECTIND);
    rest->airSurfaceInd() = row->getChar(AIRSURFACEIND);
    rest->market1() = row->getString(MARKET1);
    rest->market2() = row->getString(MARKET2);
    rest->viaMarket() = row->getString(VIAMARKET);
    rest->viaCarrier() = row->getString(VIACARRIER);
    rest->mpm() = row->getInt(MPM);
    rest->market1type() = row->getChar(MARKET1TYPE);
    rest->market2type() = row->getChar(MARKET2TYPE);

    return rest;
  } // mapRowToRoutingRestriction()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

template <class QUERYCLASS>
class QueryGetRoutingRestHistoricalSQLStatement : public QueryGetRoutingRestSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("("
                             "select VENDOR,CARRIER,ROUTINGTARIFF,ROUTING,EFFDATE,CREATEDATE,"
                             " RESTRSEQNO,RESTRICTION,MARKET1,MARKET2,VIAMARKET,VIACARRIER,"
                             " MARKETAPPL,NEGVIAAPPL,VIATYPE,NONSTOPDIRECTIND,AIRSURFACEIND,"
                             " MPM,MARKET1TYPE,MARKET2TYPE");
    partialStatement.From("=ROUTINGRESTH");
    partialStatement.Where(" VENDOR = %1q"
                           " and CARRIER = %2q"
                           " and ROUTINGTARIFF = %3n"
                           " and ROUTING = %4q"
                           " and EFFDATE = %5n"
                           " and CREATEDATE = %6n"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all"
                             " ("
                             "select VENDOR,CARRIER,ROUTINGTARIFF,ROUTING,EFFDATE,CREATEDATE,"
                             " RESTRSEQNO,RESTRICTION,MARKET1,MARKET2,VIAMARKET,VIACARRIER,"
                             " MARKETAPPL,NEGVIAAPPL,VIATYPE,NONSTOPDIRECTIND,AIRSURFACEIND,"
                             " MPM,MARKET1TYPE,MARKET2TYPE");
    partialStatement.From("=ROUTINGREST");
    partialStatement.Where(" VENDOR = %7q"
                           " and CARRIER = %8q"
                           " and ROUTINGTARIFF = %9n"
                           " and ROUTING = %10q"
                           " and EFFDATE = %11n"
                           " and CREATEDATE = %12n"
                           ")");
    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
  }

  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};
}
