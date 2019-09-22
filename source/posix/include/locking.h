#ifndef LOCKING_H

    #define LOCKING_H_

	struct flock create_lock();
	int reset_lock(struct flock);
	int lock_fd(int, struct flock, long long, long long);
	int unlock_fd(int, struct flock);

#endif