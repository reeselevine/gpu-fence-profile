static void spin(__global atomic_uint* barrier, uint workgroups) {
  uint val = atomic_fetch_add_explicit(barrier, 1, memory_order_relaxed);
  while (val < workgroups) {
    val = atomic_load_explicit(barrier, memory_order_relaxed);
  }
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
