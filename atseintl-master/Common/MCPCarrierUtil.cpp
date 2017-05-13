//----------------------------------------------------------------------------
//
//  Copyright Sabre 2010
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

#include "Common/MCPCarrierUtil.h"

#include "Common/Config/ConfigMan.h"
#include "Common/Logger.h"
#include "DataModel/Trx.h"

#include <boost/tokenizer.hpp>

namespace tse
{

static Logger
logger("atseintl.Common.MCPCarrierUtil");
std::vector<std::pair<CarrierCode, CarrierCode> > MCPCarrierUtil::_mcpCxrMap;
std::vector<CarrierCode> MCPCarrierUtil::_mcpHosts;
std::multimap<CarrierCode, CarrierCode> MCPCarrierUtil::_preferedCarriers;
std::vector<CarrierCode> MCPCarrierUtil::_iapRestricted;
std::vector<CarrierCode> MCPCarrierUtil::_journeyByMarriageCarriers;
std::vector<std::pair<CarrierCode, CarrierCode> > MCPCarrierUtil::_mcpNeutralCxrMap;

void
MCPCarrierUtil::getConfig(tse::ConfigMan& config, const std::string& argName, std::string& retValue)
{
  if (config.getValue(argName, retValue, "XFORMS_MCP"))
  {
    if (retValue.empty())
      LOG4CXX_INFO(logger, "Invalid value " << retValue << " for '" << argName << "'");
  }
  else
    LOG4CXX_INFO(logger, "No config entry for '" << argName << "'");
  LOG4CXX_DEBUG(logger, "XFORMS:" << argName << " = " << retValue << "");
}

bool
MCPCarrierUtil::getCxrPair(const std::string& cfgPair,
                           CarrierCode& cxrPseudo,
                           CarrierCode& cxrActual)
{
  boost::char_separator<char> sepDash("-", "", boost::keep_empty_tokens);
  boost::tokenizer<boost::char_separator<char> > cxrTokens(cfgPair, sepDash);
  boost::tokenizer<boost::char_separator<char> >::const_iterator f = cxrTokens.begin();
  CarrierCode* pCxr[2] = { &cxrActual, &cxrPseudo };
  int count = 0;
  for (; f != cxrTokens.end() && count < 2; count++, f++)
  {
    if ((*f).length() != 2)
    {
      LOG4CXX_ERROR(logger, "Incorrect Carrier '" << *f << "' in entry: " << cfgPair);
      return false;
    }
    *pCxr[count] = *f;
  }
  if (count != 2 || f != cxrTokens.end())
  {
    LOG4CXX_ERROR(logger, "Incorrect Carrier pair entry: " << cfgPair);
    return false;
  }
  return true;
}

bool
MCPCarrierUtil::getPreferedCarrier(const std::string& cfgPrefCarrier,
                                   std::vector<CarrierCode>& carriers)
{
  // prepare vector
  carriers.clear();
  // create tokenizer
  boost::char_separator<char> sepSlash("/", "", boost::keep_empty_tokens);
  boost::tokenizer<boost::char_separator<char> > cxrTokens(cfgPrefCarrier, sepSlash);
  boost::tokenizer<boost::char_separator<char> >::const_iterator f = cxrTokens.begin();
  for (; f != cxrTokens.end(); f++)
  {
    // check if this is carrier code (2 chars)
    if ((*f).length() != 2)
    {
      LOG4CXX_ERROR(logger,
                    "Incorrect prefered Carrier '" << *f << "' in entry: " << cfgPrefCarrier);
      return false;
    }
    // populate set of carriers
    carriers.push_back(*f);
  }
  // fail if vector is empty
  return !carriers.empty();
}

bool
MCPCarrierUtil::getPreferedCarrier(const std::string& cfgPrefCarrier,
                                   CarrierCode& hostCarrier,
                                   std::vector<CarrierCode>& carriers)
{
  // create tokenizet
  boost::char_separator<char> sepDash("-", "", boost::keep_empty_tokens);
  boost::tokenizer<boost::char_separator<char> > cxrTokens(cfgPrefCarrier, sepDash);
  boost::tokenizer<boost::char_separator<char> >::const_iterator f = cxrTokens.begin();
  // check for host carrier
  if (f == cxrTokens.end())
  {
    LOG4CXX_ERROR(logger, "Incorrect Prefered Carrier string: " << cfgPrefCarrier);
    return false;
  }
  // check if host carrier is 2 char
  if ((*f).length() != 2)
  {
    LOG4CXX_ERROR(logger, "Incorrect Host Carrier '" << *f << "' in entry: " << cfgPrefCarrier);
    return false;
  }
  hostCarrier = *f;
  // check if prefered carrier string exists
  if (++f == cxrTokens.end())
  {
    LOG4CXX_ERROR(logger, "Incorrect Prefered Carrier string: " << cfgPrefCarrier);
    return false;
  }
  // check if prefered carrier string was parsed correctly
  if (!getPreferedCarrier(*f, carriers))
    return false;
  // there should be only 2 tokens in main cfg string
  if (++f != cxrTokens.end())
  {
    LOG4CXX_ERROR(logger, "Incorrect Prefered Carrier string: " << cfgPrefCarrier);
    return false;
  }
  return true;
}

template <typename T>
void
MCPCarrierUtil::tokenize(const std::string& input, const bool checkCarrier, std::vector<T>& output)
{
  boost::char_separator<char> sepPipe("|", "", boost::keep_empty_tokens);
  boost::tokenizer<boost::char_separator<char> > tokens(input, sepPipe);
  boost::tokenizer<boost::char_separator<char> >::const_iterator f = tokens.begin();
  for (; f != tokens.end(); f++)
  {
    if (checkCarrier && (f->length() != 2))
    {
      LOG4CXX_ERROR(logger, "Incorrect Carrier code: " << *f);
      continue;
    }

    output.push_back(*f);
  }
}

void
MCPCarrierUtil::initialize(tse::ConfigMan& config)
{
  std::string strHosts, strMcpCarriers, strPreferedCarriers, iapRestricted, journeyByMarriage, strMcpNeutralCxrs;

  getConfig(config, "MCP_PRIME_HOSTS", strHosts);
  getConfig(config, "MCP_REAL_TO_PSEUDO_CXR_MAP", strMcpCarriers);
  getConfig(config, "MCP_PREFERRED_CARRIERS", strPreferedCarriers);
  getConfig(config, "IAP_PRICING_RESTRICTED", iapRestricted);
  getConfig(config, "JOURNEY_BY_MARRIAGE_MCPS", journeyByMarriage);
  getConfig(config, "MCP_ACTUAL_TO_NEUTRAL_PARTITION", strMcpNeutralCxrs);

  tokenize(strHosts, true, _mcpHosts);

  std::vector<std::string> tokens;
  tokenize(strMcpCarriers, false, tokens);

  std::vector<std::string>::const_iterator f = tokens.begin();
  for (; f != tokens.end(); f++)
  {
    CarrierCode cxrActual, cxrPseudo;
    if (getCxrPair(*f, cxrPseudo, cxrActual))
      _mcpCxrMap.push_back(std::make_pair(cxrPseudo, cxrActual));
  }

  tokens.clear();
  tokenize(strPreferedCarriers, false, tokens);
  f = tokens.begin();
  for (; f != tokens.end(); f++)
  {
    CarrierCode hostCarrier;
    std::vector<CarrierCode> preferedCarriers;
    if (getPreferedCarrier(*f, hostCarrier, preferedCarriers))
      for (const auto& preferedCarrier : preferedCarriers)
        _preferedCarriers.insert(std::make_pair(hostCarrier, preferedCarrier));
  }

  tokenize(iapRestricted, true, _iapRestricted);
  tokenize(journeyByMarriage, true, _journeyByMarriageCarriers);

  tokens.clear();
  tokenize(strMcpNeutralCxrs, false, tokens);
  f = tokens.begin();
  for (; f != tokens.end(); f++)
  {
    CarrierCode cxrActual, cxrNeutral;
    if (getCxrPair(*f, cxrNeutral, cxrActual))
      _mcpNeutralCxrMap.push_back(std::make_pair(cxrNeutral, cxrActual));
  }
}

CarrierCode
MCPCarrierUtil::swapToActual(const Trx* trx, const CarrierCode& cxr)
{
  if (trx && !trx->mcpCarrierSwap())
    return cxr;

  std::vector<std::pair<CarrierCode, CarrierCode> >::iterator it = _mcpCxrMap.begin();
  std::vector<std::pair<CarrierCode, CarrierCode> >::iterator ie = _mcpCxrMap.end();
  for (; it != ie; it++)
  {
    if (it->first == cxr)
      return it->second;
  }
  return cxr;
}

CarrierCode
MCPCarrierUtil::swapToPseudo(const Trx* trx, const CarrierCode& cxr)
{
  if (LIKELY(trx && !trx->mcpCarrierSwap()))
    return cxr;

  std::vector<std::pair<CarrierCode, CarrierCode> >::iterator it = _mcpCxrMap.begin();
  std::vector<std::pair<CarrierCode, CarrierCode> >::iterator ie = _mcpCxrMap.end();
  for (; it != ie; it++)
  {
    if (it->second == cxr)
      return it->first;
  }
  return cxr;
}

CarrierCode
MCPCarrierUtil::swapFromNeutralToActual(const CarrierCode& cxr)
{
  std::vector<std::pair<CarrierCode, CarrierCode> >::iterator it = _mcpNeutralCxrMap.begin();
  std::vector<std::pair<CarrierCode, CarrierCode> >::iterator ie = _mcpNeutralCxrMap.end();
  for (; it != ie; it++)
  {
    if (it->first == cxr)
      return it->second;
  }
  return cxr;
}

bool
MCPCarrierUtil::isPseudoCarrier(const std::string& carrier)
{
  std::vector<std::pair<CarrierCode, CarrierCode> >::iterator it = _mcpCxrMap.begin();
  std::vector<std::pair<CarrierCode, CarrierCode> >::iterator ie = _mcpCxrMap.end();
  for (; it != ie; it++)
    if (it->first == carrier)
      return true;
  return false;
}

bool
MCPCarrierUtil::isMcpHost(const std::string& host)
{
  std::vector<CarrierCode>::iterator it = _mcpHosts.begin();
  std::vector<CarrierCode>::iterator ie = _mcpHosts.end();
  for (; it != ie; it++)
    if (UNLIKELY(*it == host))
      return true;
  return false;
}

bool
MCPCarrierUtil::isJourneyByMarriageCarrier(const CarrierCode& cxr)
{
  std::vector<CarrierCode>::iterator it = _journeyByMarriageCarriers.begin();
  std::vector<CarrierCode>::iterator ie = _journeyByMarriageCarriers.end();
  for (; it != ie; it++)
  {
    if (*it == cxr)
      return true;

    if (isSameGroupCarriers(*it, cxr))
      return true;
  }

  return false;
}

bool
MCPCarrierUtil::getPreferedCarriers(const CarrierCode& hostCarrier, std::set<CarrierCode>& carriers)
{
  std::pair<std::multimap<CarrierCode, CarrierCode>::iterator,
            std::multimap<CarrierCode, CarrierCode>::iterator> ret =
      _preferedCarriers.equal_range(hostCarrier);

  for (std::multimap<CarrierCode, CarrierCode>::iterator it = ret.first; it != ret.second; it++)
    carriers.insert(it->second);
  return carriers.size() > 0;
}

bool
MCPCarrierUtil::isIAPCarrierRestricted(const CarrierCode& cxr)
{
  std::vector<CarrierCode>::iterator it = _iapRestricted.begin();
  std::vector<CarrierCode>::iterator ie = _iapRestricted.end();
  for (; it != ie; it++)
    if (*it == cxr)
      return true;
  return false;
}

bool
MCPCarrierUtil::isSameGroupCarriers(const CarrierCode& carrier1, const CarrierCode& carrier2)
{
  std::pair<std::multimap<CarrierCode, CarrierCode>::iterator,
            std::multimap<CarrierCode, CarrierCode>::iterator> ret =
      _preferedCarriers.equal_range(carrier1);

  for (std::multimap<CarrierCode, CarrierCode>::iterator it = ret.first; it != ret.second; it++)
    if (it->second == carrier2)
      return true;

  return false;
}

bool
MCPCarrierUtil::isNeutralCarrier(const std::string& carrier)
{
  std::vector<std::pair<CarrierCode, CarrierCode> >::iterator it = _mcpNeutralCxrMap.begin();
  std::vector<std::pair<CarrierCode, CarrierCode> >::iterator ie = _mcpNeutralCxrMap.end();
  for (; it != ie; it++)
    if (it->first == carrier)
      return true;
  return false;
}

}
