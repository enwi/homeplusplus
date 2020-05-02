import { Injectable, Type, InjectionToken, Inject } from '@angular/core';
import { ConditionView } from './condition-view';
import { ConstantConditionComponent } from './constant-condition.component';
import { CompareConditionComponent } from './compare-condition.component';
import { TimeConditionComponent } from './time-condition.component';
import { DeviceConditionComponent } from './device-condition.component';

export const CONDITION_VIEWS = new InjectionToken<[number, Type<ConditionView>][]>('ConditionTypes', {
  providedIn: 'root',
  factory: () => [
    [0, ConstantConditionComponent],
    [1, CompareConditionComponent],
    [2, TimeConditionComponent],
    [3, DeviceConditionComponent]
  ]
});

@Injectable({
  providedIn: 'root'
})
export class ConditionTypeService {

  private conditionViews: Map<number, Type<ConditionView>>;

  constructor(@Inject(CONDITION_VIEWS) injectedViews: [number, Type<ConditionView>][]) {
    this.conditionViews = new Map(injectedViews);
  }

  getViewType(type: number): Type<ConditionView> {
    return this.conditionViews.get(type);
  }
}
