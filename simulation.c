#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "simulation.h"

static double ibm_pseudo(unsigned long long *x)
{
    unsigned long long y;
    
    y = ((*x) * 16807) % 2147483547;
    *x = y;
    
    if (y == 0)
        return 2147483547.0 / 2147483548.0;
    else
        return (double)y / 2147483548.0;
}

static unsigned long long next_stream(unsigned long long seed, unsigned long long k, double (*fp)(unsigned long long *))
{
    for (unsigned long long i = 1; i < k; i++)
        fp(&seed);
    return seed;
}

static double inverse_normal(double r)
{
    return (pow(r, 0.135) - pow(1 - r, 0.135)) / 0.1975;
}

static double inverse_exponetial(double r, double m)
{
    return (-1.0 * m) * log(r);
}

static void GenerateCustomer(Customer_t *c, Simulation_t *s)
{
    c->arrivalTime = inverse_exponetial(ibm_pseudo((s->p.seed) + 0), s->p.arrivalRate) + s->s.currentTime;
    c->serviceTime = inverse_exponetial(ibm_pseudo((s->p.seed) + 1), s->p.serviceMean);
    c->id = (s->s.nextCustomerId)++;
}

static int event_comparer(const void *a, const void *b)
{
    Event_t *c = (Event_t *)a, *d = (Event_t *)b;
    
    if (c->eventTime < d->eventTime)
        return -1;
    else if (c->eventTime > d->eventTime)
        return 1;
    else
        return 0;
}

static int server_comparer(const void *a, const void *b)
{
    Server_t *c = (Server_t *)a, *d = (Server_t *)b;
    
    if (c->priority < d->priority)
        return -1;
    else if (c->priority > d->priority)
        return 1;
    else
        return 0;
}

// ================================================================

int SimulationInit(Simulation_t *s)
{
    memset(s, 0, sizeof(*s));
    
    s->p.elementsLeft = (unsigned int)-1;
    return HeapQueueInit(&(s->s.eventQueue), event_comparer) + HeapQueueInit(&(s->s.serverQueue), server_comparer) + LinkedQueueInit(&(s->s.customerQueue));
}

int SimulationDestroy(Simulation_t *s)
{
    return HeapQueueDestroy(&(s->s.eventQueue)) + HeapQueueDestroy(&(s->s.serverQueue)) + LinkedQueueDestroy(&(s->s.customerQueue));
}

int SimulationSetup(Simulation_t *s)
{
    Event_t *e;
    Customer_t *c;
    int ret;
    
    e = (Event_t *)malloc(sizeof(*e));
    c = (Customer_t *)malloc(sizeof(*c));
    if (!e || !c)
    {
        free(e);
        free(c);
        return -1;
    }
    
    GenerateCustomer(c, s);
    e->type = EVENT_ARRIVAL;
    e->data = c;
    e->eventTime = c->arrivalTime;
    
    ret = HeapQueueInsert(&(s->s.eventQueue), e);
    if (ret)
    {
        free(e);
        free(c);
        return ret;
    }
    
    printf("SimulationSetup: %u seeds.\n", (unsigned int)(sizeof(s->p.seed) / sizeof(s->p.seed[0])));
    printf("Seed 0: %llu\n", s->p.seed[0]);
    for (unsigned int i = 1; i < sizeof(s->p.seed) / sizeof(s->p.seed[0]); i++)
    {
        s->p.seed[i] = next_stream(s->p.seed[i - 1], SEED_STREAM_K, ibm_pseudo);
        printf("Seed %u: %llu\n", i, s->p.seed[i]);
    }
    
    printf("\n");
    return ret;
}

int SimulationStart(Simulation_t *s)
{
    Event_t *e;
    
    printf("SimulationStart:\n");
    if (!(s->p.elementsLeft))
        return 0;
    
    while (!HeapQueueRetrieve(&(s->s.eventQueue), (void **)&e))
    {
        s->s.currentTime = e->eventTime;
        printf("TIME      %lf\n", s->s.currentTime);
        switch (e->type)
        {
            case EVENT_ARRIVAL:
            {
                Event_t *newEvent;
                Customer_t *c = (Customer_t *)(e->data);
                Server_t *server;
                
                printf("ARRIVAL   %u: (%lf, %lf)\n", c->id, c->arrivalTime, c->serviceTime);
                if (HeapQueueRetrieve(&(s->s.serverQueue), (void **)&server))
                {
                    if (LinkedQueueInsert(&(s->s.customerQueue), c))
                    {
                        free(c);
                        free(e);
                        return -1;
                    }
                    
                    printf("          All servers are busy, %u customer(s) in queue.\n", LinkedQueueLength(&(s->s.customerQueue)));
                }
                else
                {
                    newEvent = (Event_t *)malloc(sizeof(*newEvent));
                    if (!newEvent)
                    {
                        free(c);
                        free(e);
                        return -1;
                    }
                    
                    server->serving = c;
                    newEvent->type = EVENT_DEPARTURE;
                    newEvent->eventTime = s->s.currentTime + (c->serviceTime * server->serviceCoef);
                    newEvent->data = server;
                    
                    if (HeapQueueInsert(&(s->s.eventQueue), newEvent))
                    {
                        free(c);
                        free(e);
                        free(newEvent);
                        return -1;
                    }
                    
                    printf("          Server %u goes busy.\n", server->id);
                    printf("          Departuare of customer ID %u (%lf, %lf) scheduled at %lf.\n", c->id, c->arrivalTime, c->serviceTime, newEvent->eventTime);
                }
                
                {
                    c = (Customer_t *)malloc(sizeof(*c));
                    newEvent = (Event_t *)malloc(sizeof(*newEvent));
                    if (!c || !newEvent)
                    {
                        free(c);
                        free(e);
                        free(newEvent);
                        return -1;
                    }
                    GenerateCustomer(c, s);
                    newEvent->type = EVENT_ARRIVAL;
                    newEvent->data = c;
                    newEvent->eventTime = c->arrivalTime;
                    
                    if (HeapQueueInsert(&(s->s.eventQueue), newEvent))
                    {
                        free(c);
                        free(e);
                        free(newEvent);
                        return -1;
                    }
                }
                
                break;
            }
            case EVENT_DEPARTURE:
            {
                Event_t *newEvent;
                Customer_t *c, *nextCustomer;
                Server_t *server = (Server_t *)(e->data);
                
                c = server->serving;
                server->serviceCount++;
                server->serviceTotalTime += c->serviceTime * server->serviceCoef;
                s->st.nCustomers++;
                s->st.customerSystemTimeTotal += c->serviceTime * server->serviceCoef;
                
                printf("DEPARTURE %u: (%lf, %lf)\n", c->id, c->arrivalTime, c->serviceTime);
                if (LinkedQueueRetrieve(&(s->s.customerQueue), (void **)&nextCustomer) == 0)
                {
                    newEvent = (Event_t *)malloc(sizeof(*newEvent));
                    if (!newEvent)
                    {
                        free(c);
                        free(e);
                        free(nextCustomer);
                        return -1;
                    }
                    
                    server->serving = nextCustomer;
                    newEvent->type = EVENT_DEPARTURE;
                    newEvent->eventTime = s->s.currentTime + (nextCustomer->serviceTime * server->serviceCoef);
                    newEvent->data = server;
                    
                    if (HeapQueueInsert(&(s->s.eventQueue), newEvent))
                    {
                        free(c);
                        free(e);
                        free(nextCustomer);
                        free(newEvent);
                        return -1;
                    }
                    
                    printf("          Server %u remains busy, %u customer(s) in queue.\n", server->id, LinkedQueueLength(&(s->s.customerQueue)));
                    printf("          Departuare of customer ID %u (%lf, %lf) scheduled at %lf.\n", nextCustomer->id, nextCustomer->arrivalTime, nextCustomer->serviceTime, newEvent->eventTime);
                    
                }
                else
                {
                    if (HeapQueueInsert(&(s->s.serverQueue), server))
                    {
                        free(c);
                        return -1;
                    }
                    printf("          Server %u goes idle.\n", server->id);
                }
                
                if (!(--(s->p.elementsLeft)))
                {
                    newEvent = (Event_t *)malloc(sizeof(*newEvent));
                    if (!newEvent)
                    {
                        free(c);
                        return -1;
                    }
                    
                    newEvent->type = EVENT_SIMULATION_STOP;
                    newEvent->eventTime = s->s.currentTime;
                    newEvent->data = NULL;
                    
                    if (HeapQueueInsert(&(s->s.eventQueue), newEvent))
                    {
                        free(c);
                        free(newEvent);
                        return -1;
                    }
                }
                
                free(c);
                break;
            }
            case EVENT_SIMULATION_STOP:
            {
                printf("SIMULATION_STOP\n");
                free(e);
                return 0;
            }
            default:
            {
                break;
            }
        }
        
        free(e);
    }
}