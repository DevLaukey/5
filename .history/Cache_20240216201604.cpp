#include <iostream>
#include <vector>
#include <cstdint>

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
    const long upperBound = 1000; // Change this according to your upper bound

    // Initialize the cache with the desired parameters
    Cache cache(256 * 1024, 8, 16);

    unsigned long hits = 0;
    unsigned long accesses = 0;

    // Assuming isComposite is a boolean array
    std::vector<char> isComposite(upperBound + 1, false);

    // Simulate cache behavior during Sieve of Eratosthenes algorithm
    for (long m = 2; m <= upperBound; m++)
    {
        // check for hit on read of isComposite[m]
        if (cache.access(reinterpret_cast<uintptr_t>(&isComposite[m])))
            hits++;
        accesses++;

        if (!isComposite[m])
        {
            for (long k = m * m; k <= upperBound; k += m)
            {
                // check for hit on read of isComposite[k]
                if (cache.access(reinterpret_cast<unsigned long>(&isComposite[k])))
                    hits++;
                accesses++;

                // Update isComposite[k]
                isComposite[k] = true;

                // check for hit on write of isComposite[k]
                if (cache.access(reinterpret_cast<unsigned long>(&isComposite[k])))
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
