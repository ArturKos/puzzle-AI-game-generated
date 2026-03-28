#!/usr/bin/env python3
"""
Embed binary resource files as C++ arrays for compile-time inclusion.

Usage: embed_resources.py <output_dir> <res_base_dir> <file1> [file2 ...]

Generates:
  embedded_img_kids.cpp   - byte arrays for img_kids/*
  embedded_img_adults.cpp - byte arrays for img_adults/*
  embedded_sounds.cpp     - byte arrays for sounds/*
  embedded_registry.cpp   - resource map + lookup functions
  embedded_resources.h    - public API header
"""

import os
import sys

def sanitize(name):
    return ''.join(c if c.isalnum() else '_' for c in name)

def write_data_file(out_path, entries):
    """Write a .cpp file containing const arrays for each resource."""
    with open(out_path, 'w') as f:
        f.write('#include <cstddef>\n\n')
        for rel_path, abs_path in entries:
            sym = sanitize(rel_path)
            data = open(abs_path, 'rb').read()
            f.write(f'extern const unsigned char {sym}[] = {{\n')
            for i in range(0, len(data), 16):
                chunk = data[i:i+16]
                f.write('    ' + ', '.join(f'0x{b:02X}' for b in chunk) + ',\n')
            f.write('};\n')
            f.write(f'extern const size_t {sym}_size = {len(data)};\n\n')

def main():
    if len(sys.argv) < 3:
        print("Usage: embed_resources.py <output_dir> <res_base_dir> [files...]")
        sys.exit(1)

    output_dir = sys.argv[1]
    res_base = sys.argv[2]
    files = sys.argv[3:]

    os.makedirs(output_dir, exist_ok=True)

    # Group files by first directory component
    groups = {}  # group_name -> [(rel_path, abs_path)]
    all_entries = []

    for fpath in files:
        rel = os.path.relpath(fpath, res_base)
        parts = rel.split(os.sep)
        group = parts[0] if len(parts) > 1 else 'misc'
        groups.setdefault(group, []).append((rel, fpath))
        all_entries.append(rel)

    # Write per-group .cpp files
    for group, entries in groups.items():
        out_file = os.path.join(output_dir, f'embedded_{sanitize(group)}.cpp')
        write_data_file(out_file, entries)

    # Write registry .cpp
    reg_path = os.path.join(output_dir, 'embedded_registry.cpp')
    with open(reg_path, 'w') as f:
        f.write('#include "embedded_resources.h"\n')
        f.write('#include <unordered_map>\n')
        f.write('#include <algorithm>\n\n')

        # Extern declarations
        for group, entries in groups.items():
            for rel_path, _ in entries:
                sym = sanitize(rel_path)
                f.write(f'extern const unsigned char {sym}[];\n')
                f.write(f'extern const size_t {sym}_size;\n')
        f.write('\n')

        # Map
        f.write('static const std::unordered_map<std::string, EmbeddedResource>& getMap() {\n')
        f.write('    static const std::unordered_map<std::string, EmbeddedResource> m = {\n')
        for group, entries in groups.items():
            for rel_path, _ in entries:
                sym = sanitize(rel_path)
                # Normalize path separators to /
                key = rel_path.replace(os.sep, '/')
                f.write(f'        {{"{key}", {{{sym}, {sym}_size}}}},\n')
        f.write('    };\n')
        f.write('    return m;\n')
        f.write('}\n\n')

        # Lookup function
        f.write('const EmbeddedResource* getEmbeddedResource(const std::string& name) {\n')
        f.write('    auto& m = getMap();\n')
        f.write('    auto it = m.find(name);\n')
        f.write('    return (it != m.end()) ? &it->second : nullptr;\n')
        f.write('}\n\n')

        # List function
        f.write('std::vector<std::string> listEmbeddedResources(const std::string& prefix) {\n')
        f.write('    std::vector<std::string> result;\n')
        f.write('    for (auto& [key, val] : getMap()) {\n')
        f.write('        if (key.rfind(prefix, 0) == 0)\n')
        f.write('            result.push_back(key);\n')
        f.write('    }\n')
        f.write('    std::sort(result.begin(), result.end());\n')
        f.write('    return result;\n')
        f.write('}\n')

    # Write header
    hdr_path = os.path.join(output_dir, 'embedded_resources.h')
    with open(hdr_path, 'w') as f:
        f.write('#pragma once\n')
        f.write('#include <cstddef>\n')
        f.write('#include <string>\n')
        f.write('#include <vector>\n\n')
        f.write('struct EmbeddedResource {\n')
        f.write('    const unsigned char* data;\n')
        f.write('    size_t size;\n')
        f.write('};\n\n')
        f.write('const EmbeddedResource* getEmbeddedResource(const std::string& name);\n')
        f.write('std::vector<std::string> listEmbeddedResources(const std::string& prefix);\n')

    print(f"Embedded {len(all_entries)} resources into {output_dir}/")

if __name__ == '__main__':
    main()
