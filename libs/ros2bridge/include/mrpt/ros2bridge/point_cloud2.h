/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          https://www.mrpt.org/                         |
   |                                                                        |
   | Copyright (c) 2005-2022, Individual contributors, see AUTHORS file     |
   | See: https://www.mrpt.org/Authors - All rights reserved.               |
   | Released under BSD License. See: https://www.mrpt.org/License          |
   +------------------------------------------------------------------------+ */

#pragma once

#include <mrpt/maps/CColouredPointsMap.h>
#include <mrpt/maps/CPointsMapXYZI.h>
#include <mrpt/maps/CSimplePointsMap.h>
#include <mrpt/obs/CObservationRotatingScan.h>

#include <sensor_msgs/msg/point_cloud2.hpp>
#include <set>
#include <string>

namespace mrpt::ros2bridge
{
/** \addtogroup mrpt_ros2bridge_grp
 * @{ */

/** @name sensor_msgs::msg::PointCloud2: ROS <-> MRPT
 *  @{ */

/** Convert sensor_msgs/PointCloud2 -> mrpt::slam::CSimplePointsMap
 *  Only (x,y,z) data is converted. To use the intensity channel, see
 * the alternative signature for CPointsMapXYZI.
 * Requires point cloud fields: x,y,z.
 * \return true on sucessful conversion, false on any error.
 * \sa toROS
 */
bool fromROS(
	const sensor_msgs::msg::PointCloud2& msg,
	mrpt::maps::CSimplePointsMap& obj);

/** \overload For (x,y,z,intensity) channels.
 * Requires point cloud fields: x,y,z,intensity
 */
bool fromROS(
	const sensor_msgs::msg::PointCloud2& msg, mrpt::maps::CPointsMapXYZI& obj);

/** Convert sensor_msgs/PointCloud2 -> mrpt::obs::CObservationRotatingScan.
 * Requires point cloud fields: x,y,z,intensity,ring
 */
bool fromROS(
	const sensor_msgs::msg::PointCloud2& m,
	mrpt::obs::CObservationRotatingScan& o,
	const mrpt::poses::CPose3D& sensorPoseOnRobot,
	unsigned int num_azimuth_divisions = 360);

/** Extract a list of fields found in the point cloud.
 * Typically: {"x","y","z","intensity"}
 */
std::set<std::string> extractFields(const sensor_msgs::msg::PointCloud2& msg);

/** Convert mrpt::slam::CSimplePointsMap -> sensor_msgs/PointCloud2
 *  The user must supply the "msg_header" field to be copied into the output
 * message object, since that part does not appear in MRPT classes.
 *
 *  Since CSimplePointsMap only contains (x,y,z) data,
 * sensor_msgs::msg::PointCloud2::channels will be empty.
 * \return true on sucessful conversion, false on any error.
 * \sa fromROS
 */
bool toROS(
	const mrpt::maps::CSimplePointsMap& obj,
	const std_msgs::msg::Header& msg_header,
	sensor_msgs::msg::PointCloud2& msg);

/** @} */
/** @} */

}  // namespace mrpt::ros2bridge
