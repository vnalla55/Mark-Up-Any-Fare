// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include <boost/ptr_container/ptr_vector.hpp>

#include "DataModel/Common/SafeEnums.h"
#include "DomainDataObjects/Message.h"
#include "DomainDataObjects/OptionalService.h"
#include "DomainDataObjects/Parameter.h"
#include "Rules/CopyableStream.h"

namespace tax
{
class OptionalService;

template <typename D, void (D::*printFunction)()>
class Printer
{
public:
  Printer() : _enabled(false), _printFunction(printFunction) {}
  void enable() { _enabled = true; }
  void print(D& diagnostic) const
  {
    if (_enabled)
      (diagnostic.*_printFunction)();
  }

private:
  bool _enabled;
  void (D::*_printFunction)();
};

class AtpcoDiagnostic
{
public:
  static const size_t LINE_LENGTH;
  AtpcoDiagnostic();
  virtual ~AtpcoDiagnostic();

  boost::ptr_vector<Message> const& messages() const { return _messages; }

  void createMessages(boost::ptr_vector<Message>& messages);

protected:
  virtual void printHeader() = 0;
  virtual void printFooter() {}
  virtual void applyParameters() = 0;
  virtual void runAll() = 0;

  void printInputFilterHelp();

  void printLine(const char c);
  void printHeaderLong(const std::string& content);
  void printHeaderShort(const std::string& content);

  CopyableStream _result;

private:
  boost::ptr_vector<Message> _messages;
};
}
