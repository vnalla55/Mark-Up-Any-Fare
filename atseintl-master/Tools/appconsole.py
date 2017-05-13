import socket
import select
from xml.dom import minidom
import struct
import argparse
from operator import attrgetter
import signal
import time
import re

class LatencyData():
    def __init__(self, element):
        self.name = element.attributes['NAM'].value
        self.elapsed_time = float(element.attributes['ELP'].value)
        self.user_cpu_time = float(element.attributes['USR'].value)
        self.system_cpu_time = float(element.attributes['SYS'].value)
        self.total_cpu_time = self.user_cpu_time + self.system_cpu_time
        
class AppConsoleConnection():
    def __init__(self, options):
        self.__options = options
        self.__address = (options.host, options.port)
    
    def __make_xml2_header(self, command, length):
        return struct.pack('!ll4s8s', 16, length, command, '00010000')
    
    def __request(self, command):
        response = ""
        
        try:
            s = socket.socket()
            s.connect(self.__address)
            s.send(self.__make_xml2_header(command, 0))
            ready = select.select([s],[],[], 5)
   
            if ready[0]:
                buf = s.recv(20)
                
                if len(buf) == 20:
                    header = struct.unpack('!ll4s8s', buf)
                    
                    payload_len = header[1]
                    bytes_recd = 0
                    
                    while bytes_recd < payload_len:
                        chunk = s.recv(min(payload_len - bytes_recd, 2048))
                        response = response + chunk
                        bytes_recd = bytes_recd + len(chunk)
                    response = response[0:payload_len - 1]
                        
        except (socket.error, select.error) as e:
            print str(self), str(e)
            exit -1
        finally:
            try:
                s.shutdown(socket.SHUT_RDWR)
                s.close()
            except socket.error:
                pass
        
        if not response:
            raise Exception("Empty response received")
         
        return response
    
    def custom_command(self, command):
        if (len(command) != 4):
            raise Exception('Invalid custom command')                    

        return self.__request(command)
        
    def reset(self):
        self.__request('CNRT')
        
    def get_stats(self):
        response = self.__request('RFSH')
        return minidom.parseString(response)

    def update_config(self):
        return self.__request('DCFG')
        
    def get_services_latency(self):
        response = self.__request('SVCL')        
        return minidom.parseString(response)
    
    def get_details(self):
        return self.__request('ETAI')
    
    def shutdown(self):
        return self.__request('DOWN')
    
    def get_error_counts(self):
        return self.__request('ERCT')
    
    def get_config(self):
        return self.__request('CNFG')
    
def parse_arguments():
    global version_string
    
    parser = argparse.ArgumentParser()
    commands = ['updatecfg', 'reset', 'stats', 'services', 'details', 'errors', 'config', 'shutdown']
    parser.add_argument('host', metavar="HOST", help="hostname of server")
    parser.add_argument('port', metavar="PORT", type=int, help="console port of server")
    parser.add_argument('command', 
                        metavar="COMMAND", help="application console command ("+", ".join(commands) + " or CUSTOM)")    
    parser.add_argument('-i', '--interval', default=0, type=int, dest="interval", help='time interval in seconds')
    parser.add_argument('-e', '--error-description-file', dest='error_descs_path', default='error_descriptions.ini', help='path to file with error descriptions')    
    parser.add_argument('-v', '--verbose', default=False, action="store_true", help="display XML response")
    
    return parser.parse_args()

def parse_latency_elements(latency_elements):
    result = []
    
    for latency_element in latency_elements:
        result.append(LatencyData(latency_element))    
    return result
            
def format_string_field(string, width):
    return string.ljust(width)

def format_number_field(number, width):
    return "{0:.6f}".format(number).rjust(width);

def find_max_name_len(latencies):
    max_len = 0;
    
    for latency in latencies:
        name_len = len(latency.name)
        
        if name_len > max_len:
            max_len = name_len
    return max_len

def print_latency_table(latencies, trx_count, totals = True, sort_by = 'total_cpu_time', asc_order = True):
    total_cpu = 0
    total_elapsed = 0
    max_name_len = find_max_name_len(latencies)
    sorted_latencies = sorted(latencies, key=attrgetter(sort_by), reverse=asc_order)
    
    print '{0} {1} {2} {3} {4}'.format('Name'.ljust(max_name_len), 
                               'Elapsed'.rjust(13),
                               'CPU'.rjust(13),
                               'Avg elapsed'.rjust(13),
                               'Avg CPU'.rjust(13))
            
    for latency in sorted_latencies:
        total_elapsed += latency.elapsed_time
        total_cpu += latency.total_cpu_time
 
        print '{0} {1} {2} {3} {4}'.format(format_string_field(latency.name, max_name_len), 
                                   format_number_field(latency.elapsed_time, 13),
                                   format_number_field(latency.total_cpu_time, 13),
                                   format_number_field(latency.elapsed_time / trx_count, 13),
                                   format_number_field(latency.total_cpu_time / trx_count, 13))
    if totals:
        print '\n{0} {1} {2} {3} {4}'.format(format_string_field('TOTAL', max_name_len), 
                                   format_number_field(total_elapsed, 13),
                                   format_number_field(total_cpu, 13),
                                   format_number_field(total_elapsed / trx_count, 13),
                                   format_number_field(total_cpu / trx_count, 13))

def display_service_latency(options, dom):
    service_latencies = parse_latency_elements(dom.getElementsByTagName('SVC'))
        
    if options.verbose:
        print dom.toprettyxml()
    
    root = dom.getElementsByTagName('SLC')[0]
    trx_count = int(root.attributes['TOK'].value) + int(root.attributes['TER'].value)
    print 'Services metrics\nTotal number of transactions: {0}\n'.format(trx_count) 
    print_latency_table(service_latencies, trx_count)

def display_stats(options, dom):
    if options.verbose:
        print dom.toprettyxml()
    
    total_elapsed_time = 0
    total_ok_requests = 0
    total_err_requests = 0
    
    for service_stat_element in dom.getElementsByTagName('SP'):
        if service_stat_element.attributes['NM'].value == 'TRX':
            total_ok_requests += int(service_stat_element.attributes['OK'].value)
            total_err_requests += int(service_stat_element.attributes['ER'].value)
            total_elapsed_time += float(service_stat_element.attributes['ET'].value)
            break
            
    root_element = dom.getElementsByTagName('STATS')[0]

    print 'Load average: {0}, {1}, {2}'.format(format_number_field(float(root_element.attributes['L1'].value), 6),
                                               format_number_field(float(root_element.attributes['L2'].value), 6),
                                               format_number_field(float(root_element.attributes['L3'].value), 6))

    total_request_count = total_ok_requests + total_err_requests
    
    print 'Total transaction count: {0} ({1}/{2})'.format(total_request_count, total_ok_requests, total_err_requests)
    print 'Total CPU time: {0}'.format(format_number_field(float(root_element.attributes['CP'].value), 6))
    print 'Total elapsed time: {0}'.format(total_elapsed_time)
    
    if total_request_count > 0:
      print 'Average CPU time: {0}'.format(format_number_field(float(root_element.attributes['CP'].value) / total_request_count, 6))
      print 'Average elapsed time: {0}'.format(total_elapsed_time / total_request_count)
    
    print 'Resident memory: {0}'.format(root_element.attributes['RS'].value) + ' bytes'
    print 'Virtual memory: {0}'.format(root_element.attributes['VM'].value) + ' bytes'

def display_error_counts(data):
    global error_description_map
    
    chunks = data.split('|')
    chunks.pop()
    
    if len(chunks) > 0:
        print 'Id    Count    Description'
        error_counts = dict(chunks[i:i + 2] for i in range(0, len(chunks), 2))
        
        for error_id, count in error_counts.items():
            
            print '{0: <4}  {1: <7}  {2}'.format(error_id, count, error_description_map.get(int(error_id), 'N/A'))           
    else:
        print 'No data'

def load_error_descriptions(error_descs_path):
    global error_description_map
    
    pattern = re.compile('\s*(.+?)\s*=\s*([0-9]+)')
    
    with open(error_descs_path, 'r') as file:
        for line in file:
            match = pattern.search(line)
            
            if match:
                try:
                    error_description_map[int(match.group(2))] = match.group(1)
                except ValueError:
                    pass
                
def signal_handler(signal, frame):
    print('Stopping by user request')
    exit(0)
    
def main():
    global options;
    global error_description_map
    
    error_description_map = dict()
    
    signal.signal(signal.SIGINT, signal_handler)
    options = parse_arguments()
    
    if options.error_descs_path:
        load_error_descriptions(options.error_descs_path)
        
    try:
        cnx = AppConsoleConnection(options)
        
        while True:
            if options.command == 'reset':
                print 'Reseting server statistics...'
                cnx.reset()
            elif options.command == 'stats':
                print 'Requesting server statistics...'  
                result = cnx.get_stats()
                display_stats(options, result)
            elif options.command == 'updatecfg':
                print 'Requesting config update...'
                print cnx.update_config()
            elif options.command == 'services':
                print 'Requesting services latency data...'
                dom = cnx.get_services_latency()
                display_service_latency(options, dom)
            elif options.command == 'details':
                print 'Requesting server details...'
                print cnx.get_details()
            elif options.command == 'errors':
                print 'Requesting server error counts...'
                result = cnx.get_error_counts()
                display_error_counts(result)
            elif options.command == 'config':
                print cnx.get_config()
            elif options.command == 'shutdown':
                print 'Requesting server shutdown...'
                cnx.shutdown()
            else:
                custom_command = options.command.upper();
                
                print 'Requesting custom command: ' + custom_command + '...'
                print cnx.custom_command(custom_command)
                
            if options.interval > 0:
              time.sleep(options.interval)
              print ''
            else:
              break
                
    except Exception as e:
        print e;
    finally:
        print 'Done.\n'
        
if __name__ == '__main__':
    main()
