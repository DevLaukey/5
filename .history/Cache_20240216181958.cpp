#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <unordered_map>
#include <istream>
#include <iomanip>
#include <string>

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

void runSimulation(int cacheSize, int associativity, int blockSize, const std::vector<unsigned long> &addresses, std::ostream &output)
{
    Cache cache(cacheSize, associativity, blockSize);

    unsigned long hits = 0;
    unsigned long accesses = 0;

    for (const auto &address : addresses)
    {
        // check for hit on read or write
        if (cache.access(address))
            hits++;
        accesses++;
    }

    // Output hit rate and other relevant information
    double hitRate = (accesses > 0) ? static_cast<double>(hits) / accesses : 0.0;
    output << std::setw(5) << cacheSize << "," << std::setw(5) << blockSize << "," << std::setw(5) << associativity << ",";
    output << std::fixed << std::setprecision(4) << hitRate << std::endl;
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

    // Read addresses from the input file
    std::vector<unsigned long> addresses;
    std::string line;

    while (std::getline(inputFile, line))
    {
        try
        {
            unsigned long currentAddress = std::stoul(line, nullptr, 16);
            addresses.push_back(currentAddress);
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

    // Output header
    std::cout << "size (bytes),block size,associativity,hit rate" << std::endl;

    // Define cache configurations
    std::vector<std::tuple<int, int, int>> cacheConfigurations = {
        {2048, 4, 1}, {2048, 4, 2}, {4096, 4, 2}, {8192, 8, 2}, {8192, 8, 4}, {16384, 8, 4}, {16384, 8, 8}, {32768, 16, 4}, {32768, 16, 8}, {65536, 32, 4}, {65536, 32, 8}};

    // Run simulations for each cache configuration
    for (const auto &config : cacheConfigurations)
    {
        int cacheSize, blockSize, associativity;
        std::tie(cacheSize, blockSize, associativity) = config;

        runSimulation(cacheSize, associativity, blockSize, addresses, std::cout);
    }

    return 0;
}
