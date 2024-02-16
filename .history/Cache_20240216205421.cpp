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

    const long upperBound = maxAddress;

    // Initialize the cache with the desired parameters
    Cache cache(16384, 8, 64);

    unsigned long hits1 = 0;
    unsigned long accesses1 = 0;

    // Reset the file stream to the beginning of the file
    inputFile.clear();
    inputFile.seekg(0, std::ios::beg);

    unsigned long address;

    // First run through the patterns
    while (inputFile >> std::hex >> address)
    {
        // check for hit on read or write
        if (cache.access(address))
            hits1++;
        accesses1++;
    }

    // Output hit rate for the first run
    double hitRate1 = (accesses1 > 0) ? static_cast<double>(hits1) / accesses1 : 0.0;
    std::cout << "First Run - Hits: " << hits1 << ", Accesses: " << accesses1 << std::endl;
    std::cout << "First Run - Hit Rate: " << hitRate1 << std::endl;

    // Second run through the patterns without resetting the cache
    unsigned long hits2 = 0;
    unsigned long accesses2 = 0;

    // Reset the file stream to the beginning of the file again
    inputFile.clear();
    inputFile.seekg(0, std::ios::beg);

    // Second run through the patterns
    while (inputFile >> std::hex >> address)
    {
        // check for hit on read or write
        if (cache.access(address))
            hits2++;
        accesses2++;
    }

    // Output hit rate for the second run
    double hitRate2 = (accesses2 > 0) ? static_cast<double>(hits2) / accesses2 : 0.0;
    std::cout << "Second Run - Hits: " << hits2 << ", Accesses: " << accesses2 << std::endl;
    std::cout << "Second Run - Hit Rate: " << hitRate2 << std::endl;

    return 0;
}
