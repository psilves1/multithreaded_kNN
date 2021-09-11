#include "kdNode.h"

#include <memory>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <vector>

kdNode::kdNode()
{
    left = nullptr;
    right = nullptr;
    depth = 0;
    float vals[] = {0,0,0,0,0};
    val = vals;
    distance = -1.0;
}

kdNode::kdNode(float vals[], int nodeDepth)
{
    val = vals;
    depth = nodeDepth;
    left = nullptr;
    right = nullptr;
    distance = -1.0;
    //nodeDepth should be modded before being passed in
}


void kdNode::setLeft(kdNode *n)
{
    left = n;
}

void kdNode::setRight(kdNode *n)
{
    right = n;
}

kdNode *kdNode::getLeft()
{
    return left;
}

kdNode *kdNode::getRight()
{
    return right;
}

float kdNode::getVal(int valIndex)
{
    return val[valIndex];
}

float* kdNode::getVals(){
    return val;
}

void kdNode::insert(kdNode *newNode, int dim)
{
    
    //std::cout<<dim<<std::endl;

    //1) know which variable level we are
    //2)we need to then compare
    //3) we need to recursively add it to the tree
    newNode->incDepth(dim);

    if (newNode->getVal(depth) > val[depth])
    {
        //std::cout << "added to the right" << std::endl;
        //insert to the right
        if (getRight() == NULL)
        {
            setRight(newNode); //if no right node, add right node
            mtx.unlock();
        }
        else
        { //if there is a right node, call right.insert()
            mtx.unlock();
            while(!right->mtx.try_lock()){}
            right->insert(newNode, dim);
            right->mtx.unlock();
        }
    }
    else
    {
        //std::cout<< "added left" << std::endl;
        if (getLeft() == NULL)
        {
            setLeft(newNode);
            mtx.unlock();
            //std::cout<<getLeft()<<std::endl;
        }
        else
        {
            while(!left->mtx.try_lock()){}
            left->insert(newNode,dim);
            left->mtx.unlock();
            //std::cout<<getLeft()<<std::endl;
        }
    }
}

void kdNode::incDepth(int dim)
{
    depth++;
    depth %= dim; //change 3 to whatever the "k" value is
}

int kdNode::getDepth()
{
    return depth;
}


bool isTopK(std::vector<double> dists, kdNode *q, kdNode *n, uint64_t dim, int* p)
{
    int arr[] = {0, 0, 0};

    for (int i = 0; i < (int)dim; i++)
    {
        arr[i] = (q->getVal(i) - n->getVal(i)) * (q->getVal(i) - n->getVal(i));
    }

    int dist = sqrt((arr[0] + arr[1] + arr[2]));

    //std::cout<<n->getVal(0)<<std::endl;

    int greatest = dist;
    int index = 0;


    for (int i = 0; i < (int)dim; i++)
    {
        if (dists[i] > greatest)
        {
            greatest = dists[i];
            index = i;
        }
    }

    //std::cout<<dists[0]<<std::endl;

    if (greatest > dist)
    {
        arr[index] = dist;
        *p = index;
        return true;
    }




    return false;
}

void kdNode::setDistance(double d){
    distance = d;
}

double kdNode::getDistance(){
    return distance;
}

kdNode* closest(kdNode* target, kdNode* temp, kdNode* root){

    double rootDist = 0;
    double tempDist = 0;

    if(temp == nullptr){    //somtimes temp will be a nullptr. Must check
        return root;
    }

    for(int i = 0; i < 1; i++){
        rootDist += (root->getVal(i) - target->getVal(i)) * (root->getVal(i) - target->getVal(i));
        tempDist += (temp->getVal(i) - target->getVal(i)) * (temp->getVal(i) - target->getVal(i));
    }

    if(rootDist < tempDist) return root;

    return temp;
}

float* kdNode::getFloats(){
    return val;
}

void kdNode::setFloats(void* ptr){
    val = (float*)ptr;
}

double distanceSquared(kdNode* target, kdNode* best, int dims){
    double dist = 0;

    if(best == nullptr){
        return INFINITY;
    }


    for(int i = 0; i < dims; i++){
        dist+= (target->getVal(i) - best->getVal(i)) * (target->getVal(i) - best->getVal(i));
    }
    return dist;
}


int insertBest(std::vector<kdNode*>* vec, kdNode* best, int k){
    
    //std::cout<<vec->size()<<std::endl;
    if(best == nullptr){
        return 0;
    }

    /*
    if(std::find_if(vec->begin(), vec->end(), 

    [=](kdNode* curr)->bool
    {
        if(curr == nullptr) return false;
        for(int i = 0; i < dim; i++){
            if(curr->getVal(i) != best->getVal(i)) 
                return true;
        }
        return false;
    } 

    ) != vec->end()){ //check if we're already in the vector
           //std::cout<< "here" << std::endl;
            return 1;
    }
    */
   if(std::find(vec->begin(), vec->end(), best) != vec->end()){
       return 1;
   }

    //std::cout<<(vec == nullptr)<<std::endl;

    for(int i = 0; i < (int)vec->size(); i++){


        if(vec->at(i) == nullptr){
            vec->insert(vec->begin()+i, best);
            vec->pop_back();
            return 2;
        }

        
        if(best->getDistance() < vec->at(i)->getDistance()){
            vec->insert(vec->begin()+i, best);
            if((int)vec->size() > (int)k){
                vec->pop_back();
            }
            return 3;
        }       
    }


    if((int)vec->size() < (int)k){ 
        vec->push_back(best);
        return 4;
    }
    return 5;    
}


kdNode* kdNode::nearestNeighbor(kdNode* root, kdNode* target, int dims, 
                                std::vector<kdNode*>* vec, int k){
    
    if(root == nullptr) return nullptr;

    if((root->getVal(0) < 0.00001 && root->getVal(0) > 0) || 
        (root->getVal(0) > -0.00001 && root->getVal(0) < 0)){

            return vec->at(0);
    }

    root->setDistance(distanceSquared(target, root, dims));
    

    insertBest(vec, root, k);

    //std::cout<<root->getLeft()<<std::endl;

    int depth = root->getDepth();

    kdNode* nextBranch;
    kdNode* otherBranch;

    if(target->getVal(depth) < root->getVal(depth)){
        nextBranch = root->getLeft();
        otherBranch = root->getRight();
    }else{
        nextBranch = root->getRight();
        otherBranch = root->getLeft();
    }

    kdNode* temp = nearestNeighbor(nextBranch, target, dims, vec, k);

    insertBest(vec, temp, k); //choice between the two past in nodes and find out who is closer 


    double radiusSquared = distanceSquared(target, vec->at(vec->size()-1), dims);

        //std::cout<<"here"<<std::endl;


    double dist = target->getVal(depth) - root->getVal(depth);



    if(radiusSquared >= dist*dist){
        temp = nearestNeighbor(otherBranch, target, dims, vec, k);
        insertBest(vec, temp, k);
    }

    return temp;
}














