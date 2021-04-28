CXX := g++
CXX_LIBS :=
CXX_INCLUDE :=

all: do-common

include extern/extern.macos.makefile
include common.makefile
