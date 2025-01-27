#include "Strategy/ShooterStrategy.h"
#include <iostream>

ShooterManualStrategy::ShooterManualStrategy(std::string name, Shooter &shooter, Controllers &contGroup) : Strategy(name), _shooter(shooter), _contGroup(contGroup) {
  SetCanBeInterrupted(true);
  SetCanBeReused(true);
  Requires(&shooter);

}

void ShooterManualStrategy::OnUpdate(double dt) {
  double manualFlyWheelPower = fabs(_contGroup.Get(ControlMap::manualFlyWheel)) > fabs(ControlMap::xboxDeadzone) ? _contGroup.Get(ControlMap::manualFlyWheel) : 0;

  if (_contGroup.Get(ControlMap::innerCircleShoot)) {
    _shooter.setPID(ControlMap::Shooter::innerCircleShootValue, dt);
  } else if (_contGroup.Get(ControlMap::outerCircleShoot)) {
    _shooter.setPID(ControlMap::Shooter::outerCircleShootValue, dt);
  } else if (_contGroup.Get(ControlMap::shooterEject)) {
    _shooter.setManual(-ControlMap::Shooter::shooterEjectPower);
  } else if (_contGroup.Get(ControlMap::farShoot)) {
    _shooter.setPID(ControlMap::Shooter::farShootValue, dt);
  } else if (_contGroup.Get(ControlMap::noahShoot)) {
    _shooter.setPID(ControlMap::Shooter::noahShootValue, dt);
  } else {
    // auto &motor = _shooter._shooterSystem.shooterGearbox.motor;
    _shooter.setManual(manualFlyWheelPower * 12);
  }

  double indexSpeed = _contGroup.Get(ControlMap::indexSpin);
  if (indexSpeed >= 0.1) {
    _shooter.setIndex(0.5);
  } else if (indexSpeed <= -0.1) {
    _shooter.setIndex(-0.5);
  } else {
    _shooter.setIndex(0);
  }
}
