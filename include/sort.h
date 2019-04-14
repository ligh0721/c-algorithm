#ifndef ALGORITHM_SORT_H
#define ALGORITHM_SORT_H

#include "algorithm.h"


void bubble_sort(Object arr[], int size, compare_function compare);
void insert_sort(Object arr[], int size, compare_function compare);
void quick_sort(Object arr[], int size, compare_function compare);
void merge_sort(Object arr[], int size, compare_function compare);

void sort(Object arr[], int size, compare_function compare);

#endif //ALGORITHM_SORT_H