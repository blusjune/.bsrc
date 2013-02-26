#!/bin/sh
## .bdx.0100.y.mk_anal_data_from_velomgmt_log.sh
##
##tstamp: 20121023_014154
##tstamp: 20121027_023410




_tstamp="$(tstamp)";
_velolog_fn=".file";




if [ -f $_velolog_fn ]; then
	_cache_trace_log="$(cat $_velolog_fn)";
	if [ "X$_cache_trace_log" = "X" ]; then
		read -p ">> please specify the velobit tracelog file: " _cache_trace_log;
		if [ "X$_cache_trace_log" = "X" ]; then
			_cache_trace_log="cache_trace.log"; # default
		fi
	fi
else
	echo "> could not find $_velolog_fn file";
	exit 0;
fi




_tid="$(basename $(pwd))";
read -p "> Do you want to use '$_tid' as Test ID? [y|n] " _ans;
if [ "X$_ans" != "Xy" ]; then
	read -p "> Set Test ID: " _tid;
fi

_merged_table="_df2.merged_table.${_tid}.txt";
_gziped_table="${_merged_table}.gz";
_gziped_cachetrace="${_cache_trace_log}.${_tid}.gz";
_dir_apple="/x/tmp/bmt/${_tstamp}-${_tid}";




_t="t";
if [ ! -d $_t ]; then
	mkdir -p $_t;
fi
cd t;
ln -s ../$_cache_trace_log;




##extract_list{
cat $_cache_trace_log | grep 'hit ratio' | awk '{ print $13 }' > _df1.hit_ratio;
##
cat $_cache_trace_log | grep 'read reqs' | awk '{ print $3 }' > _df1.read_reqs;
cat $_cache_trace_log | grep 'write_reqs' | awk '{ print $5 }' > _df1.write_reqs;
cat $_cache_trace_log | grep 'read from disk' | awk '{print $4}' > _df1.reads_from_hdd;
cat $_cache_trace_log | grep 'writes to disk' | awk '{ print $8 }' > _df1.writes_to_hdd;
cat $_cache_trace_log | grep 'reads from ssd' | awk '{ print $4 }' > _df1.reads_from_ssd;
cat $_cache_trace_log | grep 'writes to ssd' | awk '{ print $8 }' > _df1.writes_to_ssd;
##
cat $_cache_trace_log | grep '#vblocks' | awk '{ print $10 }' > _df1.no_vblocks;
cat $_cache_trace_log | grep 'reference blocks' | awk '{ print $3 }' > _df1.ref_blks;
cat $_cache_trace_log | grep 'associated blocks' | awk '{ print $6 }' > _df1.ass_blks;
cat $_cache_trace_log | grep 'independent blocks' | awk '{ print $9 }' > _df1.ind_blks;
cat $_cache_trace_log | grep '#dirty blocks' | awk '{ print $11 }' > _df1.dirty_blks;
cat $_cache_trace_log | grep 'Flushing RAM' | sed -e 's/.*Flushing RAM\/s: \([0-9]*\).*/\1/g' > _df1.flushing_ram_per_sec;
cat $_cache_trace_log | grep 'Flushing SSD' | sed -e 's/.*Flushing SSD\/s: \([0-9]*\).*/\1/g' > _df1.flushing_ssd_per_sec;
cat $_cache_trace_log | grep 'Destage RAM/s' | awk '{ print $3 }' > _df1.destage_ram_per_sec;
cat $_cache_trace_log | grep 'Destage SSD/s' | awk '{ print $6 }' > _df1.destage_ssd_per_sec;
cat $_cache_trace_log | grep 'Destage vblk/s' | awk '{ print $9 }' > _df1.destage_vblk_per_sec;
##
cat $_cache_trace_log | grep 'Mem alloc/used' | awk '{ print $3 }' | sed -e 's/[0-9]*\/\([0-9]*\)/\1/g' > _df1.mem_used;
cat $_cache_trace_log | grep 'SSD used' | awk '{ print $6 }' > _df1.ssd_used;
##
cat $_cache_trace_log | grep 'find ref success' | awk '{ print $4 }' > _df1.find_ref_success;
cat $_cache_trace_log | grep 'find ref failed' | awk '{ print $8 }' > _df1.find_ref_failed;
cat $_cache_trace_log | grep 'false pos refs' | awk '{ print $12 }' > _df1.false_pos_ref;
cat $_cache_trace_log | grep 'false pos assoc' | awk '{ print $8 }' > _df1.false_pos_ass;
cat $_cache_trace_log | grep 'false pos indep' | awk '{ print $4 }' > _df1.false_pos_ind;
##
cat $_cache_trace_log | grep 'assoc hit ram(r/w)' | awk '{ print $4 }' | sed -e 's/\([0-9]*\)\/\([0-9]*\)/\1/g' > _df1.assoc_hit_ram_r;
cat $_cache_trace_log | grep 'assoc hit ram(r/w)' | awk '{ print $4 }' | sed -e 's/\([0-9]*\)\/\([0-9]*\)/\2/g' > _df1.assoc_hit_ram_w;
cat $_cache_trace_log | grep 'assoc hit ssd(r/w)' | awk '{ print $8 }' | sed -e 's/\([0-9]*\)\/\([0-9]*\)/\1/g' > _df1.assoc_hit_ssd_r;
cat $_cache_trace_log | grep 'assoc hit ssd(r/w)' | awk '{ print $8 }' | sed -e 's/\([0-9]*\)\/\([0-9]*\)/\2/g' > _df1.assoc_hit_ssd_w;
cat $_cache_trace_log | grep 'indep hit ram(r/w)' | awk '{ print $4 }' | sed -e 's/\([0-9]*\)\/\([0-9]*\)/\1/g' > _df1.indep_hit_ram_r;
cat $_cache_trace_log | grep 'indep hit ram(r/w)' | awk '{ print $4 }' | sed -e 's/\([0-9]*\)\/\([0-9]*\)/\2/g' > _df1.indep_hit_ram_w;
cat $_cache_trace_log | grep 'indep hit ssd(r/w)' | awk '{ print $8 }' | sed -e 's/\([0-9]*\)\/\([0-9]*\)/\1/g' > _df1.indep_hit_ssd_r;
cat $_cache_trace_log | grep 'indep hit ssd(r/w)' | awk '{ print $8 }' | sed -e 's/\([0-9]*\)\/\([0-9]*\)/\2/g' > _df1.indep_hit_ssd_w;
##
cat $_cache_trace_log | grep 'input randomness%' | awk '{ print $3 }' > _df1.input_randomness;
cat $_cache_trace_log | grep 'output randomness%' | awk '{ print $6 }' > _df1.output_randomness;
##
cat $_cache_trace_log | grep 'avg read resp time' | awk '{ print $5 }' > _df1.avg_read_resp_time;
cat $_cache_trace_log | grep 'avg write resp time' | awk '{ print $10 }' > _df1.avg_write_resp_time;
cat $_cache_trace_log | grep 'avg read proc time' | awk '{ print $5 }' > _df1.avg_read_proc_time;
cat $_cache_trace_log | grep 'avg write proc time' | awk '{ print $10 }' > _df1.avg_write_proc_time;
##
cat $_cache_trace_log | grep 'ssd_avg_read_time' | awk '{ print $2 }' > _df1.ssd_avg_read_time;
cat $_cache_trace_log | grep 'ssd_avg_write_time' | awk '{ print $4 }' > _df1.ssd_avg_write_time;
cat $_cache_trace_log | grep 'disk_avg_read_time' | awk '{ print $6 }' > _df1.hdd_avg_read_time;
cat $_cache_trace_log | grep 'disk_avg_write_time' | awk '{ print $2 }' > _df1.hdd_avg_write_time;
##}

## cat list.1{42,91} | sed -e 's/.* > \(.*\);/\1/g' | grep -v '##' > list.2




echo "\
_df1.hit_ratio \
_df1.read_reqs \
_df1.write_reqs \
_df1.reads_from_hdd \
_df1.writes_to_hdd \
_df1.reads_from_ssd \
_df1.writes_to_ssd \
_df1.no_vblocks \
_df1.ref_blks \
_df1.ass_blks \
_df1.ind_blks \
_df1.dirty_blks \
_df1.flushing_ram_per_sec \
_df1.flushing_ssd_per_sec \
_df1.destage_ram_per_sec \
_df1.destage_ssd_per_sec \
_df1.destage_vblk_per_sec \
_df1.mem_used \
_df1.ssd_used \
_df1.find_ref_success \
_df1.find_ref_failed \
_df1.false_pos_ref \
_df1.false_pos_ass \
_df1.false_pos_ind \
_df1.assoc_hit_ram_r \
_df1.assoc_hit_ram_w \
_df1.assoc_hit_ssd_r \
_df1.assoc_hit_ssd_w \
_df1.indep_hit_ram_r \
_df1.indep_hit_ram_w \
_df1.indep_hit_ssd_r \
_df1.indep_hit_ssd_w \
_df1.input_randomness \
_df1.output_randomness \
_df1.avg_read_resp_time \
_df1.avg_write_resp_time \
_df1.avg_read_proc_time \
_df1.avg_write_proc_time \
_df1.ssd_avg_read_time \
_df1.ssd_avg_write_time \
_df1.hdd_avg_read_time \
_df1.hdd_avg_write_time \
" | sed -e 's/_df1.//g' > $_merged_table;




paste \
_df1.hit_ratio \
_df1.read_reqs \
_df1.write_reqs \
_df1.reads_from_hdd \
_df1.writes_to_hdd \
_df1.reads_from_ssd \
_df1.writes_to_ssd \
_df1.no_vblocks \
_df1.ref_blks \
_df1.ass_blks \
_df1.ind_blks \
_df1.dirty_blks \
_df1.flushing_ram_per_sec \
_df1.flushing_ssd_per_sec \
_df1.destage_ram_per_sec \
_df1.destage_ssd_per_sec \
_df1.destage_vblk_per_sec \
_df1.mem_used \
_df1.ssd_used \
_df1.find_ref_success \
_df1.find_ref_failed \
_df1.false_pos_ref \
_df1.false_pos_ass \
_df1.false_pos_ind \
_df1.assoc_hit_ram_r \
_df1.assoc_hit_ram_w \
_df1.assoc_hit_ssd_r \
_df1.assoc_hit_ssd_w \
_df1.indep_hit_ram_r \
_df1.indep_hit_ram_w \
_df1.indep_hit_ssd_r \
_df1.indep_hit_ssd_w \
_df1.input_randomness \
_df1.output_randomness \
_df1.avg_read_resp_time \
_df1.avg_write_resp_time \
_df1.avg_read_proc_time \
_df1.avg_write_proc_time \
_df1.ssd_avg_read_time \
_df1.ssd_avg_write_time \
_df1.hdd_avg_read_time \
_df1.hdd_avg_write_time \
>> $_merged_table;




if [ ! -d "$_dir_apple" ]; then
	_me=$(whoami);
	sudo mkdir -p $_dir_apple;
	sudo chown -R $_me:$_me $_dir_apple;
fi
gzip -c $_merged_table > $_gziped_table;
cp -p $_gziped_table $_dir_apple;
gzip -c $_cache_trace_log > $_gziped_cachetrace;
cp -p $_gziped_cachetrace $_dir_apple;
echo "> directory '$_dir_apple' contains:
	$_gziped_table
	$_gziped_cachetrace
";




cd ..;
ln -s $_dir_apple 




#####
#####
#####

exit 0;

#####
#####
#####




_df1.hit_ratio \
_df1.read_reqs \
_df1.write_reqs \
_df1.reads_from_hdd \
_df1.writes_to_hdd \
_df1.reads_from_ssd \
_df1.writes_to_ssd \
_df1.no_vblocks \
_df1.ref_blks \
_df1.ass_blks \
_df1.ind_blks \
_df1.dirty_blks \
_df1.flushing_ram_per_sec \
_df1.flushing_ssd_per_sec \
_df1.destage_ram_per_sec \
_df1.destage_ssd_per_sec \
_df1.destage_vblk_per_sec \
_df1.mem_used \
_df1.ssd_used \
_df1.find_ref_success \
_df1.find_ref_failed \
_df1.false_pos_ref \
_df1.false_pos_ass \
_df1.false_pos_ind \
_df1.assoc_hit_ram_r \
_df1.assoc_hit_ram_w \
_df1.assoc_hit_ssd_r \
_df1.assoc_hit_ssd_w \
_df1.indep_hit_ram_r \
_df1.indep_hit_ram_w \
_df1.indep_hit_ssd_r \
_df1.indep_hit_ssd_w \
_df1.input_randomness \
_df1.output_randomness \
_df1.avg_read_resp_time \
_df1.avg_write_resp_time \
_df1.avg_read_proc_time \
_df1.avg_write_proc_time \
_df1.ssd_avg_read_time \
_df1.ssd_avg_write_time \
_df1.hdd_avg_read_time \
_df1.hdd_avg_write_time \




