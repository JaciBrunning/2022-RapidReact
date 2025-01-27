#include "Shooter.h"
#include <iostream>
#include "frc/RobotController.h"

using namespace wml;
using namespace wml::controllers;

Shooter::Shooter(RobotMap::ShooterSystem &shooterSystem, SmartControllerGroup &contGroup) : _shooterSystem(shooterSystem), _contGroup(contGroup) {}

void Shooter::setManual(double voltage) {
  _flyWheelVoltage = voltage;
  _state = ShooterState::kManual;
}

void Shooter::setIndex(double voltage) {
  _setIndexVoltage = voltage;
}

void Shooter::setPID(double goal, double dt) {
  if (_state != ShooterState::kPID) {
    _flyWheelVoltage = 0;
    _sum = 0;
    _previousError = 0;
  }
  _state = ShooterState::kPID;
  _angularVelocityGoal = goal;
}

void Shooter::updateShooter(double dt) {
  switch (_state) {
  case ShooterState::kManual:
    _shooterSystem.indexWheel.Set(_setIndexVoltage);
    break;

  case ShooterState::kIdle:
    _shooterSystem.indexWheel.Set(_setIndexVoltage);
    _flyWheelVoltage = 0;
    break;

  case ShooterState::kPID:
    _flyWheelVoltage = calculatePID(_angularVelocityGoal, dt);
    _shooterSystem.indexWheel.Set(_setIndexVoltage);
    break;

  default:
    _state = ShooterState::kIdle;
    std::cout << "in default case, somthing is wrong" << std::endl;
    break;
  }

  double angularVel = -_shooterSystem.shooterGearbox.encoder->GetEncoderAngularVelocity();
  auto &motor = _shooterSystem.shooterGearbox.motor;
  double Vmax = ControlMap::ShooterGains::IMax * motor.R() + motor.kw() * angularVel;
  double Vmin = -(ControlMap::ShooterGains::IMax) * motor.R() + motor.kw() * angularVel;
  double manualOutput = std::min(_flyWheelVoltage, Vmax);

  nt::NetworkTableInstance::GetDefault().GetTable("shooter gains")->GetEntry("Vout").SetDouble(manualOutput);

  _shooterSystem.shooterGearbox.transmission->SetVoltage(manualOutput);
}

void Shooter::Update(double dt) {
  updateShooter(dt);
}


double Shooter::calculatePID(double angularVelocity, double dt) {
  double input = -(_shooterSystem.shooterGearbox.encoder->GetEncoderAngularVelocity());
  double error = angularVelocity - input;
  double derror = (error - _previousError) / dt;
  _sum += error * dt;

  auto &motor = _shooterSystem.shooterGearbox.motor;
  double output = ControlMap::ShooterGains::kp * error + ControlMap::ShooterGains::ki * _sum + (ControlMap::ShooterGains::kd) * derror +  motor.kw() * angularVelocity;

  auto inst = nt::NetworkTableInstance::GetDefault();
  auto table = inst.GetTable("shooter gains");

  table->GetEntry("input").SetDouble(input);
  table->GetEntry("output").SetDouble(output);
  table->GetEntry("goal").SetDouble(angularVelocity);
  table->GetEntry("error").SetDouble(error);

  table->GetEntry("P").SetDouble(ControlMap::ShooterGains::kp * error);
  table->GetEntry("I").SetDouble(ControlMap::ShooterGains::ki * _sum);
  table->GetEntry("D").SetDouble(ControlMap::ShooterGains::kd * derror);


  _previousError = error;
  return output;
}
