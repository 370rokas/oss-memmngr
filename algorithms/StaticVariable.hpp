#pragma once
#include "BaseAlgo.hpp"
#include <algorithm>

/*
    Statinis kintancio dydzio bloku algoritmas (n^2 dydzio blokai)
*/
class StaticVariable : public BaseAlgo {
public:
    void initialize(int totalSize, std::vector<MemoryBlock>& blocks) override {
        blocks.clear();

        int remaining = totalSize;
        int id = 0;
        int offset = 0;
        int blockSize = 2;
        
        while (remaining > 0) {
            // Ar tilps visas blokas? Jei ne, sukuriam bloka su kiek liko
            int size = (blockSize <= remaining) ? blockSize : remaining;

            blocks.push_back(
                { id++, offset, size, true, "" }
            );

            offset += size;
            remaining -= size;
            blockSize *= 2; // n^2 dydzio blokai
        }
    }

    /**
        randa blokus, i kuriuos priskirti procesa.
        blokai neprivalo eiti paeiliui. taciau bando rast best-fit,
        o jei neranda bando apjungt kelis didziausius blokus (greedy-fit)
    */
    std::vector<int> findBlocks(int size, const std::vector<MemoryBlock>& blocks) override {
        // best-fit - bando rast maziausia bloka, i kuri tilptu visas procesas
        int best = -1;
        for (int i = 0; i < (int)blocks.size(); i++) {
            if (blocks[i].isFree && blocks[i].size >= size) {
                if (best == -1 || blocks[i].size < blocks[best].size) {
                    best = i;
                }
            }
        }
        if (best != -1) return { best };

        // greedy-fit - bando apjungt didziausius blokus, kol tilps visas procesas
        std::vector<int> freeIdx;
        for (int i = 0; i < (int)blocks.size(); i++) {
            if (blocks[i].isFree) freeIdx.push_back(i);
        }

        // rikiuojam laisvus blokus pagal dydi mazejanciai
        std::sort(freeIdx.begin(), freeIdx.end(), [&](int a, int b) {
            return blocks[a].size > blocks[b].size;
        });

        std::vector<int> result;
        int remaining = size;

        for (int idx : freeIdx) {
            if (remaining <= 0) break;
            result.push_back(idx);
            remaining -= blocks[idx].size;
        }

        if (remaining > 0) return {}; // nepavyko rasti bloku i kuriuos tilptu
        return result;
    }
};
