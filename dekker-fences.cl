void p0(__global atomic_uint* flag0, __global atomic_uint* flag1, __global atomic uint* turn, __global uint* var) {
    atomic_store_explicit(flag0, 1, memory_order_seq_cst);
    while (atomic_load_explicit(flag1, memory_order_relaxed) == 1) {
	if (atomic_load_explicit(turn, memory_order_relaxed) != 0) {
	    atomic_store_explicit(flag0, 0, memory_order_relaxed);
	    while(atomic_load_explicit(turn, memory_order_relaxed != 0));
	    atomic_store_explicit(flag0, 1, memory_order_seq_cst);
	} else {
	    k
}

__kernel void litmus_test(__global atomic_uint* barrier, __global uint* data, __global atomic_uint* results, __global uint* numWorkgroups) {
    const uint workgroups = numWorkgroups[0];
    if (get_local_id(0) == 0) {
	if (get_group_id(0) == workgroups - 1) {
	    data[0] = 1;
	    spin(barrier, workgroups);
	} else {
	    spin(barrier, workgroups);
	    uint result = data[0];
	    atomic_store_explicit(&results[get_group_id(0)], result, memory_order_relaxed);
	}
    }
}
