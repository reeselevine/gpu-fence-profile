static void spin(__global atomic_int* barrier, __global atomic_int* flag, int workgroups, int localSense) {
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_acq_rel, memory_scope_device);
    if (atomic_fetch_add_explicit(barrier, 1, memory_order_relaxed) == workgroups - 1) {
        atomic_store_explicit(barrier, 0, memory_order_relaxed);
        atomic_store_explicit(flag, localSense, memory_order_relaxed);
    } else {
	int curFlag = atomic_load_explicit(flag, memory_order_relaxed);
        while (curFlag != localSense) {
	    curFlag = atomic_load_explicit(flag, memory_order_relaxed);
            //atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_acq_rel, memory_scope_device);
	}
    }
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, memory_order_acq_rel, memory_scope_device);
}

__kernel void litmus_test(__global atomic_int* barrier, __global atomic_int* flag, __global atomic_int* data, __global int* results, __global int* paramsBuffer) {
    const int workgroups = paramsBuffer[0];
    int localSense = 0;
    for (int i = 0; i < paramsBuffer[1]; i++) {
    	if (get_local_id(0) == 0) {
            localSense = 1 - localSense;
	    if ((int) get_group_id(0) == workgroups - 1) {
	    	atomic_fetch_add_explicit(data, 1, memory_order_relaxed);
	        spin(barrier, flag, workgroups, localSense);
	    	localSense = 1 - localSense;
	    	spin(barrier, flag, workgroups, localSense);
	    } else {
		spin(barrier, flag, workgroups, localSense);
		results[get_group_id(0)] = results[get_group_id(0)] + atomic_load_explicit(data, memory_order_relaxed);
	    	localSense = 1 - localSense;
	    	spin(barrier, flag, workgroups, localSense);
	    }
        }
    }
}
