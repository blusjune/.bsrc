#!/bin/sh




echo "#>> update gighub source tree (add-commit-push)";

rm -f .tstamp.*;
touch .tstamp.$(tstamp);

git add -A
git commit -a
git push -u




