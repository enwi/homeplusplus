import {CommonModule} from '@angular/common';
import {NgModule} from '@angular/core';
import {MatSnackBarModule} from '@angular/material/snack-bar';

import {PopupNotificationDirective} from './popup-notification.directive';

@NgModule({
  imports: [CommonModule, MatSnackBarModule],
  declarations: [PopupNotificationDirective],
  exports: [PopupNotificationDirective]
})
export class NotificationsModule {
}
