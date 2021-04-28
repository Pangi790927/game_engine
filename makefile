ifeq '$(findstring ;,$(PATH))' ';'
	detected_OS := Windows
else
	detected_OS := $(shell uname 2>/dev/null || echo Unknown)
	detected_OS := $(patsubst CYGWIN%,Cygwin,$(detected_OS))
	detected_OS := $(patsubst MSYS%,MSYS,$(detected_OS))
	detected_OS := $(patsubst MINGW%,MSYS,$(detected_OS))
endif

ifeq ($(detected_OS), Windows)
all: win32_build

endif
ifeq ($(detected_OS), Darwin)
all: macos_build

else
all: linux_build 

endif

linux_build:
	make -f linux.makefile

macos_build:
	make -f macos.makefile

win32_build:
	make -f win32.makefile
