import {CommonModule} from '@angular/common';
import {NgModule} from '@angular/core';
import {ReactiveFormsModule} from '@angular/forms';
import {MatButtonModule} from '@angular/material/button';
import {MatCardModule} from '@angular/material/card';
import {MatFormFieldModule} from '@angular/material/form-field';
import {MatIconModule} from '@angular/material/icon';
import {MatInputModule} from '@angular/material/input';

import {LoginComponent} from '../login/login.component';

import {AuthRoutingModule} from './auth-routing.module';
import {LogoutComponent} from './logout/logout.component';

@NgModule({
  imports: [
    CommonModule, ReactiveFormsModule, MatCardModule, MatFormFieldModule,
    MatIconModule, MatButtonModule, MatInputModule, AuthRoutingModule
  ],
  declarations: [LoginComponent, LogoutComponent],
  exports: [LoginComponent]
})
export class AuthModule {
}
