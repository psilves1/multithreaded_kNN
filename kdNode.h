#ifndef KDNODE_H
#define KDNODE_H

#include <vector>
#include <mutex>

class kdNode
{
private:
    float* val; //value stored within the node

    int depth;
    double distance;

    kdNode* left;
    kdNode* right;

    

public:

    std::mutex mtx;

    kdNode();

    kdNode(float vals[], int nodeDepth);

    float* getVals();

    void setLeft(kdNode* n);

    void setRight(kdNode* n);

    kdNode* getLeft();

    kdNode* getRight();

    float getVal(int valIndex);

    void insert(kdNode* newNode, int dim);

    void incDepth(int dim);

    int getDepth();

    void getKNN(kdNode kNN[], kdNode *q, kdNode* root, int k, int dim, std::vector<kdNode*>* vec);

    void setDistance(double d);

    double getDistance();

    float* getFloats();

    void setFloats(void* ptr);

    kdNode* nearestNeighbor(kdNode* root, kdNode* target, int dims, std::vector<kdNode*>* vec, int k);

};

#endif