#pragma once

#include "Xform/CustomXMLParser/IXMLErrorMsgs.h"

#include <string>

class IBaseHandler;
class ISchemaBase;

class IParser
{
public:
  IParser (const char source[],
           size_t length,
           IBaseHandler &handler,
           const ISchemaBase &schema);
  IParser (const std::string &source,
           IBaseHandler &handler,
           const ISchemaBase &schema);
  IParser (const char * const source,
           size_t length,
           IBaseHandler &handler,
           bool checkNames = false);
  IParser (const std::string &source,
           IBaseHandler &handler,
           bool checkNames = false);
  ~IParser ();

  void parse ();
private:
  size_t parseParts ();
  size_t parseTag () const;
  size_t parseOutComments () const;
  size_t oncdata () const;
  void throwXMLError (const char *prefix,
                      const char *buffer,
                      size_t length) const;
  void throwXMLError (IXMLERROR errorCode,
                      const char *buffer,
                      size_t length) const;
  size_t _index;
  const size_t _length;
  const char * const _source;
  IBaseHandler &_handler;
  ISchemaBase *_defaultSchema;
  const ISchemaBase &_schema;
  const char * const _sourceEnd;
  // not implemented
  IParser (const IParser &);
  IParser &operator = (const IParser &);
};
