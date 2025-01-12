# define build paths
REL_ROOT=./build/linux/
include $(REL_ROOT)MakeDirPaths

.PHONY: all
all: c_procs python_package

.PHONY: c_procs
c_procs: c_proc_en c_proc_fr

.PHONY: c_proc_en
c_proc_en:
	cd $(DIR_C_MAKEFILE)   && $(MAKE) STC_LANG=en

.PHONY: c_proc_fr
c_proc_fr:
	cd $(DIR_C_MAKEFILE)   && $(MAKE) STC_LANG=fr

.PHONY: python_package
python_package: default_py_bin c_procs
	@echo ""
	@echo "## Building Python Package"
	cd $(DIR_PYTHON_ROOT)   && $(MAKE) PY_BIN=$(PY_BIN)

# "only" meaning without building dependencies
.PHONY: python_package_only
python_package_only: default_py_bin
	@echo ""
	@echo "## Building Python Package"
	cd $(DIR_PYTHON_ROOT)   && $(MAKE) PY_BIN=$(PY_BIN)

.PHONY: install
install: default_py_bin
	@echo ""
	@echo "## Installing Python Package"
	cd $(DIR_PYTHON_ROOT)   && $(MAKE) install  PY_BIN=$(PY_BIN)

.PHONY: uninstall
uninstall: default_py_bin
	@echo ""
	@echo "## Uninstalling Python Package"
	cd $(DIR_PYTHON_ROOT)   && $(MAKE) uninstall PY_BIN=$(PY_BIN)

.PHONY: test
test:
	cd $(DIR_TEST_ROOT) && ./run_pytest.sh /auto_pass

.PHONY: clean
clean: clean_c_proc_en clean_c_proc_fr clean_python

.PHONY: clean_c_proc_en
clean_c_proc_en:
	cd $(DIR_C_MAKEFILE)   && $(MAKE) clean_procs STC_LANG=en
	cd $(DIR_C_MAKEFILE)   && $(MAKE) clean_libs_internal STC_LANG=en
	cd $(DIR_C_MAKEFILE)   && $(MAKE) clean_libs_open_source STC_LANG=en

.PHONY: clean_c_proc_fr
clean_c_proc_fr:
	cd $(DIR_C_MAKEFILE)   && $(MAKE) clean_procs STC_LANG=fr
	cd $(DIR_C_MAKEFILE)   && $(MAKE) clean_libs_internal STC_LANG=fr
	cd $(DIR_C_MAKEFILE)   && $(MAKE) clean_libs_open_source STC_LANG=fr

.PHONY: clean_python
clean_python:
	@echo ""
	@echo "## Cleaning Python Package"
	cd $(DIR_PYTHON_ROOT)   && $(MAKE) clean

default_py_bin:
ifndef PY_BIN
	$(eval PY_BIN=python3.11)
	$(info Using default Python interpreter `(PY_BIN)`)
	$(info To override use `PY_BIN=<python-command>` in call to `make`)
endif