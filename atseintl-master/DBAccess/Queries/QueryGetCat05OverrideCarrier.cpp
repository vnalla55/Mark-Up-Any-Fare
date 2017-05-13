
/*----------------------------------------------------------------------------
  File:           QueryGetCat05OverrideCarrier.cpp
  Description:    QueryGetCat05OverrideCarrier
  Created:        04/22/2013
  Author:         Sashi Reddy

 Copyright 2013, Sabre Inc. All rights reserved. This software/documentation is the
 confidential and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
 or transfer of this software/documentation, in any medium, or incorporation of this
 software/documentation into any system or publication, is strictly prohibited
 ----------------------------------------------------------------------------*/

#include "DBAccess/Queries/QueryGetCat05OverrideCarrier.h"

#include "Common/Global.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCat05OverrideCarrierSQLStatement.h"

namespace tse
{
log4cxx::LoggerPtr
QueryGetCat05OverrideCarrier::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SQLQuery.GetCat05OverrideCarrier"));
std::string QueryGetCat05OverrideCarrier::_baseSQL;
bool
QueryGetCat05OverrideCarrier::_isInitialized(false);
SQLQueryInitializerHelper<QueryGetCat05OverrideCarrier> _getCat05OverrideCarrier;

const char*
QueryGetCat05OverrideCarrier::getQueryName() const
{
  return "GETCAT05OVERRIDECARRIER";
}

void
QueryGetCat05OverrideCarrier::initialize()
{
  if (!_isInitialized)
  {
    QueryGetCat05OverrideCarrierSQLStatement<QueryGetCat05OverrideCarrier> sqlStatement;
    _baseSQL = sqlStatement.RegisterColumnsAndBaseSQL("GETCAT05OVERRIDECARRIER");
    substTableDef(&_baseSQL);
    _isInitialized = true;
  }
}
void
QueryGetCat05OverrideCarrier::findCat05OverrideCarriers(
    std::vector<tse::Cat05OverrideCarrier*>& lst, const PseudoCityCode& pcityCode)
{

  Row* row(nullptr);
  DBResultSet res(_dbAdapt);

  std::string pCity(pcityCode.c_str());

  substParm(pCity, 1, true);
  // substParm("8HF6",1);
  // substParm(1, "8HF6");
  // std::cerr<< "Pcitycode:" << pcityCode<< std::endl;
  // std::cerr << this->getSQL();
  LOG4CXX_EXECUTECODE_INFO(_logger, startTimer());
  res.executeQuery(this);
  // std::cerr << this->getSQL();
  Cat05OverrideCarrier* cat05Cxr = nullptr;

  while ((row = res.nextRow()))
  {
    if (cat05Cxr == nullptr)
      cat05Cxr = new Cat05OverrideCarrier();

    QueryGetCat05OverrideCarrierSQLStatement<
        QueryGetCat05OverrideCarrier>::mapRowToCat05OverrideCarrier(row, cat05Cxr);
  }
  if (cat05Cxr)
    lst.push_back(cat05Cxr);
  LOG4CXX_INFO(_logger,
               "GETCAT05OVERRIDECARRIER: NumCarriers: "
                   << res.numRows() << " Time = " << stopTimer() << " (" << stopCPU() << ") ms");
  res.freeResult();
}
}
