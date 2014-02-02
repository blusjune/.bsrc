#!/bin/bash
## bsc.x9.bmt.fio.run_fio.sh
## _ver=20131217_161045
## FIO benchmark execution script

FIO=/usr/local/bin/fio




#DEV=/dev/nvme0n1
#DEV=/dev/dm-17 # DEV=/dev/nvme-vg/nvme-vol-01
if [ "X$DEV" = "X" ]; then
	read -p "#>> {DEV} Target device path (e.g., '/dev/nvme0n1' OR '/dev/dm-17' OR ...): " DEV;
	if [ "X$DEV" = "X" ]; then
		echo "#>> ERROR 'DEV' variable should be specified -- EXIT 1";
		exit 1;
	fi
fi




if [ "X$FIO_OPER" = "X" ]; then
	echo "#>> FIO Operation 'ss': steady-state making";
	echo "#>> FIO Operation 'pt': performance test (default)";
	echo "#>> FIO Operation 'na': no action";
	read -p "#>> {FIO_OPER} Specify FIO Operation ['ss'|'pt'|'na']: "  FIO_OPER;
	if [ "X$FIO_OPER" = "X" ]; then
		FIO_OPER='pt';
	fi
fi




DRIVEBASE=$(basename $DEV)
DRIVESIZEBYTES=$(($(cat /sys/block/$DRIVEBASE/size) *512))
DRIVESIZEGB=$(($DRIVESIZEBYTES/1000000000))
DRIVESIZEGiB=$(($DRIVESIZEBYTES/1024/1024/1024))

echo "DEV: $DEV"
echo "DRIVESIZEGiB: $DRIVESIZEGiB"



# === SUBROUTINES =========

#-------------------
# Percent of advertised capacity used for the test. If running test with several percentage points put them in increasing order. This setting must preceed sequential filling and random write conditioning.
# IMPORTANT NOTE! For USEDCAPACITYPERCENT < 100 the SSD must be reset/erased first using vendor specific tools before running this script.
USEDCAPACITYPERCENT=100 
TESTCAPACITY=$(($DRIVESIZEGiB*$USEDCAPACITYPERCENT/100))  # Do not change this line
echo "TESTCAPACITY: $TESTCAPACITY"
#-------------------


RandomWrite1Hours_blusjune()
{
  echo "Steady-state/Phase2 Start: Random write 1 hour."
  
  $FIO --output=${DRIVESIZEGB}GB_RandomWriteOneHours --name=RandomWriteOneHours --filename=$DEV --ioengine=posixaio --direct=1 --norandommap --randrepeat=0 --refill_buffers --time_based --runtime=1h --blocksize=4k --rw=randwrite --iodepth=256 --overwrite=1 --write_iops_log=RandomWriteOneHours --log_avg_msec=1000
  CheckError
  
  echo "Steady-state/Phase2 Finished: Random write 1 hour."
}


RandomWrite8Hours_blusjune()
{
  echo "Steady-state/Phase2 Start: Random write 8 hours."
  
  $FIO --output=${DRIVESIZEGB}GB_RandomWriteOneHours --name=RandomWriteOneHours --filename=$DEV --ioengine=posixaio --direct=1 --norandommap --randrepeat=0 --refill_buffers --time_based --runtime=8h --blocksize=4k --rw=randwrite --iodepth=256 --overwrite=1 --write_iops_log=RandomWriteOneHours --log_avg_msec=1000
  CheckError
  
  echo "Steady-state/Phase2 Finished: Random write 8 hours."
}


RandomWriteEightHours()
{
  echo Start Random write eight hours.     
  
  $FIO --output=${DRIVESIZEGB}GB_RandomWriteEightHours --name=RandomWriteEightHours --filename=$DEV --ioengine=libaio --direct=1 --norandommap --randrepeat=0 --refill_buffers --time_based --runtime=8h --blocksize=4k --rw=randwrite --iodepth=256 --overwrite=1 --write_iops_log=RandomWriteEightHours --log_avg_msec=1000
  CheckError
  
  echo Finished random write conditioning.
}


FillSequentiallyOneTime()
{
  echo -e '\nFilling the drive sequentially to make sure we do not have any empty sectors and the reserved capacity is filled too. Using '$USEDCAPACITYPERCENT'% of the drive capacity.'

 $FIO --output=FillSequentiallyOneTime --name=FillSequentiallyOneTime --readwrite=write --bs=128k --ioengine=libaio --iodepth=256 --direct=1 --filename=$DEV --size="$TESTCAPACITY"G --write_iops_log=FillSequentiallyOneTime --log_avg_msec=1000
  CheckError

  echo Finished filling sequentially.
}


FillSequentiallyOneTime_blusjune()
{
  echo -e '\nFilling the drive sequentially to make sure we do not have any empty sectors and the reserved capacity is filled too. Using '$USEDCAPACITYPERCENT'% of the drive capacity.'

 $FIO --output=FillSequentiallyOneTime --name=FillSequentiallyOneTime --readwrite=write --bs=128k --ioengine=posixaio --iodepth=256 --direct=1 --filename=$DEV --size="$TESTCAPACITY"G --write_iops_log=FillSequentiallyOneTime --log_avg_msec=1000
  CheckError

  echo Finished filling sequentially.
}


CheckError()
{
  if [ $? != 0 ]; then
    echo ERROR
    exit 1;
  fi
}


BasicPerformance_orig()
{
  local TestCaseName=$1
  local Testcase=${DRIVESIZEGB}GB_${TestCaseName}

  $FIO --output=$Testcase --name=$Testcase --filename=$DEV --ioengine=libaio --direct=1 --norandommap --randrepeat=0 --refill_buffers --time_based --ramp_time=30s --runtime=30s --blocksize=$4 --rw=$2 --rwmixwrite=$3 --iodepth=$6 --overwrite=1 --numjobs=$5 --group_reporting

  echo "$TestCaseName Done".
}


BasicPerformance() ## blusjune
{
  local TestCaseName=$1
  local Testcase=${DRIVESIZEGB}GB_${TestCaseName}

  $FIO --output=$Testcase --name=$Testcase --filename=$DEV --ioengine=posixaio --direct=1 --norandommap --randrepeat=0 --refill_buffers --time_based --ramp_time=30s --runtime=30s --blocksize=$4 --rw=$2 --rwmixwrite=$3 --iodepth=$6 --overwrite=1 --numjobs=$5 --group_reporting

  echo "$TestCaseName Done".
}


BasicPerformance_blusjune() ## blusjune
{
  local TestCaseName=$1
  local Testcase=${DRIVESIZEGB}GB_${TestCaseName}

  $FIO --output=$Testcase --name=$Testcase --filename=$DEV --ioengine=posixaio --direct=1 --norandommap --randrepeat=0 --refill_buffers --time_based --ramp_time=30s --runtime=30s --blocksize=$4 --rw=$2 --rwmixwrite=$3 --iodepth=$6 --overwrite=1 --numjobs=$5 --group_reporting

  echo "$TestCaseName Done".
}


DellPerformance()
{
  local TestCaseName=$1
  local Testcase=${DRIVESIZEGB}GB_${TestCaseName}

  $FIO --output=$Testcase --name=$Testcase --filename=$DEV --ioengine=libaio --direct=1 --norandommap --randrepeat=0 --refill_buffers --time_based --ramp_time=30s --runtime=30s --blocksize=$4 --blockalign=$7 --rw=$2 --rwmixwrite=$3 --iodepth=$6 --overwrite=1 --numjobs=$5 --group_reporting

  echo "$TestCaseName Done".
}

LongPerformance()
{
  local HourIndex=$7
  local TestCaseName=$1_${HourIndex}h
  local Testcase=${DRIVESIZEGB}GB_${TestCaseName}

  $FIO --output=$Testcase --name=$Testcase --filename=$DEV --ioengine=libaio --direct=1 --norandommap --randrepeat=0 --refill_buffers --time_based --ramp_time=30s --runtime=1h --blocksize=$4 --rw=$2 --rwmixwrite=$3 --iodepth=$6 --overwrite=1 --numjobs=$5 --group_reporting --write_iops_log=$Testcase --log_avg_msec=1000

  echo "$TestCaseName Done".
}

SequentialBandwidth()
{
  local bs="128k"
  local qd="64"

  #BasicPerformance TestCaseName --rw --rwmixwrite --blocksize --numjobs --iodepth 
  BasicPerformance "SequentialBandwidth_Read${bs}B" "rw" "0" "$bs" "1" "$qd"
  BasicPerformance "SequentialBandwidth_Write${bs}B" "rw" "100" "$bs" "1" "$qd"
}

SequentialLatency()
{
  local RequestSize="512 4k"

  for bs in $RequestSize
  do 
    #BasicPerformance TestCaseName --rw --rwmixwrite --blocksize --numjobs --iodepth 
    BasicPerformance "SequentialLatency_Read${bs}B" "rw" "0" "$bs" "1" "1"
    BasicPerformance "SequentialLatency_Write${bs}B" "rw" "100" "$bs" "1" "1"
  done
}

RandomIOPS()
{
  local bs="4k"
  local job="8"
  local qd="32"

  #BasicPerformance TestCaseName --rw --rwmixwrite --blocksize --numjobs --iodepth 
  BasicPerformance "RandomIOPS_Read${bs}B" "randrw" "0" "4k" "$job" "$qd"
  BasicPerformance "RandomIOPS_Write${bs}B" "randrw" "100" "4k" "$job" "$qd"
  BasicPerformance "RandomIOPS_OLTP${bs}B" "randrw" "30" "4k" "$job" "$qd"
}

RandomLatency()
{
  local RequestSize="512 4k"

  for bs in $RequestSize
  do 
    #BasicPerformance TestCaseName --rw --rwmixwrite --blocksize --numjobs --iodepth 
    BasicPerformance "RandomLatency_Read${bs}B" "randrw" "0" "$bs" "1" "1"
    BasicPerformance "RandomLatency_Write${bs}B" "randrw" "100" "$bs" "1" "1"
  done
}


Run()
{
  local SeqOrRand=$1
  local Job=$2
  local BlockAlign=$3
  
  local RdOrWr="0 100" 
  local QueueDepth="1 2 4 8 16 32 64"
  local RequestSize="512 4k 8k 16k 32k 64k 128k 256k 512k 1024k"

  if [ ${SeqOrRand} = "Sequential" ]; then 
    RequestPattern=rw
  else
    RequestPattern=randrw
  fi

  for rdwr in $RdOrWr
  do
  if [ ${rdwr} = "0" ]; then 
    RequestType=Read
  else
    RequestType=Write
  fi
    for bs in $RequestSize
    do
      for qd in $QueueDepth
      do
        local TestCaseName=${SeqOrRand}Run${BlockAlign}A_${RequestType}${bs}B_${Job}Thrx${qd}QD
        #BasicPerformance TestCaseName --rw --rwmixwrite --blocksize --numjobs --iodepth --blockalign
        DellPerformance "$TestCaseName" "$RequestPattern" "$rdwr" "$bs" "$Job" "$qd" "$BlockAlign"
      done
    done
  done
}

SequentialRun4kA()
{
  Run "Sequential" "$1" "4k"
}

SequentialRun512bA()
{
  Run "Sequential" "$1" "512b" 
}


RandomRun4kA()
{
  #Run "Random" "$1" "4k"
  RandomRunOLTP "$1" "4k"
}


RandomRun4kA_blusjune()
{
  #Run "Random" "$1" "4k"
  RandomRunOLTP_blusjune "$1" "4k"
}


RandomRun512bA()
{
  Run "Random" "$1" "512b"
  RandomRunOLTP "$1" "512b"
}

RandomRunOLTP()
{
  local RequestSize="4k 8k"
  local QueueDepth="1 2 4 8 16 32 64"
  local Job=$1
  local BlockAlign=$2

  for bs in $RequestSize
  do 
    for qd in $QueueDepth
    do
      #BasicPerformance TestCaseName --rw --rwmixwrite --blocksize --numjobs --iodepth 
      BasicPerformance "RandomRun${BlockAlign}A_OLTP${bs}B_${Job}Thrx${qd}QD" "randrw" "30" "$bs" "$Job" "$qd"
    done
  done
}


RandomRunOLTP_blusjune()
{
#  local RequestSize="4k 8k"
  local RequestSize="1k 4k 8k 16k 128k 4096k"
  local QueueDepth="1 2 4 8 16 32 64 128 256"
  local Job=$1
  local BlockAlign=$2

  for bs in $RequestSize
  do 
    for qd in $QueueDepth
    do
      #BasicPerformance TestCaseName --rw --rwmixwrite --blocksize --numjobs --iodepth 
      BasicPerformance_blusjune "RandomRun${BlockAlign}A_OLTP${bs}B_${Job}Thrx${qd}QD" "randrw" "30" "$bs" "$Job" "$qd"
    done
  done
}


SequentialRunLatency()
{
  Latency "SequentialRunLatency" "Read" "rw" "0"
  Latency "SequentialRunLatency" "Write" "rw" "100"
}

RandomRunLatency()
{
  Latency "RandomRunLatency" "Read" "randrw" "0"
  Latency "RandomRunLatency" "Write" "randrw" "100"
  Latency "RandomRunLatency" "OLTP" "randrw" "30"
}

Latency()
{
  local BlockAlign="512 4k"

  if [ $2 != "OLTP" ]; then 
    RequestSize="512 4k"
  else
    RequestSize="4k 8k"
  fi

  for ba in $BlockAlign
  do
    local TestCaseName="$1${ba}A_$2"
    for bs in $RequestSize
    do
      #BasicPerformance TestCaseName --rw --rwmixwrite --blocksize --numjobs --iodepth
      BasicPerformance "${TestCaseName}${bs}B" "$3" "$4" "$bs" "1" "1"
    done
  
  done 
}

QualityOfService()
{
  local job=$1
  local qd=$2
  local bs="4k"

  #BasicPerformance TestCaseName --rw --rwmixwrite --blocksize --numjobs --iodepth 
  BasicPerformance "QualityOfService_Read${bs}B_${job}Thrx${qd}QD" "randrw" "0" "$bs" "$job" "$qd"
  BasicPerformance "QualityOfService_Write${bs}B_${job}Thrx${qd}QD" "randrw" "100" "$bs" "$job" "$qd"
  BasicPerformance "QualityOfService_OLTP${bs}B_${job}Thrx${qd}QD" "randrw" "30" "$bs" "$job" "$qd"
}

PerformanceConsistency()
{
  local Job="8"
  local QueueDepth="32"

  i=1
  while [ $i -le $1 ]
  do
    #LongPerformance TestCaseName --rw --rwmixwrite --blocksize --numjobs --iodepth hourIndex
    LongPerformance "PerformanceConsistency_Read4kB" "randrw" "0" "4k" "$Job" "$QueueDepth" "$i"
    LongPerformance "PerformanceConsistency_Write4kB" "randrw" "100" "4k" "$Job" "$QueueDepth" "$i"
    i=`expr $i + 1`
  done
}








####
#### blusjune's test main ####
####

fio_exec_ss()
{
	FillSequentiallyOneTime_blusjune;
	RandomWrite1Hours_blusjune;
	#RandomWrite8Hours_blusjune;
}

fio_exec_pt()
{
	RandomRun4kA_blusjune "1";	# 4-threads (!!-Dell Requirement-!! need long time)
	#RandomRun4kA_blusjune "4";	# 4-threads (!!-Dell Requirement-!! need long time)
}

case $FIO_OPER in
'ss')
	fio_exec_ss;
	;;
'pt')
	fio_exec_pt;
	;;
'sspt')
	fio_exec_ss;
	fio_exec_pt;
	;;
'na')
	echo "#>> Nothing happened -- EXIT";
	;;
*)
	echo "#>> Undefined operation -- EXIT";
	exit 1;
	;;
esac
exit 0;










## === Test Cases =========

### Preconditioning (Phase 1 : Filling All the Logical Blocks with Sequential Write)
FillSequentiallyOneTime
###

## Sequential Performance 
SequentialLatency		# Basic Requirement
SequentialBandwidth		# Basic Requirement

SequentialRunLatency		# (!!-Dell Requirement-!!)
SequentialRun4kA "4"		# 4-threads (!!-Dell Requirement-!! need long time)
SequentialRun512bA "4"		# 4-threads (!!-Dell Requirement-!! need long time)
###

### Preconditioning (Phase 2 : Making the Drive to be Sustained by Writing with Random Pattern)
RandomWriteEightHours
###

## Random Performance
RandomLatency			# Basic Requirement
RandomIOPS 			# Basic Requirement

RandomRunLatency		# (!!-Dell Requirement-!!)
RandomRun4kA "4"		# 4-threads (!!-Dell Requirement-!! need long time)
RandomRun512bA "4"		# 4-threads (!!-Dell Requirement-!! need long time)
###

## Random Maximum Performance 
RandomRun4kA "8"		# 8-threads
RandomRun512bA "8"		# 8-threads
###

## Quality of Service
QualityOfService "1" "1"	# 1-thread X QD1 = Total 1
QualityOfService "4" "8"	# 4-threads X QD8 = Total 32
QualityOfService "8" "32"	# 8-threads X QD32 = Total 256
###

## Performance Consistency
PerformanceConsistency "10"	# 10-hours (need long time)
###
