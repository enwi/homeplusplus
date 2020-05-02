import { Component, OnInit, EventEmitter } from '@angular/core';
import { Notification } from './sub-actions';
import { SubActionView } from './sub-action-view';
import { SubAction } from '../action';
import { notificationCategoryToString } from '../../notifications/notification';

@Component({
  selector: 'app-notification',
  templateUrl: './notification.component.html',
  styleUrls: ['./notification.component.css', './sub-action.css']
})
export class NotificationComponent implements OnInit, SubActionView {

  subAction: Notification;
  edit: EventEmitter<void>;
  delete: EventEmitter<void>;

  constructor() { }

  ngOnInit() {
  }

  initialize(subAction: SubAction, editAction: EventEmitter<void>, deleteAction: EventEmitter<void>) {
    this.subAction = subAction as Notification;
    this.edit = editAction;
    this.delete = deleteAction;
  }

  categoryToString(category: number): string {
    return notificationCategoryToString(category);
  }

}
