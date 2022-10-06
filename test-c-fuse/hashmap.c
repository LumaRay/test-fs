#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/timeb.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include "hashmap.h"

#define HASH_SIZE	32

#define MAX_HASHES	(8 * 1024 * 1024)
#define ROOT_NODE_ADDRESS_BITS (3*8 + 2)
#define CHILD_NODE_ADDRESS_BITS (1*8 - 6)

//#define MAX_HASHES	(32 * 1024 * 1024)
//#define ROOT_NODE_ADDRESS_BITS (3*8 + 3)
//#define CHILD_NODE_ADDRESS_BITS (1*8 - 7)

//#define MAX_HASHES	(8 * 1024 * 1024)
//#define ROOT_NODE_ADDRESS_BITS (3*8 + 2)
//#define CHILD_NODE_ADDRESS_BITS (1*8 - 6)

//#define MAX_HASHES	(32 * 1024 * 1024)
//#define ROOT_NODE_ADDRESS_BITS (3*8 + 3)
//#define CHILD_NODE_ADDRESS_BITS (1*8)

//typedef unsigned long int uint64_t;
//typedef unsigned int uint32_t;
//typedef unsigned short uint16_t;
//typedef unsigned char uint8_t;

//typedef int obj_type;

//uint8_t** hashes;
uint8_t (*hashes)[HASH_SIZE];
/*
struct HashNode {
	obj_type* mpObject;
	uint8_t* mpHash;
	struct HashNode** mChildNodes;
	int mAddressBits;
};

struct HashTree {
	struct HashNode mRootNode;
};
*/
void MakeHashTree(struct HashTree* pHashTree, int hashes_count);

void TestHashTree(struct HashTree* pHashTree, int hashes_count);

/*void HashTreeAdd(struct HashTree* pHashTree, uint8_t* pHash, obj_type* pObject);

obj_type* HashTreeGet(struct HashTree* this, uint8_t* hash);

void HashTreeInit(struct HashTree* pHashTree);*/

//struct HashNode* HashNodeCreateEmpty();

struct HashNode* HashNodeCreate(uint8_t* pHash, obj_type* pObject);

void HashNodeDestroy(struct HashNode* this);

void HashNodeSetObject(struct HashNode* this, uint8_t* pHash, obj_type* pObject, const int iByteOffset);

obj_type* HashNodeGetObject(struct HashNode* this, uint8_t* pHash, const int iByteOffset);

void GenerateRandomHashes(size_t hashes_count);

void DestroyRandomHashes();

int main_() {
	time_t seconds_start, seconds_end;
	struct timeb start, end;
	
	size_t hashes_count = MAX_HASHES;

	printf("Generating hashes...");
	GenerateRandomHashes(hashes_count);
	printf("Done\n");
	
	struct HashTree hashTree;
	HashTreeInit(&hashTree);
	//time(&seconds_start);
	ftime(&start);
	MakeHashTree(&hashTree, hashes_count);
	//time(&seconds_end);
	ftime(&end);
	//printf("MakeHashTree %lf \n", seconds_end - seconds_start);
	printf("MakeHashTree %d \n", (int) (1000.0 * (end.time - start.time) + (end.millitm - start.millitm)));
	
	//time(&seconds_start);
	ftime(&start);
	TestHashTree(&hashTree, hashes_count);
	//time(&seconds_end);
	ftime(&end);
	//printf("TestHashTree %d \n", seconds_end - seconds_start);
	printf("TestHashTree %d \n", (int) (1000.0 * (end.time - start.time) + (end.millitm - start.millitm)));

	HashNodeDestroy(&(hashTree.mRootNode));
	
	DestroyRandomHashes();
	
	return 0;
}

void HashTreeInit(struct HashTree* pHashTree) {
	pHashTree->mRootNode.mChildNodes = NULL;
	pHashTree->mRootNode.mpObject = NULL;
	pHashTree->mRootNode.mpHash = NULL;
	pHashTree->mRootNode.mAddressBits = ROOT_NODE_ADDRESS_BITS;
}

/*struct HashNode* HashNodeCreateEmpty() {
	struct HashNode* pNode = malloc(sizeof(struct HashNode));
	pNode->mChildNodes = NULL;
	pNode->mpObject = NULL;
	pNode->mpHash = NULL;
	pNode->mAddressBits = 3*8 + 7;
	return pNode;
}*/

struct HashNode* HashNodeCreate(uint8_t* pHash, obj_type* pObject) {
	struct HashNode* pNode = malloc(sizeof(struct HashNode));
	pNode->mChildNodes = NULL;
	pNode->mpObject = pObject;
	pNode->mpHash = pHash;
	pNode->mAddressBits = CHILD_NODE_ADDRESS_BITS;
	return pNode;
}

void HashNodeDestroy(struct HashNode* this) {
	if (this->mpObject) {
		free(this->mpObject);
	}

	if (this->mpHash) {
		free(this->mpHash);
	}

	if (this->mChildNodes) {
		for (int idxChildNode = 0; idxChildNode < 1 << this->mAddressBits; idxChildNode++)
			if (this->mChildNodes[idxChildNode]) {
				HashNodeDestroy(this->mChildNodes[idxChildNode]);
				free(this->mChildNodes[idxChildNode]);
			}
		free(this->mChildNodes);
	}
}

void HashNodeSetObject(struct HashNode* this, uint8_t* pHash, obj_type* pObject, const int iByteOffset) {
	if (!this->mChildNodes)
		//this->mChildNodes = calloc((uint64_t)1 << this->mAddressBits, sizeof(struct HashNode));
		this->mChildNodes = calloc((uint64_t)1 << this->mAddressBits, sizeof(void*));
	if (this->mpHash && this->mpObject) {
		uint8_t* pExistingHash = this->mpHash;
		int iExistingHashAddress = ~((uint32_t)-1 << this->mAddressBits) & *((uint32_t*)&pExistingHash[iByteOffset]);
		this->mChildNodes[iExistingHashAddress] = HashNodeCreate(this->mpHash, this->mpObject);
		this->mpHash = NULL;
		this->mpObject = NULL;
	}
	int iHashAddress = ~((uint32_t)-1 << this->mAddressBits) & *((uint32_t*)&pHash[iByteOffset]);
	struct HashNode** ppChildNode = &this->mChildNodes[iHashAddress];
	if (*ppChildNode == NULL || iByteOffset == HASH_SIZE - 1) {
		*ppChildNode = HashNodeCreate(pHash, pObject);
	}
	else {
		HashNodeSetObject(*ppChildNode, pHash, pObject, iByteOffset + (int)ceil((float)this->mAddressBits / 8));
	}
}

obj_type* HashNodeGetObject(struct HashNode* this, uint8_t* pHash, const int iByteOffset) {
	if (this->mpHash && this->mpObject) if (memcmp(this->mpHash + iByteOffset, pHash + iByteOffset, (size_t)HASH_SIZE - iByteOffset) == 0) {
		return this->mpObject;
	}
	if (!this->mChildNodes) {
		//printf("No children! %ld\n", (unsigned long)this);
		return NULL;
    }
	int iHashAddress = ~((uint32_t)-1 << this->mAddressBits) & *((uint32_t*)&pHash[iByteOffset]);
	struct HashNode** ppChildNode = &this->mChildNodes[iHashAddress];
	if (*ppChildNode == NULL || iByteOffset + (int)ceil((float)this->mAddressBits / 8) >= HASH_SIZE - 1) {
		//printf("Hash overflow!\n");
		return NULL;
	}
	return HashNodeGetObject(*ppChildNode, pHash, iByteOffset + (int)ceil((float)this->mAddressBits / 8));
}

void MakeHashTree(struct HashTree* pHashTree, int hashes_count) {
	for (int idxHash = 0; idxHash < hashes_count; idxHash++) {
		uint8_t* hash = hashes[idxHash];
		unsigned long in = hash[0];
		HashTreeAdd(pHashTree, hash, &in);
	}
}

void TestHashTree(struct HashTree* pHashTree, int hashes_count) {
	for (int idxHash = 0; idxHash < hashes_count; idxHash++) {
		uint8_t* hash = hashes[idxHash];
		obj_type* b = HashTreeGet(pHashTree, hash);
		uint8_t hashByte = hash[0];
		if (*HashTreeGet(pHashTree, hash) != (obj_type)hash[0]) printf("TestHashTree!\n");
	}
}

void HashTreeAdd(struct HashTree* this, uint8_t* hash, obj_type* object) {
	uint8_t* const hashCopy = malloc(HASH_SIZE);
	memcpy(hashCopy, hash, HASH_SIZE);
	obj_type* const pObjectCopy = malloc(sizeof(obj_type));
	memcpy(pObjectCopy, object, sizeof(obj_type));
	HashNodeSetObject(&(this->mRootNode), hashCopy, pObjectCopy, 0);
}

obj_type* HashTreeGet(struct HashTree* this, uint8_t* hash) {
	return HashNodeGetObject(&(this->mRootNode), hash, 0);
}

//void HashTreeDestroy(HashTree& this) {
//	memfree(this.mRootNode)
//}

void GenerateRandomHashes(size_t hashes_count) {
	hashes = calloc(hashes_count, sizeof(uint8_t) * HASH_SIZE);

	for (int idxHash = 0; idxHash < hashes_count; idxHash++) {
		srand((uint32_t)time(0) + idxHash);
		for (int idxByte = 0; idxByte < HASH_SIZE; idxByte += 2)
			*(uint16_t*)&hashes[idxHash][idxByte] = (uint16_t)rand();
	}
}

void DestroyRandomHashes() {
	free(hashes);
}
