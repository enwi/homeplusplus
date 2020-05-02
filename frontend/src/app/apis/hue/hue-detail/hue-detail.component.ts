import {Component, OnInit} from '@angular/core';
import {FormArray, FormBuilder, FormGroup} from '@angular/forms';
import {MatSliderChange} from '@angular/material/slider';
import {Observable} from 'rxjs';

import {hexToHueSat, hueSatToHex} from '../../../color-convert.module';
import {Device, PropertyChange} from '../../../devices/device';
import {DeviceDetail} from '../../../devices/device-detail/device-detail';
import {DeviceService} from '../../../devices/device.service';

@Component({
  selector: 'app-hue-detail',
  templateUrl: './hue-detail.component.html',
  styleUrls: ['./hue-detail.component.css']
})
export class HueDetailComponent implements OnInit, DeviceDetail {
  hue: Device;
  form: FormGroup;

  constructor(private deviceService: DeviceService, private fb: FormBuilder) {}

  ngOnInit() {}

  setDevice(d: Device) {
    this.hue = d;
    this.buildForm();
  }

  onPropertyChange(propertyChange: PropertyChange): void {
    this.hue.properties.set(propertyChange.key, propertyChange.value);
  }

  buildForm() {
    const propertyControls = {};
    const onControl = this.fb.control(this.hue.properties.get('on'));
    onControl.valueChanges.subscribe(value => this.onToggleChange(value));
    propertyControls['on'] = onControl;
    if (this.hue.properties.has('hue') &&
        this.hue.properties.has('saturation')) {
      const formControl = this.fb.control(hueSatToHex(
          this.hue.properties.get('hue'),
          this.hue.properties.get('saturation')));
      formControl.valueChanges.subscribe(value => this.onColorChange(value));
      propertyControls['hueSat'] = formControl;
    }
    if (this.hue.properties.has('brightness')) {
      const formControl =
          this.fb.control(this.hue.properties.get('brightness'));
      formControl.valueChanges.subscribe(
          value => this.onControlChange('brightness', value));
      propertyControls['brightness'] = formControl;
    }
    if (this.hue.properties.has('colorTemperature')) {
      const formControl =
          this.fb.control(this.hue.properties.get('colorTemperature'));
      formControl.valueChanges.subscribe(
          value => this.onControlChange('colorTemperature', value));
      propertyControls['colorTemperature'] = formControl;
    }
    this.form = this.fb.group({properties: this.fb.group(propertyControls)});
  }

  onColorChange(value: string) {
    const [h, s, v] = hexToHueSat(value);
    if (v !== 1.0) {
      const hex = hueSatToHex(h, s);
      this.form.get('properties.hueSat').setValue(hex, {emitEvent: false});
    }
    this.deviceService.setDeviceProperty(this.hue.id, 'hue', h);
    this.deviceService.setDeviceProperty(this.hue.id, 'saturation', s);
  }
  onToggleChange(value: boolean) {
    this.deviceService.setDeviceProperty(this.hue.id, 'on', value);
  }
  onControlChange(property: string, value: number) {
    this.deviceService.setDeviceProperty(this.hue.id, property, value);
  }
}
