#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

unsigned int intlog2(unsigned int x)
{
    unsigned int y = 0;
    while (x > 1)
    {
        x >>= 1;
        ++y;
    }
    return y;
}
void printA(long unsigned int *array, int size)
{
    for (int i = 0; i < size; i++)
    {
        printf("%lx ", array[i]);
    }
    printf("\n");
}

long unsigned int *addtoqueue(long unsigned int *queue, int size, int head, long unsigned int value)
{
    if (size == 1)
    {
        queue[0] = value;
    }
    else if (head == size)
    {
        for (int i = size; i > 0; i--)
        {
            queue[i] = queue[i - 1];
        }
        queue[0] = value;
    }
    else if (head == -1)
    {
        queue[0] = value;
    }
    else
    {
        for (int i = head; i >= 0; i--)
        {
            queue[i + 1] = queue[i];
        }
        queue[0] = value;
    }
    return queue;
}

struct set
{
    unsigned long int *queue; //set is size of queue to be num lines
    int head;
    int isValid;
};

int main(int argc, char **argv)
{
    //const long unsigned int adressSize=48;
    int misses = 0;
    int hits = 0;
    int reads = 0;
    int writes = 0;
    int pmisses = 0;
    int phits = 0;
    int preads = 0;
    int pwrites = 0;

    long unsigned int cachesize = atoi(argv[1]);
    char *associativity = argv[2];
    //char* policy = argv[3];
    long unsigned int blocksize = atoi(argv[4]);
    char *filename = argv[5];
    FILE *file;
    file = fopen(filename, "r");
    char rorw;
    int numSets;
    int numLines;
    long unsigned int blockOffsetbits;
    long unsigned int setIndexbits;
    //long unsigned int tagBits;
    if (cachesize % 2 != 0)
    {
        printf("ERROR CACHEKSIZE IS NOT POWER OF 2");
        return EXIT_FAILURE;
    }
    if (blocksize % 2 != 0)
    {
        printf("ERROR BLOCKSIZE IS NOT POWER OF 2");
        return EXIT_FAILURE;
    }
    if (strcmp(associativity, "direct") == 0)
    {
        numSets = cachesize / blocksize;
        numLines = 1;
    }
    else if (strcmp(associativity, "assoc") == 0)
    {
        numSets = 1;
        numLines = cachesize / blocksize;
    }
    else
    {
        int temp = strlen(associativity);
        char nchar = associativity[temp - 1];
        int n = nchar - '0';
        if (n % 2 != 0)
        {
            printf("ERROR N IS NOT POWER OF 2");
            return EXIT_FAILURE;
        }
        numSets = cachesize / (blocksize * n);
        numLines = n;
    }
    blockOffsetbits = intlog2(blocksize);
    setIndexbits = intlog2(numSets);
    //tagBits= adressSize-blockOffsetbits-setIndexbits;
    unsigned long int address;
    struct set *cache = malloc(numSets * sizeof(struct set));
    struct set *pcache = malloc(numSets * sizeof(struct set));
    for (int i = 0; i < numSets; i++)
    {
        cache[i].isValid = 0;
        pcache[i].isValid = 0;
    }
    while (1)
    {
        fscanf(file, "%*x:");
        fscanf(file, "%*c");
        fscanf(file, "%c", &rorw);
        fscanf(file, "%lx", &address);
        
        if (rorw == 'e')
        {
            break;
        }
        //long unsigned int bbits= address & ( (1<<blockOffsetbits) - 1);
        long unsigned int sbits = (address >> blockOffsetbits) & ((1 << setIndexbits) - 1);
        long unsigned int tbits = address >> (setIndexbits + blockOffsetbits);
        long unsigned int modifiedaddress = (address >> blockOffsetbits) + 1;
        long unsigned int modifiedsbits = (modifiedaddress) & ((1 << setIndexbits) - 1);
        long unsigned int modifiedtbits = modifiedaddress >> (setIndexbits);
        int prefetch = 0;
        if ((cache[sbits].isValid) == 0)
        {
            cache[sbits].isValid = 1;
            cache[sbits].head = -1;
            cache[sbits].queue = calloc(numLines, numLines * sizeof(long unsigned int));
            for (int i = 0; i < numLines; i++)
            {
                addtoqueue(cache[sbits].queue, numLines, cache[sbits].head, tbits);
                if (cache[sbits].head != numLines)
                {
                    cache[sbits].head++;
                }
            }
            if (rorw == 'R')
            {
                misses++;
                reads++;
            }
            else
            {
                misses++;
                reads++;
                writes++;
            }
        }
        else if (cache[sbits].isValid == 1)
        {
            for (int i = 0; i < numLines; i++)
            {
                if (cache[sbits].queue[i] == tbits)
                {
                    if (rorw == 'R')
                    {
                        hits++;
                    }
                    else
                    {
                        hits++;
                        writes++;
                    }
                    break;
                }
                if (i == numLines - 1)
                {
                    addtoqueue(cache[sbits].queue, numLines, cache[sbits].head, tbits);
                    if (cache[sbits].head != numLines)
                    {
                        cache[sbits].head++;
                    }
                    if (rorw == 'R')
                    {
                        misses++;
                        reads++;
                    }
                    else
                    {
                        misses++;
                        reads++;
                        writes++;
                    }
                }
            }
        }

        if ((pcache[sbits].isValid) == 0)
        {
            pcache[sbits].isValid = 1;
            pcache[sbits].head = -1;
            pcache[sbits].queue = calloc(numLines, numLines * sizeof(long unsigned int));
            for (int i = 0; i < numLines; i++)
            {
                addtoqueue(pcache[sbits].queue, numLines, pcache[sbits].head, tbits);
                prefetch = 1;
                if (pcache[sbits].head != numLines)
                {
                    pcache[sbits].head++;
                }
            }
            if (rorw == 'R')
            {
                preads++;
                pmisses++;
                
            }
            else
            {
                
                pmisses++;
                preads++;
                pwrites++;
            }
        }
        else if (pcache[sbits].isValid == 1)
        {
            for (int i = 0; i < numLines; i++)
            {
                if (pcache[sbits].queue[i] == tbits)
                {
                    if (rorw == 'R')
                    {
                        phits++;
                    }
                    else
                    {
                        phits++;
                        pwrites++;
                    }
                    break;
                }
                if (i == numLines - 1)
                {
                    addtoqueue(pcache[sbits].queue, numLines, pcache[sbits].head, tbits);
                    prefetch = 1;
                    if (pcache[sbits].head != numLines)
                    {
                        pcache[sbits].head++;
                    }
                    
                    if (rorw == 'R')
                    {
                        
                        
                        pmisses++;
                        preads++;
                    }
                    else
                    {
                        
                        pmisses++;
                        preads++;
                        pwrites++;
                    }
                }
            }
        }

        if (prefetch == 1)
        {
            if ((pcache[modifiedsbits].isValid) == 0)
            {
                pcache[modifiedsbits].isValid = 1;
                pcache[modifiedsbits].head = -1;
                pcache[modifiedsbits].queue = calloc(numLines, numLines * sizeof(long unsigned int));
                for (int i = 0; i < numLines; i++)
                {
                    addtoqueue(pcache[modifiedsbits].queue, numLines, pcache[modifiedsbits].head, modifiedtbits);
                    if (pcache[modifiedsbits].head != numLines)
                    {
                        pcache[modifiedsbits].head++;
                    }
                }
                preads++;
            }
            else if (pcache[modifiedsbits].isValid == 1)
            {
                for (int i = 0; i < numLines; i++)
                {
                    if (pcache[modifiedsbits].queue[i] == modifiedtbits)
                    {
                        break;
                    }
                    if (i == numLines - 1)
                    {
                        addtoqueue(pcache[modifiedsbits].queue, numLines, pcache[modifiedsbits].head, modifiedtbits);
                        if (pcache[modifiedsbits].head != numLines)
                        {
                            pcache[modifiedsbits].head++;
                        }
                        preads++;
                    }
                }
            }
        }
    
    }
    for (int i = 0; i < numSets; i++)
    {
        if ((cache[i].isValid) == 1)
            free(cache[i].queue);
    }
    free(cache);

    for (int i = 0; i < numSets; i++)
    {
        if ((pcache[i].isValid) == 1)
            free(pcache[i].queue);
    }
    free(pcache);

    printf("Prefetch 0\nMemory reads: %d\nMemory writes: %d\nCache hits: %d\nCache misses: %d\n", reads, writes, hits, misses);
    printf("Prefetch 1\nMemory reads: %d\nMemory writes: %d\nCache hits: %d\nCache misses: %d\n", preads, pwrites, phits, pmisses);
}
