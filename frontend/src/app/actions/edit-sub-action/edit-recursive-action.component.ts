import { Component, OnInit } from '@angular/core';
import { TypeForm } from './type-form';
import { SubAction, Action } from '../action';
import { Observable } from 'rxjs';
import { FormGroup, FormBuilder, Validators } from '@angular/forms';
import { ActionService } from '../action.service';
import { RecursiveAction } from '../sub-actions/sub-actions';

@Component({
  selector: 'app-edit-recursive-action',
  templateUrl: './edit-recursive-action.component.html',
  styleUrls: ['./edit-recursive-action.component.css']
})
export class EditRecursiveActionComponent implements OnInit, TypeForm {

  subAction: RecursiveAction;
  actions$: Observable<Action[]>;
  form: FormGroup;

  constructor(private actionService: ActionService, private fb: FormBuilder) { }

  ngOnInit() {
  }

  setSubAction(subAction: SubAction) {
    if (typeof (subAction) === typeof (RecursiveAction)) {
      this.subAction = subAction as RecursiveAction;
    } else {
      this.subAction = new RecursiveAction();
      if (subAction) {
        Object.assign(this.subAction, subAction);
      }
    }
    this.actions$ = this.actionService.getActionsAsArray();
    this.form = this.fb.group({
      actionId: [this.subAction.actionId, Validators.required]
    });
  }
  getSubAction(): SubAction {
    if (!this.form.valid) {
      return undefined;
    }
    this.subAction.actionId = this.form.value.actionId;
    return this.subAction;
  }
  isValid(): boolean {
    return this.form.valid;
  }
}
