void p0(__global atomic_uint* flag0, __global atomic_uint* flag1, __global atomic_uint* turn, __global uint* var) {
    atomic_store_explicit(flag0, 1, memory_order_relaxed);
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_seq_cst, memory_scope_device);
    while (atomic_load_explicit(flag1, memory_order_relaxed) == 1) {
	    if (atomic_load_explicit(turn, memory_order_relaxed) != 0) {
	        atomic_store_explicit(flag0, 0, memory_order_relaxed);
	        while(atomic_load_explicit(turn, memory_order_relaxed != 0));
	        atomic_store_explicit(flag0, 1, memory_order_relaxed);
            atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_seq_cst, memory_scope_device);
	    }
    }
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_acquire, memory_scope_device);
    var[0] = 1;
    atomic_store_explicit(turn, 1, memory_order_relaxed);
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_release, memory_scope_device);
    atomic_store_explicit(flag0, 0, memory_order_relaxed);
}

void p1(__global atomic_uint* flag0, __global atomic_uint* flag1, __global atomic_uint* turn, __global uint* var) {
    atomic_store_explicit(flag1, 1, memory_order_relaxed);
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_seq_cst, memory_scope_device);
    while (atomic_load_explicit(flag0, memory_order_relaxed) == 1) {
	    if (atomic_load_explicit(turn, memory_order_relaxed) != 1) {
	        atomic_store_explicit(flag1, 0, memory_order_relaxed);
	        while(atomic_load_explicit(turn, memory_order_relaxed != 1));
	        atomic_store_explicit(flag1, 1, memory_order_relaxed);
            atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_seq_cst, memory_scope_device);
	    }
    }
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_acquire, memory_scope_device);
    var[0] = 2;
    atomic_store_explicit(turn, 0, memory_order_relaxed);
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_release, memory_scope_device);
    atomic_store_explicit(flag1, 0, memory_order_relaxed);
}


__kernel void litmus_test(__global atomic_uint* flag0, __global atomic_uint* flag1, __global atomic_uint* turn, __global uint* var) {
    if (get_global_id(0) == 0) {
        p0(flag0, flag1, turn ,var);
    } else if (get_global_id(0) == 1 * get_local_size(0)) {
        p1(flag0, flag1, turn, var);
    }
}
