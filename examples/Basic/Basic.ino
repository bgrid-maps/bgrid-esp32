#include <Arduino.h>
#include <BGrid.h>

// Minimal embedded wordlist example with two words only to demonstrate wiring.
// In production, generate/compile the full 2048-word arrays for 9 languages.
static std::map<String, std::vector<String>> not_used; // Placeholder (Arduino String incompatible with std::string)

static std::map<std::string, std::vector<std::string>> kEmbeddedWords = {
  {"en", { /* 2048 words to be compiled in via a generated header */ }},
};

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }
  Serial.println("BGrid Basic Example");

  BGrid bgrid;
  // For Arduino, embed words (this object is empty here, see README for generation step)
  bgrid.embedWords(kEmbeddedWords);

  // Encode DD -> BGrid indices
  const double lat = 40.7128; // NYC
  const double lon = -74.0060;
  const int level = 3;
  String idxStr = String(bgrid.ddToBGrid(lat, lon, level).c_str());
  Serial.print("BGrid indices: "); Serial.println(idxStr);

  // Decode indices -> DD
  auto dd = bgrid.bGridToDD(std::string(idxStr.c_str()));
  Serial.print("Center DD: "); Serial.print(dd.first, 6); Serial.print(", "); Serial.println(dd.second, 6);

  // Words encode/decode requires a loaded language with 2048 words
  // String words = String(bgrid.encodeToWords(std::string(idxStr.c_str()), "en").c_str());
  // Serial.print("Words: "); Serial.println(words);
  // String back = String(bgrid.decodeFromWords(std::string(words.c_str()), "en").c_str());
  // Serial.print("Back indices: "); Serial.println(back);
}

void loop() {
}
