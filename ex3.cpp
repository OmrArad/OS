// Omer Arad 314096389
// Using second format for configuration file
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <queue>
#include <string>
#include <iostream>
#include <fstream>
#include <thread>

using namespace std;

class UnboundedQueue : public queue<string>
{
    sem_t mutex; // mutex
    sem_t full; // how many cells are full

public:
    // constructor
    UnboundedQueue() {
        sem_init(&mutex, 0, 1);
        sem_init(&full, 0, 0);
    }

    // destructor
    virtual ~UnboundedQueue() {
        sem_destroy(&mutex);
        sem_destroy(&full);
    }


    void enqueue(string &s) {
        sem_wait(&mutex);
        queue::push(s);
        sem_post(&mutex);
        sem_post(&full);
    }

    string dequeue() {
        sem_wait(&full);
        sem_wait(&mutex);
        string s = queue::front();
        queue<string>::pop();
        sem_post(&mutex);
        return s;
    }


};

class BoundedQueue : public queue<string>
{
    sem_t mutex; // mutex
    sem_t full; // how many cells are full
    sem_t empty; // how many cells are empty


public:
    // constructor
    BoundedQueue(int size) {
        sem_init(&mutex, 0, 1);
        sem_init(&full, 0, 0);
        sem_init(&empty, 0, size);
    }

    // destructor
    virtual ~BoundedQueue() {
        sem_destroy(&mutex);
        sem_destroy(&full);
        sem_destroy(&empty);
    }

    void enqueue(string &s) {
        sem_wait(&empty);
        sem_wait(&mutex);
        queue::push(s);
        sem_post(&mutex);
        sem_post(&full);
    }

    string dequeue() {
        sem_wait(&full);
        sem_wait(&mutex);
        string s = queue::front();
        queue::pop();
        sem_post(&mutex);
        sem_post(&empty);
        return s;
    }
};

vector<BoundedQueue*> queueArray;
UnboundedQueue *news, *sports, *weather;
sem_t pushBackM;

struct producerInput {
    int ID;
    int numOfProducts;
};

struct coEditInput {
    UnboundedQueue* read;
    BoundedQueue* write;
};

void* producer(void *arg) {
    struct producerInput *producerInput = (struct producerInput*)arg;
    int countN = 0, countS = 0, countW = 0;

    // create BQ using ID

    // insert products into BQ
    for (int i = 0; i < producerInput->numOfProducts; i++) {

        // produce product
        string s = "Producer ";
        s.append(std::to_string(producerInput->ID));
        switch ((i + producerInput->ID) % 3) {
            case 0:
                s.append(" SPORTS ");
                s.append(to_string(countS));
                countS++;
                break;
            case 1:
                s.append(" NEWS ");
                s.append(to_string(countN));
                countN++;
                break;
            case 2:
                s.append(" WEATHER ");
                s.append(to_string(countW));
                countW++;
                break;
            default:
                break;
        }
        try {
            queueArray.at(producerInput->ID - 1)->enqueue(s);
        } catch (const out_of_range& oor) {
            cerr << "Out of Range error: " << oor.what() << '\n';
        }
    }

    // insert empty product (-1) or change queue to null
    string final = "-1";
    queueArray[producerInput->ID - 1]->enqueue(final);

    return (nullptr);
}

void dispatcher() {
    // create 3 UBQ for News, Sports, Weather
    int countEmptyQueues = 0;
    int index = 0;
    int size = (int)queueArray.size();
    string prod;

    // read from product Q in RR method
    while (countEmptyQueues != size) {

        if(queueArray.empty()) {
            printf("empty queue\n");
            continue;
        }

        if (queueArray[index] == nullptr) {
            index = (index + 1) % size;
            continue;
        }

        BoundedQueue* current = queueArray[index];

        prod = current->dequeue();
        string final = "-1";
        if (prod == final) {
            countEmptyQueues++;
            queueArray[index] = nullptr;
            continue;
        }

        // dispatch each product to relevant UBQ
        if(prod[11] == 'S' || prod[12] == 'S') {
            sports->enqueue(prod);
        } else if (prod[11] == 'N' || prod[12] == 'N') {
            news->enqueue(prod);
        } else if (prod[11] == 'W' || prod[12] == 'W') {
            weather->enqueue(prod);
        }
        
        index++;
        index %= size;
    }
    // insert end string for each queue
    string final = "-1";
    sports->enqueue(final);
    news->enqueue(final);
    weather->enqueue(final);
}

void* co_editor(void *input) {
    string final = "-1";
    struct coEditInput args = *(struct coEditInput*)input;
    string prod = args.read->dequeue();
    int count = 0;

    while (prod != final) {
        // edit
        usleep(150000);

        // send to global BQ / or given in args
        args.write->enqueue(prod);
        count++;

        // read from relevant UBQ
        prod = args.read->dequeue();
    }

    // insert final string to screenQueue
    args.write->enqueue(final);

    return nullptr;
}

void screen_manager(BoundedQueue* coEditQueue) {
    int countFinals = 0;
    string prod;
    string final = "-1";

    // read from global BQ
    while (countFinals < 3) {

        prod = coEditQueue->dequeue();

        if (prod == final) {
            countFinals++;
            continue;
        }

        // print to screen
        cout << prod << "\n";
    }
    cout << "Done\n";
}



int main(int argc, char *argv[]) {
    // read from config
    fstream config;
    vector<pthread_t> producerTh;
    pthread_t coEditTh[3];
    vector<producerInput*> input;
    string ID;


    sem_init(&pushBackM, 0, 1);

    config.open(argv[1],ios::in);
    if(!config.is_open()) {
        perror("failed to open configuration file !");
        return 0;
    }

    // create threads for all producers
    while (true) {
        string numOfProd, queueSize;
        getline(config,ID);
        if (ID.empty()) {
            getline(config, ID);
        }
        if(!getline(config, numOfProd)) {
            break;
        }
        getline(config, queueSize);

        input.push_back((struct producerInput *) malloc(sizeof (struct producerInput)));
        int i = stoi(ID) - 1;
        input[i]->ID = i + 1;
        input[i]->numOfProducts = stoi(numOfProd);

        // create new bounded queue for this producer and init producer thread
        queueArray.push_back(new BoundedQueue(stoi(queueSize)));
        producerTh.emplace_back();
        int err;
        err = pthread_create(&producerTh[i],nullptr, producer, (void*)input[i]);
        if (err != 0) {
            perror("can't create thread\n");
        }
    }

    sports = new UnboundedQueue();
    news = new UnboundedQueue();
    weather = new UnboundedQueue();

    // init dispatcher
    thread thread_dispatch(dispatcher);

    int qSize = stoi(ID);
    BoundedQueue* coEditQueue = new BoundedQueue(qSize);

    // init co_editors
    struct coEditInput *coEdit1 = (struct coEditInput *) malloc(sizeof (struct coEditInput));
    struct coEditInput *coEdit2 = (struct coEditInput *) malloc(sizeof (struct coEditInput));
    struct coEditInput *coEdit3 = (struct coEditInput *) malloc(sizeof (struct coEditInput));

    coEdit1->read = sports;
    coEdit2->read = news;
    coEdit3->read = weather;
    coEdit1->write = coEditQueue;
    coEdit2->write = coEditQueue;
    coEdit3->write = coEditQueue;

    if (pthread_create(&coEditTh[0], NULL, co_editor, (void*)coEdit1) != 0) {
        perror("can't create thread\n");
    }
    if (pthread_create(&coEditTh[1], NULL, co_editor, (void*)coEdit2) != 0) {
        perror("can't create thread\n");
    }
    if (pthread_create(&coEditTh[2], NULL, co_editor, (void*)coEdit3) != 0) {
        perror("can't create thread\n");
    }

    // init screenManager
    thread screenManager (screen_manager, coEditQueue);

    // join
    int numOfProducers = (int)producerTh.size();
    void* retVal[numOfProducers];
    for (int i = 0; i < numOfProducers; i++) {
        pthread_join(producerTh[i], &retVal[i]);
    }
    thread_dispatch.join();
    pthread_join(coEditTh[0], &retVal[0]);
    pthread_join(coEditTh[1], &retVal[1]);
    pthread_join(coEditTh[2], &retVal[2]);
    screenManager.join();

    // free resources
    for (auto & i : input) {
        free(i);
    }
    free(coEdit1);
    free(coEdit2);
    free(coEdit3);

}