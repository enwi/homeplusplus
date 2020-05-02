import { InjectionToken } from '@angular/core';

export const ACTION_ICONS = new InjectionToken<[string, string][]>('ActionIcons', {
  providedIn: 'root',
  factory: () => [
    ['cancel.svg', 'Cancel'],
    ['lightbulb_off.png', 'Lightbulb off'],
    ['lightbulb_on.png', 'Lightbulb on']
  ]
});
