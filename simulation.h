#ifndef _SIMULATION_H_
#define _SIMULATION_H_

#include "heapqueue.h"
#include "linkedqueue.h"

#define SEED_STREAM_K 67108864

typedef enum {
    EVENT_UNKNOWN,
    EVENT_ARRIVAL,
    EVENT_DEPARTURE,
    EVENT_FEEL_HUNGER,
    EVENT_FILL_HUNGER,
    EVENT_FEEL_THRIST,
    EVENT_FILL_THRIST,
    EVENT_FEEL_SLEEPY,
    EVENT_FILL_SLEEPY,
    EVENT_FEEL_PEE,
    EVENT_FILL_PEE,
    EVENT_FEEL_POO,
    EVENT_FILL_POO,
    EVENT_FEEL_RELAX,
    EVENT_FILL_RELAX,
    EVENT_SIMULATION_STOP = 0xFFFFFFFF
} EventType_t;

typedef struct {
    EventType_t type;
    void *data;
    double eventTime;
} Event_t;

typedef struct {
    double arrivalTime;
    double serviceTime;
    unsigned int id;
} Customer_t;

#define SERVER_FLAG_SERVING     0x00000001
#define SERVER_FLAG_HUNGER      0x00000002
#define SERVER_FLAG_HUNGER_S    0x00000004
#define SERVER_FLAG_THRIST      0x00000008
#define SERVER_FLAG_THRIST_S    0x00000010
#define SERVER_FLAG_SLEEPY      0x00000020
#define SERVER_FLAG_SLEEPY_S    0x00000040
#define SERVER_FLAG_PEE         0x00000080
#define SERVER_FLAG_PEE_S       0x00000100
#define SERVER_FLAG_POO         0x00000200
#define SERVER_FLAG_POO_S       0x00000400
#define SERVER_FLAG_RELAX       0x00000800
#define SERVER_FLAG_RELAX_S     0x00001000
#define SERVER_FLAG_INQUEUE     0x80000000

#define SERVER_FLAGS_NEEDS      (SERVER_FLAG_HUNGER_S | SERVER_FLAG_THRIST_S | SERVER_FLAG_SLEEPY_S | SERVER_FLAG_PEE_S | SERVER_FLAG_POO_S | SERVER_FLAG_RELAX_S)

typedef struct {
    Customer_t *serving;
    double serviceTotalTime;
    unsigned int serviceCount;
    unsigned int statusFlags;
} ServerStatsAndStatistics_t;

typedef struct {
    double serviceCoef;
    double joinTime;
    double hungerMeanTime;
    double hungerStddev;
    double hungerFillMeanTime;
    double hungerFillStddev;
    double thristMeanTime;
    double thristFillMeanTime;
    double thristFillStddev;
    double sleepMeanTime;
    double sleepStddev;
    double sleepFillMeanTime;
    double sleepFillStddev;
    double peeMeanTime;
    double peeFillMeanTime;
    double peeFillStddev;
    double pooMeanTime;
    double pooStddev;
    double pooFillMeanTime;
    double pooFillStddev;
    double relaxMeanTime;
    double relaxStddev;
    double relaxFillMeanTime;
    double relaxFillStddev;
    unsigned int id;
    unsigned int priority;
    ServerStatsAndStatistics_t st;
} Server_t;

#define SEED_ARRIVAL        0
#define SEED_SERVICE        1
#define SEED_FEEL_HUNGER    2
#define SEED_FILL_HUNGER    3
#define SEED_FEEL_THRIST    4
#define SEED_FILL_THRIST    5
#define SEED_FEEL_SLEEPY    6
#define SEED_FILL_SLEEPY    7
#define SEED_FEEL_PEE       8
#define SEED_FILL_PEE       9
#define SEED_FEEL_POO       10
#define SEED_FILL_POO       11
#define SEED_FEEL_RELAX     12
#define SEED_FILL_RELAX     13
#define SEED_MAX            14

typedef struct {
    double arrivalRate;
    double serviceMean;
    unsigned long long seed[SEED_MAX];
    unsigned int elementsLeft;
} SimulationParameters_t;

typedef struct {
    HeapQueue_t eventQueue;
    HeapQueue_t serverQueue;
    LinkedQueue_t customerQueue;
    double currentTime;
    unsigned int nextCustomerId;
} SimulationStatus_t;

typedef struct {
    double customerSystemTimeTotal;
    unsigned int nCustomers;
} SimulationStatistics_t;

typedef struct {
    SimulationParameters_t p;
    SimulationStatus_t s;
    SimulationStatistics_t st;
} Simulation_t;

int SimulationInit(Simulation_t *s);
int SimulationDestroy(Simulation_t *s);

int SimulationSetup(Simulation_t *s);
int SimulationStart(Simulation_t *s);
int SimulationAddServer(Simulation_t *s, Server_t *server);

#endif
