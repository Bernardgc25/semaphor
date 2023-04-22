/*	Bernard J. Gole Cruz 
	CS-370
	Project 3: sleeping professor 
*/


#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <stdbool.h>


# define STUDENT_COUNT_MIN 	2
# define STUDENT_COUNT_MAX 	10
# define CHAIR_COUNT 		3
# define HELPS_MAX			3

//mutex lock 
pthread_mutex_t Hchair;

//semaphores 
sem_t professor_sem, students_sem, chair_sem; 

//global variables
int *helpcount = NULL; 
int waiting = 0;
int awake = 0;

//thread function 
void *professor(void *arg);
void *students(void *arg);


int main()
{
	//semaphore variables
	sem_init(&professor_sem, 0,0);		//professor sleeping/available
	sem_init(&students_sem, 0,0);		//students need help/don't need help
	sem_init(&chair_sem, 0,0);			//chair semaphore
		
	pthread_mutex_init(&Hchair, NULL);

	//thread variables
	pthread_t professorthd;
	pthread_t *studentsthd = NULL;
	pthread_attr_t attr; 

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	void *status;

	int tmpNum;

    //number of students
	printf("How many students coming to professor's office?");
    scanf("%d", &tmpNum); 
   
   	//dynamic helpcount array 
    helpcount = (int*) calloc (tmpNum, sizeof(int));

	for(int i=0; i<tmpNum; i++)
	{
		helpcount[i] = rand() % HELPS_MAX + 1;
	}

	if (tmpNum > STUDENT_COUNT_MIN  && tmpNum < STUDENT_COUNT_MAX)
	{
	////////////////////////////////////////////////////////////////
		
		//initialize mutex variables
		if (pthread_mutex_init(&Hchair, NULL) != 0)
    	{
        	printf("\n mutex init failed\n");
            return 1;
        }
		
	
		//professor thread
		if (pthread_create(&(professorthd), &attr, &professor, NULL) != 0)
		{
  			printf("Can't create professor thread \n");
		}
			//printf("\033[0;92mProfessor is sleeping\033[0m\n");					
	////////////////////////////////////////////////////////////////
		//dynamic student threads
		
		int *index = NULL; 
		int i; 
		studentsthd = malloc(sizeof(pthread_t)*tmpNum); 
   		//check for errors 			
		if (studentsthd == NULL)
   		{
     		printf("out of memory\n");
      		exit(EXIT_FAILURE);
   		}	

		for(i=0; i<tmpNum;i++)
        {	
			index = malloc(sizeof(int)*tmpNum); 
			*index = i;
		
			
           	if (pthread_create(&(studentsthd[i]), &attr, &students, index) != 0)
			{
  				printf("Can't create students thread \n");
			}
			
			printf("\033[0;93mStudent %d doing assignment.\033[0m\n", i+1);  
	  		
	    }
		
		//destroy attribute
		pthread_attr_destroy(&attr);
	
		//join student threads
		for(i = 0; i<tmpNum; i++)
        {
           	pthread_join(studentsthd[i], &status);
        }


	////////////////////////////////////////////////////////////////
/*		
		//destroy attribute
		pthread_attr_destroy(&attr);
		//join professor thread	
       	pthread_join(professorthd, &status);
*/		
		pthread_mutex_destroy(&Hchair);

	////////////////////////////////////////////////////////////////
	
		//done helping all students
		printf("\n");
		printf("\033[0;94mAll students assisted, professor is leaving.\033[0m\n"); 

	}
	
	else
	{
		printf("Invalid number of students\n");
		goto Done;
	}
		//deallocate memory
		free(helpcount);
		helpcount = NULL; 

		free(studentsthd);
		studentsthd = NULL; 
				
		 		
	
	Done:;
	//pthread_exit(NULL);
	return 0;
}

//professor
void *professor(void *arg)
{
	while (true)
	{
		//professor waits for student		
		sem_wait(&students_sem);
		//printf("\033[0;92mstudent left the hallway: %d \033[0m\n", waiting);
	
		if (waiting != 0 )
		{
			//only 1 student in the office 
			pthread_mutex_lock(&Hchair);
			waiting--;
			pthread_mutex_unlock(&Hchair);
			
			//professor is sleeping, awaken by student
			if(awake == 0)
			{
				awake = 1;
				usleep (rand() % 15000);
				printf("\033[0;92mProfessor has been awakened by a student\033[0m\n");			
				goto helpnow;
			}
			
			//professor is already awake, keep helping students
			else
			{
				helpnow:
				//student vacates hallway and enter office 
				printf("\033[0;92mStudent frees chair and enters professor's office. Remaining chairs: %d \033[0m\n", (CHAIR_COUNT-waiting));	
				
				//signal the next student
				sem_post(&professor_sem);
			
				//professor is helping 
				printf("\033[0;92mProfessor is helping a student student\033[0m\n");
				usleep (rand() % 1500000);
			}
		}

		else
		{
		awake = 0;
		//exit thread
		break; 
		}
	}
	
		
	pthread_exit((void *) 0);
}


//students
void *students(void *arg)
{
	int id = *(int*)arg;
	int count = helpcount[id];

	
	
	while (true)
	{
		getchair:
		if(count != 0)
		{
			//printf("Student[%d] has count: %d\n", id+1, count);
			//getchair:
			//student works in random time 
			usleep(rand() % 2000000);
	
			//student decides to ask for help 
			printf("\033[0;31mStudent %d needs help from the professor.\033[0m\n", id+1); 
							
			//student waits and get a chair
        	if (waiting < CHAIR_COUNT)
			{
				//maximum of 3 students in hallway
				pthread_mutex_lock(&Hchair);
				waiting++;
				pthread_mutex_unlock(&Hchair);

			
				//wake the professor 
				sem_post(&students_sem);
			
				//professor is helping the student 
				sem_wait(&professor_sem);
				printf("\033[0;31mStudent %d is getting help from the professor. \033[0m\n", id+1); 
				
				//decrement help count
				goto updatecount;
			}
					
			//student leaves
			else
			{
				printf("\033[0;31mChairs occupied, student %d will return later. \033[0m\n", id+1); 
				printf("\033[0;93mStudent %d doing assignment.\033[0m\n", id+1);  
				goto getchair; 
			}

			//decrement help count
			updatecount:
			count--;
			
		}

		else
		{
			//exit thread
			break;
		}


	}	
	
	free(arg);

	pthread_exit((void *) 0);

}
