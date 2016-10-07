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

#pragma once

typedef void* net_mutex_t;

#define NET_MUTEX_INVALID ((net_mutex_t)0)

net_mutex_t net_mutex_new();

void net_mutex_lock(net_mutex_t m);


/*
 * Returns 0 on success, non-0 otherwise.
 */
int net_mutex_trylock(net_mutex_t m);

void net_mutex_unlock(net_mutex_t m);

