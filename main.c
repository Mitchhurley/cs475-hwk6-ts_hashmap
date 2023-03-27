
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "ts_hashmap.h"

ts_hashmap_t *newMap;
int num_threads;
int hash_cap;
pthread_mutex_t lock;

void* threadTester(void *arg) {
  int id = *(int*)arg;
  srand(id); // Seed the random number generator with the thread ID so consistent output
  
  for (int i = 0; i < 1000; i++) {
    int key = rand() % 10000;
    int val = rand() % 10000;
    
    // Perform a random operation with equal probability
    int op = rand() % 3;
    if (op == 0) {
      put(newMap, key, val);
    } else if (op == 1) {
      get(newMap, key);
    } else {
      del(newMap, key);
    }
  }
  return NULL;
}

int main(int argc, char *argv[]) {
  //case of improper input

  pthread_mutex_init(&lock, NULL);

  if (argc != 3) {
    fprintf(stderr, "Usage: %s <num threads> <hashmap capacity>\n", argv[0]);
    return 1;
  }
  //get num threads and capacity from command line
  num_threads = atoi(argv[1]);
  hash_cap = atoi(argv[2]);

  if (num_threads <= 0) {
    fprintf(stderr, "Invalid number of threads: %s\n", argv[1]);
    return 1;
  }
  
  newMap = initmap(hash_cap);
  pthread_t threads[num_threads];
  int thread_ids[num_threads];
  
  // Create the threads
  for (int i = 0; i < num_threads; i++) {
    thread_ids[i] = i;
    if (pthread_create(&threads[i], NULL, threadTester, &thread_ids[i]) != 0) {
      fprintf(stderr, "Error creating thread %d\n", i);
      return 1;
    }
  }
  
  // Wait for the threads to complete
  for (int i = 0; i < num_threads; i++) {
    if (pthread_join(threads[i], NULL) != 0) {
      fprintf(stderr, "Error joining thread %d\n", i);
      return 1;
    }
  }
  
  // Print the contents of the map
  printmap(newMap);
  for (int i = 0; i < newMap->capacity; i++){
    ts_entry_t *curr_ent = newMap->table[i];
    while(curr_ent){
      ts_entry_t *tmp = curr_ent->next;
      free(curr_ent);
      curr_ent = tmp;
    }
    free(newMap->table[i]);
  }
  free(newMap->table);
  free(newMap);

  //destroy the lock
  pthread_mutex_destroy(&lock);
  
  return 0;
}
/*

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("Usage: %s <num threads> <hashmap capacity>\n", argv[0]);
		return 1;
	}
  	srand(time(NULL));
	int num_threads = atoi(argv[1]);
	int capacity = (unsigned int) atoi(argv[2]);

	// TODO: Write your test


	return 0;
}
*/
