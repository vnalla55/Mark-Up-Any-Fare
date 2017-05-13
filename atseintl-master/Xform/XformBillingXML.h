//----------------------------------------------------------------------------
//      File:        XformBillingXML.h
//      Created:     February 8, 2006
//      Authors:     Valentin Perov
//      Description:
//
//  Copyright Sabre 2006
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//----------------------------------------------------------------------------
#pragma once

#include "Xform/Xform.h"


namespace tse
{

class TseServer;
class XformBillingXML;
class Billing;

class XformBillingXML : public Xform
{
public:
  XformBillingXML(const std::string& name, ConfigMan& config);
  XformBillingXML(const std::string& name, TseServer& srv);
  virtual ~XformBillingXML();

  virtual bool initialize(int argc, char* argv[]) override;

  virtual bool convert(Trx& trx, std::string& response) override;

  // pure methods from Xform, which I don't need
  virtual bool convert(DataHandle& dataHandle, std::string& request, Trx*& trx, bool throttled) override;
  virtual bool convert(tse::ErrorResponseException& ere, Trx& trx, std::string& response) override;

protected:
  void buildXML(const Billing& billing, const Trx& trx, std::string& xmlString);

private:
  // Placed here so they wont be called
  XformBillingXML(const XformBillingXML& rhs);
  XformBillingXML& operator=(const XformBillingXML& rhs);
};

// INLINES
inline bool
XformBillingXML::convert(tse::ErrorResponseException& ere, Trx& trx, std::string& response)
{
  return true;
}

inline bool
XformBillingXML::convert(DataHandle& dataHandle, std::string& request, Trx*& trx, bool throttled)
{
  return true;
}

} // tse

