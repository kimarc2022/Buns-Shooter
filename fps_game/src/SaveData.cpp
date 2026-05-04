#include "SaveData.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <sys/stat.h>

static std::string savePath() {
    const char* home = getenv("HOME");
    std::string dir  = home ? std::string(home) + "/.local/share/battle_royale" : ".";
    mkdir(dir.c_str(), 0755);
    return dir + "/save.dat";
}

int skinPrice(int idx) {
    if (idx == 0) return 0;
    static const int kPrices[] = {800, 900, 1000, 1200, 900, 800, 1000, 800};
    return kPrices[idx % 8];
}

bool saveGame(const SaveData& data) {
    std::ofstream f(savePath());
    if (!f) return false;
    f << "dbucks "      << data.dbucks       << "\n"
      << "totalKills "  << data.totalKills   << "\n"
      << "selectedSkin "<< data.selectedSkin << "\n"
      << "gliderIdx "   << data.gliderIdx    << "\n"
      << "ownedSkins";
    for (int s : data.ownedSkins) f << " " << s;
    f << "\n";
    return true;
}

bool loadGame(SaveData& data) {
    std::ifstream f(savePath());
    if (!f) return false;
    std::string key;
    while (f >> key) {
        if      (key == "dbucks")        f >> data.dbucks;
        else if (key == "totalKills")    f >> data.totalKills;
        else if (key == "selectedSkin")  f >> data.selectedSkin;
        else if (key == "gliderIdx")     f >> data.gliderIdx;
        else if (key == "ownedSkins") {
            data.ownedSkins.clear();
            std::string line;
            std::getline(f, line);
            std::istringstream ss(line);
            int s;
            while (ss >> s) data.ownedSkins.push_back(s);
            if (data.ownedSkins.empty()) data.ownedSkins.push_back(0);
        }
    }
    return true;
}
