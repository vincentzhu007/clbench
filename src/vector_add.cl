__kernel void vector_add(__global const float *a, __global const float *b, __global float *result) {
    int gid = get_global_id(0);
    // printf("a[%d] = %f, b[%d] = %f\n", gid, a[gid], gid, b[gid]);
    result[gid] = a[gid] + b[gid];
}