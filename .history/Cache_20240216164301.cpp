#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>

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

    bool access(int address)
    {
        int set_index = (address / block_size) % sets;
        int block_offset = address % block_size;

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

double runSimulation(const char *inputFileName, int cacheSize, int associativity, int blockSize)
{
    Cache cache(cacheSize, associativity, blockSize);

    unsigned long hits = 0;
    unsigned long accesses = 0;

    std::ifstream inputFile(inputFileName);

    if (!inputFile.is_open())
    {
        std::cerr << "Error: Unable to open file " << inputFileName << std::endl;
        return 0.0;
    }

    unsigned long address;
    while (inputFile >> std::hex >> address)
    {
        if (cache.access(address))
            hits++;
        accesses++;
    }

    return (accesses > 0) ? static_cast<double>(hits) / accesses : 0.0;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    const char *inputFileName = argv[1];

    std::cout << "size (bytes),1-way 1st loop,1-way 2nd loop,2-way 1st loop,2-way 2nd loop,4-way 1st loop,4-way 2nd loop,8-way 1st loop,8-way 2nd loop," << std::endl;

    std::vector<int> cacheSizes = {2048, 4096, 8192, 16384, 32768};
    std::vector<int> associativities = {1, 2, 4, 8};
    std::vector<int> blockSizes = {64};

    for (int cacheSize : cacheSizes)
    {
        std::cout << cacheSize << ",";

        for (int associativity : associativities)
        {
            for (int blockSize : blockSizes)
            {
                double hitRate = runSimulation(inputFileName, cacheSize, associativity, blockSize);
                std::cout << std::fixed << std::setprecision(4) << hitRate << ",";

                std::cout << std::endl;
            }
        }
    }

    return 0;
}
