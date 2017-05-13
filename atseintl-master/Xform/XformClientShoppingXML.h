//----------------------------------------------------------------------------
//
//      File: XformClientShoppingXML.h
//      Description: Class to transform client XML to/from Trx
//      Created: August 04, 2004
//      Authors: David White
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
//----------------------------------------------------------------------------

#pragma once

#include "Xform/Xform.h"
#include "Xform/XMLWriter.h"

namespace tse
{

class Itin;
class PricingTrx;
class TseServer;
class XformClientShoppingXML;
class Logger;
class Cabin;
class AirSeg;

class XformClientShoppingXML : public Xform
{
  friend class XformClientShoppingXMLTest;

public:
  XformClientShoppingXML(const std::string& name, ConfigMan& config);
  XformClientShoppingXML(const std::string& name, TseServer& srv);

  virtual ~XformClientShoppingXML();

  virtual bool convert(DataHandle& dataHandle, std::string& request, Trx*& trx, bool throttled) override;

  virtual bool convert(Trx& trx, std::string& response) override;

  virtual bool convert(ErrorResponseException& ere, Trx& trx, std::string& response) override;
  virtual bool convert(ErrorResponseException& ere, std::string& response) override;

  virtual bool initialize(int argc, char** argv) override;

private:
  static void validateTrx(PricingTrx& trx);
  static void populateBookCabinForWPNIC(std::vector<Itin*>& itins, PricingTrx& trx);
  static const Cabin* getCabin(PricingTrx& trx, const AirSeg& airSeg);
  void performDiagnostic982(PricingTrx& trx, const std::map<int, Itin*>& itinMap) const;

  XMLWriter _writer;

  typedef XMLWriter::Node Node;
  static uint16_t _minAccCorpIds;

  static Logger _logger;
};

} // tse

