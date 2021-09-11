#include <iostream>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <iomanip>

class Reader {
    public:
        Reader(const char *p) : ptr{p} {}
        template <typename T>
        Reader &operator>>(T &o);
    private:
        const char *ptr;
};


float*
dumpTraining(const std::string &fn, uint64_t* idRet, uint64_t* dimsRet, uint64_t* numPoints);

kdNode*
dumpTrainingGetRoot(const std::string &fn, uint64_t* dimsRet, uint64_t* numPoints);

void
dumpQ(const std::string &fn, std::vector<float*>* vec, uint64_t* qID, uint64_t* numQ, uint64_t* k);

void
dumpR(const std::string &fn, std::vector<float>* vect, uint64_t trainingID, uint64_t qID,
        uint64_t numQ, uint64_t dim, uint64_t k);

void
dumpRIn(const std::string &fn);