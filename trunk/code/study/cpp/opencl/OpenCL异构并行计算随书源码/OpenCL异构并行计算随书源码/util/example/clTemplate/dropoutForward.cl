//#define T float

kernel void dropoutForward(const int num, 
	global const T* in,
	global const T* mask,
	const T threshold,
	const T scale,
	global T* out) {
	int gid = get_global_id(0);
	out[gid] = in[gid] * (mask[gid] > threshold) * scale;
}

