#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "simulation.h"

static Server_t servers[] = {
    {NULL, 1.00, 0.0, 0, 0, 0},
    {NULL, 1.00, 0.0, 1, 1, 0},
    {NULL, 1.00, 0.0, 2, 1, 0},
    {NULL, 1.00, 0.0, 3, 1, 0},
    {NULL, 1.00, 0.0, 4, 1, 0},
};

int main(void)
{
    Simulation_t s;
    double startTime, u;
    
    SimulationInit(&s);
    s.p.arrivalRate = 1.0;
    s.p.serviceMean = 4.0;
    s.p.seed[0] = time(NULL);
    s.p.elementsLeft = 1024;
    for (unsigned int i = 0; i < sizeof(servers) / sizeof(servers[0]); i++)
        HeapQueueInsert(&(s.s.serverQueue), servers + i);
    
    SimulationSetup(&s);
    SimulationStart(&s);
    
    s.p.elementsLeft = 16384;
    for (unsigned int i = 0; i < sizeof(servers) / sizeof(servers[0]); i++)
        servers[i].serviceTotalTime = servers[i].serviceCount = 0;
    s.st.customerSystemTimeTotal = s.st.nCustomers = 0;
    startTime = s.s.currentTime;
    SimulationStart(&s);
    
    printf("Simulation Time: %lf\n", s.s.currentTime - startTime);
    for (unsigned int i = 0; i < sizeof(servers) / sizeof(servers[0]); i++)
    {
        u = 100.0 * servers[i].serviceTotalTime / (s.s.currentTime - startTime);
        if (u > 100.0)
            u = 100.0;
        
        printf("Server %u:\n", servers[i].id);
        printf("       Util: %5.1lf %%\n", u);
        printf("       Coef: %lf\n", servers[i].serviceCoef);
        printf("       Time: %lf\n", servers[i].serviceTotalTime);
        printf("       Count: %u\n", servers[i].serviceCount);
        printf("       Prio.: %u\n", servers[i].priority);
    }
    printf("Number of customers: %u\n", s.st.nCustomers);
    printf("Total system time: %lf\n", s.st.customerSystemTimeTotal);
    printf("Average system time: %lf\n", s.st.customerSystemTimeTotal / s.st.nCustomers);
    
    return 0;
}