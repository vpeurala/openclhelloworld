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
    //printf("input: %i\n", input);
    cl_uint num_entries = 8;
    cl_platform_id* platforms = malloc(num_entries * sizeof (cl_platform_id));
    cl_uint num_platforms = -1;
    clGetPlatformIDs(num_entries, platforms, &num_platforms);
    //printf("platform_id: %p\n", platforms[0]);
    //printf("number_of_platforms: %u\n", num_platforms);

    size_t param_size = 1024 * sizeof (char);
    char* param_value = malloc(param_size);

    clGetPlatformInfo(platforms[0], CL_PLATFORM_VERSION, 1024 * sizeof (char), param_value, NULL);

    //printf("platform_version: %s\n", param_value);

    cl_uint num_devices = -1;
    cl_device_id* devices = malloc(num_entries * sizeof (cl_device_id));
    clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, num_entries, devices, &num_devices);

    //printf("num_devices: %u\n", num_devices);

    for (int i = 0; i < num_devices; i++) {
        size_t device_type_size = sizeof(cl_device_type);
        cl_device_type* device_type = malloc(device_type_size);
        clGetDeviceInfo(devices[i], CL_DEVICE_TYPE, device_type_size, device_type, NULL);
        //printf("device_type: %llu\n", device_type[0]);
        if (device_type[0] == CL_DEVICE_TYPE_GPU) {
            //printf("THIS IS A GPU\n");
            clGetDeviceInfo(devices[i], CL_DEVICE_VENDOR, param_size, param_value, NULL);
            //printf("GPU VENDOR: %s\n", param_value);
            clGetDeviceInfo(devices[i], CL_DEVICE_VERSION, param_size, param_value, NULL);
            //printf("GPU VERSION: %s\n", param_value);
            clGetDeviceInfo(devices[i], CL_DEVICE_PROFILE, param_size, param_value, NULL);
            //printf("GPU PROFILE: %s\n", param_value);
            clGetDeviceInfo(devices[i], CL_DEVICE_EXTENSIONS, param_size, param_value, NULL);
            //printf("GPU EXTENSIONS: %s\n", param_value);
            cl_context context = clCreateContext(NULL, 1, &devices[i], NULL, NULL, NULL);
            //printf("cl_context: %p\n", context);
            cl_int errcode_ret = -1;
            cl_command_queue command_queue = clCreateCommandQueue(context, devices[i], 0, &errcode_ret);
            //printf("errcode_ret: %i\n", errcode_ret);
            //printf("command_queue %p\n", command_queue);

            cl_mem buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, sizeof (cl_int), NULL, &errcode_ret);
            //printf("errcode_ret: %i\n", errcode_ret);
            //printf("buffer: %p\n", buffer);

            const char* source = "__kernel void increment(int in, __global int* out) { out[0] = in + 1; }";

            cl_program program = clCreateProgramWithSource(context, 1, &source, NULL, &errcode_ret);
            //printf("errcode_ret: %i\n", errcode_ret);
            //printf("program: %p\n", program);

            cl_int program_build_status = clBuildProgram(program, 1, &devices[i], "", NULL, NULL);
            //printf("program_build_status: %i\n", program_build_status);

            clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_LOG, param_size, param_value, NULL);
            //printf("Build log: %s\n", param_value);

            cl_kernel kernel = clCreateKernel(program, "increment", &errcode_ret);
            //printf("errcode_ret: %i\n", errcode_ret);
            //printf("kernel: %p\n", kernel);

            cl_int arg_set_result_0 = clSetKernelArg(kernel, 0, sizeof(cl_int), &input);
            //printf("arg_set_result_0: %i\n", arg_set_result_0);

            cl_int arg_set_result_1 = clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffer);
            //printf("arg_set_result_1: %i\n", arg_set_result_1);

            cl_int enqueue_task_result = clEnqueueTask(command_queue, kernel, 0, NULL, NULL);
            //printf("enqueue_task_result: %i\n", enqueue_task_result);

            cl_int flush_result = clFlush(command_queue);
            //printf("flush_result: %i\n", flush_result);
            cl_int finish_result = clFinish(command_queue);
            //printf("finish_result: %i\n", finish_result);

            cl_int end_result = 0;

            cl_int enqueue_read_buffer_result = clEnqueueReadBuffer(command_queue, buffer, CL_TRUE, 0, sizeof (cl_int), &end_result, 0, NULL, NULL);
            //printf("enqueue_read_buffer_result: %i\n", enqueue_read_buffer_result);

            printf("%i\n", end_result);
        }
    }
    return 0;
}

