#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <unordered_map>
#include <istream>
#include <string>
#include <iomanip>

class Cache
{
private:
    int size; // in bytes
    int associativity;
    int block_size;
    int sets;
    std::vector<std::vector<bool>> valid;      // Valid bit for each block in each set
    std::vector<std::vector<int>> lru_counter; // LRU counter for each block in each set
    unsigned long hits = 0;
    unsigned long accesses = 0;

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
                hits++;
                return true;
            }
        }

        // Cache miss
        int victim_index = findLRUVictim(set_index);
        valid[set_index][victim_index] = true;
        updateLRU(set_index, victim_index);
        accesses++;
        return false;
    }

    double getHitRate()
    {
        return (accesses > 0) ? static_cast<double>(hits) / accesses : 0.0;
    }

    void resetCounters()
    {
        hits = 0;
        accesses = 0;
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

void simulateCache(std::ofstream &outputFile, int size, int associativity, int block_size)
{
    Cache cache(size, associativity, block_size);
    cache.resetCounters();

    for (int i = 0; i < 2; ++i)
    {
        std::ifstream inputFile("your_input_file.txt"); // Replace with your actual input file

        if (!inputFile.is_open())
        {
            std::cerr << "Error: Unable to open file." << std::endl;
            return;
        }

        unsigned long address;
        while (inputFile >> std::hex >> address)
        {
            cache.access(address);
        }

        // Output hit rate to the file
        double hitRate = cache.getHitRate();
        outputFile << std::fixed << std::setprecision(4) << hitRate << ",";

        cache.resetCounters();
    }

    outputFile << std::endl;
}

int main()
{
    // Open the output file
    std::ofstream outputFile("solution.csv");

    if (!outputFile.is_open())
    {
        std::cerr << "Error: Unable to open output file." << std::endl;
        return 1;
    }

    // Output CSV header to the file
    outputFile << "size (bytes),1-way 1st loop,1-way 2nd loop,2-way 1st loop,2-way 2nd loop,4-way 1st loop,4-way 2nd loop,8-way 1st loop,8-way 2nd loop," << std::endl;

    // Iterate over different cache configurations
    const int block_size = 64;

    for (int size : {2048, 4096, 8192, 16384, 32768})
    {
        outputFile << size << ",";

        for (int associativity : {1, 2, 4, 8})
        {
            // First loop
            simulateCache(outputFile, size, associativity, block_size);

            // Second loop
            simulateCache(outputFile, size, associativity, block_size);
        }
    }

    // Close the output file
    outputFile.close();

    return 0;
}
