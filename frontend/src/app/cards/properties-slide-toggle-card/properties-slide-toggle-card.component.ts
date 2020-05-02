import {Component, Input, OnChanges, OnDestroy, OnInit, SimpleChanges} from '@angular/core';
import {FormBuilder, FormGroup} from '@angular/forms';
import {MatDialog} from '@angular/material/dialog';
import {MatSlideToggleChange} from '@angular/material/slide-toggle';
import {Subscription} from 'rxjs';

import {DeviceService} from '../../devices/device.service';
import {PropertyHistoryDialogComponent} from '../../dialogs/property-history-dialog/property-history-dialog.component';

@Component({
  selector: 'app-properties-slide-toggle-card',
  templateUrl: './properties-slide-toggle-card.component.html',
  styleUrls: [
    './properties-slide-toggle-card.component.css',
    '../../styles/positioning.css'
  ]
})
export class PropertiesSlideToggleCardComponent implements OnInit, OnDestroy,
                                                           OnChanges {
  @Input() deviceId: number;
  @Input() title: string;
  @Input() properties: Map<string, string>;
  @Input() disabled = false;

  subscription: Subscription;
  form: FormGroup;
  valid = false;

  masterChecked = false;

  constructor(
      private deviceService: DeviceService, private fb: FormBuilder,
      private dialog: MatDialog) {}

  ngOnInit() {
    const deviceSub =
        this.deviceService.getDevice(this.deviceId).subscribe(device => {
          deviceSub.unsubscribe();
          const propertyControls = {};
          this.properties.forEach((propertyValue, propertyKey) => {
            if (device.properties.has(propertyKey)) {
              const formControl =
                  this.fb.control(device.properties.get(propertyKey));
              formControl.valueChanges.subscribe(
                  value => this.onBooleanChange(propertyKey, value));
              propertyControls[propertyKey] = formControl;
              this.valid = true;
            }
          });
          this.form =
              this.fb.group({properties: this.fb.group(propertyControls)});
          this.handleDisabled(this.disabled);
          // Make sure that master button gets enabled, when any property is
          // true before an update happens
          this.checkAllProperties();

          this.subscription =
              this.deviceService.getDevicePropertyChanges(this.deviceId)
                  .subscribe(propertyChange => {
                    if (this.properties.has(propertyChange.key)) {
                      const change = {'properties': {}};
                      change.properties[propertyChange.key] =
                          propertyChange.value;
                      this.form.patchValue(change, {emitEvent: false});

                      if (propertyChange.value === true) {
                        this.masterChecked = true;
                      } else if (propertyChange.value === false) {
                        this.checkAllProperties();
                      }
                    }
                  });
        });
  }

  private checkAllProperties() {
    let enabled = false;
    this.properties.forEach((value, key) => {
      const element = this.form.get('properties.' + key);
      if (element != null) {
        if (element.value === true) {
          enabled = true;
        }
      }
    });
    this.masterChecked = enabled;
  }

  ngOnDestroy() {
    if (this.subscription) {
      this.subscription.unsubscribe();
    }
  }

  ngOnChanges(changes: SimpleChanges) {
    if (changes.disabled !== undefined) {
      console.log('ngOnChanges ' + changes.disabled.currentValue);
      this.handleDisabled(changes.disabled.currentValue);
    }
  }

  handleDisabled(disabled: boolean) {
    this.disabled = disabled;
    if (this.form !== undefined) {
      if (disabled) {
        this.form.disable({emitEvent: false});
      } else {
        this.form.enable({emitEvent: false});
      }
    }
  }

  updateMaster(e: MatSlideToggleChange) {
    this.properties.forEach((value, key) => {
      const element = this.form.get('properties.' + key);
      if (element != null) {
        element.setValue(e.checked);
      }
    });
  }

  onBooleanChange(property: string, value: boolean) {
    this.deviceService.setDeviceProperty(this.deviceId, property, value);
  }

  showHistory() {
    const availableProps: {[propertyId: string]: boolean} = {};
    this.properties.forEach((value, key) => {
      availableProps[key] = false;
    });
    this.showDialog(this.createDialogData(availableProps));
  }

  private createDialogData(properties: {[propertyId: string]: boolean}):
      {[deviceId: number]: {[propertyId: string]: boolean}} {
    const data = {};
    data[this.deviceId] = properties;
    return data;
  }

  private showDialog(
      data: {[deviceId: number]: {[propertyId: string]: boolean}}) {
    const dialogRef = this.dialog.open(PropertyHistoryDialogComponent, {
      height: '80vh',
      width: '80vw',
      data: data,
    });
    dialogRef.afterClosed().subscribe(result => {
      console.log('The dialog was closed');
    });
  }
}
