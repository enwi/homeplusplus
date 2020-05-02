import {CommonModule} from '@angular/common';
import {NgModule} from '@angular/core';
import {FormsModule, ReactiveFormsModule} from '@angular/forms';
import {MatButtonModule} from '@angular/material/button';
import {MatCardModule} from '@angular/material/card';
import {MatCheckboxModule} from '@angular/material/checkbox';
import {MatRippleModule} from '@angular/material/core';
import {MatDialogModule} from '@angular/material/dialog';
import {MatDividerModule} from '@angular/material/divider';
import {MatFormFieldModule} from '@angular/material/form-field';
import {MatIconModule} from '@angular/material/icon';
import {MatInputModule} from '@angular/material/input';
import {MatSelectModule} from '@angular/material/select';

import {ColorConvertModule} from '../color-convert.module';
import {DeleteDialogModule} from '../delete-dialog/delete-dialog.module';

import {ActionRoutingModule} from './/action-routing.module';
import {ActionAddComponent} from './action-add/action-add.component';
import {ActionDetailsComponent} from './action-details/action-details.component';
import {SubActionListComponent} from './action-details/sub-action-list.component';
import {SubActionDirective} from './action-details/sub-action.directive';
import {ActionListComponent} from './action-list/action-list.component';
import {EditNotificationComponent} from './edit-sub-action/edit-notification.component';
import {EditPropertySetComponent} from './edit-sub-action/edit-property-set.component';
import {EditPropertyToggleComponent} from './edit-sub-action/edit-property-toggle.component';
import {EditRecursiveActionComponent} from './edit-sub-action/edit-recursive-action.component';
import {EditSubActionComponent} from './edit-sub-action/edit-sub-action.component';
import {TypeFormDirective} from './edit-sub-action/type-form.directive';
import {ActorSetComponent} from './sub-actions/actor-set.component';
import {ActorToggleComponent} from './sub-actions/actor-toggle.component';
import {NotificationComponent} from './sub-actions/notification.component';
import {RecursiveActionComponent} from './sub-actions/recursive-action.component';

@NgModule({
  imports: [
    CommonModule, ReactiveFormsModule, FormsModule, MatCardModule,
    MatIconModule, MatRippleModule, MatDividerModule, MatButtonModule,
    MatFormFieldModule, MatInputModule, MatSelectModule, MatCheckboxModule,
    MatDialogModule, ColorConvertModule, DeleteDialogModule, ActionRoutingModule
  ],
  declarations: [
    ActionListComponent, ActionDetailsComponent, NotificationComponent,
    SubActionDirective, ActorSetComponent, ActorToggleComponent,
    RecursiveActionComponent, EditSubActionComponent, TypeFormDirective,
    EditNotificationComponent, EditRecursiveActionComponent,
    EditPropertySetComponent, EditPropertyToggleComponent,
    SubActionListComponent, ActionAddComponent
  ],
  exports: [SubActionListComponent]
})
export class ActionsModule {
}
