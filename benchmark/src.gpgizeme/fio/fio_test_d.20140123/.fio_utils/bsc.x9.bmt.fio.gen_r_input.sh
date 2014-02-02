#!/bin/sh
## bsc.x9.bmt.fio.gen_r_input.sh
## _ver=20140123_084028
## _ver=20140105_172240
## _ver=20131217_020035
## _ver=20131217_172741
##
## Script to parse the FIO result data




#_fio__size_gb="";
#_fio__thrx="";
#_fio__iosz="";
#_fio__iod="";




# fname example: "399GB_RandomRun4kA_OLTP4kB_4Thrx256QD"
if [ "X$_fio__size_gb" = "X" ]; then
	if [ "X$(ls -1 *GB* | sed -e 's/\([0-9]\)GB_.*/\1/g' | sort -u | wc -l)" = "X1" ]; then
		_size_gb_maybe=$(ls -1 *GB* | sed -e 's/\([0-9]\)GB_.*/\1/g' | sort -u);
	else
		_size_gb_maybe="__";
	fi
	read -p "#<< target storage device size in GB [$_size_gb_maybe]: " _fio__size_gb;
	if [ "X$_fio__size_gb" = "X" ]; then
		_fio__size_gb=$_size_gb_maybe;
	fi
fi
if [ "X$_fio__thrx" = "X" ]; then
	_fio__thrx__default="1 4 16";
	read -p "#<< number of threads [$_fio__thrx__default]: " _fio__thrx;
	if [ "X$_fio__thrx" = "X" ]; then
		_fio__thrx=$_fio__thrx__default;
	fi
fi
if [ "X$_fio__iosz" = "X" ]; then
	_fio__iosz__default="1 4 8 16 64 128 4096";
	read -p "#<< list of io size in kB [$_fio__iosz__default]: " _fio__iosz;
	if [ "X$_fio__iosz" = "X" ]; then
		_fio__iosz="$_fio__iosz__default";
	fi
fi
if [ "X$_fio__iod" = "X" ]; then
	_fio__iod__default="1 2 4 8 16 32 64 128 256";
	read -p "#<< list of io depth [$_fio__iod__default]: " _fio__iod;
	if [ "X$_fio__iod" = "X" ]; then
		_fio__iod="$_fio__iod__default";
	fi
fi

#_fname_prefix="${_fio__size_gb}GB_RandomRun4kA_OLTP4kB_4Thrx";
#_fname_prefix="${_fio__size_gb}GB_RandomRun4kA_OLTP4kB_1Thrx";




for _thrx in $_fio__thrx; do #{
	for _iosz in $_fio__iosz; do #{
		echo ""
		echo ""
		echo ""
		echo ""
		_fname_prefix="${_fio__size_gb}GB_RandomRun4kA_OLTP${_iosz}kB_${_thrx}Thrx";
		_fname_suffix="QD";
		echo "## FIO benchmark result -- $(tstamp) -- ${_fname_prefix}";
		echo "IODepth 	Rd.BW_KBps	Rd.IOPS		Rd.Lat.Mean_us		Rd.Lat.SD_us		Wr.BW_KBps	Wr.IOPS		Wr.Lat.Mean_us		Wr.Lat.SD_us";
		for _iod in $_fio__iod; do #{
			_fname_orig=${_fname_prefix}${_iod}${_fname_suffix}; #_fname_orig="${_fio__size_gb}GB_RandomRun4kA_OLTP${_iosz}kB_${_thrx}Thrx${_iod}QD";
			_fname="${_fname_orig}.proc_1";
			_lnum_total=$(cat -n $_fname_orig | wc -l);
			_lnum_fio_ver=$( cat -n $_fname_orig | grep '\<fio-[\.0-9]*\>' | awk '{ print $1 }');
			_lnum_4_tail=$(expr $_lnum_total - $_lnum_fio_ver);
			cat $_fname_orig | tail --lines=${_lnum_4_tail} > $_fname
			if [ ! -f $_fname ]; then
				echo ">>> ERROR: $_fname does not exist";
			fi
			_read_bw_iops=$(cat $_fname | head -4 | tail -1);
			_read_bw=$(echo $_read_bw_iops | awk '{print $4}' | sed -e 's/bw=\(.*\)KB\/s,/\1/g' );
			_read_iops=$(echo $_read_bw_iops | awk '{print $5}' | sed -e 's/iops=\(.*\),/\1/g' );
			_read_lat_avg_sd=$(cat $_fname | head -7 | tail -1 | sed -e 's/min=[0-9]*, max=[0-9]*, //g');
			_read_lat_avg_p1=$( echo $_read_lat_avg_sd | awk -F':' '{ print $2 }' | awk -F',' '{ print $1 }' | awk -F'=' '{ print $2}'); export _read_lat_avg_p1;
			_read_lat_sd_p1=$( echo $_read_lat_avg_sd | awk -F':' '{ print $2 }' | awk -F',' '{ print $2 }' | awk -F'=' '{ print $2}'); export _read_lat_sd_p1;
			_read_lat_avg=$( echo $_read_lat_avg_sd | awk '{ if ($2 == "(msec):") { print ENVIRON["_read_lat_avg_p1"] * 1000 } else { print ENVIRON["_read_lat_avg_p1"] } }');
			_read_lat_sd=$( echo $_read_lat_avg_sd | awk '{ if ($2 == "(msec):") { print ENVIRON["_read_lat_sd_p1"] * 1000} else { print ENVIRON["_read_lat_sd_p1"] } }');
			_write_bw_iops=$(cat $_fname | head -15 | tail -1);
			_write_bw=$(echo $_write_bw_iops | awk '{print $3}' | sed -e 's/bw=\(.*\)KB\/s,/\1/g' );
			_write_iops=$(echo $_write_bw_iops | awk '{print $4}' | sed -e 's/iops=\(.*\),/\1/g' );
			_write_lat_avg_sd=$(cat $_fname | head -18 | tail -1 | sed -e 's/min=[0-9]*, max=[0-9]*, //g' );
			_write_lat_avg_p1=$( echo $_write_lat_avg_sd | awk -F':' '{ print $2 }' | awk -F',' '{ print $1 }' | awk -F'=' '{ print $2}'); export _write_lat_avg_p1;
			_write_lat_sd_p1=$( echo $_write_lat_avg_sd | awk -F':' '{ print $2 }' | awk -F',' '{ print $2 }' | awk -F'=' '{ print $2}'); export _write_lat_sd_p1;
			_write_lat_avg=$( echo $_write_lat_avg_sd | awk '{ if ($2 == "(msec):") { print ENVIRON["_write_lat_avg_p1"] * 1000 } else { print ENVIRON["_write_lat_avg_p1"] } }');
			_write_lat_sd=$( echo $_write_lat_avg_sd | awk '{ if ($2 == "(msec):") { print ENVIRON["_write_lat_sd_p1"] * 1000} else { print ENVIRON["_write_lat_sd_p1"] } }');
			echo "$_iod\t\t$_read_bw\t\t$_read_iops\t\t$_read_lat_avg\t\t\t$_read_lat_sd\t\t\t$_write_bw\t\t$_write_iops\t\t$_write_lat_avg\t\t\t$_write_lat_sd";
		done #}
	done #}
done #}




