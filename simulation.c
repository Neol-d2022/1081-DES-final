#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "simulation.h"

/*static double ibm_pseudo(unsigned long long *x)
{
    unsigned long long y;
    
    y = ((*x) * 16807) % 2147483547;
    *x = y;
    
    if (y == 0)
        return 2147483547.0 / 2147483548.0;
    else
        return (double)y / 2147483548.0;
}*/

static double lehmer_pseudo(unsigned long long *x)
{
    unsigned long long y;
    
    y = ((*x) * 279470273llu) % 0xfffffffbllu;
    *x = y;
    
    if (y == 0)
        return 4294967295.0 / 4294967296.0;
    else
        return (double)y / 4294967296.0;
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
    c->arrivalTime = inverse_exponetial(lehmer_pseudo((s->p.seed) + SEED_ARRIVAL), s->p.arrivalRate) + s->s.currentTime;
    c->serviceTime = inverse_exponetial(lehmer_pseudo((s->p.seed) + SEED_SERVICE), s->p.serviceMean);
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

static inline int server_busy(const Server_t *server)
{
    return (server->st.statusFlags & ~SERVER_FLAG_INQUEUE) ? (-1) : (0);
}

static int server_fill_needs(Server_t *server, Simulation_t *s, int *ready)
{
    double t;
    Event_t *newEvent;
    int ret = 0;
    
    if (server->st.statusFlags & SERVER_FLAG_SERVING)
        *ready = 0;
    else if (server->st.statusFlags & SERVER_FLAG_PEE)
    {
        *ready = 0;
        
        if (!(server->st.statusFlags & SERVER_FLAGS_NEEDS))
        {
            newEvent = (Event_t *)malloc(sizeof(*newEvent));
            if (newEvent)
            {
                t = inverse_normal(lehmer_pseudo(s->p.seed + SEED_FILL_PEE)) * server->peeFillStddev + server->peeFillMeanTime;
                if (t < 1.0)
                    t = 1.0;
                
                newEvent->type = EVENT_FILL_PEE;
                newEvent->eventTime = s->s.currentTime + t;
                newEvent->data = server;
                
                ret = HeapQueueInsert(&(s->s.eventQueue), newEvent);
                if (ret == 0)
                {
                    server->st.statusFlags |= SERVER_FLAG_PEE_S;
					server->st.timeFillingNeeds = t;
                    printf("%17.6lf   Server %u is peeing (takes %lf, til %lf)\n", s->s.currentTime, server->id, t, newEvent->eventTime);
                    printf("%17.6lf   %u customer(s) in queue.\n", s->s.currentTime, LinkedQueueLength(&(s->s.customerQueue)));
                }
            }
            else
                ret = 1;
        }
    }
    else if (server->st.statusFlags & SERVER_FLAG_THIRST)
    {
        *ready = 0;
        
        if (!(server->st.statusFlags & SERVER_FLAGS_NEEDS))
        {
            newEvent = (Event_t *)malloc(sizeof(*newEvent));
            if (newEvent)
            {
                t = inverse_normal(lehmer_pseudo(s->p.seed + SEED_FILL_THIRST)) * server->thristFillStddev + server->thristFillMeanTime;
                if (t < 1.0)
                    t = 1.0;
                
                newEvent->type = EVENT_FILL_THIRST;
                newEvent->eventTime = s->s.currentTime + t;
                newEvent->data = server;
                
                ret = HeapQueueInsert(&(s->s.eventQueue), newEvent);
                if (ret == 0)
                {
                    server->st.statusFlags |= SERVER_FLAG_THIRST_S;
					server->st.timeFillingNeeds = t;
                    printf("%17.6lf   Server %u is drinking (takes %lf, til %lf)\n", s->s.currentTime, server->id, t, newEvent->eventTime);
                    printf("%17.6lf   %u customer(s) in queue.\n", s->s.currentTime, LinkedQueueLength(&(s->s.customerQueue)));
                }
            }
            else
                ret = 1;
        }
    }
    else if (server->st.statusFlags & SERVER_FLAG_POO)
    {
        *ready = 0;
        
        if (!(server->st.statusFlags & SERVER_FLAGS_NEEDS))
        {
            newEvent = (Event_t *)malloc(sizeof(*newEvent));
            if (newEvent)
            {
                t = inverse_normal(lehmer_pseudo(s->p.seed + SEED_FILL_POO)) * server->pooFillStddev + server->pooFillMeanTime;
                if (t < 1.0)
                    t = 1.0;
                
                newEvent->type = EVENT_FILL_POO;
                newEvent->eventTime = s->s.currentTime + t;
                newEvent->data = server;
                
                ret = HeapQueueInsert(&(s->s.eventQueue), newEvent);
                if (ret == 0)
                {
                    server->st.statusFlags |= SERVER_FLAG_POO_S;
					server->st.timeFillingNeeds = t;
                    printf("%17.6lf   Server %u is pooing (takes %lf, til %lf)\n", s->s.currentTime, server->id, t, newEvent->eventTime);
                    printf("%17.6lf   %u customer(s) in queue.\n", s->s.currentTime, LinkedQueueLength(&(s->s.customerQueue)));
                }
            }
            else
                ret = 1;
        }
    }
    else if (server->st.statusFlags & SERVER_FLAG_HUNGER)
    {
        *ready = 0;
        
        if (!(server->st.statusFlags & SERVER_FLAGS_NEEDS))
        {
            newEvent = (Event_t *)malloc(sizeof(*newEvent));
            if (newEvent)
            {
                t = inverse_normal(lehmer_pseudo(s->p.seed + SEED_FILL_HUNGER)) * server->hungerFillStddev + server->hungerFillMeanTime;
                if (t < 1.0)
                    t = 1.0;
                
                newEvent->type = EVENT_FILL_HUNGER;
                newEvent->eventTime = s->s.currentTime + t;
                newEvent->data = server;
                
                ret = HeapQueueInsert(&(s->s.eventQueue), newEvent);
                if (ret == 0)
                {
                    server->st.statusFlags |= SERVER_FLAG_HUNGER_S;
					server->st.timeFillingNeeds = t;
                    printf("%17.6lf   Server %u is eating (takes %lf, til %lf)\n", s->s.currentTime, server->id, t, newEvent->eventTime);
                    printf("%17.6lf   %u customer(s) in queue.\n", s->s.currentTime, LinkedQueueLength(&(s->s.customerQueue)));
                }
            }
            else
                ret = 1;
        }
    }
    else if (server->st.statusFlags & SERVER_FLAG_RELAX)
    {
        *ready = 0;
        
        if (!(server->st.statusFlags & SERVER_FLAGS_NEEDS))
        {
            newEvent = (Event_t *)malloc(sizeof(*newEvent));
            if (newEvent)
            {
                t = inverse_normal(lehmer_pseudo(s->p.seed + SEED_FILL_RELAX)) * server->relaxFillStddev + server->relaxFillMeanTime;
                if (t < 1.0)
                    t = 1.0;
                
                newEvent->type = EVENT_FILL_RELAX;
                newEvent->eventTime = s->s.currentTime + t;
                newEvent->data = server;
                
                ret = HeapQueueInsert(&(s->s.eventQueue), newEvent);
                if (ret == 0)
                {
                    server->st.statusFlags |= SERVER_FLAG_RELAX_S;
					server->st.timeFillingNeeds = t;
                    printf("%17.6lf   Server %u is relaxing (takes %lf, til %lf)\n", s->s.currentTime, server->id, t, newEvent->eventTime);
                    printf("%17.6lf   %u customer(s) in queue.\n", s->s.currentTime, LinkedQueueLength(&(s->s.customerQueue)));
                }
            }
            else
                ret = 1;
        }
    }
    else if (server->st.statusFlags & SERVER_FLAG_SLEEPY)
    {
        *ready = 0;
        
        if (!(server->st.statusFlags & SERVER_FLAGS_NEEDS))
        {
            newEvent = (Event_t *)malloc(sizeof(*newEvent));
            if (newEvent)
            {
                t = inverse_normal(lehmer_pseudo(s->p.seed + SEED_FILL_SLEEPY)) * server->sleepFillStddev + server->sleepFillMeanTime;
                if (t < 1.0)
                    t = 1.0;
                
                newEvent->type = EVENT_FILL_SLEEPY;
                newEvent->eventTime = s->s.currentTime + t;
                newEvent->data = server;
                
                ret = HeapQueueInsert(&(s->s.eventQueue), newEvent);
                if (ret == 0)
                {
                    server->st.statusFlags |= SERVER_FLAG_SLEEPY_S;
					server->st.timeFillingNeeds = t;
                    printf("%17.6lf   Server %u is sleeping (takes %lf, til %lf)\n", s->s.currentTime, server->id, t, newEvent->eventTime);
                    printf("%17.6lf   %u customer(s) in queue.\n", s->s.currentTime, LinkedQueueLength(&(s->s.customerQueue)));
                }
            }
            else
                ret = 1;
        }
    }
    else
        *ready = -1;
    
    return ret;
}

static int server_idle(Server_t *server, Simulation_t *s)
{
    Event_t *newEvent;
    Customer_t *nextCustomer;
    int ready;
    
    if (server_fill_needs(server, s, &ready))
        return -1;
    
    if (!ready)
    {
        // Not ready for next customer.
    }
    else if (LinkedQueueRetrieve(&(s->s.customerQueue), (void **)&nextCustomer) == 0)
    {
        newEvent = (Event_t *)malloc(sizeof(*newEvent));
        if (!newEvent)
        {
            free(nextCustomer);
            return -1;
        }
        
        server->st.serving = nextCustomer;
        server->st.statusFlags |= SERVER_FLAG_SERVING;
        newEvent->type = EVENT_DEPARTURE;
        newEvent->eventTime = s->s.currentTime + (nextCustomer->serviceTime * server->serviceCoef);
        newEvent->data = server;
        
        if (HeapQueueInsert(&(s->s.eventQueue), newEvent))
        {
            free(nextCustomer);
            free(newEvent);
            return -1;
        }
        
        printf("%17.6lf   Server %u remains busy\n", s->s.currentTime, server->id);
        printf("%17.6lf   %u customer(s) in queue.\n", s->s.currentTime, LinkedQueueLength(&(s->s.customerQueue)));
        printf("%17.6lf   Departuare of customer ID %u (%lf, %lf) scheduled at %lf.\n", s->s.currentTime, nextCustomer->id, nextCustomer->arrivalTime, nextCustomer->serviceTime, newEvent->eventTime);
    }
    else
    {
        if (!(server->st.statusFlags & SERVER_FLAG_INQUEUE))
        {
            if (HeapQueueInsert(&(s->s.serverQueue), server))
                return -1;
            server->st.statusFlags |=  SERVER_FLAG_INQUEUE;
        }
        printf("%17.6lf   Server %u goes idle.\n", s->s.currentTime, server->id);
    }
    
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
    
    printf("SimulationSetup: %u seeds.\n", (unsigned int)(sizeof(s->p.seed) / sizeof(s->p.seed[0])));
    printf("Seed 0: %u\n", (unsigned int)(s->p.seed[0]));
    for (unsigned int i = 1; i < sizeof(s->p.seed) / sizeof(s->p.seed[0]); i++)
    {
        s->p.seed[i] = next_stream(s->p.seed[i - 1], SEED_STREAM_K, lehmer_pseudo);
        printf("Seed %u: %u\n", i, (unsigned int)(s->p.seed[i]));
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
    
    printf("\n");
    return ret;
}

int SimulationStart(Simulation_t *s)
{
    Event_t *e, *newEvent;
    
    printf("SimulationStart:\n");
    if (!(s->p.elementsLeft))
        return 0;
    
    while (!HeapQueueRetrieve(&(s->s.eventQueue), (void **)&e))
    {
		s->st.customerQueueLengthTotal += (e->eventTime - s->s.currentTime) * LinkedQueueLength(&(s->s.customerQueue));
        s->s.currentTime = e->eventTime;
        switch (e->type)
        {
            case EVENT_ARRIVAL:
            {
                Customer_t *c = (Customer_t *)(e->data);
                Server_t *server;
                
                printf("%17.6lf   ARRIVAL   %u: (%lf, %lf)\n", s->s.currentTime, c->id, c->arrivalTime, c->serviceTime);
retry_next_server:
                if (HeapQueueRetrieve(&(s->s.serverQueue), (void **)&server))
                {
                    if (LinkedQueueInsert(&(s->s.customerQueue), c))
                    {
                        free(c);
                        free(e);
                        return -1;
                    }
                    
                    printf("%17.6lf   All servers are busy\n", s->s.currentTime);
                    printf("%17.6lf   %u customer(s) in queue.\n", s->s.currentTime, LinkedQueueLength(&(s->s.customerQueue)));
                }
                else
                {
                    server->st.statusFlags &= ~SERVER_FLAG_INQUEUE;
                    if (server_busy(server))
                        goto retry_next_server;
                    
                    newEvent = (Event_t *)malloc(sizeof(*newEvent));
                    if (!newEvent)
                    {
                        free(c);
                        free(e);
                        return -1;
                    }
                    
                    server->st.serving = c;
                    server->st.statusFlags |= SERVER_FLAG_SERVING;
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
                    
                    printf("%17.6lf   Server %u goes busy.\n", s->s.currentTime, server->id);
                    printf("%17.6lf   %u customer(s) in queue.\n", s->s.currentTime, LinkedQueueLength(&(s->s.customerQueue)));
                    printf("%17.6lf   Departuare of customer ID %u (%lf, %lf) scheduled at %lf.\n", s->s.currentTime, c->id, c->arrivalTime, c->serviceTime, newEvent->eventTime);
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
                Customer_t *c;
                Server_t *server = (Server_t *)(e->data);
                
                c = server->st.serving;
                server->st.serviceCount++;
                server->st.serviceTotalTime += c->serviceTime * server->serviceCoef;
				server->st.customerSystemTimeTotal += s->s.currentTime - c->arrivalTime;
                server->st.statusFlags &= ~SERVER_FLAG_SERVING;
                s->st.nCustomers++;
                s->st.customerSystemTimeTotal += s->s.currentTime - c->arrivalTime;
                s->st.customerResponseTimeTotal += s->s.currentTime - c->arrivalTime - (c->serviceTime * server->serviceCoef);
                printf("%17.6lf   DEPARTURE %u: (%lf, %lf)\n", s->s.currentTime, c->id, c->arrivalTime, c->serviceTime);
                
                if (server_idle(server, s))
                {
                    free(c);
                    free(e);
                    return -1;
                }
                
                if (!(--(s->p.elementsLeft)))
                {
                    newEvent = (Event_t *)malloc(sizeof(*newEvent));
                    if (!newEvent)
                    {
                        free(c);
                        free(e);
                        return -1;
                    }
                    
                    newEvent->type = EVENT_SIMULATION_STOP;
                    newEvent->eventTime = s->s.currentTime;
                    newEvent->data = NULL;
                    
                    if (HeapQueueInsert(&(s->s.eventQueue), newEvent))
                    {
                        free(c);
                        free(e);
                        free(newEvent);
                        return -1;
                    }
                }
                
                free(c);
                break;
            }
            case EVENT_FEEL_HUNGER:
            {
                Server_t *server = (Server_t *)(e->data);
                int ready;
                
                printf("%17.6lf   HUNGER(-) %u\n", s->s.currentTime, server->id);
                server->st.statusFlags |= SERVER_FLAG_HUNGER;
                if (server_fill_needs(server, s, &ready))
                {
                    free(e);
                    return -1;
                }
                break;
            }
            case EVENT_FILL_HUNGER:
            {
                double t;
                Server_t *server = (Server_t *)(e->data);
                server->st.statusFlags &= ~SERVER_FLAG_HUNGER;
                server->st.statusFlags &= ~SERVER_FLAG_HUNGER_S;
				server->st.hungryCount++;
				server->st.timeEating += server->st.timeFillingNeeds;
                
                printf("%17.6lf   HUNGER(+) %u\n", s->s.currentTime, server->id);
                if (server_idle(server, s))
                {
                    free(e);
                    return -1;
                }
                
                newEvent = (Event_t *)malloc(sizeof(*newEvent));
                if (!newEvent)
                {
                    free(e);
                    return -1;
                }
                
                t = inverse_normal(lehmer_pseudo(s->p.seed + SEED_FEEL_HUNGER)) * (server->hungerStddev) + server->hungerMeanTime;
                if (t < 1.0)
                    t = 1.0;
                newEvent->type = EVENT_FEEL_HUNGER;
                newEvent->eventTime = s->s.currentTime + t;
                newEvent->data = server;
                
                if (HeapQueueInsert(&(s->s.eventQueue), newEvent))
                {
                    free(e);
                    free(newEvent);
                    return -1;
                }
                break;
            }
            case EVENT_FEEL_THIRST:
            {
                Server_t *server = (Server_t *)(e->data);
                int ready;
                
                printf("%17.6lf   THIRST(-) %u\n", s->s.currentTime, server->id);
                server->st.statusFlags |= SERVER_FLAG_THIRST;
                if (server_fill_needs(server, s, &ready))
                {
                    free(e);
                    return -1;
                }
                break;
            }
            case EVENT_FILL_THIRST:
            {
                double t;
                Server_t *server = (Server_t *)(e->data);
                server->st.statusFlags &= ~SERVER_FLAG_THIRST;
                server->st.statusFlags &= ~SERVER_FLAG_THIRST_S;
				server->st.thirstyCount++;
				server->st.timeDrinking += server->st.timeFillingNeeds;
                
                printf("%17.6lf   THIRST(+) %u\n", s->s.currentTime, server->id);
                if (server_idle(server, s))
                {
                    free(e);
                    return -1;
                }
                
                newEvent = (Event_t *)malloc(sizeof(*newEvent));
                if (!newEvent)
                {
                    free(e);
                    return -1;
                }
                
                t = inverse_exponetial(lehmer_pseudo(s->p.seed + SEED_FEEL_THIRST), server->thristMeanTime);
                if (t < 1.0)
                    t = 1.0;
                newEvent->type = EVENT_FEEL_THIRST;
                newEvent->eventTime = s->s.currentTime + t;
                newEvent->data = server;
                
                if (HeapQueueInsert(&(s->s.eventQueue), newEvent))
                {
                    free(e);
                    free(newEvent);
                    return -1;
                }
                break;
            }
            case EVENT_FEEL_SLEEPY:
            {
                Server_t *server = (Server_t *)(e->data);
                int ready;
                
                printf("%17.6lf   SLEEPY(-) %u\n", s->s.currentTime, server->id);
                server->st.statusFlags |= SERVER_FLAG_SLEEPY;
                if (server_fill_needs(server, s, &ready))
                {
                    free(e);
                    return -1;
                }
                break;
            }
            case EVENT_FILL_SLEEPY:
            {
                double t;
                Server_t *server = (Server_t *)(e->data);
                server->st.statusFlags &= ~SERVER_FLAG_SLEEPY;
                server->st.statusFlags &= ~SERVER_FLAG_SLEEPY_S;
				server->st.sleepyCount++;
				server->st.timeSleeping += server->st.timeFillingNeeds;
                
                printf("%17.6lf   SLEEPY(+) %u\n", s->s.currentTime, server->id);
                if (server_idle(server, s))
                {
                    free(e);
                    return -1;
                }
                
                newEvent = (Event_t *)malloc(sizeof(*newEvent));
                if (!newEvent)
                {
                    free(e);
                    return -1;
                }
                
                t = inverse_normal(lehmer_pseudo(s->p.seed + SEED_FEEL_SLEEPY)) * (server->sleepStddev) + server->sleepMeanTime;
                if (t < 1.0)
                    t = 1.0;
                newEvent->type = EVENT_FEEL_SLEEPY;
                newEvent->eventTime = s->s.currentTime + t;
                newEvent->data = server;
                
                if (HeapQueueInsert(&(s->s.eventQueue), newEvent))
                {
                    free(e);
                    free(newEvent);
                    return -1;
                }
                break;
            }
            case EVENT_FEEL_PEE:
            {
                Server_t *server = (Server_t *)(e->data);
                int ready;
                
                printf("%17.6lf   PEE(-)    %u\n", s->s.currentTime, server->id);
                server->st.statusFlags |= SERVER_FLAG_PEE;
                if (server_fill_needs(server, s, &ready))
                {
                    free(e);
                    return -1;
                }
                break;
            }
            case EVENT_FILL_PEE:
            {
                double t;
                Server_t *server = (Server_t *)(e->data);
                server->st.statusFlags &= ~SERVER_FLAG_PEE;
                server->st.statusFlags &= ~SERVER_FLAG_PEE_S;
				server->st.peeCount++;
				server->st.timePeeing += server->st.timeFillingNeeds;
                
                printf("%17.6lf   PEE(+)    %u\n", s->s.currentTime, server->id);
                if (server_idle(server, s))
                {
                    free(e);
                    return -1;
                }
                
                newEvent = (Event_t *)malloc(sizeof(*newEvent));
                if (!newEvent)
                {
                    free(e);
                    return -1;
                }
                
                t = inverse_exponetial(lehmer_pseudo(s->p.seed + SEED_FEEL_PEE), server->peeMeanTime);
                if (t < 1.0)
                    t = 1.0;
                newEvent->type = EVENT_FEEL_PEE;
                newEvent->eventTime = s->s.currentTime + t;
                newEvent->data = server;
                
                if (HeapQueueInsert(&(s->s.eventQueue), newEvent))
                {
                    free(e);
                    free(newEvent);
                    return -1;
                }
                break;
            }
            case EVENT_FEEL_POO:
            {
                Server_t *server = (Server_t *)(e->data);
                int ready;
                
                printf("%17.6lf   POO(-)    %u\n", s->s.currentTime, server->id);
                server->st.statusFlags |= SERVER_FLAG_POO;
                if (server_fill_needs(server, s, &ready))
                {
                    free(e);
                    return -1;
                }
                break;
            }
            case EVENT_FILL_POO:
            {
                double t;
                Server_t *server = (Server_t *)(e->data);
                server->st.statusFlags &= ~SERVER_FLAG_POO;
                server->st.statusFlags &= ~SERVER_FLAG_POO_S;
				server->st.pooCount++;
				server->st.timePooing += server->st.timeFillingNeeds;
                
                printf("%17.6lf   POO(+)    %u\n", s->s.currentTime, server->id);
                if (server_idle(server, s))
                {
                    free(e);
                    return -1;
                }
                
                newEvent = (Event_t *)malloc(sizeof(*newEvent));
                if (!newEvent)
                {
                    free(e);
                    return -1;
                }
                
                t = inverse_normal(lehmer_pseudo(s->p.seed + SEED_FEEL_POO)) * (server->pooStddev) + server->pooMeanTime;
                if (t < 1.0)
                    t = 1.0;
                newEvent->type = EVENT_FEEL_POO;
                newEvent->eventTime = s->s.currentTime + t;
                newEvent->data = server;
                
                if (HeapQueueInsert(&(s->s.eventQueue), newEvent))
                {
                    free(e);
                    free(newEvent);
                    return -1;
                }
                break;
            }
            case EVENT_FEEL_RELAX:
            {
                Server_t *server = (Server_t *)(e->data);
                int ready;
                
                printf("%17.6lf   RELAX(-)  %u\n", s->s.currentTime, server->id);
                server->st.statusFlags |= SERVER_FLAG_RELAX;
                if (server_fill_needs(server, s, &ready))
                {
                    free(e);
                    return -1;
                }
                break;
            }
            case EVENT_FILL_RELAX:
            {
                double t;
                Server_t *server = (Server_t *)(e->data);
                server->st.statusFlags &= ~SERVER_FLAG_RELAX;
                server->st.statusFlags &= ~SERVER_FLAG_RELAX_S;
				server->st.relaxCount++;
				server->st.timeRelaxing += server->st.timeFillingNeeds;
                
                printf("%17.6lf   RELAX(+)  %u\n", s->s.currentTime, server->id);
                if (server_idle(server, s))
                {
                    free(e);
                    return -1;
                }
                
                newEvent = (Event_t *)malloc(sizeof(*newEvent));
                if (!newEvent)
                {
                    free(e);
                    return -1;
                }
                
                t = inverse_normal(lehmer_pseudo(s->p.seed + SEED_FEEL_RELAX)) * (server->relaxStddev) + server->relaxMeanTime;
                if (t < 1.0)
                    t = 1.0;
                newEvent->type = EVENT_FEEL_RELAX;
                newEvent->eventTime = s->s.currentTime + t;
                newEvent->data = server;
                
                if (HeapQueueInsert(&(s->s.eventQueue), newEvent))
                {
                    free(e);
                    free(newEvent);
                    return -1;
                }
                break;
            }
            case EVENT_SIMULATION_STOP:
            {
                printf("%17.6lf   SIMULATION_STOP\n", s->s.currentTime);
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
    
    return 0;
}

int SimulationAddServer(Simulation_t *s, Server_t *server)
{
    EventType_t needs[] = {EVENT_FILL_HUNGER, EVENT_FILL_THIRST, EVENT_FILL_SLEEPY, EVENT_FILL_PEE, EVENT_FILL_POO, EVENT_FILL_RELAX};
    Event_t *newEvent;
    unsigned int i, n = sizeof(needs) / sizeof(needs[0]);
    
	memset(&(server->st), 0, sizeof(server->st));
    server->st.statusFlags |= (SERVER_FLAG_HUNGER | SERVER_FLAG_HUNGER_S);
	server->st.hungryCount = server->st.thirstyCount = server->st.sleepyCount = server->st.peeCount = server->st.pooCount = server->st.relaxCount = 0xFFFFFFFF;
    for (i = 0; i < n; i++)
    {
        newEvent = (Event_t *)malloc(sizeof(*newEvent));
        if (!newEvent)
            return -1;
        
        newEvent->type = needs[i];
        newEvent->eventTime = s->s.currentTime + server->joinTime;
        newEvent->data = server;
        
        if (HeapQueueInsert(&(s->s.eventQueue), newEvent))
        {
            free(newEvent);
            return -1;
        }
    }
    
    return 0;
}
