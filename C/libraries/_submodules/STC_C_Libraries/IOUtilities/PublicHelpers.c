#include "PublicHelpers.h"
#include "SUtil.h"  // used in `free_memory()`
#include <stdio.h>
#include "PlatformSupport.h"

/* flush_std_buffers
	- exposes fflush for stdout and stderr buffers
	- caller: use this after completion/exception of procedure
	*/
EXPORTED_FUNCTION void flush_std_buffers() {
	fflush(stdout);
	fflush(stderr);
	return;
}

/* free_memory 
	- Exposes a memory freeing function for use by calling application 
	In Python, output dataset parameters are created using `ctypes.c_char_p()`, 
	and subsequently populated in C.  To free the content produced by C, 
	pass the same pointer to this function.  */
EXPORTED_FUNCTION void free_memory(void* ptr) {
	SUtil_FreeMemory(ptr);
}

/* test_nls
	Print a message which demonstrates Native Language Support (NLS).  
	For any supported language, the message should be printed to the console
	in the correct language, with special characters appearing correctly.  */
EXPORTED_FUNCTION int test_nls() {
	UINT saved_codepage = set_console_output_encoding(UTF8_ENCODING_ID);
	printf(IO_TEST_MESSAGE);
	printf("\n");
	set_console_output_encoding(saved_codepage);
	return 0;
}