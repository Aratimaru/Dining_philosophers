#include <thread>
#include <iostream>
#include <mutex>
#include <vector>
#include <chrono>
#include <map>
#include <utility>

#define DEBUG 0

#define PHILO_NUMBER 50
#define FORKS_NUMBER PHILO_NUMBER
#define MAX_TIMES_TO_EAT 10
#define EATING_DURATION 2 //ms

// just a custom formula that works in all cases I tested
#define TIMEOUT_FOR_STARVATION (EATING_DURATION*MAX_TIMES_TO_EAT*PHILO_NUMBER/5)  //ms

#define SET_OUTPUT_COLOR_GREEN "\033[1;32m"
#define SET_OUTPUT_COLOR_RED "\033[1;31m"
#define SET_OUTPUT_COLOR_BLUE "\033[1;96m"
#define RESET_OUTPUT_COLOR "\033[39;49m"


/**
 * Dinner visualization
 * 
 * F0       F1      F2      F3      F4      F0
 *      P0      P1      P2      P3      P4
*/
std::mutex forks[FORKS_NUMBER];
std::mutex io_operations_mutex;

void start_dinner(uint id);
bool try_take_forks(uint id, uint left_fork_id, uint right_fork_id);
void eat(uint left_fork_id, uint right_fork_id);

int main(){
    std::vector<std::thread> philosophers;

    for(uint i=0; i<PHILO_NUMBER; i++){
        philosophers.push_back(std::thread(start_dinner ,i));
    }

    for(int i=0; i<philosophers.size(); i++){
        philosophers[i].join();
    }

    return 0;
}

void start_dinner(uint id){
    int eating_times = 0;
    uint left_fork_id, right_fork_id;
    bool forks_taken;
    auto start_time{std::chrono::_V2::steady_clock::now()};

    left_fork_id = id;
    right_fork_id = (id+1 == PHILO_NUMBER ? 0 : id+1);
    
    if(DEBUG)
    {
        io_operations_mutex.lock();
        std::cout << "Philosopher " << id << " has adjacent forks: " << left_fork_id << ", " << right_fork_id << '\n';
        io_operations_mutex.unlock();
    }
    
    while(eating_times < MAX_TIMES_TO_EAT)
    {
        auto current_time{std::chrono::_V2::steady_clock::now()};
        if((current_time - start_time) > std::chrono::milliseconds(TIMEOUT_FOR_STARVATION))
        {
            io_operations_mutex.lock();
            std::cout << SET_OUTPUT_COLOR_RED << "Philosopher " << id << " DIED!!!!!! No, you murdered them!!!!!!!!" << RESET_OUTPUT_COLOR << '\n';
            io_operations_mutex.unlock();
            return;   
        }

        forks_taken = try_take_forks(id, left_fork_id, right_fork_id);
        
        if(forks_taken)
        {
            if(DEBUG)
            {
                io_operations_mutex.lock();
                std::cout << SET_OUTPUT_COLOR_GREEN << "Philosopher " << id << " ate. Thank you!" << RESET_OUTPUT_COLOR << '\n';
                io_operations_mutex.unlock();
            }
            eat(left_fork_id, right_fork_id);
            start_time = std::chrono::_V2::steady_clock::now();
            eating_times++;
        }

        forks_taken = false;
        
    }

    io_operations_mutex.lock();
    std::cout << SET_OUTPUT_COLOR_BLUE << "Philosopher " << id << " had a great dinner!" << RESET_OUTPUT_COLOR << '\n';    
    io_operations_mutex.unlock();
}

bool try_take_forks(uint id, uint left_fork_id, uint right_fork_id){
    if(left_fork_id >= FORKS_NUMBER){
        std::cout << "Dont make me angry!!! Where am I supposed to take fork " << left_fork_id << "???\n";
        return false;
    }
    if(right_fork_id >= FORKS_NUMBER){
        std::cout << "Dont make me angry!!! Where am I supposed to take fork " << right_fork_id << "???\n";
        return false;
    }

    bool left_fork_taken = forks[left_fork_id].try_lock();
    bool right_fork_taken = forks[right_fork_id].try_lock();

    if(left_fork_taken && !right_fork_taken)
    {
        forks[left_fork_id].unlock();

        if(DEBUG)
        {
            io_operations_mutex.lock();
            std::cout << "Philosopher " << id << " tried to take fork " << left_fork_id << " but " << right_fork_id << " had been already taken" << '\n';
            io_operations_mutex.unlock();
        }

        return false;
    }

    if(right_fork_taken && !left_fork_taken)
    {
        forks[right_fork_id].unlock();

        if(DEBUG)
        {
            io_operations_mutex.lock();
            std::cout << "Philosopher " << id << " tried to take fork " << right_fork_taken << " but " << left_fork_taken << " had been already taken" << '\n';
            io_operations_mutex.unlock();
        }

        return false;
    }

    if(left_fork_taken && right_fork_taken)
    {
        if(DEBUG)
        {
            io_operations_mutex.lock();
            std::cout << "Philosopher " << id << " took fork " << left_fork_id << " and fork " << right_fork_id << '\n';
            io_operations_mutex.unlock();
        }

        return true;
    }

    return false;
}

void eat(uint left_fork_id, uint right_fork_id){
    std::this_thread::sleep_for(std::chrono::milliseconds(EATING_DURATION));
    forks[left_fork_id].unlock();
    forks[right_fork_id].unlock();
}