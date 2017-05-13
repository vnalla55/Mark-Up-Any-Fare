#include "Xform/CustomXMLParser/ISchemaBase.h"

#include "Util/BranchPrediction.h"
#include "Xform/CustomXMLParser/IAttributes.h"
#include "Xform/CustomXMLParser/IBaseHandler.h"
#include "Xform/CustomXMLParser/IXMLErrorMsgs.h"
#include "Xform/CustomXMLParser/IXMLUtils.h"

#include <boost/bind.hpp>

#include <algorithm>
#include <stdexcept>

namespace
{
/*
An XML Name (sometimes called simply a Name) is a token that
begins with a letter, underscore, or colon (but not other punctuation)
continues with letters, digits, hyphens, underscores, colons,
or full stops [periods], known as name characters
*/
  bool isValidFirst (char c)
  {
    return (c >= 'A' && c <= 'Z')
           || (c >= 'a' && c <= 'z')//isalpha(c)
           || '_' == c
           || ':' == c;
  }
  bool isValidNext (char c)
  {
    return isValidFirst(c)
           || (c >= '0' && c <= '9')//isdigit(c)
           || '-' == c
           || '.' == c;
  }
}
ISchemaBase::ISchemaBase (bool checkWellFormedness)
  : _checkWellFormedness(checkWellFormedness)
  , _nbrOfRoots(0)
  , _attrsSize(0)
{
}

ISchemaBase::~ISchemaBase ()
{
}

void ISchemaBase::insertAttr (const char *pName,
                              size_t nameLength,
                              const char *value,
                              size_t valueLength) const
{
  IKeyString name(pName, nameLength);
  if (_checkWellFormedness)
  {
    checkName(name);
  }
  const char *val(value);
  size_t valLen(valueLength);
  decode(val, valLen);
  IValueString valCStr(val, valLen);
  std::pair<IMap::iterator, bool> pr(_map.insert(std::make_pair(name, valCStr)));
  if (!pr.second)
  {
    throw std::runtime_error(_xmlErrorMsgs[DUPLICATEATTRIBUTENAME]
                             + std::string(pName, nameLength) + ",value:"
                             + std::string(val, valLen));
  }
  ++_attrsSize;
}

void ISchemaBase::characters (IBaseHandler &handler,
                              const char *value,
                              size_t length,
                              bool bdecode) const
{
  const char *val = value;
  size_t len(length);
  if (bdecode)
  {
    decode(val, len);
  }
  handler.characters(val, len);
}

void ISchemaBase::startElement (IBaseHandler &handler,
                                const IKeyString &name) const
{
  IAttributes attrs(0 == _attrsSize ? nullptr : &_map);
  handler.startElement(name, attrs);
}

void ISchemaBase::endElement (IBaseHandler &handler,
                              const IKeyString &name) const
                               
{
  if (_checkWellFormedness)
  {
    onEndElement(name);
  }
  handler.endElement(name);
}

void ISchemaBase::startDocument (IBaseHandler &handler) const
{
  handler.startDocument();
}

void ISchemaBase::endDocument (IBaseHandler &handler) const
{
  if (_checkWellFormedness)
  {
    onEndDocument();
  }
  handler.endDocument();
}

void ISchemaBase::onStartElement (const IKeyString &name) const
{
  checkName(name);
  _checkStack.push(name);
}

void ISchemaBase::onEndElement (const IKeyString &name) const
{
  if (UNLIKELY(_checkStack.empty() || _checkStack.top() != name))
  {
    std::string errorType(_xmlErrorMsgs[MISMATCHEDNAMEINENDELEMENT]);
    std::string mustbe(_NA);
    if (!_checkStack.empty())
    {
      mustbe.assign(_checkStack.top().c_str(), _checkStack.top().length());
    }
    throw std::runtime_error(errorType + '\''
                             + std::string(name.c_str(), name.length())
                             + "',must be:'" + mustbe + '\'');
  }
  _checkStack.pop();
  if (UNLIKELY(_checkStack.empty() && ++_nbrOfRoots > 1))
  {
    throw std::runtime_error(_xmlErrorMsgs[NOROOTELEMENT]);
  }
}

void ISchemaBase::onEndDocument () const
{
  if (!_checkStack.empty())
  {
    throw std::runtime_error(_xmlErrorMsgs[NOTBALANCEDSTARTANDENDELEMENTS]);
  }
}

void ISchemaBase::newElement (const IKeyString &name) const
{
  if (_attrsSize > 0)
  {
    _map.clear();
    _attrsSize = 0;
  }
  if (_checkWellFormedness)
  {
    onStartElement(name);
  }
}

void ISchemaBase::clear () const
{
  _nbrOfRoots = 0;
  while (!_checkStack.empty())
  {
    _checkStack.pop();
  }
  _map.clear();
}

void ISchemaBase::decode (const char *&buffer,
                          size_t &size) const
{
  const char *bufferEnd(buffer + size),
             *start(buffer),
             *prvEnd(buffer);
  bool changed(false);
  while ((start = std::find(start, bufferEnd, '&')) != bufferEnd)
  {
    if (!changed)
    {
      std::vector<char> modified;
      _modifiedValues.push_back(modified);
      _modifiedValues.back().reserve(size);
      changed = true;
    }
    std::vector<char> &inserted(_modifiedValues.back());
    std::copy(prvEnd, start, std::back_inserter(inserted));
    const char *pEnd(nullptr);
    if (bufferEnd == (pEnd = std::find(start, bufferEnd, ';')))
    {
      throw std::runtime_error(_xmlErrorMsgs[NOMATCHINGSEMICOLONFORAMPERSANDINENCODING]
                               + std::string(buffer, size));
    }
    size_t length(pEnd - start + 1),
           i(0);
    for (; i < IXMLUtils::_encodingCount; ++i)
    {
      if (0 == strncmp(start + 1, IXMLUtils::_encoding[i] + 1, length - 2))
      {
        inserted.push_back(IXMLUtils::_specChars[i]);
        prvEnd = pEnd + 1;
        start = prvEnd;
        break;
      }
    }
    if (IXMLUtils::_encodingCount == i)
    {
      throw std::runtime_error(_xmlErrorMsgs[CANNOTTRANSLATEENCODING]
                               + std::string(buffer, size));
    }
  }
  if (UNLIKELY(changed))
  {
    std::vector<char> &inserted(_modifiedValues.back());
    if (prvEnd < bufferEnd)
    {
      std::copy(prvEnd, bufferEnd, std::back_inserter(inserted));
    }
    buffer = &inserted[0];
    size = inserted.size();
  }
}

void ISchemaBase::checkName (const IKeyString &name)
{
  if (UNLIKELY(name.empty())
      || !isValidFirst(name[0])
      || std::find_if(name.begin() + 1,
                      name.end(),
            !boost::bind(&isValidNext, _1)) != name.end())
  {
    throw std::runtime_error(_xmlErrorMsgs[INVALIDXMLNAME]
                             + std::string(name.c_str(), name.length()));
  }
}
