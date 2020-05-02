import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { Routes, RouterModule } from '@angular/router';
import { RuleListComponent } from './rule-list/rule-list.component';
import { RuleDetailsComponent } from './rule-details/rule-details.component';
import { AuthGuard } from '../auth/auth-guard.service';
import { RuleAddComponent } from './rule-add/rule-add.component';
import { NavigationComponent } from '../navigation/navigation.component';

const routes: Routes = [
  {
    path: '',
    component: NavigationComponent,
    canActivate: [AuthGuard],
    canActivateChild: [AuthGuard],
    children: [
      { path: 'rules', component: RuleListComponent },
      { path: 'rule/add', component: RuleAddComponent },
      { path: 'rule/:id', component: RuleDetailsComponent }
    ]
  }
];

@NgModule({
  imports: [
    [RouterModule.forChild(routes)]
  ],
  exports: [RouterModule]
})
export class RuleRoutingModule { }
