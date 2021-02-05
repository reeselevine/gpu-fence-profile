static bool other_thread_waiting(__global atomic_int* flags, int id, int N) {
    for (int j = 0; j < N; j++) {
        if (j != id && atomic_load_explicit(&flags[j], memory_order_relaxed) == 1) {
            return true;
        }
    }
    return false;
}

static void dekkers(__global atomic_int* flags, __global atomic_int* turn, __global int* var, int id, int N) {
    atomic_store_explicit(&flags[id], 1, memory_order_relaxed);
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_seq_cst, memory_scope_device);
    while (other_thread_waiting(flags, id, N)) {
        atomic_store_explicit(&flags[id], 0, memory_order_relaxed);
        while(atomic_load_explicit(turn, memory_order_relaxed) != 0);
        atomic_store_explicit(turn, id + 1, memory_order_relaxed);
        atomic_store_explicit(&flags[id], 1, memory_order_relaxed);
        atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_seq_cst, memory_scope_device);
    }
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_acquire, memory_scope_device);
    var[0] = var[0] + 1;
    atomic_store_explicit(turn, 0, memory_order_relaxed);
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_release, memory_scope_device);
    atomic_store_explicit(&flags[id], 0, memory_order_relaxed);

}

__kernel void litmus_test(__global atomic_int* flags, __global atomic_int* turn, __global  int* var, __global int* numWorkgroups) {
    const int workgroups = numWorkgroups[0];
    if (get_local_id(0) == 0) {
        for (uint i = 0; i < 1; i++) {
            dekkers(flags, turn, var, (int) get_group_id(0), workgroups);
        }
    }
}
