import { NgModule } from '@angular/core';
import { Routes, RouterModule } from '@angular/router';
import { ActionListComponent } from './action-list/action-list.component';
import { ActionDetailsComponent } from './action-details/action-details.component';
import { AuthGuard } from '../auth/auth-guard.service';
import { ActionAddComponent } from './action-add/action-add.component';
import { NavigationComponent } from '../navigation/navigation.component';

const routes: Routes = [
  {
    path: '',
    component: NavigationComponent,
    canActivate: [AuthGuard],
    canActivateChild: [AuthGuard],
    children: [
      { path: 'actions', component: ActionListComponent },
      { path: 'action/add', component: ActionAddComponent },
      { path: 'action/:id', component: ActionDetailsComponent }
    ]
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class ActionRoutingModule { }
