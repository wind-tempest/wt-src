// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        include/lib/kgeneral/kerrno/kerrno.h
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern int kerrno;

// Error codes (POSIX / C Standard)
#define KENONE        0   // No error information
#define KEPERM        1   // Operation not permitted
#define KENOENT       2   // No such file or directory
#define KESRCH        3   // No such process
#define KEINTR        4   // Interrupted system call
#define KEIO          5   // I/O error
#define KENXIO        6   // No such device or address
#define KE2BIG        7   // Argument list too long
#define KENOEXEC      8   // Exec format error
#define KEBADF        9   // Bad file number
#define KECHILD       10  // No child processes
#define KEAGAIN       11  // Try again
#define KENOMEM       12  // Out of memory
#define KEACCES       13  // Permission denied
#define KEFAULT       14  // Bad address
#define KENOTBLK      15  // Block device required
#define KEBUSY        16  // Device or resource busy
#define KEEXIST       17  // File exists
#define KEXDEV        18  // Cross-device link
#define KENODEV       19  // No such device
#define KENOTDIR      20  // Not a directory
#define KEISDIR       21  // Is a directory
#define KEINVAL       22  // Invalid argument
#define KENFILE       23  // File table overflow
#define KEMFILE       24  // Too many open files
#define KENOTTY       25  // Not a typewriter
#define KETXTBSY      26  // Text file busy
#define KEFBIG        27  // File too large
#define KENOSPC       28  // No space left on device
#define KESPIPE       29  // Illegal seek
#define KEROFS        30  // Read-only file system
#define KEMLINK       31  // Too many links
#define KEPIPE        32  // Broken pipe
#define KEDOM         33  // Math argument out of domain of func
#define KERANGE       34  // Math result not representable
#define KEDEADLK      35  // Resource deadlock would occur
#define KENAMETOOLONG 36  // File name too long
#define KENOLCK       37  // No record locks available
#define KENOSYS       38  // Function not implemented
#define KENOTEMPTY    39  // Directory not empty
#define KELOOP        40  // Too many symbolic links encountered

// Networking, IPC, and others (optional)
#define KEWOULDBLOCK     EAGAIN  // Operation would block
#define KENOMSG          42      // No message of desired type
#define KEIDRM           43      // Identifier removed
#define KECHRNG          44      // Channel number out of range
#define KEL2NSYNC        45      // Level 2 not synchronized
#define KEL3HLT          46      // Level 3 halted
#define KEL3RST          47      // Level 3 reset
#define KELNRNG          48      // Link number out of range
#define KEUNATCH         49      // Protocol driver not attached
#define KENOCSI          50      // No CSI structure available
#define KEL2HLT          51      // Level 2 halted
#define KEOVERFLOW       52      // Value too large for defined data type
#define KEBADMSG         53      // Bad message
#define KENOTRECOVERABLE 54      // State not recoverable
#define KEOWNERDEAD      55      // Previous owner died
#define KESTRPIPE        56      // Streams pipe error

// Thread support
#ifdef K__THREAD_LOCAL_ERRNO
#	undef errno
#	define errno (*K__errno_location())
extern int *
    K__errno_location (void);
#endif

#ifdef __cplusplus
}
#endif
