
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <string>
#include <thread>
#include <numeric>

#include <pthread.h>
#include <limits.h>
#include <assert.h>
#include <omp.h>
#include <time.h>


class Timer
{
private:
    std::string name;
    std::chrono::high_resolution_clock::time_point start, end;
    std::chrono::duration<float> duration;
public:
    Timer(std::string _name) : name(_name)
    {
        start = std::chrono::high_resolution_clock::now();
    }

    ~Timer()
    {
        end = std::chrono::high_resolution_clock::now();
        duration = end - start;
        float ms = duration.count() * 1000.0f;

        std::cout << name + " took: " << ms << "ms" << std::endl;
    }

};