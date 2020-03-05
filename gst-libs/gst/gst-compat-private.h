#ifndef __GST_COMPAT_PRIVATE_H__
#define __GST_COMPAT_PRIVATE_H__

#include <glib.h>

G_BEGIN_DECLS

/* copies */

#if !GST_CHECK_VERSION(1,12,0)
 /**
 * gst_pad_get_task_state:
 * @pad: the #GstPad to get task state from
 *
 * Get @pad task state. If no task is currently
 * set, GST_TASK_STOPPED is returned.
 *
 * Returns: The current state of @pad's task.
 */
static inline GstTaskState
gst_pad_get_task_state (GstPad * pad)
{
  GstTask *task;
  GstTaskState res;

  g_return_val_if_fail (GST_IS_PAD (pad), GST_TASK_STOPPED);

  GST_OBJECT_LOCK (pad);
  task = GST_PAD_TASK (pad);
  if (task == NULL)
    goto no_task;
  res = gst_task_get_state (task);
  GST_OBJECT_UNLOCK (pad);

  return res;

no_task:
  {
    GST_DEBUG_OBJECT (pad, "pad has no task");
    GST_OBJECT_UNLOCK (pad);
    return GST_TASK_STOPPED;
  }
}
#endif

/* adaptations */

G_END_DECLS

#endif
