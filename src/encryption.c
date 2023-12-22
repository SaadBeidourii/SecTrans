#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/encryption.h"
#include <stdint.h>


//////////////// 3 DES ///////////////////////

int PC1[] = {
    57, 49, 41, 33, 25, 17, 9,
    1, 58, 50, 42, 34, 26, 18,
    10, 2, 59, 51, 43, 35, 27,
    19, 11, 3, 60, 52, 44, 36,
    63, 55, 47, 39, 31, 23, 15,
    7, 62, 54, 46, 38, 30, 22,
    14, 6, 61, 53, 45, 37, 29,
    21, 13, 5, 28, 20, 12, 4
};


// Permutation applied on shifted key to get Ki+1
int PC2[] = {
    14, 17, 11, 24, 1, 5,
    3, 28, 15, 6, 21, 10,
    23, 19, 12, 4, 26, 8,
    16, 7, 27, 20, 13, 2,
    41, 52, 31, 37, 47, 55,
    30, 40, 51, 45, 33, 48,
    44, 49, 39, 56, 34, 53,
    46, 42, 50, 36, 29, 32
};

// Number of bit shifts
int shifts[] = {
    1, 1, 2, 2,
    2, 2, 2, 2,
    1, 2, 2, 2,
    2, 2, 2, 1
};


// Permutation function to map input using a permutation table
uint64_t permute(uint64_t k, int* arr, int n) {
    uint64_t permutedKey = 0;
    for (int i = 0; i < n; i++) {
        int shift = 64 - arr[i];
        permutedKey |= ((k >> shift) & 1) << (n - 1 - i);
    }
    return permutedKey;
}

// Function to perform the circular left shift by n
uint64_t leftShift(uint64_t k, int shifts) {
    uint64_t mask = 0xFFFFFFF;  // 28-bit mask
    uint64_t leftmostBits = (k >> (28 - shifts)) & mask;
    return ((k << shifts) & mask) | leftmostBits;
}

// Function to get the 16 keys for the 16 rounds of DES
void generateSubKeys(uint64_t originalKey, uint64_t* roundKeys) {
    // Apply PC-1 to get the 56-bit key
    uint64_t key56 = permute(originalKey, PC1, 56);

    // Splitting the key into two halves
    uint64_t left = key56 >> 28;
    uint64_t right = key56 & 0x0FFFFFFF; // Mask to ensure the right half is 28 bits

    // Generating 16 subkeys
    for (int i = 0; i < 16; i++) {
        // Left shifts
        left = leftShift(left, shifts[i]);
        right = leftShift(right, shifts[i]);

        // Combining the two halves and applying PC-2
        uint64_t combinedKey = (left << 28) | right;
        roundKeys[i] = permute(combinedKey, PC2, 48);
    }
}



// Encrypts a bloc using 3DES
void encrypt(char *input, char *output, char *key){
   

}