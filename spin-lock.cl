static void spin_lock(__global atomic_int* flag, __global int* var) {
    int expected = 0;
    while(!atomic_compare_exchange_strong_explicit(flag, &expected, 1, memory_order_relaxed, memory_order_relaxed)) {
        expected = 0;
    }
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_acquire, memory_scope_device);
    var[0] = var[0] + 1;
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_release, memory_scope_device);
    atomic_store_explicit(flag, 0, memory_order_relaxed);
}

__kernel void litmus_test(__global atomic_int* flag, __global int* var, __global int* paramsBuffer) {
    if (get_local_id(0) == 0) {
        for (int i = 0; i < paramsBuffer[0]; i++) {
	    spin_lock(flag, var);
        }
    }
}
