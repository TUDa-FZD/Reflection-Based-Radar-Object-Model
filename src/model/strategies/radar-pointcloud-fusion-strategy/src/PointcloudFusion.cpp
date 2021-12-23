//
// Copyright Philipp Rosenberger, M. Sc.
// Institute of Automotive Engineering
// of Technical University of Darmstadt, 2021.
// Licensed under the EUPL-1.2-or-later
//
// This work covered by the EUPL can be used/merged and distributed
// in other works covered by GPL-2.0, GPL-3.0, LGPL, AGPL, CeCILL,
// OSL, EPL, MPL and other licences listed as compatible in the EUPL
// Appendix. This applies to the other (combined) work, while the
// original project stays covered by the EUPL without re-licensing.
//
// Alternatively, the contents of this file may be used under the
// terms of the Mozilla Public License, v. 2.0. If a copy of the MPL
// was not distributed with this file, you can obtain one at
// http://mozilla.org/MPL/2.0/.
//

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include "model/strategies/transformation-functions/TransformationFunctions.hpp"
#include "pointcloudfusion/PointcloudFusion.hpp"
#include <string>

#ifdef _WIN32
#include <math.h>
#else
#include <cmath>
#endif

using namespace model;
using namespace osi3;

void PointcloudFusion::apply(SensorData &sensor_data) {
    log("Starting point cloud fusion");

    if (!sensor_data.sensor_view(0).lidar_sensor_view().empty()){
        if (sensor_data.has_feature_data() & (sensor_data.feature_data().lidar_sensor_size() > 0)) {
            calculate_fused_pointcloud_for_given_lidar_sensors(sensor_data, log);
            log("Size of logical detections after point cloud fusion: " + std::to_string(sensor_data.logical_detection_data().logical_detection_size()));
        }
        else{
            auto timestamp = (double) sensor_data.sensor_view(0).global_ground_truth().timestamp().seconds() + (double) sensor_data.sensor_view(0).global_ground_truth().timestamp().nanos() / 1000000000;
            log("No feature data available for timestamp " + std::to_string(timestamp));
        }
    }
    else if (!sensor_data.sensor_view(0).radar_sensor_view().empty()){
        if (sensor_data.has_feature_data() & (sensor_data.feature_data().radar_sensor_size() > 0)) {
            calculate_fused_pointcloud_for_given_radar_sensors(sensor_data, log);
            log("Size of logical detections after point cloud fusion: " + std::to_string(sensor_data.logical_detection_data().logical_detection_size()));
        }
        else{
            auto timestamp = (double) sensor_data.sensor_view(0).global_ground_truth().timestamp().seconds() + (double) sensor_data.sensor_view(0).global_ground_truth().timestamp().nanos() / 1000000000;
            log("No feature data available for timestamp " + std::to_string(timestamp));
        }
    }
}

//// Functions
void PointcloudFusion::calculate_fused_pointcloud_for_given_lidar_sensors(SensorData &sensor_data, const Log &log) {
    sensor_data.mutable_logical_detection_data()->clear_logical_detection();

    for (uint64_t sensor_idx = 0; sensor_idx < sensor_data.feature_data().lidar_sensor_size(); sensor_idx++) {
        for (uint64_t detection_no = 0; detection_no < sensor_data.feature_data().lidar_sensor(sensor_idx).detection_size(); detection_no++) {

            double elevation = sensor_data.feature_data().lidar_sensor(sensor_idx).detection(detection_no).position().elevation();
            double azimuth   = sensor_data.feature_data().lidar_sensor(sensor_idx).detection(detection_no).position().azimuth();
            double distance  = sensor_data.feature_data().lidar_sensor(sensor_idx).detection(detection_no).position().distance();

            Vector3d point_cartesian_sensor;
            point_cartesian_sensor.set_x(distance * cos(elevation) * cos(azimuth));
            point_cartesian_sensor.set_y(distance * cos(elevation) * sin(azimuth));
            point_cartesian_sensor.set_z(distance * sin(elevation));
            Vector3d point_cartesian_in_ego_coordinates = TransformationFunctions::transform_from_local_coordinates(point_cartesian_sensor,
                                                                                                                    sensor_data.feature_data().lidar_sensor(sensor_idx).header().mounting_position().orientation(),
                                                                                                                    sensor_data.feature_data().lidar_sensor(sensor_idx).header().mounting_position().position());

            auto current_logical_detection = sensor_data.mutable_logical_detection_data()->add_logical_detection();
            current_logical_detection->mutable_position()->CopyFrom(point_cartesian_in_ego_coordinates);
            if (sensor_data.feature_data().lidar_sensor(sensor_idx).detection(detection_no).has_intensity()) {
                current_logical_detection->set_intensity(sensor_data.feature_data().lidar_sensor(sensor_idx).detection(detection_no).intensity());
            }
            else if (sensor_data.feature_data().lidar_sensor(sensor_idx).detection(detection_no).has_echo_pulse_width()) {
                current_logical_detection->set_echo_pulse_width(sensor_data.feature_data().lidar_sensor(sensor_idx).detection(detection_no).echo_pulse_width()); //TODO: Field for echo pulse width within logical detections not existing, yet
            }
        }
    }
}

void PointcloudFusion::calculate_fused_pointcloud_for_given_radar_sensors(SensorData &sensor_data, const Log &log) {
    sensor_data.mutable_logical_detection_data()->clear_logical_detection();

    for (uint64_t sensor_idx = 0; sensor_idx < sensor_data.feature_data().radar_sensor_size(); sensor_idx++) {
        for (uint64_t detection_no = 0; detection_no < sensor_data.feature_data().radar_sensor(sensor_idx).detection_size(); detection_no++) {

            double elevation = sensor_data.feature_data().radar_sensor(sensor_idx).detection(detection_no).position().elevation();
            double azimuth   = sensor_data.feature_data().radar_sensor(sensor_idx).detection(detection_no).position().azimuth();
            double distance  = sensor_data.feature_data().radar_sensor(sensor_idx).detection(detection_no).position().distance();

            Vector3d point_cartesian_sensor;
            point_cartesian_sensor.set_x(distance * cos(elevation) * cos(azimuth));
            point_cartesian_sensor.set_y(distance * cos(elevation) * sin(azimuth));
            point_cartesian_sensor.set_z(distance * sin(elevation));
            Vector3d point_cartesian_in_ego_coordinates = TransformationFunctions::transform_from_local_coordinates(point_cartesian_sensor,
                                                                                                                    sensor_data.feature_data().radar_sensor(sensor_idx).header().mounting_position().orientation(),
                                                                                                                    sensor_data.feature_data().radar_sensor(sensor_idx).header().mounting_position().position());

            auto current_logical_detection = sensor_data.mutable_logical_detection_data()->add_logical_detection();
            current_logical_detection->mutable_position()->CopyFrom(point_cartesian_in_ego_coordinates);
            current_logical_detection->set_intensity(sensor_data.feature_data().radar_sensor(sensor_idx).detection(detection_no).rcs());
        }
    }
}