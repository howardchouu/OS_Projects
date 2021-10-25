#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fstream>
#include <deque>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
using namespace std;

struct datatype{
    int thread_n = 0;
    int patten_n = 0;
    string data;
    int thread_num;
    deque<string> patten;
};

void *child(void *arg){
    datatype *data = (datatype *) arg;
    int *result = (int *)malloc(sizeof(int)*100); // allocate the memory to result
    bool judge = true;
    int counter = 0;
    cout <<"[ Tid = "<<pthread_self()<<" ] search "<<data->patten[data->patten_n]
        <<" at "<< data->data.size()/data->thread_num*data->thread_n 
        << " " << data->data.substr(data->data.size()/data->thread_num*data->thread_n,8)<< endl;
    // data segmentation
    string segment = data->data.substr(data->data.size()/data->thread_num*data->thread_n
        ,data->data.size()/data->thread_num);
    // use the pre-segmentation data to search the key string
    for(int i = 0; i < segment.size() - data->patten[data->patten_n].size() + 1; i++){
        judge = true;
        for(int j = 0; j < data->patten[data->patten_n].size(); j++){
            if(data->patten[data->patten_n][j] != segment[i+j]){
                judge = false;
                break;
            }
        }
        if(judge)
            result[counter++] = i;
    }
    result[counter] = -1;
    pthread_exit((void *) result); // return value
}

int main(int argc, char* argv[]) {
    struct rusage ru;
    struct timeval utime;
    struct timeval stime;
    // open file
    ifstream input_file;
    input_file.open(argv[1],ios::in);
    if(!input_file.is_open()){
        printf("fail to open data file. exit...\n");
        exit(1);
    }
    // init
    void *ret;
    datatype *data = new datatype;
    deque<deque<int>> ans;
    string tmp_input;

    //read the data file
    getline(input_file,tmp_input);
    (*data).data = tmp_input;
    getline(input_file,tmp_input);
    (*data).thread_num = stoi(tmp_input);
    while(getline(input_file,tmp_input))  {
        deque<string> release_patten;
        release_patten.push_back(tmp_input);
        bool judge =true;
        while(judge){
            judge = false;
            // step by step inspect every input
            for(deque<string>::iterator i = release_patten.begin(); i < release_patten.end();){
                deque<string>::iterator ori_i =i;
                // if meet ?
                for(int k = 0; k < (*i).size(); k++){
                    if((*i)[k]=='?'){
                        (*i)[k] = 'A';
                        release_patten.push_back((*i));
                        (*i)[k] = 'C';
                        release_patten.push_back((*i));
                        (*i)[k] = 'G';
                        release_patten.push_back((*i));
                        (*i)[k] = 'T';
                        release_patten.push_back((*i));
                        deque<string>::iterator i_1 = i;
                        i++;
                        release_patten.erase(i_1);
                        judge =true;
                        break;
                    }
                }
                // if meet {}
                for(int k = 0; k < (*i).size(); k++){
                    if((*i)[k] == '{'){
                        char patten_kind[5] ={};
                        int b = 0, j = k;
                        while((*i)[++j] != '}')
                            if ((*i)[j] != ',')
                                patten_kind[b++] = (*i)[j];
                        string released_string;
                        // Filter string to exclude {}
                        for(int a = 0; a < (*i).size(); a++){
                            if( a < k )
                                released_string.push_back((*i)[a]);
                            else if( a > j )
                                released_string.push_back((*i)[a]);
                        }
                        //tmp insert key word and push to test case
                        for(int a = 0; a < b; a++){
                            released_string.insert(k,1,patten_kind[a]);
                            release_patten.push_back(released_string);
                            released_string.erase(k,1);
                        }
                        deque<string>::iterator i_1 = i;
                        i++;
                        release_patten.erase(i_1);
                        judge =true;
                        break;
                    }

                }
                if(ori_i == i)  i++;
            }
        }
        for(int x = 0;x < release_patten.size(); x++)
            (*data).patten.push_back(release_patten[x]);
    }
    // make pthread array to store
    pthread_t threads[(*data).thread_num*(*data).patten.size()];
    // make thread and execute
    for(int i = 0;i < (*data).patten.size(); i++){
        (*data).patten_n = i;
        for(int j = 0; j < (*data).thread_num; j++){
            (*data).thread_n = j;
            pthread_create(&threads[j+(*data).thread_num * i], NULL, child, (void*) data);
            usleep(10000);
        }
    }
    // receive the thread return value
    for(int i = 0;i < (*data).patten.size(); i++){
        deque<int>tmp;
        for(int j = 0; j < (*data).thread_num; j++){
            pthread_join(threads[j+(*data).thread_num * i], &ret);
            int *result = (int *) ret;
            for(int k = 0; result[k] != -1; k++){
                tmp.push_back(result[k]);
            }
            free(result);
        }
        ans.push_back(tmp);
    }

    for(int i = 0; i < ans.size(); i++){
        cout << "[" << (*data).patten[i] << "]" ;
        for(int n = 0; n < ans[i].size(); n++)
            cout << " " << ans[i][n];
        cout << endl;
    }

    getrusage(RUSAGE_SELF, &ru);

    utime = ru.ru_utime;
    stime = ru.ru_stime;
    double utime_used = utime.tv_sec + (double)utime.tv_usec / 1000000.0;
    double stime_used = stime.tv_sec + (double)stime.tv_usec / 1000000.0;
    //printf("User Time = %f\n", utime_used);
    //printf("System Time = %f\n", stime_used);
    cout << "CPU time: " << utime_used+stime_used << endl;

    return 0;
}