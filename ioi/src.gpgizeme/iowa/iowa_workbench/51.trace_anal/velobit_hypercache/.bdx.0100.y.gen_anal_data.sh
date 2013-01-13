#!/bin/sh

##.bdx.0100.y.gen_anal_data.sh
##_ver=20121022_210200
##_ver=20121213_020226


_tstamp="$(tstamp)";
_velo_logname=".velo_tracelog";




## data files
_df1_hit_ratio="_df1_hit_ratio.txt";
_df1_read_reqs="_df1_read_reqs.txt";
_df1_write_reqs="_df1_write_reqs.txt";
_df1_reads_from_hdd="_df1_reads_from_hdd.txt";
_df1_writes_to_hdd="_df1_writes_to_hdd.txt";
_df1_reads_from_ssd="_df1_reads_from_ssd.txt";
_df1_writes_to_ssd="_df1_writes_to_ssd.txt";
_df1_no_vblocks="_df1_no_vblocks.txt";
_df1_ref_blks="_df1_ref_blks.txt";
_df1_ass_blks="_df1_ass_blks.txt";
_df1_ind_blks="_df1_ind_blks.txt";
_df1_mem_used="_df1_mem_used.txt";
_df1_ssd_used="_df1_ssd_used.txt";
_df1_false_pos_refs="_df1_false_pos_refs.txt";
_df1_false_pos_indep="_df1_false_pos_indep.txt";
_df2_merged_table="_df2_merged_table.txt";




_velo_log="$(cat $_velo_logname)";
if [ "X$_velo_log" = "X" ]; then
	echo "> could not find $_velo_logname file";
	read -p ">> please specify the velobit tracelog file: " _velo_log;
	if [ "X$_velo_log" = "X" ]; then
		_velo_log="velomgmt.log";
	fi
fi




cat $_velo_log | grep 'hit ratio' | awk '{ print $13 }' > $_df1_hit_ratio
cat $_velo_log | grep 'read reqs' | awk '{ print $3 }' > $_df1_read_reqs
cat $_velo_log | grep 'write_reqs' | awk '{ print $5 }' > $_df1_write_reqs
cat $_velo_log | grep 'read from disk' | awk '{print $4}' > $_df1_reads_from_hdd
cat $_velo_log | grep 'writes to disk' | awk '{ print $8 }' > $_df1_writes_to_hdd
cat $_velo_log | grep 'reads from ssd' | awk '{ print $4 }' > $_df1_reads_from_ssd
cat $_velo_log | grep 'writes to ssd' | awk '{ print $8 }' > $_df1_writes_to_ssd
cat $_velo_log | grep '#vblocks' | awk '{ print $10 }' > $_df1_no_vblocks
cat $_velo_log | grep 'reference blocks' | awk '{ print $3 }' > $_df1_ref_blks
cat $_velo_log | grep 'associated blocks' | awk '{ print $6 }' > $_df1_ass_blks
cat $_velo_log | grep 'independent blocks' | awk '{ print $9 }' > $_df1_ind_blks
cat $_velo_log | grep 'Mem alloc/used' | awk '{ print $3 }' | sed -e 's/[0-9]*\/\([0-9]*\)/\1/g' > $_df1_mem_used
cat $_velo_log | grep 'SSD used' | awk '{ print $6 }' > $_df1_ssd_used
cat $_velo_log | grep 'false pos refs' | awk '{ print $12 }' > $_df1_false_pos_refs
cat $_velo_log | grep 'false pos indep' | awk '{ print $4 }' > $_df1_false_pos_indep




echo "\
$_df1_hit_ratio \
$_df1_read_reqs \
$_df1_write_reqs \
$_df1_reads_from_hdd \
$_df1_writes_to_hdd \
$_df1_reads_from_ssd \
$_df1_writes_to_ssd \
$_df1_no_vblocks \
$_df1_ref_blks \
$_df1_ass_blks \
$_df1_ind_blks \
$_df1_mem_used \
$_df1_ssd_used \
$_df1_false_pos_refs \
$_df1_false_pos_indep \
" | sed -e 's/_df1_//g' | sed -e 's/\.txt//g' > $_df2_merged_table
#" | sed -e 's/_df1_\(.*\)\.txt/\1/g' > $_df2_merged_table




paste \
$_df1_hit_ratio \
$_df1_read_reqs \
$_df1_write_reqs \
$_df1_reads_from_hdd \
$_df1_writes_to_hdd \
$_df1_reads_from_ssd \
$_df1_writes_to_ssd \
$_df1_no_vblocks \
$_df1_ref_blks \
$_df1_ass_blks \
$_df1_ind_blks \
$_df1_mem_used \
$_df1_ssd_used \
$_df1_false_pos_refs \
$_df1_false_pos_indep \
>> $_df2_merged_table




