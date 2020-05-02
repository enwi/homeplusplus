import { Routes } from '@angular/router';

// routing components
import { DashboardComponent } from './dashboard/dashboard.component';
import { UserComponent } from './user/user.component';
import { StatisticsComponent } from './statistics/statistics.component';
import { AuthGuard } from './auth/auth-guard.service';
import { NavigationComponent } from './navigation/navigation.component';

export const routes: Routes = [
  { path: '', redirectTo: '/dashboard', pathMatch: 'full' },
  {
    path: '',
    component: NavigationComponent,
    canActivate: [AuthGuard],
    canActivateChild: [AuthGuard],
    children: [
      { path: 'dashboard', component: DashboardComponent },
      { path: 'user', component: UserComponent },
      { path: 'stats', component: StatisticsComponent }
    ]
  }
];
