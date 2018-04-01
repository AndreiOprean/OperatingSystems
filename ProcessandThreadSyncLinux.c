#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"
#include <sys/sem.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/mman.h>

//pt 2.3
pthread_mutex_t lock2;
pthread_cond_t cond2;
pthread_cond_t cond21;
int c21  = 0; //variabila pentru verificare ordine sfarsit
int c22 = 0; //var pt verif ordine inceput

//pt 2.4

pthread_mutex_t lock9;
int nrThread = 0;
pthread_cond_t cond9;
pthread_cond_t cond91;
pthread_cond_t cond92;
int c91 = 0;
int total = 0;

// pt 2.5
pthread_mutex_t lock6;
pthread_mutexattr_t attrmutex;
pthread_cond_t *cond6;
pthread_cond_t *cond61;
pthread_condattr_t attrcond;
pthread_condattr_t attrcond1;
int *c6;
int *c61;

void* func2(void* arg){

	int th_id = *((int*) arg);


	if (pthread_mutex_lock(&lock2) != 0){
		return 0;
	}


	if ((th_id == 2) && (c22 == 0)){
		if (pthread_cond_wait(&cond21, &lock2) != 0)
			return 0;

	}

	//pt 2.5
	if (th_id == 4 && *c6 == 0){
		pthread_cond_wait(cond6, &lock2);
	}

	info(BEGIN,2,th_id);

	if (th_id == 1){
		if (pthread_cond_signal(&cond21) != 0)
			return 0;
		c22 = 1;

	}

	if ((th_id == 1) && (c21 == 0)){
		if (pthread_cond_wait(&cond2, &lock2) != 0){
			return 0;
		}
	}


	info(END,2,th_id);

	if (th_id == 4){

		pthread_cond_signal(cond61);
		*c61 = 1;
	}

	if (th_id == 2){
		if (pthread_cond_signal(&cond2) != 0)
			return 0;
		c21 = 1;
	}


	if (pthread_mutex_unlock(&lock2) != 0){
		return 0;
	}

	return NULL;
}


void* func9(void* arg){
	int id = *((int*)arg);

	if (pthread_mutex_lock(&lock9) != 0){
		return 0;
	}

	if (id != 14 && c91 == 0){

		pthread_cond_wait(&cond92, &lock9);
	}

	if (id == 14){
		pthread_cond_signal(&cond92);

		c91 = 1;
	}

	while (nrThread >= 4){
		if (pthread_cond_wait(&cond9, &lock9) != 0){
			return 0;
		}
	}


	nrThread++;
	total++;

	if (nrThread == 4){
		pthread_cond_signal(&cond91);
	}



	pthread_mutex_unlock(&lock9);


	info(BEGIN,9,id);


	pthread_mutex_lock(&lock9);

	if (id == 14){	
		if (total < 45)
			while (nrThread != 4){
				pthread_cond_wait(&cond91,&lock9);
			}

	}

	info(END,9,id);

	nrThread--;

	if (pthread_cond_signal(&cond9) != 0){
		return 0;
	}
	pthread_cond_signal(&cond92);

	if (pthread_mutex_unlock(&lock9) != 0){
		return 0;
	}


	return NULL;
}


void* func6(void *arg){

	int id = *((int*)arg);
	pthread_mutex_lock(&lock6);

	if (id == 2 && *c61 == 0){
		pthread_cond_wait(cond61,&lock6);
	}


	pthread_mutex_unlock(&lock6);

	info(BEGIN,6,id);


	pthread_mutex_lock(&lock6);

	info(END,6,id);

	if (id == 4){

		pthread_cond_signal(cond6);
		*c6 = 1;
	}

	pthread_mutex_unlock(&lock6);


	return NULL;
}



int main(int argc, char *argv[]){




	pid_t p2,p3,p4,p5,p6,p7,p8,p9;
	int status;
	cond6 = (pthread_cond_t*)mmap(NULL, sizeof(pthread_cond_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	cond61 = (pthread_cond_t*)mmap(NULL, sizeof(pthread_cond_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	c6 = mmap(NULL, sizeof *c6, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	c61 = mmap(NULL, sizeof *c61, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	pthread_condattr_init(&attrcond);
	pthread_condattr_setpshared(&attrcond, PTHREAD_PROCESS_SHARED);
	pthread_cond_init(cond6, &attrcond);

	pthread_condattr_init(&attrcond1);
	pthread_condattr_setpshared(&attrcond1, PTHREAD_PROCESS_SHARED);
	pthread_cond_init(cond61, &attrcond1);

	init(); 
	info(BEGIN, 1, 0);

	if ((p2 = fork()) == 0){
		info(BEGIN,2,0);

		int i;
		pthread_t t2[4];
		int th_args[4];

		//initializare mutex si var de conditii
		if (pthread_mutex_init(&lock2, NULL) != 0)
			return 0;

		if (pthread_cond_init(&cond2,NULL) != 0)
			return 0;

		if (pthread_cond_init(&cond21,NULL) != 0)
			return 0;

		//creare threaduri
		for (i = 1; i <= 4; i++){
			th_args[i-1] = i;

			if (pthread_create(&t2[i-1], NULL, func2, &th_args[i-1]) != 0){
				return 0;
			}
		}

		for (i = 0; i < 4; i++){
			pthread_join(t2[i],NULL);	//asteptam sa se termine toate
		}	

		//terminare mutex si var de conditii
		if(pthread_mutex_destroy(&lock2) != 0)
			return 0;

		if (pthread_cond_destroy(&cond2) != 0){
			return 0;
		}

		if (pthread_cond_destroy(&cond21) != 0){
			return 0;
		}

		info(END,2,0);
		return 0;
	}

	if ((p3 = fork()) == 0){
		info(BEGIN,3,0);
		if ((p5 = fork()) == 0){
			info(BEGIN,5,0);


			info(END,5,0);
			return 0;

		}
		if ((p6 = fork()) == 0){
			info(BEGIN,6,0);
			*c6 = 0;
			*c61 = 0;
			int k;
			pthread_t t6[4];
			int th_args6[4];
			pthread_mutexattr_init(&attrmutex);
			pthread_mutexattr_setpshared(&attrmutex, PTHREAD_PROCESS_SHARED);


			if (pthread_mutex_init(&lock6, &attrmutex) != 0)
				return 0;

			for (k = 1; k <= 6; k++){
				th_args6[k-1] = k;
				if(pthread_create(&t6[k-1], NULL, func6, &th_args6[k-1]) != 0)
					return 0;
			}

			for (k = 0; k < 6; k++){
				pthread_join(t6[k], NULL);
			}

			if(pthread_mutex_destroy(&lock6) != 0)
				return 0;

			pthread_mutexattr_destroy(&attrmutex);
			pthread_cond_destroy(cond6);
			pthread_condattr_destroy(&attrcond);
			pthread_cond_destroy(cond61);
			pthread_condattr_destroy(&attrcond1);
			munmap(c6, sizeof *c6);
			munmap(cond6, sizeof *cond6);
			munmap(cond61, sizeof *cond6);

			info(END,6,0);
			return 0;

		}
		waitpid(p5,&status,WUNTRACED | WCONTINUED);
		waitpid(p6,&status,WUNTRACED | WCONTINUED);


		info(END,3,0);
		return 0;
	}
	if ((p4 = fork()) == 0){
		info(BEGIN,4,0);

		if ((p8 = fork()) == 0){

			info(BEGIN,8,0);


			info(END,8,0);
			return 0;

		}

		waitpid(p8,&status,WUNTRACED | WCONTINUED);
		info(END,4,0);	
		return 0;

	}
	if ((p7 = fork()) == 0){
		info(BEGIN,7,0);


		info(END,7,0);
		return 0;

	}

	if ((p9 = fork()) == 0){
		info(BEGIN,9,0);

		pthread_t t9[45];
		int j;
		int th_args9[45];

		if (pthread_mutex_init(&lock9, NULL) != 0){
			return 0;
		}

		if (pthread_cond_init(&cond9,NULL) != 0)
			return 0;

		if (pthread_cond_init(&cond91,NULL) != 0)
			return 0;

		if (pthread_cond_init(&cond92,NULL) != 0)
			return 0;

		for (j = 1; j <= 45; j++){
			th_args9[j-1] = j;

			if (pthread_create(&t9[j-1], NULL, func9, &th_args9[j-1]) != 0){
				return 0;
			}
		}

		for (j = 0; j < 45; j++){
			pthread_join(t9[j],NULL);
		}	

		if(pthread_mutex_destroy(&lock9) != 0){	
			return 0;
		}

		if (pthread_cond_destroy(&cond9) != 0){
			return 0;
		}

		if (pthread_cond_destroy(&cond91) != 0){
			return 0;
		}

		if (pthread_cond_destroy(&cond92) != 0){
			return 0;
		}

		info(END,9,0);

		return 0;
	}


	waitpid(p2,&status,WUNTRACED | WCONTINUED);
	waitpid(p3,&status,WUNTRACED | WCONTINUED);
	waitpid(p4,&status,WUNTRACED | WCONTINUED);
	waitpid(p7,&status,WUNTRACED | WCONTINUED);
	waitpid(p9,&status,WUNTRACED | WCONTINUED);



	info (END, 1, 0);

	return 0;
}
