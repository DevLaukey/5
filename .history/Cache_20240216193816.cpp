#include <iostream>
#include <vector>
#include <unordered_map>
#include <deque>

class Cache
{
public:
    Cache(int cache_size, int block_size, int associativity) : cache_size(cache_size),
                                                               block_size(block_size),
                                                               associativity(associativity),
                                                               num_sets(associativity > 1 ? cache_size / (block_size * associativity) : 1),
                                                               cache(num_sets, std::vector<CacheBlock>(associativity, {-1, false})),
                                                               lru_tracker(num_sets, std::deque<int>(associativity, 0)) {}

    bool access(int address)
    {
        int tag, index, offset;
        extractFields(address, tag, index, offset);

        // Check if the block is in the cache
        for (int i = 0; i < associativity; ++i)
        {
            if (cache[index][i].tag == tag && cache[index][i].valid)
            {
                // Cache hit, update LRU
                updateLRU(index, i);
                return true;
            }
        }

        // Cache miss, update the cache
        updateCache(tag, index);
        return false;
    }

private:
    struct CacheBlock
    {
        int tag;
        bool valid;
    };

    int cache_size;
    int block_size;
    int associativity;
    int num_sets;
    std::vector<std::vector<CacheBlock>> cache;
    std::vector<std::deque<int>> lru_tracker;

    void updateCache(int tag, int index)
    {
        // Choose a way to replace using LRU policy
        int wayToReplace = lru_tracker[index].front();
        lru_tracker[index].pop_front();
        lru_tracker[index].push_back(wayToReplace);

        // Replace the block in the cache
        cache[index][wayToReplace] = {tag, true};
    }

    void updateLRU(int index, int way)
    {
        // Move the accessed way to the back of the LRU tracker
        lru_tracker[index].remove(way);
        lru_tracker[index].push_back(way);
    }

    void extractFields(int address, int &tag, int &index, int &offset)
    {
        int offsetMask = (1 << (int)log2(block_size)) - 1;
        int indexMask = ((1 << (int)log2(num_sets)) - 1) << (int)log2(block_size);
        tag = address >> ((int)log2(block_size) + (int)log2(num_sets));
        index = (address >> (int)log2(block_size)) & (num_sets - 1);
        offset = address & offsetMask;
    }
};

// Function to read trace file and simulate cache access
void simulateCacheAccess(const std::vector<int> &addresses, Cache &cache)
{
    int hits = 0;
    int total_accesses = addresses.size();

    for (int address : addresses)
    {
        if (cache.access(address))
        {
            hits++;
        }
    }

    double hit_rate = static_cast<double>(hits) / total_accesses;
    std::cout << "Hit Rate: " << hit_rate * 100 << "%" << std::endl;
}

int main()
{
    // Example usage:
    int cache_size = 256 * 1024; // Cache size in bytes (e.g., 256 KiB)
    int block_size = 64;         // Block size in bytes
    int associativity = 8;       // Associativity (1 for direct-mapped, >1 for set-associative)

    Cache cache(cache_size, block_size, associativity);

    // Simulate cache accesses using a trace file (replace with your actual trace file)
    std::vector<int> trace_addresses = {0x100, 0x104, 0x108, 0x10C, 0x110, 0x114, 0x118, 0x11C};

    // Test the cache object
    simulateCacheAccess(trace_addresses, cache);

    return 0;
}
