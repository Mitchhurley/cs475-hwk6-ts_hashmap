#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ts_hashmap.h"



/**
 * Creates a new thread-safe hashmap. 
 *
 * @param capacity initial capacity of the hashmap.
 * @return a pointer to a new thread-safe hashmap.
 */
ts_hashmap_t *initmap(int capacity) {
  ts_hashmap_t *map = (ts_hashmap_t*) malloc(sizeof(ts_hashmap_t*));
  map->size = 0;
  map->capacity = capacity;
  map->table = (ts_entry_t**) malloc(sizeof(ts_entry_t*) * capacity);
  for (int i = 0; i < capacity; i++){
    map->table[i] = (ts_entry_t*) malloc(sizeof(ts_entry_t));
  }
  return map;
}

/**
 * Obtains the value associated with the given key.
 * @param map a pointer to the map
 * @param key a key to search
 * @return the value associated with the given key, or INT_MAX if key not found
 */
int get(ts_hashmap_t *map, int key) {
  //get the right index of the hashtable where the node should be
  pthread_mutex_lock(&lock);
  int hash = key % map->capacity;
  ts_entry_t *entry = map->table[hash];
  while (entry != NULL) {
    if (entry->key == key){
      pthread_mutex_unlock(&lock);
      return entry->value;
    }
    entry = entry->next;
  }
  pthread_mutex_unlock(&lock);
  //Only happens if the value isnt found at that index
  return INT_MAX;
}


/**
 * Associates a value associated with a given key.
 * @param map a pointer to the map
 * @param key a key
 * @param value a value
 * @return old associated value, or INT_MAX if the key was new
 */
int put(ts_hashmap_t *map, int key, int value) {
  pthread_mutex_lock(&lock);
  // Calculate the hash value for the given key
  int hash = key % map->capacity;
  // Get the linked list entry for the hash code
  ts_entry_t *currNode = map->table[hash];
  
  // Search for the key in the linked list
  while (currNode != NULL) {
    if (currNode->key == key) {
      // Key already exists, update the value
      int old_value = currNode->value;
      currNode->value = value;
      pthread_mutex_unlock(&lock);
      return old_value;
    }
    currNode = currNode->next;
  }
  
  // Key does not exist, add it to the linked list if size allows
  if (lf(map) < 1.0){
    ts_entry_t *newEntry = (ts_entry_t*) malloc(sizeof(ts_entry_t));
    newEntry->key = key;
    newEntry->value = value;
    newEntry->next = map->table[hash];
    map->table[hash] = newEntry;
    map->size++;
    pthread_mutex_unlock(&lock);
    return INT_MAX;
  }
  else{
    pthread_mutex_unlock(&lock);
    //TODO handle issue of full list
    return 1;
  }
}

/**
 * Removes an entry in the map
 * @param map a pointer to the map
 * @param key a key to search
 * @return the value associated with the given key, or INT_MAX if key not found
 */
int del(ts_hashmap_t *map, int key) {
  int hash = key % map->capacity;
  pthread_mutex_lock(&lock);
  ts_entry_t *entry = map->table[hash];
  ts_entry_t *prev = NULL;
  while (entry != NULL) {
    if (entry->key == key) {
      //hold the value so we can free the value
      int value = entry->value;
      //case of first value being the one to delete
      if (prev == NULL){
        map->table[hash] = entry->next;
      }
      else {
        prev->next = entry->next;
      }
      free(entry);
      map->size--;
      pthread_mutex_unlock(&lock);
      return value;
    }
    prev = entry;
    entry = entry->next;
  }
  //case of "weve gone through list and key not found"
  pthread_mutex_unlock(&lock);
  return INT_MAX;
  }

/**
 * @return the load factor of the given map
 */
double lf(ts_hashmap_t *map) {
  return (double) map->size / map->capacity;
}

/**
 * Prints the contents of the map
 */
void printmap(ts_hashmap_t *map) {
  for (int i = 0; i < map->capacity; i++) {
    printf("[%d] -> ", i);
    ts_entry_t *entry = map->table[i];
    while (entry != NULL) {
      printf("(%d,%d)", entry->key, entry->value);
      if (entry->next != NULL)
        printf(" -> ");
      entry = entry->next;
    }
    printf("\n");
  }
}