#!/bin/bash
if [[ "$0" != "igawk" ]] ; then exec igawk --file <(sed '2s/./#/' "$0") "$@"; fi; exit
BEGIN {
  FS=","
  HUMAN_READABLE = 0

@include comments.awk

  no_data1 = 0
  no_data2 = 0
}

/Trx No/ {
  trx_no++;
}

/RESPONSE1 returned no data/ {
  no_data1++;
}

/RESPONSE2 returned no data/ {
  no_data2++;
}

/^ / {
  gsub(/ /, "", $0)
  sum[$1] += $4
  count[$1]++
  if ($4 == 0)
    count_0[$1]++
  else if ($4 < 1)
    count_1[$1]++
  else if ($4 < 10)
    count_10[$1]++
  else if ($4 < 100)
    count_99[$1]++
  else if ($4 == 100)
    count_100[$1]++
}

END {
  for (i in sum)  {
    sum_all += sum[i]/count[i]
    count_all++
    weight_sum_all += sum[i]
    weight_count_all += count[i]
    if (sum[i]/count[i] == 0)
    {
      taxes_0++
    }
    else if (sum[i]/count[i] < 1)
    {
      taxes_1++
    }
    else if (sum[i]/count[i] < 10)
    {
      taxes_10++
    }
    else if (sum[i]/count[i] < 100)
    {
      taxes_99++
    }
    else
    {
      taxes_100++
    }
  }

  print "Number of requests: "trx_no
  print "Number of taxes: "count_all

  if (HUMAN_READABLE == 1)
  {
    printf("Taxes calculated correctly: %d/%d = %f%\n", taxes_0, count_all, taxes_0/count_all*100)
    print ""
    
    sort = "sort -n -k2"
    printf("%5s %11s   %9s\n", "Tax", "Correctly", "Comment")
    for (i in sum)  {
      printf ("%4s:%11f%%,   %9s\n", i, count_0[i]/count[i]*100, comment[i]) | sort
    }
    close(sort)
  }
  else
  {
    print ""
    print "Average difference: "sum_all/count_all"%"
    print "Weighted average difference: "weight_sum_all/weight_count_all"%"
    print ""
    print "0% difference: "taxes_0/count_all*100"%"
    print "<0,1)% difference: "taxes_1/count_all*100"%"
    print "<1,10)% difference: "taxes_10/count_all*100"%"
    print "<10,100)% difference: "taxes_99/count_all*100"%"
    print "100% difference: "taxes_100/count_all*100"%"
    print ""
    print "No response from server 1: "no_data1
    print "No response from server 2: "no_data2
    print ""

    sort = "sort -t, -r -n"
    printf("%7s,%4s,%5s,%15s,%19s,%17s,%17s,%17s,   %9s\n", "Rank", "Tax", "Count", "No discrepancy", "Minimal discrepancy", "Small discrepancy", "Big discrepancy", "Full discrepancy", "Comment")
    for (i in sum)  {
      printf ("%7.2f,%4s,%5d,%15d,%19d,%17d,%17d,%17d,   %9s\n", sum[i]/100.0, i, count[i], count_0[i], count_1[i], count_10[i], count_99[i], count_100[i], comment[i]) | sort
    }
    close(sort)
  }
}
