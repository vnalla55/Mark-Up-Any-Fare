#pragma once

#include "Xform/CustomXMLParser/ICString.h"

#include <stack>
#include <vector>

#include <tr1/unordered_map>

class IBaseHandler;

typedef std::tr1::unordered_map<IKeyString, IValueString, IKeyStringHasher> IMap;

class ISchemaBase
{
public:
  ISchemaBase (bool checkWellFormedness);
  virtual ~ISchemaBase ();

  virtual void insertAttr (const char *name,
                           size_t nameLength,
                           const char *value,
                           size_t valueLength) const;
  virtual void startElement (IBaseHandler &handler,
                             const IKeyString &name) const;
  virtual void endElement (IBaseHandler &handler,
                           const IKeyString &name) const;
  virtual void characters (IBaseHandler &handler,
                           const char *value,
                           size_t length,
                           bool bdecode) const;
  virtual void newElement (const IKeyString &name) const;
  virtual void clear () const;

  void startDocument (IBaseHandler &handler) const;
  void endDocument (IBaseHandler &handler) const;
protected:
  void onEndDocument () const;
  void onStartElement (const IKeyString &name) const;
  void onEndElement (const IKeyString &name) const;
  void decode (const char *&buffer,
               size_t &len) const;
  static void checkName (const IKeyString &name);

  const bool _checkWellFormedness;
  mutable int _nbrOfRoots;
  mutable std::stack<IKeyString> _checkStack;
  mutable int _attrsSize;
private:
  mutable std::vector<std::vector<char> > _modifiedValues;
  mutable IMap _map;
  // not implemented
  ISchemaBase (const ISchemaBase &);
  ISchemaBase &operator = (const ISchemaBase &);
};
