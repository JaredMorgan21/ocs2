/*
 * OCS2AnymalInterface.cpp
 *
 *  Created on: May 11, 2017
 *      Author: farbod
 */

//config file loading
#include "ocs2_anymal_interface/OCS2AnymalInterface.h"

namespace anymal {
/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
/**
 *
 * @param time
 * @param hyqState
 * @param comPose
 * @param comVelocities
 */
void OCS2AnymalInterface::fromHyqStateToComStateOrigin(const double& time,
		const Eigen::Matrix<double,36,1>& hyqState,
		com_coordinate_t& comPose,
		com_coordinate_t& comVelocities)  {

	// rotation matrix
	Eigen::Matrix3d b_R_o = switched_model::RotationMatrixOrigintoBase(hyqState.head<3>());
	// switched hyq state
	dimension_t::state_vector_t switchedState;
	switchedModelStateEstimator_.estimateComkinoModelState(hyqState, switchedState);
	comPose = switchedState.segment<6>(0);
	com_coordinate_t comLocalVelocities = switchedState.segment<6>(6);
	comVelocities.head<3>() = b_R_o.transpose() * comLocalVelocities.head<3>();
	comVelocities.tail<3>() = b_R_o.transpose() * comLocalVelocities.tail<3>();
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void OCS2AnymalInterface::adjustFootZdirection(
		const double& time, std::array<Eigen::Vector3d,4>& origin_base2StanceFeet, std::array<bool,4>& stanceLegSequene) {

//	size_t activeSubsystemIndexInStock = findActiveSubsystemIndex(time);
//	stanceLegSequene = switched_model::modeNumber2StanceLeg(initSwitchingModes_[activeSubsystemIndexInStock]);
//
//	for (size_t j=0; j<4; j++) {
//		if (stanceLegSequene[j]==true)
//			origin_base2StanceFeet[j] = origin_base2StanceFeetPrev_[j];
//		else
//			origin_base2StanceFeet[j](2) = plannedCPGs_[activeSubsystemIndexInStock][j]->calculatePosition(time);
//
//		origin_base2StanceFeetPrev_[j] = origin_base2StanceFeet[j];
//	}  // end of j loop
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void OCS2AnymalInterface::getPerformanceIndeces(double& costFunction, double& constriantISE1, double& constriantISE2) const {
	costFunction = costFunction_;
	constriantISE1 = constriantISE1_;
	constriantISE2 = constriantISE2_;
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void OCS2AnymalInterface::getSwitchingTimes(std::vector<double>& switchingTimes) const {
	switchingTimes = switchingTimes_;
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void OCS2AnymalInterface::getController(dimension_t::controller_array_t& controllersStock) const {
	controllersStock = controllersStock_;
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
std::shared_ptr<const OCS2AnymalInterface::dimension_t::controller_array_t> OCS2AnymalInterface::getControllerPtr() const {
	return controllersStockPtr_;
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
std::shared_ptr<const std::vector<OCS2AnymalInterface::dimension_t::scalar_array_t>> OCS2AnymalInterface::getTimeTrajectoriesPtr() const {
	return timeTrajectoriesStockPtr_;
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
std::shared_ptr<const OCS2AnymalInterface::dimension_t::state_vector_array2_t> OCS2AnymalInterface::getStateTrajectoriesPtr() const {
	return stateTrajectoriesStockPtr_;
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
std::shared_ptr<const OCS2AnymalInterface::dimension_t::control_vector_array2_t> OCS2AnymalInterface::getInputTrajectoriesPtr() const {
	return inputTrajectoriesStockPtr_;
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void OCS2AnymalInterface::getTrajectories(std::vector<dimension_t::scalar_array_t>& timeTrajectoriesStock,
		dimension_t::state_vector_array2_t& stateTrajectoriesStock,
		dimension_t::control_vector_array2_t& inputTrajectoriesStock) const {
	timeTrajectoriesStock  = timeTrajectoriesStock_;
	stateTrajectoriesStock = stateTrajectoriesStock_;
	inputTrajectoriesStock = inputTrajectoriesStock_;
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void OCS2AnymalInterface::getStanceLegSequene(std::vector<std::array<bool,4>>& stanceLegSequene) const {
	stanceLegSequene = stanceLegSequene_;
}


/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void OCS2AnymalInterface::getSwitchingModeSequence(std::vector<size_t>& switchingModes) const {
	switchingModes = switchingModes_;
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void OCS2AnymalInterface::getGapIndicatorPtrs(std::vector<switched_model::EndEffectorConstraintBase::ConstPtr>& gapIndicatorPtrs) const {
	gapIndicatorPtrs = gapIndicatorPtrs_;
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void OCS2AnymalInterface::getMpcOptions(ocs2::MPC_Settings& mpcOptions){
	mpcOptions = mpcSettings_;
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void OCS2AnymalInterface::concatenate()  {

	timeTrajectory_  = timeTrajectoriesStock_[0];
	stateTrajectory_ = stateTrajectoriesStock_[0];
	inputTrajectory_  = inputTrajectoriesStock_[0];
	for(size_t i=1; i<numSubsystems_; i++) {
		timeTrajectory_.insert(timeTrajectory_.end(), timeTrajectoriesStock_[i].begin(), timeTrajectoriesStock_[i].end());
		stateTrajectory_.insert(stateTrajectory_.end(), stateTrajectoriesStock_[i].begin(), stateTrajectoriesStock_[i].end());
		inputTrajectory_.insert(inputTrajectory_.end(), inputTrajectoriesStock_[i].begin(), inputTrajectoriesStock_[i].end());
	}

	controllerTimeTrajectory_ = controllersStock_[0].time_;
	controllerFBTrajectory_   = controllersStock_[0].k_;
	controllerFFTrajector_    = controllersStock_[0].uff_;
	for(size_t i=1; i<numSubsystems_; i++) {
		controllerTimeTrajectory_.insert(controllerTimeTrajectory_.end(), controllersStock_[i].time_.begin(), controllersStock_[i].time_.end());
		controllerFBTrajectory_.insert(controllerFBTrajectory_.end(), controllersStock_[i].k_.begin(), controllersStock_[i].k_.end());
		controllerFFTrajector_.insert(controllerFFTrajector_.end(), controllersStock_[i].uff_.begin(), controllersStock_[i].uff_.end());
	}

	linInterpolateState_.setTimeStamp(&timeTrajectory_);
	linInterpolateState_.setData(&stateTrajectory_);

	linInterpolateInput_.setTimeStamp(&timeTrajectory_);
	linInterpolateInput_.setData(&inputTrajectory_);

	linInterpolateUff_.setTimeStamp(&controllerTimeTrajectory_);
	linInterpolateUff_.setData(&controllerFFTrajector_);

	linInterpolateK_.setTimeStamp(&controllerTimeTrajectory_);
	linInterpolateK_.setData(&controllerFBTrajectory_);
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void OCS2AnymalInterface::runSLQ(const double& initTime,
		const Eigen::Matrix<double,36,1>& initHyQState,
		const dimension_t::controller_array_t& initialControllersStock/*=dimension_t::controller_array_t()*/,
		const std::vector<double>& switchingTimes/*=std::vector<double>()*/)  {

	initTime_ = initTime;
	switchedModelStateEstimator_.estimateComkinoModelState(initHyQState, initSwitchedState_);

	if (switchingTimes.empty()==true)
		switchingTimes_ = initSwitchingTimes_;
	else
		switchingTimes_ = switchingTimes;

	dimension_t::scalar_array_t costAnnealingStartTimes(numSubsystems_, switchingTimes_.front());
	dimension_t::scalar_array_t costAnnealingFinalTimes(numSubsystems_, switchingTimes_.back());

	// run slqp
	if (initialControllersStock.empty()==true) {
		std::cerr << "Cold initialization." << std::endl;
		controllersStock_.clear();
		slqPtr_->run(initTime_, initSwitchedState_, finalTime_, partitioningTimes_,
				desiredTimeTrajectoriesStock_, desiredStateTrajectoriesStock_);

	} else {
		std::cerr << "Warm initialization." << std::endl;
		controllersStock_ = initialControllersStock;
		slqPtr_->run(initTime_, initSwitchedState_, finalTime_, partitioningTimes_,
				controllersStock_, desiredTimeTrajectoriesStock_, desiredStateTrajectoriesStock_);
	}


	// get the optimizer parametet
//	getOptimizerParameters(slqPtr_);

	// get the optimizer outputs
	slqPtr_->getIterationsLog(iterationCost_, iterationISE1_, iterationISE2_);
	ocs2Iterationcost_.clear();
	slqPtr_->getPerformanceIndeces(costFunction_, constriantISE1_, constriantISE2_);
	slqPtr_->getSwitchingTimes(switchingTimes_);
	slqPtr_->getController(controllersStock_);
	slqPtr_->getNominalTrajectories(timeTrajectoriesStock_, stateTrajectoriesStock_, inputTrajectoriesStock_);

//	concatenate();

//	hyq::SwitchedModelKinematics::FeetPositionsBaseFrame(initSwitchedState_.tail<12>(), origin_base2StanceFeetPrev_);
//	Eigen::Matrix3d o_R_b = hyq::SwitchedModelKinematics::RotationMatrixOrigintoBase(initSwitchedState_.head<3>()).transpose();
//	for (size_t j=0; j<4; j++) origin_base2StanceFeetPrev_[j] = (o_R_b * origin_base2StanceFeetPrev_[j] + initHyQState.segment<3>(3)).eval();
//
//	feetZDirectionPlannerPtr_->planAllModes(switchingTimes_, plannedCPGs_);
}


/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
template<class T>
void OCS2AnymalInterface::getOptimizerParameters(const std::shared_ptr<T>& optimizerPtr) {

	optimizerPtr->getSwitchingTimes(switchingTimes_);
	numSubsystems_ = switchingTimes_.size() + 1;

	switchingModes_  = initSwitchingModes_;
	stanceLegSequene_ = initStanceLegSequene_;
}


/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
bool OCS2AnymalInterface::runMPC(const double& initTime, const Eigen::Matrix<double,36,1>& initHyQState)  {

//	initTime_ = initTime;
//	switchedModelStateEstimator_.estimateComkinoModelState(initHyQState, initSwitchedState_);
//
//	// update controller
//	bool controllerIsUpdated = mpcPtr_->run(initTime_, initSwitchedState_);
//
//	// get the optimizer outputs
//	controllersStockPtr_ = mpcPtr_->getControllerPtr();
//
//	// get the optimizer outputs
//	timeTrajectoriesStockPtr_ = mpcPtr_->getTimeTrajectoriesPtr();
//	stateTrajectoriesStockPtr_ = mpcPtr_->getStateTrajectoriesPtr();
//	inputTrajectoriesStockPtr_ = mpcPtr_->getInputTrajectoriesPtr();
//
//	// get the optimizer parametet: switchingTimes_, systemStockIndexes_,
//	// numSubsystems_, switchingModes_, and stanceLegSequene_
//	getOptimizerParameters(mpcPtr_);
//
//	return controllerIsUpdated;
}


/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
bool OCS2AnymalInterface::runMPC(const double& initTime, const dimension_t::state_vector_t& initHyQState)  {

//	initTime_ = initTime;
//	initSwitchedState_ = initHyQState;
//
//	// update controller
//	bool controllerIsUpdated = mpcPtr_->run(initTime_, initSwitchedState_);
//
//	// get the optimizer outputs
//	mpcPtr_->getController(controllersStock_);
//
//	// get the optimizer outputs
//	mpcPtr_->getTrajectories(timeTrajectoriesStock_, stateTrajectoriesStock_, inputTrajectoriesStock_);
//
//	// get the optimizer parametet: switchingTimes_, systemStockIndexes_,
//	// numSubsystems_, switchingModes_, and stanceLegSequene_
//	getOptimizerParameters(mpcPtr_);
//
//	return controllerIsUpdated;
}


/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void OCS2AnymalInterface::setNewGoalStateMPC(const dimension_t::scalar_t& newGoalDuration,
		const dimension_t::state_vector_t& newGoalState) {

//	mpcPtr_->setNewGoalState(newGoalDuration, newGoalState);
}


/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void OCS2AnymalInterface::runOCS2(const double& initTime,
		const Eigen::Matrix<double,36,1>& initHyQState,
		const std::vector<double>& switchingTimes/*=std::vector<double>()*/)  {

//	if (switchingTimes.empty()==true)
//		switchingTimes_ = initSwitchingTimes_;
//	else
//		switchingTimes_ = switchingTimes;
//
//	initTime_ = initTime;
//	switchedModelStateEstimator_.estimateComkinoModelState(initHyQState, initSwitchedState_);
//
//	// run ocs2
//	ocs2Ptr_->run(initTime_, initSwitchedState_, switchingTimes_.back(), initSystemStockIndexes_, switchingTimes_,
//			dimension_t::controller_array_t(),
//			desiredTimeTrajectoriesStock_, desiredStateTrajectoriesStock_);
//
//	// get the optimizer outputs
//	ocs2Ptr_->getOCS2IterationsLog(ocs2Iterationcost_);
//	ocs2Ptr_->getSLQIterationsLog(iterationCost_, iterationISE1_);
//	ocs2Ptr_->getCostFunction(costFunction_);
//	constriantISE_ = iterationISE1_.back()(0);
//	ocs2Ptr_->getSwitchingTimes(switchingTimes_);
//	ocs2Ptr_->getController(controllersStock_);
//	ocs2Ptr_->getNominalTrajectories(timeTrajectoriesStock_, stateTrajectoriesStock_, inputTrajectoriesStock_);

}


/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void OCS2AnymalInterface::loadSimulationSettings(const std::string& filename, double& dt, double& tFinal, double& initSettlingTime)
{
	const double defaultInitSettlingTime = 1.5;
	boost::property_tree::ptree pt;

	try	{
		boost::property_tree::read_info(filename, pt);
		dt     = pt.get<double>("simulationSettings.dt");
		tFinal = pt.get<double>("simulationSettings.tFinal");
		initSettlingTime = pt.get<double>("simulationSettings.initSettlingTime", defaultInitSettlingTime);
	}
	catch (const std::exception& e){
		std::cerr << "Tried to open file " << filename << " but failed: " << std::endl;
		std::cerr<<"Error in loading simulation settings: " << e.what() << std::endl;
		throw;
	}
	std::cerr<<"Simulation Settings: " << std::endl;
	std::cerr<<"=====================================" << std::endl;
	std::cerr<<"Simulation time step ......... " << dt << std::endl;
	std::cerr<<"Controller simulation time ... [0, " << tFinal  << "]" << std::endl;
	std::cerr<<"initial settling time ......... " << initSettlingTime  << std::endl << std::endl;

}


/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void OCS2AnymalInterface::loadVisualizationSettings(const std::string& filename, double& slowdown, double& vizTime)
{
	boost::property_tree::ptree pt;

	try	{
		boost::property_tree::read_info(filename, pt);
		slowdown = pt.get<double>("visualization.slowdown");
		vizTime = pt.get<double>("visualization.vizTime");
	}
	catch (const std::exception& e){
		std::cerr<<"Error in loading viz settings: " << e.what() << std::endl;
	}
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void OCS2AnymalInterface::loadSettings(const std::string& pathToConfigFile) {

	// load SLQ settings
	slqSettings_.loadSettings(pathToConfigFile, true);

	// load MPC settings
	mpcSettings_.loadSettings(pathToConfigFile, true);

	// load switched model settings
	modelSettings_.loadSettings(pathToConfigFile, true);

	std::cerr << std::endl;

	double dt, finalTime, initSettlingTime;
	loadSimulationSettings(pathToConfigFile, dt, finalTime, initSettlingTime);

	initTime_ = 0.0;
	finalTime_ = finalTime-initSettlingTime;

	// initial state of the switched system
	ocs2::LoadConfigFile::loadMatrix(pathToConfigFile, "initial_RBD_state", initRbdState_);
	switchedModelStateEstimator_.estimateComkinoModelState(initRbdState_, initSwitchedState_);

	// cost function components
	ocs2::LoadConfigFile::loadMatrix(pathToConfigFile, "Q", Q_);
	ocs2::LoadConfigFile::loadMatrix(pathToConfigFile, "R", R_);
	ocs2::LoadConfigFile::loadMatrix(pathToConfigFile, "Q_final", QFinal_);
	// target state
	dimension_t::state_vector_t xFinalLoaded;
	ocs2::LoadConfigFile::loadMatrix(pathToConfigFile, "x_final", xFinalLoaded);
	xFinal_ = initSwitchedState_;
	xFinal_.head<6>() += xFinalLoaded.head<6>();

	// load the switchingModes
	switched_model::loadSwitchingModes(pathToConfigFile, initSwitchingModes_, true);
	initNumSubsystems_ = initSwitchingModes_.size();
	// display
	std::cerr << "Initial Switching Modes: {";
	for (const auto& switchingMode: initSwitchingModes_)
		std::cerr << switchingMode << ", ";
	std::cerr << "\b\b}" << std::endl;

	// stanceLeg sequence
	initStanceLegSequene_.resize(initNumSubsystems_);
	for (size_t i=0; i<initNumSubsystems_; i++)  initStanceLegSequene_[i] = switched_model::modeNumber2StanceLeg(initSwitchingModes_[i]);

	// Initial switching times
	size_t NumNonFlyingSubSystems=0;
	for (size_t i=0; i<initNumSubsystems_; i++)
		if (initSwitchingModes_[i] != switched_model::FLY)
			NumNonFlyingSubSystems++;
	initSwitchingTimes_.resize(initNumSubsystems_);
	initSwitchingTimes_.front() = initTime_;
	for (size_t i=0; i<initNumSubsystems_-1; i++)
		if (initSwitchingModes_[i] != switched_model::FLY)
			initSwitchingTimes_[i+1] = initSwitchingTimes_[i] + (finalTime_-initTime_)/NumNonFlyingSubSystems;
		else
			initSwitchingTimes_[i+1] = initSwitchingTimes_[i] + 0.2;
	initSwitchingTimes_.erase(initSwitchingTimes_.begin());
	// display
	std::cerr << "Initial Switching Times: {";
	for (const auto& switchingtime: initSwitchingTimes_)
		std::cerr << switchingtime << ", ";
	if (initSwitchingTimes_.empty()==false)
		std::cerr << "\b\b}" << std::endl;
	else
		std::cerr << "}" << std::endl;

	// partitioning times
	partitioningTimes_.clear();
	partitioningTimes_.push_back(initTime_);
	for (const auto& t : initSwitchingTimes_)
		partitioningTimes_.push_back(t);
	partitioningTimes_.push_back(finalTime_);
	// display
	std::cerr << "Time Partition: {";
	for (const auto& timePartition: partitioningTimes_)
		std::cerr << timePartition << ", ";
	std::cerr << "\b\b}" << std::endl;

	// Gap Indicators
	switched_model::loadGaps(pathToConfigFile, gapIndicatorPtrs_, true);

}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void OCS2AnymalInterface::setupOptimizer()  {


	// for each subsystem that defined in stanceLegSequene
//	dynamicsPtr_.resize(initNumSubsystems_);
//	dynamicsDerivativesPtr_.resize(initNumSubsystems_);
//	constraintsPtr_
//	costFunctionPtr_.resize(initNumSubsystems_);
//	stateOperatingPoints_.resize(initNumSubsystems_);
//	inputOperatingPoints_.resize(initNumSubsystems_);
	controllersStock_.resize(initNumSubsystems_);
//	desiredTimeTrajectoriesStock_.resize(initNumSubsystems_);
//	desiredStateTrajectoriesStock_.resize(initNumSubsystems_);
//	for (size_t i=0; i<initNumSubsystems_; i++) {
//
//		// nominal time
//		dimension_t::scalar_array_t tNominalTrajectory(2);
//		tNominalTrajectory[0] = initSwitchingTimes_.front();
//		tNominalTrajectory[1] = initSwitchingTimes_.back();
//		desiredTimeTrajectoriesStock_[i] = tNominalTrajectory;
//		// nominal state
//		dimension_t::state_vector_array_t xNominalTrajectory(2);
//		xNominalTrajectory[0] = initSwitchedState_;
//		xNominalTrajectory[1] = xFinal_;
//		desiredStateTrajectoriesStock_[i] = xNominalTrajectory;
//		// nominal control inputs for weight compensation
//		dimension_t::control_vector_t uNominalForWeightCompensation;
//		std::array<Eigen::Vector3d,4> sphericalWeightCompensationForces;
//		Eigen::Matrix3d b_R_o = switched_model::RotationMatrixOrigintoBase(initSwitchedState_.head<3>());
//		weightCompensationForces_.ComputeCartesianForces(b_R_o*gravity_, std::array<bool,4>{1,1,1,1}/*initStanceLegSequene_[i]*/, initSwitchedState_.tail<12>(),
//					sphericalWeightCompensationForces);
//		for (size_t j=0; j<4; j++)
//			uNominalForWeightCompensation.segment<3>(3*j) = sphericalWeightCompensationForces[j];
//		uNominalForWeightCompensation.tail<12>().setZero();
//		dimension_t::control_vector_array_t uNominalTrajectory(2, uNominalForWeightCompensation);
//
//		// state and input operating points
//		stateOperatingPoints_[i] = initSwitchedState_;
//		inputOperatingPoints_[i] = uNominalForWeightCompensation;
//		if (modelSettings_.constrainedIntegration_==false)  {
//			inputOperatingPoints_[i].setZero();
//			weightCompensationForces_.ComputeSphericalForces(b_R_o*gravity_, std::array<bool,4>{1,1,1,1}, initSwitchedState_.tail<12>(), sphericalWeightCompensationForces);
//			for (size_t j=0; j<4; j++)  inputOperatingPoints_[i].segment<3>(3*j) = sphericalWeightCompensationForces[j];
//		}
//
//		// R matrix
//		dimension_t::control_matrix_t nondiagonalR = R_;
//		double meanRz = (R_(2,2)+R_(5,5)+R_(8,8)+R_(11,11))/4;
//		for (int j=0; j<4; j++)
//			for (int k=0; k<=j; k++)
//				if (k==j) {
//					nondiagonalR(3*j+0,3*k+0) = 1.05*R_(3*j+0,3*k+0);
//					nondiagonalR(3*j+1,3*k+1) = 1.05*R_(3*j+1,3*k+1);
//					nondiagonalR(3*j+2,3*k+2) = 1.05*meanRz;
//				}
//				else {
//					if (initStanceLegSequene_[i][j] && initStanceLegSequene_[i][k])
//						nondiagonalR(3*j+2,3*k+2) = 2*meanRz;
//					else
//						nondiagonalR(3*j+2,3*k+2) = 0.0;
//				}
//		nondiagonalR = 0.5*(nondiagonalR + nondiagonalR.transpose()).eval();
//
//		subsystemCostFunctionsPtr_[i] = std::shared_ptr<cost_funtion_t>( new cost_funtion_t(initStanceLegSequene_[i], Q_, nondiagonalR,
//				desiredTimeTrajectoriesStock_[i], desiredStateTrajectoriesStock_[i], uNominalTrajectory, QFinal_, xFinal_, modelSettings_.copWeight_) );
//
//		// subsystem settings
//		subsystemDynamicsPtr_[i] = std::shared_ptr<system_dynamics_t>( new system_dynamics_t(initStanceLegSequene_[i], -gravity_(2), modelSettings_,
//				feetZDirectionPlannerPtr_, gapIndicatorPtrs_) );
//		subsystemDerivativesPtr_[i] = std::shared_ptr<system_dynamics_derivative_t>( new system_dynamics_derivative_t(initStanceLegSequene_[i], -gravity_(2), modelSettings_,
//				feetZDirectionPlannerPtr_, gapIndicatorPtrs_) );
//
//	}  // end of i loop

	dynamicsPtr_            = std::shared_ptr<system_dynamics_t>( new system_dynamics_t(modelSettings_) );
	dynamicsDerivativesPtr_ = std::shared_ptr<system_dynamics_derivative_t>( new system_dynamics_derivative_t(modelSettings_) );
	constraintsPtr_  = std::shared_ptr<constraint_t>( new constraint_t(modelSettings_) );
	costFunctionPtr_ = std::shared_ptr<cost_funtion_t>( new cost_funtion_t(Q_, R_, QFinal_, xFinal_, modelSettings_.copWeight_) );

	operating_point_t::generalized_coordinate_t defaultCoordinate = initRbdState_.head<18>();
	operatingPointsPtr_ = std::shared_ptr<operating_point_t>( new operating_point_t(modelSettings_, defaultCoordinate) );

	logicRulesPtr_ = std::shared_ptr<logic_rules_t>( new logic_rules_t(modelSettings_.swingLegLiftOff_, 1.0 /*swingTimeScale*/) );
	logicRulesPtr_->setMotionConstraints(initSwitchingTimes_, initSwitchingModes_, gapIndicatorPtrs_);

	switchingModes_     = initSwitchingModes_;
	stanceLegSequene_   = initStanceLegSequene_;
	switchingTimes_     = initSwitchingTimes_;

}


} // end of namespace anymal
