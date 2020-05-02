import {PropertyListContainer} from './../actions/edit-sub-action/property-list-container';
export class Device {
  id: number;
  name: string;
  icon: string;
  groups: string[];
  type: string;
  properties: Map<string, any>;

  toJson(): string {
    return JSON.stringify(this);
  }
  parse(json: any): void {
    this.id = json['id'];
    this.name = json['name'];
    this.icon = json['icon'];
    this.groups = json['groups'];
    this.type = json['type'];
    this.properties = new Map<string, any>();
    for (const property of Object.keys(json['properties'])) {
      this.properties.set(property, json['properties'][property]);
    }
  }
}

export class PropertyChange {
  deviceId: number;
  key: string;
  value: any;

  constructor(deviceId: number, key: string, value: any) {
    this.deviceId = deviceId;
    this.key = key;
    this.value = value;
  }
}

export class PropertyHistory {
  history: {[deviceId: number]: {[propertyKey: string]: {[date: string]: any}}};
}

export class MetaEntry {
  dataType: number;
  access: number;
  getUserRead(): boolean {
    return (this.access & 1) !== 0;
  }
  getUserWrite(): boolean {
    return (this.access & 2) !== 0;
  }
  getRuleRead(): boolean {
    return (this.access & 4) !== 0;
  }
  getActionWrite(): boolean {
    return (this.access & 8) !== 0;
  }
}

export class DeviceMeta {
  type: string;
  entries: Map<string, MetaEntry>;
}
