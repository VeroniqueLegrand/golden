set breakpoint pending on
set env DYLD_INSERT_LIBRARIES /usr/lib/libgmalloc.dylib
set env MallocStackLoggingNoCompact 1 
b malloc_error_break
