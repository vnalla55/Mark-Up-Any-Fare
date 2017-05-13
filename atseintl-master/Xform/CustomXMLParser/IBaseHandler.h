#pragma once

#include "Xform/CustomXMLParser/ICString.h"

class IAttributes;

class IBaseHandler
{
public:
  virtual ~IBaseHandler() = default;
  IBaseHandler(const IBaseHandler&) = delete;
  IBaseHandler& operator=(const IBaseHandler&) = delete;

  virtual bool startElement (const IKeyString &name,
                             const IAttributes &attrs) = 0;
  virtual void characters (const char *pValue,
                           size_t length) = 0;
  virtual bool endElement (const IKeyString &name) = 0;

  virtual bool startElement (int idx,
                             const IAttributes &attrs) = 0;
  virtual bool endElement (int idx) = 0;

  virtual void startDocument() {}
  virtual void endDocument() {}

protected:
  IBaseHandler () {}
};

