#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstdint>
#include <cstring>
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

    bool access(uintptr_t address)
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

    // Cache configurations
    std::vector<std::pair<int, int>> cacheConfigurations = {
        {1, 1}, {1, 2}, {2, 1}, {2, 2}, {4, 1}, {4, 2}, {8, 1}, {8, 2}};

    // Print header
    std::cout << "size (bytes),";

    for (const auto &config : cacheConfigurations)
    {
        int associativity = config.first;
        int block_size = config.second;

        std::cout << associativity << "-way " << block_size << "B 1st loop," << associativity << "-way " << block_size << "B 2nd loop,";
    }

    std::cout << std::endl;

    // Initialize the cache with the desired parameters
    Cache cache(256 * 1024, 8, 64);

    for (const auto &config : cacheConfigurations)
    {
        int associativity = config.first;
        int block_size = config.second;

        std::cout << "2048"; // Size (bytes)

        cache = Cache(256 * 1024, associativity, block_size);

        unsigned long hits = 0;
        unsigned long accesses = 0;

        // Reset the file stream to the beginning of the file
        inputFile.clear();
        inputFile.seekg(0, std::ios::beg);

        unsigned long address;

        // Simulate sieve of Eratosthenes algorithm with cache access checks
        for (long m = 2; m <= upperBound; m++)
        {
            bool isComposite = false; // Define isComposite here

            // check for hit on read of isComposite[m]
            if (cache.access(reinterpret_cast<uintptr_t>(&isComposite)))
                hits++;
            accesses++;

            if (!isComposite)
            {
                for (long k = m * m; k <= upperBound; k += m)
                {
                    // check for hit on read of isComposite[k]
                    if (cache.access(reinterpret_cast<uintptr_t>(&isComposite)))
                        hits++;
                    accesses++;

                    // check for hit on write of isComposite[k]
                    if (cache.access(reinterpret_cast<uintptr_t>(&isComposite)))
                        hits++;
                    accesses++;
                }
            }
        }

        // Output hit rate for the current configuration
        double hitRate = (accesses > 0) ? static_cast<double>(hits) / accesses : 0.0;
        std::cout << "," << hitRate << ",";
    }

    std::cout << std::endl;

    return 0;
}
