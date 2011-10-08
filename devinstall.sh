#!/bin/sh

stop m210d
VERSION=`python setup.py --version`
cat dist/m210-$VERSION/installedfiles | xargs rm -rf
rm MANIFEST
set -e

cd m210c-qt
make
cd ..
rm -rf dist
python setup.py sdist
cd dist
tar zxvf m210-$VERSION.tar.gz
cd m210-$VERSION
python setup.py build && python setup.py install --record=installedfiles
start m210d
