#include <rkisp_dev_manager.h>
#include <rkisp_control_loop.h>
#include <smartptr.h>
#include <v4l2_device.h>
#include <rkisp_device.h>
#include <dlfcn.h>

using namespace XCam;

int run_ispcl() {
    void *device_manager = NULL;
	SmartPtr<V4l2Device> capture_device;
    struct rkisp_cl_prepare_params_s params;

    rkisp_cl_init(&device_manager, "/etc/cam_iq.xml", NULL);
    params.isp_sd_node_path="/dev/v4l-subdev0";
    params.isp_vd_params_path="/dev/video3";
    params.isp_vd_stats_path="/dev/video2";
    params.sensor_sd_node_path="/dev/v4l-subdev2";
	params.lens_sd_node_path=NULL;

	rkisp_cl_prepare(device_manager, &params);
	capture_device = new RKispDevice ("/dev/video0");
    capture_device->set_mem_type (V4L2_MEMORY_MMAP);
    capture_device->set_buffer_count (4);
    capture_device->open ();
    ((RkispDeviceManager*)device_manager)->set_capture_device (capture_device);

	rkisp_cl_start(device_manager);

	for (int i=0; i<3; i++) {
		printf("++++++get buffer frome rksip device manager, NO.%d\n", i);
		((RkispDeviceManager*)device_manager)->dequeue_buffer();
	}

	rkisp_cl_stop(device_manager);
	rkisp_cl_deinit(device_manager);

    return 0;
}
int main() {
    int cnt = 3;

    while(cnt-- > 0) {
        sleep(1);
        run_ispcl();
    }
}
