#ifndef PBL_OCL_COMMANDQUEUE_H_
#define PBL_OCL_COMMANDQUEUE_H_

extern "C" PBLStatus_t pblOCLCreateCommandQueue(cl_context context, cl_device_id device, cl_command_queue_properties properties, cl_command_queue *queue) {
    int err;
    *queue = clCreateCommandQueue(context, device, properties, &err);
    return pblMapOCLErrorToPBLStatus(err);
}

#endif
