//----------------------------------------------------------------------------
//  File:           QueryGetAllATPResNationZones.cpp
//  Description:    QueryGetAllATPResNationZones
//  Created:        5/24/2006
// Authors:         Mike Lillis
//
//  Updates:
//
// � 2006, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetAllATPResNationZones.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetAllATPResNationZonesSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetAllATPResNationZones::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllATPResNationZones"));
std::string QueryGetAllATPResNationZones::_baseSQL;
bool QueryGetAllATPResNationZones::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllATPResNationZones> g_GetAllATPResNationZones;

const char*
QueryGetAllATPResNationZones::getQueryName() const
{
  return "GETALLATPRESNATIONZONES";
}

void
QueryGetAllATPResNationZones::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllATPResNationZonesSQLStatement<QueryGetAllATPResNationZones> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLATPRESNATIONZONES");

    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllATPResNationZones::findAllATPResNationZones(std::vector<ATPResNationZones*>& nZones)
{
  Row* row;
  DBResultSet res(_dbAdapt);

  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);

  ATPResNationZones* zones = nullptr;
  ATPResNationZones* zonesPrev = nullptr;
  while ((row = res.nextRow()))
  {
    zones = QueryGetAllATPResNationZonesSQLStatement<
        QueryGetAllATPResNationZones>::mapRowToATPResNationZones(row, zonesPrev);
    if (zones != zonesPrev)
      nZones.push_back(zones);

    zonesPrev = zones;
  }
  LOG4CXX_INFO(_logger,
               "GETALLATPRESNATIONZONES: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                     << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllGenericTaxCode()
}
