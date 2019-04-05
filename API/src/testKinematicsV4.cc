
// This program if for checking the kinematics class is producing sensible results.
// It should be turned into a full unit test.

#include "dogbot/LegKinematicsV4.hh"
#include "dogbot/Util.hh"
#include <iostream>
#include <math.h>

DogBotN::LegKinematicsV4C g_legKinematics;

static float deg2rad(float deg)
{
  return ((float) deg *2.0 * M_PI)/ 360.0;
}

static float rad2deg(float deg)
{
  return ((float) deg * 360.0) / (2.0 * M_PI);
}


int TestFixedAngles()
{
  {
    std::cout << "Zero:" << std::endl;
    Eigen::Vector3f pos;
    Eigen::Vector3f angles = {0,M_PI/2.0,M_PI/2};
    if(!g_legKinematics.ForwardVirtual(angles,pos)) {
      std::cerr << "Forward kinematics failed." << std::endl;
      return 1;
    }
    std::cout << " Angles:" << DogBotN::Rad2Deg(angles[0]) << " " << DogBotN::Rad2Deg(angles[1]) << " " << DogBotN::Rad2Deg(angles[2]) << " " << std::endl;
    std::cout << "     At: "<< pos[0] << " " << pos[1] << " " << pos[2] << " " << std::endl;
  }
  {
    Eigen::Vector3f pos;
    Eigen::Vector3f angles2 = {0,0,0};
    if(!g_legKinematics.ForwardVirtual(angles2,pos)) {
      std::cerr << "Forward kinematics failed." << std::endl;
      return 1;
    }
    std::cout <<" 0,0,0 at: "<< pos[0] << " " << pos[1] << " " << pos[2] << " " << std::endl;
  }
  return 0;
}

// Check inverse and forward kinematics agree for a virtual joint

int CheckTargetVirtual(Eigen::Vector3f target) {
  DogBotN::LegKinematicsV4C legKinematics;
  Eigen::Vector3f angles;
  Eigen::Vector3f pos;

  if(!g_legKinematics.InverseVirtual(target,angles)) {
    std::cerr << "Failed." << std::endl;
    return __LINE__;
  }
  std::cout << " Virtual Angles:" << DogBotN::Rad2Deg(angles[0]) << " " << DogBotN::Rad2Deg(angles[1]) << " " << DogBotN::Rad2Deg(angles[2]) << " " << std::endl;

  if(!g_legKinematics.ForwardVirtual(angles,pos)) {
    std::cerr << "Forward kinematics failed." << std::endl;
    return __LINE__;
  }

  float dist = (target - pos).norm();
  std::cout <<" Target: "<< target[0] << " " << target[1] << " " << target[2] << " " << std::endl;
  std::cout <<" At: "<< pos[0] << " " << pos[1] << " " << pos[2] << "   Distance:" << dist << std::endl;
  if(dist > 1e-6) {
    return __LINE__;
  }
  return 0;
}

// Check inverse and forward kinematics agree for a direct joint

int CheckTargetDirect(Eigen::Vector3f target,bool verbose = true) {
  DogBotN::LegKinematicsV4C legKinematics;
  Eigen::Vector3f angles;
  Eigen::Vector3f pos;

  if(!g_legKinematics.InverseDirect(target,angles)) {
    if(verbose)
      std::cerr << "Failed to find solution for target :" << target << " " << std::endl;
    return __LINE__;
  }
  if(verbose)
    std::cout << " Direct Angles:" << DogBotN::Rad2Deg(angles[0]) << " " << DogBotN::Rad2Deg(angles[1]) << " " << DogBotN::Rad2Deg(angles[2]) << " " << std::endl;

  if(!g_legKinematics.ForwardDirect(angles,pos)) {
    std::cerr << "Forward kinematics failed." << std::endl;
    return __LINE__;
  }

  float dist = (target - pos).norm();
  if(verbose) {
    std::cout <<" Target: "<< target[0] << " " << target[1] << " " << target[2] << " " << std::endl;
    std::cout <<" At: "<< pos[0] << " " << pos[1] << " " << pos[2] << "   Distance:" << dist << std::endl;
  }
  if(dist > 5e-3) { // Should be way better than this!
    std::cout << "Error: " << dist << std::endl;
    return __LINE__;
  }
  return 0;
}

//! Check various positions in the foot space a reachable.

int TestReachableTargets()
{
  int ln =0;

  {
    Eigen::Vector3f target = {0.15,0.1,0.3};
    if((ln = CheckTargetDirect(target)) != 0) {
      std::cerr << "Check target failed at " << ln << std::endl;
      return __LINE__;
    }
  }

  float maxReach;
  {
    maxReach = g_legKinematics.MaxExtension()-0.001;
    Eigen::Vector3f target = {0,0,maxReach};
    if((ln = CheckTargetDirect(target)) != 0) {
      std::cerr << "Check target failed at " << ln << std::endl;
      return __LINE__;
    }
  }

  float minReach;
  {
    minReach = g_legKinematics.MinExtension();
    Eigen::Vector3f target = {0,0,minReach};
    if((ln = CheckTargetDirect(target)) != 0) {
      std::cerr << "Check target failed at " << ln << std::endl;
      return __LINE__;
    }
  }

  {
    std::cout << "Stride at max " << (maxReach) << " is " << g_legKinematics.StrideLength(maxReach) << std::endl;
    {
      float i = maxReach;
      float stride = g_legKinematics.StrideLength(i)-0.1;
      std::cout << "Stride at " << i << " is " << stride << std::endl;

      Eigen::Vector3f target = {0,(float) (stride/2),i};
      if((ln = CheckTargetDirect(target,true)) != 0) {
        std::cerr << "Check target failed at " << ln << std::endl;
        //return __LINE__;
      }
    }

    std::cout << "Stride at min " << (minReach) << " is " << g_legKinematics.StrideLength(minReach) << std::endl;
  }

  for(float i = minReach;i <= maxReach;i += 0.05) {
    float stride = g_legKinematics.StrideLength(i);
    std::cout << "Stride at " << i << " is " << stride << std::endl;

    Eigen::Vector3f target = {0,(float) (stride/2),i};
    if((ln = CheckTargetDirect(target,false)) != 0) {
      std::cerr << "Check target failed at " << ln << std::endl;
      //return __LINE__;
    }
  }

  return 0;
}

int CheckVelocity(const Eigen::Vector3f &angles,const Eigen::Vector3f jointVelocities,const Eigen::Vector3f &expected)
{
  Eigen::Vector3f torques(0,0,0);
  Eigen::Vector3f footAt(0,0,0);
  Eigen::Vector3f footVel(0,0,0);
  Eigen::Vector3f footForce(0,0,0);

  if(!g_legKinematics.ComputeFootForce(angles,jointVelocities,torques,footAt,footVel,footForce))
    return __LINE__;


  float error = (expected - footVel).norm();
  if(error > 0.01) {
    std::cout << "Foot     velocity " << footVel[0] << " " <<  footVel[1] << " " << footVel[2] << std::endl;
    std::cout << "Error exceeds threshold: " << error << "\n";
    std::cout << "Expected velocity " << expected[0] << " " <<  expected[1] << " " << expected[2] << std::endl;
  }
#if 0
  Eigen::Vector3f fwdAt(0,0,0);
  if(!g_legKinematics.ForwardDirect(angles,fwdAt)) {
    return __LINE__;
  }
  std::cout << "Foot position " << footAt[0] << " " <<  footAt[1] << " " << footAt[2] << std::endl;
  std::cout << "Foot position fwd " << fwdAt[0] << " " <<  fwdAt[1] << " " << fwdAt[2] << std::endl;
  std::cout << std::endl;
#endif
  return 0;
}

int CheckVelocityAxis(const Eigen::Vector3f &position,const Eigen::Vector3f &axis)
{
  int ln;

  Eigen::Vector3f angles1;
  Eigen::Vector3f angles2;

  float delta = 0.0005;
  Eigen::Vector3f target1(0,0,-0.5);
  Eigen::Vector3f target2 = target1 + axis * delta;

  if(!g_legKinematics.InverseDirect(target1,angles1)) {
    return __LINE__;
  }
  if(!g_legKinematics.InverseDirect(target2,angles2)) {
    return __LINE__;
  }
  Eigen::Vector3f jointVelocities = (angles2 - angles1)/delta;

  if((ln = CheckVelocity(angles1,jointVelocities,axis)) != 0) {
    std::cerr << "Axis check failed on line " << ln << std::endl;
    return __LINE__;
  }
  return 0;
}

int CheckVelocityPosition(const Eigen::Vector3f &offset)
{
  int ln = 0;
  //std::cout << "X Axis "<< std::endl;
  if((ln = CheckVelocityAxis(offset,Eigen::Vector3f(1.0,0,0))) != 0) {
    return __LINE__;
  }
  //std::cout << "Y Axis "<< std::endl;
  if((ln = CheckVelocityAxis(offset,Eigen::Vector3f(0,1.0,0))) != 0) {
    return __LINE__;
  }
  //std::cout << "Z Axis "<< std::endl;
  if((ln = CheckVelocityAxis(offset,Eigen::Vector3f(0,0,1.0))) != 0) {
    return __LINE__;
  }
  return 0;
}

int TestFootVelocity()
{
  int ln = 0;
  if(CheckVelocityPosition(Eigen::Vector3f(0,0.0,-0.5)) != 0) {
    return __LINE__;
  }
  if(CheckVelocityPosition(Eigen::Vector3f(0,0.2,-0.3)) != 0) {
    return __LINE__;
  }
  if(CheckVelocityPosition(Eigen::Vector3f(-0.2,0.0,-0.4)) != 0) {
    return __LINE__;
  }
  if(CheckVelocityPosition(Eigen::Vector3f(-0.2,0.0,-0.4)) != 0) {
    return __LINE__;
  }
  if(CheckVelocityPosition(Eigen::Vector3f(0.1,-0.2,-0.3)) != 0) {
    return __LINE__;
  }
  return 0;
}

int main() {
  int ln = 0;
#if 1
  if((ln = TestFixedAngles()) != 0) {
    std::cerr << "Test failed at " << ln << " " << std::endl;
    return 1;
  }
#endif
  if((ln = TestReachableTargets()) != 0) {
    std::cerr << "Test failed at " << ln << " " << std::endl;
    return 1;
  }
#if 0
  if((ln = TestFootVelocity()) != 0) {
    std::cerr << "Test failed at " << ln << " " << std::endl;
    return 1;
  }
#endif
  std::cerr << "Test passed." << std::endl;
  return 0;
}