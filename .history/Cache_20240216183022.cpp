#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <iomanip> // for setprecision

class Cache
{
private:
    struct CacheBlock
    {
        bool valid;
        unsigned long tag;
        unsigned long lruCounter;
    };

    std::vector<std::vector<CacheBlock>> cache;                   // 2D vector for cache blocks
    std::unordered_map<unsigned long, unsigned long> lruCounters; // To track LRU counters for each address
    unsigned int setSize;
    unsigned int associativity;
    unsigned int blockSize;
    unsigned int cacheSize;
    unsigned int totalAccesses;
    unsigned int hitCount;

public:
    Cache(unsigned int setSize, unsigned int associativity, unsigned int blockSize)
        : setSize(setSize), associativity(associativity), blockSize(blockSize)
    {

        // Calculate cache size
        cacheSize = setSize * associativity * blockSize;

        // Initialize cache blocks
        cache.resize(setSize, std::vector<CacheBlock>(associativity, {false, 0, 0}));

        // Initialize counters
        totalAccesses = 0;
        hitCount = 0;
    }

    bool access(unsigned long address)
    {
        totalAccesses++;

        unsigned long index = (address / blockSize) % setSize;
        unsigned long tag = address / (blockSize * setSize);

        // Check if the block is in the cache
        for (unsigned int i = 0; i < associativity; i++)
        {
            if (cache[index][i].valid && cache[index][i].tag == tag)
            {
                // Cache hit
                updateLRUCounters(index, i);
                hitCount++;
                return true;
            }
        }

        // Cache miss, find the least recently used block to replace
        unsigned int replaceIndex = findLRUBlockIndex(index);
        cache[index][replaceIndex] = {true, tag, 0};
        updateLRUCounters(index, replaceIndex);

        return false;
    }

    void updateLRUCounters(unsigned long index, unsigned long accessedBlock)
    {
        for (unsigned int i = 0; i < associativity; i++)
        {
            if (cache[index][i].valid)
            {
                if (i == accessedBlock)
                {
                    cache[index][i].lruCounter = 0; // Reset counter for the accessed block
                }
                else
                {
                    cache[index][i].lruCounter++; // Increment counters for other blocks
                }
                lruCounters[cache[index][i].tag] = cache[index][i].lruCounter;
            }
        }
    }

    unsigned int findLRUBlockIndex(unsigned long index)
    {
        unsigned int lruIndex = 0;
        unsigned long maxLRUCounter = 0;

        for (unsigned int i = 0; i < associativity; i++)
        {
            if (!cache[index][i].valid)
            {
                return i; // If an invalid block is found, use it
            }

            if (cache[index][i].lruCounter > maxLRUCounter)
            {
                maxLRUCounter = cache[index][i].lruCounter;
                lruIndex = i;
            }
        }

        return lruIndex;
    }

    double getHitRate() const
    {
        return static_cast<double>(hitCount) / totalAccesses;
    }
};
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    std::vector<std::vector<double>> hitRates;

    // Iterate over different cache configurations
    for (unsigned int size : {2048, 4096, 8192, 16384, 32768})
    {
        for (unsigned int associativity : {1, 2, 4, 8})
        {
            // Example usage
            Cache cache(size, associativity, 64); // Assuming 64-byte block size

            std::ifstream inputFile(argv[1]);
            if (!inputFile.is_open())
            {
                std::cerr << "Error opening input file." << std::endl;
                return 1;
            }

            // Read addresses from the file and simulate cache access
            unsigned long address;
            while (inputFile >> std::hex >> address)
            {
                cache.access(address);
            }

            inputFile.close();

            // Store the hit rate in the table
            hitRates.push_back({size, associativity, cache.getHitRate()});
        }
    }

    // Print the table header
    std::cout << std::setw(10) << "Size" << std::setw(10) << "Associativity" << std::setw(15) << "Hit Rate" << std::endl;

    // Print the hit rates in a formatted table
    for (const auto &row : hitRates)
    {
        std::cout << std::setw(10) << row[0] << std::setw(10) << row[1] << std::fixed << std::setprecision(4) << std::setw(15) << row[2] << std::endl;
    }

    return 0;
}