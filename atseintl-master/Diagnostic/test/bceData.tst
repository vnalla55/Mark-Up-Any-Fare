[MAIN]


[SEQUENCE1]
itemNo=3456
primeInd=X
tableType=W
seqNo=1000
constructSpecified=S
applInd=M
ifTag=.
segCnt=2


[SEGMENT1-1]
segNo=1
viaCarrier=X$
primarySecondary=P
fltRangeAppl=R
flight1=0
flight2=0
equipType=737
tvlPortion=X
tsi=0
directionInd=F
loc1Type=C
loc1=LAX
loc2Type=C
loc2=DFW
posTsi=22
posLocType=C
posLoc=WAS
soldInOut=I
startDate=1
stopDate=2
daysOfWeek=2345
startTime=1
stopTime=22
fareclassType=F
fareclass=Y
restriction=P
bkcode1=Q
bkcode2=L

[SEGMENT1-2]
segNo=2
viaCarrier=X$
primarySecondary=P
fltRangeAppl=R
flight1=0
flight2=0
equipType=737
tvlPortion=X
tsi=0
directionInd=F
loc1Type=C
loc1=LAX
loc2Type=C
loc2=DFW
posTsi=22
posLocType=C
posLoc=WAS
soldInOut=I
startDate=1
stopDate=2
daysOfWeek=2345
startTime=1
stopTime=22
fareclassType=F
fareclass=Y
restriction=P
bkcode1=Q
bkcode2=L


[SEQUENCE2]
itemNo=3456
primeInd=X
tableType=Y
seqNo=2000
constructSpecified=X
applInd=P
ifTag=2
segCnt=1


[SEGMENT2-1]
segNo=1
viaCarrier=$$
primarySecondary=P
fltRangeAppl=I
flight1=987
flight2=1007
equipType=.
tvlPortion=X
tsi=0
directionInd=F
loc1Type=C
loc1=SAN
loc2Type=C
loc2=ORD

[SEQUENCE3]
itemNo=3456
primeInd=X
tableType=Y
seqNo=3000
constructSpecified=X
applInd=P
ifTag=2
segCnt=1


[SEGMENT3-1]
segNo=1
viaCarrier=AA
primarySecondary=P
fltRangeAppl=R
flight1=987
flight2=999
equipType=747
tvlPortion=X
tsi=0
directionInd=F
loc1Type=C
loc1=SAN
loc2Type=C
loc2=ORD


[SEQUENCE4]
itemNo=3456
primeInd=X
tableType=Y
seqNo=3000
constructSpecified=X
applInd=P
ifTag=2
segCnt=1


[SEGMENT4-1]
segNo=1
viaCarrier=AA
primarySecondary=P
fltRangeAppl=R
flight1=987
flight2=999
equipType=747
tvlPortion=X
tsi=0
directionInd=F
loc1Type=0
loc1=SAN
loc2Type=0
loc2=ORD
posLocType=0


[FARE1]
carrier=LH
fareclass=Y1
globalDir=1


[AIRSEG1-1]
segmentNum=1
originType=C
origin=CHI
destType=C
dest=NYC
tvlSegType=2
geoTvlType=3
equipType=737
mktgCarrier=DL
flightNum=4033
stopType=0
result=MATCH


[AIRSEG1-2]
segmentNum=2
originType=C
origin=NYC
destType=C
dest=FRA
tvlSegType=1
geoTvlType=2
equipType=767
mktgCarrier=LH
flightNum=988
stopType=1
result=NOMATCH

[AIRSEG1-3]
segmentNum=3
originType=C
origin=FRA
destType=C
dest=STO
tvlSegType=1
geoTvlType=2
equipType=767
mktgCarrier=SK
flightNum=987
stopType=1
result=MATCH

[RESULTGROUP]
------------------- BookingCodeException --------------------
validate:ItemNumber 3456 Flight 789 
 Prime is X: Carrier correctly does not match:
  FlightCarrier = DL bookingCodeCarrier = AA 
  Continuing
validateSegment: Item 3456 sequence 1000 SegNum = 1 Flight 789 
BookingCodeException::validateCarrier: FAILED Sequence 1000 Segment 1
  BCESegmentCarrier= UA FareCarrier= LH 

validateSegment: Item 3456 sequence 2000 SegNum = 1 Flight 789 
BookingCodeException::validateCarrier: Does Not Pass - SKIP to next segment 
  BCESegmentCarrier= SK fltCarrier= DL 
BookingCodeException::validateSegment: Flight 789 FAILED for  sequence 2000 

validateSegment: Item 3456 sequence 3000 SegNum = 1 Flight 789 
BookingCodeException::validateCarrier: Does Not Pass - SKIP to next segment 
  BCESegmentCarrier= AA fltCarrier= DL 
BookingCodeException::validateSegment: Flight 789 FAILED for  sequence 3000 


*****
BookingCodeException:: Flight 789 FAILED for  itemNumber  3456" 
[END]


[RESULT1-2]
result="This is my Result2."

[RESULT1-3]
result="This is my Result3."



