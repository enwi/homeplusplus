import { Injectable, Type, InjectionToken, Inject } from '@angular/core';
import { ActorToggleComponent } from './actor-toggle.component';
import { ActorSetComponent } from './actor-set.component';
import { SubActionView } from './sub-action-view';
import { NotificationComponent } from './notification.component';
import { RecursiveActionComponent } from './recursive-action.component';
import { TypeFormInfo } from '../edit-sub-action/type-form';
import { EditPropertySetComponent } from '../edit-sub-action/edit-property-set.component';
import { EditPropertyToggleComponent } from '../edit-sub-action/edit-property-toggle.component';
import { EditNotificationComponent } from '../edit-sub-action/edit-notification.component';
import { EditRecursiveActionComponent } from '../edit-sub-action/edit-recursive-action.component';

export const SUB_ACTION_VIEWS = new InjectionToken<[number, Type<SubActionView>][]>('SubActionTypes', {
  providedIn: 'root',
  factory: () => [
    [0, ActorSetComponent],
    [1, ActorToggleComponent],
    [2, NotificationComponent],
    [5, RecursiveActionComponent]
  ]
});

export const SUB_ACTION_EDITS = new InjectionToken<TypeFormInfo[]>('SubActionEdits', {
  providedIn: 'root',
  factory: () => [
    { displayName: 'actorSet', typeId: 0, type: EditPropertySetComponent },
    { displayName: 'actorToggle', typeId: 1, type: EditPropertyToggleComponent },
    { displayName: 'notification', typeId: 2, type: EditNotificationComponent },
    { displayName: 'recursiveAction', typeId: 5, type: EditRecursiveActionComponent }
  ]
});

@Injectable({
  providedIn: 'root'
})
export class SubActionTypeService {

  private subActionViews: Map<number, Type<SubActionView>>;

  constructor(@Inject(SUB_ACTION_VIEWS) injectedTypes: [number, Type<SubActionView>][],
    @Inject(SUB_ACTION_EDITS) private subActionEdit: TypeFormInfo[]) {
    this.subActionViews = new Map(injectedTypes);
  }

  getViewType(type: number): Type<SubActionView> {
    return this.subActionViews.get(type);
  }
  getEditTypes(): Array<TypeFormInfo> {
    return this.subActionEdit;
  }
  getEditType(type: number): TypeFormInfo {
    return this.subActionEdit.find(t => t.typeId === type);
  }
}
