#!/usr/bin/env python3
import json
import os
from pathlib import Path

# Generates BGridWords.h from src/bip39-wordlist JSONs.
# Output: src/BGridWords.h at the repository root.

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / 'src' / 'bip39-wordlist'
OUT = ROOT / 'src' / 'BGridWords.h'

LANG_MAP = {
    'bip39-en.json': 'en',
    'bip39-es.json': 'es',
    'bip39-fr.json': 'fr',
    'bip39-it.json': 'it',
    'bip39-pt.json': 'pt',
    'bip39-zh.json': 'zh',
    'bip39-ja.json': 'ja',
    'bip39-ko.json': 'ko',
    'bip39-cs.json': 'cs',
}

def main():
    data = {}
    for fname, tag in LANG_MAP.items():
        p = SRC / fname
        with open(p, 'r', encoding='utf-8') as f:
            arr = json.load(f)
            if not isinstance(arr, list) or len(arr) != 2048:
                raise SystemExit(f"Invalid wordlist: {p}")
            data[tag] = arr

    with open(OUT, 'w', encoding='utf-8') as out:
        out.write('#pragma once\n')
        out.write('#include <map>\n#include <string>\n#include <vector>\n')
        out.write('inline std::map<std::string, std::vector<std::string>> BGRID_EMBEDDED_WORDS = {\n')
        first_lang = True
        for tag, words in data.items():
            if not first_lang:
                out.write(',\n')
            first_lang = False
            out.write(f'  {{"{tag}", {{')
            for i, w in enumerate(words):
                ws = w.replace('\\', r'\\').replace('"', r'\"')
                sep = ',' if i < len(words)-1 else ''
                out.write(f'"{ws}"{sep}')
            out.write('}}')
        out.write('\n};\n')
    print(f"Generated {OUT}")

if __name__ == '__main__':
    main()
