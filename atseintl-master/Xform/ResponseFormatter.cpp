//----------------------------------------------------------------------------
//
//  File:  ResponseFormatter.cpp
//  Description: See ResponseFormatter.h file
//  Created:  Nov 9, 2006
//  Authors:  Jeff Hoffman
//
//  Copyright Sabre 2005
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

#include "Xform/ResponseFormatter.h"

#include "Common/Config/ConfigurableValue.h"
#ifdef CONFIG_HIERARCHY_REFACTOR
#include "Common/Config/ConfigurableValuesAdapterPool.h"
#else
#include "Common/Config/ConfigurableValuesPool.h"
#endif
#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/TrxUtil.h"
#include "Common/TseSrvStats.h"
#include "Common/TseUtil.h"
#include "Common/XMLConstruct.h"
#include "DataModel/PricingTrx.h"
#include "Server/TseServer.h"

namespace tse
{

namespace
{
ConfigurableValue<std::string>
port("SERVER_SOCKET_ADP", "PORT", "0");
}

bool
ResponseFormatter::hostDiagString(std::vector<std::string>& hostMsgVec) const
{
  const uint16_t HOSTNAME_SIZE = 256;
  char hostname[HOSTNAME_SIZE];

  if (gethostname(hostname, HOSTNAME_SIZE) != 0)
    return false;

  std::string hostnameStr(hostname);

  std::transform(
      hostnameStr.begin(), hostnameStr.end(), hostnameStr.begin(), (int (*)(int))toupper);

  const std::string hostMsg = "HOSTNAME/PORT:  " + hostnameStr + "/" + port.getValue();
  TseUtil::splitTextLine(hostMsg, hostMsgVec);
  return true;
}

void
ResponseFormatter::buildDiagString(std::vector<std::string>& buildMsgVec) const
{
  std::string buildMsg;
  buildMsg = "BASELINE: " BUILD_LABEL_STRING;
  std::transform(buildMsg.begin(), buildMsg.end(), buildMsg.begin(), (int (*)(int))toupper);

  TseUtil::splitTextLine(buildMsg, buildMsgVec);

  if (TseServer::getInstance())
  {
    buildMsg = "\nPROCESS IMAGE NAME: ";
    buildMsg += TseServer::getInstance()->getProcessImageName();
    buildMsg += '\n';
    TseUtil::splitTextLine(buildMsg, buildMsgVec);
  }
}

bool
ResponseFormatter::configDiagString(std::vector<std::string>& configMsgVec, Trx& trx, bool force)
    const
{
  const Diagnostic& diag = trx.diagnostic();
  std::string configMsg;
  std::vector<tse::ConfigMan::NameValue> configs;

  if (diag.diagParamMapItem(Diagnostic::FARE_CLASS_CODE) == "CONFIGSTATIC")
    Global::config().getValues(configs);
  else if (diag.diagParamMapItem(Diagnostic::FARE_CLASS_CODE) == "CONFIG")
  {
    tse::ConfigMan* dynamicCfg = trx.dynamicCfg().get();
    if (dynamicCfg)
      dynamicCfg->getValues(configs);
    else
      Global::config().getValues(configs);
  }
  else if (diag.diagParamMapItem(Diagnostic::FARE_CLASS_CODE) == "CONFIGDYNAMIC" ||
           diag.diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ALLDYNAMIC")
  {
    if (PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(&trx))

#ifdef CONFIG_HIERARCHY_REFACTOR
      ConfigurableValuesAdapterPool::collectConfigMsg(*pricingTrx, configMsgVec);
#else
      DynamicConfigurableValuesPool::collectConfigMsg(*pricingTrx, configMsgVec);
#endif

  }
  else if (!force)
    return false;

  std::vector<tse::ConfigMan::NameValue>::iterator iter = configs.begin();
  std::vector<tse::ConfigMan::NameValue>::iterator iterE = configs.end();

  for (; iter != iterE; iter++)
  {
    std::string name = iter->name;
    std::replace(name.begin(), name.end(), '_', ' ');

    std::string group = iter->group;
    std::replace(group.begin(), group.end(), '_', ' ');

    const std::string& diagConfigSection = diag.diagParamMapItem(Diagnostic::DISPLAY_DETAIL);
    const std::string& diagConfigItem = diag.diagParamMapItem(Diagnostic::RULE_NUMBER);

    if (!diagConfigSection.empty())
    {
      std::string::size_type pos = group.find(diagConfigSection);
      if (pos == std::string::npos)
        continue;
    }

    if (!diagConfigItem.empty())
    {
      std::string::size_type pos = name.find(diagConfigItem);
      if (pos == std::string::npos)
        continue;
    }

    std::string value = iter->value;
    std::replace(value.begin(), value.end(), '_', ' ');

    configMsg = group + "/" + name + "/" + value;

    std::transform(configMsg.begin(), configMsg.end(), configMsg.begin(), (int (*)(int))toupper);

    configMsgVec.push_back(configMsg);
  }

  if (configMsgVec.empty())
    return false;

  configMsgVec.insert(configMsgVec.begin(), "***** CONFIGURATION DIAGNOSTIC START *****");
  configMsgVec.push_back("***** CONFIGURATION DIAGNOSTIC END *****");

  return true;
}

void
ResponseFormatter::dbDiagString(std::vector<std::string>& dbMsgVec) const
{
  std::string dbMsg;
  dbMsg = "DATABASE: " + TseSrvStats::getCurrentDatabase();
  std::transform(dbMsg.begin(), dbMsg.end(), dbMsg.begin(), (int (*)(int))std::toupper);
  TseUtil::splitTextLine(dbMsg, dbMsgVec);
}

void
ResponseFormatter::addMessageLine(const std::string& line,
                                  XMLConstruct& construct,
                                  const std::string& msgType,
                                  int recNum)
{
  char tmpBuf[256];

  construct.openElement("MSG");
  construct.addAttribute("N06", msgType);
  sprintf(tmpBuf, "%06d", recNum);
  construct.addAttribute("Q0K", tmpBuf);
  construct.addAttribute("S18", line);
  construct.closeElement();
}

void
ResponseFormatter::buildDiag854(XMLConstruct& construct, int& recNum) const
{
  // Check if host and db info needed before the display
  // add host/db diag info
  std::vector<std::string> hostInfo;
  std::vector<std::string> buildInfo;
  std::vector<std::string> dbInfo;
  static const std::string msgType = "X"; // General msg

  if (hostDiagString(hostInfo))
  {
    for (int i = 0, n = hostInfo.size(); i < n; i++)
      addMessageLine(hostInfo[i], construct, msgType, recNum++);
  }

  buildDiagString(buildInfo);
  for (int i = 0, n = buildInfo.size(); i < n; i++)
    addMessageLine(buildInfo[i], construct, msgType, recNum++);

  dbDiagString(dbInfo);
  for (int i = 0, n = dbInfo.size(); i < n; i++)
    addMessageLine(dbInfo[i], construct, msgType, recNum++);
}
} // namespace
