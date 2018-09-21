#!/bin/bash
bash ./install_fedora_dependencies.sh

if [ "$?" -ne 0 ] ; then
	exit 1
fi

bash ./build.sh

if [ "$?" -ne 0 ] ; then
	exit 1
fi

bash ./build_coverage.sh

if [ "$?" -ne 0 ] ; then
	exit 1
fi

bash ./build_release.sh

if [ "$?" -ne 0 ] ; then
	exit 1
fi

bash <(curl -s https://codecov.io/bash) || echo "Codecov did not collect coverage reports""