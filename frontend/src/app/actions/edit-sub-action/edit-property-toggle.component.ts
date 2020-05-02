import {Component, OnDestroy, OnInit} from '@angular/core';
import {FormBuilder, FormGroup, Validators} from '@angular/forms';
import {Subscription} from 'rxjs';

import {Device} from '../../devices/device';
import {DeviceService} from '../../devices/device.service';
import {SubAction} from '../action';
import {PropertyToggle} from '../sub-actions/sub-actions';

import {PropertyListContainer, PropertyListController} from './property-list-container';
import {TypeForm} from './type-form';

@Component({
  selector: 'app-edit-property-toggle',
  templateUrl: './edit-property-toggle.component.html',
  styleUrls: ['./edit-property-toggle.component.css']
})
export class EditPropertyToggleComponent implements OnInit, OnDestroy, TypeForm,
                                                    PropertyListController {
  subAction: PropertyToggle;
  private subscription: Subscription;
  form: FormGroup;
  properties: Map<string, any>;
  container: PropertyListContainer;

  constructor(private deviceService: DeviceService, private fb: FormBuilder) {
    this.container = new PropertyListContainer(deviceService, this);
  }

  ngOnInit() {}
  ngOnDestroy() {
    if (this.subscription) {
      this.subscription.unsubscribe();
    }
  }

  setSubAction(subAction: SubAction) {
    if (typeof (subAction) === typeof (PropertyToggle)) {
      this.subAction = subAction as PropertyToggle;
    } else {
      this.subAction = new PropertyToggle();
      if (subAction) {
        Object.assign(this.subAction, subAction);
      }
    }
    this.container.initialize();
    if (!this.form) {
      this.form = this.fb.group({
        deviceId: [this.subAction.deviceId, Validators.required],
        property: [this.subAction.property, Validators.required]
      });
      this.form.get('deviceId')
          .valueChanges.subscribe((value) => this.onDeviceChange(value));
    } else {
      this.form.setValue({
        deviceId: this.subAction.deviceId,
        property: this.subAction.property
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
  onPropertyChange(property: string): void {}
  getSubAction(): SubAction {
    if (!this.form.valid) {
      return undefined;
    }
    this.subAction.deviceId = this.form.value.deviceId;
    this.subAction.property = this.form.value.property;
    return this.subAction;
  }
  isValid(): boolean {
    return this.form.valid;
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
