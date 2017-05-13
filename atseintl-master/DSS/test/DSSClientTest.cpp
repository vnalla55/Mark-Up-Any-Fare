#include "test/include/CppUnitHelperMacros.h"
#include "DSS/test/DSSClientTest.h"

#include "AppConsole/SocketUtils.h"
#include "DataModel/Itin.h"
#include "Common/DateTime.h"
#include <iostream>
#include <fstream>

using namespace tse;
using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION(DSSClientTest);

void
DSSClientTest::setUp()
{
}
void
DSSClientTest::tearDown()
{
}

void
DSSClientTest::testgetScheduleCount()
{
  ac::SocketUtils::Message req, res;
  req.command = "RQST";
  req.xmlVersion = "10";
  req.payload = "<DSS ARI=\"false\" COR=\"TVL\" DEP=\"true\" DNR=\"true\" DUP=\"false\" "
                "ETX=\"false\" IGP=\"false\" IGS=\"true\" ONL=\"false\" VER=\"2.0\" VRB=\"NO\" "
                "XCC=\"false\" XCP=\"false\" XCT=\"false\" XSC=\"false\" XTR=\"false\"><BIL "
                "AAA=\"KE0A\" AKD=\"S\" ASI=\"RKG\" CSV=\"SGSCHEDS\" PID=\"AA\" "
                "TXN=\"-4469273311278856959\" UBR=\"3470\" UCD=\"HDQ\" USA=\"F22606\" "
                "UST=\"925\"/><UID CTY=\"DFW\" OWN=\"1S\" UDD=\"34\" WEB=\"false\"/><ODI "
                "BRD=\"DFW\" BTP=\"A\" OFF=\"JFK\" OTP=\"A\"/><DAT BK1=\"0\" OUT=\"0\" "
                "TG1=\"2005-12-27\"/><TIM BK2=\"360\" OUT=\"1080\" TGT=\"360\"/><FLT MAX=\"20\" "
                "MIN=\"6\"/><ENT DAT=\" \" TYP=\" \"/><CNX CMN=\"0\" CMX=\"0\"/><SEG SMN=\"1\" "
                "SMX=\"10\"/><STP TMN=\"0\" TMX=\"15\"/> <FCT IIC=\"1\" IOC=\"1\"/></DSS>";

  Socket socket;
  int tmp = socket.connect("pinlc101.sabre.com", 65001);

  /*if(tmp == 0)
  {
       std::cout << "SuccessFull " << std::cout;
  }
  else
  {
  std::cout << "Unsucessful " << std::cout;
   }*/

  CPPUNIT_ASSERT(ac::SocketUtils::writeMessage(socket, req, true));
  CPPUNIT_ASSERT(ac::SocketUtils::readMessage(socket, res, true));
  /**
  std::fstream file;
  file.open("out.txt", ios::out)
      file << " This is Test";
  file.close();
  **/
  // std::cout << res.payload;
}
