#include<bits/stdc++.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <mutex>
#include <time.h>
#include <assert.h>
#include <stdio.h>

/*
Luego de comparar los resultados, se concluye que uso de las
variables atómicas es mejor respecto al uso de locks.
*/

using namespace std;
using namespace std::chrono;

vector<atomic<int>> local_maximums_atomic_vector;
vector<int> local_maximums_int_vector;
mutex mylock;

#define NTESTS 20

void initialize_int_vector(int n){
    local_maximums_int_vector.resize(n);
}

void initialize_atomic_vector(int n){
    local_maximums_atomic_vector = vector<atomic<int>>(n);
}

vector<int> create_values_vector(int num){
    vector<int> values(num);
    for(int i = 0; i < num; ++i){
        values[i] = rand()%(2*num+1) - num;       
    }
    return values;
}

int get_max_value(vector<int>& values, int start, int end){
    int maximum = numeric_limits<int>::min();
    for(int i=start; i<end; ++i)
        maximum = max(maximum, values[i]);
    return maximum;
}

void using_locks_function(vector<int>& values, int start, int end, int id){
    int maximum = get_max_value(values, start, end);
    mylock.lock();
    local_maximums_int_vector[id] = maximum;
    mylock.unlock();
}

void using_atomic_variables_function(vector<int>& values, int start, int end, int id){
    int maximum = get_max_value(values, start, end);
    local_maximums_atomic_vector[id] = maximum;
}

double execute_program(int nthreads, int n, bool using_locks){
    vector<thread> threads(nthreads);
    using_locks ? initialize_int_vector(nthreads) : initialize_atomic_vector(nthreads);
    auto values = create_values_vector(n);
    int step = n/nthreads;
    for (int i=0; i<nthreads-1; i++)
        threads[i] = using_locks ? thread(using_locks_function, ref(values), step*i, step*(i+1), i)
                        : thread(using_atomic_variables_function, ref(values), step*i, step*(i+1), i);
    
    threads[nthreads-1] = using_locks ? thread(using_locks_function, ref(values), step*(nthreads-1), n, nthreads-1)
                    : thread(using_atomic_variables_function, ref(values), step*(nthreads-1), n, nthreads-1);
    
    auto start = high_resolution_clock::now();
    for (int i=0; i<nthreads; i++)
        threads[i].join();
    int result = using_locks ? *max_element(local_maximums_int_vector.begin(), local_maximums_int_vector.end())
                    : max_element(local_maximums_atomic_vector.begin(), local_maximums_atomic_vector.end())->load();
    auto stop = high_resolution_clock::now();
    
    auto duration = duration_cast<microseconds>(stop - start);

    assert(result == *max_element(values.begin(), values.end())); // Demora la verificacion
    return duration.count();
}

int main(){
    srand(time(0));
    cout << "**********************************************" << endl;
    int nthreads, n;
    float average_time_locks, average_time_atomic_variables;
    for(int i = 1; i <= 7; ++i){
        nthreads = i*2;
        n = pow(10, i);
        average_time_atomic_variables = 0;
        average_time_locks = 0;
        cout << "\t Tiempo de ejecucion (s) promedio de "<< NTESTS << " tests" << endl;
        cout << "Numero de elementos >> " << n  << endl;
        cout << "Numero de threads   >> " << nthreads <<  endl;
        for(int j = 1; j <= NTESTS; ++j){
            average_time_locks += execute_program(nthreads, n, true);
            average_time_atomic_variables += execute_program(nthreads, n, false);
        }
        cout << "[Locks]           >> " << average_time_locks/NTESTS/1e6 << endl;
        cout << "[AtomicVariables] >> " << average_time_atomic_variables/NTESTS/1e6 << endl;
        
        cout << "**********************************************" << endl;
    }
}