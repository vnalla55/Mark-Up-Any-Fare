//----------------------------------------------------------------------------
//
//  File:               XMLWriter.cpp
//  Description:        implementation of simple XML document writing code
//  Created:            01/14/2005
//  Authors:            David White
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#include "Xform/XMLWriter.h"

#include "Common/Assert.h"
#include "Util/BranchPrediction.h"

#include <algorithm>

namespace tse
{

XMLWriter::XMLWriter() : _openElements(0), _inStartElement(false), _simpleString(false)
{
  openSubDocument();
  _streams.top() += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
}

XMLWriter::~XMLWriter()
{
  closeSubDocument();
  TSE_ASSERT(_streams.empty());
}

std::string
XMLWriter::result() const
{
  TSE_ASSERT(!_streams.empty()); // lint !e666
  return _streams.top();
}

void
XMLWriter::result(std::string& res)
{
  if (!_streams.empty())
  {
    res.assign(_streams.top());
  }
}

void
XMLWriter::startNode(const std::string& name)
{
  enterNode();

  _streams.top().append(_openElements.size() * 2, ' ');

  std::string buf;
  _streams.top() += "<" + escapeString(name, buf);
  _openElements.push_back(name);
  _inStartElement = true;
}

void
XMLWriter::addAttribute(const std::string& name, const std::string& value)
{
  TSE_ASSERT(_inStartElement);
  std::string bufName, bufValue;
  _streams.top() +=
      " " + escapeString(name, bufName) + "=\"" + escapeString(value, bufValue) + '\"';
}

void
XMLWriter::enterNode()
{
  if (_inStartElement)
  {
    _streams.top() += ">\n";
    _inStartElement = false;
  }
}

void
XMLWriter::addText(const std::string& text)
{
  enterNode();

  std::string buf;
  _streams.top() += escapeString(text, buf);
}

void
XMLWriter::addSimpleText(const std::string& text)
{
  _simpleString = true;

  if (LIKELY(_inStartElement))
  {
    _streams.top() += ">";
    _inStartElement = false;
  }

  std::string buf;
  _streams.top() += escapeString(text, buf);
}

void
XMLWriter::addData(const std::string& text)
{
  enterNode();

  _streams.top() += "<![CDATA[";
  addText(text);
  _streams.top() += "]]>";
}

void
XMLWriter::addRawText(const std::string& text)
{
  enterNode();

  _streams.top() += text;
}

void
XMLWriter::endNode()
{
  TSE_ASSERT(_openElements.empty() == false); // lint !e666

  if (_inStartElement)
  {
    _streams.top() += "/>\n";
    _inStartElement = false;
  }
  else
  {
    std::string buf;
    if (LIKELY(_openElements.size() > 0))
    {
      if (!_simpleString)
        for (size_t item = 0; item < _openElements.size() - 1; ++item)
        {
          _streams.top() += "  ";
        }
      else
        _simpleString = false;
    }
    _streams.top() += "</" + escapeString(_openElements.back(), buf) + ">\n";
  }

  _openElements.pop_back();
}

namespace
{

const std::string escapeChars = "&<>";

struct needsEscaping
{
  bool operator()(const char c) const
  {
    return c == 0 || std::find(escapeChars.begin(), escapeChars.end(), c) != escapeChars.end();
  }
};
}

const std::string&
XMLWriter::escapeString(const std::string& str, std::string& buf)
{
  if (LIKELY(std::find_if(str.begin(), str.end(), needsEscaping()) == str.end()))
  {
    return str;
  }

  buf.clear();
  for (const char elem : str)
  {
    switch (elem)
    {
    case '\0':
      break; // null characters get stripped
    case '<':
      buf += "&lt;";
      break;
    case '>':
      buf += "&gt;";
      break;
    case '&':
      buf += "&amp;";
      break;
    default:
      buf.push_back(elem);
      break;
    }
  }

  return buf;
}

void
XMLWriter::openSubDocument()
{
  enterNode();
  _streams.push("");
}

void
XMLWriter::closeSubDocument()
{
  TSE_ASSERT(!_streams.empty());
  _streams.pop();
}

XMLWriter::Node::Node(XMLWriter& writer, const std::string& element) : _writer(writer)
{
  _writer.startNode(element);
}

XMLWriter::Node::~Node() { _writer.endNode(); }

XMLWriter::Node&
XMLWriter::Node::attr(const std::string& name, const std::string& value)
{
  _writer.addAttribute(name, value);
  return *this;
}

XMLWriter::Node&
XMLWriter::Node::addText(const std::string& text)
{
  _writer.addText(text);
  return *this;
}

XMLWriter::Node&
XMLWriter::Node::addSimpleText(const std::string& text)
{
  _writer.addSimpleText(text);
  return *this;
}

XMLWriter::Node&
XMLWriter::Node::addData(const std::string& text)
{
  _writer.addData(text);
  return *this;
}

XMLWriter::Node&
XMLWriter::Node::addRawText(const std::string& text)
{
  _writer.addRawText(text);
  return *this;
}

XMLWriter::Node&
XMLWriter::Node::enterNode()
{
  _writer.enterNode();
  return *this;
}
}
