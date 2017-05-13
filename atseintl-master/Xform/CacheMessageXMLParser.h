//-------------------------------------------------------------------
//
//  File:        CacheMessageXMLParser.h
//
//  Description: Implements parsing of XML cache update requests
//
//  Copyright Sabre 2004
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

#pragma once

#include "DataModel/CacheTrx.h"
#include "DBAccess/DataHandle.h"

#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>

#include <stack>

namespace tse
{

class CacheMessageXMLParser
{
public:
  CacheMessageXMLParser(CacheTrx& trx, DataHandle& dataHandle);
  virtual ~CacheMessageXMLParser();

  void startElement(const XMLCh* element, const xercesc::Attributes& attributes);
  void endElement(const XMLCh* element, const XMLCh* text);

private:
  class ElementParser
  {
  public:
    virtual ~ElementParser();

    virtual void startParsing(const xercesc::Attributes& attributes) {}

    virtual void endParsing(const XMLCh* text) {}

    virtual ElementParser* newElement(const XMLCh* element, const xercesc::Attributes& attributes)
    {
      return nullptr;
    }

  protected:
    ElementParser(CacheMessageXMLParser& parser);

    CacheMessageXMLParser& parser() { return _parser; }
    CacheTrx& trx();
    DataHandle& dataHandle();

  private:
    CacheMessageXMLParser& _parser;

    ElementParser(const ElementParser&);
    void operator=(const ElementParser&);
  };

  // a parser for <CacheWarmUp> elements
  class CacheWarmUpParser : public ElementParser
  {
  public:
    CacheWarmUpParser(CacheMessageXMLParser& parser);
    virtual ~CacheWarmUpParser();

    // virtual void startParsing(const Attributes& attributes);
    // virtual void endParsing(const XMLCh* text);
    virtual ElementParser* newElement(const XMLCh* element, const xercesc::Attributes& attributes) override;

  private:
    void parseObjectType(const xercesc::Attributes& attributes);
  };

  // a parser for <CacheUpdate> elements
  class CacheUpdateParser : public ElementParser
  {
  public:
    CacheUpdateParser(CacheMessageXMLParser& parser);
    virtual ~CacheUpdateParser();

    // virtual void startParsing(const Attributes& attributes);
    // virtual void endParsing(const XMLCh* text);
    virtual ElementParser* newElement(const XMLCh* element, const xercesc::Attributes& attributes) override;

  private:
    void parseObjectType(const xercesc::Attributes& attributes);
  };

  // a parser for <Object> elements
  class ObjectTypeParser : public ElementParser
  {
  public:
    ObjectTypeParser(CacheMessageXMLParser& parser);
    virtual ~ObjectTypeParser();

    virtual void startParsing(const xercesc::Attributes& attributes) override;
    virtual void endParsing(const XMLCh* text) override;
    virtual ElementParser* newElement(const XMLCh* element, const xercesc::Attributes& attributes) override;
  };

  // a parser for <Name> elements
  class NameTypeParser : public ElementParser
  {
  public:
    NameTypeParser(CacheMessageXMLParser& parser);
    virtual ~NameTypeParser();

    // virtual void startParsing(const Attributes& attributes);
    virtual void endParsing(const XMLCh* text) override;
    // virtual ElementParser* newElement(const XMLCh* element,
    //                                   const Attributes& attributes);
  };

  // a parser for <Key> elements
  class KeyTypeParser : public ElementParser
  {
  public:
    KeyTypeParser(CacheMessageXMLParser& parser);
    virtual ~KeyTypeParser();

    // virtual void startParsing(const Attributes& attributes);
    // virtual void endParsing(const XMLCh* text);
    virtual ElementParser* newElement(const XMLCh* element, const xercesc::Attributes& attributes) override;
  };

  // a parser for <Field> elements
  class FieldTypeParser : public ElementParser
  {
  public:
    FieldTypeParser(CacheMessageXMLParser& parser);
    virtual ~FieldTypeParser();

    virtual void startParsing(const xercesc::Attributes& attributes) override;
    virtual void endParsing(const XMLCh* text) override;
    // virtual ElementParser* newElement(const XMLCh* element,
    //                                   const Attributes& attributes);
  };

  // the parsers that can be returned from 'newElement()'
  CacheWarmUpParser _cacheWarmUpParser;
  CacheUpdateParser _cacheUpdateParser;
  ObjectTypeParser _objectParser;
  KeyTypeParser _keyParser;
  NameTypeParser _nameParser;
  FieldTypeParser _fieldParser;

  CacheTrx& _trx;
  DataHandle& _dataHandle;

  ObjectKey _objectKey;
  std::string _fieldId;

  // pointers to the stack of parsers. The parser on the top of
  // the stack is the parser currently active. A parser is pushed
  // onto the stack every time an element is opened, and popped
  // off the stack every time an element is closed.
  std::stack<ElementParser*> _parsers;

  // forbidden operations
  CacheMessageXMLParser(const CacheMessageXMLParser&);
  void operator=(const CacheMessageXMLParser&);
};
}

