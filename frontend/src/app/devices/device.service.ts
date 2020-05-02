import {Inject, Injectable, OnDestroy} from '@angular/core';
import {from, Observable, of, Subject} from 'rxjs';
import {filter, map, scan, take} from 'rxjs/operators';
import {WebSocketSubject} from 'rxjs/webSocket';

import {WebsocketChannelService} from '../websocket/websocket-channel.service';
import {WebsocketService} from '../websocket/websocket.service';

import {Device, DeviceMeta, MetaEntry, PropertyChange, PropertyHistory} from './device';
import {DeviceParser} from './device-parser';
import {DEVICE_PARSERS} from './device.config';

@Injectable({providedIn: 'root'})
export class DeviceService implements OnDestroy {
  private parserMap: Map<string, DeviceParser>;

  constructor(
      private channel: WebsocketChannelService,
      @Inject(DEVICE_PARSERS) private parsers: [string, DeviceParser][]) {
    this.parserMap = new Map(parsers);
  }

  private subject: Subject<any>;

  private messageToDevice(message: any): Device|undefined {
    if (message && message.device) {
      const device = message.device;
      const parser = this.parserMap.get(device.type);
      if (parser) {
        return parser.parse(device);
      } else {
        throw new Error('Device parser not found for type: ' + device.type);
      }
    }
    return undefined;
  }

  private messageToMeta(message: any): DeviceMeta|undefined {
    if (message && message.meta) {
      const meta = message.meta;
      const mMeta = new DeviceMeta();
      mMeta.entries = new Map<string, MetaEntry>();
      mMeta.type = message.typeName;
      for (const property of Object.keys(meta)) {
        const entry = meta[property];
        const metaEntry = new MetaEntry();
        metaEntry.dataType = entry['dataType'];
        metaEntry.access = entry['access'];
        mMeta.entries.set(property, metaEntry);
      }
      return mMeta;
    }
    return undefined;
  }

  private messageToPropertyChange(message: any): PropertyChange|undefined {
    if (message && message.propertyChange) {
      return new PropertyChange(
          message.propertyChange.deviceId, message.propertyChange.propertyKey,
          message.propertyChange.value);
    }
    return undefined;
  }

  private messageToPropertyHistory(message: any): PropertyHistory|undefined {
    if (message && message.log) {
      const history = new PropertyHistory();
      history.history = message.log;
      return history;
    }
    return undefined;
  }

  private initSubject(): void {
    if (!this.subject || this.subject.closed) {
      this.subject = this.channel.getChannel('devices');
    }
  }

  getDevices(): Observable<Device> {
    this.initSubject();
    this.subject.next({command: 'GET_DEVICES'});
    return this.subject.pipe(
        map((message: any) => this.messageToDevice(message)),
        // filter(device => device != null));
        filter(device => device !== undefined));
  }

  getDevicesAsArray(): Observable<Device[]> {
    return this.getDevices().pipe(
        scan((list: Device[], device: Device): Device[] => {
          const index = list.findIndex(d => d.id === device.id);
          if (index !== -1) {
            list[index] = device;
          } else {
            list.push(device);
          }
          return list;
        }, []));
  }

  getDevice(id: number): Observable<Device> {
    this.initSubject();
    const obs = this.subject.pipe(
        map((message: any) => this.messageToDevice(message)),
        filter(device => device != null && device.id === id));
    this.subject.next({command: 'GET_DEVICE', id: id});
    return obs;
  }

  getDevicePropertyChanges(id: number): Observable<PropertyChange> {
    this.initSubject();
    return this.subject.pipe(
        map((message: any) => this.messageToPropertyChange(message)),
        filter(property => property != null && property.deviceId === id));
  }

  getPropertyChanges(): Observable<PropertyChange> {
    this.initSubject();
    return this.subject.pipe(
        map((message: any) => this.messageToPropertyChange(message)),
        filter(property => property != null));
  }

  deleteDevice(id: number) {
    this.initSubject();
    this.subject.next({command: 'DELETE_DEVICE', id: id});
  }

  setDeviceName(id: number, name: string) {
    this.initSubject();
    this.subject.next({command: 'SET_DEVICE_NAME', id: id, name: name});
  }

  setDeviceGroups(id: number, groups: string[]) {
    this.initSubject();
    this.subject.next({command: 'SET_DEVICE_GROUPS', id: id, groups: groups});
  }

  setDeviceProperty(deviceId: number, property: string, value: any) {
    this.initSubject();
    this.subject.next({
      command: 'SET_DEVICE_PROPERTY',
      id: deviceId,
      property: property,
      value: value
    });
  }

  getDeviceMeta(type: string): Observable<DeviceMeta> {
    this.initSubject();
    this.subject.next({command: 'GET_TYPE_META', type: type});
    return this.subject.pipe(
        map((message: any) => this.messageToMeta(message)),
        filter(meta => meta != null && meta.type === type), take(1));
  }

  getPropertyHistory(
      properties: {[deviceId: number]: string[]}, start: Date, end?: Date,
      compression?: number): Observable<PropertyHistory> {
    this.initSubject();
    // request = {command: "GET_PROPERTY_LOG", properties: {<deviceid>
    // :[properties]}, compression: <seconds>}
    const command = {
      command: 'GET_PROPERTY_LOG',
      properties: properties,
      // start: start.toUTCString()
      start: Math.round(start.getTime() / 1000)
    };
    if (end !== undefined) {
      // command['end'] = end.toUTCString();
      command['end'] = Math.round(end.getTime() / 1000);
    }
    if (compression !== undefined) {
      command['compression'] = compression;
    }
    this.subject.next(command);
    return this.subject.pipe(
        map((message: any) => this.messageToPropertyHistory(message)),
        filter(
            log => log != null && this.containsAll(log.history, properties)));
  }

  containsAll(obj: any, properties: {[deviceId: number]: string[]}): boolean {
    for (const deviceId of Object.keys(properties)) {
      if (!obj.hasOwnProperty(deviceId)) {
        return false;
      }
      for (const propertyId of properties[deviceId]) {
        if (!obj[deviceId].hasOwnProperty(propertyId)) {
          return false;
        }
      }
    }
    return true;
  }

  ngOnDestroy(): void {
    this.subject.unsubscribe();
  }
}
