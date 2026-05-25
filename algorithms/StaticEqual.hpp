#pragma once
#include "BaseAlgo.hpp"

/*
    Statinis vienodo dydzio bloku algoritmas
*/
class StaticEqual : public BaseAlgo {
public:
    explicit StaticEqual(int blockSize) : blockSize(blockSize) {}

    void initialize(int totalSize, std::vector<MemoryBlock>& blocks) override {
        blocks.clear();

        // pilno dydzio blokai
        int count = totalSize / blockSize;
        for (int i = 0; i < count; i++) {
            blocks.push_back(
                { i, i * blockSize, blockSize, true, "" }
            );
        }

        // galinis blokas, jei liko laisvos atminties
        int rem = totalSize % blockSize;
        if (rem > 0) {
            blocks.push_back(
                { count, count * blockSize, rem, true, "" }
            );
        }
    }

    /*
        randa blokus, i kuriuos priskirti procesa.
        blokai neprivalo eiti paeiliui
    */
    std::vector<int> findBlocks(int size, const std::vector<MemoryBlock>& blocks) override {
        int needed = (size + blockSize - 1) / blockSize;
        std::vector<int> result;

        for (int i = 0; i < (int)blocks.size() && (int)result.size() < needed; i++) {
            if (blocks[i].isFree) {
                result.push_back(i);
            }
        }

        if ((int)result.size() < needed) return {}; // nepakanka laisvu bloku

        return result;
    }

private:
    int blockSize;
};
