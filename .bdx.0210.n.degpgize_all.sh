#!/bin/sh

## .bdx.0210.y.degpgize_all.sh


_tstamp=$(date +%Y%m%d_%H%M%S);
_bpg_ca_comment="Brian M. JUNG <brian.m.jung@gmail.com> // $_tstamp";
_bpg_ca="gpg -ca --cipher-algo twofish --openpgp --comment \"$_bpg_ca_comment\"";


read -p "#>> passphrase for bpg: " _passphrase;

for _i in `find . -name '*.gpgizeme*.asc'`
do
	echo "#>> degpgize $_i";
	echo $_passphrase | gpg --passphrase-fd 0 $_i;
	if [ "X$?" = "X0" ]; then
		rm $_i;
	else
		echo "#>> ERROR: wrong passphrase! abort this operation.";
		exit 1;
	fi

done

for _i in `find . -name '*.gpgizeme.tgz'`
do
	echo "#>> untar $_i";
	gunzip -c $_i | tar xf -;
	rm $_i;
done
