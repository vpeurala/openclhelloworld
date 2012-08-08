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
    cl_uint num_entries = 8;
    cl_platform_id* platforms = malloc(num_entries * sizeof (cl_platform_id));
    cl_uint num_platforms = -1;
    clGetPlatformIDs(num_entries, platforms, &num_platforms);

    size_t param_size = 1024 * sizeof (char);
    char* param_value = malloc(param_size);

    clGetPlatformInfo(platforms[0], CL_PLATFORM_VERSION, 1024 * sizeof (char), param_value, NULL);

    cl_uint num_devices = -1;
    cl_device_id* devices = malloc(num_entries * sizeof (cl_device_id));
    clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, num_entries, devices, &num_devices);

    for (int i = 0; i < num_devices; i++) {
        size_t device_type_size = sizeof(cl_device_type);
        cl_device_type* device_type = malloc(device_type_size);
        clGetDeviceInfo(devices[i], CL_DEVICE_TYPE, device_type_size, device_type, NULL);
        if (device_type[0] == CL_DEVICE_TYPE_GPU) {
            clGetDeviceInfo(devices[i], CL_DEVICE_VENDOR, param_size, param_value, NULL);
            clGetDeviceInfo(devices[i], CL_DEVICE_VERSION, param_size, param_value, NULL);
            clGetDeviceInfo(devices[i], CL_DEVICE_PROFILE, param_size, param_value, NULL);
            clGetDeviceInfo(devices[i], CL_DEVICE_EXTENSIONS, param_size, param_value, NULL);
            cl_context context = clCreateContext(NULL, 1, &devices[i], NULL, NULL, NULL);
            cl_int errcode_ret = -1;
            cl_command_queue command_queue = clCreateCommandQueue(context, devices[i], 0, &errcode_ret);

            cl_mem buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, sizeof (cl_int), NULL, &errcode_ret);

            const char* source = "__kernel void increment(int in, __global int* out) { out[0] = in + 1; }";

            cl_program program = clCreateProgramWithSource(context, 1, &source, NULL, &errcode_ret);

            clBuildProgram(program, 1, &devices[i], "", NULL, NULL);

            clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_LOG, param_size, param_value, NULL);

            cl_kernel kernel = clCreateKernel(program, "increment", &errcode_ret);

            clSetKernelArg(kernel, 0, sizeof(cl_int), &input);

            clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffer);

            clEnqueueTask(command_queue, kernel, 0, NULL, NULL);

            clFlush(command_queue);
            clFinish(command_queue);

            cl_int kernel_result = 0;

            clEnqueueReadBuffer(command_queue, buffer, CL_TRUE, 0, sizeof (cl_int), &kernel_result, 0, NULL, NULL);
            printf("%i\n", kernel_result);
        }
    }
    return 0;
}

