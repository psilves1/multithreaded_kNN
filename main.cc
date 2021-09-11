#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <x86intrin.h>
#include <immintrin.h>
#include <future>

#include "kdNode.h"
#include "dump.h"

//std::mutex mtx;

void deleteTree(kdNode* root, int dim);
void addNode(kdNode* root, float* points, int dims, int size, int last);
std::vector<float>* qSearch(std::vector<float*>* vec, 
                            kdNode* root, int dim, int k, int index, 
                            std::vector<std::vector<float>*>* vecStore);

inline auto start_tsc() {
    _mm_lfence();
    auto tsc = __rdtsc();
    _mm_lfence();
    return tsc;
}

inline auto stop_tsc() {
    unsigned int aux;
    auto tsc = __rdtscp(&aux);
    _mm_lfence();
    return tsc;
}


int main(int argc, char* argv[]){

    //float arr1[] = {0, 0, 0};

    unsigned long long start = start_tsc();

    if(argc != 5){
        std::cout << "usage: ./k-nn n_cores training_file query_file result_file" <<std::endl;
        exit(2);
    }

    const int numCores = atoi(argv[1]);
    char* trainingFile = argv[2];
    char* queryFile = argv[3];
    char* resultFile = argv[4];


    kdNode *root = nullptr;

    uint64_t training_ID= 0;
    uint64_t dim= 0;
    uint64_t numPoints = 0;


    root = dumpTrainingGetRoot(trainingFile, &dim, &numPoints);

    float* arr = nullptr; 


    arr = dumpTraining(trainingFile, &training_ID, &dim, &numPoints);

    //std::thread threads[numCores];
    std::vector<std::thread> threads = std::vector<std::thread>();


    std::mutex mtx;
    int size = numPoints*dim/numCores; //2 is num of threads 

    int last = 0;

    for(int i = 0; i < numCores; i++){
        if(i == numCores - 1) last = 1;
        threads.push_back(std::thread(addNode, root , arr+(size*i),(int)dim, size, last));//arr[size*i]
    }

    for(int i = 0; i < numCores; i++){
        threads[i].join();
    }

    //threads.clear();
    

    std::vector<float*>* vec = new std::vector<float*>;

    uint64_t qID =0;
    uint64_t numQ=0;
    uint64_t k = 0;


    dumpQ(queryFile, vec, &qID, &numQ, &k); //fills vec with all the needed query data 

    kdNode* temp= new kdNode(vec->at(0), 0); 


    std::vector<kdNode*>* vec2 = new std::vector<kdNode*>();
    std::vector<std::vector<float>*>* vec3 = new std::vector<std::vector<float>*>();

    for(int i = 0; i < (int)numQ; i++){

        vec3->push_back(nullptr);    
    }


    //std::vector<float>* qSearch(std::vector<float*>* vec,
    // int numQ, kdNode* root, int dim, int k, int index)

    //std::async(qSearch, vec, (int)numQ, root, (int)dim, (int)k, q+i);

    //std::vector<std::future<std::vector<float>*>> futures;

    //std::cout<< "qNum: " << numQ <<std::endl;

    int iter = 0;

    for(int q = 0; q < (int)numQ/numCores; q++){

        for(int i = 0; i < numCores; i++){
            threads[i] = std::thread(qSearch, vec, root, (int)dim, (int)k, iter, vec3);
            iter++;
        }   

        //std::cout<< "iter: " << iter << std::endl;

        for(int i = 0; i < numCores; i++){
            threads[i].join();
        }
        if(iter >= (int)numQ){
            //std::cout<<"here"<<std::endl;
            break;
        }
    }

    

    int x = numQ%numCores;

    //this code takes care of the few query nodes that the above for loop will miss
    //due to the loss of percision. (EX: 100/8 = 12 not 12.5) This takes care of the 0.5
    if(x != 0){
        for(int i = 0; i < x; i++){
    
            qSearch(vec, root, (int)dim, (int)k, iter, vec3);
            iter++;
        }   
    }
    


    std::vector<float>* vec4 = new std::vector<float>;

    iter = 0;

    for(int i = 0; i < (int)vec3->size(); i++){
        for(int d = 0; d < (int)(k*dim); d++){
            if(vec3->at(i) == nullptr){
                continue;
            }
            //std::cout<<vec3->at(i)->at(d)<<std::endl;
            vec4->push_back(vec3->at(i)->at(d));
            //delete(vec3->at(i));
            iter++;
        }
    }

    //std::cout<<iter<<std::endl;


    /*
    for(int i = 0; i < (int)numQ; i++){//for every query we need to find thier kNN
        delete(temp);
        temp = new kdNode(vec->at(i), 0);

        for(int i = 0; i < (int)k; i++){
            vec2->push_back(nullptr);
        }   


        temp->nearestNeighbor(root, temp, (int)dim, vec2, (int)k);//vec2 contains a long list of neighbors 


        for(int i = 0; i < (int)k; i++){
            for(int d = 0; d < (int)dim; d++){ 
                
                if(vec2->at(i) == nullptr){
                    //std::cout<<"null"<<std::endl;
                    break;
                }
                vec3->push_back(vec2->at(i)->getVal(d));
            }
        }

        vec2->clear();

      
    }
    */

    
    
    dumpR(resultFile, vec4, training_ID, qID, numQ, dim, k); 
    //dumpRIn(resultFile);
    //dumpRIn("trainingData.dat");
    

    for (int i = 0; i < (int)vec->size(); i++){
        delete[] vec->at(i);
    }

    for(int i = 0; i < (int)vec3->size(); i++){
        delete(vec3->at(i));
        //delete[] vec3->at(i);
    }

    for(int i = 0; i < (int)vec2->size(); i++){
        delete(vec2->at(i));
        //delete[] vec2->at(i);
    }

    delete[] arr;
   delete(vec);
   delete(vec2);
   delete(vec3);
   delete(vec4);
   delete(temp);
   deleteTree(root, (int)dim);
   

    unsigned long long end = stop_tsc();

    std::cout << double(end - start)/(double(1800000000)) << std::endl;

   return 0;
}

void deleteTree(kdNode* root, int dim){
    if(root->getLeft()!=nullptr){
        deleteTree(root->getLeft(), dim);
    }
    if(root->getRight()!=nullptr){
        deleteTree(root->getRight(), dim);
    }
     
    //delete(root->getFloats());
    delete[] root->getFloats();
    delete(root);
}


void addNode(kdNode* root, float* points, int dims, int size, int last){
    int index = 0;

    float* arr;

    for(int j = 0; j < size/dims; j++){ 

        arr = new float[dims];
        for(int i = 0; i < dims; i++){           
            if(last == 1 && i == dims -1 && j == (size/dims)-1){ //prevents weird memory error
                arr[i] = (float)NULL;
                delete[] arr;
                return;
            } 
            arr[i] = points[index]; //error is right here
            index++;
        }
                //std::cout<<"here"<<std::endl;


        //while(!root->mtx.try_lock()){sleep(1);}
        //std::cout<<"here"<<std::endl;

        
        root->insert(new kdNode(arr, 0), dims);
        //delete(arr);
//root->mtx.unlock();
    }
}


std::vector<float>* qSearch(std::vector<float*>* vec, kdNode* root,
                             int dim, int k, int index, std::vector<std::vector<float>*>* vecStore){

    //std::cout<<"here1"<<std::endl;

    //kdNode* temp= new kdNode(vec->at(0), 0); 


    std::vector<kdNode*>* vec2 = new std::vector<kdNode*>();
    std::vector<float>* vec3 = new std::vector<float>();
    
    kdNode *temp = new kdNode(vec->at(index), 0);

    for(int i = 0; i < (int)k; i++){
        vec2->push_back(nullptr);
    }   


    temp->nearestNeighbor(root, temp, (int)dim, vec2, (int)k);//vec2 contains a long list of neighbors 

    delete(temp);

    for(int i = 0; i < (int)k; i++){
        for(int d = 0; d < (int)dim; d++){ 
                
            if(vec2->at(i) == nullptr){
                //std::cout<<"null"<<std::endl;
                break;
            }
            //std::cout<<vec2->at(i)->getVal(d)<<std::endl;
            vec3->push_back(vec2->at(i)->getVal(d));
        }
    }

    for(int i = 0; i < (int)vec2->size(); i++){ 
        //delete(vec2->at(i));
    }

    delete(vec2);

    vecStore->at(index) = new std::vector<float>;

    for(int i = 0; i < (int)vec3->size(); i++){
        
        if(vecStore->at(index) == nullptr){
            continue;
        }

        //std::cout<<vec3->at(i)<<std::endl;

        vecStore->at(index)->push_back(vec3->at(i));

    }

    delete(vec3);

    return vec3;
}







