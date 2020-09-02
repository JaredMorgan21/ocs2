/******************************************************************************
Copyright (c) 2020, Farbod Farshidian. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

 * Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

#include <tf/tf.h>
#include <urdf/model.h>
#include <kdl_parser/kdl_parser.hpp>

#include <geometry_msgs/PoseArray.h>
#include <visualization_msgs/MarkerArray.h>

#include "ocs2_mobile_manipulator_example/MobileManipulatorDummyVisualization.h"
#include "ocs2_mobile_manipulator_example/MobileManipulatorVisualizationHelpers.h"
#include "ocs2_mobile_manipulator_example/definitions.h"

namespace mobile_manipulator {

void MobileManipulatorDummyVisualization::launchVisualizerNode(ros::NodeHandle& nodeHandle) {
  // load a kdl-tree from the urdf robot description and initialize the robot state publisher
  const std::string urdfName = "robot_description";
  urdf::Model model;
  if (!model.initParam(urdfName)) {
    ROS_ERROR("URDF model load was NOT successful");
  }
  KDL::Tree tree;
  if (!kdl_parser::treeFromUrdfModel(model, tree)) {
    ROS_ERROR("Failed to extract kdl tree from xml robot description");
  }

  robotStatePublisherPtr_.reset(new robot_state_publisher::RobotStatePublisher(tree));
  robotStatePublisherPtr_->publishFixedTransforms("", true);

  stateOptimizedPublisher_ = nodeHandle.advertise<visualization_msgs::MarkerArray>("/mobile_manipulator/optimizedStateTrajectory", 1);
  stateOptimizedPosePublisher_ = nodeHandle.advertise<geometry_msgs::PoseArray>("/mobile_manipulator/optimizedPoseTrajectory", 1);
}

void MobileManipulatorDummyVisualization::update(const ocs2::SystemObservation& observation, const ocs2::PrimalSolution& policy,
                                                 const ocs2::CommandData& command) {
  const ros::Time timeStamp = ros::Time::now();

  publishObservation(timeStamp, observation);
  publishDesiredTrajectory(timeStamp, command.mpcCostDesiredTrajectories_);
  publishOptimizedTrajectory(timeStamp, policy);
}

void MobileManipulatorDummyVisualization::publishObservation(const ros::Time& timeStamp, const ocs2::SystemObservation& observation) {
  // publish world -> base transform
  const auto position = getBasePosition(observation.state);
  const auto orientation = getBaseOrientation(observation.state);

  geometry_msgs::TransformStamped base_tf;
  base_tf.header.stamp = timeStamp;
  base_tf.header.frame_id = "world";
  base_tf.child_frame_id = "base";
  base_tf.transform.translation = getVectorMsg(position);
  base_tf.transform.rotation = getOrientationMsg(orientation);
  tfBroadcaster_.sendTransform(base_tf);

  // publish joints transforms
  const auto j_arm = getArmJointPositions(observation.state);
  std::map<std::string, double> jointPositions{{"SH_ROT", j_arm(0)}, {"SH_FLE", j_arm(1)}, {"EL_FLE", j_arm(2)},
                                               {"EL_ROT", j_arm(3)}, {"WR_FLE", j_arm(4)}, {"WR_ROT", j_arm(5)}};
  robotStatePublisherPtr_->publishTransforms(jointPositions, timeStamp, "");
}

void MobileManipulatorDummyVisualization::publishDesiredTrajectory(const ros::Time& timeStamp,
                                                                   const ocs2::CostDesiredTrajectories& costDesiredTrajectory) {
  // publish command transform
  const Eigen::Vector3d eeDesiredPosition = costDesiredTrajectory.desiredStateTrajectory().back().head(3);
  Eigen::Quaterniond eeDesiredOrientation;
  eeDesiredOrientation.coeffs() = costDesiredTrajectory.desiredStateTrajectory().back().tail(4);
  geometry_msgs::TransformStamped command_tf;
  command_tf.header.stamp = timeStamp;
  command_tf.header.frame_id = "world";
  command_tf.child_frame_id = "command";
  command_tf.transform.translation = getVectorMsg(eeDesiredPosition);
  command_tf.transform.rotation = getOrientationMsg(eeDesiredOrientation);
  tfBroadcaster_.sendTransform(command_tf);
}

void MobileManipulatorDummyVisualization::publishOptimizedTrajectory(const ros::Time& timeStamp, const ocs2::PrimalSolution& policy) {
  const double TRAJECTORYLINEWIDTH = 0.005;
  const std::array<double, 3> red{0.6350, 0.0780, 0.1840};
  const std::array<double, 3> blue{0, 0.4470, 0.7410};
  const auto& mpcStateTrajectory = policy.stateTrajectory_;

  visualization_msgs::MarkerArray markerArray;

  // Base trajectory
  std::vector<geometry_msgs::Point> baseTrajectory;
  baseTrajectory.reserve(mpcStateTrajectory.size());
  geometry_msgs::PoseArray poseArray;
  poseArray.poses.reserve(mpcStateTrajectory.size());

  // End effector trajectory
  std::vector<geometry_msgs::Point> endEffectorTrajectory;
  endEffectorTrajectory.reserve(mpcStateTrajectory.size());
  std::for_each(mpcStateTrajectory.begin(), mpcStateTrajectory.end(), [&](const Eigen::VectorXd& state) {
    const auto pose = pinocchioInterface_.getBodyPoseInWorldFrame("WRIST_2", state);
    endEffectorTrajectory.push_back(getPointMsg(pose.position));
  });

  markerArray.markers.emplace_back(getLineMsg(std::move(endEffectorTrajectory), blue, TRAJECTORYLINEWIDTH));
  markerArray.markers.back().ns = "EE Trajectory";

  // Extract base pose from state
  std::for_each(mpcStateTrajectory.begin(), mpcStateTrajectory.end(), [&](const vector_t& state) {
    geometry_msgs::Pose pose;
    pose.position = getPointMsg(getBasePosition(state));
    pose.orientation = getOrientationMsg(getBaseOrientation(state));
    baseTrajectory.push_back(pose.position);
    poseArray.poses.push_back(std::move(pose));
  });

  markerArray.markers.emplace_back(getLineMsg(std::move(baseTrajectory), red, TRAJECTORYLINEWIDTH));
  markerArray.markers.back().ns = "Base Trajectory";

  assignHeader(markerArray.markers.begin(), markerArray.markers.end(), getHeaderMsg("world", timeStamp));
  assignIncreasingId(markerArray.markers.begin(), markerArray.markers.end());
  poseArray.header = getHeaderMsg("world", timeStamp);

  stateOptimizedPublisher_.publish(markerArray);
  stateOptimizedPosePublisher_.publish(poseArray);
}

}  // namespace mobile_manipulator
