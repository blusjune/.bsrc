## for iozone output files after post-processing (line number added at column 1)
unset log;
set xlabel "Trials";
set ylabel "Latency (in msec)";
set yrange [-5:100];
set autoscale;
plot './num.0001.Child_0_randrol.dat.txt' u 1:3 w lines;
