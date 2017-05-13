#!/bin/awk --posix

BEGIN {
   req = "";
   pos = 0;
   if (NUMREQS == "") NUMREQS = 50;
   if (NUMREQS == "ALL") NUMREQS = 1000000000;
   if (NUMREQS != 0 + NUMREQS)
   {
      print "The NUMREQS variable, if specified, has to be either numeric or 'ALL'.";
      exit 1;
   }
   inReq = 0;
   count = 0;
   if (stamp == 0)
   {
      print "This script requires the stamp variable to be passed in the command line.";
      exit 1;
   }
   border=strftime("%Y-%m-%d %H:%M:%S", stamp+1);
   print "Searching for requests logged just before " border;
}

/^20[0-9][0-9]-[0-1][0-9]-[0-3][0-9] [0-2][0-9]:[0-5][0-9]:[0-5][0-9],[0-9][0-9][0-9]: / {
   if ( $0 > border ) {
      inReq = 0;
      next;
   }
   if (req != "")
   {
      count++;
      requests[pos] = req;
      stamps[pos] = timestamp;
      pos++;
      if (pos >= NUMREQS) {
         pos = 0;
      }
   }

   if (index ($0, "<") > 0)
      req = substr ($0, index ($0, "<"));
   else
      req = "";

   timespec = substr ($0, 1, 19);
   gsub ( /[:-]/, " ", timespec);
   timestamp = mktime (timespec);
   inReq = 1;
   next;
}

/}}}/ {
   inReq = 0;
   next;
}

{
   if (inReq) {
      if (req != "")
         req = req "\n";
      req = req $0;
   }
}

END {
   if (req != "")
   {
      count++;
      requests[pos] = req;
      stamps[pos] = timestamp;
      pos++;
      if (pos >= NUMREQS) {
         pos = 0;
      }
   }
   newest = 0;
   oldest = stamp + 100;
   saved = (count > NUMREQS) ? NUMREQS : count;
   for (ofs = 0; ofs < NUMREQS; ofs++)
   {
      if (count < NUMREQS)
         curr = count - ofs + 1;
      else
         curr = NUMREQS - ((pos + ofs) % NUMREQS);
      if (requests[ofs] == "")
         break;
      if (stamps[ofs] > newest)
         newest = stamps[ofs];
      if (stamps[ofs] < oldest)
         oldest = stamps[ofs];
      file = "log." curr ".req";
      print requests[ofs] > file;
   }
   if (oldest > newest)
      print "No requests recovered from the log files.";
   else
      print saved " requests extracted from the log files, covering interval " strftime("%Y-%m-%d %H:%M:%S", oldest) " to " strftime("%Y-%m-%d %H:%M:%S", newest);
}
