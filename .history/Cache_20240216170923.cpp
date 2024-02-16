#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <istream>
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

void simulateCache(const std::vector<bool> &isComposite, Cache &cache, std::ofstream &outputFile)
{
    cache.resetCounters();

    const long upperBoundSquareRoot = static_cast<long>(sqrt(isComposite.size()));

    for (long m = 2; m <= upperBoundSquareRoot; m++)
    {
        // Check for hit on read of isComposite[m]
        if (cache.access((unsigned long)(&isComposite[m])))
            hits++;
        accesses++;

        if (!isComposite[m])
        {
            for (long k = m * m; k < isComposite.size(); k += m)
            {
                // Check for hit on read of isComposite[k]
                if (cache.access((unsigned long)(&isComposite[k])))
                    hits++;
                accesses++;

                // Update isComposite[k]
                isComposite[k] = true;

                // Check for hit on write of isComposite[k]
                if (cache.access((unsigned long)(&isComposite[k])))
                    hits++;
                accesses++;
            }
        }
    }

    // Output hit rate to the file
    double hitRate = cache.getHitRate();
    outputFile << std::fixed << std::setprecision(4) << hitRate << ",";
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

    // Read the input file to determine the upper bound dynamically
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
    Cache cache(256 * 1024, 8, 64);

    // Reset the file stream to the beginning of the file
    inputFile.clear();
    inputFile.seekg(0, std::ios::beg);

    // Array to store composite flags
    std::vector<bool> isComposite(upperBound + 1, false);

    // Open output files
    std::ofstream outputFileByAssociativity("random_by_associativity.csv");
    std::ofstream outputFileByBlockSize("random_by_blocksize.csv");

    if (!outputFileByAssociativity.is_open() || !outputFileByBlockSize.is_open())
    {
        std::cerr << "Error: Unable to open output files." << std::endl;
        return 1;
    }

    // Output CSV headers to the files
    outputFileByAssociativity << "Associativity,Hit Rate," << std::endl;
    outputFileByBlockSize << "Block Size,Hit Rate," << std::endl;

    // Iterate over different associativity values
    for (int associativity : {1, 2, 4, 8})
    {
        cache.resetCounters();

        // First loop for associativity
        simulateCache(isComposite, cache, outputFileByAssociativity);

        // Output associativity value to the file
        outputFileByAssociativity << associativity << std::endl;
    }

    // Iterate over different block sizes
    for (int blockSize : {32, 64, 128})
    {
        cache.resetCounters();
        cache = Cache(256 * 1024, 8, blockSize); // Update cache with new block size

        // Second loop for block size
        simulateCache(isComposite, cache, outputFileByBlockSize);

        // Output block size value to the file
        outputFileByBlockSize << blockSize << std::endl;
    }

    // Close output files
    outputFileByAssociativity.close();
    outputFileByBlockSize.close();

    std::cout << "Simulation completed. Results saved in 'random_by_associativity.csv' and 'random_by_blocksize.csv'." << std::endl;

    return 0;
}
