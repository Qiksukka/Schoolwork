/*SKINNY-128-384 cipher implementation in C
 *BY: Santeri Röning
 *santeri.roning@tuni.fi
 */

#include <stdint.h>
#include "skinny.h"
#include <stdio.h>
#include <stdint.h>

/*SKINNY Sbox taken straight from the paper*/
const unsigned char sbox[256] = {
0x65 ,0x4c ,0x6a ,0x42 ,0x4b ,0x63 ,0x43 ,0x6b ,0x55 ,0x75 ,0x5a ,0x7a ,0x53 ,0x73 ,0x5b ,0x7b ,
0x35 ,0x8c ,0x3a ,0x81 ,0x89 ,0x33 ,0x80 ,0x3b ,0x95 ,0x25 ,0x98 ,0x2a ,0x90 ,0x23 ,0x99 ,0x2b ,
0xe5 ,0xcc ,0xe8 ,0xc1 ,0xc9 ,0xe0 ,0xc0 ,0xe9 ,0xd5 ,0xf5 ,0xd8 ,0xf8 ,0xd0 ,0xf0 ,0xd9 ,0xf9 ,
0xa5 ,0x1c ,0xa8 ,0x12 ,0x1b ,0xa0 ,0x13 ,0xa9 ,0x05 ,0xb5 ,0x0a ,0xb8 ,0x03 ,0xb0 ,0x0b ,0xb9 ,
0x32 ,0x88 ,0x3c ,0x85 ,0x8d ,0x34 ,0x84 ,0x3d ,0x91 ,0x22 ,0x9c ,0x2c ,0x94 ,0x24 ,0x9d ,0x2d ,
0x62 ,0x4a ,0x6c ,0x45 ,0x4d ,0x64 ,0x44 ,0x6d ,0x52 ,0x72 ,0x5c ,0x7c ,0x54 ,0x74 ,0x5d ,0x7d ,
0xa1 ,0x1a ,0xac ,0x15 ,0x1d ,0xa4 ,0x14 ,0xad ,0x02 ,0xb1 ,0x0c ,0xbc ,0x04 ,0xb4 ,0x0d ,0xbd ,
0xe1 ,0xc8 ,0xec ,0xc5 ,0xcd ,0xe4 ,0xc4 ,0xed ,0xd1 ,0xf1 ,0xdc ,0xfc ,0xd4 ,0xf4 ,0xdd ,0xfd ,
0x36 ,0x8e ,0x38 ,0x82 ,0x8b ,0x30 ,0x83 ,0x39 ,0x96 ,0x26 ,0x9a ,0x28 ,0x93 ,0x20 ,0x9b ,0x29 ,
0x66 ,0x4e ,0x68 ,0x41 ,0x49 ,0x60 ,0x40 ,0x69 ,0x56 ,0x76 ,0x58 ,0x78 ,0x50 ,0x70 ,0x59 ,0x79 ,
0xa6 ,0x1e ,0xaa ,0x11 ,0x19 ,0xa3 ,0x10 ,0xab ,0x06 ,0xb6 ,0x08 ,0xba ,0x00 ,0xb3 ,0x09 ,0xbb ,
0xe6 ,0xce ,0xea ,0xc2 ,0xcb ,0xe3 ,0xc3 ,0xeb ,0xd6 ,0xf6 ,0xda ,0xfa ,0xd3 ,0xf3 ,0xdb ,0xfb ,
0x31 ,0x8a ,0x3e ,0x86 ,0x8f ,0x37 ,0x87 ,0x3f ,0x92 ,0x21 ,0x9e ,0x2e ,0x97 ,0x27 ,0x9f ,0x2f ,
0x61 ,0x48 ,0x6e ,0x46 ,0x4f ,0x67 ,0x47 ,0x6f ,0x51 ,0x71 ,0x5e ,0x7e ,0x57 ,0x77 ,0x5f ,0x7f ,
0xa2 ,0x18 ,0xae ,0x16 ,0x1f ,0xa7 ,0x17 ,0xaf ,0x01 ,0xb2 ,0x0e ,0xbe ,0x07 ,0xb7 ,0x0f ,0xbf ,
0xe2 ,0xca ,0xee ,0xc6 ,0xcf ,0xe7 ,0xc7 ,0xef ,0xd2 ,0xf2 ,0xde ,0xfe ,0xd7 ,0xf7 ,0xdf ,0xff
};

//Switchrows Permutation
const unsigned char switchRowsP[4][4] = {{0,1,2,3}, {7,4,5,6}, {10,11,8,9}, {13,14,15,12}};

//Roundconstant Permutation
const unsigned char roundConstant[62] = {
		0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3E, 0x3D, 0x3B, 0x37, 0x2F,
		0x1E, 0x3C, 0x39, 0x33, 0x27, 0x0E, 0x1D, 0x3A, 0x35, 0x2B,
		0x16, 0x2C, 0x18, 0x30, 0x21, 0x02, 0x05, 0x0B, 0x17, 0x2E,
		0x1C, 0x38, 0x31, 0x23, 0x06, 0x0D, 0x1B, 0x36, 0x2D, 0x1A,
		0x34, 0x29, 0x12, 0x24, 0x08, 0x11, 0x22, 0x04, 0x09, 0x13,
		0x26, 0x0c, 0x19, 0x32, 0x25, 0x0a, 0x15, 0x2a, 0x14, 0x28,
		0x10, 0x20};

// TweaKey permutations
const unsigned char TKP[4][4] = {{9,15,8,13}, {10,14,12,11}, {0,1,2,3}, {4,5,6,7}};

// SBox tricks borrowed from TinyAES implementation of AES
void subBox(unsigned char currentState[4][4])
{
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            currentState[i][j] = sbox[currentState[i][j]];
        }
    }
}

// MixColumns
void mixColumn(unsigned char currentState[4][4])
{
    unsigned char tempState;

	for(int i = 0; i < 4; i++){
        currentState[1][i] = currentState[1][i] ^ currentState[2][i];
        currentState[2][i] = currentState[2][i] ^ currentState[0][i];
        currentState[3][i] = currentState[3][i] ^ currentState[2][i];

        tempState=currentState[3][i];
        currentState[3][i] = currentState[2][i];
        currentState[2][i] = currentState[1][i];
        currentState[1][i] = currentState[0][i];
        currentState[0][i] = tempState;
	}
}

// Swiftrows:
void shiftRows(unsigned char currentState[4][4])
{
	int position;
	unsigned char tempState[4][4];
	for (int i = 0; i < 4; i++){
		for (int j = 0; j < 4; j++){
			position = switchRowsP[i][j];
			tempState[i][j] = currentState[position / 4][position % 4];
		}
	}
	
	for (int i = 0; i < 4; i++){
		for (int j = 0; j < 4; j ++){
			currentState[i][j] = tempState[i][j];
		}
	}
}


/* Roundconstant work - Had a bit of help with the RoundConstant work with the masks and shifts. */
void addConst(unsigned char currentState[4][4], int i)
{
	// XOR with 4 first bits of roundconstants  w/ currentState
	currentState[0][0] = currentState[0][0] ^ (roundConstant[i] & 0xf);
	// Take the rc5 and rc4 via shifting and xor it with currentState
	currentState[1][0] = currentState[1][0] ^ ((roundConstant[i]>>4) & 0x3);
	currentState[2][0] = currentState[2][0] ^ 0x2;
}

// Tweakey
void tweakeyWork (unsigned char currentState[4][4], unsigned char tweaKey[3][4][4])
{
	unsigned char position;
	unsigned char tempKey[3][4][4];
	
	// Subtweakey XOR currentState first two rows
	for (int i = 0; i < 2; i++){
		for (int j = 0; j < 4; j++){
			currentState[i][j] = currentState[i][j] ^ tweaKey[0][i][j] ^ tweaKey[1][i][j] ^ tweaKey[2][i][j];
		}
	}
	
	// Permutations to the subtweakey
	for (int i = 0; i < 3; i++){
		for (int j = 0; j < 4; j++){
			for (int k = 0; k < 4; k++){
				position = TKP[j][k];
				tempKey[i][j][k] = tweaKey[i][position / 4][position % 4];
			}
		}
	}
	// Update Tweakey LFSRs
	for (int i = 0; i < 3; i++){
		for (int j = 0; j < 2; j++){
			for (int k = 0; k < 4; k++){
				if (i == 1){
					// x7|x6|x5|x4|x3|x2|x1|x0->x6|x5|x4|x3|x2|x1|x0|x7 XOR x5
					tempKey[i][j][k]=((tempKey[i][j][k]>>7)&0x01)^((tempKey[i][j][k]>>5)&0x01)^((tempKey[i][j][k]<<1)&0xFE);
				}
				else if (i == 2){
					// x7|x6|x5|x4|x3|x2|x1|x0->x0 XOR x6|x7|x6|x5|x4|x3|x2|x1
					tempKey[i][j][k]=((tempKey[i][j][k]<<7)&0x80)^((tempKey[i][j][k]<<1)&0x80)^((tempKey[i][j][k]>>1)&0x7F);
				}
			}
		}
	}
	for(int i = 0; i < 3; i++){
        for(int j = 0; j < 4; j++){
            for(int k = 0; k < 4; k++){
                tweaKey[i][j][k]=tempKey[i][j][k];
            }
        }
    }
}

/**
 * SKINNY-128-384 block cipher encryption.
 * Under 48-byte tweakey at k, encrypt 16-byte plaintext at p and store the 16-byte output at c.
 */
void skinny(unsigned char *c, const unsigned char *p, const unsigned char *k) {

    unsigned char currentState[4][4];
    unsigned char tweaKey[3][4][4];

	// Init the state
    for (int i = 0; i < 4; i++){
		for (int j = 0; j < 4; j++){
	        currentState[i][j]= p[i*4+j];
		}
    }
    // Init the tweakey
    for (int i = 0; i < 4; i++){
		for (int j = 0; j < 4; j++){
        	tweaKey[0][i][j] = k[i*4+j];
        	tweaKey[1][i][j] = k[i*4+j+16];
        	tweaKey[2][i][j] = k[i*4+j+32];
		}
    }

    for (int i = 0; i < 56; i++){
        subBox(currentState);
		addConst(currentState, i);
		tweakeyWork(currentState, tweaKey);
        shiftRows(currentState);
        mixColumn(currentState);
	}
    for (int i = 0; i < 4; i++){
		for (int j = 0; j < 4; j++){
			c[i*4+j] = currentState[i][j];
		}
    }
}
