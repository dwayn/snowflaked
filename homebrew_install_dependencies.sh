#!/bin/bash
brew update
pushd `brew versions check | egrep '0\.9\.8\b' | awk '{print $5}' | sed -E 's/^(.*)\/.*/\1/'`
`brew versions check | egrep '0\.9\.8\b' | awk '{print $2,$3,$4,$5}'`
popd
brew install libevent check pkgconfig libtool automake autoconf gperftools
