//----------------------------------------------------------------------------
//
//  File:  RBRequestFormatter.cpp
//  Copyright Sabre 2003
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

#include "RTG/RBRequestFormatter.h"

#include "BookingCode/RBData.h"
#include "Common/ErrorResponseException.h"
#include "Common/FareDisplayUtil.h"
#include "RTG/GenerateRuleRequestFormatter.h"

#include <vector>

namespace tse
{
void
RBRequestFormatter::buildRequest(FareDisplayTrx& trx, bool skipDiag854)
{
  // RBData data(trx);
  if (_rbData.rbItems().empty())
  {
    return;
  }
  else if (populateXMLTags(trx, _rbData.rbItems(), skipDiag854))
  {
  }
}

bool
RBRequestFormatter::populateXMLTags(const FareDisplayTrx& trx,
                                    std::vector<RBDataItem*>& items,
                                    bool skipDiag854)
{
  XMLConstruct construct;
  construct.openElement("GenerateRBRequest");
  GenerateRuleRequestFormatter formatter(true);
  formatter.addTVLType(trx, construct, skipDiag854);
  std::vector<RBDataItem*>::iterator i(items.begin()), start(items.begin()), end(items.end());
  for (i = start; i != end; ++i)
  {
    addRBKType(construct, **i);
  }
  construct.closeElement();
  _request = construct.getXMLData();
  return (_request.empty() == false);
}

void
RBRequestFormatter::addRBKType(XMLConstruct& construct, RBDataItem& item)
{

  construct.openElement("RBK");

  construct.addAttribute("S37", item._vendor);

  char tmpBuf[10];
  sprintf(tmpBuf, "%d", item._itemNo);
  construct.addAttribute("Q41", tmpBuf);

  sprintf(tmpBuf, "%d", item._seqNo);
  construct.addAttribute("Q1K", tmpBuf);

  sprintf(tmpBuf, "%d", item._segNo);
  construct.addAttribute("Q0C", tmpBuf);

  construct.closeElement();
}
}
