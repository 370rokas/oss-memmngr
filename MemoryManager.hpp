#pragma once
#include "algorithms/BaseAlgo.hpp"
#include "algorithms/StaticEqual.hpp"
#include "algorithms/StaticVariable.hpp"
#include "algorithms/DynamicAlgo.hpp"
#include <memory>
#include <ctime>

/*
    pagrindine programos klase - veikia kaip interface'as tarp UI ir algoritmu
*/
class MemoryManager {
public:
    enum class Mode { StaticEqual, StaticVariable, Dynamic };

    std::vector<MemoryBlock> blocks;
    std::vector<LogEntry> log;
    bool initialized = false;
    int totalSize = 0;
    Mode mode = Mode::Dynamic;

    void initialize(int total, Mode m, int blockSz = 8, FitStrategy fit = FitStrategy::FirstFit) {
        totalSize = total;
        mode = m;
        nextId = 0;
        blocks.clear();
        log.clear();

        // Initializuoja pasirinkta algoritma
        switch (m) {
            case Mode::StaticEqual:    algo = std::make_unique<StaticEqual>(blockSz); break;
            case Mode::StaticVariable: algo = std::make_unique<StaticVariable>();     break;
            case Mode::Dynamic:        algo = std::make_unique<DynamicAlgo>(fit);     break;
        }
        algo->initialize(total, blocks);

        for (int i = 0; i < (int)blocks.size(); i++) {
            blocks[i].id = i;
            nextId = i + 1;
        }

        initialized = true;
    }

    /*
        alokuoja atminti procesui su duotu pavadinimu ir dydziu, grazina true jei pavyko
    */
    bool allocate(const std::string& name, int size) {
        auto indices = algo->findBlocks(size, blocks);
        if (indices.empty()) return false;

        if (mode == Mode::Dynamic) {
            int idx = indices[0];
            if (blocks[idx].size > size) {
                MemoryBlock rem;
                rem.id          = nextId++;
                rem.offset      = blocks[idx].offset + size;
                rem.size        = blocks[idx].size - size;
                rem.isFree      = true;
                rem.processName = "";
                blocks[idx].size = size;
                blocks.insert(blocks.begin() + idx + 1, rem);
            }
            blocks[idx].isFree      = false;
            blocks[idx].processName = name;
            log.push_back({ timestamp(), name, blocks[idx].id, blocks[idx].size, "Allocated" });

        } else {
            for (int idx : indices) {
                blocks[idx].isFree      = false;
                blocks[idx].processName = name;
                log.push_back({ timestamp(), name, blocks[idx].id, blocks[idx].size, "Allocated" });
            }
        }

        return true;
    }

    /*
        atlaisvina atminti procesui su duotu pavadinimu, grazina true jei pavyko
    */
    bool free(const std::string& name) {
        bool found = false;

        for (auto& b : blocks) {
            if (!b.isFree && b.processName == name) {
                log.push_back({ timestamp(), name, b.id, b.size, "Freed" });
                b.isFree      = true;
                b.processName = "";
                found = true;
            }
        }

        if (found && mode == Mode::Dynamic) {
            mergeAdjacentFree();
        }
        return found;
    }

    /*
        defragmentacija
    */
    void defragment() {
        std::vector<MemoryBlock> occupied, freeBlocks;
        for (const auto& b : blocks) {
            (b.isFree ? freeBlocks : occupied).push_back(b);
        }
 
        if (mode == Mode::Dynamic) {
            int freeTotal = 0;
            for (const auto& b : freeBlocks) freeTotal += b.size;
            blocks = occupied;

            if (freeTotal > 0) {
                blocks.push_back({ nextId++, 0, freeTotal, true, "" });
            }
        } else {
            blocks = occupied;
            blocks.insert(blocks.end(), freeBlocks.begin(), freeBlocks.end());
        }

        int off = 0;
        for (auto& b : blocks) {
            b.offset = off;
            off += b.size;
        }
        log.push_back({ timestamp(), "-", -1, 0, "Defragmented" });
    }

    void reset() {
        blocks.clear();
        log.clear();
        algo.reset();
        initialized = false;
        totalSize   = 0;
        nextId      = 0;
    }

private:
    std::unique_ptr<BaseAlgo> algo;
    int nextId = 0;

    std::string timestamp() {
        time_t now = time(nullptr);
        char buf[20];
        strftime(buf, sizeof(buf), "%H:%M:%S", localtime(&now));
        return buf;
    }

    /*
        sujungia gretimus laisvus blokus i viena
    */
    void mergeAdjacentFree() {
        for (int i = 0; i + 1 < (int)blocks.size(); ) {
            if (blocks[i].isFree && blocks[i + 1].isFree) {
                blocks[i].size += blocks[i + 1].size;
                blocks.erase(blocks.begin() + i + 1);
            } else {
                i++;
            }
        }
    }
};
