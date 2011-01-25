/*
 * vmath.c
 *
 *  Created on: Jan 19, 2011
 *      Author: seris
 */
#include "vmath.h"
#include <stdlib.h>
#include <stdio.h>

void prntV(vec3_t vec0) {
	printf("%5.2f, %5.2f, %5.2f \n",vec0[0],vec0[1],vec0[2]);
}

void prntF(float num) {
	printf("%5.2f \n",num);
}

int main(void) {
	vec3_t vec1 = {2.0,3.0,4.0};
	vec3_t vec2 = {7.0,6.0,5.0};
	vec3_t vec0;
	float num;
	DotProduct(vec1,vec2,num);
	prntF(num);
	CrossProduct(vec1,vec2,vec0);
	prntV(vec0);
	VectorAdd(vec1,vec2,vec0);
	prntV(vec0);
	VectorSubtract(vec1,vec2,vec0);
	prntV(vec0);
	VectorScale(vec1,3,vec0);
	prntV(vec0);
	VectorCopy(vec1,vec0);
	prntV(vec0);
	vec3_t vec4 = {1.0,1.0,1.0};
	VectorClear(vec4);
	prntV(vec4);
	VectorInverse(vec1,vec0);
	prntV(vec0);
	VectorMagnitude(vec1,num);
	prntF(num);
	VectorNormalize(vec1,num,vec0);
	prntV(vec0);
	return EXIT_SUCCESS;
}
