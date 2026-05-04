#pragma once
#include <vector>

struct SaveData {
    int  dbucks       = 0;
    int  totalKills   = 0;
    int  selectedSkin = 0;
    int  gliderIdx    = 0;
    std::vector<int> ownedSkins = {0};   // skin indices; 0 = default, always owned

    bool ownsSkin(int idx) const {
        for (int s : ownedSkins) if (s == idx) return true;
        return false;
    }
    void addSkin(int idx) {
        if (!ownsSkin(idx)) ownedSkins.push_back(idx);
    }
};

int  skinPrice(int idx);   // 0 = free, else 800-1200
bool saveGame (const SaveData& data);
bool loadGame (SaveData& data);
