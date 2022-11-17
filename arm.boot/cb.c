/*
 * cb.c
 *
 *  Created on: Oct 8, 2022
 *      Author: ogruber
 */
#include "main.h"
#include "cb.h"

int cb_full(struct cb *cb)
{
	int next = (cb->head + 1) % CAPACITY;
	return (next == cb->tail);
}

int cb_empty(struct cb *cb)
{
	return (cb->tail == cb->head);
}

int cb_put(struct cb *cb, uint8_t byte)
{
	int next = (cb->head + 1) % CAPACITY;
	if (next == cb->tail)
		return -1;
	cb->buffer[cb->head] = byte;
	cb->head = next;
	cb->size = cb->size + 1;
	return 0;
}

int cb_get(struct cb *cb, uint8_t *bytePt)
{
	if (cb->tail == cb->head)
		return -1;
	*bytePt = cb->buffer[cb->tail];
	cb->tail = (cb->tail + 1) % CAPACITY;
	cb->size = cb->size - 1;
	return 0;
}

void cb_init(struct cb *cb)
{
	cb->head = cb->tail = 0;
	cb->size = 0;
}

int is_size(struct cb *cb, int s)
{
	return cb->size == s;
}
