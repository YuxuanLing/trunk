kernel void QueryMinimumGranularity(int nLoop, global int *pOut)
{
    // 同步标志。初始状态为1；状态-1由后半边线程组设置，用于标识能够与前半线程组并发执行；状态-2由前半线程组设置，用于说明找到了不可并发执行的线程组的粒度，终止程序
    local volatile int flag;

    int index = get_global_id(0);

    // 这里，全局工作组个数应该总是为当前计算设备的最大工作组大小（MAX_WOKR_GROUP_SIZE）
    int totalItems = get_global_size(0);

    do
    {
        int halfIndex = totalItems / 2;

        // 设置初始标志状态值
        if(index == 0)
            flag = 1;

#if __OPENCL_C_VERSION__ < 200
        barrier(CLK_LOCAL_MEM_FENCE);
#else
        work_group_barrier(CLK_LOCAL_MEM_FENCE);
#endif

        if(index < halfIndex)
        {
            // 这里先用一个稍大的循环来等待后半线程组的并发执行，如果它能发生的话
            for(int i = 0; i < nLoop; i++)
            {
                // 如果当前标志
                if(flag == -1)
                    break;
            }

            // 若状态仍然为1，说明后半线程组并未并发执行，当前整个线程组粒度即为最小并行线程粒度
            if(flag != -1)
            {
                // 设置终止标志状态
                if(index == 0)
                {
                    *pOut = totalItems;
                    flag = 2;
                }
            }
        }
        else
        {
            if(index == halfIndex)
            {
                // 如果状态不是终止，那么设置-1状态，表示后半线程组已经并发执行了
                if(flag != 2)
                    flag = -1;
            }
        }

#if __OPENCL_C_VERSION__ < 200
        barrier(CLK_LOCAL_MEM_FENCE);
#else
        work_group_barrier(CLK_LOCAL_MEM_FENCE);
#endif

        if(flag == 2)
            break;

        // 如果不是最小并行线程粒度，则再对当前线程组的前半线程作为一个新的组进行测试
        totalItems /= 2;
    }
    while(totalItems > 0);
}
