//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "Diagnostic/DiagTools.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/MetricsUtil.h"
#include "Common/TseSrvStats.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/Trx.h"
#include "DBAccess/DataManager.h"

#include <boost/algorithm/string/case_conv.hpp>

#include <stdlib.h> // getenv

namespace tse
{
namespace
{
ConfigurableValue<std::string>
port("SERVER_SOCKET_ADP", "PORT");
}

namespace utils
{

bool
shouldDiagBePrintedForTrx(const Trx& trx)
{
  if (trx.recordMetrics() == true)
  {
    return true;
  }

  if (trx.diagnostic().diagnosticType() == DiagnosticNone)
  {
    return false;
  }

  return true;
}

std::string
getHostname()
{
  return getenv("HOSTNAME");
}

std::string
getPortNumber()
{
  return port.getValue();
}

std::string
getDbInfo()
{

  std::string dbInfo;
  const DataManager& dbMan = DataManager::instance();
  const tse::ConfigMan& dbConfig = dbMan.dbConfig();
  std::string dbConnection;

  if (!dbConfig.getValue("DEFAULT", dbConnection, "CONNECTION"))
  {
    dbConnection = "Default";
  }

  std::string dbServer;
  std::string dbPort;

  if (dbConfig.getValue("HOST", dbServer, dbConnection))
  {
    dbConfig.getValue("PORT", dbPort, dbConnection);
    std::transform(dbServer.begin(), dbServer.end(), dbServer.begin(), (int (*)(int))std::toupper);
    dbInfo += "DB SERVER/PORT: " + dbServer + "/" + dbPort + "\n";
  }
  else // database in CERT and PROD
  {
    dbInfo = "DATABASE:" + TseSrvStats::getCurrentDatabase() + "\n";
  }

  return dbInfo;
}

std::string
getMetricsInfo(const Trx& trx)
{
  std::ostringstream metrics;
  if (trx.recordMetrics())
  {
    MetricsUtil::trxLatency(metrics, trx);
  }
  return metrics.str();
}

std::string
getDiagnosticPrintout(Trx& trx, const std::string& build_label_string)
{
  std::string hostname = getHostname();
  boost::to_upper(hostname);
  std::string port = getPortNumber();
  std::string hostNPort = "HOSTNAME : " + hostname + " PORT : " + port + "\n";

  std::string dbInfo = getDbInfo();

  std::string buildMsg = std::string("BASELINE: ") + build_label_string + "\n\n";
  boost::to_upper(buildMsg);

  std::string metrics = getMetricsInfo(trx);

  return hostNPort + dbInfo + buildMsg + trx.diagnostic().toString() + metrics;
}

bool
canTruncateDiagnostic(const PricingTrx& trx)
{
  if (trx.diagnostic().diagParamMapItemPresent("NO_TRUNCATE"))
  {
    return false;
  }
  return true;
}

std::string
truncateDiagnostic(const std::string& str, size_t maxLen, const std::string& truncateMsg)
{
  if (str.size() <= maxLen)
  {
    return str;
  }
  return str.substr(0, maxLen) + truncateMsg;
}

std::string
truncateDiagIfNeeded(const std::string& str, const PricingTrx& trx)
{
  if (!canTruncateDiagnostic(trx))
  {
    return str;
  }
  return truncateDiagnostic(str);
}

} // namespace utils

} // namespace tse
