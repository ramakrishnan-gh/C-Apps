#include<iostream>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
using namespace std;

int a[10];
int rPtr = -1;
int wPtr = -1;

sem_t seqSem;

sem_t prodSem;
sem_t consSem;

pthread_mutex_t mMutex;
pthread_t prodThread, consThread;

void produce()
{
   sem_post(&prodSem);	
}

void consume()
{
   sem_post(&consSem);
}

int getNextPtrVal(int currVal)
{
   if ( currVal == -1 )
   {
	return 0;
   }
   else
   {
       if ( currVal < 9 )
	return (currVal+1);
       else
        return 0;
   }
}

void display()
{
    pthread_mutex_lock(&mMutex);
    for ( int i = 0 ; i < 10; i++ )
	cout << "a["<< i << "]=" << a[i] << endl;	

    cout << "rPtr is " << rPtr << "wPtr is " << wPtr << endl;
    pthread_mutex_unlock(&mMutex);
}

void * producer(void *arg )
{
   while (1)
   {
	   cout << "Waiting for prod call" << endl;
	   sem_wait(&prodSem);
	   cout << "Got a prod call" << endl;
	   do
	   {
		   pthread_mutex_lock(&mMutex);
		   cout << "Got the sync mutex" << endl;
		   int newWPtr;
		   bool goodToWrite = false;			   
                   if ( rPtr == -1 )
		   { 
			if ( wPtr < 9 )
                        {
                            newWPtr = wPtr + 1; 
			    goodToWrite = true;
			}
		   }
                   else
		   {
			newWPtr = getNextPtrVal(wPtr);
			if ( newWPtr != rPtr )
				goodToWrite = true;
		   }
		   if ( goodToWrite )			
		   {
		      wPtr = newWPtr;
		      a[wPtr] = 10*wPtr;
		      cout << "Wrote at [" << wPtr <<"] value [" << a[wPtr] <<"]" << endl;
		      pthread_mutex_unlock(&mMutex);
		      sem_post(&seqSem);
		      break;	
		   }
		   else
		   {
		      int semVal;
		      cout << " Can't write as buff full with rPtr is "  << rPtr << ", wPtr is:" << wPtr << endl;
		      sem_getvalue(&seqSem, &semVal);
		      cout << " Value of seq sem before is" << semVal << endl; 
		      pthread_mutex_unlock(&mMutex);
		      sem_wait(&seqSem);
		      cout << " Value of seq sem after is" << semVal << endl; 
		   }
	    }while(1);
	}
}

void * consumer(void * arg )
{
   sem_wait(&seqSem); //Reader can't start till the 1st write is done..
   while (1)
   {
	   cout << "Waiting for consume call" << endl;
	   sem_wait(&consSem);
	   cout << "Got a consume call" << endl;
	   do
	   {
		   pthread_mutex_lock(&mMutex);
		   cout << "Got the sync mutex" << endl;
		   int newRPtr;
                   bool goodToRead = false;
		   if ( wPtr == 9 )
		   {
		   	if ( rPtr < 9 )
		   	{
			   newRPtr = getNextPtrVal(rPtr);
			      goodToRead = true;
		   	}
                   }
                   else if ( wPtr != -1 )
		   {
			newRPtr = getNextPtrVal(rPtr);
		 	if (newRPtr <= wPtr )
				goodToRead = true;
		   }
		   if ( goodToRead )
		   {
		      rPtr = newRPtr;
		      cout << "Yipee - read done on rPtr[ " << rPtr << "] value is [" << a[rPtr] << "]" << endl; 
		      pthread_mutex_unlock(&mMutex);
		      sem_post(&seqSem);
		      break;	
		   }
		   else
		   {
		      int semVal;
		      cout << " Can't read as buff is empty with wPtr is "  << wPtr << ", rPtr is:" << rPtr << endl;
		      sem_getvalue(&seqSem, &semVal);
		      cout << " Value of seq sem before is" << semVal << endl; 
		      pthread_mutex_unlock(&mMutex);
		      sem_wait(&seqSem);
		      cout << " Value of seq sem after is" << semVal << endl; 
		   }
	    }while(1);
	}
}

int main()
{

    sem_init(&seqSem, 0, 0);
    sem_init(&prodSem, 0, 0);
    sem_init(&consSem, 0, 0);
    memset(a, 0, 10*sizeof(int));
    int retVal = pthread_create(&prodThread, NULL, producer, NULL);
    retVal = pthread_create(&consThread, NULL, consumer, NULL);
    int option;
    while(1)
    {
        cout << "EHllo1" << endl;
        fflush(stdin);
        cout << "Enter your option 0/1/2:";
	scanf("%d", &option);
        cout << "Hello2, option"<< option << endl;
        switch(option)
        {
	    case 0:
		produce();
		break;
	    case 1:
		consume();
		break;
	    case 2:
		display();
		break;	
	}     
	sleep(1);
    }
    pthread_join(prodThread,NULL);
    pthread_join(consThread,NULL);
    return 0;
}
