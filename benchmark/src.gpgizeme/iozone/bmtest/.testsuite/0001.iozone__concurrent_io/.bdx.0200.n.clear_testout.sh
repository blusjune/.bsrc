#!/bin/sh

_tmpfile=".files_to_be_deleted";

echo "-----";
find . -name '_testlog*' >> $_tmpfile;
find . -name '_iozone*' >> $_tmpfile;
find ./td.* -name '*Child*dat*' >> $_tmpfile;
find ./td.* -name '.log.*' >> $_tmpfile;
_numoffiles="$(cat $_tmpfile | wc -l)";
cat $_tmpfile;
echo "-----";
echo "$_numoffiles test-output files are found.";

if [ "X$_numoffiles" = "X0" ]; then
	echo ">>> no files to delete.";
else
	read -p ">>> Do you want remove all the files listed above? [y|n] " _ans;
	if [ "X$_ans" = "XY" -o "X$_ans" = "Xy" ]; then
		rm -f $(find . -name '_testlog*');
		rm -f $(find . -name '_iozone*');
		rm -f $(find td.* -name '*Child*dat*');
		rm -f $(find td.* -name '.log.*');
		echo ">>> deleted well.";
	else
		echo ">>> nothing happened.";
	fi
fi

rm -f $_tmpfile;
