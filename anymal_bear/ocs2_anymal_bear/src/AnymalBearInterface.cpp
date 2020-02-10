/*
 * AnymalBearInterface.cpp
 *
 *  Created on: May 11, 2017
 *      Author: farbod
 */

#include "ocs2_anymal_bear/AnymalBearInterface.h"

#include <ocs2_anymal_bear_switched_model/core/AnymalCom.h>
#include <ocs2_anymal_bear_switched_model/core/AnymalKinematics.h>

namespace anymal {
/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
AnymalBearInterface::AnymalBearInterface(const std::string& pathToConfigFolder)
    : BASE(AnymalKinematics(), AnymalCom(), pathToConfigFolder) {
  dynamicsPtr_.reset(new system_dynamics_t(AnymalKinematicsAd(), AnymalComAd(), modelSettings_.recompileLibraries_));
  dynamicsDerivativesPtr_.reset(dynamicsPtr_->clone());
  constraintsPtr_.reset(new constraint_t(AnymalKinematicsAd(), AnymalComAd(), logicRulesPtr_, modelSettings_));
  costFunctionPtr_.reset(new cost_function_t(AnymalCom(), logicRulesPtr_, Q_, R_, QFinal_));
  operatingPointsPtr_.reset(new operating_point_t(AnymalCom(), logicRulesPtr_));
  timeTriggeredRolloutPtr_.reset(new time_triggered_rollout_t(*dynamicsPtr_, rolloutSettings_));
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
std::unique_ptr<AnymalBearInterface::mpc_t> AnymalBearInterface::getMpc() const {
  if (!modelSettings_.gaitOptimization_) {
    return std::unique_ptr<mpc_t>(
        new mpc_t(timeTriggeredRolloutPtr_.get(), dynamicsDerivativesPtr_.get(), constraintsPtr_.get(), costFunctionPtr_.get(),
                  operatingPointsPtr_.get(), partitioningTimes_, slqSettings_, mpcSettings_, logicRulesPtr_, &defaultModeSequenceTemplate_));
  } else {
    throw std::runtime_error("mpc_ocs2 not configured, set gait optimization to 0");
  }
}

}  // end of namespace anymal
