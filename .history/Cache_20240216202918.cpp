#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <unordered_map>
#include <istream>
#include <string>

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

    bool access(int address)
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
    void resetCacheState()
    {
        // Reset the cache state for the next run
        valid.assign(sets, std::vector<bool>(associativity, false));
        lru_counter.assign(sets, std::vector<int>(associativity, 0));
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

int main()
{
    // Define cache configurations
    std::vector<std::tuple<int, int>> cacheConfigurations = {{2048, 1}, {2048, 2}, {4096, 1}, {4096, 2}, {8192, 1}, {8192, 2}, {16384, 1}, {16384, 2}};

    // Print header
    std::cout << "size (bytes)\t1-way 1st loop\t1-way 2nd loop\t2-way 1st loop\t2-way 2nd loop\t4-way 1st loop\t4-way 2nd loop\t8-way 1st loop\t8-way 2nd loop" << std::endl;

    // Iterate through cache configurations
    for (const auto &config : cacheConfigurations)
    {
        int size = std::get<0>(config);
        int associativity = std::get<1>(config);

        // Initialize the cache with the desired parameters
        Cache cache(size, associativity, 16);

        // Print size (bytes)
        std::cout << size << "\t";

        // Run the two loops
        for (int run = 1; run <= 2; ++run)
        {
            unsigned long hits = 0;
            unsigned long accesses = 0;

            // Assuming isComposite is a boolean array
            std::vector<char> isComposite(size + 1, false);

            // Simulate cache behavior during Sieve of Eratosthenes algorithm
            for (long m = 2; m <= size; m++)
            {
                // check for hit on read of isComposite[m]
                if (cache.access(reinterpret_cast<uintptr_t>(&isComposite[m])))
                    hits++;
                accesses++;

                if (!isComposite[m])
                {
                    for (long k = m * m; k <= size; k += m)
                    {
                        // check for hit on read of isComposite[k]
                        if (cache.access(reinterpret_cast<uintptr_t>(&isComposite[k])))
                            hits++;
                        accesses++;

                        // Update isComposite[k]
                        isComposite[k] = true;

                        // check for hit on write of isComposite[k]
                        if (cache.access(reinterpret_cast<uintptr_t>(&isComposite[k])))
                            hits++;
                        accesses++;
                    }
                }
            }

            // Output hit rate for the current run
            double hitRate = (accesses > 0) ? static_cast<double>(hits) / accesses : 0.0;
            std::cout << hitRate;

            // Reset the cache state after the first run
            if (run == 1)
                cache.resetCacheState();

            // Print tab separator
            if (run < 2)
                std::cout << "\t";
        }

        // Move to the next line for the next cache configuration
        std::cout << std::endl;
    }

    return 0;
}