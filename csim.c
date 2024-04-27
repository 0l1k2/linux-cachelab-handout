/********************************************************
* csim.c - Cache Simulator
*********************************************************/

#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#define ADDRESS_SIZE 64

// Cache parameters
int s, E, b;

// Counter for cache hits and misses
int hit_count = 0;
int miss_count = 0;
int eviction_count = 0;

// Cache line structure
typedef struct {
    int valid;
    unsigned long long tag;
    int timestamp;
} cache_line;

// Cache set structure
typedef struct {
    cache_line* lines;
} cache_set;

// Cache structure
typedef struct {
    cache_set* sets;
} cache;

/**** Function prototypes ****/
void init_cache(int S, int E, int B, cache* cache_ptr);
void access_cache(char* address, int size, cache* cache_ptr);

/**************************************************************
* init_cache - Initialize the cache structure
**************************************************************/
void init_cache(int S, int E, int B, cache* cache_ptr)
{
    int num_sets = 1 << S;
    cache_set* sets = (cache_set*)malloc(num_sets * sizeof(cache_set));
    cache_ptr->sets = sets;

    for (int i = 0; i < num_sets; i++) {
        cache_line* lines = (cache_line*)malloc(E * sizeof(cache_line));
        sets[i].lines = lines;

        for (int j = 0; j < E; j++) {
            lines[j].valid = 0;
            lines[j].tag = 0;
            lines[j].timestamp = 0;
        }
    }
}

/**************************************************************
* access_cache - Simulate cache access for a memory address
**************************************************************/
void access_cache(char* address, int size, cache* cache_ptr)
{
    unsigned long long int addr = strtoull(address, NULL, 16);
    int set_index = (addr >> b) & ((1 << s) - 1);
    int tag = addr >> (s + b);

    cache_set set = cache_ptr->sets[set_index];
    int oldest_timestamp = -1;
    int oldest_line = -1;
    int line_found = 0;

    for (int i = 0; i < E; i++) {
        cache_line line = set.lines[i];

        if (line.valid && line.tag == tag) {
            hit_count++; // Cache hit
            set.lines[i].timestamp = 0; // Reset timestamp
            line_found = 1;
        } else if (!line.valid) {
            if (!line_found) {
                set.lines[i].valid = 1;
                set.lines[i].tag = tag;
                set.lines[i].timestamp = 0;
                miss_count++; // Cache miss
            }
            line_found = 1;
        } else {
            set.lines[i].timestamp++; // Increment timestamp for LRU replacement

            if (oldest_timestamp < set.lines[i].timestamp) {
                oldest_timestamp = set.lines[i].timestamp;
                oldest_line = i;
            }
        }
    }

    if (!line_found) {
        set.lines[oldest_line].tag = tag;
        set.lines[oldest_line].timestamp = 0;
        miss_count++; // Cache miss
        eviction_count++; // Eviction
    }
}

int main(int argc, char* argv[])
{
    char* trace_file = NULL;
    char c;
    cache sim_cache;

    // Parse command line arguments
    while ((c = getopt(argc, argv, "s:E:b:t:")) != -1) {
        switch (c) {
            case 's': // Number of set index bits
                s = atoi(optarg);
                break;
            case 'E': // Associativity
                E = atoi(optarg);
                break;
            case 'b': // Number of block bits
                b = atoi(optarg);
                break;
            case 't': // Trace file path
                trace_file = optarg;
                break;
            default:
                printf("Invalid command line arguments.\n");
                exit(EXIT_FAILURE);
        }
    }

    if (s == 0 || E == 0 || b == 0 || trace_file == 0) {
        printf("Missing required command line arguments.\n");
        exit(EXIT_FAILURE);
    }

    // Initialize cache
    init_cache(s, E, b, &sim_cache);

    // Simulate cache access for each memory address in the trace file
    FILE* trace = fopen(trace_file, "r");

    if (trace == NULL) {
        printf("Unable to open trace file.\n");
        exit(EXIT_FAILURE);
    }

    char line[256];
    while (fgets(line, sizeof(line), trace)) {
        char access_type;
        char address[ADDRESS_SIZE];

        sscanf(line, " %c %s,", &access_type, address);
        
        if (access_type == 'L' || access_type == 'S' || access_type == 'M') {
            // Simulate cache access
            access_cache(address, sizeof(address), &sim_cache);

            if (access_type == 'M') {
                // Simulate cache access again for modification
                access_cache(address, sizeof(address), &sim_cache);
            }
        }
    }

    // Close trace file
    fclose(trace);

    // Print simulation summary
    printSummary(hit_count, miss_count, eviction_count);

    // Free allocated memory
    for (int i = 0; i < (1 << s); i++) {
        free(sim_cache.sets[i].lines);
}
      free(sim_cache.sets);
      return 0;
}


