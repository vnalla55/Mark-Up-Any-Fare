#include "Xform/CustomXMLParser/IParser.h"

#include "Xform/CustomXMLParser/ISchemaBase.h"
#include "Util/BranchPrediction.h"

#include <algorithm>
#include <stdexcept>

const char _CDATA_START[] = "<![CDATA[";
const char _CDATA_END[] = "]]>";
const char _COMMENT_START[] = "<!--";
const char _COMMENT_END[] = "-->";
const char _WHITE[] = " \n\t\r";
const char _CHARS1[] = " \n\t/>\r";
const char _CHARS2[] = "= \n\t\r";
const size_t _npos(std::string::npos),
             _cdataStartLength(strlen(_CDATA_START)),
             _cdataEndLength(strlen(_CDATA_END)),
             _comStartLength(strlen(_COMMENT_START)),
             _comEndLength(strlen(_COMMENT_END));
namespace
{
  inline const char *_find (const char *buffer,
                            const char * const bufferEnd,
                            char ch)
  {
    const char *pCh(buffer);
    for (; pCh < bufferEnd && *pCh != ch; ++pCh);
    return pCh;
  }
  inline const char *_find_first_of (const char *buffer,
                                     const char *bufferEnd,
                                     const char * const chrs)
  {
    const char *pChar(buffer);
    for (; pChar < bufferEnd; ++pChar)
    {
      const char *pCh(chrs);
      char r(0);
      for (char chCurr(*pChar); (r = *pCh) != 0 && chCurr != r; ++pCh);
      if (r != 0)
      {
        break;
      }
    }
    return pChar;
  }
  inline const char *_find_first_not_white (const char *buffer,
                                            const char *bufferEnd)
  {
    const char *pChar(buffer);
    for (; pChar < bufferEnd; ++pChar)
    {
      const char *sep(_WHITE);
            char r(0);
            for (char chCurr(*pChar); (r = *sep) != 0 && chCurr != r; ++sep);
      if (0 == r && *pChar != 0)
      {
        break;
      }
    }
    return pChar;
  }
  inline const char *_find_last_not_white (const char *bufferLast,
                                           const char *bufferFirst)
  {
    const char *pResult(nullptr);
    for (const char *pChar(bufferLast); pChar >= bufferFirst; --pChar)
    {
      const char *pCh(_WHITE);
      char r(0);
      for (char chCurr(*pChar); (r = *pCh) != 0 && chCurr != r; ++pCh);
      if (0 == r)
      {
        pResult = pChar;
        break;
      }
    }
    return pResult;
  }
}
IParser::IParser (const std::string &source,
                  IBaseHandler &handler,
                  const ISchemaBase &schema)
  : _index(0)
  , _length(source.length())
  , _source(source.c_str())
  , _handler(handler)
  , _defaultSchema(nullptr)
  , _schema(schema)
  , _sourceEnd(_source + _length)
{
}

IParser::IParser (const char * const source,
                  size_t length,
                  IBaseHandler &handler,
                  const ISchemaBase &schema)
  : _index(0)
  , _length(length)
  , _source(source)
  , _handler(handler)
  , _defaultSchema(nullptr)
  , _schema(schema)
  , _sourceEnd(_source + _length)
{
}

IParser::IParser (const char * const source,
                  size_t length,
                  IBaseHandler &handler,
                  bool checkNames)
  : _index(0)
  , _length(length)
  , _source(source)
  , _handler(handler)
  , _defaultSchema(new ISchemaBase(checkNames))
  , _schema(*_defaultSchema)
  , _sourceEnd(_source + _length)
{
}

IParser::IParser (const std::string &source,
                  IBaseHandler &handler,
                  bool checkNames)
  : _index(0)
  , _length(source.length())
  , _source(source.c_str())
  , _handler(handler)
  , _defaultSchema(new ISchemaBase(checkNames))
  , _schema(*_defaultSchema)
  , _sourceEnd(_source + _length)
{
}

IParser::~IParser ()
{
  delete _defaultSchema;
}

size_t IParser::parseParts ()
{
  size_t length(0);
  const char *index(_source + _index);
  const char *sourceEnd(_source + _length);
  if (_index + 2 < _length)
  {
    const char *first(nullptr);
    if ('<' == *index)
    {
      if (UNLIKELY('!' == index[1]))
      {
        if (0 == (length = parseOutComments()))
        {
          length = oncdata();
        }
      }
      else
      {
        length = parseTag();
      }
    }
    // characters
    else if (LIKELY((first = _find_first_not_white(index, sourceEnd)) != sourceEnd))
    {
      const char *found(first);
      // filter out formatting
      if ('<' == *first)
      {
      }
      else if ((found = _find(first + 1, sourceEnd, '<')) != sourceEnd)
      {
        // first is already known
        const char *last(nullptr);
        if (nullptr == (last = _find_last_not_white(found - 1, first + 1)))
        {
          last = first;
        }
        _schema.characters(_handler, first, last - first + 1, true);
      }
      else
      {
        throwXMLError(TEXTELEMENTNOTCLOSED, index, _length - _index);
      }
      length = found - index;
    }
  }
  _index += length;
  return length;
}

size_t IParser::parseTag () const
{
  size_t length(0);
  if (LIKELY(_length > _index + 2))
  {
    length = _length - _index;
    const char *buffer(_source + _index),
               *bufferEnd(_sourceEnd),
               *index(buffer + 1),
               *first(index);
    bool startElement(true),
         endElement(false);
    if ('/' == *index)
    {
      ++index;
      ++first;
      startElement = false;
      endElement = true;
    }
    index = _find_first_of(index, bufferEnd, _CHARS1);
    if (UNLIKELY(bufferEnd == index))
    {
      throwXMLError(INCOMPLETEXML, buffer, length);
    }
    else if (UNLIKELY(index == first))
    {
      IXMLERROR errorCode(XMLNOERROR);
      char c(*first);
      switch (c)
      {
      case ' ':
      case '\t':
      case '\r':
      case '\n':
        errorCode = WHITESPACENOTALLOWED;
        break;
      case '>':
      case '/':
        errorCode = EMPTYELEMENTNAME;
        break;
      default:
        break;
      }
      throwXMLError(errorCode, buffer, length);
    }
    IKeyString elementName(first, index - first);
    if (startElement)
    {
      _schema.newElement(elementName);
    }
    char c(*index);
    if (UNLIKELY('/' == c))
    {
      length = index - buffer + 2;
      bufferEnd = buffer + length;
      endElement = true;
    }
    else if ('>' == c)
    {
      length = index - buffer + 1;
      bufferEnd = buffer + length;
    }
    else
    {
      while (index + 1 < bufferEnd
             && (first = _find_first_not_white(index + 1, bufferEnd)) != bufferEnd)
      {
        char c(*first);
        if ('/' == c)
        {
          length = first - buffer + 2;
          bufferEnd = buffer + length;
          endElement = true;
          break;
        }
        else if ('>' == c)
        {
          length = first - buffer + 1;
          bufferEnd = buffer + length;
          break;
        }
        if (UNLIKELY(index + 1 == first)
            && ('\'' == (c = *index) || '\"' == c))
        {
          throwXMLError(NOWHITESPACEBETWEENATTRIBUTES, buffer, length);
        }
        if (UNLIKELY(bufferEnd == (index = _find_first_of(first + 1, bufferEnd, _CHARS2))))
        {
          throwXMLError(INVALIDATTRIBUTEFORMAT, buffer, length);
        }
        const char *name(first);
        size_t nameLength(index - first);
        const char *indEq(index);
        if (UNLIKELY(*indEq != '=')
            && (bufferEnd == (indEq = _find_first_not_white(index + 1, bufferEnd))
                || *indEq != '='))
        {
          throwXMLError(EQUALSIGNEXPECTED, buffer, length);
        }
        first = indEq + 1;
        if (UNLIKELY(*first != '\"')
            && (bufferEnd == (first = _find_first_not_white(first, bufferEnd))
                || ((c = *first) != '\'' && c != '\"')))
        {
          throwXMLError(QUOTEEXPECTED, buffer, length);
        }
        c = *first++;
        if (UNLIKELY(bufferEnd == (index = _find(first, bufferEnd, c))))
        {
          std::string msg(1, '\'');
          msg.append(1, c);
          msg.append("\' expected:");
          throwXMLError(msg.c_str(), buffer, length);
        }
        _schema.insertAttr(name, nameLength, first, index - first);
      }
    }
    if (startElement)
    {
      _schema.startElement(_handler, elementName);
    }
    if (endElement)
    {
      _schema.endElement(_handler, elementName);
    }
    if (UNLIKELY('>' != buffer[length - 1]))
    {
      std::string msg;
      if ('/' == buffer[length - 2])
      {
        msg.assign("incorrectly closed element:");
      }
      else
      {
        msg.assign("\'>\' expected:");
      }
      throwXMLError(msg.c_str(), buffer, length);
    }
  }
  return length;
}

size_t IParser::oncdata () const
{
  size_t length(0),
         index(_index);
  while (index + _cdataStartLength < _length
         && 0 == strncmp(_CDATA_START, _source + index, _cdataStartLength))
  {
    size_t start(index + _cdataStartLength);
    const char *first(_source + start),
               *last(_source + _length),
               *chEnd(last);
    if (last != (chEnd = std::search(first,
                                     last,
                                     _CDATA_END,
                                     _CDATA_END + _cdataEndLength)))
    {
      size_t idx(chEnd - _source);
      _schema.characters(_handler, _source + start, idx - start, false);
      size_t len(idx + _cdataEndLength - index);
      index += len;
      length += len;
    }
    else
    {
      throwXMLError(CDATANOTCLOSED, _source + index, _length - index);
    }
  }
  return length;
}

void IParser::parse ()
{
  _index = 0;
  _schema.startDocument(_handler);
  parseOutComments();
  const char *end(_source + _length);
  char c(0);
  // ignore special element(s)
  while (_index + 2 < _length
         && '<' == _source[_index]
         && ('?' == (c = _source[_index + 1]) || '!' == c))
  {
    const char *found(_find(_source + _index + 2, end, '<'));
    _index = found - _source;
  }
  if (_length == _index)
  {
    throw std::runtime_error(_xmlErrorMsgs[EMPTYXML]);
  }
  // do the rest
  while (parseParts() != 0);
  if (_index < _length && end != _find_first_not_white(_source + _index, end))
  {
    throw std::runtime_error(_xmlErrorMsgs[GARBAGEAFTERTHEEND]);
  }
  _schema.endDocument(_handler);
}

inline size_t IParser::parseOutComments () const
{
  size_t length(0),
         index(_index);
  while (index + _comStartLength < _length
         && 0 == strncmp(_COMMENT_START, _source + index, _comStartLength))
  {
    const char *first(_source + index + _comStartLength),
               *last(_source + _length),
               *chEnd(last);
    if (last != (chEnd = std::search(first,
                                     last,
                                     _COMMENT_END,
                                     _COMMENT_END + _comEndLength)))
    {
      size_t idx(chEnd - _source),
             len(idx + _comEndLength - index);
      index += len;
      length += len;
    }
    else
    {
      throwXMLError(COMMENTNOTCLOSED, _source + index, _length - index);
    }
  }
  return length;
}

void IParser::throwXMLError (const char *prefix,
                             const char *buffer,
                             size_t length) const
{
  const char *bufferEnd(buffer + length),
             *close(_find(buffer, bufferEnd, '>'));
  if (close != bufferEnd)
  {
    ++close;
  }
  std::string error(buffer, close - buffer);
  error = prefix + error;
  throw std::runtime_error(error);
}

void IParser::throwXMLError (IXMLERROR errorCode,
                             const char *buffer,
                             size_t length) const
{
  throwXMLError(_xmlErrorMsgs[errorCode], buffer, length);
}
