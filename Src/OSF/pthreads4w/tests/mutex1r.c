/*
 * mutex1r.c
 *
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads4w - POSIX Threads for Windows
 *      Copyright 1998 John E. Bossom
 *      Copyright 1999-2018, Pthreads4w contributors
 *
 *      Homepage: https://sourceforge.net/projects/pthreads4w/
 *
 *      The current list of contributors is contained
 *      in the file CONTRIBUTORS included with the source
 *      code distribution. The list can also be seen at the
 *      following World Wide Web location:
 *
 *      https://sourceforge.net/p/pthreads4w/wiki/Contributors/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * --------------------------------------------------------------------------
 *
 * As for mutex1.c but with type set to PTHREAD_MUTEX_RECURSIVE.
 *
 * Create a simple mutex object, lock it, unlock it, then destroy it.
 * This is the simplest test of the pthread mutex family that we can do.
 *
 * Depends on API functions:
 *	pthread_mutexattr_settype()
 *      pthread_mutex_init()
 *	pthread_mutex_destroy()
 */
#include "test.h"

pthread_mutex_t mutex = NULL;
pthread_mutexattr_t mxAttr;

int main()
{
	assert(pthread_mutexattr_init(&mxAttr) == 0);
	BEGIN_MUTEX_STALLED_ROBUST(mxAttr)
	assert(pthread_mutexattr_settype(&mxAttr, PTHREAD_MUTEX_RECURSIVE) == 0);
	assert(mutex == NULL);
	assert(pthread_mutex_init(&mutex, &mxAttr) == 0);
	assert(mutex != NULL);
	assert(pthread_mutex_lock(&mutex) == 0);
	assert(pthread_mutex_unlock(&mutex) == 0);
	assert(pthread_mutex_destroy(&mutex) == 0);
	assert(mutex == NULL);
	END_MUTEX_STALLED_ROBUST(mxAttr)
	return 0;
}
