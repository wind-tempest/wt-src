#!/bin/bash

headers_bad=false

while IFS= read -r -d '' file; do
    relpath=$(echo "$file" | sed 's|^\./||')

    header=$(head -n 25 "$file")

    if echo "$header" | grep -q "Copyright (C) 2025 Tempest Foundation"; then
        if ! echo "$header" | grep -q "File:.*$relpath"; then
            echo "Updating File: in header for $file"
            headers_bad=true

            tmpfile="$file.tmp"
            sed "s|^\s*\* File:.*| * File:        $relpath|" "$file" > "$tmpfile"
            mv "$tmpfile" "$file"
        fi
    else
        echo "Inserting header in: $file"
        headers_bad=true

        tmpfile="$file.tmp"

        cat > "$tmpfile" <<EOF
// SPDX-License-Identifier: LSL-1.4
/*
 * -- BEGIN LICENSE HEADER --
 * The Wind/Tempest Project
 *
 * File:        $relpath
 * Author(s):   Russian95 <russian95@tempestfoundation.org>
 *              (https://github.com/Russian95CrE)
 * Maintainer:  Tempest Foundation <development@tempestfoundation.org>
 * Link:        https://wtsrc.tempestfoundation.org
 *
 * Copyright (C) 2025 Tempest Foundation
 * Licensed under the Liberty Software License, Version 1.4
 * -- END OF LICENSE HEADER --
 */

EOF
        cat "$file" >> "$tmpfile"
        mv "$tmpfile" "$file"
    fi
done < <(find . -type f \( -name "*.c" -o -name "*.h" \) -print0)

if [ "$headers_bad" = false ]; then
    echo "Nah, everything is good."
fi
