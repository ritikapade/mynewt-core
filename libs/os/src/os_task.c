/**
 * Copyright (c) 2015 Stack Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "os/os.h"

#include <string.h>

uint8_t g_task_id;

static void
_clear_stack(os_stack_t *stack_bottom, int size) 
{
    int i; 

    for (i = 0; i < size; i++) {
        stack_bottom[i] = OS_STACK_PATTERN;
    }
}

static inline uint8_t 
os_task_next_id(void)
{
    uint8_t rc;
    os_sr_t sr;

    OS_ENTER_CRITICAL(sr);
    rc = g_task_id;
    g_task_id++;
    OS_EXIT_CRITICAL(sr);

    return (rc);
}

int 
os_task_init(struct os_task *t, char *name, os_task_func_t func, void *arg, 
        uint8_t prio, os_stack_t *stack_bottom, uint16_t stack_size)
{
    int rc; 

    memset(t, 0, sizeof(*t));

    t->t_func = func;
    t->t_arg = arg;
    
    t->t_taskid = os_task_next_id(); 
    t->t_prio = prio;

    t->t_state = OS_TASK_READY;
    t->t_name = name;
    t->t_next_wakeup = 0; 

    _clear_stack(stack_bottom, stack_size);
    t->t_stackptr = os_arch_task_stack_init(t, &stack_bottom[stack_size], 
            stack_size);

    /* insert this task into the scheduler list */
    rc = os_sched_insert(t, 0);
    if (rc != OS_OK) {
        goto err;
    }

    return (0);
err:
    return (rc);
}

