#include <iostream>
#include <list>
#include <algorithm>

#include <omp.h>

template<class iterator_type>
iterator_type get_middle(iterator_type begin, iterator_type end) {
	iterator_type mid;
	for (iterator_type it = mid = begin; it != end; ++it) {
		if (++it == end)
			break;
		++mid;
	}
	return mid;
}

template<class iterator_type>
void sort_it(iterator_type begin, iterator_type end) {
	if (begin == end || std::next(begin) == end)
		return;
	iterator_type mid = get_middle(begin, end);

	#pragma omp parallel num_threads(2)
	{
		if (omp_get_thread_num() == 0)
			sort_it(begin, mid);
		else
			sort_it(mid, end);
	}

	std::inplace_merge(begin, mid, end);
}

template<class iterator_type>
void parallel_merge_sort(iterator_type begin, iterator_type end) {
	omp_set_nested(1);
	sort_it(begin, end);
}

int main() {
	std::list<int> list = { 0, 5, 2, 4, 1, 0, 5, 2, 2323, -2, 2, 0, 1, 1 };
	parallel_merge_sort(list.begin(), list.end());
	for (const auto &p: list)
		std::cout << p << " ";
	std::cout << std::endl;
	return 0;
}
