#include <Arduino.h>
#include <BGrid.h>
#include "BGridWords.h" // user-generated header with 9x2048 words

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }

  BGrid bgrid;
  bgrid.embedWords(BGRID_EMBEDDED_WORDS);

  const int level = 2;
  auto idx = bgrid.ddToBGrid(40.7128, -74.0060, level);
  Serial.print("Indices: "); Serial.println(idx.c_str());

  auto words = bgrid.encodeToWords(idx, "en");
  Serial.print("Words: "); Serial.println(words.c_str());

  auto backIdx = bgrid.decodeFromWords(words, "en");
  Serial.print("Back: "); Serial.println(backIdx.c_str());

  auto dd = bgrid.bGridToDD(backIdx);
  Serial.print("DD center: "); Serial.print(dd.first, 6); Serial.print(", "); Serial.println(dd.second, 6);
}

void loop() {
}
