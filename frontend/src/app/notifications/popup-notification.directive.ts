import {Directive, OnDestroy, OnInit} from '@angular/core';
import {MatSnackBar} from '@angular/material/snack-bar';
import {Subscription} from 'rxjs';

import {Notification} from './notification';
import {notificationCategoryToString} from './notification';
import {NotificationService} from './notification.service';

@Directive({selector: 'app-popup-notification'})
export class PopupNotificationDirective implements OnInit, OnDestroy {
  subscription: Subscription;
  previousNotifications = 0;

  constructor(
      private notificationService: NotificationService,
      private snackBar: MatSnackBar) {}

  ngOnInit() {
    this.subscription = this.notificationService.getNotifications().subscribe(
        (notification: Notification) => this.onNotification(notification));
  }
  ngOnDestroy() {
    if (this.subscription) {
      this.subscription.unsubscribe();
    }
  }
  onNotification(notification: Notification) {
    let message = `${notificationCategoryToString(notification.category)}: ${
        notification.message}`;
    if (this.previousNotifications > 0) {
      message += ` (+ ${this.previousNotifications} weitere)`;
    }
    const snackBarRef =
        this.snackBar.open(message, undefined, {duration: 2000});
    ++this.previousNotifications;
    snackBarRef.afterDismissed().subscribe(dismiss => this.afterDismissed());
  }
  afterDismissed() {
    this.previousNotifications = 0;
  }
}
