//----------------------------------------------------------------------------
//  File:           QueryGetAllBrandedCarriers.cpp
//  Description:    QueryGetAllBrandedCarriers
//  Created:        2/28/2008
//  Authors:        Mauricio Dantas
//  Updates:
//
// � 2008, Sabre Inc. All rights reserved.  This software/documentation is the confidential
// and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
// or transfer of this software/documentation, in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/Queries/QueryGetAllBrandedCarriers.h"

#include "Common/Global.h"
#include "Common/TseConsts.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetAllBrandedCarriersSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetAllBrandedCarriers::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetAllBrandedCarriers"));
std::string QueryGetAllBrandedCarriers::_baseSQL;
bool QueryGetAllBrandedCarriers::_isInitialized = false;
SQLQueryInitializerHelper<QueryGetAllBrandedCarriers> g_GetAllBrandedCarriers;

const char*
QueryGetAllBrandedCarriers::getQueryName() const
{
  return "GETALLBRANDEDCARRIERS";
}
void
QueryGetAllBrandedCarriers::initialize()
{
  if (!_isInitialized)
  {
    QueryGetAllBrandedCarriersSQLStatement<QueryGetAllBrandedCarriers> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETALLBRANDEDEDCARRIERS");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
} // initialize()

void
QueryGetAllBrandedCarriers::findAllBrandedCarriers(std::vector<tse::BrandedCarrier*>& infos)
{
  Row* row;
  DBResultSet res(_dbAdapt);
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  while ((row = res.nextRow()))
  {
    infos.push_back(
        QueryGetAllBrandedCarriersSQLStatement<QueryGetAllBrandedCarriers>::mapRowToBrandedCarrier(
            row));
  }
  LOG4CXX_INFO(_logger,
               "GETALLBRANDEDCARRIERS: NumRows = " << res.numRows() << " Time = " << stopTimer()
                                                   << " (" << stopCPU() << ")");
  res.freeResult();
} // findAllBrandedCarriers()
}
