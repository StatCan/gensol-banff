.PHONY: all
all: static

ifeq ($(INIT_DIR_PATH),)
REL_ROOT=../../../build/linux/
include $(REL_ROOT)MakeDirPaths
endif

.PHONY: static
static: build/libnanoarrow.a 

build/libnanoarrow.a: build/Makefile
	cd build && $(MAKE)

build/Makefile:
	mkdir -p build
	cd build && cmake $(DIR_ARROW_SRC) -DCMAKE_POSITION_INDEPENDENT_CODE=ON

.PHONY: clean
clean: build/Makefile
	cd build && $(MAKE) clean
