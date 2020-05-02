import { SubAction } from '../action';

export class Notification extends SubAction {
  category: number;
  message: string;
}

export class PropertySet extends SubAction {
  deviceId: number;
  property: string;
  value: any;
}

export class PropertyToggle extends SubAction {
  deviceId: number;
  property: string;
}

export class RecursiveAction extends SubAction {
  actionId: number;
}
