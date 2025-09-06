// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        sys/fs/vfs/vfs.c
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

#include "vfs.h"

#include <fs/ext2/ext2.h>
#include <lib/kstdio/kstdio.h>
#include <lib/kstring/kstring.h>

// TODO: Implement a redirect to ext2 to not use hardcode calls to ext2

static char cwd_path[256] = "/";

void
    vfs_getcwd (char *out, ksize_t size) {
	kstrncpy(out, cwd_path, size);
}

void
    vfs_normalize_path (const char *path, char *out, ksize_t size) {
	if (!path || *path == '\0') {
		kstrncpy(out, "/", size);
		return;
	}

	// Require absolute path
	if (path[0] != '/') {
		kstrncpy(out, path, size);
		return;
	}

	char    tmp[256] = {0};
	ksize_t pos      = 0;
	tmp[pos++]       = '/';

	const char *p = path + 1;  // skip leading '/'
	while (*p) {
		// Skip repeated '/'
		while (*p == '/')
			++p;
		if (!*p)
			break;

		const char *start = p;
		while (*p && *p != '/')
			++p;
		ksize_t len = (ksize_t) (p - start);

		char comp[256];
		kmemcpy(comp, start, len);
		comp[len] = '\0';

		if (len == 1 && comp[0] == '.')
			continue;  // ignore '.'
		if (len == 2 && comp[0] == '.' && comp[1] == '.') {
			// Go up one directory
			if (pos > 1) {
				// remove trailing '/'
				if (tmp[pos - 1] == '/')
					--pos;
				// pop until previous '/'
				while (pos > 0 && tmp[pos - 1] != '/')
					--pos;
			}
			continue;
		}

		if (pos > 1 && tmp[pos - 1] != '/')
			tmp[pos++] = '/';
		if (pos + len >= sizeof(tmp))
			break;  // prevent overflow
		kmemcpy(&tmp[pos], comp, len);
		pos += len;
	}

	if (pos > 1 && tmp[pos - 1] == '/')
		--pos;
	tmp[pos] = '\0';

	kstrncpy(out, tmp, size);
}

void
    vfs_resolve (const char *path, char *out, ksize_t size) {
	char temp[256] = {0};
	if (!path || *path == '\0') {
		kstrcpy(temp, cwd_path);
	} else if (path[0] == '/') {
		// Absolute
		kstrcpy(temp, path);
	} else {
		if (kstrcmp(cwd_path, "/") == 0)
			ksnprintf(temp, sizeof(temp), "/%s", path);
		else
			ksnprintf(temp, sizeof(temp), "%s/%s", cwd_path, path);
	}
	vfs_normalize_path(temp, out, size);
}

int
    vfs_chdir (const char *path) {
	char resolved[256] = {0};
	vfs_resolve(path, resolved, sizeof(resolved));

	ext2_file_t dir;
	int         rc = ext2.open(resolved, &dir);
	if (rc != 0)
		return rc;  // Propagate ext2 error
	if (!(dir.inode.mode & 0x4000))
		return -1;  // Not a directory

	kstrcpy(cwd_path, resolved);
	return 0;
}

struct Vfs vfs = {
    .getcwd    = vfs_getcwd,
    .normalize = vfs_normalize_path,
    .resolve   = vfs_resolve,
    .chdir     = vfs_chdir,
};
