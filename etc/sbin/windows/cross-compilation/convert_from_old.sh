#!/bin/bash

# This is for anyone who was using the old scripts to build baka. Execute this once and only once to
#  convert to the new script. It will reuse existing build trees and download from source.

# Note: if stuff doesn't work after this, you may need to rebuild mxe (by deleting it of course)
#  I experienced such an issue I believe because they must be using absolute paths.

rm -r build;
mkdir -p build src;
mv *.{x86_64,i686} build/;
