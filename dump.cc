#include <iostream>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <iomanip>
#include <vector>
#include <fstream>
#include <thread>
#include <mutex>

#include "kdNode.h"

// Wrapper around a pointer, for reading values from byte sequence.
class Reader
{
public:
    Reader(const char *p) : ptr{p} {}
    template <typename T>
    Reader &operator>>(T &o)
    {
        // Assert alignment.
        assert(uintptr_t(ptr) % sizeof(T) == 0);
        o = *(T *)ptr;
        ptr += sizeof(T);
        return *this;
    }
    template <typename T>
    Reader &operator<<(T &o)
    {
        // Assert alignment.
        assert(uintptr_t(ptr) % sizeof(T) == 0);
        *(T *)ptr = o;
        ptr += sizeof(T);
        return *this;
    }


private:
    const char *ptr;
};

float*
dumpTraining(const std::string &fn, uint64_t* idRet, uint64_t* dimsRet, uint64_t* numPoints)
{

    std::mutex mtx;

    int fd = open(fn.c_str(), O_RDONLY);
    if (fd < 0)
    {
        int en = errno;
        std::cerr << "Couldn't open " << fn << ": " << strerror(en) << "." << std::endl;
        exit(2);
    }

    // Get the actual size of the file.
    struct stat sb;
    int rv = fstat(fd, &sb);
    assert(rv == 0);
    // std::cout << sb.st_size << std::endl;

    // Use some flags that will hopefully improve performance.
    void *vp = mmap(nullptr, sb.st_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
    if (vp == MAP_FAILED)
    {
        int en = errno;
        fprintf(stderr, "mmap() failed: %s\n", strerror(en));
        exit(3);
    }
    char *file_mem = (char *)vp;

    // Tell the kernel that it should evict the pages as soon as possible.
    rv = madvise(vp, sb.st_size, MADV_SEQUENTIAL | MADV_WILLNEED);
    assert(rv == 0);

    rv = close(fd);
    assert(rv == 0);

    // Prefix to print before every line, to improve readability.
    std::string pref("    ");

    /*
     * Read file type string.
     */
    int n = strnlen(file_mem, 8);
    std::string file_type(file_mem, n);
    //std::cout << pref << "File type string: " << file_type << std::endl;

    // Start to read data, skip the file type string.
    Reader reader{file_mem + 8};

    // TODO: Code below is repetitive, cleanup.
    // TODO: Add io manip to print with commas.
    bool first = true;


    if (file_type == "TRAINING")
    {

        uint64_t id;
        uint64_t n_points;
        uint64_t n_dims;

        reader >> id >> n_points >> n_dims;
        *idRet = id;//get training ID
        *dimsRet = n_dims; //get number of dimensions 


        float* arr;// = new float[n_dims*n_points - n_dims];
        int iter = 0;



        arr = new float[n_dims*n_points - n_dims];
        iter = 0;

        *numPoints = n_points;

        for (std::uint64_t i = 0; i < n_points; i++)
        {
            //float holder[n_dims];
            std::vector<float> holder(n_dims);// = std::vector<float>();
            for (std::uint64_t j = 0; j < n_dims; j++)
            {
                float f;
                reader >> f;

                holder[j] = f;
                iter++;
            }

            
            if(!first){
                for(int i = 0; i < (int)n_dims; i++){
                    arr[iter+i-n_dims-n_dims] = holder[i];
                }
            }
            else{
                first = false;
                //delete(holder);
            }
            
        
        }

        rv = munmap(file_mem, sb.st_size); assert(rv == 0);


        return arr;
    }

    std::cerr << "Unknown file type: " << file_type << std::endl;
    exit(2);

    rv = munmap(file_mem, sb.st_size); assert(rv == 0);


    return nullptr;

}

kdNode*
dumpTrainingGetRoot(const std::string &fn, uint64_t* dimsRet, uint64_t* numPoints)
{
    //root->getVal(0);
    //std::cout << fn << std::endl;
    kdNode* r;
    /*
     * Use mmap() for convenience.
     */

    int fd = open(fn.c_str(), O_RDONLY);
    if (fd < 0)
    {
        int en = errno;
        std::cerr << "Couldn't open " << fn << ": " << strerror(en) << "." << std::endl;
        exit(2);
    }

    // Get the actual size of the file.
    struct stat sb;
    int rv = fstat(fd, &sb);
    assert(rv == 0);
    // std::cout << sb.st_size << std::endl;

    // Use some flags that will hopefully improve performance.
    void *vp = mmap(nullptr, sb.st_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);
    if (vp == MAP_FAILED)
    {
        int en = errno;
        fprintf(stderr, "mmap() failed: %s\n", strerror(en));
        exit(3);
    }
    char *file_mem = (char *)vp;

    // Tell the kernel that it should evict the pages as soon as possible.
    rv = madvise(vp, sb.st_size, MADV_SEQUENTIAL | MADV_WILLNEED);
    assert(rv == 0);

    rv = close(fd);
    assert(rv == 0);

    // Prefix to print before every line, to improve readability.
    std::string pref("    ");

    /*
     * Read file type string.
     */
    int n = strnlen(file_mem, 8);
    std::string file_type(file_mem, n);
    //std::cout << pref << "File type string: " << file_type << std::endl;

    // Start to read data, skip the file type string.
    Reader reader{file_mem + 8};

    // TODO: Code below is repetitive, cleanup.
    // TODO: Add io manip to print with commas.

    if (file_type == "TRAINING")
    {

        uint64_t id;
        uint64_t n_points;
        uint64_t n_dims;

        reader >> id >> n_points >> n_dims;

        *dimsRet = n_dims;
        *numPoints = n_points;

        /*
        std::cout << pref << "Training file ID: " << std::hex << std::setw(16) << std::setfill('0') << id << std::dec << std::endl;
        std::cout << pref << "Number of points: " << n_points << std::endl;
        std::cout << pref << "Number of dimensions: " << n_dims << std::endl;
        */

        float *arr;
        int iter = 0;
        //bool first = true;

            //std::cout << pref << "Point " << i << ": ";
            arr = new float[3];
            iter = 0;
            for (std::uint64_t j = 0; j < n_dims; j++)
            {
                float f;
                reader >> f;
                //std::cout << std::fixed << std::setprecision(6) << std::setw(20) << std::setfill(' ') << f;
                //std::cout<<std::endl;

                arr[iter] = f;
                iter++;
            }
            //std::cout<<arr[0]<<std::endl;
            //std::cout << arr[0] << std::endl;
            iter = 0;

            r = new kdNode(arr,0);
            //std::cout<<iter2<<std::endl;
            /*
            if (first)
            {
                kdNode *temp = new kdNode(arr, 0);
                delete(root);
                root = temp;
                first = false;
            }
            */
            //else
            //{
            //}
            //std::cout << root->getVal(0) << std::endl;
            //delete(arr);
        }
        //std::cout << root << std::endl;

        //return root;
        return r;

    std::cerr << "Unknown file type: " << file_type << std::endl;
    exit(2);

    rv = munmap(file_mem, sb.st_size); assert(rv == 0);

    return nullptr;
}

void
dumpQ(const std::string &fn, std::vector<float*>* vec, uint64_t* qID, uint64_t* numQ, uint64_t* k) {

    /*
     * Use mmap() for convenience.
     */

    int fd = open(fn.c_str(), O_RDONLY);
    if (fd < 0) {
        int en = errno;
        std::cerr << "Couldn't open " << fn << ": " << strerror(en) << "." << std::endl;
        exit(2);
    }

    // Get the actual size of the file.
    struct stat sb;
    int rv = fstat(fd, &sb); assert(rv == 0);
    // std::cout << sb.st_size << std::endl;

    // Use some flags that will hopefully improve performance.
    void *vp = mmap(nullptr, sb.st_size, PROT_READ, MAP_PRIVATE|MAP_POPULATE, fd, 0);
    if (vp == MAP_FAILED) {
        int en = errno;
        fprintf(stderr, "mmap() failed: %s\n", strerror(en));
        exit(3);
    }
    char *file_mem = (char *) vp;

    // Tell the kernel that it should evict the pages as soon as possible.
    rv = madvise(vp, sb.st_size, MADV_SEQUENTIAL|MADV_WILLNEED); assert(rv == 0);

    rv = close(fd); assert(rv == 0);

    // Prefix to print before every line, to improve readability.
    std::string pref("    ");

    /*
     * Read file type string.
     */
    int n = strnlen(file_mem, 8);
    std::string file_type(file_mem, n);
    //std::cout << pref << "File type string: " << file_type << std::endl;

    // Start to read data, skip the file type string.
    Reader reader{file_mem + 8};

    if (file_type == "QUERY") {

        uint64_t id;
        uint64_t n_queries;
        uint64_t n_dims;
        uint64_t n_neighbors;

        reader >> id >> n_queries >> n_dims >> n_neighbors;

        *numQ = n_queries;
        *qID = id;
        *k = n_neighbors;
        
       float* arr;
       int iter =0;

        for (std::uint64_t i = 0; i < n_queries; i++) {

            //delete(arr);

            arr = new float[n_dims];

            //std::cout << pref << "Query " << i << ": ";
            for (std::uint64_t j = 0; j < n_dims; j++) {
                float f;
                reader >> f;
                //std::cout << std::fixed << std::setprecision(6) << std::setw(15) << std::setfill(' ') << f;
                arr[iter] = f;
                iter++;
            }
            iter= 0;

            vec->push_back(arr);
            //delete(arr);
        }
        

    } else {
        std::cerr << "Unknown file type: " << file_type << std::endl;
        exit(2);
    }

    rv = munmap(file_mem, sb.st_size); assert(rv == 0);

}



void
dumpR(const std::string &fn, std::vector<float>* vect, uint64_t tID, uint64_t qID, 
        uint64_t numQ, uint64_t numD, uint64_t k) {

    /*
     * Use mmap() for convenience.
     

    int fd = open(fn.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0x0777);
    if (fd < 0) {
        int en = errno;
        std::cerr << "Couldn't open " << fn << ": " << strerror(en) << "." << std::endl;
        exit(2);
    }
    
    */

    std::ofstream myfile(fn, std::ofstream::binary);

    std::string result_txt("RESULT");

    myfile.write(result_txt.c_str(), result_txt.size()); 
    myfile.write("\0", sizeof(char));
    myfile.write("\0", sizeof(char));
    //myfile.write("RESULT", fn.length()-3); //for some reason we need -3 ...
    

    myfile.write(reinterpret_cast<char*>(&tID), sizeof(tID));
    myfile.write(reinterpret_cast<char*>(&qID), sizeof(qID));
    tID += qID;
    myfile.write((const char*) (&tID), sizeof(tID)); //generate a random id number for results 
    myfile.write((const char*)&numQ, sizeof(numQ));
    myfile.write((const char*) &numD, sizeof(numD));
    myfile.write((const char*) &k, sizeof(k));


    //TODO: add output for loop here
    for (std::uint64_t i = 0; i < vect->size(); i++) {
        //std::cout<<(const char*)&vect->at(i)<<std::endl;
        myfile.write((const char*)&vect->at(i), sizeof(float));
    }
                

    //myfile.write((const char*)&float, sizeof(float));

    myfile.close();

}


void
dumpRIn(const std::string &fn) {

    std::cout << fn << std::endl;

    /*
     * Use mmap() for convenience.
     */

    int fd = open(fn.c_str(), O_RDONLY);
    if (fd < 0) {
        int en = errno;
        std::cerr << "Couldn't open " << fn << ": " << strerror(en) << "." << std::endl;
        exit(2);
    }

    // Get the actual size of the file.
    struct stat sb;
    int rv = fstat(fd, &sb); assert(rv == 0);
    // std::cout << sb.st_size << std::endl;

    // Use some flags that will hopefully improve performance.
    void *vp = mmap(nullptr, sb.st_size, PROT_READ, MAP_PRIVATE|MAP_POPULATE, fd, 0);
    if (vp == MAP_FAILED) {
        int en = errno;
        fprintf(stderr, "mmap() failed: %s\n", strerror(en));
        exit(3);
    }
    char *file_mem = (char *) vp;

    // Tell the kernel that it should evict the pages as soon as possible.
    rv = madvise(vp, sb.st_size, MADV_SEQUENTIAL|MADV_WILLNEED); assert(rv == 0);

    rv = close(fd); assert(rv == 0);

    // Prefix to print before every line, to improve readability.
    std::string pref("    ");

    /*
     * Read file type string.
     */
    int n = strnlen(file_mem, 8);
    std::string file_type(file_mem, n);
    std::cout << pref << "File type string: " << file_type << std::endl;

    // Start to read data, skip the file type string.
    Reader reader{file_mem + 8};

    // TODO: Code below is repetitive, cleanup.
    // TODO: Add io manip to print with commas.

    if (file_type == "TRAINING") {

        uint64_t id;
        uint64_t n_points;
        uint64_t n_dims;

        reader >> id >> n_points >> n_dims;

        std::cout << pref << "Training file ID: " << std::hex << std::setw(16) << std::setfill('0') << id << std::dec << std::endl;
        std::cout << pref << "Number of points: " << n_points << std::endl;
        std::cout << pref << "Number of dimensions: " << n_dims << std::endl;
        for (std::uint64_t i = 0; i < n_points; i++) {
            std::cout << pref << "Point " << i << ": ";
            for (std::uint64_t j = 0; j < n_dims; j++) {
                float f;
                reader >> f;
                std::cout << std::fixed << std::setprecision(6) << std::setw(15) << std::setfill(' ') << f;
                // Add comma.
                if (j < n_dims - 1) {
                    std::cout << ", ";
                }
            }
            std::cout << std::endl;
        }

    } else if (file_type == "QUERY") {

        uint64_t id;
        uint64_t n_queries;
        uint64_t n_dims;
        uint64_t n_neighbors;

        reader >> id >> n_queries >> n_dims >> n_neighbors;

        std::cout << pref << "Query file ID: " << std::hex << std::setw(16) << std::setfill('0') << id << std::dec << std::endl;
        std::cout << pref << "Number of queries: " << n_queries << std::endl;
        std::cout << pref << "Number of dimensions: " << n_dims << std::endl;
        std::cout << pref << "Number of neighbors to return for each point: " << n_neighbors << std::endl;
        for (std::uint64_t i = 0; i < n_queries; i++) {
            std::cout << pref << "Query " << i << ": ";
            for (std::uint64_t j = 0; j < n_dims; j++) {
                float f;
                reader >> f;
                std::cout << std::fixed << std::setprecision(6) << std::setw(15) << std::setfill(' ') << f;
                // Add comma.
                if (j < n_dims - 1) {
                    std::cout << ", ";
                }
            }
            std::cout << std::endl;
        }

    } else if (file_type == "RESULT") {

        uint64_t training_id;
        uint64_t query_id;
        uint64_t result_id;
        uint64_t n_queries;
        uint64_t n_dims;
        uint64_t n_neighbors;

        reader >> training_id >> query_id >> result_id >> n_queries >> n_dims >> n_neighbors;

        std::cout << pref << "Training file ID: " << std::hex << std::setw(16) << std::setfill('0') << training_id << std::dec << std::endl;
        std::cout << pref << "Query file ID: " << std::hex << std::setw(16) << std::setfill('0') << query_id << std::dec << std::endl;
        std::cout << pref << "Result file ID: " << std::hex << std::setw(16) << std::setfill('0') << result_id << std::dec << std::endl;
        std::cout << pref << "Number of queries: " << n_queries << std::endl;
        std::cout << pref << "Number of dimensions: " << n_dims << std::endl;
        std::cout << pref << "Number of neighbors returned for each query: " << n_neighbors << std::endl;
        for (std::uint64_t i = 0; i < n_queries; i++) {
            for (std::uint64_t n = 0; n < n_neighbors; n++) {
                std::cout << pref << "Neighbor " << n << " for query " << i << ": ";
                for (std::uint64_t j = 0; j < n_dims; j++) {
                    float f;
                    reader >> f;
                    std::cout << std::fixed << std::setprecision(6) << std::setw(15) << std::setfill(' ') << f;
                    // Add comma.
                    if (j < n_dims - 1) {
                        std::cout << ", ";
                    }
                }
                std::cout << std::endl;
            }
        }

    } else {
        std::cerr << "Unknown file type: " << file_type << std::endl;
        exit(2);
    }

    rv = munmap(file_mem, sb.st_size); assert(rv == 0);
}







