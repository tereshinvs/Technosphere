#include "Semaphore.hpp"

#include <vector>
#include <string>

int Semaphore::update(std::size_t start, std::size_t m, short value, short flag) const {
	std::vector<sembuf> sb(m);
	for (std::size_t i = 0; i < m; i++) {
		sb[i].sem_num = start + i;//reinterpret_cast<unsigned short>(start + i);
		sb[i].sem_op = value;
		sb[i].sem_flg = flag;
	}
	return semop(semid, sb.data(), m);
}

Semaphore::Semaphore(std::size_t _n, unsigned short _max_value,
	const std::string &file_name, int proj_id):
	n(_n), max_value(_max_value),
	semkey(ftok(file_name.data(), proj_id)),
	semid(semget(semkey, n, IPC_CREAT | IPC_EXCL | 0666)) {
	if (semid >= 0 && full_release(0, n) == -1) {
		int e = errno;
		semctl(semid, n, IPC_RMID);
		errno = e;
		semid = -1;			
	}
}

Semaphore::~Semaphore() {
	if (semid > 0)
		semctl(semid, n, IPC_RMID);	
}

/*int Semaphore::lock(unsigned short start, unsigned short m, short value) const {
	return update(start, m, -value, SEM_UNDO);
}

int Semaphore::lock_nowait(unsigned short start, unsigned short m, short value) const {
	return update(start, m, -value, SEM_UNDO | IPC_NOWAIT);
}

int Semaphore::release(unsigned short start, unsigned short m, short value) const {
	return update(start, m, value, SEM_UNDO);
}*/

int Semaphore::lock(std::size_t start, std::size_t m) const {
	return update(start, m, -1, SEM_UNDO);	
}

int Semaphore::full_lock(std::size_t start, std::size_t m) const {
	return update(start, m, -max_value, SEM_UNDO);
}

int Semaphore::lock_nowait(std::size_t start, std::size_t m) const {
	return update(start, m, -1, SEM_UNDO | IPC_NOWAIT);
}

int Semaphore::full_release(std::size_t start, std::size_t m) const {
	return update(start, m, max_value, SEM_UNDO);
}

int Semaphore::release(std::size_t start, std::size_t m) const {
	return update(start, m, 1, SEM_UNDO);
}
