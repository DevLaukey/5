#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <istream>
#include <string>

class Cache
{
private:
    int size;
    int associativity;
    int block_size;
    int sets;
    std::vector<std::vector<bool>> valid;
    std::vector<std::vector<int>> lru_counter;

public:
    Cache(int size, int associativity, int block_size) : size(size), associativity(associativity), block_size(block_size)
    {
        sets = size / (associativity * block_size);
        valid.assign(sets, std::vector<bool>(associativity, false));
        lru_counter.assign(sets, std::vector<int>(associativity, 0));
    }

    bool access(bool *address)
    {
        int set_index = ((reinterpret_cast<unsigned long>(address)) / block_size) % sets;

        for (int i = 0; i < associativity; ++i)
        {
            if (valid[set_index][i] && lru_counter[set_index][i] == 0)
            {
                updateLRU(set_index, i);
                return true;
            }
        }

        int victim_index = findLRUVictim(set_index);
        valid[set_index][victim_index] = true;
        updateLRU(set_index, victim_index);
        return false;
    }

private:
    void updateLRU(int set_index, int used_index)
    {
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
        int max_lru = -1;
        int victim_index = 0;

        for (int i = 0; i < associativity; ++i)
        {
            if (!valid[set_index][i])
            {
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

    // Determine the upper bound dynamically based on the content of the file
    unsigned long maxAddress = 0;
    std::string line;

    std::ifstream inputFile(inputFileName);

    if (!inputFile.is_open())
    {
        std::cerr << "Error: Unable to open file " << inputFileName << std::endl;
        return 1;
    }

    while (std::getline(inputFile, line))
    {
        try
        {
            unsigned long currentAddress = std::stoul(line, nullptr, 16);
            maxAddress = std::max(maxAddress, currentAddress);
        }
        catch (const std::invalid_argument &e)
        {
            std::cerr << "Error: Invalid address in the file." << std::endl;
            return 1;
        }
        catch (const std::out_of_range &e)
        {
            std::cerr << "Error: Address out of range." << std::endl;
            return 1;
        }
    }

    const long upperBound = maxAddress;
    const long upperBoundSquareRoot = static_cast<long>(std::sqrt(upperBound));

    // Initialize the cache with the desired parameters
    Cache cache(256 * 1024, 8, 64);

    // Initialize isComposite array
    std::vector<std::vector<bool>::reference> isComposite(upperBound + 1, std::vector<bool>::reference());

    for (long i = 0; i <= upperBound; ++i)
    {
        isComposite[i] = false;
    }

    unsigned long hits = 0;
    unsigned long accesses = 0;

    for (long m = 2; m <= upperBoundSquareRoot; m++)
    {
        // check for hit on read of isComposite[m]
        if (cache.access(&isComposite[m]))
            hits++;
        accesses++;

        if (!isComposite[m])
        {
            for (long k = m * m; k <= upperBound; k += m)
            {
                // check for hit on read of isComposite[k]
                if (cache.access(&isComposite[k]))
                    hits++;
                accesses++;

                isComposite[k] = true;

                // check for hit on write of isComposite[k]
                if (cache.access(&isComposite[k]))
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
