FIO_OPER="sspt" ./.run_fio;
./.gen_r_input > fio_result_r_dataframe.log;
cat fio_result_r_dataframe.log;
