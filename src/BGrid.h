#ifndef BGRID_H
#define BGRID_H

#include <string>
#include <vector>
#include <map>

// BGrid: Hierarchical discrete global grid with alternating split factors per level.
// Level i odd: lon splits=64, lat splits=32; level i even: lon=32, lat=64.
// Each level yields an index Ni in [0..2047] computed as row*lonSplits + col.
// Encoding DD->BGrid iteratively refines normalized x,y in [0,1).
// Decoding BGrid->DD iteratively refines bounding box and returns center.
class BGrid {
public:
    BGrid();

    // Load BIP39 wordlists from JSON files located in a directory.
    // Note: On Arduino ESP32, filesystem JSON parsing is heavy; prefer embedWords().
    void loadLanguages(const std::string& directory);

    // Embed pre-generated wordlists directly (call once at startup). The map key is ISO tag like "en".
    void embedWords(const std::map<std::string, std::vector<std::string>>& data);

    // Convert Decimal Degrees (lat in [-90,90], lon in [-180,180]) to CSV BGrid string: "N1,N2,..." (1-4 levels).
    std::string ddToBGrid(double latitude, double longitude, int level) const;

    // Convert BGrid CSV string (numbers or 1-based indices) back to (lat,lon) center.
    std::pair<double, double> bGridToDD(const std::string& bgridString) const;

    // Encode CSV indices -> CSV words in selected language (expects 1-based indices in string).
    std::string encodeToWords(const std::string& bgridString, const std::string& language) const;

    // Decode CSV words -> CSV indices (1-based). Accepts separators comma or space.
    std::string decodeFromWords(const std::string& words, const std::string& language) const;

    // Area in km^2 of a level cell given Earth surface ~510e6 km^2 and 2048^level partitions.
    double getCellArea(int level) const;

private:
    std::map<std::string, std::vector<std::string>> wordlists_; // language -> 2048 words
    void loadWordlist(const std::string& filepath, const std::string& language);
    static void split(const std::string& s, std::vector<std::string>& out, const char delim1=',');
    static std::string join(const std::vector<std::string>& parts, const char sep=',');
};

#endif
