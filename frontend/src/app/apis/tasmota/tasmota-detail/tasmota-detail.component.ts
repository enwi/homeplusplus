import {Component, OnInit} from '@angular/core';
import {FormBuilder, FormGroup} from '@angular/forms';
import {MatDialog} from '@angular/material/dialog';

import {Device, PropertyChange} from '../../../devices/device';
import {DeviceDetail} from '../../../devices/device-detail/device-detail';
import {DeviceService} from '../../../devices/device.service';

import {PropertyHistoryDialogComponent} from '../../../dialogs/property-history-dialog/property-history-dialog.component';

@Component({
  selector: 'app-tasmota-detail',
  templateUrl: './tasmota-detail.component.html',
  styleUrls:
      ['./tasmota-detail.component.css', '../../../styles/positioning.css']
})
export class TasmotaDetailComponent implements OnInit, DeviceDetail {
  tasmota: Device;
  form: FormGroup;
  disabled = true;

  powerControls = new Map([
    ['POWER1', 'Power 1'], ['POWER2', 'Power 2'], ['POWER3', 'Power 3'],
    ['POWER4', 'Power 4'], ['POWER5', 'Power 5'], ['POWER6', 'Power 6'],
    ['POWER7', 'Power 7'], ['POWER8', 'Power 8']
  ]);

  constructor(
      private deviceService: DeviceService, private fb: FormBuilder,
      private dialog: MatDialog) {}

  ngOnInit() {}

  setDevice(d: Device) {
    this.tasmota = d;
    this.buildForm();
  }

  onPropertyChange(propertyChange: PropertyChange): void {
    this.tasmota.properties.set(propertyChange.key, propertyChange.value);
    const change = {'properties': {}};
    if (propertyChange.key === 'Color') {
      change.properties[propertyChange.key] = '#' + propertyChange.value;
    } else {
      change.properties[propertyChange.key] = propertyChange.value;
    }
    this.form.patchValue(change, {emitEvent: false});
    if (propertyChange.key === 'online') {
      this.checkOnline();
    }
  }

  buildForm() {
    const propertyControls = {};

    // Actuators
    if (this.tasmota.properties.has('POWER1')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('POWER1'));
      formControl.valueChanges.subscribe(
          value => this.onBooleanChange('POWER1', value));
      propertyControls['POWER1'] = formControl;
    }
    if (this.tasmota.properties.has('POWER2')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('POWER2'));
      formControl.valueChanges.subscribe(
          value => this.onBooleanChange('POWER2', value));
      propertyControls['POWER2'] = formControl;
    }
    if (this.tasmota.properties.has('POWER3')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('POWER3'));
      formControl.valueChanges.subscribe(
          value => this.onBooleanChange('POWER3', value));
      propertyControls['POWER3'] = formControl;
    }
    if (this.tasmota.properties.has('POWER4')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('POWER4'));
      formControl.valueChanges.subscribe(
          value => this.onBooleanChange('POWER4', value));
      propertyControls['POWER4'] = formControl;
    }
    if (this.tasmota.properties.has('POWER5')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('POWER5'));
      formControl.valueChanges.subscribe(
          value => this.onBooleanChange('POWER5', value));
      propertyControls['POWER5'] = formControl;
    }
    if (this.tasmota.properties.has('POWER6')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('POWER6'));
      formControl.valueChanges.subscribe(
          value => this.onBooleanChange('POWER6', value));
      propertyControls['POWER6'] = formControl;
    }
    if (this.tasmota.properties.has('POWER7')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('POWER7'));
      formControl.valueChanges.subscribe(
          value => this.onBooleanChange('POWER7', value));
      propertyControls['POWER7'] = formControl;
    }
    if (this.tasmota.properties.has('POWER8')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('POWER8'));
      formControl.valueChanges.subscribe(
          value => this.onBooleanChange('POWER8', value));
      propertyControls['POWER8'] = formControl;
    }

    if (this.tasmota.properties.has('Dimmer')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('Dimmer'));
      formControl.valueChanges.subscribe(
          value => this.onNumberChange('Dimmer', value));
      propertyControls['Dimmer'] = formControl;
    }
    if (this.tasmota.properties.has('Dimmer1')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('Dimmer1'));
      formControl.valueChanges.subscribe(
          value => this.onBooleanChange('Dimmer1', value));
      propertyControls['Dimmer1'] = formControl;
    }
    if (this.tasmota.properties.has('Dimmer2')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('Dimmer2'));
      formControl.valueChanges.subscribe(
          value => this.onNumberChange('Dimmer2', value));
      propertyControls['Dimmer2'] = formControl;
    }
    if (this.tasmota.properties.has('Dimmer3')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('Dimmer3'));
      formControl.valueChanges.subscribe(
          value => this.onNumberChange('Dimmer3', value));
      propertyControls['Dimmer3'] = formControl;
    }

    if (this.tasmota.properties.has('Color')) {
      const formControl =
          this.fb.control('#' + this.tasmota.properties.get('Color'));
      formControl.valueChanges.subscribe(
          value => this.onColorChange('Color', value));
      propertyControls['Color'] = formControl;
    }

    if (this.tasmota.properties.has('White')) {
      const formControl = this.fb.control(this.tasmota.properties.get('White'));
      formControl.valueChanges.subscribe(
          value => this.onNumberChange('White', value));
      propertyControls['White'] = formControl;
    }

    if (this.tasmota.properties.has('CT')) {
      const formControl = this.fb.control(this.tasmota.properties.get('CT'));
      formControl.valueChanges.subscribe(
          value => this.onNumberChange('CT', value));
      propertyControls['CT'] = formControl;
    }

    if (this.tasmota.properties.has('Scheme')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('Scheme'));
      formControl.valueChanges.subscribe(
          value => this.onNumberChange('Scheme', value));
      propertyControls['Scheme'] = formControl;
    }

    if (this.tasmota.properties.has('Speed')) {
      const formControl = this.fb.control(this.tasmota.properties.get('Speed'));
      formControl.valueChanges.subscribe(
          value => this.onNumberChange('Speed', value));
      propertyControls['Speed'] = formControl;
    }

    if (this.tasmota.properties.has('ShutterClose1')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('ShutterClose1'));
      formControl.valueChanges.subscribe(
          value => this.onBooleanChange('ShutterClose1', value));
      propertyControls['ShutterClose1'] = formControl;
    }
    if (this.tasmota.properties.has('ShutterOpen1')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('ShutterOpen1'));
      formControl.valueChanges.subscribe(
          value => this.onBooleanChange('ShutterOpen1', value));
      propertyControls['ShutterOpen1'] = formControl;
    }
    if (this.tasmota.properties.has('ShutterStop1')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('ShutterStop1'));
      formControl.valueChanges.subscribe(
          value => this.onBooleanChange('ShutterStop1', value));
      propertyControls['ShutterStop1'] = formControl;
    }
    if (this.tasmota.properties.has('ShutterPosition1')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('ShutterPosition1'));
      formControl.valueChanges.subscribe(
          value => this.onNumberChange('ShutterPosition1', value));
      propertyControls['ShutterPosition1'] = formControl;
    }

    if (this.tasmota.properties.has('ShutterClose2')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('ShutterClose2'));
      formControl.valueChanges.subscribe(
          value => this.onBooleanChange('ShutterClose2', value));
      propertyControls['ShutterClose2'] = formControl;
    }
    if (this.tasmota.properties.has('ShutterOpen2')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('ShutterOpen2'));
      formControl.valueChanges.subscribe(
          value => this.onBooleanChange('ShutterOpen2', value));
      propertyControls['ShutterOpen2'] = formControl;
    }
    if (this.tasmota.properties.has('ShutterStop2')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('ShutterStop2'));
      formControl.valueChanges.subscribe(
          value => this.onBooleanChange('ShutterStop2', value));
      propertyControls['ShutterStop2'] = formControl;
    }
    if (this.tasmota.properties.has('ShutterPosition2')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('ShutterPosition2'));
      formControl.valueChanges.subscribe(
          value => this.onNumberChange('ShutterPosition2', value));
      propertyControls['ShutterPosition2'] = formControl;
    }

    if (this.tasmota.properties.has('ShutterClose3')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('ShutterClose3'));
      formControl.valueChanges.subscribe(
          value => this.onBooleanChange('ShutterClose3', value));
      propertyControls['ShutterClose3'] = formControl;
    }
    if (this.tasmota.properties.has('ShutterOpen3')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('ShutterOpen3'));
      formControl.valueChanges.subscribe(
          value => this.onBooleanChange('ShutterOpen3', value));
      propertyControls['ShutterOpen3'] = formControl;
    }
    if (this.tasmota.properties.has('ShutterStop3')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('ShutterStop3'));
      formControl.valueChanges.subscribe(
          value => this.onBooleanChange('ShutterStop3', value));
      propertyControls['ShutterStop3'] = formControl;
    }
    if (this.tasmota.properties.has('ShutterPosition3')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('ShutterPosition3'));
      formControl.valueChanges.subscribe(
          value => this.onNumberChange('ShutterPosition3', value));
      propertyControls['ShutterPosition3'] = formControl;
    }

    if (this.tasmota.properties.has('ShutterClose4')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('ShutterClose4'));
      formControl.valueChanges.subscribe(
          value => this.onBooleanChange('ShutterClose4', value));
      propertyControls['ShutterClose4'] = formControl;
    }
    if (this.tasmota.properties.has('ShutterOpen4')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('ShutterOpen4'));
      formControl.valueChanges.subscribe(
          value => this.onBooleanChange('ShutterOpen4', value));
      propertyControls['ShutterOpen4'] = formControl;
    }
    if (this.tasmota.properties.has('ShutterStop4')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('ShutterStop4'));
      formControl.valueChanges.subscribe(
          value => this.onBooleanChange('ShutterStop4', value));
      propertyControls['ShutterStop4'] = formControl;
    }
    if (this.tasmota.properties.has('ShutterPosition4')) {
      const formControl =
          this.fb.control(this.tasmota.properties.get('ShutterPosition4'));
      formControl.valueChanges.subscribe(
          value => this.onNumberChange('ShutterPosition4', value));
      propertyControls['ShutterPosition4'] = formControl;
    }

    // Sensors


    this.form = this.fb.group({properties: this.fb.group(propertyControls)});
    this.checkOnline();
  }

  checkOnline() {
    this.disabled = !this.tasmota.properties.get('online');
    if (this.disabled) {
      this.form.disable({emitEvent: false});
    } else {
      this.form.enable({emitEvent: false});
    }
  }

  updateDevice(device: Device) {
    this.tasmota.properties = device.properties;
  }

  onBooleanChange(property: string, value: boolean) {
    this.deviceService.setDeviceProperty(this.tasmota.id, property, value);
  }

  onNumberChange(property: string, value: number) {
    this.deviceService.setDeviceProperty(this.tasmota.id, property, value);
  }

  onColorChange(property: string, value: string) {
    this.deviceService.setDeviceProperty(this.tasmota.id, property, value);
  }


  showOnline() {
    this.showDialog(this.createDialogData({'online': false}));
  }


  showDimmer() {
    this.showDialog(this.createDialogData({
      'Dimmer': false,
      'Dimmer0': false,
      'Dimmer1': false,
      'Dimmer2': false
    }));
  }

  showColor() {
    this.showDialog(this.createDialogData({
      'Color': true,  // Colors currently not supported
      'White': false,
      'CT': false,
      'Speed': false,
      'Scheme': false
    }));
  }

  showShutter1() {
    this.showDialog(this.createDialogData({
      'ShutterClose1': false,
      'ShutterOpen1': true,
      'ShutterStop1': true,
      'ShutterPosition1': true
    }));
  }

  showShutter2() {
    this.showDialog(this.createDialogData({
      'ShutterClose2': false,
      'ShutterOpen2': true,
      'ShutterStop2': true,
      'ShutterPosition2': true
    }));
  }

  showShutter3() {
    this.showDialog(this.createDialogData({
      'ShutterClose3': false,
      'ShutterOpen3': true,
      'ShutterStop3': true,
      'ShutterPosition3': true
    }));
  }

  showShutter4() {
    this.showDialog(this.createDialogData({
      'ShutterClose4': false,
      'ShutterOpen4': true,
      'ShutterStop4': true,
      'ShutterPosition4': true
    }));
  }


  // Sensors
  showAnalog() {
    this.showDialog(this.createDialogData({
      'ANALOGA0': false,
      'ANALOGTemperature': false,
      'ANALOGIlluminance': false
    }));
  }

  showAM2301() {
    this.showDialog(this.createDialogData(
        {'AM2301Temperature': false, 'AM2301Humidity': true}));
  }

  showAPDS9960() {
    this.showDialog(this.createDialogData({
      'APDS9960Red': false,
      'APDS9960Green': false,
      'APDS9960Blue': false,
      'APDS9960Ambient': false,
      'APDS9960CCT': false,
      'APDS9960Proximity': false
    }));
  }

  showBH1750() {
    this.showDialog(this.createDialogData({'BH1750Illuminance': false}));
  }

  showBME280() {
    this.showDialog(this.createDialogData({
      'BME280Temperature': false,
      'BME280Humidity': false,
      'BME280DewPoint': false,
      'BME280Pressure': true
    }));
  }

  showBME680() {
    this.showDialog(this.createDialogData({
      'BME680Temperature': false,
      'BME680Humidity': false,
      'BME680DewPoint': false,
      'BME680Gas': true
    }));
  }

  showDHT11() {
    this.showDialog(this.createDialogData(
        {'DHT11Temperature': false, 'DHT11Humidity': true}));
  }

  showDS18x20() {
    this.showDialog(this.createDialogData({
      'DS18x20Temperature1': false,
      'DS18x20Temperature2': false,
      'DS18x20Temperature3': false,
      'DS18x20Temperature4': false,
      'DS18x20Temperature5': false,
      'DS18x20Temperature6': false,
      'DS18x20Temperature7': false,
      'DS18x20Temperature8': false
    }));
  }

  showHCSR04() {
    this.showDialog(this.createDialogData({'SR04Distance': false}));
  }

  showHTU21() {
    this.showDialog(this.createDialogData(
        {'HTU21Temperature': false, 'HTU21Humidity': true}));
  }

  showLM75AD() {
    this.showDialog(this.createDialogData({'LM75ADTemperature': false}));
  }

  showMLX90614() {
    this.showDialog(this.createDialogData(
        {'MLX90614ObjectT': false, 'MLX90614AmbientT': false}));
  }

  showMPU6050() {
    this.showDialog(this.createDialogData({
      'MPU6050Temperature': false,
      'MPU6050AccelXAxis': true,
      'MPU6050AccelYAxis': true,
      'MPU6050AccelZAxis': true,
      'MPU6050GyroXAxis': true,
      'MPU6050GyroYAxis': true,
      'MPU6050GyroZAxis': true,
      'MPU6050Yaw': true,
      'MPU6050Pitch': true,
      'MPU6050Roll': true
    }));
  }

  showPMS5003() {
    this.showDialog(this.createDialogData({
      'PMS5003CF1': false,
      'PMS5003CF2_5': false,
      'PMS5003CF10': false,
      'PMS5003PM1': false,
      'PMS5003PM2_5': false,
      'PMS5003PM10': false,
      'PMS5003PB0_3': true,
      'PMS5003PB0_5': true,
      'PMS5003PB1': true,
      'PMS5003PB2_5': true,
      'PMS5003PB5': true,
      'PMS5003PB10': true
    }));
  }

  showPIR() {
    this.showDialog(this.createDialogData({
      'PIR1': false,
      'PIR2': false,
      'PIR3': false,
      'PIR4': false,
      'PIR5': false,
      'PIR6': false,
      'PIR7': false,
      'PIR8': false
    }));
  }

  showSDS011() {
    this.showDialog(
        this.createDialogData({'SD0X1PM2_5': false, 'SD0X1PM10': false}));
  }

  showSHT3X() {
    this.showDialog(this.createDialogData(
        {'SHT3XTemperature': false, 'SHT3XHumidity': true}));
  }

  showTX23() {
    this.showDialog(this.createDialogData({
      'TX23SpeedAct': false,
      'TX23SpeedAvg': false,
      'TX23SpeedMin': false,
      'TX23SpeedMax': false,
      // 'TX23DirCard': false, // String not supported
      'TX23DirDeg': true,
      'TX23DirAvg': true,
      // 'TX23DirAvgCard': true, // String not supported
      'TX23DirMin': true,
      'TX23DirMax': true,
      'TX23DirRange': true
    }));
  }

  showTSL2561() {
    this.showDialog(this.createDialogData({'TSL2561Illuminance': false}));
  }

  showVL53L0X() {
    this.showDialog(this.createDialogData({'VL53L0XDistance': false}));
  }

  private createDialogData(properties: {[propertyId: string]: boolean}):
      {[deviceId: number]: {[propertyId: string]: boolean}} {
    const data = {};
    data[this.tasmota.id] = properties;
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
