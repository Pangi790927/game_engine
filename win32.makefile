CXX := g++
CXX_LIBS :=
CXX_INCLUDE :=

all: do-common

include extern/extern.win32.makefile
include common.makefile
