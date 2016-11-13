#include "allocator.h"

#include <cstdint>
#include <cstring>
#include <algorithm>

Allocator::Allocator(void *_base, size_t _size):
    base(reinterpret_cast<char*>(_base)),
	size(_size) {
    if (size < sizeof(size_t))
    	throw AllocError(AllocErrorType::NoMemory, "No memory for initialization");
    GeneralMemoryInfo *general_info = get_general_memory_info();
    general_info->allocated = sizeof(GeneralMemoryInfo);
    general_info->data_end = base + sizeof(GeneralMemoryInfo);
    general_info->begin_ref = base + size;
}

GeneralMemoryInfo *Allocator::get_general_memory_info() const {
	return reinterpret_cast<GeneralMemoryInfo*>(base);
}

MemoryInfo *Allocator::get_next(MemoryInfo *from) const {
	MemoryInfo *cur = from->next;
	char *begin_ref = get_general_memory_info()->begin_ref;
	while (reinterpret_cast<char*>(cur) < begin_ref && cur->size == 0)
		cur = cur->next;
	return reinterpret_cast<MemoryInfo*>(std::min(reinterpret_cast<size_t>(cur),
		reinterpret_cast<size_t>(begin_ref)));
}

MemoryInfo **Allocator::get_available_ref_space() const {
	MemoryInfo **begin_ref = reinterpret_cast<MemoryInfo**>(get_general_memory_info()->begin_ref);
	for (MemoryInfo **it = reinterpret_cast<MemoryInfo**>(base + size) - 1; it >= begin_ref; --it)
		if (*it == nullptr)
			return it;
	return begin_ref - 1;
}

Pointer Allocator::alloc(size_t N) {
	GeneralMemoryInfo *general_info = get_general_memory_info();

	size_t freemem = reinterpret_cast<size_t>(general_info->begin_ref)
		- reinterpret_cast<size_t>(base) - general_info->allocated;
	if (freemem < N + sizeof(MemoryInfo))
		throw AllocError(AllocErrorType::NoMemory, "No memory");

	MemoryInfo **ref_space = get_available_ref_space();
	if (general_info->data_end >= reinterpret_cast<char*>(ref_space))
		throw AllocError(AllocErrorType::NoMemory, "No available space for reference");

	if (general_info->data_end == base + sizeof(GeneralMemoryInfo)) { // First allocation
		MemoryInfo *to = reinterpret_cast<MemoryInfo*>(general_info->data_end);
		to->next = reinterpret_cast<MemoryInfo*>(base + size);
		to->reference = ref_space;
		to->size = N;

		general_info->allocated += N + sizeof(MemoryInfo);
		general_info->data_end += N + sizeof(MemoryInfo);
		general_info->begin_ref = std::min(general_info->begin_ref,
			reinterpret_cast<char*>(ref_space));

		*(to->reference) = to;

		return Pointer(ref_space);
	}

	MemoryInfo *cur = reinterpret_cast<MemoryInfo*>(base + sizeof(GeneralMemoryInfo));

	while (reinterpret_cast<char*>(cur) < general_info->begin_ref) {
		MemoryInfo *next = get_next(cur);
		size_t free_dist = std::min(reinterpret_cast<size_t>(next),
			reinterpret_cast<size_t>(ref_space))
			- reinterpret_cast<size_t>(cur)	- cur->size - sizeof(MemoryInfo);

		if (free_dist >= N + sizeof(MemoryInfo)) {
			MemoryInfo *to = reinterpret_cast<MemoryInfo*>(reinterpret_cast<char*>(cur)
				+ sizeof(MemoryInfo) + cur->size);
			cur->next = to;

			to->next = next;
			to->reference = ref_space;
			to->size = N;

			general_info->allocated += N + sizeof(MemoryInfo);
			general_info->data_end = reinterpret_cast<char*>(std::max(reinterpret_cast<size_t>(to)
				+ N + sizeof(MemoryInfo), reinterpret_cast<size_t>(general_info->data_end)));
			general_info->begin_ref = std::min(general_info->begin_ref,
				reinterpret_cast<char*>(ref_space));

			*ref_space = to;

			return Pointer(ref_space);
		}
		cur = next;
	}

	throw AllocError(AllocErrorType::NoMemory, "No memory");
}

void Allocator::realloc(Pointer &p, size_t N) {
	if (p.get() == nullptr) {
		p = alloc(N);
		return;
	}
	GeneralMemoryInfo *general_info = get_general_memory_info();
	MemoryInfo *cur = reinterpret_cast<MemoryInfo*>(p.get()) - 1;
	MemoryInfo *next = get_next(cur);
	size_t free_dist = std::min(reinterpret_cast<size_t>(next),
			reinterpret_cast<size_t>(general_info->begin_ref))
			- reinterpret_cast<size_t>(cur);
	if (free_dist >= N + sizeof(MemoryInfo)) {
		cur->size = N;
	} else {
		Pointer newp = alloc(N);
		MemoryInfo *newpinfo = reinterpret_cast<MemoryInfo*>(newp.get()) - 1;

		std::memcpy(newp.get(), p.get(), std::min(cur->size, N));

		MemoryInfo **old_ref = p.p;
		free(p);
		*old_ref = newpinfo;
		*(newpinfo->reference) = nullptr;
		newpinfo->reference = old_ref;
		newp.p = old_ref;
		p = newp;
	}
}

void Allocator::free(Pointer &p) {
	GeneralMemoryInfo *general_info = get_general_memory_info();
	MemoryInfo *from = reinterpret_cast<MemoryInfo*>(p.get());
	if (from == nullptr)
		throw AllocError(AllocErrorType::InvalidFree, "Invalid free");
	from--;
	general_info->allocated -= from->size + sizeof(MemoryInfo);
	from->size = 0;
	*(p.p) = nullptr;
	p.set(nullptr);
}

void Allocator::defrag() {
	GeneralMemoryInfo *general_info = get_general_memory_info();
	if (general_info->allocated == sizeof(GeneralMemoryInfo)) {
		general_info->data_end = base + sizeof(GeneralMemoryInfo);
		general_info->begin_ref = base + size;
		return;
	}

	general_info->data_end = base + sizeof(GeneralMemoryInfo);
	MemoryInfo *cur = reinterpret_cast<MemoryInfo*>(base + sizeof(GeneralMemoryInfo)), *prev;
	if (cur->size == 0)
		cur = get_next(cur);
	while (reinterpret_cast<char*>(cur) < general_info->begin_ref) {
		if (reinterpret_cast<char*>(cur) != general_info->data_end) {
			std::memmove(general_info->data_end, cur, cur->size + sizeof(MemoryInfo));
			cur = reinterpret_cast<MemoryInfo*>(general_info->data_end);
			*(cur->reference) = cur;
			prev->next = cur;
		}
		general_info->data_end = reinterpret_cast<char*>(cur) + sizeof(MemoryInfo) + cur->size;
		prev = cur;
		cur = get_next(cur);
	}
	while (general_info->begin_ref < base + size
		&& reinterpret_cast<MemoryInfo*>(*general_info->begin_ref) == nullptr)
		general_info->begin_ref += sizeof(MemoryInfo*);
}

std::string Allocator::dump() {
	return "";
}
