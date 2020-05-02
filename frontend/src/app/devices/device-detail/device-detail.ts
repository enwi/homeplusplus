import {Observable} from 'rxjs';
import {Device, PropertyChange} from '../device';

export interface DeviceDetail {
  setDevice(d: Device): void;
  onPropertyChange(propertyChange: PropertyChange): void;
}
