#include "Timer.hh"


#define SIZE 65536*1024
#define THREAD_COUNT 4


struct ThreadData
{
    std::vector<int>& vec;
    size_t start;
    size_t end;

    ThreadData(std::vector<int>& vec, size_t start, size_t end)
        : vec(vec), start(start), end(end) {}
};

void *threadFindMax(void * threadData)
{
    std::vector<int>& vec = ((ThreadData *) threadData)->vec;
    size_t start = ((ThreadData *) threadData)->start;
    size_t end = ((ThreadData *) threadData)->end;

    // need to allocate in heap, or will be dangling after the function call
    int *max = new int;
    *max = vec[start];

    for (size_t i = start; i < end; i++) {
        if (vec[i] > *max) {
            *max = vec[i];
        }
    }
    // return the value to main thread when joining
    pthread_exit((void *) max);
}

int pthreadFindMax(std::vector<int>& vec)
{
    pthread_t threads[THREAD_COUNT];
    int frac = SIZE / THREAD_COUNT;

    for (size_t i = 0; i < THREAD_COUNT; i++) {
        size_t s = i * frac;
        size_t e = s + frac;

        // create object to hold thread private data
        ThreadData *thrData = new ThreadData(vec, s, e);
        assert(!pthread_create(&threads[i], NULL, threadFindMax, (void *) thrData));
    }

    std::vector<int> retValues(THREAD_COUNT);

    for (size_t i = 0; i < THREAD_COUNT; i++) {
        int* retValue;
        pthread_join(threads[i], (void **) &retValue);
        retValues[i] = *retValue;
    }
    // get the max from the return values of threads
    return  *std::max_element(retValues.begin(), retValues.end());    
}

typedef void *(*VoidFuncType)(void *);

int pthreadLambdaFindMax(std::vector<int>& vec)
{
    VoidFuncType threadFunc {
        [](void * threadData) {
            std::vector<int>& vec = ((ThreadData *) threadData)->vec;
            size_t start = ((ThreadData *) threadData)->start;
            size_t end = ((ThreadData *) threadData)->end;

            // need to allocate in heap, or will be dangling after the function call
            int *max = new int;
            *max = vec[start];

            for (size_t i = start; i < end; i++) {
                if (vec[i] > *max) {
                    *max = vec[i];
                }
            }
            // return the value to main thread when joining
            pthread_exit((void *) max);            
            return (void *) max;
        }
    };

    pthread_t threads[THREAD_COUNT];
    int frac = SIZE / THREAD_COUNT;

    for (size_t i = 0; i < THREAD_COUNT; i++) {
        size_t s = i * frac;
        size_t e = s + frac;

        // create object to hold thread private data
        ThreadData *threadData = new ThreadData(vec, s, e);
        assert(!pthread_create(&threads[i], NULL, threadFunc, (void *) threadData));
    }

    std::vector<int> retValues(THREAD_COUNT);

    for (size_t i = 0; i < THREAD_COUNT; i++) {
        int* retValue;
        pthread_join(threads[i], (void **) &retValue);
        retValues[i] = *retValue;
    }
    // get the max from the return values of threads
    return  *std::max_element(retValues.begin(), retValues.end());    
}

int ompFindMax(std::vector<int>& vec)
{
    std::vector<int> retValues(THREAD_COUNT);
    int frac = SIZE / THREAD_COUNT;

    // starting parallel thread construct
    #pragma omp parallel num_threads(THREAD_COUNT)
    {
        int threadNum = omp_get_thread_num(); // get the current thread number

        int start = frac * threadNum;
        int end  = start + frac;

        int max = vec[start];
        for (size_t i = start; i < end; i++) {
            if (vec[i] > max) {
                max = vec[i];
            }
        }
        retValues[threadNum] = max;
    }
    // get the max from the return values of threads
    return *std::max_element(retValues.begin(), retValues.end());
}

int ompReductionFindMax(std::vector<int>& vec)
{
    int maxVal = 0;

    #pragma omp parallel for reduction(max : maxVal)
    for (size_t i = 0; i < vec.size(); i++) {
        if (vec[i] > maxVal) {
            maxVal = vec[i];
        }
    }
    return maxVal;
}

int cppThreadsFindMax(std::vector<int>& vec)
{
    std::vector<int> retValues(THREAD_COUNT);

    auto threadFunc = [&vec, &retValues](int id, size_t start, size_t end) {
        int maxVal = vec[0];
        for (size_t i = start; i < end; i++) {
            if (vec[i] > maxVal) {
                maxVal = vec[i];
            }
        }
        retValues[id] = maxVal;
    };

    std::thread threads[THREAD_COUNT];
    int frac = SIZE / THREAD_COUNT;

    for (size_t i = 0; i < THREAD_COUNT; i++) {
        size_t s = i * frac;
        size_t e = s + frac;
        threads[i] = std::thread(threadFunc, i, s, e);
    }

    for (size_t i = 0; i < THREAD_COUNT; i++) {
        threads[i].join();
    }

    return *std::max_element(retValues.begin(), retValues.end());
}


int findMax(std::vector<int>& vec)
{
    int max = vec[0];

    for (size_t i = 0; i < vec.size(); i++) {
        if (vec[i] > max) {
            max = vec[i];
        }
    }
    return max;
}

int main()
{
    std::vector<int> vec(SIZE);
    std::iota (std::begin(vec), std::end(vec), 0);
    
    int max = 0;

    {
        Timer timer("my max");
        max = findMax(vec);
    }
    std::cout << "max: " << max << std::endl;
    
    {
        Timer timer("std max");
        max = *std::max_element(vec.begin(), vec.end());
    }
    std::cout << "max: " << max << std::endl;

    {
        Timer timer("pthread max");
        max = pthreadFindMax(vec);
    }
    std::cout << "max: " << max << std::endl;

    {
        Timer timer("pthread lambda max");
        max = pthreadLambdaFindMax(vec);
    }
    std::cout << "max: " << max << std::endl;

    {
        Timer timer("omp max");
        max = ompFindMax(vec);
    }
    std::cout << "max: " << max << std::endl;

    {
        Timer timer("omp reduction max");
        max = ompReductionFindMax(vec);
    }
    std::cout << "max: " << max << std::endl;

    {
        Timer timer("cpp thread max");
        max = cppThreadsFindMax(vec);
    }
    std::cout << "max: " << max << std::endl;

    return 0;        
}