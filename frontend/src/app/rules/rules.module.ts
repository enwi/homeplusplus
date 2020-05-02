import {CommonModule} from '@angular/common';
import {NgModule} from '@angular/core';
import {ReactiveFormsModule} from '@angular/forms';
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

import {ActionsModule} from '../actions/actions.module';
import {ColorConvertModule} from '../color-convert.module';

import {CompareConditionComponent} from './conditions/compare-condition.component';
import {ConditionDirective} from './conditions/condition.directive';
import {ConstantConditionComponent} from './conditions/constant-condition.component';
import {DeviceConditionComponent} from './conditions/device-condition.component';
import {TimeConditionComponent} from './conditions/time-condition.component';
import {EditableConditionComponent} from './editable-condition/editable-condition.component';
import {RuleAddComponent} from './rule-add/rule-add.component';
import {RuleDetailsComponent} from './rule-details/rule-details.component';
import {RuleListComponent} from './rule-list/rule-list.component';
import {RuleRoutingModule} from './rule-routing.module';

@NgModule({
  imports: [
    CommonModule, MatCardModule, MatCheckboxModule, MatRippleModule,
    MatIconModule, MatDividerModule, ReactiveFormsModule, MatFormFieldModule,
    MatButtonModule, MatInputModule, MatDialogModule, MatSelectModule,
    ColorConvertModule, ActionsModule, RuleRoutingModule
  ],
  declarations: [
    RuleListComponent, RuleDetailsComponent, ConditionDirective,
    ConstantConditionComponent, CompareConditionComponent,
    TimeConditionComponent, DeviceConditionComponent, RuleAddComponent,
    EditableConditionComponent
  ]
})
export class RulesModule {
}
