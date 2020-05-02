import {Component, OnDestroy, OnInit} from '@angular/core';
import {FormBuilder, FormGroup, Validators} from '@angular/forms';
import {Meta} from '@angular/platform-browser';
import {Subscription} from 'rxjs';

import {Device} from '../../devices/device';
import {DeviceService} from '../../devices/device.service';
import {SubAction} from '../action';
import {PropertySet} from '../sub-actions/sub-actions';

import {PropertyListContainer, PropertyListController} from './property-list-container';
import {TypeForm} from './type-form';

@Component({
  selector: 'app-edit-property-set',
  templateUrl: './edit-property-set.component.html',
  styleUrls: ['./edit-property-set.component.css']
})
export class EditPropertySetComponent implements OnInit, OnDestroy, TypeForm,
                                                 PropertyListController {
  subAction: PropertySet;
  private subscription: Subscription;
  form: FormGroup;
  container: PropertyListContainer;
  property: string;
  value: any;
  valueInput: string;

  constructor(private deviceService: DeviceService, private fb: FormBuilder) {
    this.container = new PropertyListContainer(this.deviceService, this);
  }

  ngOnInit() {}
  ngOnDestroy() {
    this.container.ngOnDestroy();
  }

  setSubAction(subAction: SubAction) {
    if (typeof (subAction) === typeof (PropertySet)) {
      this.subAction = subAction as PropertySet;
    } else {
      this.subAction = new PropertySet();
      if (subAction) {
        Object.assign(this.subAction, subAction);
      }
    }
    this.container.initialize();
    if (!this.form) {
      this.form = this.fb.group({
        deviceId: [this.subAction.deviceId, Validators.required],
        property: [this.subAction.property, Validators.required],
        value: [this.subAction.value, Validators.required]
      });
      this.form.get('deviceId')
          .valueChanges.subscribe((value) => this.onDeviceChange(value));
      this.form.get('property')
          .valueChanges.subscribe((value) => this.onPropertyChange(value));
    } else {
      this.form.setValue({
        deviceId: this.subAction.deviceId,
        property: this.subAction.property,
        value: this.subAction.value
      });
    }
  }

  onDeviceChange(value: any): void {
    if (this.container.devices) {
      const device = this.container.findDevice(+value);
      if (device) {
        const obs =
            this.deviceService.getDeviceMeta(device.type).subscribe(meta => {
              this.container.updateProperties(device.properties, meta);
              obs.unsubscribe();
            });
      } else {
        this.container.updateProperties(undefined, undefined);
      }
    }
  }
  onPropertyChange(key: string): void {
    if (this.container.properties) {
      this.property = key;
      this.value = this.container.properties[this.property];
      // none,
      // // Custom: custom protobuf message
      // custom
      if (this.container.meta.entries &&
          this.container.meta.entries.has(this.property)) {
        switch (this.container.meta.entries.get(this.property).dataType) {
          case 1:  // boolean
            this.valueInput = 'checkbox';
            break;
          case 2:  // integer
            this.valueInput = 'number';
            break;
          case 3:  // floatingPoint
            this.valueInput = 'number';
            break;
          case 4:  // string
            this.valueInput = 'text';
            break;
          default:
            break;
        }
      }

    } else {
      this.property = key;
      this.value = undefined;
      this.valueInput = undefined;
    }
  }

  getSelectedDevice(): Device {
    if (this.form.value.deviceId === undefined) {
      return undefined;
    }
    return this.container.findDevice(+this.form.value.deviceId);
  }

  getSubAction(): SubAction {
    if (!this.form.valid) {
      return undefined;
    }
    this.subAction.deviceId = this.form.value.deviceId;
    this.subAction.property = this.form.value.property;
    switch (
        this.container.meta.entries.get(this.form.value.property).dataType) {
      case 1:
        // Checkbox
        this.subAction.value = this.getBoolean(this.form.value.value);
        break;
      case 2:
        // Number
        this.subAction.value = +this.form.value.value;
        break;
      case 3:
        // Number
        this.subAction.value = +this.form.value.value;
        break;
      // case 4:
      // Text
      // break;
      default:
        this.subAction.value = this.form.value.value;
        break;
    }
    return this.subAction;
  }

  getBoolean(value): boolean {
    switch (value) {
      case true:
      case 'true':
      case 1:
      case '1':
      case 'on':
      case 'yes':
        return true;
      default:
        return false;
    }
  }

  isValid(): boolean {
    return this.form ? this.form.valid : false;
  }
  getSelectedDeviceId(): number {
    return this.form ? this.form.value.deviceId : undefined;
  }
  getSelectedProperty(): string {
    return this.form ? this.form.value.property : undefined;
  }
  resetProperty(): void {
    this.form.get('property').setValue(undefined);
  }
}
