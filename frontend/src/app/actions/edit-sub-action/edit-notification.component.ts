import { Component, OnInit } from '@angular/core';
import { TypeForm } from './type-form';
import { SubAction } from '../action';
import { FormGroup, FormBuilder, Validators } from '@angular/forms';
import { Notification } from '../sub-actions/sub-actions';

@Component({
  selector: 'app-edit-notification',
  templateUrl: './edit-notification.component.html',
  styleUrls: ['./edit-notification.component.css']
})
export class EditNotificationComponent implements OnInit, TypeForm {

  subAction: Notification;
  form: FormGroup;

  constructor(private fb: FormBuilder) { }

  ngOnInit() {
  }

  setSubAction(subAction: SubAction) {
    if (typeof(subAction) === typeof(Notification)) {
      this.subAction = subAction as Notification;
    } else {
      this.subAction = new Notification();
      if (subAction) {
        Object.assign(this.subAction, subAction);
      }
    }
    this.form = this.fb.group({
      category: [this.subAction.category, Validators.required],
      message: [this.subAction.message, Validators.required]
    });
  }
  getSubAction(): SubAction {
    if (!this.form.valid) {
      return undefined;
    }
    this.subAction.category = +this.form.value.category;
    this.subAction.message = this.form.value.message;
    return this.subAction;
  }
  isValid(): boolean {
    return this.form.valid;
  }
}
