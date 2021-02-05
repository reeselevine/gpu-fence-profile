static int max_ticket(__global atomic_int* tickets, int N) {
    int cur_max = 0;
    for (int j = 0; j < N; j++) {
        int cur = atomic_load_explicit(&tickets[j], memory_order_relaxed);
        if (cur > cur_max) {
            cur_max = cur;
        }
    }
    return cur_max;
}

static void lamports_bakery(__global atomic_int* entering, __global atomic_int* tickets, __global int* var, int id, int N) {
    atomic_store_explicit(&entering[id], 1, memory_order_relaxed); 
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_seq_cst, memory_scope_device);
    atomic_store_explicit(&tickets[id], max_ticket(tickets, N) + 1, memory_order_relaxed);
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_seq_cst, memory_scope_device);
    atomic_store_explicit(&entering[id], 0, memory_order_relaxed);
    for (int j = 0; j < N; j++) {
        while (atomic_load_explicit(&entering[j], memory_order_relaxed) == 1);
        while (atomic_load_explicit(&tickets[j], memory_order_relaxed) != 0 && (atomic_load_explicit(&tickets[j], memory_order_relaxed) < atomic_load_explicit(&tickets[id], memory_order_relaxed) ||
                                                                                (atomic_load_explicit(&tickets[j], memory_order_relaxed) == atomic_load_explicit(&tickets[id], memory_order_relaxed) && j < id)));
    }
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_acquire, memory_scope_device);
    var[0] = var[0] + 1;
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_release, memory_scope_device);
    atomic_store_explicit(&tickets[id], 0, memory_order_relaxed);

}

__kernel void litmus_test(__global atomic_int* entering, __global atomic_int* tickets, __global int* var, __global int* numWorkgroups) {
    const int workgroups = numWorkgroups[0];
    if (get_local_id(0) == 0) {
        for (uint i = 0; i < 1; i++) {
            lamports_bakery(entering, tickets, var, (int) get_group_id(0), workgroups);
        }
    }
}
