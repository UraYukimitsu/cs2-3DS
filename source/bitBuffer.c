#include "bitBuffer.h"

bitBuffer *bitBuffer_new(buffer, len)
	uchar *buffer;
	ulong len;
{	
	//printf("Entered function bitBuffer_new.\n");

	bitBuffer *self = malloc(sizeof(bitBuffer));
	self->buffer = buffer;
	self->len = len;
	self->index = 0;

	//printf("Exited function bitBuffer_new returning %08x.\n", (ulong)self);

	return self;
}

uchar getBit(self)
	bitBuffer *self;
{
	//printf("Entered function getBit.\n");
	if(self->index > 7)
	{
		self->index = 0;
		self->buffer++;
	}
	//printf("Exited function getBit.\n");
	return (*(self->buffer) >> self->index++) & 1;
}

ulong getEliasGamma(self)
	bitBuffer *self;
{
	ulong digits = 0, ret;

	//printf("Entered function getEliasGamma.\n");

	while(!getBit(self)) digits++; //Read the number of 0's
	ret = 1 << digits;
	while(digits--)
		if(getBit(self))
			ret |= 1 << digits;

	//printf("Exited function getEliasGamma.\n");

	return ret;
}
