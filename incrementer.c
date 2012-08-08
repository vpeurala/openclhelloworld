#include <stdlib.h>
#include <stdio.h>
#include <OpenCL/OpenCL.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Missing required argument input.\n");
        printf("Usage: %s input\n", argv[0]);
        return -1;
    }
    int input = atoi(argv[1]);
    // 8 is an arbitrary maximum number of platforms.
    cl_uint num_entries = 8;
    cl_platform_id* platforms = malloc(num_entries * sizeof (cl_platform_id));
    cl_uint num_platforms = -1;
    clGetPlatformIDs(num_entries, platforms, &num_platforms);
    cl_uint num_devices = -1;
    cl_device_id* devices = malloc(num_entries * sizeof (cl_device_id));
    clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, num_entries, devices, &num_devices);
    for (int i = 0; i < num_devices; i++) {
        size_t device_type_size = sizeof(cl_device_type);
        cl_device_type* device_type = malloc(device_type_size);
        clGetDeviceInfo(devices[i], CL_DEVICE_TYPE, device_type_size, device_type, NULL);
        if (device_type[0] == CL_DEVICE_TYPE_GPU) {
            cl_context context = clCreateContext(NULL, 1, &devices[i], NULL, NULL, NULL);
            cl_command_queue command_queue = clCreateCommandQueue(context, devices[i], 0, NULL);
            cl_mem buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, sizeof (cl_int), NULL, NULL);
            const char* source = "__kernel void increment(int in, __global int* out) { out[0] = in + 1; }";
            cl_program program = clCreateProgramWithSource(context, 1, &source, NULL, NULL);
            clBuildProgram(program, 1, &devices[i], "", NULL, NULL);
            cl_kernel kernel = clCreateKernel(program, "increment", NULL);
            clSetKernelArg(kernel, 0, sizeof(cl_int), &input);
            clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffer);
            clEnqueueTask(command_queue, kernel, 0, NULL, NULL);
            clFlush(command_queue);
            clFinish(command_queue);
            cl_int kernel_result = 0;
            clEnqueueReadBuffer(command_queue, buffer, CL_TRUE, 0, sizeof (cl_int), &kernel_result, 0, NULL, NULL);
            printf("%i\n", kernel_result);
        }
        free(device_type);
    }
    free(devices);
    free(platforms);
    return 0;
}

