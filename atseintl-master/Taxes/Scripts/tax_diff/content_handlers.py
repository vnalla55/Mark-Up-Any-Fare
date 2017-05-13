import re
import xml.sax.handler

months = {1:'JAN',2:'FEB',3:'MAR',4:'APR',5:'MAY',6:'JUN',7:'JUL',8:'AUG',9:'SEP',10:'OCT',11:'NOV',12:'DEC'}

# Request handlers
class BaseReqContentHandler(xml.sax.handler.ContentHandler, object):
    def __init__(self):
        self._cmd = ''
        self._PCC = None
        self._seg_no = 0
        self.request = ''
        self._tktDate = ''

class WPReqContentHandler(BaseReqContentHandler):
    def __init__(self):
        super(WPReqContentHandler, self).__init__()
        self._countries = set()
        self._carriers = set()
        self._airline = None
        self._mainProTag = 0

    def startElement(self, name, attrs):
        if name == 'AGI':
            self._PCC = attrs.get('A20',None)
        if name == 'ITN':
            self._seg_no = 0
        if name == 'SGI':
            self._seg_no += 1
        if name == 'PRO':
            tktDate = attrs.get('D07','')
            if (tktDate != ''):
                self._tktDate = tktDate
                self._mainProTag = 1
            else:
                self._mainProTag = 0
            self._cmd += attrs.get('S14','')
        if name == 'BIL':
            self._airline = attrs.get('AE0')
            self._cmd += attrs.get('A70','')
        if name == 'FLI':
            orig = attrs.get('A01')
            dest = attrs.get('A02')

            is_ARNK = attrs.get('N03') == 'K'
            if not is_ARNK:
                carrier = attrs.get('B00')
                self._carriers.add(carrier)
                date = attrs.get('D01','OPEN')
                m = re.search('(\S+)-(\S+)-(\S+)',date)
                if m is not None:
                    date = m.group(3) + months[int(m.group(2))]
                cos = attrs.get('B30')
                flt = attrs.get('Q0B','0')
                self.request += "{0} {1}{2:4d}{3} {4} {5}{6}\n".format(self._seg_no, carrier, int(flt), cos, date, orig, dest)

        if name == 'AAF':
            self._carrier = attrs.get('B00')
            self._flight = int(attrs.get('Q0B'))
            self._cos = attrs.get('P0Z')
            self._seg_no += 1

        if name == 'BRD':
            self._orig = attrs.get('A01')
            date = attrs.get('D01')
            m = re.search('(\S+)-(\S+)-(\S+)', date)
            if m is not None:
                self.__date = m.group(3) + months[int(m.group(2))]

        if name == 'OFF':
            self._dest = attrs.get('A02')

    def endElement(self, name):
        if name == 'PRO' and self._mainProTag:
            if self._PCC is not None:
                self.request += 'PCC: {0}\n'.format(self._PCC)
            else:
                self.request += 'Airline: {0}\n'.format(self._airline)
            self.request += 'Ticketing date: {}\n'.format(self._tktDate)
            self.request += 'Pricing command: {}\n'.format(self._cmd)
        if name == 'AAF':
          self.request += "{0} {1}{2:4d}{3} {4} {5}{6}\n".format(self._seg_no, self._carrier, self._flight, 
                                                                 self._cos, self._date, self._orig, self._dest)

class AirTaxReqContentHandler(BaseReqContentHandler):
    def __init__(self):
        super(AirTaxReqContentHandler, self).__init__()
        self._flt = 0
        self._bookingCode = ''
        self._orig = ''
        self._dest = ''

    def startElement(self, name, attrs):
        if name == 'Source':
            pcc = attrs.get('PseudoCityCode',None)
            if pcc is not None:
               self.request += 'PCC: {0}\n'.format(pcc)
        if name == 'Item':
            self._seg_no = 0
            self.request += "\n"
        if name == 'FlightSegment':
            self._seg_no += 1
            self._flt = attrs.get('FlightNumber')
            self._bookingCode = attrs.get('ResBookDesigCode')

            date = attrs.get('DepartureDateTime')[:10]
            m = re.search('(\S+)-(\S+)-(\S+)',date)
            if m is not None:
              self._date = m.group(3) + months[int(m.group(2))]
        if name == 'MarketingAirline':
            self._carrier = attrs.get('Code')
        if name == 'DepartureAirport':
            self._orig = attrs.get('LocationCode')
        if name == 'ArrivalAirport':
            self._dest = attrs.get('LocationCode')

    def endElement(self, name):
        if name == 'FlightSegment':
            self.request += "{0} {1}{2:4d}{3} {4} {5}{6}\n".format(self._seg_no, self._carrier, int(self._flt),
                                                                   self._bookingCode, self._date, self._orig, self._dest)

# Response handlers
class BaseResContentHandler(xml.sax.handler.ContentHandler):
    def __init__(self):
        self.tax_by_group = dict()
        self.in_net = 0

    def getTaxCode(self, txc):
        if txc[0:2] == 'US':
            return txc
        return txc[0:2]

class WPResContentHandler(BaseResContentHandler):
    def startElement(self, name, attrs):
        if name == 'ITN': # shopping
            self.current_itin = attrs.get('NUM','NUM')
        if name == 'PSG': # shopping
            self.current_group_taxes = dict()
            self.current_group = "ITN" + self.current_itin + "/" + attrs.get('B70','PTC')
            self.current_group_type = "PTC_ITN"

        if name == 'NET':
            self.in_net = 1

        if name == 'PXI': # pricing
            self.current_group_taxes = dict()
            self.current_group = attrs.get('B70','PTC')
            self.current_group_type = "PTC"

        if name == 'TAX' and (not self.in_net): # all
            taxcode = attrs.get('BC0','##')
            taxcode = self.getTaxCode(taxcode)
            taxamount = float(attrs.get('C6B','0')) + self.current_group_taxes.get(taxcode, 0)
            self.current_group_taxes.update({taxcode : taxamount})

        if name == 'OCP': # oc and pricing
            self.current_group_taxes = dict()
            self.oc_name = 'OC='+attrs.get('SFF','')
            self.carrier = attrs.get('SFK','CX')
            self.ptc = attrs.get('SHF','PTC')
            self.segment = attrs.get('SSG','S')
            self.current_group_type = "OC"

        if name == 'OSC': # oc and pricing
            self.oc_name = 'OC='+attrs.get('SFF','')

        if name == 'OOS': # oc and pricing
            self.current_group_taxes = dict()
            self.current_group_type = "OC"
            self.segment = attrs.get('A01','???') + attrs.get('A02','???')

        if name == 'SUM':
            self.ptc = attrs.get('B70','PTC')

    def endElement(self, name):
        if name == 'PXI' or name == 'PSG':
            self.tax_by_group.update({self.current_group : self.current_group_taxes})
        if name == 'NET':
            self.in_net = 0
        if name == 'OCP':
            self.current_group = '{0}-{1} {2} {3}'.format(self.oc_name, self.carrier, self.ptc, self.segment)
            self.tax_by_group.update({self.current_group : self.current_group_taxes})
        if name == 'OOS':
            self.current_group = '{0}-{1} {2}'.format(self.oc_name, self.ptc, self.segment)
            self.tax_by_group.update({self.current_group : self.current_group_taxes})

class AirTaxResContentHandler(BaseResContentHandler):
    def startElement(self, name, attrs):
        if name == 'PassengerType':
            self.current_group_taxes = dict()
            self.current_group = attrs.get('Code','PTC')

        if name == 'Tax':
            taxcode = attrs.get('TaxCode','##')
            taxcode = self.getTaxCode(taxcode)
            taxamount = float(attrs.get('Amount','0')) + self.current_group_taxes.get(taxcode, 0)
            self.current_group_taxes.update({taxcode : taxamount})

    def endElement(self, name):
        if name == 'PassengerType':
            self.tax_by_group.update({self.current_group: self.current_group_taxes})
