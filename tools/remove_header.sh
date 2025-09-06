#!/bin/bash

find . -type f \( -name "*.c" -o -name "*.h" \) -print0 | while IFS= read -r -d '' file; do
    if grep -q "Tempest Foundation" "$file"; then
        echo "Removing header of: $file"

        sed -i '/SPDX-License-Identifier: LSL-1.4/d' "$file"
        sed -i '/\/\*/,/\*\//{ /-- BEGIN LICENSE HEADER --/,/-- END OF LICENSE HEADER --/d }' "$file"
    fi
done
