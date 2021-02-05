static bool not_at_highest_level(__global atomic_int* levels, int level, int id, int N) {
    for (int j = 0; j < N; j++) {
        if (j != id && atomic_load_explicit(&levels[j], memory_order_seq_cst) >= level) {
            return true;
        }
    }
    return false;
}

static void petersons(__global atomic_int* levels, __global atomic_int* last_to_enter, __global int* var, int id, int N, __global atomic_int* atomic_var) {
    for (int level = 0; level < N - 1; level++) {
        atomic_store_explicit(&levels[id], level, memory_order_seq_cst);
        atomic_store_explicit(&last_to_enter[level], id, memory_order_seq_cst);
        atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_seq_cst, memory_scope_device);
        while (atomic_load_explicit(&last_to_enter[level], memory_order_seq_cst) == id && not_at_highest_level(levels, level, id, N));
    }
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_seq_cst, memory_scope_device);
    var[0] = var[0] + 1;
    atomic_fetch_add_explicit(atomic_var, 1, memory_order_seq_cst);
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_seq_cst, memory_scope_device);
    atomic_store_explicit(&levels[id], -1, memory_order_seq_cst);
}

__kernel void litmus_test(__global atomic_int* levels, __global atomic_int* last_to_enter, __global int* var, __global int* numWorkgroups, __global atomic_int* atomic_var) {
    const int workgroups = numWorkgroups[0];
    if (get_local_id(0) == 0) {
        for (uint i = 0; i < 1; i++) {
            petersons(levels, last_to_enter, var, (int) get_group_id(0), workgroups, atomic_var);
        }
    }
}
