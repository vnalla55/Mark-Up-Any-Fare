import time
import socket
import struct
import select
import re

class ATSEClient(object):
    def __init__(self, host, port, timeout=300, substC20=True):
        self.__address = (host, port)
        self.__timeout = timeout
        self.__substC20 = substC20

    def send(self, request):
        response = ""
        try:
            xmlRequest = self.__getxml(request)
            if self.__substC20:
                xmlRequest = re.sub(r'C20=".*?"', r'C20="SOCKCLIENT"', xmlRequest)
            s = socket.socket()
            s.connect(self.__address)
            s.sendall(self.__header(xmlRequest) + xmlRequest)
            tstart = time.time()
            select_rsp = select.select([s],[],[],self.__timeout)
            if not s in select_rsp[0]:
                response = ' ' * 8 + self.__add_error_msg('Timeout')
            else:
                while True:
                    r = s.recv(4096)
                    if not r: break
                    response += r
            response = self.__getxml(response[8:])
        except ValueError:
            response = self.__add_error_msg('Invalid request or response')
        except (socket.error, select.error) as e:
            print str(self), str(e)
            raise e

        finally:
            try:
                s.shutdown(socket.SHUT_RDWR)
                s.close()
            except socket.error:
                pass
        tend = time.time()
        resptime = tend - tstart
        return (response, resptime)

    def __str__(self):
        return self.__address[0] + ":" + str(self.__address[1])

    @staticmethod
    def __add_error_msg(msgtext):
        return '<Error>' + msgtext + '</Error>'

    @staticmethod
    def __getxml(line):
        xmlStart, xmlEnd = line.index('<'), line.rindex('>')
        return line[xmlStart:xmlEnd+1]

    @staticmethod
    def __header(request):
        return struct.pack('!ll4s8s8s', 16,len(request)+8,'REQ','00010000','TransNum')
