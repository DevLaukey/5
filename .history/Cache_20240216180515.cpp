#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstring>

class Cache
{
private:
    // Define cache parameters
    int size; // in bytes
    int associativity;
    int block_size;
    int sets;
    std::vector<std::vector<bool>> valid;      // Valid bit for each block in each set
    std::vector<std::vector<int>> lru_counter; // LRU counter for each block in each set

public:
    Cache(int size, int associativity, int block_size) : size(size), associativity(associativity), block_size(block_size)
    {
        // Calculate the number of sets
        sets = size / (associativity * block_size);

        // Initialize cache state
        valid.assign(sets, std::vector<bool>(associativity, false));
        lru_counter.assign(sets, std::vector<int>(associativity, 0));
    }

    bool access(unsigned long address)
    {
        // Simulate cache behavior for the given address
        int set_index = (address / block_size) % sets;
        int block_offset = address % block_size;

        // Check if the block is in the cache
        for (int i = 0; i < associativity; ++i)
        {
            if (valid[set_index][i] && lru_counter[set_index][i] == 0)
            {
                // Cache hit
                updateLRU(set_index, i);
                return true;
            }
        }

        // Cache miss
        int victim_index = findLRUVictim(set_index);
        valid[set_index][victim_index] = true;
        updateLRU(set_index, victim_index);
        return false;
    }

private:
    void updateLRU(int set_index, int used_index)
    {
        // Update LRU counters based on the accessed block
        for (int i = 0; i < associativity; ++i)
        {
            if (valid[set_index][i] && i != used_index)
            {
                lru_counter[set_index][i]++;
            }
        }
        lru_counter[set_index][used_index] = 0;
    }

    int findLRUVictim(int set_index)
    {
        // Find the index of the block with the highest LRU counter
        int max_lru = -1;
        int victim_index = 0;

        for (int i = 0; i < associativity; ++i)
        {
            if (!valid[set_index][i])
            {
                // Found an invalid block, use it as victim
                return i;
            }

            if (lru_counter[set_index][i] > max_lru)
            {
                max_lru = lru_counter[set_index][i];
                victim_index = i;
            }
        }

        return victim_index;
    }
};

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    const char *inputFileName = argv[1];
    std::ifstream inputFile(inputFileName);

    if (!inputFile.is_open())
    {
        std::cerr << "Error: Unable to open file " << inputFileName << std::endl;
        return 1;
    }

    std::vector<std::pair<int, int>> cacheConfigurations = {
        {1, 1}, {1, 2}, {2, 1}, {2, 2}, {4, 1}, {4, 2}, {8, 1}, {8, 2}};

    std::vector<int> sizes = {2048, 4096, 8192, 16384, 32768};

    // Print header
    std::cout << "size (bytes),1-way 1st loop,1-way 2nd loop,2-way 1st loop,2-way 2nd loop,4-way 1st loop,4-way 2nd loop,8-way 1st loop,8-way 2nd loop" << std::endl;

    for (const auto &size : sizes)
    {
        std::cout << size;

        // Initialize the isComposite array
        bool *isComposite = new bool[size + 1];
        std::memset(isComposite, 0, (size + 1) * sizeof(bool));

        for (const auto &config : cacheConfigurations)
        {
            int associativity = config.first;
            int block_size = config.second;

            // Initialize the cache with the desired parameters
            Cache cache(size, associativity, block_size);

            unsigned long hits = 0;
            unsigned long accesses = 0;

            long upperBoundSquareRoot = static_cast<long>(std::sqrt(size));

            // Simulate sieve of Eratosthenes algorithm with cache access checks
            for (long m = 2; m <= upperBoundSquareRoot; m++)
            {
                // check for hit on read of isComposite[m]
                if (cache.access(reinterpret_cast<unsigned long>(&isComposite[m])))
                    hits++;
                accesses++;

                if (!isComposite[m])
                {
                    for (long k = m * m; k <= size; k += m)
                    {
                        // check for hit on read of isComposite[k]
                        if (cache.access(reinterpret_cast<unsigned long>(&isComposite[k])))
                            hits++;
                        accesses++;

                        isComposite[k] = true;

                        // check for hit on write of isComposite[k]
                        if (cache.access(reinterpret_cast<unsigned long>(&isComposite[k])))
                            hits++;
                        accesses++;
                    }
                }
            }

            // Output hit rate for the current configuration
            double hitRate = (accesses > 0) ? static_cast<double>(hits) / accesses : 0.0;
            std::cout << "," << hitRate;
        }

        // Cleanup allocated memory
        delete[] isComposite;

        std::cout << std::endl;
    }

    return 0;
}
