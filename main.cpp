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


void quicksort(std::shared_ptr<std::promise<void>> spPromiseParent, int *array, int left, int right) {
	if(left >= right) {
		if (spPromiseParent != nullptr) {
			spPromiseParent->set_value();
		}
		return;
	}

	int left_bound = left;
	int right_bound = right;

	int middle = array[(left_bound + right_bound) / 2];

	do {
		while(array[left_bound] < middle) {
			left_bound++;
		}
		while(array[right_bound] > middle) {
			right_bound--;
		}

		//Меняем элементы местами
		if (left_bound <= right_bound) {
			std::swap(array[left_bound], array[right_bound]);
			left_bound++;
			right_bound--;
		}
	} while (left_bound <= right_bound);

	if (spPromiseParent != nullptr) {
		spPromiseParent->set_value();
	}

	if(make_thread && (right_bound - left > 10000))
	{
		// std::shared_ptr<std::promise<void>> spPromise(new std::promise<void>);
		// std::future<void> f = spPromise->get_future();

		if (spPromiseParent != nullptr) {
			pool.pushRequest(quicksort, nullptr, array, left, right_bound);
		} else {
			quicksort(nullptr, array, left, right_bound);	
		}
		quicksort(nullptr, array, left_bound, right);
	} else {
		// запускаем обе части синхронно
		quicksort(nullptr, array, left, right_bound);
		quicksort(nullptr, array, left_bound, right);
	}
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
	int arr_size = 10000000;
	int* array = new int[arr_size];

	for (long i = 0; i < arr_size; i++) {
		array[i] = rand() % 90 + 10;
	}

    auto begin = system_clock::now();

	std::shared_ptr<std::promise<void>> spPromise(new std::promise<void>);
	std::future<void> f = spPromise->get_future();

	quicksort(spPromise, array, 0, arr_size - 1);

	f.wait();

    auto end = system_clock::now();

	checkSort(array, arr_size);

    std::cout << "The time: " << duration_cast<milliseconds>(end - begin).count() << " milliseconds" << std::endl << std::endl;

	delete[] array;

	return 0;
}