
#include <gtest/gtest.h>

#include <ocs2_core/Types.h>
#include <ocs2_core/constraint/ConstraintBase.h>
#include <ocs2_core/cost/QuadraticCostFunction.h>
#include <ocs2_core/dynamics/LinearSystemDynamics.h>
#include <ocs2_core/initialization/OperatingPoints.h>
#include <ocs2_mpc/MPC_SLQ.h>
#include <ocs2_oc/rollout/TimeTriggeredRollout.h>

#include <ocs2_python_interface/PythonInterface.h>
#include <ocs2_robotic_tools/common/RobotInterface.h>

namespace ocs2 {
namespace pybindings_test {

class DummyInterface final : public RobotInterface {
 public:
  DummyInterface() {
    matrix_t A(2, 2);
    A << 0, 1, 0, 0;
    matrix_t B(2, 1);
    B << 0, 1;
    dynamicsPtr_.reset(new LinearSystemDynamics(A, B));

    matrix_t Q(2, 2), R(1, 1), Qf(2, 2);
    Q << 1, 0, 0, 1;
    R << 1;
    Qf << 2, 0, 0, 2;
    vector_t xNomianl(2), uNominal(2);
    xNomianl.setZero();
    uNominal.setZero();
    costPtr_.reset(new QuadraticCostFunction(Q, R, xNomianl, uNominal, Qf, xNomianl));

    constraintPtr_.reset(new ConstraintBase());

    operatingPointPtr_.reset(new OperatingPoints(vector_t::Zero(2), vector_t::Zero(1)));

    Rollout_Settings rolloutSettings;
    rolloutPtr_.reset(new TimeTriggeredRollout(*dynamicsPtr_, rolloutSettings));
  }
  ~DummyInterface() override = default;

  std::unique_ptr<ocs2::MPC_SLQ> getMpc() {
    SLQ_Settings slqSettings;
    MPC_Settings mpcSettings;
    return std::unique_ptr<MPC_SLQ>(new MPC_SLQ(rolloutPtr_.get(), dynamicsPtr_.get(), constraintPtr_.get(), costPtr_.get(),
                                                operatingPointPtr_.get(), slqSettings, mpcSettings));
  }

  const SystemDynamicsBase& getDynamics() const override { return *dynamicsPtr_; }
  const CostFunctionBase& getCost() const override { return *costPtr_; }
  const ConstraintBase* getConstraintPtr() const override { return constraintPtr_.get(); }
  const OperatingPoints& getOperatingPoints() const override { return *operatingPointPtr_; }

  std::unique_ptr<LinearSystemDynamics> dynamicsPtr_;
  std::unique_ptr<QuadraticCostFunction> costPtr_;
  std::unique_ptr<ConstraintBase> constraintPtr_;
  std::unique_ptr<OperatingPoints> operatingPointPtr_;
  std::unique_ptr<RolloutBase> rolloutPtr_;
};

class DummyPyBindings final : public PythonInterface {
 public:
  using Base = PythonInterface;

  DummyPyBindings(const std::string& taskFileFolder) {
    DummyInterface robot;
    PythonInterface::init(robot, robot.getMpc());
  }
};

}  // namespace pybindings_test
}  // namespace ocs2

TEST(OCS2PyBindingsTest, createDummyPyBindings) {
  ocs2::pybindings_test::DummyPyBindings dummy("mpc");
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
