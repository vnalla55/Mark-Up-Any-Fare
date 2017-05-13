#!/atse_git/fbldfunc/python/bin/python
#####################
num_workers = 8

################### Don't change anything below this line. #######################
import time
import sys
import Queue
import threading
import heapq
import os
import base64
import bz2
import traceback
import xml.sax.handler

from ATSEClient import ATSEClient
from tax_comparator import compare_taxes
from content_handlers import WPReqContentHandler
from content_handlers import AirTaxReqContentHandler

host_port1 = (sys.argv[1], int(sys.argv[2])) if len(sys.argv) >= 3 else ("atsebld101.dev.sabre.com", 13579)
host_port2 = (sys.argv[3], int(sys.argv[4])) if len(sys.argv) >= 5 else ("atsebld101.dev.sabre.com", 13580)

cnx1 = ATSEClient(*host_port1, substC20='TXDIFFTOOL')
cnx2 = ATSEClient(*host_port2, substC20='TXDIFFTOOL')
lock = threading.Lock()

def worker(in_queue):
    while True:
        V2cnx, req_idx, request, out_queue = in_queue.get()
        if __debug__:
            with lock:
                print "Sending req#", req_idx, "to", V2cnx
        try:
          xml_rsp, rsp_time = V2cnx.send(request)
        except:
          traceback.print_exc()
          os._exit(0)
        if __debug__:
            with lock:
                print "Response to",req_idx, "received from", V2cnx, "after %.2f" % rsp_time,"seconds"
        in_queue.task_done()
        out_queue.put((req_idx, request, xml_rsp, rsp_time))

outQueue1 = Queue.Queue()
outQueue2 = Queue.Queue()
inQueue = Queue.Queue(num_workers)

for i in range(num_workers):
    threading.Thread(target=lambda : worker(inQueue)).start()

def receiver(outQueue):
    resp_needed = 1
    pq = []
    while True:
        resp = outQueue.get()
        outQueue.task_done()
        if resp[0] != resp_needed:
            pq.append(resp)
        else:
            yield resp
            resp_needed += 1
            heapq.heapify(pq)
            while len(pq) >0 and pq[0][0] == resp_needed:
                yield heapq.heappop(pq)
                resp_needed += 1

resp_time_avrg1 = 0
resp_time_avrg2 = 0
processed_count = 0
logfile = open("tx_diff_tool.log","w")

def decompress_if_needed(xml):
    dts_pos = xml.find('DTS')
    if dts_pos != -1:
        b64_begin = xml.find('>', dts_pos) + 1
        b64_end   = xml.find('<', b64_begin)
        encoded = xml[b64_begin:b64_end]
        decoded = bz2.decompress(base64.b64decode(encoded))
        xml = xml[:b64_begin] + decoded + xml[b64_end:]
    return xml

def get_WP(line):
    xmlStart, xmlEnd = line.find('<PricingRequest'), line.rfind('</PricingRequest>')
    if xmlStart < 0 or xmlEnd <= 0:
      xmlStart, xmlEnd = line.find('<PRICINGREQUEST'), line.rfind('</PRICINGREQUEST>')
    if xmlStart < 0 or xmlEnd <= 0:
      xmlStart, xmlEnd = line.find('<AirTaxRQ'), line.rfind('</AirTaxRQ>')
    if xmlStart < 0 or xmlEnd <= 0:
      xmlStart, xmlEnd = line.find('<AncillaryPricingRequest'), line.rfind('</AncillaryPricingRequest>')
    if xmlStart < 0 or xmlEnd <= 0:
      xmlStart, xmlEnd = line.find('<ShoppingRequest'), line.rfind('</ShoppingRequest>')

    return line[xmlStart:] if xmlStart >= 0 and xmlEnd > 0 else None

def is_AirTaxRq(line):
    xmlStart, xmlEnd = line.find('<AirTaxRQ'), line.rfind('</AirTaxRQ>')
    return True if xmlStart >= 0 and xmlEnd > 0 else False

def process(rsp1, rsp2):
    global resp_time_avrg1
    global resp_time_avrg2
    global processed_count
    global stats
    req_idx1, req1, response1, response_time1 = rsp1
    req_idx2, req2, response2, response_time2 = rsp2
    processed_count += 1
    resp_time_avrg1 += response_time1
    resp_time_avrg2 += response_time2
    with lock:
        print "Processing ", req_idx1,
        if req_idx1 != req_idx2:
            print "General fuck-up number 231232345 occurred, exiting", req_idx1, req_idx2
            raise SystemExit
        if req1 != req2:
            print "General fuck-up number 424323121 occurred, exiting", req1, req2
            raise SystemExit
        r1_cleaned = decompress_if_needed(response1)
        r2_cleaned = decompress_if_needed(response2)

        rq_parser = WPReqContentHandler() if not is_AirTaxRq(req1) else AirTaxReqContentHandler()
        try:
            xml.sax.parseString(req1, rq_parser)
            req = rq_parser.request
        except:
            print sys.exc_info()
            req = 'UNABLE TO PARSE REQUEST'
        print >> logfile,'---------- Trx No {} -----------'.format(req_idx1)
        print >> logfile, req
        print >> logfile, compare_taxes(r1_cleaned, r2_cleaned)
        logfile.flush()


requests_file = sys.stdin #open(sys.argv[1], "r")
time_start = time.time()

### main loop
all_read = False
read_count = 0
rcv1 = receiver(outQueue1)
rcv2 = receiver(outQueue2)

while not all_read:
    try:
        while not inQueue.full():
            req = get_WP(requests_file.next())
            if req is None:
                continue
            read_count = read_count + 1
            inQueue.put((cnx1, read_count, req, outQueue1))
            inQueue.put((cnx2, read_count, req, outQueue2))
    except StopIteration:
        all_read = True
        requests_file.close()
    except:
      traceback.print_exc()
      os._exit(0)

    while not outQueue1.empty() and not outQueue2.empty():
      try:
        process(rcv1.next(), rcv2.next())
      except:
        traceback.print_exc()
        os._exit(0)


while processed_count != read_count:
  try:
    process(rcv1.next(), rcv2.next())
  except:
    traceback.print_exc()
    os._exit(0)

print "Total number of requests processed: ", read_count
print "Elapsed time: ",time.time() - time_start
print "Average response times: "
if read_count:
  print "  ", cnx1, ":", resp_time_avrg1/read_count
  print "  ", cnx2, ":", resp_time_avrg2/read_count
logfile.close()
os._exit(0) # I know it looks ugly, but this is the easiest way to kill all the threads

