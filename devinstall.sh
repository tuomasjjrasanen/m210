#!/bin/sh

sudo stop m210d

set -e

cd m210c-qt
make
cd ..
python setup.py sdist
VERSION=`python setup.py --version`
cd dist
tar zxvf m210-$VERSION.tar.gz
cd m210-$VERSION
python setup.py build && sudo python setup.py install --record=installedfiles
sudo start m210d
