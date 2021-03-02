static void spin_lock(__global atomic_int* flag, __global atomic_int* var) {
    int expected = 0;
    while(!atomic_compare_exchange_strong_explicit(flag, &expected, 1, memory_order_relaxed, memory_order_relaxed)) {
        expected = 0;
    }
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_acquire, memory_scope_device);
    int temp = atomic_load_explicit(var, memory_order_relaxed);
    atomic_store_explicit(var, temp + 1, memory_order_relaxed);
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_release, memory_scope_device);
    atomic_store_explicit(flag, 0, memory_order_relaxed);
}

__kernel void litmus_test(__global atomic_int* flag, __global atomic_int* var, __global int* paramsBuffer) {
    if (get_local_id(0) == 0) {
        for (int i = 0; i < paramsBuffer[1]; i++) {
	    spin_lock(flag, var);
        }
    }
}
