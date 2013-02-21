_this_prog=".bdx.0100.y.iowa_start.sh";
_ver="20130131_012925";

echo "#>> Please make sure: tracelog is linked well & all the parameters are configured properly"
read -p "#<< Ready to go? [y|N] " _ans;
if [ "X$_ans" = "Xy" ]; then
	(cd preproc; _BDX)
	(cd analysis; _BDX)
else
	echo "#>> Try again when you get ready -- EXIT";
	exit 0;
fi
