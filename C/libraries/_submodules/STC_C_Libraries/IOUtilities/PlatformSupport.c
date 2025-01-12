#include "PlatformSupport.h"

/* set_console_output_encoding
    Set the host console's output encoding.  
	This determines how printf statements are decoded and displayed to the user.  
    Currently only supports Windows, Linux seems to work without configuration.  */
UINT set_console_output_encoding(UINT encoding_id) {
#if defined(_WIN32) || defined(_WIN64)
	UINT old_cpid = GetConsoleOutputCP();
	SetConsoleOutputCP(encoding_id);
	return old_cpid;
#else
	return 0;
#endif
}