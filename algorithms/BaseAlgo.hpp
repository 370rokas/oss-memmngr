#pragma once
#include <string>
#include <vector>

/*
    Struktura vaizduojanti atminties bloka.
*/
struct MemoryBlock {
    int id;
    int offset;
    int size;
    bool isFree;
    std::string processName;
};

/*
    Struktura logginimui
*/
struct LogEntry {
    std::string timestamp;
    std::string processName;
    int blockId;
    int size;
    std::string action;
};

/*
    Bendra klase algoritmu implementacijom
*/
class BaseAlgo {
public:
    /*
        size - reikiamas dydis
        blocks - dabartine atminties busena

        Grazina: bloku ids, prie kuriu priskirti procesa. Tuscias vektorius - atminties neuztenka.
    */
    virtual std::vector<int> findBlocks(int size, const std::vector<MemoryBlock>& blocks) = 0;

    /*
        totalSize - visos atminties dydis
        blocks - dabartine atminties busena
    */
    virtual void initialize(int totalSize, std::vector<MemoryBlock>& blocks) = 0;

    /*
        destruktorius
    */
    virtual ~BaseAlgo() = default;
};
