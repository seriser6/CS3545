// Vector typedefs
typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t vec5_t[5];

// DotProduct(input vector 1, input vector 2, output float)
#define DotProduct(v1,v2,out) { \
		out = v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2]; \
}

// CrossProduct(input vector 1, input vector 2, output vector)
#define CrossProduct(v1,v2,out) { \
		out[0] = v1[1]*v2[2]-v1[2]*v2[1]; \
		out[1] = v1[2]*v2[0]-v1[0]*v2[2]; \
		out[2] = v1[0]*v2[1]-v1[1]*v2[0]; \
}

// VectorAdd(input vector 1, input vector 2, output vector)
#define VectorAdd(v1,v2,out) { \
		out[0] = v1[0]+v2[0]; \
		out[1] = v1[1]+v2[1]; \
		out[2] = v1[2]+v2[2]; \
}

// VectorSubtract(input vector 1, input vector 2, output vector)
//		(performs v2 - v1)
#define VectorSubtract(v1,v2,out) { \
		out[0] = v2[0]-v1[0]; \
		out[1] = v2[1]-v1[1]; \
		out[2] = v2[2]-v1[2]; \
}

// VectorScale(input vector, input scalar, output vector
#define VectorScale(v,s,out) { \
		out[0] = s*v[0]; \
		out[1] = s*v[1]; \
		out[2] = s*v[2]; \
}

// VectorCopy(input vector, output vector)
#define VectorCopy(v,out) { \
		out[0] = v[0]; \
		out[1] = v[1]; \
		out[2] = v[2]; \
}

// VectorClear(input/output vector)
#define VectorClear(v) { \
		v[0] = 0; \
		v[1] = 0; \
		v[2] = 0; \
}

// VectorInverse(input vector, output vector)
#define VectorInverse(v,out) { \
		out[0] = -1*v[0]; \
		out[1] = -1*v[1]; \
		out[2] = -1*v[2]; \
}

// VectorMagnitude(input vector, output float)
#define VectorMagnitude(v,out) { \
		float orig = v[0]*v[0]+v[1]*v[1]+v[2]*v[2]; \
		int init,fin = (int) orig; \
		char it = 1; \
		while ((fin=fin>>1)>1) if (it=!it) init = (init>>1); \
		out = (float) init; \
		while (out*out/orig-1>0) out=0.5*(out+orig/out); \
}

// VectorNormalize(input vector, input magnitude(calculated previously), output vector)
#define VectorNormalize(v,mag,out) { \
		out[0] = v[0]/mag; \
		out[1] = v[1]/mag; \
		out[2] = v[2]/mag; \
}
