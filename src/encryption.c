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


// Initial Permutation (IP) Table
int IP[] = {
    58, 50, 42, 34, 26, 18, 10, 2,
    60, 52, 44, 36, 28, 20, 12, 4,
    62, 54, 46, 38, 30, 22, 14, 6,
    64, 56, 48, 40, 32, 24, 16, 8,
    57, 49, 41, 33, 25, 17, 9, 1,
    59, 51, 43, 35, 27, 19, 11, 3,
    61, 53, 45, 37, 29, 21, 13, 5,
    63, 55, 47, 39, 31, 23, 15, 7
};

// Final Permutation (FP) Table (inverse of IP)
int FP[] = {
    40, 8, 48, 16, 56, 24, 64, 32,
    39, 7, 47, 15, 55, 23, 63, 31,
    38, 6, 46, 14, 54, 22, 62, 30,
    37, 5, 45, 13, 53, 21, 61, 29,
    36, 4, 44, 12, 52, 20, 60, 28,
    35, 3, 43, 11, 51, 19, 59, 27,
    34, 2, 42, 10, 50, 18, 58, 26,
    33, 1, 41, 9, 49, 17, 57, 25
};

// Placeholder S-box (substitution box)
int S[8][4][16] = {
    {
        {14, 4, 13, 1, 2, 15, 11, 8, 3, 10, 6, 12, 5, 9, 0, 7}, // S1
        {0, 15, 7, 4, 14, 2, 13, 1, 10, 6, 12, 11, 9, 5, 3, 8},
        {4, 1, 13, 8, 6, 2, 11, 15, 12, 9, 7, 3, 10 ,5, 14, 0},
        {15, 12, 8, 2, 4, 9, 1, 7 ,5 ,11, 3, 14, 10, 0, 6, 13}
    },
    {
        {15 ,1 ,8 ,14 ,6 ,11 ,3 ,4 ,9 ,7 ,2 ,13 ,12 ,0 ,5 ,10}, // S2
        {3 ,13 ,4 ,7 ,15 ,2 ,8 ,14 ,12 ,0 ,1 ,10 ,6 ,9 ,11 ,5},
        {0 ,14 ,7 ,11 ,10 ,4 ,13 ,1 ,5 ,8 ,12 ,6 ,9 ,3 ,2 ,15},
        {13 ,8 ,10 ,1 ,3 ,15 ,4 ,2 ,11 ,6 ,7 ,12 ,0 ,5 ,14 ,9}
    },
    {
        {10 ,0 ,9 ,14 ,6 ,3 ,15 ,5 ,1 ,13 ,12 ,7 ,11 ,4 ,2 ,8}, // S3
        {13 ,7 ,0 ,9 ,3 ,4 ,6 ,10 ,2 ,8 ,5 ,14 ,12 ,11 ,15 ,1},
        {13 ,6 ,4 ,9 ,8 ,15 ,3 ,0 ,11 ,1 ,2 ,12 ,5 ,10 ,14 ,7},
        {1 ,10 ,13 ,0 ,6 ,9 ,8 ,7 ,4 ,15 ,14 ,3 ,11 ,5 ,2 ,12}
    },
};
// Permutation table for final permutation step
int P[32] = {
    16, 7, 20, 21, 29, 12, 28, 17,
    1, 15, 23 ,26 ,5 ,18 ,31 ,10,
    2 ,8 ,24 ,14 ,32 ,27 ,3 ,9,
    19 ,13 ,30 ,6 ,22 ,11 ,4 ,25
};

int E[48] = {
    32,  1,  2,  3,  4,  5,
     4,  5,  6,  7,  8,  9,
     8,  9, 10, 11, 12, 13,
    12, 13, 14, 15, 16, 17,
    16, 17, 18, 19, 20, 21,
    20, 21, 22, 23, 24, 25,
    24, 25, 26, 27, 28, 29,
    28, 29, 30, 31, 32,  1
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

uint64_t desEncryptBlock(uint64_t input, uint64_t* subkeys) {
    // Initial permutation
    uint64_t permutedInput = permute(input, IP, 64);

    // Splitting the input into two halves
    uint64_t left = permutedInput >> 32;
    uint64_t right = permutedInput & 0xFFFFFFFF; // Mask to ensure the right half is 32 bits

    // 16 rounds of DES
    for (int i = 0; i < 16; i++) {
        uint64_t temp = right;
        right = left ^ f(right, subkeys[i]);
        left = temp;
    }

    // Combining the two halves
    uint64_t combined = (left << 32) | right;

    // Final permutation
    return permute(combined,FP, 64);
}

uint64_t desDecryptBlock(uint64_t input, uint64_t* subkeys) {
    // Initial permutation
    uint64_t permutedInput = permute(input, IP, 64);

    // Splitting the input into two halves
    uint64_t left = permutedInput >> 32;
    uint64_t right = permutedInput & 0xFFFFFFFF;

    // 16 rounds of DES decryption
    for (int i = 15; i >= 0; i--) {
        uint64_t temp = left;
        left = right ^ f(left, subkeys[i]); // Note the reversal of roles for left and right
        right = temp;
    }

    // Combining the two halves
    uint64_t combined = (right << 32) | left; // Note that right and left are swapped

    // Final permutation
    return permute(combined, FP, 64);
}

// The Feistel function
uint64_t f(uint64_t r, uint64_t k) {
    // Expansion permutation
    uint64_t expandedR = permute(r, E, 48);

    // XOR with the key
    uint64_t xored = expandedR ^ k;

    // Substitution boxes
    uint64_t substituted = 0;
    for (int i = 0; i < 8; i++) {
        // Get the bits we are currently processing
        uint64_t bits = (xored >> (42 - i * 6)) & 0x3F;

        // Get the row and column numbers
        int row = ((bits & 0x20) >> 4) | (bits & 1);
        int col = (bits >> 1) & 0xF;

        // Look up the value in the S-box table
        substituted |= (uint64_t)S[i][row][col] << (32 - 4 * (i + 1));
    }

    // Straight permutation
    return permute(substituted, P, 32);
}

// Corrected encrypt function
void encrypt(uint64_t input, uint64_t *output, uint64_t key[3]){
    uint64_t roundKeys1[16], roundKeys2[16], roundKeys3[16];

    // Generate subkeys for all three keys
    generateSubKeys(key[0], roundKeys1);
    generateSubKeys(key[1], roundKeys2);
    generateSubKeys(key[2], roundKeys3);

    // Encrypt with first key
    uint64_t temp = desEncryptBlock(input, roundKeys1);

    // Decrypt with second key (using a decryption function, not shown here)
    temp = desDecryptBlock(temp, roundKeys2);

    // Encrypt with third key
    *output = desEncryptBlock(temp, roundKeys3);
}

// Corrected decrypt function
void decrypt(uint64_t input, uint64_t *output, uint64_t key[3]){
    uint64_t roundKeys1[16], roundKeys2[16], roundKeys3[16];

    // Generate subkeys for all three keys
    generateSubKeys(key[0], roundKeys1);
    generateSubKeys(key[1], roundKeys2);
    generateSubKeys(key[2], roundKeys3);

    // Decrypt with third key
    uint64_t temp = desDecryptBlock(input, roundKeys3);

    // Encrypt with second key (using a decryption function, not shown here)
    temp = desEncryptBlock(temp, roundKeys2);

    // Decrypt with first key
    *output = desDecryptBlock(temp, roundKeys1);
}

int main() {
    // Test data and key
    uint64_t input = 0x0123456789ABCDEF; // Example plaintext block
    uint64_t output;                     // Will hold the encrypted data
    uint64_t decrypted;                  // Will hold the decrypted data
    uint64_t keys[3] = {
        0x133457799BBCDFF1,             // Example key 1
        0x123456789ABCDEF0,             // Example key 2
        0xFEDCBA9876543210              // Example key 3
    };

    // Encrypt the data
    encrypt(input, &output, keys);

    // Decrypt the data
    decrypt(output, &decrypted, keys);

    // Compare the decrypted data with the original input
    if (input == decrypted) {
        printf("Success! The decrypted data matches the original input.\n");
    } else {
        printf("Failure! The decrypted data does not match the original input.\n");
    }

    return 0;
}


