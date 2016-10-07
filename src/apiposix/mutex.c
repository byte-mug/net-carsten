/*
 *   Copyright 2016 Simon Schmidt
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

#include <netstd/mutex.h>
#include <pthread.h>
#include <stdlib.h>


net_mutex_t net_mutex_new(){
	pthread_mutex_t* mtx = malloc(sizeof(pthread_mutex_t));
	if(!mtx) return NET_MUTEX_INVALID;
	if(pthread_mutex_init(mtx,0)) return mtx;
	free(mtx);
	return NET_MUTEX_INVALID;
}

void net_mutex_lock(net_mutex_t m){
	pthread_mutex_lock((pthread_mutex_t*)m);
}

int net_mutex_trylock(net_mutex_t m){
	return pthread_mutex_trylock((pthread_mutex_t*)m);
}

void net_mutex_unlock(net_mutex_t m){
	pthread_mutex_unlock((pthread_mutex_t*)m);
}

