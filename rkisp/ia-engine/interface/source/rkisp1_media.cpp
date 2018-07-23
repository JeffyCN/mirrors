#include "rkisp1_media.h"
#include <utils/Log.h>

#define TRUE 1
#define FALSE 0
#define SYS_PATH                "/sys/class/video4linux/"
#define DEV_PATH                "/dev/"

bool
rk_common_v4l2device_find_by_name (const char *name, char *ret_name)
{
  DIR *dir;
  struct dirent *ent;
  bool ret = FALSE;

  if ((dir = opendir (SYS_PATH)) != NULL) {
    while ((ent = readdir (dir)) != NULL) {
      FILE *fp;
      char path[512];
      char dev_name[512];

      snprintf (path, 512, SYS_PATH "%s/name", ent->d_name);
      fp = fopen (path, "r");
      if (!fp)
        continue;
      if (!fgets (dev_name, 32, fp))
        dev_name[0] = '\0';
      fclose (fp);

      if (!strstr (dev_name, name))
        continue;

      if (ret_name)
        snprintf (ret_name, 512, DEV_PATH "%s", ent->d_name);

      ret = TRUE;
      break;
    }
    closedir (dir);
  }

  return ret;
}

#if 0
int
rkisp1_init_media(int width, int height)
{
	int media_index;
	char *videodev = "/dev/video0";
	char vdev[32];
	char *sensor_name = "/dev/video0";
	struct media_entity *isp_subdev;
	struct media_entity *phy_subdev;
	struct media_entity *sensor_subdev;

	rk_common_v4l2device_find_by_name ("rkisp1_mainpath", vdev);
	if (strcmp (videodev, vdev)) {
		LOGD ("start ISP self path......\n");
	} else {
		LOGD ("start ISP main path......\n");
	}

	media_index =
		rk_common_media_find_by_vnode (videodev);
	if (media_index == -1)
		return FALSE;
	
	isp_subdev =
		rk_common_media_find_subdev_by_name (media_index,
		"rkisp1-isp-subdev");
	phy_subdev =
		rk_common_media_find_subdev_by_name (media_index,
		"rkisp1-input-params");

	if (sensor_name)
		sensor_subdev =
			rk_common_media_find_subdev_by_name (media_index,
				sensor_name);
	else
		/* assume the last enity is sensor */
		sensor_subdev =
			rk_common_media_get_last_enity (media_index);
	/* TODO: change link */

	rkisp1_set_media(width, height, sensor_subdev, phy_subdev, isp_subdev);
}
int
rkisp1_set_media(int width, int height,
		struct media_entity *sensor_subdev,
		struct media_entity *phy_subdev,
		struct media_entity *isp_subdev)
{
	struct v4l2_mbus_framefmt format;
	
	  /* TODO: should calculate? */
	format.width = width;
	format.height = height;
	format.field = V4L2_FIELD_NONE;
	v4l2_subdev_set_format (sensor_subdev, &format, 0,
		V4L2_SUBDEV_FORMAT_ACTIVE);
	v4l2_subdev_get_format (sensor_subdev, &format, 0,
		V4L2_SUBDEV_FORMAT_ACTIVE);
	
	/* propagate to dphy */
	v4l2_subdev_set_format (phy_subdev, &format, MIPI_DPHY_SY_PAD_SINK,
		V4L2_SUBDEV_FORMAT_ACTIVE);
	v4l2_subdev_set_format (phy_subdev, &format,
		MIPI_DPHY_SY_PAD_SOURCE, V4L2_SUBDEV_FORMAT_ACTIVE);
	
	/* propagate to rkisp */
	v4l2_subdev_set_format (isp_subdev, &format,
		RKISP1_ISP_PAD_SINK, V4L2_SUBDEV_FORMAT_ACTIVE);
	format.code = MEDIA_BUS_FMT_YUYV8_2X8;
	v4l2_subdev_set_format (isp_subdev, &format,
		RKISP1_ISP_PAD_SOURCE_PATH, V4L2_SUBDEV_FORMAT_ACTIVE);

	return 0;
}

int
rk_common_media_find_by_vnode (const char *vnode)
{
  int i, j;

  for (i = 0; i < 16; i++) {
    unsigned int nents;

    if (g_media_data[i].index == -1)
      break;

    nents = media_get_entities_count (g_media_data[i].device);
    for (j = 0; j < nents; ++j) {
      struct media_entity *entity =
          media_get_entity (g_media_data[i].device, j);
      const char *devname = media_entity_get_devname (entity);

      if (!strcmp (devname, vnode))
        return g_media_data[i].index;
    }
  }

  return -1;
}

struct media_entity *
rk_common_media_find_subdev_by_name (int index, const char *subdev_name)
{
  return media_get_entity_by_name (g_media_data[index].device, subdev_name,
      strlen (subdev_name));
}

struct media_entity *
rk_common_media_get_last_enity (int index)
{
  struct media_entity *entity;
  int nents;

  nents = media_get_entities_count (g_media_data[index].device);
  entity = media_get_entity (g_media_data[index].device, nents - 1);

  return entity;
}
#endif
