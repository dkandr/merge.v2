#include <iostream>
#include <future>
#include <memory>
#include <chrono>
#include <random>
#include "optimized_thread.h"

using namespace std;
using namespace std::chrono;

bool make_thread = true;
// bool make_thread = false;

RequestHandler pool;
	
void merge(std::shared_ptr<std::promise<void>> spPromise, int* arr, int l, int m, int r) {
	int nl = m - l + 1;
	int nr = r - m;

	int* left = new int[nl];
	int* right = new int[nr];

	for (int i = 0; i < nl; i++)
		left[i] = arr[l + i];
	for (int j = 0; j < nr; j++)
		right[j] = arr[m + 1 + j];

	long i = 0, j = 0;
	long k = l;

	while (i < nl && j < nr) {
		if (left[i] <= right[j]) {
			arr[k] = left[i];
			i++;
		}
		else {
			arr[k] = right[j];
			j++;
		}
		k++;
	}
	while (i < nl) 	{
		arr[k++] = left[i++];
	}
	while (j < nr) 	{
		arr[k++] = right[j++];
	}

	spPromise->set_value();

	delete[] left;
	delete[] right;
}

void mergeSort(int* arr, int l, int r) {
	std::shared_ptr<std::promise<void>> spPromise(new std::promise<void>);
	std::future<void> f = spPromise->get_future();

	if (l >= r) {
		// spPromise->set_value();
		return;
	}

	long m = (l + r - 1) / 2;

	mergeSort(arr, l, m);
	mergeSort(arr, m + 1, r);
	
	if (make_thread && (m - l > 10000)) {
		pool.pushRequest(merge, spPromise, arr, l, m, r);
	} else {
		merge(spPromise, arr, l, m, r);
	}
	
	f.wait();
}

void showArray(int* array, int size) {
	for (int i = 0; i < size; i++) {
		std::cout << array[i] << " ";
	}
	std::cout << std::endl;
}

void checkSort(int* array, int size) {
	bool sort = true;
	for (int i = 0; i < size - 1; i++) {
		if (array[i] > array[i+1]) {
			sort = false;
			break;
		}
	}
	std::cout << "Array sorted : " << (sort ? "true" : "false") << std::endl;
}

int main() {
	srand(time(NULL));
	int arr_size = 1000000;
	int* array = new int[arr_size];

	for (long i = 0; i < arr_size; i++) {
		array[i] = rand() % 90 + 10;
	}

    auto begin = system_clock::now();

	mergeSort(array, 0, arr_size - 1);

    auto end = system_clock::now();

	checkSort(array, arr_size);

    std::cout << "The time: " << duration_cast<milliseconds>(end - begin).count() << " milliseconds" << std::endl << std::endl;

	delete[] array;

	return 0;
}