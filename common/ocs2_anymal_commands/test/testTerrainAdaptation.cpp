//
// Created by rgrandia on 04.05.20.
//

#include <gtest/gtest.h>

#include "ocs2_anymal_commands/TerrainAdaptation.h"

using namespace switched_model;

TerrainPlane getRandomTerrain() {
  vector3_t eulerXYZ = vector3_t::Random();
  return {vector3_t::Random(), rotationMatrixOriginToBase(eulerXYZ)};
}

TEST(TestTerrainAdaptation, adaptDesiredOrientationToTerrain_flatTerrain) {
  vector3_t eulerXYZ(0, 0, 0.3);
  TerrainPlane flatTerrain = {vector3_t::Random(), rotationMatrixOriginToBase(eulerXYZ)};

  vector3_t desiredEulerXYZ(0, 0, 0.0);
  const auto adaptedEulerXYZ = adaptDesiredOrientationToTerrain(desiredEulerXYZ, flatTerrain);
  ASSERT_TRUE(desiredEulerXYZ.isApprox(adaptedEulerXYZ));

  vector3_t desiredEulerXYZ_multipleRotations(0, 0, 10.0);
  const auto adaptedEulerXYZ_multipleRotations = adaptDesiredOrientationToTerrain(desiredEulerXYZ_multipleRotations, flatTerrain);
  ASSERT_TRUE(desiredEulerXYZ_multipleRotations.isApprox(adaptedEulerXYZ_multipleRotations));
}

TEST(TestTerrainAdaptation, adaptDesiredOrientationToTerrain_randomTerrain) {
  const auto randomTerrain = getRandomTerrain();
  TerrainPlane canonicalTerrain;

  {  // Zero desired angle
    const vector3_t desiredEulerXYZ(0, 0, 0.0);
    const auto adaptedEulerXYZ = adaptDesiredOrientationToTerrain(desiredEulerXYZ, randomTerrain);
    const vector3_t adaptedXaxis = rotationMatrixBaseToOrigin(adaptedEulerXYZ).col(0);
    const vector3_t projectedAdaptedXaxis = projectPositionInWorldOntoPlaneAlongGravity(adaptedXaxis, canonicalTerrain).normalized();
    ASSERT_DOUBLE_EQ(projectedAdaptedXaxis.x(), 1.0);
  }

  {  // Multi rotation desired angle
    const vector3_t desiredEulerXYZ(0, 0, 10.0);
    const vector3_t desiredXaxis = rotationMatrixBaseToOrigin(desiredEulerXYZ).col(0);
    const auto adaptedEulerXYZ = adaptDesiredOrientationToTerrain(desiredEulerXYZ, randomTerrain);
    const vector3_t adaptedXaxis = rotationMatrixBaseToOrigin(adaptedEulerXYZ).col(0);
    const vector3_t projectedAdaptedXaxis = projectPositionInWorldOntoPlaneAlongGravity(adaptedXaxis, canonicalTerrain).normalized();
    ASSERT_TRUE(desiredXaxis.isApprox(projectedAdaptedXaxis));
  }

  {  // random
    const vector3_t desiredEulerXYZ(0.1, 0.2, -5.0);
    const vector3_t desiredXaxis = rotationMatrixBaseToOrigin(desiredEulerXYZ).col(0);
    const vector3_t projectedDesiredXaxis = projectPositionInWorldOntoPlaneAlongGravity(desiredXaxis, canonicalTerrain).normalized();
    const auto adaptedEulerXYZ = adaptDesiredOrientationToTerrain(desiredEulerXYZ, randomTerrain);
    const vector3_t adaptedXaxis = rotationMatrixBaseToOrigin(adaptedEulerXYZ).col(0);
    const vector3_t projectedAdaptedXaxis = projectPositionInWorldOntoPlaneAlongGravity(adaptedXaxis, canonicalTerrain).normalized();
    ASSERT_TRUE(projectedDesiredXaxis.isApprox(projectedAdaptedXaxis));
  }

  {  // Debug : previously produced euler angles around pi, pi, pi
    const vector3_t positionInWorld(-0.00807168, 0.00535307, 0.00841279);
    const vector3_t surfaceNormal(-8.46512e-06, 2.99018e-05, 1);
    TerrainPlane debugTerrain{positionInWorld, orientationWorldToTerrainFromSurfaceNormalInWorld(surfaceNormal)};
    const vector3_t desiredEulerXYZ(-0.000508991, -0.000493831, 0.000299875);
    const vector3_t desiredXaxis = rotationMatrixBaseToOrigin(desiredEulerXYZ).col(0);
    const vector3_t projectedDesiredXaxis = projectPositionInWorldOntoPlaneAlongGravity(desiredXaxis, canonicalTerrain).normalized();
    const auto adaptedEulerXYZ = adaptDesiredOrientationToTerrain(desiredEulerXYZ, debugTerrain);
    const vector3_t adaptedXaxis = rotationMatrixBaseToOrigin(adaptedEulerXYZ).col(0);
    const vector3_t projectedAdaptedXaxis = projectPositionInWorldOntoPlaneAlongGravity(adaptedXaxis, canonicalTerrain).normalized();
    ASSERT_TRUE(projectedDesiredXaxis.isApprox(projectedAdaptedXaxis));

    //    [Ocs2MotionPlanner::advanceOcs2Terrain] positionInWorld:
    //    [Ocs2MotionPlanner::advanceOcs2Terrain] surfaceNormal: -8.46512e-06  2.99018e-05            1
    //    [Ocs2MotionPlanner::advanceOcs2Terrain] basePositionDesired: -0.00807614  0.00536881    0.543053
    //    [Ocs2MotionPlanner::advanceOcs2Terrain] baseOrientationReference:
    //    [Ocs2MotionPlanner::advanceOcs2Terrain] baseOrientationDesired:  3.14156 -3.14158 -3.14129
  }
}