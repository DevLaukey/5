#include <cstdint>
#include <memory>
#include <vector>
#include <unordered_map>
#include <algorithm>

// Define helper functions to extract address components
inline uint32_t getTag(uint64_t address, uint32_t tagBits)
{
    return address >> (blockOffsetBits + setIndexBits);
}

inline uint32_t getSetIndex(uint64_t address, uint32_t tagBits, uint32_t blockOffsetBits)
{
    return (address >> blockOffsetBits) & ((1 << (setIndexBits)) - 1);
}

inline uint32_t getBlockOffset(uint64_t address, uint32_t blockOffsetBits)
{
    return address & ((1 << blockOffsetBits) - 1);
}

class Cache
{
public:
    // Constructor with configurable parameters
    Cache(uint32_t size, uint32_t associativity, uint32_t blockBytes)
        : tagBits(std::ceil(std::log2(size / (associativity * blockBytes)))),
          blockOffsetBits(std::ceil(std::log2(blockBytes))),
          setIndexBits(tagBits + blockOffsetBits - std::ceil(std::log2(associativity))),
          sets(size / (associativity * blockBytes)),
          linesPerSet(associativity),
          lruCounts(sets * linesPerSet, 0)
    {
        blocks.resize(sets * linesPerSet);
        for (auto &block : blocks)
        {
            block.valid = false;
        }
    }

    // Access the cache
    bool access(uint64_t address)
    {
        uint32_t tag = getTag(address, tagBits);
        uint32_t setIndex = getSetIndex(address, tagBits, blockOffsetBits);

        // Find the matching block within the set
        int lruVictim = -1;
        uint32_t maxLruCount = 0;
        for (int i = 0; i < linesPerSet; ++i)
        {
            if (blocks[setIndex * linesPerSet + i].tag == tag && blocks[setIndex * linesPerSet + i].valid)
            {
                // Cache hit: update LRU and return true
                updateLru(setIndex, i);
                return true;
            }
            else if (lruCounts[setIndex * linesPerSet + i] > maxLruCount)
            {
                // Track candidate for LRU replacement
                lruVictim = setIndex * linesPerSet + i;
                maxLruCount = lruCounts[lruVictim];
            }
        }

        // Cache miss: load the block, update LRU, and return false
        if (lruVictim != -1)
        {
            // Perform actual data loading (not implemented here)
            blocks[lruVictim].valid = true;
            blocks[lruVictim].tag = tag;
            updateLru(setIndex, lruVictim % linesPerSet);
            return false;
        }

        // Cache is full and no eviction candidate found: report error
        throw std::runtime_error("Cache is full and no eviction candidate found");
    }

private:
    // Update LRU counts for a specific set
    void updateLru(uint32_t setIndex, int lineIndex)
    {
        for (int i = 0; i < linesPerSet; ++i)
        {
            lruCounts[setIndex * linesPerSet + i]++;
        }
        lruCounts[setIndex * linesPerSet + lineIndex] = 0;
    }

    struct Block
    {
        bool valid;
        uint32_t tag;
    };

    uint32_t tagBits;
    uint32_t blockOffsetBits;
    uint32_t setIndexBits;
    uint32_t sets;
    uint32_t linesPerSet;
    std::vector<Block> blocks;
    std::vector<uint32_t> lruCounts;
};

int main()
{
    // Get user input for cache size, associativity, block bytes, and trace file
    uint32_t size, associativity, blockBytes;
    std::string traceFile;

    std::cout << "Enter cache size (bytes): ";
    std::cin >> size;

    std::cout << "Enter associativity (1, 2, 4, or 8): ";
    std::cin >> associativity;

    std::cout << "Enter block size (bytes): ";
    std::cin >> blockBytes;

    std::cout << "Enter trace file name: ";
    std::cin >> traceFile;

    // Construct the cache
    Cache cache(size, associativity, blockBytes);

    // Open the trace file
    std::ifstream infile(traceFile);
    if (!infile.is_open())
    {
        std::cerr << "Error opening file: " << traceFile << std::endl;
        return 1;
    }

    // Process the trace file
    uint64_t address;
    uint32_t hits = 0, accesses = 0;
    while (infile >> std::hex >> address)
    {
        try
        {
            if (cache.access(address))
            {
                hits++;
            }
            accesses++;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            break;
        }
    }
    infile.close();

    // Calculate and print hit rate
    double hitRate = static_cast<double>(hits) / accesses;
    std::cout << "Hit rate: " << std::fixed << std::setprecision(4) << hitRate << std::endl;

    return 0;
}
