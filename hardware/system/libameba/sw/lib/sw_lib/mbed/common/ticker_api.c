/* mbed Microcontroller Library
 * Copyright (c) 2015 ARM Limited
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
 */
#include <stddef.h>
#include "ticker_api.h"
#include "cmsis.h"

static uint32_t last_time=0;

void ticker_set_handler(const ticker_data_t *const data, ticker_event_handler handler) {
    data->interface->init();

    data->queue->event_handler = handler;
	
	//DiagPrintf(" %s event_handler : 0x%x\r\n", __FUNCTION__, data->queue->event_handler);
}

void ticker_irq_handler(const ticker_data_t *const data) {
	//static uint32_t last_time=0;	
	uint32_t cur_time;
	uint32_t timestamp;
	uint32_t difftime;
	uint8_t isHit;
	//uint8_t isRewind;
	

    data->interface->clear_interrupt();

    cur_time = data->interface->read();
	

    /* Go through all the pending TimerEvents */
    while (1) {
        if (data->queue->head == NULL) {
            // There are no more TimerEvents left, so disable matches.
            data->interface->disable_interrupt();
			break;
        }

		//Neo+++ : This is for time overrun and reset to 0
		cur_time = (int)data->interface->read();
		timestamp = (int)data->queue->head->timestamp;

		isHit = 0;
		if  ( (((int)(cur_time) < 0) && ((int)(timestamp) > 0)) ) {
			difftime = cur_time - timestamp; 
			if ( difftime < 0x7fffffff ) isHit = 1; 
		} else if (((int)(cur_time) > 0) && ((int)(timestamp) < 0))  {
			difftime = timestamp - cur_time;
			if ( difftime > 0x80000000 ) isHit = 1;
		} else {
			if ( timestamp < cur_time ) isHit = 1; 			
		}
		//rtl_printf("ticker_irq_handler: timestamp(0x%x) cur_time(0x%x) isHit(%d) \r\n", 
			//timestamp, cur_time, isHit);
		// Neo ---

		if ( isHit ) {
	            // This event was in the past:
	            //      point to the following one and execute its handler
	            ticker_event_t *p = data->queue->head;
	            data->queue->head = data->queue->head->next;
	            if (data->queue->event_handler != NULL) {
	                (*data->queue->event_handler)(p->id); // NOTE: the handler can set new events
	            }
	            /* Note: We continue back to examining the head because calling the
             			* event handler may have altered the chain of pending events. */
		}
        else {
            // This event and the following ones in the list are in the future:
            //      set it as next interrupt and return
            data->interface->set_interrupt(timestamp);
            break;
        }
    }
	last_time = cur_time;
}

void ticker_insert_event(const ticker_data_t *const data, ticker_event_t *obj, timestamp_t timestamp, uint32_t id) {
	
	uint32_t cur_time;
	uint8_t isRewind;

    /* disable interrupts for the duration of the function */
    __disable_irq();

    // initialise our data
    obj->timestamp = timestamp;
    obj->id = id;

    /* Go through the list until we either reach the end, or find
       an element this should come before (which is possibly the
       head). */
    ticker_event_t *prev = NULL, *p = data->queue->head;

	cur_time = data->interface->read();
	isRewind = ( last_time > timestamp )? 1 : 0; 
	rtl_printf("ticker_insert_event: timestamp=0x%x, last_time=0x%x, cur_time=0x%x, isRewind=%d\r\n", 
		timestamp, last_time, cur_time, isRewind);
	
    while (p != NULL) {

		if (isRewind) {
			if ( timestamp > p->timestamp ) break;
		} else {
			if ( timestamp < p->timestamp ) break;
		}
        /* go to the next element */
        prev = p;
        p = p->next;
    }
	
    /* if prev is NULL we're at the head */
    if (prev == NULL) {
        data->queue->head = obj;
        data->interface->set_interrupt(timestamp);
    } else {
        prev->next = obj;
    }
    /* if we're at the end p will be NULL, which is correct */
    obj->next = p;

    __enable_irq();
}

void ticker_remove_event(const ticker_data_t *const data, ticker_event_t *obj) {
    __disable_irq();

    // remove this object from the list
    if (data->queue->head == obj) {
        // first in the list, so just drop me
        data->queue->head = obj->next;
        if (data->queue->head == NULL) {
            data->interface->disable_interrupt();
        } else {
            data->interface->set_interrupt(data->queue->head->timestamp);
        }
    } else {
        // find the object before me, then drop me
        ticker_event_t* p = data->queue->head;
        while (p != NULL) {
            if (p->next == obj) {
                p->next = obj->next;
                break;
            }
            p = p->next;
        }
    }

    __enable_irq();
}

timestamp_t ticker_read(const ticker_data_t *const data)
{
    return data->interface->read();
}

int ticker_get_next_timestamp(const ticker_data_t *const data, timestamp_t *timestamp)
{
    int ret = 0;

    /* if head is NULL, there are no pending events */
    __disable_irq();
    if (data->queue->head != NULL) {
        *timestamp = data->queue->head->timestamp;
        ret = 1;
    }
    __enable_irq();

    return ret;
}

