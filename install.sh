#!/bin/sh

set -e

cd pylib
python setup.py build && sudo python setup.py install
cd ..

cd daemon
python setup.py build && sudo python setup.py install
cd ..

cd cli
python setup.py build && sudo python setup.py install
cd ..

cd qt
python setup.py build && sudo python setup.py install
cd ..
