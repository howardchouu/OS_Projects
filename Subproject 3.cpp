#include <sys/time.h>
#include <sys/resource.h>
#include <fstream>
#include <deque>
#include <string.h>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
using namespace std;

pthread_mutex_t get_num = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ordering = PTHREAD_MUTEX_INITIALIZER;

struct datatype {

    int times[35] = {};
    int tmp_count = 1;
    int student_num = 0;
    int finish = 0;
    deque<int> order;
};

void* robot(void* arg) {
    //init
    time_t time_seconds = time(0);
    struct tm* now_time = localtime(&time_seconds);
    datatype* data = (datatype*)arg;
    bool r_sleep = false;

    // robot work
    while (data->finish != data->student_num) {
        pthread_mutex_lock(&ordering);
        // inspect order queue
        if (data->order.size() != 0) {  // have student in queue
            time_seconds = time(0);
            now_time = localtime(&time_seconds);

            for(int i = 0; i < data->order.size();i++){
            cout << now_time->tm_hour <<":"<<now_time->tm_min<<":"<<now_time->tm_sec;
            cout<<" Student"<<data->order[i]<<": ";
                if(i == 0)
                    cout<<"Entering to get a shot\n";
                else if(i == 1){
                    cout<<"Sitting#1\n";
                }
                else{
                    cout<<"Sitting#2\n";
                }
            }
            r_sleep = false;
            sleep(2);
            data->times[data->order.front()] += 1;
            time_seconds = time(0);
            now_time = localtime(&time_seconds);
            cout << now_time->tm_hour <<":"<<now_time->tm_min<<":"<<now_time->tm_sec;
            cout<<" Student"<<data->order.front()<<": Leaving\n";
            if (data->times[data->order.front()] == 3)
                data->finish++;
            data->order.pop_front();
        }
        else if (data->order.size() == 0 && !r_sleep) { // without student in queue
            r_sleep = true;
            time_seconds = time(0);
            now_time = localtime(&time_seconds);
            cout << now_time->tm_hour <<":"<<now_time->tm_min<<":"<<now_time->tm_sec;
            cout<<" Robot : Sleep\n";
        }
        pthread_mutex_unlock(&ordering);
    }
    time_seconds = time(0);
    now_time = localtime(&time_seconds);
    cout << now_time->tm_hour <<":"<<now_time->tm_min<<":"<<now_time->tm_sec;
    cout<<" Robot : All " << data->finish << " students receive vaccines.\n";

    pthread_exit(NULL); // return value
}


void* child(void* arg) {

    //init
    float waitting_time, max, min;
    int thread_number = 0;
    bool in_queue = false;
    datatype* data = (datatype*)arg;
    int current_times = 0;

    // get thread number
    pthread_mutex_lock(&get_num);
    thread_number = data->tmp_count;
    data->tmp_count = thread_number + 1;
    pthread_mutex_unlock(&get_num);

    //first to shot
    min = 0.0;
    max = 10.0;
    waitting_time = (max - min) * rand() / (RAND_MAX + 1.0) + min;
    usleep(waitting_time * 1000000);

    while (data->times[thread_number] < 3) {
        if (!in_queue || current_times + 1 == data->times[thread_number]) {

            pthread_mutex_lock(&ordering);
            if (data->order.size() < 3) {
                // push in queue
                data->order.push_back(thread_number);
                current_times = data->times[thread_number];
                min = 10.0;
                max = 30.0;
                in_queue = true;
            }
            else {
                min = 5.0;
                max = 10.0;
            }
            
            if (current_times + 1 == data->times[thread_number]) {
                in_queue = false;
            }
            pthread_mutex_unlock(&ordering);
            if (!in_queue) {
                // waitting
                waitting_time = (max - min) * rand() / (RAND_MAX + 1.0) + min;
                usleep(waitting_time * 1000000);
            }
        }
    }


    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {

    // Input verification
    bool valid = true;
    if(strlen(argv[1]) > 2 || strlen(argv[1]) == 0 || strlen(argv[2]) > 3 || strlen(argv[2]) == 0){
        valid = false;
    }
    for(int k = 1; k < 3; k++){
        for(int i = 0; i < strlen(argv[k]); i++){
            if(argv[k][i] > '9' || argv[k][i] < '0'){
                valid = false;
            }
        }
    }
    if(!valid ||stoi(argv[1]) > 20 || stoi(argv[1]) < 10 || stoi(argv[2]) > 100 || stoi(argv[2]) < 0)
        valid = false;
    if(!valid){
        cout<<"Please try again.\n";
        exit(1);
    }
    int student_num = stoi(argv[1]), seed = stoi(argv[2]);

    // init
    srand(seed);
    datatype* data = new datatype;
    data->student_num = student_num;

    // make pthread array to store
    pthread_t threads[student_num + 1];

    // make thread and execute
    pthread_create(&threads[0], NULL, robot, (void*)data);
    for (int i = 1; i < student_num + 1; i++) {
        pthread_create(&threads[i], NULL, child, (void*)data);
    }

    // receive the thread return value
    for (int i = 0; i < (*data).student_num + 1; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}