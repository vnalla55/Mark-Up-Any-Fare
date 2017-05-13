// ----------------------------------------------------------------
//
//   Copyright Sabre 2016
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#include "Xform/XMLConvertUtils.h"
#include <gtest/gtest.h>
#include <string>

#define private public
#include "Xform/XformClientXML.h"

namespace tse
{
static std::string correctSortedWithPXI{"<REQUEST><MORE/><BIL/><PXI/><SGI/><MORE/></REQUEST>"};
static std::string correctSortedWithoutPXI{"<REQUEST><MORE/><BIL/><SGI/><MORE/></REQUEST>"};

TEST(MultiPAXTagReorderingTest, RightBILPXISGIOrdering)
{
  ConfigMan cm{};
  XformClientXML client{"dummy", cm};

  std::string request{"<REQUEST><MORE/><BIL/><PXI/><SGI/><MORE/></REQUEST>"};
  client.sortXmlTags(request.c_str());
  ASSERT_EQ(request, correctSortedWithPXI);
}

TEST(MultiPAXTagReorderingTest, WrongPXISGIBIL)
{
  ConfigMan cm{};
  XformClientXML client{"dummy", cm};

  std::string requestWithPXI{"<REQUEST><MORE/><PXI/><SGI/><BIL/><MORE/></REQUEST>"};
  client.sortXmlTags(requestWithPXI.c_str());
  ASSERT_EQ(requestWithPXI, correctSortedWithPXI);
}

TEST(MultiPAXTagReorderingTest, WrongPXIBILSGI)
{
  ConfigMan cm{};
  XformClientXML client{"dummy", cm};

  std::string requestWithPXI{"<REQUEST><MORE/><PXI/><BIL/><SGI/><MORE/></REQUEST>"};
  client.sortXmlTags(requestWithPXI.c_str());
  ASSERT_EQ(requestWithPXI, correctSortedWithPXI);
}

TEST(MultiPAXTagReorderingTest, WrongSGIPXIBIL)
{
  ConfigMan cm{};
  XformClientXML client{"dummy", cm};

  std::string requestWithPXI{"<REQUEST><MORE/><SGI/><PXI/><BIL/><MORE/></REQUEST>"};
  client.sortXmlTags(requestWithPXI.c_str());
  ASSERT_EQ(requestWithPXI, correctSortedWithPXI);
}

TEST(MultiPAXTagReorderingTest, WrongBILSGIPXI)
{
  ConfigMan cm{};
  XformClientXML client{"dummy", cm};

  std::string requestWithPXI{"<REQUEST><MORE/><BIL/><SGI/><PXI/><MORE/></REQUEST>"};
  client.sortXmlTags(requestWithPXI.c_str());
  ASSERT_EQ(requestWithPXI, correctSortedWithPXI);
}

TEST(MultiPAXTagReorderingTest, WrongSGIBILPXI)
{
  ConfigMan cm{};
  XformClientXML client{"dummy", cm};

  std::string requestWithPXI{"<REQUEST><MORE/><SGI/><BIL/><PXI/><MORE/></REQUEST>"};
  client.sortXmlTags(requestWithPXI.c_str());
  ASSERT_EQ(requestWithPXI, correctSortedWithPXI);
}

TEST(MultiPAXTagReorderingTest, NoPIXInXML)
{
  ConfigMan cm{};
  XformClientXML client{"dummy", cm};

  std::string request1{"<REQUEST><MORE/><BIL/><SGI/><MORE/></REQUEST>"};
  client.sortXmlTags(request1.c_str());
  ASSERT_EQ(request1, correctSortedWithoutPXI);

  std::string request2{"<REQUEST><MORE/><SGI/><BIL/><MORE/></REQUEST>"};
  client.sortXmlTags(request2.c_str());
  ASSERT_EQ(request2, correctSortedWithoutPXI);
}
}
