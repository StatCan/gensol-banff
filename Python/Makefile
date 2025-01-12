# If paths undefined, call MakeDirPaths Make file
ifeq ($(INIT_DIR_PATH),)
REL_ROOT=../../../build/linux/
include $(REL_ROOT)MakeDirPaths
endif
##### DEFINE LIBRARY AND PROCEDURE PATHS ###################################### 
# variables like `DIR_<library_name>` are defined by the following Make file

# by default, only build libraries
.PHONY: all
all: python_package

.PHONY: python_package
python_package: clean validate_py_bin
	#echo "## Building Python package"

	cd $(DIR_PYTHON_PKG) && $(PY_BIN) -m build

.PHONY: clean
clean: 
	@echo ""
	@echo "## Cleaning Python package builds"
	-rm -r -v --one-file-system --preserve-root=all $(DIR_PYTHON_PKG)dist/

.PHONY: install
install: validate_py_bin uninstall
	$(PY_BIN) -m pip install banff[test] --find-links file://$(DIR_PYTHON_PKG)dist/

.PHONY: uninstall
uninstall: validate_py_bin
	$(PY_BIN) -m pip uninstall -y banff

.PHONY: validate_py_bin
validate_py_bin:
ifndef PY_BIN
	$(warning use `PY_BIN=<python-command>` in call to `make`)
	$(warning example `make install PY_BIN=python3.9`)
	$(error PY_BIN is not set)
endif