#!/bin/sh
### Fri Jan 27 00:42:44 KST 2012
##_ver=20130109_201257
##_ver=20130110_192518
##_ver=20130110_230517




#. ${HOME}/.b/.src/x/hadoop/bsc.hadoop.conf.env_var.cf
. ./bsc.hadoop.conf.env_var.cf




echo "
-----------------------------------
Hadoop installation $(tstamp)
-----------------------------------
";


### sanity check
if [ "X$(id -u)" != "X0" ]; then
	echo ">>> ERROR: This command should be executed with a root privilege -- EXIT";
	echo "    Terminate this program";
	exit 1;
fi

if [ -d ${BX_HADOOP__ROOT_PREFIX}/hadoop -o -L ${BX_HADOOP__ROOT_PREFIX}/hadoop ]; then
	echo ">>> WARNING: ${BX_HADOOP__ROOT_PREFIX}/hadoop exists.";
	read -p "    seems there exists previously installed hadoop. remove it? [y|n] " _ans;
	if [ "X$_ans" = "Xy" ]; then
		./bsc.hadoop.adm.uninstall.sh
	else
		echo "Please try again after removing previous version of hadoop -- EXIT";
		exit 1;
	fi
fi

if [ ! -d .files ]; then
	echo ">>> '.files' directory does not exist (now creating)";
	mkdir .files;
fi


echo "
>>> Installation begins with the following environment variables:"
echo "    BX_HADOOP__VERSION=${BX_HADOOP__VERSION}";
echo "    BX_HADOOP__NO_NEED_TO_DOWNLOAD=${BX_HADOOP__NO_NEED_TO_DOWNLOAD}"; 
echo "    BX_HADOOP__STABLE_DOWNLOAD_URL=${BX_HADOOP__STABLE_DOWNLOAD_URL}";
echo "    BX_HADOOP__LATEST_DOWNLOAD_URL=${BX_HADOOP__LATEST_DOWNLOAD_URL}"; 
echo "    BX_HADOOP__ROOT_PREFIX=${BX_HADOOP__ROOT_PREFIX}";
echo "    BX_HADOOP__HADOOP_HOME=${BX_HADOOP__HADOOP_HOME}";
echo "    BX_HADOOP__JAVA_HOME=${BX_HADOOP__JAVA_HOME}";
echo "    BX_HADOOP__TMP_DIR=${BX_HADOOP__TMP_DIR}";


### determine hadoop version to download
_hdpkg="_hadoop.tgz"
_hdpkg_selected="";
_hdpkg_url="";

case "X${BX_HADOOP__VERSION}" in
"Xstable")
	_hdpkg_stable=$(echo ${BX_HADOOP__STABLE_DOWNLOAD_URL} | sed -e 's/.*\(hadoop-.*\.tar\.gz\)/\1/g')
	_hdpkg_selected=$_hdpkg_stable;
	_hdpkg_url="$BX_HADOOP__STABLE_DOWNLOAD_URL";
	echo ">>> Hadoop stable version ($_hdpkg_selected) is selected"
	;;
"Xlatest")
	_hdpkg_latest=$(echo ${BX_HADOOP__LATEST_DOWNLOAD_URL} | sed -e 's/.*\(hadoop-.*\.tar\.gz\)/\1/g')
	_hdpkg_selected=$_hdpkg_latest;
	_hdpkg_url="$BX_HADOOP__LATEST_DOWNLOAD_URL";
	echo ">>> Hadoop latest version ($_hdpkg_selected) is selected"
	;;
*)
	_hdpkg_stable=$(echo ${BX_HADOOP__STABLE_DOWNLOAD_URL} | sed -e 's/.*\(hadoop-.*\.tar\.gz\)/\1/g')
	_hdpkg_selected=$_hdpkg_stable;
	_hdpkg_url="$BX_HADOOP__STABLE_DOWNLOAD_URL";
	echo ">>> going default (Hadoop stable: $_hdpkg_selected)"
	;;
esac


echo ">>> If you already have $_hdpkg_selected file,
you can place it into .files directory to avoid downloading";
echo "";
read -p ">>> Escape to the shell? [y|n] " _ans;
if [ "X$_ans" = "Xy" ]; then
	bash;
fi

echo "";
read -p ">>> Ready to go? [y|n] " _ans;
if [ "X$_ans" = "Xn" ]; then
	echo ">>> Installation Aborted -- Exit Now";
	exit 1;
fi


### add a dedicated Hadoop system user and group (hduser:hadoop)
_hduser_idval="2007";
id -un hduser > /dev/null;
if [ "X$?" != "X0" ]; then
	echo "";
	echo ">>> hduser does not exist, now create hduser";
#	addgroup hadoop
	groupadd -g $_hduser_idval hadoop
	useradd -p "" -m -g hadoop -s /bin/bash -u $_hduser_idval hduser	# adduser --ingroup hadoop hduser
fi


### generate ssh key
echo "
>>> generate ssh key for hduser";
cat << EOF > ~hduser/.tmp.1.sh
ssh-keygen -t rsa -P ""
cat ~hduser/.ssh/id_rsa.pub >> ~hduser/.ssh/authorized_keys
EOF
chmod 755 ~hduser/.tmp.1.sh
chown hduser:hadoop ~hduser/.tmp.1.sh
sudo -u hduser ~hduser/.tmp.1.sh
rm ~hduser/.tmp.1.sh


### disable IPv6
cat << EOF >> /etc/sysctl.conf

# disable IPv6
net.ipv6.conf.all.disable_ipv6=1
net.ipv6.conf.default.disable_ipv6=1
net.ipv6.conf.lo.disable_ipv6=1
EOF
sysctl -w net.ipv6.conf.all.disable_ipv6=1
sysctl -w net.ipv6.conf.default.disable_ipv6=1
sysctl -w net.ipv6.conf.lo.disable_ipv6=1


_bxhd_download_hdpkg()
{
	echo "wget $_hdpkg_url";
	wget $_hdpkg_url;
	echo "ln -s $_hdpkg_selected $_hdpkg";
	ln -s $_hdpkg_selected $_hdpkg;
}

if [ "X${BX_HADOOP__NO_NEED_TO_DOWNLOAD}" = "Xyes" ]; then
	if [ ! -f .files/${_hdpkg_selected} ]; then
		(cd .files; _bxhd_download_hdpkg;)
	fi
else
	(cd .files; _bxhd_download_hdpkg;)
fi


### extract hadoop tar.gz file
mkdir ~hduser/hd_work;
gunzip -c .files/${_hdpkg_selected} | (cd ~hduser/hd_work; tar xf -);
_hadoop_dir_basename=$(cd ~hduser/hd_work; ls -1d hadoop-*);
_hadoop_dir_fullpath=$(cd ~hduser/hd_work; pwd)/${_hadoop_dir_basename}
(cd ~hduser; chown -R hduser:hadoop hd_work);


### install (link) hadoop directory
cd ${BX_HADOOP__ROOT_PREFIX}
ln -s ${_hadoop_dir_fullpath} hadoop
chown -R hduser:hadoop hadoop


### add important environment variables to hduser's .bashrc
HADOOP_HOME=${BX_HADOOP__HADOOP_HOME}; # HADOOP_HOME should be '/usr/local/hadoop'
if [ "X$(grep beee_hadoop_work ~hduser/.bashrc > /dev/null; echo $?)" != "X0" ]; then
	echo "";
	echo ">>> Setup the environment variables in hduser's .bashrc"
	echo "    HADOOP_HOME=${BX_HADOOP__HADOOP_HOME}"
	echo "    JAVA_HOME=${BX_HADOOP__JAVA_HOME}"
	echo "    PATH=\$PATH:\$HADOOP_HOME/bin:. export PATH"

	echo "" >> ~hduser/.bashrc
	echo "### beee_hadoop_work { ###" >> ~hduser/.bashrc
	echo "HADOOP_HOME=${BX_HADOOP__HADOOP_HOME} export HADOOP_HOME" >> ~hduser/.bashrc
	echo "JAVA_HOME=${BX_HADOOP__JAVA_HOME} export JAVA_HOME" >> ~hduser/.bashrc
	echo "PATH=\$PATH:\$HADOOP_HOME/bin:. export PATH" >> ~hduser/.bashrc
	echo "" >> ~hduser/.bashrc
	echo "alias l='ls -alF'" >> ~hduser/.bashrc
	echo "### } beee_hadoop_work ###" >> ~hduser/.bashrc
fi	


### create Hadoop tmp directory
mkdir -p ${BX_HADOOP__TMP_DIR};
chown hduser:hadoop ${BX_HADOOP__TMP_DIR};
chmod 750 ${BX_HADOOP__TMP_DIR};


### Hadoop configuration: $HADOOP_HOME/conf/hadoop-env.sh
cd $HADOOP_HOME/conf;
echo "" >> hadoop-env.sh;
echo "JAVA_HOME=${BX_HADOOP__JAVA_HOME}" >> hadoop-env.sh;




###--------------------------------------------------




_hd_conf_gen__v01()
{ ##_ver=20130110_191441


### This configuration file works well at least the following Hadoop versions:
###
### Hadoop 0.20.203.X 
### Hadoop 0.20.205.X
### Hadoop 1.0.4.X
### Hadoop 1.1.1.X


### core-site.xml
cd $HADOOP_HOME/conf;
cat << EOF > beee.core-site.xml
<?xml version="1.0"?>
<?xml-stylesheet type="text/xsl" href="configuration.xsl"?>
<!-- Automatically generated by BEEE $(date)-->
<!-- Put site-specific property overrides in this file. -->
<configuration>
	<!-- conf/core-site.xml -->
	<property>
		<name>hadoop.tmp.dir</name>
		<value>${BX_HADOOP__TMP_DIR}</value>
	</property>
	<property>
		<name>fs.default.name</name>
		<value>${BX_HADOOP__CONF__FS_DEFAULT_NAME}</value>
	</property>
</configuration>
EOF

if [ -f core-site.xml ]; then
	mv core-site.xml orig.core-site.xml;
fi
ln -s beee.core-site.xml core-site.xml;


### mapred-site.xml 
cd $HADOOP_HOME/conf;
cat << EOF > beee.mapred-site.xml
<?xml version="1.0"?>
<?xml-stylesheet type="text/xsl" href="configuration.xsl"?>
<!-- Automatically generated by BEEE $(date)-->
<!-- Put site-specific property overrides in this file. -->
<configuration>
	<!-- conf/mapred-site.xml -->
	<property>
		<name>mapred.job.tracker</name>
		<value>${BX_HADOOP__CONF__MAPRED_JOB_TRACKER}</value>
	</property>
</configuration>
EOF

if [ -f mapred-site.xml ]; then
	mv mapred-site.xml orig.mapred-site.xml;
fi
ln -s beee.mapred-site.xml mapred-site.xml;


### hdfs-site.xml 
cd $HADOOP_HOME/conf;
cat << EOF > beee.hdfs-site.xml 
<?xml version="1.0"?>
<?xml-stylesheet type="text/xsl" href="configuration.xsl"?>
<!-- Automatically generated by BEEE $(date)-->
<!-- Put site-specific property overrides in this file. -->
<configuration>
	<!-- conf/hdfs-site.xml -->
	<property>
		<name>dfs.replication</name>
		<value>${BX_HADOOP__CONF__DFS_REPLICATION}</value>
	</property>
</configuration>
EOF

if [ -f hdfs-site.xml ]; then
	mv hdfs-site.xml orig.hdfs-site.xml;
fi
ln -s beee.hdfs-site.xml hdfs-site.xml 


### masters
cd $HADOOP_HOME/conf;
if [ "X${BX_HADOOP__MASTER_NODE_NUM}" = "X" ]; then
	read -p ">>>>>> How many master nodes? " _num_of_master
else
	_num_of_master=${BX_HADOOP__MASTER_NODE_NUM};
fi

echo "### Hadoop cluster node ###" > beee.etc_hosts;
echo "" >> beee.etc_hosts;
echo -n "" > beee.masters; # to make it sure that target is blank file

_i=0;
while [ ${_i} -lt ${_num_of_master} ]; do
	_i=$(expr $_i + 1);
	printf "ip.v4.addr.%d hd-master-%02d\n" ${_i} ${_i} >> beee.etc_hosts;
	printf "hd-master-%02d\n" ${_i} >> beee.masters;
done
if [ -f masters ]; then
	mv masters orig.masters
fi
ln -s beee.masters masters


### slaves
cd $HADOOP_HOME/conf;
if [ "X${BX_HADOOP__SLAVE_NODE_NUM}" = "X" ]; then
	echo "";
	read -p ">>>>>> How many slave nodes? " _num_of_slave
else
	_num_of_slave=${BX_HADOOP__SLAVE_NODE_NUM};
fi

echo -n "" > beee.slaves; # to make sure that target is blank file

_j=0;
while [ ${_j} -lt ${_num_of_slave} ]; do
	_i=$(expr $_i + 1);
	_j=$(expr $_j + 1);
	printf "ip.v4.addr.%d hd-slave-%04d\n" ${_i} ${_j} >> beee.etc_hosts;
	printf "hd-slave-%04d\n" ${_j} >> beee.slaves;
done
if [ -f slaves ]; then
	mv slaves orig.slaves
fi
ln -s beee.slaves slaves

echo "" >> beee.etc_hosts;


### /etc/hosts

echo "
>>> modified Hadoop configuration files:
	/usr/local/hadoop/conf/
		core-site.xml 
		mapred-site.xml 
		hdfs-site.xml 
		masters
		slaves
>>> to-be-modified system configuration files:
	/etc/hosts";

echo "
>>> Please modify /etc/hosts file using 'beee.etc_hosts' file
    to add ${_num_of_master} hd-master-XX node(s) and
           ${_num_of_slave} hd-slave-XXXX node(s)";
if [ "X$(grep $(hostname) /etc/hosts > /dev/null; echo $?)" = "X0" ]; then
	echo "    and to unmap '$(hostname)' from '127.0.0.1' OR '::1'"
fi
echo "";
read -p ">>>>>> Escape to the shell to modify '/etc/hosts' ? [y|n] " _answer
if [ "X${_answer}" = "Xy" -o "X${_answer}" = "XY" ]; then
	bash;
fi


} ## _hd_conf_gen__v01() ##_ver=20130110_191441




###--------------------------------------------------

### As of 2013-01-10, per-version information is:
###
### 1.0.X		: current stable version, 1.0 release
### 1.1.X		: current beta version, 1.1 release
### 2.X.X		: current alpha version
### 0.23.X		: similar to 2.X.X but missing NN HA
### 0.22.X		: does not include security
### 0.20.203.X	: old legacy stable version
### 0.20.X		: old legacy version

### As of 2012-02-12, per-version information is:
### 
### 0.20.X		: legacy stable version
### 0.20.203.X	: current stable version
### 0.22.X		: does not include security
### 0.23.X		: current alpha version
### 1.0.X		: current beta version, 1.0 release

### This Hadoop installation script supports well the following version
###	0.20.203.X version
###	1.0.4 version

###--------------------------------------------------

_hadoop_version=$(echo $_hadoop_dir_basename | sed -e 's/hadoop-\(.*\)/\1/g');
if [ "X${_hadoop_version}" = "X" ]; then
	echo "";
	echo ">>> ERROR:: cannot determine the version of downloaded Hadoop package";
	exit 1;
fi
_hadoop_ver_major=$(echo $_hadoop_version | sed -e 's/\([0-9]*\)\..*/\1/g');
_hadoop_ver_minor=$(echo $_hadoop_version | sed -e 's/[0-9]*\.\([0-9]*\)\..*/\1/g');
_hadoop_ver_minor_sub=$(echo $_hadoop_version | sed -e 's/[0-9]*\.[0-9]*\.\([0-9]*\)\..*/\1/g');
_hadoop_conf_gen_func="_hd_conf_gen__${_hadoop_ver_major}_${_hadoop_ver_minor}_${_hadoop_ver_minor_sub}_X";
if [ "X$_hadoop_ver_minor_sub" = "X$_hadoop_version" ]; then
	_hadoop_ver_minor_sub=$(echo $_hadoop_version | sed -e 's/[0-9]*\.[0-9]*\.\([0-9]*\)/\1/g');
	_hadoop_conf_gen_func="_hd_conf_gen__${_hadoop_ver_major}_${_hadoop_ver_minor}_${_hadoop_ver_minor_sub}";
fi

echo "## just for debugging ##";
echo "_hadoop_version = ($_hadoop_version)";
echo "_hadoop_ver_major = ($_hadoop_ver_major)";
echo "_hadoop_ver_minor = ($_hadoop_ver_minor)";
echo "_hadoop_ver_minor_sub = ($_hadoop_ver_minor_sub)";
echo "_hadoop_conf_gen_func = ($_hadoop_conf_gen_func)";


echo "
>>> Automatic generation of Hadoop configuration files:";


case ".${_hadoop_ver_major}.${_hadoop_ver_minor}.${_hadoop_ver_minor_sub}" in
".0.20.203")
	echo ">>> _hd_conf_gen__v01 ($_hadoop_conf_gen_func) will be executed";
#	_hd_conf_gen__0_20_203_X;
	_hd_conf_gen__v01;
	;;
".0.20.205")
	echo ">>> _hd_conf_gen__v01 ($_hadoop_conf_gen_func) will be executed";
#	_hd_conf_gen__0_20_205_X;
	_hd_conf_gen__v01;
	;;
".0.23.1")
	echo ">>> _hd_conf_gen__v01 ($_hadoop_conf_gen_func) will be executed";
#	_hd_conf_gen__0_23_1;
	_hd_conf_gen__v01;
	;;
".1.0.4")
	echo ">>> _hd_conf_gen__v01 ($_hadoop_conf_gen_func) will be executed";
#	_hd_conf_gen__1_0_4_X;
	_hd_conf_gen__v01;
	;;
".1.1.1")
	echo ">>> _hd_conf_gen__v01 ($_hadoop_conf_gen_func) will be executed";
#	_hd_conf_gen__1_1_1_X;
	_hd_conf_gen__v01;
	;;
*)
	echo ">>> _hd_conf_gen__${_hadoop_ver_major}_${_hadoop_ver_minor}_${_hadoop_ver_minor_sub}_X is not supported yet";
	exit 1;
	;;
esac


echo "
>>> CAUTION!
    You SHOULD NOT use underscore \"_\" in host name such as:
    'hd_master_01' or 'hd_slave_0001'

    This may cause frustrating ERROR such as:
    \"ERROR org.apache.hadoop.hdfs.server.datanode.DataNode:
    java.lang.IllegalArgumentException:
    Does not contain a valid host:port authority\"

    Please DO USE hypen \"-\" if needed, just like:
    'hd-master-01' or 'hd-slave-0001'

>>> Congratulations!!! Hadoop installed successfully.
";




