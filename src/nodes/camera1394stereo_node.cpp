/* $Id: camera1394_node.cpp 34660 2010-12-11 18:27:24Z joq $ */

/*********************************************************************
* Software License Agreement (BSD License)
*
*  Copyright (c) 2010 Jack O'Quin
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the author nor other contributors may be
*     used to endorse or promote products derived from this software
*     without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/

#include <signal.h>
#include "driver1394stereo.h"
#include <std_srvs/Empty.h>

/** @file

    @brief ROS driver node for IIDC-compatible IEEE 1394 digital cameras.

*/

bool start_acq = false;

/** Segfault signal handler.
 *
 *  Sadly, libdc1394 sometimes crashes.
 */
void sigsegv_handler(int sig)
{
  signal(SIGSEGV, SIG_DFL);
  fprintf(stderr, "Segmentation fault, stopping camera driver.\n");
  ROS_ERROR("Segmentation fault, stopping camera.");
  ros::shutdown();                      // stop the main loop
}

bool startImageAcquisition(std_srvs::Empty::Request&, std_srvs::Empty::Response&)
{
  start_acq = true;
  return true;
}

/** Main node entry point. */
int main(int argc, char **argv)
{
  ros::init(argc, argv, "camera1394stereo_node");
  ros::NodeHandle node;
  ros::NodeHandle priv_nh("~");
  ros::NodeHandle camera_nh("stereo_camera");
  signal(SIGSEGV, &sigsegv_handler);
  camera1394stereo_driver::Camera1394StereoDriver dvr(priv_nh, camera_nh);

  bool wait_start_srv;
  node.param("use_start_srv", wait_start_srv, false);

  ros::ServiceServer start_srv = node.advertiseService("start_image_acquisition", &startImageAcquisition);

  dvr.setup();
  while (node.ok())
  {
    if (wait_start_srv && start_acq)
    {
      dvr.poll();
    } else if (!wait_start_srv) {
      dvr.poll();
    }
    ros::spinOnce();
  }
  dvr.shutdown();

  return 0;
}
