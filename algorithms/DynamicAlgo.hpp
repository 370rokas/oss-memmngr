#pragma once
#include "BaseAlgo.hpp"

/*
    dinaminio algoritmo strategijos: first-fit, best-fit, worst-fit
*/
enum class FitStrategy { FirstFit, BestFit, WorstFit };

/*
    dinaminis algoritmas - priskira blokus pagal ju dydi ir strategija.
        - first-fit: pirmas laisvas blokas, i kuri tilps visas procesas
        - best-fit: maziausias laisvas blokas, i kuri tilps visas procesas
        - worst-fit: didziausias laisvas blokas, i kuri tilps visas procesas
*/
class DynamicAlgo : public BaseAlgo {
public:
    explicit DynamicAlgo(FitStrategy strategy) : strategy(strategy) {}

    void initialize(int totalSize, std::vector<MemoryBlock>& blocks) override {
        blocks.clear();
        blocks.push_back(
            { 0, 0, totalSize, true, "" }
        );
    }

    /*
        randa bloka, i kuri paskirti procesa
        funkcija iskviecia strategijos algoritma, kuri jei neranda
        tinkamo bloko, grazina -1
    */
    std::vector<int> findBlocks(int size, const std::vector<MemoryBlock>& blocks) override {
        int idx = -1;
        switch (strategy) {
            case FitStrategy::FirstFit: idx = firstFit(size, blocks);  break;
            case FitStrategy::BestFit:  idx = bestFit(size, blocks);   break;
            case FitStrategy::WorstFit: idx = worstFit(size, blocks);  break;
        }
        if (idx == -1) return {};
        return { idx };
    }

private:
    // strategija, pagal kuri priskiriami blokai
    FitStrategy strategy;

    // first-fit algoritmas - pirmas laisvas blokas, i kuri tilps visas procesas
    static int firstFit(int size, const std::vector<MemoryBlock>& blocks) {
        for (int i = 0; i < (int)blocks.size(); i++) {
            if (blocks[i].isFree && blocks[i].size >= size) return i;
        }

        return -1;
    }

    // best-fit algoritmas - maziausias laisvas blokas, i kuri tilps visas procesas
    static int bestFit(int size, const std::vector<MemoryBlock>& blocks) {
        int best = -1;
        for (int i = 0; i < (int)blocks.size(); i++) {
            if (blocks[i].isFree && blocks[i].size >= size) {
                if (best == -1 || blocks[i].size < blocks[best].size) best = i;
            }
        }

        return best;
    }

    // worst-fit algoritmas - didziausias laisvas blokas, i kuri tilps visas procesas
    static int worstFit(int size, const std::vector<MemoryBlock>& blocks) {
        int worst = -1;
        for (int i = 0; i < (int)blocks.size(); i++) {
            if (blocks[i].isFree && blocks[i].size >= size) {
                if (worst == -1 || blocks[i].size > blocks[worst].size) worst = i;
            }
        }

        return worst;
    }
};
