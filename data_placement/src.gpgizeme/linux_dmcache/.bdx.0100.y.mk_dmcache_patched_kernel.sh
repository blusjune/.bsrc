#!/bin/sh


_kernel_ver="3.0.8";
_kernel_name="linux-${_kernel_ver}";
_kernel_tarball="${_kernel_name}.tar.bz2";
_dmcache_dir="dm-cache";
_dmcache_patch=".dm_cache.patch-${_kernel_ver}";




echo "> 1. git cloning dm-cache";
if [ ! -d $_dmcache_dir ]; then
	echo ">> git clone https://github.com/mingzhao/dm-cache.git";
	git clone https://github.com/mingzhao/dm-cache.git;
	if [ ! -f $_dmcache_patch ]; then
		cp dm-cache/stable/3.0/patch-3.0.8 $_dmcache_patch;
		echo ">>> please modify $_dmcache_patch file to remove the string '/usr/src/$_kernel_name'";
		echo ">>> escape to the shell now";
		bash;
	fi
fi


echo "> 2. download $_kernel_tarball";
if [ ! -f $_kernel_tarball ]; then
	echo ">> wget http://www.kernel.org/pub/linux/kernel/v3.0/$_kernel_tarball";
	wget http://www.kernel.org/pub/linux/kernel/v3.0/$_kernel_tarball;
fi


echo "> 3. decompress $_kernel_tarball ('${_kernel_name}/' directory will be created)";
if [ ! -d $_kernel_name ]; then
	echo ">> bunzip2 -c $_kernel_tarball | tar xf -";
	bunzip2 -c $_kernel_tarball | tar xf -;
fi


echo "> 4. patch dm-cache to $_kernel_name";
if [ -f $_dmcache_patch -a -d $_kernel_name ]; then
	if [ ! -f ${_kernel_name}/${_dmcache_patch} ]; then
		echo ">> cp $_dmcache_patch $_kernel_name";
		cp $_dmcache_patch $_kernel_name;
		echo ">> ( cd $_kernel_name; patch -p0 < $_dmcache_patch; )";
		( cd $_kernel_name; patch -p0 < $_dmcache_patch; )
	fi
fi


echo "> 5. now you can build dm-cache patched kernel";
echo "     $ make menuconfig; make";
echo "     please make sure that you have .config file with 'CONFIG_DM_CACHE=m'";
echo "";
echo "> 6. install dmcache_ctrl files:";
echo "     after installing the dm-cache enabled kernel on the system,";
echo "     please exec _BDX at './cachectrl' directory to install the dmcache_ctrl files"




