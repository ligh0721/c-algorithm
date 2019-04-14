#ifndef ALGORITHM_SORT_H
#define ALGORITHM_SORT_H

#include "algorithm.h"


void bubble_sort(VALUE arr[], size_t size, compare_function compare);
void select_sort(VALUE arr[], size_t size, compare_function compare);
void insert_sort(VALUE arr[], size_t size, compare_function compare);
void quick_sort(VALUE arr[], size_t size, compare_function compare);
void merge_sort(VALUE arr[], size_t size, compare_function compare);

void sort(VALUE arr[], size_t size, compare_function compare);

#endif //ALGORITHM_SORT_H