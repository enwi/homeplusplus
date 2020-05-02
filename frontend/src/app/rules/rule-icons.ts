import { InjectionToken } from '@angular/core';

export const RULE_ICONS = new InjectionToken<[string, string][]>('RuleIcons', {
  providedIn: 'root',
  factory: () =>
    [
      ['cancel.svg', 'Cancel'],
      ['lightbulb_off.png', 'Lightbulb off'],
      ['lightbulb_on.png', 'Lightbulb on']
    ]
});
