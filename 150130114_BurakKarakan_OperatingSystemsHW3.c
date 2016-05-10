/****************************************


BLG312E - Operating Systems HW #3
Burak Karakan
150130114


*****************************************

The program below models a hiring process for a company.
The three parts of the hiring components are each modeled as a process.
The "Registrar" is the main process.
The "Written Exam Committee" is another process with T threads -members-.
The "Interviewer" is another process.

*****************************************

The "questionTyping" method takes input arguments from the main thread and
executes the given instructions.

*****************************************

To compile the code, the '-lpthread' and '-lrt' arguments must be given along with the gcc,
in order to code to compile correctly. '-lpthread' is for pthreads, and '-lrt' is for sem_ and shm_ 
operations.

After compiling the code, it must be executed in the following format:

/program input_file number_of_WEC_members written_exam_duration interview_duration

input_file: The name of the input file.
number_of_WEC_members: Number of committee members.
written_exam_duration: Duration of the written exam.
interview_duration: Duration of the interview.

****************************************/

// The required libraries.
#include <unistd.h>     
#include <sys/types.h>  
#include <errno.h>      
#include <stdio.h>      
#include <stdlib.h>     
#include <pthread.h>    
#include <string.h>     
#include <semaphore.h>  
#include <sys/mman.h>
#include <fcntl.h>

// Useful definitions.
#define NUMBER_OF_APPLICANTS 3
#define TOTAL_SCORE_CALCULATION_DURATION 3
#define APPLICATION_TAKING_DURATION 3
#define SHM_NAME "/sharedMemoryForHW4"

// Thread prototype.
void questionTyping ( void *arguments );

// Global semaphore declarations.
sem_t mutex;
sem_t mutexForApproval;
sem_t canProceed; 

// Question struct definition.
typedef struct question{	
	char text[30];
	int approvals;
} Question;

// Member struct definition.
typedef struct WECMember{
	int memberId;
	int duration;
	char questionTopic[30];
} WECMember;

// Application struct definition.
typedef struct application{
	int exam_score;
	int interview_score;
	int total_score;
} ApplicationForm;

// Global file pointers.
char* fileName;			
FILE* fileOpen;			

// Global questions array, in order to be reachable from different threads.
Question *questions;

// Global self-explaining counter variables.
int totalNumberOfMembers;
int allPrepared = 0;
int finishedThreads = 0;

int main(int argc, char *argv[]){

    // Check the number of arguments, they must be more than 2.
    if (argc < 2){
      printf("Not enough variables.");
      return 0;
    }

    // Use the time for a more realistic randomization algorithm.
    srand(time(NULL));
    
    // Process declarations.
    pid_t wecProcess, interviewerProcess; 
    
    // Simple counter to use in loops.
    int counter = 0;

    // Get command line arguments.
    int numberOfMembers = atoi(argv[2]);
    int writtenExamDuration = atoi(argv[3]);
    int interviewDuration = atoi(argv[4]);

    // Assigning the local arguments to global variables.
    fileName = argv[1];
    totalNumberOfMembers = numberOfMembers;
	
	// Allocate the required memory for global usage.
	pthread_t members[numberOfMembers];
	questions = malloc(numberOfMembers * sizeof(Question));
	
	// temporary variable to hold the member as an object to be passed to the thread.
	WECMember memberToThread; 						
	// Return value of pthread_create.
	int check = 0;									
	// Basic initializations.
	memberToThread.duration = 1;					
	memberToThread.memberId = 2;					

	// The applicants array must be in shared memory as well, in order to different processes to access and modify.
    ApplicationForm *applicantList;
    // The file descriptor that returns from shared memory allocation.
	int sharedMemory;
	// Size of the shared memory.
	size_t size = NUMBER_OF_APPLICANTS*sizeof(ApplicationForm);
	// Get the shared memory.
	sharedMemory = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
	
	// Check for shared memory allocation errors.
	if(sharedMemory == -1){
		printf("Error while allocating shared memory. Program is terminating.\n");
		return;
	}
   	
   	// Expand it to the required size.
   	ftruncate(sharedMemory, size);
   	// Map the object to the 'applicantList'.
   	applicantList = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, sharedMemory, 0);
	
	// Check for mapping errors.
	if(applicantList == MAP_FAILED){
		printf("Memory mapping failed. Program is terminating.\n");
		return;
	}

	// The semaphore to use between processes must be in the shared memory. 
	sem_t *signalToRegistrar;
    // Initialize the semaphore for interprocess communication.
    signalToRegistrar = sem_open ("temporarySem", O_CREAT | O_EXCL, 0644, 0); 
    // Unlink the temporary semaphore immediately to prevent it existing even after the program faces with an error. 
    sem_unlink ("temporarySem");   

    // Check for semaphore errors. 
    if(signalToRegistrar == SEM_FAILED){
    	printf("Could not create th e semaphore. Program is terminating.\n");
    }

	// Initializing the semaphores.
	sem_init(&mutex, 0, 1);
	sem_init(&mutexForApproval, 0, 0);  
	sem_init(&canProceed, 1, 0); 

	// simulate the application taking process.
	printf("The Registrar started taking applications.\n");
	sleep(APPLICATION_TAKING_DURATION); 
	printf("The Registrar finished taking applications.\n");


	// Create a new process to model Written Exam Committee.
    wecProcess = fork();

    if(wecProcess == 0){
    	/*----------------------------------*/
    	/* This is the child -WEC- process. */
		/*----------------------------------*/
	    printf("The Written Exams Committee started preparing questions.\n");

	    // Open the file.
	    fileOpen = fopen(fileName, "r");
	    
	    // Check file open for errors.
	    if(fileOpen == NULL){
			printf("Error occurred while opening the file.\n");
			return 0;
	    }
	    
	    // Read from the file and create threads for every member that will do their job.
	    for(counter = 0; counter < numberOfMembers; counter++){ 																
			fscanf(fileOpen, "%d %s %d", &memberToThread.memberId, (memberToThread.questionTopic),  &memberToThread.duration);
			sleep(memberToThread.duration);				
			check = pthread_create (&members[counter], NULL, (void *) &questionTyping, (void *)&memberToThread);
			sleep(1);
	    }

	    // Wait a signal from the threads to end, before proceeding to the end and signaling the registrar the question preparing is finished.
	    sem_wait(&canProceed);

	    printf("The Written Exams Committee finished preparing questions.\n");

	    // Signal the registrar to continue.
	    sem_post(signalToRegistrar);

	    // Return the allocated resources, destroy the semaphore and exit.
	    sem_destroy(&canProceed);
	    munmap(applicantList, size);
	    fclose(fileOpen);		
	   	exit(0);
    }

    else{
    	/*-----------------------------------------*/
    	/* This is the parent -Registrar- process. */
		/*-----------------------------------------*/

		// Wait the signal from WEC that the questions are ready.
    	sem_wait(signalToRegistrar);

    	// Start the written exam.
	    printf("The Registrar started the written exam.\n");
	    sleep(writtenExamDuration);

	    // Announce the written exam grades of the applicants.
	    for(counter = 0; counter < NUMBER_OF_APPLICANTS; counter++){
	    	applicantList[counter].exam_score = rand() % 50; 
	    	printf("Written exam score of applicant %d is %d.\n", counter+1, applicantList[counter].exam_score);
	    }

	    //The exam is finished.
	    printf("The Registrar finished the written exam.\n");

	    // Create a new process to model the Interviewer.
	    interviewerProcess = fork();

	    if(interviewerProcess == 0){
			/*------------------------------------------*/
			/* This is the child -Interviewer- process. */
			/*------------------------------------------*/

	    	// Start the interviews.
	    	printf("The Interviewer started interviews.\n");

	    	// Do the interviews and announce the interview score.
	    	for(counter = 0; counter < NUMBER_OF_APPLICANTS; counter++){
	    		// The interview is 'interviewDuration' long.
	    		sleep(interviewDuration);
	    		// After the interview, write down the score to the application form.
	    		applicantList[counter].interview_score = rand() % 50;
	    		// Then announce it.
	    		printf("Interview score of applicant %d is %d.\n", counter + 1, applicantList[counter].interview_score);
	    	}

	    	printf("The Interviewer finished interviews.\n");

	    	// Signal the registrar to continue.
	    	sem_post(signalToRegistrar);

	    	// Return the allocated resources and exit.
	    	munmap(applicantList, size);
	    	exit(0);
	    }

	    else{
			/*-----------------------------------------*/
	    	/* This is the parent -Registrar- process. */
			/*-----------------------------------------*/

			// Wait the signal from the Interviewer that the interviews has ended.
	    	sem_wait(signalToRegistrar);

	    	// Start the total score calculation process.
	    	printf("The Registrar started calculating total scores.\n");
	    	sleep(TOTAL_SCORE_CALCULATION_DURATION);

	    	// Calculate and announce the total scores
	    	for(counter = 0; counter < NUMBER_OF_APPLICANTS; counter++){
	    		applicantList[counter].total_score = applicantList[counter].exam_score + applicantList[counter].interview_score;
	    		printf("Total score of applicant %d is %d.\n", counter + 1, applicantList[counter].total_score);
	    	}

	    	printf("The Registrar finished calculating total scores.\n");

	    	// Return the allocated resources.
	    	munmap(applicantList, size);
	    }
	}
    
    // Unlink and destroy the semaphores and end the parent process.
	sem_destroy(&mutex);
	sem_destroy(&mutexForApproval);
	sem_destroy(signalToRegistrar);

	shm_unlink(SHM_NAME);
	return(0);
}


void questionTyping (void *arguments)
{
	/*----------------------------------------------------*/
	/* This is the thread to demonstrate the WEC members. */
	/*----------------------------------------------------*/

	// Get the arguments into local variables.
    WECMember *args = (WECMember *)arguments; 			
    int id = args->memberId; 							
    int duration = args->duration; 						
    char *questionTopic = args->questionTopic;			
    int i = 0;

    // Wait for the signal and enter the critical section.
    sem_wait(&mutex); 									

		printf("WEC member %d: A question is prepared on %s\n", id, questionTopic);
		// Get the questions to the 'questions' array.
		strncpy(questions[id-1].text, questionTopic, 30);
		// Increment the counter that counts the number of prepared questions.
		allPrepared++;
		// If all the questions are prepared, start the question approving stage by signaling the semaphore.
		if(allPrepared == totalNumberOfMembers){
			sem_post(&mutexForApproval);
		}

	// Increment the semaphore for other threads to continue.
    sem_post(&mutex);  
    
    // Wait for the signal to proceed to the approving stage.
    sem_wait(&mutexForApproval);

    	// Start approving all the questions.
		for(i=0; i < totalNumberOfMembers; i++){
			// Questions that do not belong to the member can be approved by the member.
			if(i != id-1){										
				// Simulate the approving time and approve it by incrementing the 'approvals' counter.
				sleep(1);
				questions[i].approvals = questions[i].approvals +1;
				// And if the member is the last one to approve, announce the question as ready.
				if(questions[i].approvals == 2){
					printf("WEC member %d: Question %d is ready\n", id, i+1);
				}
			}
		}

	// Increment the semaphore for other members to approve.
    sem_post(&mutexForApproval);
    
    // Count the finished thread in order to signal the WEC process to proceed.
    finishedThreads++;
    // And if all of them had executed, than signal the WEC process to continue.
    if(finishedThreads == totalNumberOfMembers){
    	sem_post(&canProceed);
    }

    // Exit the thread.
    pthread_exit(0); 
}
