#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h> 
#include <math.h>

int hit_count = 0, miss_count = 0, eviction_count = 0; 
int sflag = 0;
int eflag = 0;
int bflag = 0;
char *t = NULL; 
int tagBits = 0;
int indexBits = 0;
int globalTime = 0;

struct Line {
    int validBit;
    int tag;
    int time;
};

struct Set {
    struct Line** lines;
};

typedef struct Cache {
    struct Set** sets;
} Cache;

Cache* initStructs() {
    //num sets = ln(s) num lines/set = E
    Cache* cache = (Cache*)malloc(sizeof(Cache));
    int numSets = 1 << sflag;
    cache->sets = (struct Set**) malloc(sizeof(struct Set*) * numSets);
    for(int i = 0; i < numSets; ++i) {
          cache->sets[i] = (struct Set*)malloc(sizeof(struct Set));
          cache->sets[i]->lines = (struct Line **) malloc(eflag * sizeof(struct Line*));
          for(int j = 0; j < eflag; ++j) {
                cache->sets[i]->lines[j] = (struct Line*)malloc(sizeof(struct Line));
                cache->sets[i]->lines[j]->tag = 0;
                cache->sets[i]->lines[j]->validBit = 0;
                cache->sets[i]->lines[j]->time = 0;
          }
    } 
    indexBits = sflag;
    tagBits = 64 - (sflag + bflag);
    return cache;
}

void runWithInput(unsigned long long int memaddrs, Cache* cache) {
  
   unsigned long long int tag = memaddrs >> (sflag + bflag);
   unsigned long long int index = memaddrs << tagBits;
   index = index >> (tagBits + bflag);
   struct Set* checkSet = cache->sets[index];
   globalTime++; 
   for (int i = 0; i < eflag; ++i) {
        struct Line* checkLine = checkSet->lines[i];
        if(checkLine->validBit) {
            if(checkLine->tag == tag) {
                checkLine->time = globalTime;
                hit_count++;
                return;
            }
        } 
   } 
   miss_count++;
    
   for (int i = 0; i < eflag; ++i) {
        struct Line* checkLine = checkSet->lines[i];
        if (checkLine->validBit == 0) {
            checkLine->validBit = 1;
            checkLine->tag = tag;
            checkLine->time = globalTime;  
            return;      
        }
    }
   int lowestTime = checkSet->lines[0]->time;
   int lruIndex = 0;
   for (int i = 1; i < eflag; ++i) {
        if(checkSet->lines[i]->time <= lowestTime) {
            lowestTime = checkSet->lines[i]->time;
            lruIndex = i;
        }      
   }
   
   eviction_count++; 
   checkSet->lines[lruIndex]->tag = tag;
   checkSet->lines[lruIndex]->time = globalTime;
   return;

}

void readInInput(Cache* cache) {
    FILE *file;
    file = fopen(t, "r"); 

    if(file == NULL) {
        printf("%s\n", "Error opening file");
        exit(1);
    }
    
    long long int memaddrs; 
    char type;    
    int space;
    while(fscanf(file, " %c %llx,%d", &type, &memaddrs, &space) != EOF) { 
            if (type != 'I') {
            if (type == 'M') {
                  runWithInput(memaddrs, cache);
            }

            runWithInput(memaddrs, cache);
        }
    }    
}


int main(int argc, char *argv[])
{
    opterr = 0;

    const char *optstring = "s:E:b:t:";

    int ret; 

    while((ret = getopt(argc, argv, optstring)) != -1) {
        switch(ret) {
            case 's':
                sflag = atoi(optarg);
                break;
            case 'E':
                eflag = atoi(optarg);
                break;
            case 'b':
                bflag = atoi(optarg);
                break;
            case 't':
                t = optarg;
                break;
        }
    }
    

    Cache* cache = initStructs(); 
    readInInput(cache);

    int numSets = 1 << sflag;
    for (int i = 0; i < numSets; ++i) {
        for (int j = 0; j < eflag; ++j) {
            free(cache->sets[i]->lines[j]); 
        }
        free(cache->sets[i]->lines);
        free(cache->sets[i]); 
    }

    free(cache->sets);
    free(cache);



    printSummary("%d, %d, %d", hit_count, miss_count, eviction_count);
    return 0;
}