#include <BGrid.h>

BGrid bgrid;

void setup() {
    Serial.begin(115200);

    // Load languages from the bip39-wordlist directory
    try {
        bgrid.loadLanguages("src/bip39-wordlist");
        Serial.println("Languages loaded successfully.");
    } catch (const std::exception& e) {
        Serial.print("Error loading languages: ");
        Serial.println(e.what());
        return;
    }

    // Convert Decimal Degrees to BGrid
    String bgridStr = bgrid.ddToBGrid(40.7128, -74.0060, 3);
    Serial.println("BGrid: " + bgridStr);

    // Convert BGrid to Decimal Degrees
    auto coords = bgrid.bGridToDD(bgridStr);
    Serial.println("Coordinates: " + String(coords.first) + ", " + String(coords.second));

    // Encode to BIP39 words
    String words = bgrid.encodeToWords(bgridStr, "en");
    Serial.println("Words: " + words);

    // Decode from BIP39 words
    String decodedBGrid = bgrid.decodeFromWords(words, "en");
    Serial.println("Decoded BGrid: " + decodedBGrid);
}

void loop() {
    // Nothing to do here
}
