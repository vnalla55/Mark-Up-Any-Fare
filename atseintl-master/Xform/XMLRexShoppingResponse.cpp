//----------------------------------------------------------------------------
//  File:        XMLRexShoppingResponse.cpp
//  Created:     2009-01-26
//
//  Description: RexShopping response formatter
//
//  Updates:
//
//  Copyright Sabre 2009
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

#include "Xform/XMLRexShoppingResponse.h"

#include "Common/ClassOfService.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/ExchShopCalendarUtils.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/MetricsUtil.h"
#include "Common/Money.h"
#include "Common/TseSrvStats.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "DBAccess/DataManager.h"
#include "DBAccess/ReissueSequence.h"
#include "Diagnostic/DiagTools.h"
#include "Rules/RuleUtil.h"

namespace tse
{
FALLBACK_DECL(reissueOptionsMap);
FALLBACK_DECL(exsCalendar)

namespace
{
ConfigurableValue<std::string>
port("SERVER_SOCKET_ADP", "PORT");
}
bool
XMLRexShoppingResponse::generateXML(PricingTrx& trx, std::string& res)
{
  XMLRexShoppingResponse builder(trx);
  builder.getXML(res);
  return true;
}

void
XMLRexShoppingResponse::getXML(std::string& result)
{
  generateResponse();
  _writer.result(result);
}

XMLRexShoppingResponse::XMLRexShoppingResponse(PricingTrx& trx, ErrorResponseException* ere)
  : _trx(trx), _rexShoppingTrx(static_cast<RexShoppingTrx*>(&trx)), _error(ere)
{
}

void
XMLRexShoppingResponse::generateError()
{
  Node nodeDIA(_writer, "DIA");
  {
    std::string error =
        utils::truncateDiagIfNeeded(_error->message() + "\n" + _trx.diagnostic().toString(), _trx);

    if (error.empty())
    {
      error = "UNKNOWN EXCHANGE SHOPPING ERROR";
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
XMLRexShoppingResponse::generateResponse()
{
  Node nodeRexShoppingResponse(_writer, "ShoppingResponse");

  if (!_trx.token().empty())
  {
    nodeRexShoppingResponse.convertAttr("STK", _trx.token());
  }

  if (nullptr != _error)
  {
    nodeRexShoppingResponse.convertAttr("Q0S", 0);
    generateError();
    return;
  }

  // Print number of solutions
  size_t solutionsCount;
  solutionsCount = 1; // Q0S
  nodeRexShoppingResponse.convertAttr("Q0S", solutionsCount);

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

  if (_trx.diagnostic().diagnosticType() == DiagnosticNone)
  {
    if (_rexShoppingTrx->isShopOnlyCurrentItin())
    {
      nodeRexShoppingResponse.convertAttr("PR4", "T");
    }
    else
    {
      // generate O/D
      generateOAD(_rexShoppingTrx->oadConsRest());
      // forced connections
      generateFCL();
    }

    // Print fare component
    std::vector<const PaxTypeFare*> ptfv;
    RuleUtil::getAllPTFs(ptfv, *_rexShoppingTrx->exchangeItin().front()->farePath().front());

    for (const auto ptf : ptfv)
    {
      generateFCI(ptf);
    }
  }
}

void
XMLRexShoppingResponse::generateFCI(const PaxTypeFare* ptf)
{
  Node nodeFCI(_writer, "FCI");
  FareCompInfo* fareComponent =
      _rexShoppingTrx->exchangeItin().front()->findFareCompInfo(ptf->fareMarket());

  if (fareComponent)
  {
    nodeFCI.convertAttr("NUM", fareComponent->fareCompNumber());

    if (ptf)
    {
      // Print VCTR
      generateVCTR(ptf, fareComponent->fareMarket());
      // Print matched R3
      generateR3I(ptf);
    }
  }
}

void
XMLRexShoppingResponse::generateOAD(
    const RexShoppingTrx::OADConsolidatedConstrainsVect& oadConsRest)
{
  RexShoppingTrx::OADConsolidatedConstrainsVect::const_iterator oadConsRestIter =
      oadConsRest.begin();

  for (; oadConsRestIter != oadConsRest.end(); ++oadConsRestIter)
  {
    RexShoppingTrx::OADConsolidatedConstrains& oadConsolConstr = **oadConsRestIter;
    Node nodeOAD(_writer, "OAD");
    nodeOAD.convertAttr("A01", oadConsolConstr.origAirport);
    nodeOAD.convertAttr("A02", oadConsolConstr.destAirport);
    nodeOAD.convertAttr("D01", oadConsolConstr.travelDate.dateToString(YYYYMMDD, "-"));

    if (!oadConsolConstr.unflown)
      nodeOAD.convertAttr("PCI", "T");

    std::vector<RexShoppingTrx::SubOriginDestination*>::const_iterator sodIter =
        oadConsolConstr.sod.begin();

    for (; sodIter != oadConsolConstr.sod.end(); ++sodIter)
    {
      RexShoppingTrx::SubOriginDestination& sod = **sodIter;
      Node nodeSOD(_writer, "SOD");
      nodeSOD.convertAttr("A01", sod.origAirport);
      nodeSOD.convertAttr("A02", sod.destAirport);
      nodeSOD.convertAttr("D01", sod.travelDate.dateToString(YYYYMMDD, "-"));
      if (ExchShopCalendar::isEXSCalendar(*_rexShoppingTrx))
      {
        nodeSOD.convertAttr("D41", sod.calendarRange.firstDate.dateToString(YYYYMMDD, "-"));
        nodeSOD.convertAttr("D42", sod.calendarRange.lastDate.dateToString(YYYYMMDD, "-"));
      }

      nodeSOD.convertAttr("PR0", ((sod.change) ? "T" : "F"));

      if (sod.change)
      {
        if (sod.carriers && !sod.carriers->usrCxr.cxrList.empty())
        {
          nodeSOD.convertAttr("PR2", ((sod.exact_cxr) ? "T" : "F"));
        }

        if (sod.carriers && !sod.carriers->govCxr.cxrList.empty())
        {
          if (sod.preferred_cxr)
            nodeSOD.convertAttr("PR3", "T");
        }
      }

      // FLIGHTS LIST
      std::set<int>::const_iterator fltIter = sod.flights.begin();

      for (; fltIter != sod.flights.end(); ++fltIter)
      {
        Node nodeFLT(_writer, "FLT");
        nodeFLT.convertAttr("Q1K", *fltIter);

        if (sod.bkcCanChange.count(*fltIter) != 0)
        {
          nodeFLT.convertAttr("PR1", "T");
        }
      }

      // GOV CARRIER
      if (sod.change && sod.preferred_cxr && sod.carriers && !sod.carriers->govCxr.cxrList.empty())
      {
        std::set<CarrierCode>::const_iterator cxrIter = sod.carriers->govCxr.cxrList.begin();

        for (; cxrIter != sod.carriers->govCxr.cxrList.end(); ++cxrIter)
        {
          Node nodeCRL(_writer, "CR1");
          nodeCRL.convertAttr("B00", *cxrIter);
        }
      }

      // USER CARRIER
      if (sod.change && sod.carriers && !sod.carriers->usrCxr.cxrList.empty())
      {
        std::set<CarrierCode>::const_iterator cxrIter = sod.carriers->usrCxr.cxrList.begin();

        for (; cxrIter != sod.carriers->usrCxr.cxrList.end(); ++cxrIter)
        {
          Node nodeCRL(_writer, "CRL");
          nodeCRL.convertAttr("B00", *cxrIter);
        }
      }
    }
  }
}

void
XMLRexShoppingResponse::generateFCL()
{
  if (_rexShoppingTrx->oadResponse().empty())
  {
    return;
  }

  for (const auto& oadResponse : _rexShoppingTrx->oadResponse().begin()->second)
  {
    const auto& frcConxRest = oadResponse.forcedConnections;
    for (auto frcCnnxIter = frcConxRest.begin(); frcCnnxIter != frcConxRest.end(); ++frcCnnxIter)
    {
      Node nodeFCL(_writer, "FCL");
      nodeFCL.convertAttr("AJ0", *frcCnnxIter);
    }
  }
}

void
XMLRexShoppingResponse::generateVCTR(const PaxTypeFare* ptf, const FareMarket* fareMarket)
{
  // if(fareCompInfo->hasVCTR())
  Node nodeVCT(_writer, "VCT");
  nodeVCT.convertAttr("S37", ptf->vendor());
  nodeVCT.convertAttr("B09", ptf->carrier());
  nodeVCT.convertAttr("S89", ptf->fareTariff());
  nodeVCT.convertAttr("S90", ptf->ruleNumber());
  nodeVCT.convertAttr("RTD", ptf->retrievalDate().dateToString(YYYYMMDD, "-"));
}

void
XMLRexShoppingResponse::generateR3I(const PaxTypeFare* ptf)
{
  std::vector<ReissueOptions::R3WithDateRange> r3v;
  _rexShoppingTrx->reissueOptions().getRec3s(ptf, r3v);

  if (r3v.empty() && ptf->isFareByRule())
  {
    ptf = ptf->baseFare();
    _rexShoppingTrx->reissueOptions().getRec3s(ptf, r3v);
  }

  if (!r3v.empty())
  {
    const bool isEXSCalendar = ExchShopCalendar::isEXSCalendar(*_rexShoppingTrx);

    for (const auto& r3Pair : r3v)
    {
      Node nodeMR3(_writer, "R3I");
      nodeMR3.convertAttr("RID", r3Pair.first->itemNo());
      std::vector<ReissueOptions::ReissueSeqWithDateRange> t988v;
      _rexShoppingTrx->reissueOptions().getT988s(ptf, r3Pair.first, t988v);

      if (!fallback::exsCalendar(&_trx) && isEXSCalendar)
      {
        nodeMR3.convertAttr("D41", r3Pair.second.firstDate.dateToString(YYYYMMDD, "-"));
        nodeMR3.convertAttr("D42", r3Pair.second.lastDate.dateToString(YYYYMMDD, "-"));
      }

      if (!t988v.empty())
      {
        for (const auto& seqPair : t988v)
        {
          if (!seqPair.first)
            continue;

          Node nodeSEQ(_writer, "SEQ");
          nodeSEQ.convertAttr("SID", seqPair.first->seqNo());

          if (!fallback::exsCalendar(&_trx) && isEXSCalendar)
          {
            nodeSEQ.convertAttr("D41", seqPair.second.firstDate.dateToString(YYYYMMDD, "-"));
            nodeSEQ.convertAttr("D42", seqPair.second.lastDate.dateToString(YYYYMMDD, "-"));
          }
        }
      }
    }
  }
}

std::string
XMLRexShoppingResponse::formatDBInfo()
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
} // END namespace tse
