#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"

static Server_t servers[] = {
    // The hard worker
    {
        1.10, 0.0,                          // coef, join time
        21600.0, 3600.0, 1200.0, 300.0,     // Hunger
        7200.0, 180.0, 30.0,                // Thristy
        61200.0, 1800.0, 25200.0, 1800.0,   // Sleep
        14400.0, 180.0, 30.0,               // Pee
        86400.0, 21600.0, 600.0, 120.0,     // Poo
        2678400.0, 1.0, 1.0, 1.0,           // Relax
        0, 0                                // id, prio
    },
    
    // The king of peeing
    {
        1.00, 0.0,                          // coef, join time
        21600.0, 3600.0, 1200.0, 300.0,     // Hunger
        7200.0, 180.0, 30.0,                // Thristy
        61200.0, 1800.0, 25200.0, 1800.0,   // Sleep
        1800.0, 180.0, 30.0,                // Pee
        86400.0, 21600.0, 600.0, 120.0,     // Poo
        86400.0, 21600.0, 3600.0, 600.0,    // Relax
        1, 0                                // id, prio
    },
    
    // The king of pooing
    {
        1.00, 0.0,                          // coef, join time
        21600.0, 3600.0, 1200.0, 300.0,     // Hunger
        7200.0, 180.0, 30.0,                // Thristy
        61200.0, 1800.0, 25200.0, 1800.0,   // Sleep
        14400.0, 180.0, 30.0,               // Pee
        7200.0, 600.0, 600.0, 120.0,        // Poo
        86400.0, 21600.0, 3600.0, 600.0,    // Relax
        2, 0                                // id, prio
    },
    
    // The ultimate sleeper
    {
        1.00, 0.0,                          // coef, join time
        21600.0, 3600.0, 1200.0, 300.0,     // Hunger
        7200.0, 180.0, 30.0,                // Thristy
        43200.0, 1800.0, 43200.0, 1800.0,   // Sleep
        14400.0, 180.0, 30.0,               // Pee
        86400.0, 21600.0, 600.0, 120.0,     // Poo
        86400.0, 21600.0, 3600.0, 600.0,    // Relax
        3, 0                                // id, prio
    },
    
    // The ultimate slacker
    {
        1.00, 0.0,                          // coef, join time
        21600.0, 3600.0, 1200.0, 300.0,     // Hunger
        7200.0, 180.0, 30.0,                // Thristy
        61200.0, 1800.0, 25200.0, 1800.0,   // Sleep
        14400.0, 180.0, 30.0,               // Pee
        86400.0, 21600.0, 600.0, 120.0,     // Poo
        21600.0, 1800.0, 1800.0, 300.0,     // Relax
        4, 0                                // id, prio
    },
    
    // The king of eating
    {
        1.00, 0.0,                          // coef, join time
        7200.0, 600.0, 1200.0, 300.0,       // Hunger
        7200.0, 180.0, 30.0,                // Thristy
        61200.0, 1800.0, 25200.0, 1800.0,   // Sleep
        14400.0, 180.0, 30.0,               // Pee
        77760.0, 19440.0, 600.0, 120.0,     // Poo
        86400.0, 21600.0, 3600.0, 600.0,    // Relax
        5, 0                                // id, prio
    },
    
    // The man of water
    {
        1.00, 0.0,                          // coef, join time
        21600.0, 3600.0, 1200.0, 300.0,     // Hunger
        1800.0, 180.0, 30.0,                // Thristy
        61200.0, 1800.0, 25200.0, 1800.0,   // Sleep
        10800.0, 180.0, 30.0,               // Pee
        86400.0, 21600.0, 600.0, 120.0,     // Poo
        86400.0, 21600.0, 3600.0, 600.0,    // Relax
        6, 0                                // id, prio
    },
    
    // The ultimate multitasker
    {
        0.80, 0.0,                          // coef, join time
        21600.0, 3600.0, 1200.0, 300.0,     // Hunger
        7200.0, 180.0, 30.0,                // Thristy
        72000.0, 1800.0, 14400.0, 1800.0,   // Sleep
        14400.0, 180.0, 30.0,               // Pee
        86400.0, 21600.0, 600.0, 120.0,     // Poo
        86400.0, 21600.0, 3600.0, 600.0,    // Relax
        7, 0                                // id, prio
    },
    
    // The template
    /*
    {
        1.00, 0.0,                          // coef, join time
        21600.0, 3600.0, 1200.0, 300.0,     // Hunger
        7200.0, 180.0, 30.0,                // Thristy
        61200.0, 1800.0, 25200.0, 1800.0,   // Sleep
        7200.0, 180.0, 30.0,                // Pee
        86400.0, 21600.0, 600.0, 120.0,     // Poo
        86400.0, 21600.0, 3600.0, 600.0,    // Relax
        0, 0                                // id, prio
    },
    */
};

int main(void)
{
    Simulation_t s;
    double startTime = 0.0, u;
    Event_t *e;
    
    SimulationInit(&s);
    s.p.arrivalRate = 150.0;
    s.p.serviceMean = 600.0;
    s.p.seed[0] = time(NULL);
    s.p.elementsLeft = 0xFFFFFFFF;
    for (unsigned int i = 0; i < sizeof(servers) / sizeof(servers[0]); i++)
        SimulationAddServer(&s, servers + i);
    
    e = (Event_t *)malloc(sizeof(*e));
    e->type = EVENT_SIMULATION_STOP;
    e->eventTime = 2678400.0;
    e->data = NULL;
    HeapQueueInsert(&(s.s.eventQueue), e);
    
    SimulationSetup(&s);
    SimulationStart(&s);
    
    s.p.elementsLeft = 0xFFFFFFFF;
    for (unsigned int i = 0; i < sizeof(servers) / sizeof(servers[0]); i++)
        servers[i].st.serviceTotalTime = servers[i].st.serviceCount = 0;
    s.st.customerSystemTimeTotal = s.st.nCustomers = 0;
    startTime = s.s.currentTime;
    e = (Event_t *)malloc(sizeof(*e));
    e->type = EVENT_SIMULATION_STOP;
    e->eventTime = s.s.currentTime + 2678400.0;
    e->data = NULL;
    HeapQueueInsert(&(s.s.eventQueue), e);
    
    SimulationStart(&s);
    
    printf("Simulation Time: %lf\n", s.s.currentTime - startTime);
    for (unsigned int i = 0; i < sizeof(servers) / sizeof(servers[0]); i++)
    {
        u = 100.0 * servers[i].st.serviceTotalTime / (s.s.currentTime - startTime);
        if (u > 100.0)
            u = 100.0;
        
        printf("Server %u:\n", servers[i].id);
        printf("       Util: %5.1lf %%\n", u);
        printf("       Coef: %lf\n", servers[i].serviceCoef);
        printf("       Time: %lf\n", servers[i].st.serviceTotalTime);
        printf("       Cnt.: %u\n", servers[i].st.serviceCount);
        printf("       Prio: %u\n", servers[i].priority);
    }
    printf("Number of customers: %u\n", s.st.nCustomers);
    printf("Total system time: %lf\n", s.st.customerSystemTimeTotal);
    printf("Average system time: %lf\n", s.st.customerSystemTimeTotal / s.st.nCustomers);
    
    return 0;
}