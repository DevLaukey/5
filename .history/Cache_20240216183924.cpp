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

    // Print the table header
    std::cout << std::setw(13) << "Size (bytes)" << std::setw(15) << "1-way 1st loop" << std::setw(15) << "1-way 2nd loop"
              << std::setw(15) << "2-way 1st loop" << std::setw(15) << "2-way 2nd loop" << std::setw(15) << "4-way 1st loop"
              << std::setw(15) << "4-way 2nd loop" << std::setw(15) << "8-way 1st loop" << std::setw(15) << "8-way 2nd loop" << std::endl;

    // Iterate over different cache configurations
    for (unsigned int size : {2048, 4096, 8192, 16384, 32768})
    {
        std::cout << std::setw(13) << size; // Print size in the first column

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

            unsigned long totalHits = 0;
            unsigned long totalAccesses = 0;

            // Read addresses from the file and simulate cache access
            unsigned long address;
            while (inputFile >> std::hex >> address)
            {
                totalAccesses++;
                if (cache.access(address))
                {
                    totalHits++;
                }
            }

            inputFile.close();

            // Calculate and print the hit rate for the specific associativity
            double hitRate = static_cast<double>(totalHits) / totalAccesses;
            std::cout << std::setw(15) << std::fixed << std::setprecision(4) << hitRate;
        }

        std::cout << std::endl; // Move to the next line after printing hit rates for all associativities
    }

    return 0;
}