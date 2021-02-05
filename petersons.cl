static bool not_at_highest_level(__global atomic_int* levels, int level, int id, int N) {
    for (int j = 0; j < N; j++) {
        if (j != id && atomic_load_explicit(&levels[j], memory_order_relaxed) >= level) {
            return true;
        }
    }
    return false;
}

static void petersons(__global atomic_int* levels, __global atomic_int* last_to_enter, __global int* var, int id, int N) {
    for (int level = 0; level < N - 1; level++) {
        atomic_store_explicit(&levels[id], level, memory_order_relaxed);
        atomic_store_explicit(&last_to_enter[level], id, memory_order_relaxed);
	int last_in_level = id;
        while (last_in_level == id && not_at_highest_level(levels, level, id, N)) {
    	    last_in_level = atomic_load_explicit(&last_to_enter[level], memory_order_relaxed);
    	}
    }
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_acquire, memory_scope_device);
    var[0] = var[0] + 1;
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_release, memory_scope_device);
    atomic_store_explicit(&levels[id], -1, memory_order_relaxed);
}

__kernel void litmus_test(__global atomic_int* levels, __global atomic_int* last_to_enter, __global int* var, __global int* paramsBuffer) {
    const int workgroups = paramsBuffer[0];
    if (get_local_id(0) == 0) {
        for (int i = 0; i < paramsBuffer[1]; i++) {
            petersons(levels, last_to_enter, var, (int) get_group_id(0), workgroups);
        }
    }
}
