# BGrid Library for ESP32 (Arduino)

Hierarchical discrete global grid with BIP39 word encoding. Implements the BGrid system (2048 parcels per level, 1–4 levels) and bilingual word conversion using BIP39 lists.

## Features
- DD <-> BGrid conversion (levels 1..4) using alternating split factors per level.
- BIP39 words encode/decode in 9 languages (English, Spanish, French, Italian, Portuguese, Chinese, Japanese, Korean, Czech).
- Zero heap allocations in hot paths; uses small vectors and simple parsing.
- ESP32 focused; desktop helpers included for generating embedded word headers.

## Install
1. Copy the `BGrid` folder into your Arduino `libraries` directory (or use as a local library in Arduino IDE).
2. Open `File > Examples > BGrid > Basic`.

## Usage (indices only)
```cpp
#include <BGrid.h>

BGrid bgrid;

void setup() {
    Serial.begin(115200);
    auto idxStr = bgrid.ddToBGrid(40.7128, -74.0060, 3); // "N1,N2,N3"
    Serial.println(idxStr.c_str());
    auto dd = bgrid.bGridToDD(idxStr); // {lat, lon} center
    Serial.printf("%f,%f\n", dd.first, dd.second);
}
```

## Usage (words)
To use word encoding on Arduino, embed the 2048-word arrays per language at compile time:

1. Generate a header `BGridWords.h` from the JSON files in `src/bip39-wordlist` (provided in this repo). The header must define:
```cpp
// BGridWords.h
inline std::map<std::string, std::vector<std::string>> BGRID_EMBEDDED_WORDS = {
    {"en", {/* 2048 strings */}},
    {"es", {/* 2048 strings */}},
    // ... other languages
};
```
2. In your sketch:
```cpp
#include <BGrid.h>
#include "BGridWords.h"

void setup() {
    Serial.begin(115200);
    BGrid bgrid;
    bgrid.embedWords(BGRID_EMBEDDED_WORDS);
    auto idx = bgrid.ddToBGrid(40.7128, -74.0060, 2); // e.g., "1045,45"
    auto words = bgrid.encodeToWords(idx, "en");       // e.g., "little,airport"
    Serial.println(words.c_str());
    auto backIdx = bgrid.decodeFromWords(words, "en");
    auto dd = bgrid.bGridToDD(backIdx);
    Serial.printf("%f,%f\n", dd.first, dd.second);
}
```

On desktop (unit tests, tooling), you can load from JSON files directly:
```cpp
BGrid bgrid;
bgrid.loadLanguages("src/bip39-wordlist");
```

## Technical Notes
- Level i odd: lonSplits=64, latSplits=32. Level i even: lonSplits=32, latSplits=64.
- Index per level Ni (1..2048) = floor(y*latSplits)*lonSplits + floor(x*lonSplits) + 1.
- Decoding refines [minLon,maxLon]x[minLat,maxLat] and returns center.
- Area(level) ≈ 510e6 km² / 2048^level.
- Word indices are 1-based into the BIP39 arrays (per BIP39 convention).

## Examples
- Basic: DD <-> BGrid indices
- Words (after embedding): indices <-> words <-> DD

## License
MIT. See `LICENSE`.
