// flag is initialized to 0                                                                                           
__kernel void litmus_test(__global atomic_int* flag) {
  if(get_group_id(0) == 0 && get_local_id(0) == 0) {
    while (atomic_load_explicit(flag,memory_order_relaxed) == 0);
  }
}
