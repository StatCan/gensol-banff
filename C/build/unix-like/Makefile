##### DEFINE LIBRARY AND PROCEDURE PATHS ###################################### 
# variables like `DIR_<library_name>` are defined by the following Make file
REL_ROOT=../../../build/linux/
include $(REL_ROOT)MakeDirPaths

# by default, only build libraries
.PHONY: all
all: libs_open_source libs_internal determin donorimp editstat errorloc estimato massimpu outlier prorate verifyed

.PHONY: test
test: determin_test donorimp_test editstat_test errorloc_test estimato_test massimpu_test outlier_test prorate_test verifyed_test

.PHONY: determin_test
determin_test:
	@echo ""
	@echo "## Testing determin"
	export BANFF_DEBUG_STATS=1
	cd $(DIR_DETERMIN)   && $(MAKE) test
	
.PHONY: donorimp_test
donorimp_test:
	@echo ""
	@echo "## Testing donorimp"
	export BANFF_DEBUG_STATS=1
	cd $(DIR_DONORIMP)   && $(MAKE) test

.PHONY: editstat_test
editstat_test:
	@echo ""
	@echo "## Testing editstat"
	cd $(DIR_EDITSTAT)   && $(MAKE) test

.PHONY: errorloc_test
errorloc_test:
	@echo ""
	@echo "## Testing errorloc"
	cd $(DIR_ERRORLOC)   && $(MAKE) test

.PHONY: estimato_test
estimato_test:
	@echo ""
	@echo "## Testing estimato"
	cd $(DIR_ESTIMATO)   && $(MAKE) test

.PHONY: massimpu_test
massimpu_test:
	@echo ""
	@echo "## Testing massimpu"
	cd $(DIR_MASSIMPU)   && $(MAKE) test

.PHONY: outlier_test
outlier_test:
	@echo ""
	@echo "## Testing outlier"
	cd $(DIR_OUTLIER)   && $(MAKE) test

.PHONY: prorate_test
prorate_test:
	@echo ""
	@echo "## Testing prorate"
	cd $(DIR_PRORATE)   && $(MAKE) test

.PHONY: verifyed_test
verifyed_test:
	@echo ""
	@echo "## Testing verifyed"
	cd $(DIR_VERIFYED)   && $(MAKE) test
	
	
# build determin
.PHONY: determin
determin: libs_open_source libs_internal
	@echo ""
	@echo "## Building determin"
	cd $(DIR_DETERMIN)   && $(MAKE)
	
# build donorimp
.PHONY: donorimp
donorimp: libs_open_source libs_internal
	@echo ""
	@echo "## Building donorimp"
	cd $(DIR_DONORIMP)   && $(MAKE)

# build editstat
.PHONY: editstat
editstat: libs_open_source libs_internal
	@echo ""
	@echo "## Building editstat"
	cd $(DIR_EDITSTAT)   && $(MAKE)

# build errorloc
.PHONY: errorloc
errorloc: libs_open_source libs_internal
	@echo ""
	@echo "## Building errorloc"
	cd $(DIR_ERRORLOC)   && $(MAKE)

# build estimato
.PHONY: estimato
estimato: libs_open_source libs_internal
	@echo ""
	@echo "## Building estimato"
	cd $(DIR_ESTIMATO)   && $(MAKE)

# build massimpu
.PHONY: massimpu
massimpu: libs_open_source libs_internal
	@echo ""
	@echo "## Building massimpu"
	cd $(DIR_MASSIMPU)   && $(MAKE)

# build outlier
.PHONY: outlier
outlier: libs_open_source libs_internal
	@echo ""
	@echo "## Building outlier"
	cd $(DIR_OUTLIER)   && $(MAKE)

# build prorate
.PHONY: prorate
prorate: libs_open_source libs_internal
	@echo ""
	@echo "## Building prorate"
	cd $(DIR_PRORATE)   && $(MAKE)

# build verifyed
.PHONY: verifyed
verifyed: libs_open_source libs_internal
	@echo ""
	@echo "## Building verifyed"
	cd $(DIR_VERIFYED)   && $(MAKE)
	
# build all libraries as static libraries
.PHONY: libs_internal
libs_internal: 
	@echo ""
	@echo "## Building BanffAPI"
	cd $(DIR_BANFFAPI)   && $(MAKE) static
	
	@echo ""
	@echo "## Building GLPK"
	cd $(DIR_GLPK)       && $(MAKE) static
	
	@echo ""
	@echo "## Building CommonAPI"
	cd $(DIR_COMMONAPI)  && $(MAKE) static
	
	@echo ""
	@echo "## Building LPInterface"
	cd $(DIR_LPINTERFACE) && $(MAKE) static
	
	@echo ""
	@echo "## Building IOUtilities"
	cd $(DIR_IOUTIL)    && $(MAKE) static
	
.PHONY: libs_open_source
libs_open_source:
	@echo ""
	@echo "## Building Jansson"
	cd $(DIR_JANSSON)   && $(MAKE) static
	@echo ""
	@echo "## Building Arrow"
	cd $(DIR_ARROW)   && $(MAKE)
	
.PHONY: diagnose
diagnose:
	@echo ""
	@echo "## Diagnosing BanffAPI"
	cd $(DIR_BANFFAPI)   && $(MAKE) diagnose
	@echo "DIR_BANFFAPI    $(DIR_BANFFAPI)"
	
	@echo "DIR_COMMONAPI   $(DIR_COMMONAPI)"
	
	cd $(DIR_GLPK)   && $(MAKE) diagnose
	@echo "DIR_GLPK        $(DIR_GLPK)"
	
	@echo "DIR_LPINTERFACE $(DIR_LPINTERFACE)"
	
	@echo "DIR_IOUTIL    $(DIR_IOUTIL)"
	
	@echo "DIR_DETERMIN    $(DIR_DETERMIN)"
	
	@echo "DIR_DONORIMP    $(DIR_DONORIMP)"
	
	@echo "DIR_EDITSTAT     $(DIR_EDITSTAT)"
	
	@echo "DIR_ERRORLOC    $(DIR_ERRORLOC)"
	
	@echo "DIR_ESTIMATO    $(DIR_ESTIMATO)"
	
	@echo "DIR_MASSIMPU    $(DIR_MASSIMPU)"
	
	@echo "DIR_OUTLIER     $(DIR_OUTLIER)"
	
	@echo "DIR_PRORATE     $(DIR_PRORATE)"
	
	@echo "DIR_VERIFYED     $(DIR_VERIFYED)"
	
# clean all libraries and procedures/modules
.PHONY: clean
clean: clean_libs_open_source clean_libs_internal clean_procs

.PHONY: clean_procs
clean_procs:
	@echo ""
	@echo "## Cleaning determin"
	cd $(DIR_DETERMIN) && $(MAKE) clean
	
	@echo ""
	@echo "## Cleaning donorimp"
	cd $(DIR_DONORIMP) && $(MAKE) clean
	
	@echo ""
	@echo "## Cleaning editstat"
	cd $(DIR_EDITSTAT) && $(MAKE) clean
	
	@echo ""
	@echo "## Cleaning errorloc"
	cd $(DIR_ERRORLOC) && $(MAKE) clean
	
	@echo ""
	@echo "## Cleaning estimato"
	cd $(DIR_ESTIMATO) && $(MAKE) clean
	
	@echo ""
	@echo "## Cleaning massimpu"
	cd $(DIR_MASSIMPU) && $(MAKE) clean
	
	@echo ""
	@echo "## Cleaning outlier"
	cd $(DIR_OUTLIER) && $(MAKE) clean
	
	@echo ""
	@echo "## Cleaning prorate"
	cd $(DIR_PRORATE) && $(MAKE) clean
	
	@echo ""
	@echo "## Cleaning verifyed"
	cd $(DIR_VERIFYED) && $(MAKE) clean


.PHONY: clean_libs_internal
clean_libs_internal:
	@echo ""
	@echo "## Cleaning BanffAPI"
	cd $(DIR_BANFFAPI)   && $(MAKE) clean
	
	@echo ""
	@echo "## Cleaning CommonAPI"
	cd $(DIR_COMMONAPI)  && $(MAKE) clean
	
	@echo ""
	@echo "## Cleaning GLPK"
	cd $(DIR_GLPK)       && $(MAKE) clean
	
	@echo ""
	@echo "## Cleaning LPInterface"
	cd $(DIR_LPINTERFACE) && $(MAKE) clean
	
	@echo ""
	@echo "## Cleaning IOUtilities"
	cd $(DIR_IOUTIL) && $(MAKE) clean
	
.PHONY: clean_libs_open_source
clean_libs_open_source:
	@echo ""
	@echo "## Cleaning Arrow"
	cd $(DIR_ARROW)   && $(MAKE) clean
	
	@echo ""
	@echo "## Cleaning Jansson"
	cd $(DIR_JANSSON)   && $(MAKE) clean

