#!/bin/sh

## .bdx.0100.y.gpgize_all.sh

_tstamp=$(date +%Y%m%d_%H%M%S);
_bpg_ca_comment="Brian M. JUNG <brian.m.jung@gmail.com> // $_tstamp";
_bpg_ca="gpg -ca --cipher-algo twofish --openpgp --comment \"$_bpg_ca_comment\"";


read -p "passphrase for bpg: " _passphrase;

for _i in `find . -name '*.gpgizeme'`
do
	echo ">>> gpgize $_i";

	if [ -d $_i ]; then
		_tarball=${_i}.tgz;
		tar cf - $_i | gzip -c > $_tarball;
		_bpg_ca_target=$_tarball;
	else
		_bpg_ca_target=$_i;
	fi
	echo $_passphrase | gpg -ca --cipher-algo twofish --openpgp --comment "$_bpg_ca_comment" --passphrase-fd 0 $_bpg_ca_target;
	if [ -f $_tarball ]; then
		rm $_tarball;
	fi
	rm -fr $_i;
done
