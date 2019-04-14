#ifndef ALGORITHM_SORT_H
#define ALGORITHM_SORT_H

#include "algorithm.h"


void bubble_sort(VALUE arr[], long size, COMPARE compare);
void select_sort(VALUE arr[], long size, COMPARE compare);
void insert_sort(VALUE arr[], long size, COMPARE compare);
void quick_sort(VALUE arr[], long size, COMPARE compare);
void merge_sort(VALUE arr[], long size, COMPARE compare);

void sort(VALUE arr[], long size, COMPARE compare);

#endif //ALGORITHM_SORT_H