.PHONY: diagnose clean static shared all

##### DEFINE FILES, PATHS, AND TARGETS ########################################
# name of final build output
LIB_NAME = glpk
TARGET_STATIC = lib$(LIB_NAME).a
TARGET_SHARED = lib$(LIB_NAME).so

# If paths undefined, call MakeDirPaths Make file
ifeq ($(INIT_DIR_PATH),)
REL_ROOT=../../../build/linux/
include $(REL_ROOT)MakeDirPaths
endif

# Default Values
include $(DIR_C_MAKE_INCLUDE)MakeDefaults

# object files are binary are stored in their own "build directories"
# define the relative path to build directories
DP_BIN = ./bin/
DP_OBJ = ./obj/

DIR_SRC_ROOT = $(DIR_GLPK_SRC)
# define source files/directories
#  LST_SRC_SUBDIR - list of source-code-containing subfolders
#  LST_FP_HEADER  - list of file paths to header files
#  LST_FP_SRCS    - list of file paths to source files
#  LST_SRC_NAMES  - list of names (with extensions) of source files
#  LST_FP_OBJS    - list of names (with extensions) of object files
#                   inferred from the LST_FP_SRCS
LST_SRC_SUBDIR = amd bflib cglib colamd env minisat misc proxy zlib
LST_FP_HEADER  = $(wildcard $(DIR_SRC_ROOT)*.h $(foreach fd, $(LST_SRC_SUBDIR), $(DIR_SRC_ROOT)$(fd)/*.h))
LST_FP_SRCS    = $(wildcard $(DIR_SRC_ROOT)*.c $(foreach fd, $(LST_SRC_SUBDIR), $(DIR_SRC_ROOT)$(fd)/*.c))
LST_SRC_NAMES  = $(notdir $(LST_FP_SRCS))
LST_FP_OBJS    = $(subst $(DIR_SRC_ROOT), $(DP_OBJ), $(LST_FP_SRCS:c=o)) # obj/xxx.o obj/folder/xxx .o

##### DEFINE COMPILER AND LINKER FLAGS ########################################
CC = gcc
CDEFS  = 
CFLAGS = $(DEFAULT_C_FLAGS) -c -m64 -fPIC $(CDEFS) 
LDOPTS = -shared -Bsymbolic -lc -lm 

# generate list of include flags (ex. "-I path/to/lib1 -I /path/lib2 ...")
LST_CF_INCLUDE = -I$(DIR_SRC_ROOT) $(foreach fd, $(LST_SRC_SUBDIR), -I$(DIR_SRC_ROOT)$(fd))
# append paths to imported external libraries
LST_DP_EXTRA_HEADER = 
LST_CF_INCLUDE     += $(addprefix -I, $(LST_DP_EXTRA_HEADER))

##### DEFINE CALLABLE BUILD TARGETS ###########################################
# default target: build static library
all: static

# build static library
static: $(DP_BIN)$(TARGET_STATIC)

# build shared library
shared: $(DP_BIN)$(TARGET_SHARED)

##### DEFINE BUILD STEPS ######################################################
# STATIC: Archive objects
$(DP_BIN)$(TARGET_STATIC): $(LST_FP_OBJS)
	mkdir -p $(@D)
	ar -crs $@ $(LST_FP_OBJS)

# SHARED: Link objects
$(DP_BIN)$(TARGET_SHARED): $(LST_FP_OBJS)
	mkdir -p $(@D)
	$(CC) -o $@ $(LST_FP_OBJS) $(LDOPTS)

# COMPILE PROPER source code
$(DP_OBJ)%.o: $(DIR_SRC_ROOT)%.c $(LST_FP_HEADER)
	mkdir -p $(@D)
	$(CC) -o $@ $< $(CFLAGS) $(LST_CF_INCLUDE)

##### DEFINE OTHER CALLABLE TARGETS ###########################################
diagnose:
	@echo "Makefile ($(LIB_NAME)): DIAGNOSTICS"
	@echo
	@echo "LIB_NAME: ${LIB_NAME}"
	@echo
	@echo "TARGET_STATIC: ${TARGET_STATIC}"
	@echo
	@echo "TARGET_SHARED: ${TARGET_SHARED}"
	@echo
	@echo "DP_BIN: ${DP_BIN}"
	@echo
	@echo "DP_OBJ: ${DP_OBJ}"
	@echo
	@echo "DIR_SRC_ROOT: ${DIR_SRC_ROOT}"
	@echo
	@echo "LST_SRC_SUBDIR: ${LST_SRC_SUBDIR}"
	@echo
	@echo "LST_FP_HEADER: ${LST_FP_HEADER}"
	@echo
	@echo "LST_FP_SRCS: ${LST_FP_SRCS}"
	@echo
	@echo "LST_SRC_NAMES: ${LST_SRC_NAMES}"
	@echo
	@echo "LST_FP_OBJS: ${LST_FP_OBJS}"
	@echo
	@echo "CC: ${CC}"
	@echo
	@echo "CFLAGS: ${CFLAGS}"
	@echo
	@echo "LDOPTS: ${LDOPTS}"
	@echo
	@echo "LST_CF_INCLUDE: ${LST_CF_INCLUDE}"
	
	
clean:
	-rm -r -v --one-file-system --preserve-root=all $(DP_OBJ)
	-rm -r -v --one-file-system --preserve-root=all $(DP_BIN)