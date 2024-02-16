#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>

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

    bool access(uintptr_t address)
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

void runSimulation(unsigned long upperBound)
{
    // Initialize the isComposite array
    bool *isComposite = new bool[upperBound + 1];
    std::memset(isComposite, 0, (upperBound + 1) * sizeof(bool));

    // Initialize the cache with the desired parameters (256KiB, 8-way set associative, 64B block size)
    Cache cache(256 * 1024, 8, 64);

    unsigned long hits = 0;
    unsigned long accesses = 0;

    // Simulate sieve of Eratosthenes algorithm with cache access checks
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
                if (cache.access(reinterpret_cast<uintptr_t>(&isComposite[k])))
                    hits++;
                accesses++;

                // check for hit on write of isComposite[k]
                if (cache.access(reinterpret_cast<uintptr_t>(&isComposite[k])))
                    hits++;
                accesses++;

                isComposite[k] = true;
            }
        }
    }

    // Output hit rate and other relevant information
    double hitRate = (accesses > 0) ? static_cast<double>(hits) / accesses : 0.0;
    std::cout << "Hits: " << hits << ", Accesses: " << accesses << std::endl;
    std::cout << "Hit Rate: " << hitRate << std::endl;

    // Cleanup allocated memory
    delete[] isComposite;
}

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

    // Determine the upper bound dynamically based on the content of the file
    unsigned long maxAddress = 0;
    std::string line;

    while (std::getline(inputFile, line))
    {
        try
        {
            unsigned long currentAddress = std::stoul(line, nullptr, 16);
            maxAddress = std::max(maxAddress, currentAddress);
        }
        catch (const std::invalid_argument &e)
        {
            // Handle invalid address (non-hexadecimal)
            std::cerr << "Error: Invalid address in the file." << std::endl;
            return 1;
        }
        catch (const std::out_of_range &e)
        {
            // Handle out of range address
            std::cerr << "Error: Address out of range." << std::endl;
            return 1;
        }
    }

    // Run the simulation with the determined upper bound
    runSimulation(maxAddress);

    return 0;
}
