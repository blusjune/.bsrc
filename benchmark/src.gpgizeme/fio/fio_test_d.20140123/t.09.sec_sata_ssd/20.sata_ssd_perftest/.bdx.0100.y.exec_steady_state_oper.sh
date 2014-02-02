#FIO_OPER="sspt" ./.run_fio;
FIO_OPER="pt" ./.run_fio;
_size_gb=480 ./.gen_r_input > fio_result_r_dataframe.log;
cat fio_result_r_dataframe.log;
