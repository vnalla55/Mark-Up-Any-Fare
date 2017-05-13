//----------------------------------------------------------------------------
//  File:        OcFeeGroupConfig.cpp
//  Created:     2010-04-12
//
//  Description: Class used to keep summary groups configuration of OC Fees
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

#include "DataModel/OcFeeGroupConfig.h"

#include <boost/tokenizer.hpp>

#include <string>
#include <vector>

using namespace tse;

std::string
OcFeeGroupConfig::parseOCFeesSummaryConfiguration(const std::string& inputString,
                                                  std::vector<OcFeeGroupConfig>& outputVec)
{
  if (inputString.empty())
  {
    return "Input string is empty - no group configuration defined";
  }

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> separator("|");
  tokenizer tokens(inputString, separator);

  tokenizer::iterator tokensIter;

  for (tokensIter = tokens.begin(); tokensIter != tokens.end(); ++tokensIter)
  {
    std::string token = ((std::string)tokensIter->data());

    OcFeeGroupConfig groupConfig;
    std::string retValue = parseOcFeeGroup(token, groupConfig);

    if ("OK" != retValue)
    {
      return retValue + ": " + token;
    }

    outputVec.push_back(groupConfig);
  }

  return "OK";
}

std::string
OcFeeGroupConfig::parseOcFeeGroup(const std::string& inputString, OcFeeGroupConfig& outputGroup)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> separator("*");
  tokenizer tokens(inputString, separator);

  bool allTokensFound = false;
  int tokenNum = 1;
  tokenizer::iterator tokensIter;

  for (tokensIter = tokens.begin(); tokensIter != tokens.end(); ++tokensIter, ++tokenNum)
  {
    std::string token = ((std::string)tokensIter->data());

    switch (tokenNum)
    {
    case 1:
    {
      if (token.size() > 3)
      {
        return "Incorrect group: " + token + ", more than 3 characters";
      }

      outputGroup.groupCode() = token;
    }
    break;

    case 2:
    {
      std::string retValue = parseQuantity(token, outputGroup);

      if ("OK" != retValue)
      {
        return retValue;
      }
    }
    break;

    case 3:
    {
      allTokensFound = true;
      std::string retValue = parseSortingRules(token, outputGroup);

      if ("OK" != retValue)
      {
        return retValue;
      }
    }
    break;

    default:
      return "Too many tokens in group configuration";
      break;
    }
  }

  if (!allTokensFound)
  {
    return "Incorrect group configuration";
  }

  return "OK";
}

std::string
OcFeeGroupConfig::parseQuantity(const std::string& inputString, OcFeeGroupConfig& outputGroup)
{
  if (inputString != "NA")
  {
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> separator("-");
    tokenizer tokens(inputString, separator);

    bool allTokensFound = false;
    int tokenNum = 1;
    tokenizer::iterator tokensIter;

    for (tokensIter = tokens.begin(); tokensIter != tokens.end(); ++tokensIter, ++tokenNum)
    {
      std::string token = ((std::string)tokensIter->data());

      switch (tokenNum)
      {
      case 1:
        try
        {
          outputGroup.startRange() = atoi(token.c_str());

          if ((outputGroup.startRange() < 1) || (outputGroup.startRange() > 1000))
          {
            return "Incorrect Start range (should be integer value 1 - 1000)";
          }
        }
        catch (std::exception& e)
        {
          return "Error while parsing Start range value";
        }
        break;

      case 2:
        try
        {
          allTokensFound = true;
          outputGroup.endRange() = atoi(token.c_str());

          if ((outputGroup.endRange() < 1) || (outputGroup.endRange() > 1000))
          {
            return "Incorrect End range (should be integer value 1 - 1000)";
          }
        }
        catch (std::exception& e)
        {
          return "Error while parsing End range value";
        }
        break;

      default:
        return "Too many tokens in quantity string";
        break;
      }
    }

    if (!allTokensFound)
    {
      return "Incorrect quantity";
    }

    if (outputGroup.startRange() > outputGroup.endRange())
    {
      return "Start range is grater than End range";
    }
  }
  else
  {
    outputGroup.startRange() = -1;
    outputGroup.endRange() = -1;
  }

  return "OK";
}

std::string
OcFeeGroupConfig::parseSortingRules(const std::string& inputString, OcFeeGroupConfig& outputGroup)
{
  if (inputString.empty())
  {
    return "Command / Sub Group Codes string is empty";
  }

  if (inputString.at(0) == '#')
  {
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> separator(":");
    tokenizer tokens(inputString, separator);

    int tokenNum = 1;
    tokenizer::iterator tokensIter;

    for (tokensIter = tokens.begin(); tokensIter != tokens.end(); ++tokensIter, ++tokenNum)
    {
      std::string token = ((std::string)tokensIter->data());

      switch (tokenNum)
      {
      case 1:
      {
        outputGroup.applyTo() = token.at(token.size() - 1);

        if ((outputGroup.applyTo() != 'I') && (outputGroup.applyTo() != 'P'))
        {
          return "Unknown apply to field - " + token;
        }

        outputGroup.commandName() = token.substr(0, token.size() - 1);

        if ((outputGroup.commandName() != "#LO") && (outputGroup.commandName() != "#LO-EX"))
        {
          return "Unknown command name - " + token;
        }
      }
      break;

      case 2:
      {
        std::string retValue = parseSubTypeCodes(token, outputGroup);

        if ("OK" != retValue)
        {
          return retValue;
        }
      }
      break;

      default:
        return "Too many tokens in Command / Sub Group Codes string";
        break;
      }
    }
  }
  else
  {
    std::string retValue = parseSubTypeCodes(inputString, outputGroup);

    if ("OK" != retValue)
    {
      return retValue;
    }
  }

  return "OK";
}

std::string
OcFeeGroupConfig::parseSubTypeCodes(const std::string& inputString, OcFeeGroupConfig& outputGroup)
{
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> separator("-");
  tokenizer tokens(inputString, separator);

  tokenizer::iterator tokensIter;

  for (tokensIter = tokens.begin(); tokensIter != tokens.end(); ++tokensIter)
  {
    std::string token = ((std::string)tokensIter->data());

    if (token.size() > 3)
    {
      return "Incorrect Sub Group Code: " + token + ", more than 3 characters";
    }

    outputGroup.subTypeCodes().push_back(token);
  }

  return "OK";
}
