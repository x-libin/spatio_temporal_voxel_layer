/*********************************************************************
 *
 * Software License Agreement
 *
 *  Copyright (c) 2018, Simbe Robotics, Inc.
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
 *   * Neither the name of Simbe Robotics, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
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
 *
 * Author: Steve Macenski (steven.macenski@simberobotics.com)
 * Purpose: Replace the ROS voxel grid / obstacle layers using VoxelGrid
 *          with OpenVDB's more efficient and capacble voxel library with
 *          ray tracing and knn.
 *********************************************************************/

#ifndef VOLUME_GRID_LAYER_H_
#define VOLUME_GRID_LAYER_H_

// voxel grid
#include <spatio_temporal_voxel_layer/spatio_temporal_voxel_grid.hpp>
// ROS
#include <ros/ros.h>
#include <message_filters/subscriber.h>
// costmap
#include <costmap_2d/layer.h>
#include <costmap_2d/layered_costmap.h>
#include <costmap_2d/costmap_layer.h>
#include <costmap_2d/footprint.h>
// openVDB
#include <openvdb/openvdb.h>
// STL
#include <vector>
#include <string>
#include <iostream>
#include <time.h>
// msgs
#include <sensor_msgs/LaserScan.h>
#include <sensor_msgs/PointCloud2.h>
#include <sensor_msgs/point_cloud_conversion.h>
#include <geometry_msgs/Point.h>
#include <spatio_temporal_voxel_layer/SaveGrid.h>
// projector
#include <laser_geometry/laser_geometry.h>
// tf
#include <tf/message_filter.h>
#include <tf/exceptions.h>

namespace spatio_temporal_voxel_layer
{

// conveniences for line lengths
typedef std::vector<boost::shared_ptr<message_filters::SubscriberBase> >::iterator observation_subscribers_iter;
typedef std::vector<boost::shared_ptr<buffer::MeasurementBuffer> >::iterator observation_buffers_iter;

// Core ROS voxel layer class
class SpatioTemporalVoxelLayer : public costmap_2d::CostmapLayer
{
public:
  SpatioTemporalVoxelLayer(void);
  virtual ~SpatioTemporalVoxelLayer(void);

  // Core Functions
  virtual void onInitialize(void);
  virtual void updateBounds(double robot_x, double robot_y, double robot_yaw, \
                   double* min_x, double* min_y, double* max_x, double* max_y);
  virtual void updateCosts(costmap_2d::Costmap2D& master_grid, int min_i, int min_j, \
                                                               int max_i, int max_j);

  // Functions to interact with other layers
  virtual void matchSize(void);

  // Functions for layer high level operations
  virtual void reset(void);
  virtual void activate(void);
  virtual void deactivate(void);

  // Functions for sensor feeds
  bool GetMarkingObservations(std::vector<observation::MeasurementReading>& marking_observations) const;
  bool GetClearingObservations(std::vector<observation::MeasurementReading>& marking_observations) const;

  // Functions to interact with maps
  void UpdateROSCostmap(double* min_x, double* min_y, double* max_x, double* max_y);
  bool updateFootprint(double robot_x, double robot_y, double robot_yaw, \
                       double* min_x, double* min_y, double* max_x, double* max_y);
  void ResetGrid(void);

  // Saving grids callback for openVDB
  bool SaveGridCallback(spatio_temporal_voxel_layer::SaveGrid::Request& req, \
                        spatio_temporal_voxel_layer::SaveGrid::Response& resp);

private:
  // Sensor callbacks
  void LaserScanCallback(const sensor_msgs::LaserScanConstPtr& message, \
                         const boost::shared_ptr<buffer::MeasurementBuffer>& buffer);
  void LaserScanValidInfCallback(const sensor_msgs::LaserScanConstPtr& raw_message, \
                                 const boost::shared_ptr<buffer::MeasurementBuffer>& buffer);
  void PointCloud2Callback(const sensor_msgs::PointCloud2ConstPtr& message, \
                          const boost::shared_ptr<buffer::MeasurementBuffer>& buffer);

  // Functions for adding static obstacle zones
  bool AddStaticObservations(const observation::MeasurementReading& obs);
  bool RemoveStaticObservations(void);

  laser_geometry::LaserProjection                                  _laser_projector;
  std::vector<boost::shared_ptr<message_filters::SubscriberBase> > _observation_subscribers;
  std::vector<boost::shared_ptr<tf::MessageFilterBase> >           _observation_notifiers;
  std::vector<boost::shared_ptr<buffer::MeasurementBuffer> >       _observation_buffers;
  std::vector<boost::shared_ptr<buffer::MeasurementBuffer> >       _marking_buffers;
  std::vector<boost::shared_ptr<buffer::MeasurementBuffer> >       _clearing_buffers;

  bool                                 _publish_voxels, _mapping_mode;
  ros::Publisher                       _voxel_pub;
  ros::ServiceServer                   _grid_saver;
  ros::Duration                        _map_save_duration;
  ros::Time                            _last_map_save_time;
  std::string                          _global_frame;
  double                               _voxel_size;
  int                                  _combination_method, _mark_threshold;
  bool                                 _update_footprint_enabled, _enabled;
  std::vector<geometry_msgs::Point>    _transformed_footprint;
  std::vector<observation::MeasurementReading> _static_observations;
  volume_grid::SpatioTemporalVoxelGrid*        _voxel_grid;
};

}; // end namespace
#endif
