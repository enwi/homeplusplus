import { InjectionToken, Type } from '@angular/core';
import { DeviceDetail } from './device-detail/device-detail';
import { DeviceParser } from './device-parser';

export const DEVICE_DETAILS = new InjectionToken<[string, Type<DeviceDetail>][]>('DeviceTypes', { factory: () => [] });

export const DEVICE_PARSERS = new InjectionToken<[string, DeviceParser][]>('DeviceParsers', { factory: () => [] });

