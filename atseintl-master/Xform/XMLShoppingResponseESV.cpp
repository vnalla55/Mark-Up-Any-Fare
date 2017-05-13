//----------------------------------------------------------------------------
//  File:        XMLShoppingResponseESV.cpp
//  Created:     2008-04-16
//
//  Description: ESV response formatter
//
//  Updates:
//
//  Copyright Sabre 2008
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Xform/XMLShoppingResponseESV.h"

#include "Common/ClassOfService.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/MetricsUtil.h"
#include "Common/Money.h"
#include "DataModel/Agent.h"
#include "DBAccess/DataManager.h"
#include "Diagnostic/DiagTools.h"

namespace
{
log4cxx::LoggerPtr
_logger(log4cxx::Logger::getLogger("atseintl.Xform.XMLShoppingResponseESV"));
}

using namespace tse;

namespace tse
{
namespace
{
ConfigurableValue<std::string>
port("SERVER_SOCKET_ADP", "PORT");
}

bool
XMLShoppingResponseESV::generateXML(PricingTrx& trx, std::string& res)
{
  XMLShoppingResponseESV builder(trx);
  builder.getXML(res);
  return true;
}

void
XMLShoppingResponseESV::getXML(std::string& result)
{
  generateResponse();
  _writer.result(result);
}

XMLShoppingResponseESV::XMLShoppingResponseESV(PricingTrx& trx, ErrorResponseException* ere)
  : _trx(trx), _shoppingTrx(dynamic_cast<const ShoppingTrx*>(&trx)), _error(ere)
{
}

void
XMLShoppingResponseESV::generateError()
{
  Node nodeDIA(_writer, "DIA");

  {
    std::string error =
        utils::truncateDiagIfNeeded(_error->message() + "\n" + _trx.diagnostic().toString(), _trx);

    if (error.empty())
    {
      error = "UNKNOWN ESV ERROR";
    }

    Node nodeMTP(_writer, "MTP");
    nodeMTP.addData(error);
  }

  {
    std::ostringstream metrics;
    MetricsUtil::trxLatency(metrics, _trx);

    Node nodeMTP(_writer, "MTP");
    nodeMTP.addData(metrics.str());
  }
}

void
XMLShoppingResponseESV::generateResponse()
{
  Node nodeShoppingResponse(_writer, "ShoppingResponse");

  if (!_trx.token().empty())
  {
    nodeShoppingResponse.convertAttr("STK", _trx.token());
  }

  if (nullptr != _error)
  {
    nodeShoppingResponse.convertAttr("Q0S", 0);
    generateError();
    return;
  }

  // Print number of solutions
  size_t solutionsCount;
  solutionsCount =
      _shoppingTrx->flightMatrixESV().size() + _shoppingTrx->estimateMatrixESV().size();

  nodeShoppingResponse.convertAttr("Q0S", solutionsCount);

  // Print metrics
  if ((false != _trx.recordMetrics()) || (_trx.diagnostic().diagnosticType() != DiagnosticNone))
  {
    std::ostringstream metrics;
    MetricsUtil::trxLatency(metrics, _trx);

    Node nodeDIA(_writer, "DIA");
    nodeDIA.convertAttr("Q0A", static_cast<int>(_trx.diagnostic().diagnosticType()));

    // Displaying hostname/port and database
    std::string hostname = getenv("HOSTNAME");

    for (auto& elem : hostname)
    {
      elem = std::toupper(elem);
    }
    std::string hostNPort = "HOSTNAME : " + hostname + " PORT : " + port.getValue() + "\n";

    std::string dbInfo = formatDBInfo();

    nodeDIA.addData(utils::truncateDiagIfNeeded(
        hostNPort + dbInfo + _trx.diagnostic().toString() + metrics.str(), _trx));
  }

  generateOND();

  std::vector<ESVSolution*>::const_iterator esvSolutionIter;

  // Print flight matrix
  for (esvSolutionIter = _shoppingTrx->flightMatrixESV().begin();
       esvSolutionIter != _shoppingTrx->flightMatrixESV().end();
       esvSolutionIter++)
  {
    ESVSolution* esvSolution = (*esvSolutionIter);

    if (nullptr == esvSolution)
    {
      LOG4CXX_ERROR(_logger,
                    "XMLShoppingResponseESV::generateResponse - ESVSolution object is NULL.");
      continue;
    }

    // Generate ITN section
    generateITN(esvSolution);
  }

  // Print estimated matrix
  for (esvSolutionIter = _shoppingTrx->estimateMatrixESV().begin();
       esvSolutionIter != _shoppingTrx->estimateMatrixESV().end();
       esvSolutionIter++)
  {
    ESVSolution* esvSolution = (*esvSolutionIter);

    if (nullptr == esvSolution)
    {
      LOG4CXX_ERROR(_logger,
                    "XMLShoppingResponseESV::generateResponse - ESVSolution object is NULL.");
      continue;
    }

    // Generate ITN section
    generateITN(esvSolution);
  }
}

void
XMLShoppingResponseESV::generateOND()
{
  std::vector<PricingTrx::OriginDestination> odThruFM;

  for (const auto elem : _shoppingTrx->fareMarket())
  {
    if (elem->isEsvThruMarket())
      checkDuplicateThruFM(odThruFM, *elem);
  }

  // format the OND
  for (std::vector<PricingTrx::OriginDestination>::const_iterator i = odThruFM.begin();
       i != odThruFM.end();
       ++i)
  {
    Node nodeOND(_writer, "OND");
    nodeOND.attr("A01", i->boardMultiCity).attr("A02", i->offMultiCity).convertAttr(
        "D01", i->travelDate.dateToSqlString());
  }

  return;
}

void
XMLShoppingResponseESV::checkDuplicateThruFM(std::vector<PricingTrx::OriginDestination>& odThruFM,
                                             FareMarket& fareMarket)
{
  for (std::vector<PricingTrx::OriginDestination>::const_iterator iter = odThruFM.begin();
       iter != odThruFM.end();
       ++iter)
  {
    if ((fareMarket.boardMultiCity() == iter->boardMultiCity) &&
        (fareMarket.offMultiCity() == iter->offMultiCity) &&
        (fareMarket.travelSeg().front()->departureDT().date() == iter->travelDate.date()))
    {
      return;
    }
  }

  PricingTrx::OriginDestination thruFM;
  thruFM.boardMultiCity = fareMarket.boardMultiCity();
  thruFM.offMultiCity = fareMarket.offMultiCity();
  thruFM.travelDate = fareMarket.travelSeg().front()->departureDT();
  odThruFM.push_back(thruFM);
}

void
XMLShoppingResponseESV::generateITN(ESVSolution* esvSolution)
{
  if (nullptr == esvSolution)
  {
    LOG4CXX_ERROR(_logger, "XMLShoppingResponseESV::generateITN - ESVSolution object is NULL.");
    return;
  }

  // Generate ITN attributes
  Node nodeITN(_writer, "ITN");

  nodeITN.convertAttr("Q4Q", esvSolution->carrierGroupId());
  nodeITN.convertAttr("Q5Q", esvSolution->groupId());

  if (true == esvSolution->isJcb())
  {
    nodeITN.convertAttr("PXP", "T");
  }
  else
  {
    nodeITN.convertAttr("PXP", "F");
  }

  nodeITN.convertAttr("TYP", 'M');

  // Generate SID section
  generateSID(esvSolution);

  // Generate PSG section
  generatePSG(esvSolution);

  // Generate TOT section
  generateTOT(esvSolution);
}

void
XMLShoppingResponseESV::generateSID(ESVSolution* esvSolution)
{
  for (uint32_t legId = 0; legId < _shoppingTrx->legs().size(); legId++)
  {
    ESVOption* esvOption = nullptr;

    if (0 == legId)
    {
      esvOption = &(esvSolution->outboundOption());
    }
    else if (1 == legId)
    {
      esvOption = &(esvSolution->inboundOption());
    }
    else
    {
      LOG4CXX_ERROR(_logger, "XMLShoppingResponseESV::generateSID - Incorrect leg.");
      return;
    }

    const ShoppingTrx::SchedulingOption* sop =
        &(_shoppingTrx->legs()[legId].sop()[esvOption->sopId()]);

    if (nullptr == sop)
    {
      LOG4CXX_ERROR(_logger,
                    "XMLShoppingResponseESV::generateSID - SchedulingOption object is NULL.");
      return;
    }

    // Generate SID attributes
    Node nodeSID(_writer, "SID");

    nodeSID.convertAttr("Q14", legId);
    nodeSID.convertAttr("Q15", sop->originalSopId());

    // Generate BKK section
    generateBKK(esvOption);
  }
}

void
XMLShoppingResponseESV::generateBKK(ESVOption* esvOption)
{
  std::vector<ESVFareComponent*>::iterator fareCompIter;

  for (fareCompIter = esvOption->esvFareComponentsVec().begin();
       fareCompIter != esvOption->esvFareComponentsVec().end();
       fareCompIter++)
  {
    ESVFareComponent* esvFareComponent = (*fareCompIter);

    if (nullptr == esvFareComponent)
    {
      LOG4CXX_ERROR(_logger,
                    "XMLShoppingResponseESV::generateBKK - ESVFareComponent object is NULL.");
      continue;
    }

    std::vector<ESVSegmentInfo*>::iterator segInfIter;

    for (segInfIter = esvFareComponent->esvSegmentInfoVec().begin();
         segInfIter != esvFareComponent->esvSegmentInfoVec().end();
         segInfIter++)
    {
      ESVSegmentInfo* esvSegmentInfo = (*segInfIter);

      if (nullptr == esvSegmentInfo)
      {
        LOG4CXX_ERROR(_logger,
                      "XMLShoppingResponseESV::generateBKK - ESVSegmentInfo object is NULL.");
        continue;
      }

      // Generate BKK attributes
      Node nodeBKK(_writer, "BKK");

      nodeBKK.convertAttr("B30", esvSegmentInfo->bookingCode());
      nodeBKK.convertAttr("Q13", esvSegmentInfo->bookedCabin().getCabinIndicator())
          .convertAttr("B31", esvSegmentInfo->bookedCabin().getClassAlphaNum());
    }
  }
}

void
XMLShoppingResponseESV::generatePSG(ESVSolution* esvSolution)
{
  // Generate PSG attributes
  Node nodePSG(_writer, "PSG");

  nodePSG.convertAttr("Q0U", _trx.paxType()[0]->number());
  nodePSG.convertAttr("B70", _trx.paxType()[0]->paxType());
  nodePSG.convertAttr("C5A", esvSolution->totalPrice());
}

void
XMLShoppingResponseESV::generateBKC(ESVFareComponent* esvFareComponent)
{
  std::vector<ESVSegmentInfo*>::iterator segInfIter;

  for (segInfIter = esvFareComponent->esvSegmentInfoVec().begin();
       segInfIter != esvFareComponent->esvSegmentInfoVec().end();
       segInfIter++)
  {
    ESVSegmentInfo* esvSegmentInfo = (*segInfIter);

    if (nullptr == esvSegmentInfo)
    {
      LOG4CXX_ERROR(_logger,
                    "XMLShoppingResponseESV::generateBKC - ESVSegmentInfo object is NULL.");
      continue;
    }

    // Generate BKC attributes
    Node nodeBKC(_writer, "BKC");

    nodeBKC.convertAttr("B30", esvSegmentInfo->bookingCode());
  }
}

void
XMLShoppingResponseESV::generateTOT(ESVSolution* esvSolution)
{
  std::string sellingCurrency = getSellingCurrency();
  CurrencyConversionFacade ccFacade;
  Money nucTotalAmt(esvSolution->totalPrice(), NUC);
  Money convertedTotalAmt(0.0, sellingCurrency);

  if (false == ccFacade.convertCalc(convertedTotalAmt, nucTotalAmt, _trx))
  {
    LOG4CXX_ERROR(_logger,
                  "XMLShoppingResponseESV::generateTOT - Error during currency conversion.");
    return;
  }

  // Generate TOT attributes
  Node nodeTOT(_writer, "TOT");

  nodeTOT.convertAttr("C40", _trx.itin().front()->calculationCurrency());
  nodeTOT.convertAttr("C5A", nucTotalAmt.value());
  nodeTOT.convertAttr("C4C", sellingCurrency);
  nodeTOT.convertAttr("C5H", convertedTotalAmt.value());
  nodeTOT.convertAttr("C5I", convertedTotalAmt.value());
}

std::string
XMLShoppingResponseESV::formatDBInfo()
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
    std::vector<std::string> s_dbConfig;

    const DataManager& dbMan = DataManager::instance();
    const tse::ConfigMan& dbConfig = dbMan.dbConfig();
    std::string dbConnection;
    std::string dbIni;

    dbConfig.getValue("DEFAULT", dbConnection, "CONNECTION");

    if (!dbConfig.getValue("INI", dbIni, dbConnection))
    {
      return dbInfo;
    }

    std::ifstream dbIniCfg(dbIni.data());

    if (!dbIniCfg)
    {
      return dbInfo;
    }

    s_dbConfig.push_back("DATABASE:");
    dbInfo += "DATABASE:\n";

    const unsigned MAX_LINE = 256;
    char line[MAX_LINE];

    while (dbIniCfg.getline(line, MAX_LINE, '\n'))
    {
      std::string str(line);
      if (!str.empty() && str[0] == '#')
      {
        continue;
      }
      std::transform(str.begin(), str.end(), str.begin(), (int (*)(int))std::toupper);

      std::string::size_type pos = std::string::npos;
      std::string::size_type pos2 = std::string::npos;
      std::string::size_type len = str.length();

      while ((pos = str.find('\\')) != std::string::npos)
      {
        str[pos] = '/';
      }

      while ((pos = str.find('_')) != std::string::npos)
      {
        str[pos] = '-';
      }

      pos = str.find('=');
      // every line should have an '=', but just in case...

      if (pos == std::string::npos)
      {
        dbInfo += "   " + str;
        return dbInfo;
      }
      else
      {
        dbInfo += "    " + str.substr(0, pos++) + ":\n";
      }

      // replace commas w/ newlines
      while ((pos2 = str.find(',', pos)) != std::string::npos)
      {
        dbInfo += "        " + str.substr(pos, len - pos2) + "\n";
        pos = pos2 + 1;
      }

      dbInfo += "        " + str.substr(pos) + "\n";
    }
  }

  return dbInfo;
}

std::string
XMLShoppingResponseESV::getSellingCurrency()
{
  std::string sellingCurrency = "";

  if (false == _shoppingTrx->getOptions()->currencyOverride().empty())
  {
    sellingCurrency = _shoppingTrx->getOptions()->currencyOverride();
  }
  else
  {
    sellingCurrency = _shoppingTrx->getRequest()->ticketingAgent()->currencyCodeAgent();
  }

  return sellingCurrency;
}

} // END namespace tse
