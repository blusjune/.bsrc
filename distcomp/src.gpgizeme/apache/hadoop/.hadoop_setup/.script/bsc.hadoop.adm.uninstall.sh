#!/bin/sh
### Fri Jan 27 00:42:44 KST 2012




#. ${HOME}/.b/.src/x/hadoop/bsc.hadoop.conf.env_var.cf
.  bsc.hadoop.conf.env_var.cf




if [ "X$(id -u)" != "X0" ]; then
	echo ">>> This command should be executed with a root privilege";
	echo ">>> Terminate this program";
	exit 1;
fi


read -p ">>> hadoop uninstallation";
echo "    - this will delete account 'hduser:hadoop'";
echo "    - this will remove '/usr/local/hadoop' and '/app/hadoop/tmp'";

read -p ">>> Really want to uninstall hadoop? [y|n] " _answer;

if [ "X${_answer}" = "Xy" ]; then
	userdel -r hduser; groupdel hadoop;
	rm /usr/local/hadoop;
	rm -fr /app/hadoop/tmp;
	echo ">>> uninstalled successfully";
fi




