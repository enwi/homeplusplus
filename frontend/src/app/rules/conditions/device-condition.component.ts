import {Component, EventEmitter, OnDestroy, OnInit} from '@angular/core';
import {Observer, Subscription} from 'rxjs';

import {Device} from '../../devices/device';
import {DeviceService} from '../../devices/device.service';
import {RuleCondition} from '../rule';

import {ConditionView} from './condition-view';
import {RuleDeviceCondition} from './rule-conditions';

@Component({
  selector: 'app-device-condition',
  templateUrl: './device-condition.component.html',
  styleUrls: ['./device-condition.component.css']
})
export class DeviceConditionComponent implements OnInit, OnDestroy,
                                                 ConditionView {
  condition: RuleDeviceCondition;

  device: Device;
  sensor: any;
  property: string;
  value: any;
  private subscription: Subscription;

  constructor(private deviceService: DeviceService) {}

  ngOnInit() {}
  ngOnDestroy() {
    if (this.subscription) {
      this.subscription.unsubscribe();
    }
  }

  initialize(c: RuleCondition) {
    this.condition = c as RuleDeviceCondition;
    this.subscription = this.deviceService.getDevice(this.condition.deviceId)
                            .subscribe(d => this.updateDevice(d));
  }

  updateCondition(c: RuleCondition) {
    let sensorCondition = c as RuleDeviceCondition;
    // Change subscription if device changed
    if (this.condition.deviceId !== sensorCondition.deviceId) {
      this.subscription.unsubscribe();
      this.subscription = this.deviceService.getDevice(sensorCondition.deviceId)
                              .subscribe(d => this.updateDevice(d));
    }
    this.condition = sensorCondition;
  }

  updateDevice(d: Device) {
    this.device = d;
    if (this.device && this.device.properties.has(this.condition.property)) {
      this.property = this.condition.property;
      this.value = this.device.properties.get(this.property);
    } else {
      this.property = undefined;
    }
  }
  getComparisonString(): string {
    switch (this.condition.compare) {
      case 0:
        // equals
        if (this.condition.value2 !== 0) {
          return `ist gleich ${this.condition.value1} +/- ${
              this.condition.value2}`;
        } else {
          return `ist gleich ${this.condition.value1}`;
        }
      case 1:
        // not equals
        if (this.condition.value2 !== 0) {
          return `ist nicht gleich ${this.condition.value1} +/- ${
              this.condition.value2}`;
        } else {
          return `ist nicht gleich ${this.condition.value1}`;
        }
      case 2:
        // greater
        return `ist größer als ${this.condition.value1}`;
      case 3:
        // less
        return `ist kleiner als ${this.condition.value1}`
        case 4:
            // in range
            return `ist zwischen ${this.condition.value1} und ${
                this.condition.value2}`;
    }
    return 'unbekannt';
  }
}
