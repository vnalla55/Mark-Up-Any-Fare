//-------------------------------------------------------------------
//
//  File:        DecodeResponseFormatter.cpp
//  Created:     September 12, 2014
//  Authors:     Roland Kwolek
//
//  Description: Response Formatter for Decode Transaction
//
//
//  Copyright Sabre 2014
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

#include "Xform/DecodeResponseFormatter.h"

#include "Common/TseUtil.h"
#include "Common/XMLConstruct.h"
#include "Xform/PricingResponseXMLTags.h"

#include <boost/tokenizer.hpp>

#include <string>

namespace tse
{
std::string
DecodeResponseFormatter::formatResponse(const char msgType)
{
  using Tokenizer = boost::tokenizer<boost::char_separator<char>>;
  using Separator = boost::char_separator<char>;

  int lineNumber = 0;

  XMLConstruct xml;
  xml.openElement("DecodeResponse");

  std::string toSend = _trx.getResponse();

  const Separator separator("\n");
  const Tokenizer tokenizer(toSend, separator);

  Tokenizer::const_iterator token = tokenizer.begin();
  while(token != tokenizer.end())
  {
    std::vector<std::string> lines;
    std::string toBreak(*token);

    TseUtil::splitTextLine(toBreak, lines, DISPLAY_MAX_SIZE, true);

    for (std::string& line : lines)
    {
      prepareMessage(xml, msgType, lineNumber, line);
      lineNumber++;
    }

    ++token;
  }

  xml.closeElement();

  return xml.getXMLData();
}

void
DecodeResponseFormatter::formatResponse(const ErrorResponseException& ere, std::string& response)
{
  XMLConstruct xml;
  xml.openElement("DecodeResponse");

  prepareMessage(xml, Message::TYPE_ERROR, Message::errCode(ere.code()), ere.message());
  xml.closeElement();

  response = xml.getXMLData();
}

void
DecodeResponseFormatter::prepareMessage(XMLConstruct& construct,
                                        const char msgType,
                                        const uint16_t lineNum,
                                        const std::string& msgText)
{
  construct.openElement(xml2::MessageInformation);
  construct.addAttributeChar(xml2::MessageType, msgType);
  construct.addAttributeShort(xml2::MessageFailCode, lineNum);
  construct.addAttribute(xml2::MessageText, msgText);
  construct.closeElement();
}

} // namespace tse
