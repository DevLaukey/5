#include <iostream>
#include <vector>
#include <cmath>

class Cache
{
private:
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
                // Found an invalid block, use it as a victim
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
    // Example usage of the Cache class
    Cache cache(256 * 1024, 8, 64); // 256 KiB cache, 8-way set associativity, 64-byte block size

    const long upperBound = 100; // Adjust based on the size of your data

    // Example data structure for the Sieve of Eratosthenes
    std::vector<bool> isComposite(upperBound + 1, false);

    unsigned long hits = 0;
    unsigned long accesses = 0;

    for (long m = 2; m <= upperBound; m++)
    {
        if (cache.access((unsigned long)(&(isComposite[m]))))
            hits++;
        accesses++;

        if (!isComposite[m])
        {
            for (long k = m * m; k <= upperBound; k += m)
            {
                if (cache.access((unsigned long)(&(isComposite[k]))))
                    hits++;
                accesses++;

                isComposite[k] = true;

                if (cache.access((unsigned long)(&(isComposite[k]))))
                    hits++;
                accesses++;
            }
        }
    }

    // Output hit rate and other relevant information
    double hitRate = (accesses > 0) ? static_cast<double>(hits) / accesses : 0.0;
    std::cout << "Hits: " << hits << ", Accesses: " << accesses << std::endl;
    std::cout << "Hit Rate: " << hitRate << std::endl;

    return 0;
}
