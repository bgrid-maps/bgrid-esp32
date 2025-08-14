#include "BGrid.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cmath>

// Optional heavy dependencies (not used on typical Arduino builds). Guard with macros if needed.
#if !defined(ARDUINO)
  #include <nlohmann/json.hpp>
  #include <filesystem>
  using json = nlohmann::json;
  namespace fs = std::filesystem;
#endif

BGrid::BGrid() {}

void BGrid::embedWords(const std::map<std::string, std::vector<std::string>>& data) {
    // O(9*2048) copy; store by language key (e.g., "en", "es").
    wordlists_ = data;
}

void BGrid::loadLanguages(const std::string& directory) {
#if !defined(ARDUINO)
    // Desktop helper: iterate directory and parse JSON arrays of strings.
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.path().extension() == ".json") {
            std::string lang = entry.path().stem().string();
            // Normalize typical filenames like bip39-en -> en
            auto pos = lang.find("bip39-");
            if (pos == 0 && lang.size() > 6) lang = lang.substr(6);
            loadWordlist(entry.path().string(), lang);
        }
    }
#else
    (void)directory;
    // On Arduino, prefer embedWords() with a compiled-in header to avoid FS/JSON.
    throw std::runtime_error("loadLanguages not supported on Arduino; use embedWords().");
#endif
}

void BGrid::loadWordlist(const std::string& filepath, const std::string& language) {
#if !defined(ARDUINO)
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open wordlist file: " + filepath);
    }
    nlohmann::json j; file >> j;
    if (!j.is_array()) {
        throw std::runtime_error("Invalid wordlist format: " + filepath);
    }
    std::vector<std::string> words = j.get<std::vector<std::string>>();
    if (words.size() != 2048) {
        throw std::runtime_error("BIP39 list must have 2048 words: " + filepath);
    }
    wordlists_[language] = std::move(words);
#else
    (void)filepath; (void)language;
    throw std::runtime_error("loadWordlist not supported on Arduino; use embedWords().");
#endif
}

// Utility: split by comma or space; ignore empty tokens.
void BGrid::split(const std::string& s, std::vector<std::string>& out, const char delim1) {
    out.clear();
    std::string token;
    for (char c : s) {
        if (c == delim1 || c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            if (!token.empty()) { out.push_back(token); token.clear(); }
        } else {
            token.push_back(c);
        }
    }
    if (!token.empty()) out.push_back(token);
}

std::string BGrid::join(const std::vector<std::string>& parts, const char sep) {
    std::string out;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i) out.push_back(sep);
        out += parts[i];
    }
    return out;
}

std::string BGrid::ddToBGrid(double latitude, double longitude, int level) const {
    // Input domain clamp to avoid floating-point edge bleed on boundaries.
    if (level < 1 || level > 4) throw std::invalid_argument("level must be 1..4");
    if (latitude < -90.0) latitude = -90.0; if (latitude > 90.0) latitude = 90.0;
    if (longitude < -180.0) longitude = -180.0; if (longitude > 180.0) longitude = 180.0;

    // Normalize lon/lat to [0,1). Using nextafter to avoid 1.0 at max input.
    double x = (longitude + 180.0) / 360.0; // [0,1]
    double y = (latitude  +  90.0) / 180.0; // [0,1]
    const double eps = 1e-12;
    if (x >= 1.0) x = 1.0 - eps;
    if (y >= 1.0) y = 1.0 - eps;

    std::vector<std::string> indices;
    indices.reserve(level);
    for (int i = 1; i <= level; ++i) {
        // Alternating longitudinal/latitudinal partition counts per spec.
        const int lonSplits = (i % 2 == 1) ? 64 : 32;
        const int latSplits = (i % 2 == 1) ? 32 : 64;

        // Compute zero-based column/row via floor(x*lonSplits), floor(y*latSplits).
        const int col = static_cast<int>(std::floor(x * lonSplits));
        const int row = static_cast<int>(std::floor(y * latSplits));
        const int idx0 = row * lonSplits + col; // 0..2047
        const int idx1 = idx0 + 1;              // 1..2048 (human/BIP39 index)
        indices.push_back(std::to_string(idx1));

        // Residual refinement: update x,y to fractional part inside the selected subcell.
        x = x * lonSplits - col;
        y = y * latSplits - row;
    }
    return join(indices, ',');
}

std::pair<double, double> BGrid::bGridToDD(const std::string& bgridString) const {
    // Parse CSV indices; they are 1-based in [1..2048] per level.
    std::vector<std::string> parts; split(bgridString, parts, ',');
    if (parts.empty() || parts.size() > 4) throw std::invalid_argument("BGrid indices must have 1..4 parts");

    double minLon = -180.0, maxLon = 180.0;
    double minLat =  -90.0, maxLat =  90.0;

    for (size_t i = 0; i < parts.size(); ++i) {
        const int level = static_cast<int>(i) + 1;
        const int lonSplits = (level % 2 == 1) ? 64 : 32;
        const int latSplits = (level % 2 == 1) ? 32 : 64;

        // Convert 1-based Ni -> 0-based idx.
        int idx1 = std::stoi(parts[i]);
        if (idx1 < 1 || idx1 > 2048) throw std::out_of_range("Index out of [1..2048]");
        const int idx0 = idx1 - 1;

        const int col = idx0 % lonSplits;
        const int row = idx0 / lonSplits;

        const double lonWidth = (maxLon - minLon) / static_cast<double>(lonSplits);
        const double latHeight = (maxLat - minLat) / static_cast<double>(latSplits);

        minLon = minLon + col * lonWidth;
        maxLon = minLon + lonWidth;
        minLat = minLat + row * latHeight;
        maxLat = minLat + latHeight;
    }

    const double lon = (minLon + maxLon) * 0.5;
    const double lat = (minLat + maxLat) * 0.5;
    return {lat, lon};
}

std::string BGrid::encodeToWords(const std::string& bgridString, const std::string& language) const {
    auto it = wordlists_.find(language);
    if (it == wordlists_.end()) throw std::runtime_error("Language not loaded: " + language);
    const auto& words = it->second; // length 2048

    std::vector<std::string> parts; split(bgridString, parts, ',');
    if (parts.empty() || parts.size() > 4) throw std::invalid_argument("BGrid indices must have 1..4 parts");

    std::vector<std::string> out; out.reserve(parts.size());
    for (const auto& p : parts) {
        int idx1 = std::stoi(p);
        if (idx1 < 1 || idx1 > 2048) throw std::out_of_range("Index out of [1..2048]");
        out.push_back(words[static_cast<size_t>(idx1 - 1)]);
    }
    return join(out, ',');
}

std::string BGrid::decodeFromWords(const std::string& wordsCsv, const std::string& language) const {
    auto it = wordlists_.find(language);
    if (it == wordlists_.end()) throw std::runtime_error("Language not loaded: " + language);
    const auto& words = it->second;

    // Build word->index map on the fly; 2048 elements -> O(2048) per call acceptable for ESP32.
    std::map<std::string, int> rev;
    for (size_t i = 0; i < words.size(); ++i) rev[words[i]] = static_cast<int>(i) + 1; // store 1-based

    std::vector<std::string> tokens; split(wordsCsv, tokens, ',');
    if (tokens.empty() || tokens.size() > 4) throw std::invalid_argument("Expect 1..4 BIP39 words");

    std::vector<std::string> indices; indices.reserve(tokens.size());
    for (auto& w : tokens) {
        auto jt = rev.find(w);
        if (jt == rev.end()) throw std::runtime_error("Word not in language list: " + w);
        indices.push_back(std::to_string(jt->second));
    }
    return join(indices, ',');
}

double BGrid::getCellArea(int level) const {
    if (level < 1 || level > 4) throw std::invalid_argument("level must be 1..4");
    // Earth surface area approximation in km^2 and 2048^level cells.
    constexpr double earth_km2 = 510000000.0;
    double denom = 1.0;
    for (int i = 0; i < level; ++i) denom *= 2048.0;
    return earth_km2 / denom;
}
