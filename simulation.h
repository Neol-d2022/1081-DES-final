#ifndef _SIMULATION_H_
#define _SIMULATION_H_

#include "heapqueue.h"
#include "linkedqueue.h"

#define SEED_STREAM_K 10000000

typedef enum {
    EVENT_UNKNOWN,
    EVENT_ARRIVAL,
    EVENT_DEPARTURE,
    EVENT_SIMULATION_STOP
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

typedef struct {
    Customer_t *serving;
    double serviceCoef;
    double serviceTotalTime;
    unsigned int id;
    unsigned int priority;
    unsigned int serviceCount;
} Server_t;

typedef struct {
    double arrivalRate;
    double serviceMean;
    unsigned long long seed[8];
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

#endif
