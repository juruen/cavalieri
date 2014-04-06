#!/bin/bash

svn co http://googlemock.googlecode.com/svn/tags/release-1.7.0 googlemock
svn co http://svn.code.sf.net/p/libcds/code/tags/Release_1.5.0 cds

cd cds
svn patch ../cds-no-test.patch
