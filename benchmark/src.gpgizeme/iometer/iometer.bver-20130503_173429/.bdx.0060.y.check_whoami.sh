#!/bin/sh

_myuid=$(id -u);
if [ "X$_myuid" != "X0" ]; then
	echo "ERROR: you SHOULD have had root privilege -- Exit";
	exit 1;
fi
