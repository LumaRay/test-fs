typedef unsigned long obj_type;

struct HashNode {
	obj_type* mpObject;
	uint8_t* mpHash;
	struct HashNode** mChildNodes;
	int mAddressBits;
};

struct HashTree {
	struct HashNode mRootNode;
};

void HashTreeInit(struct HashTree* pHashTree);

void HashTreeAdd(struct HashTree* this, uint8_t* hash, obj_type* object);

obj_type* HashTreeGet(struct HashTree* this, uint8_t* hash);

void HashNodeDestroy(struct HashNode* this);
