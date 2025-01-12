# READ ME
# This script tests any procedure build by calling it's `test_nls()` function, which
# prints a language-specific message to the console and returns an integer.  
# It depends only on the standard python package `ctypes`.  
# It requires the `path_to_shared_object` correctly point to the built (.dll or .so).  


# get path to procedure build, argument 1 to script
import sys
path_to_shared_obj=sys.argv[1]

# 1. configure shared library access
import ctypes
# load shared object
myCdll = ctypes.CDLL(path_to_shared_obj)

# 4. call test function
rc = myCdll.test_nls()
print("RC: " + rc.__str__())